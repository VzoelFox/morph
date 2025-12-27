/*
 * Morph Memory System V2 - Implementation
 * Week 3-4: Pool Allocator (Fixed-size slab allocation)
 *
 * Design: See MEMORY_ARCHITECTURE_V2.md
 * Roadmap: See MEMORY_V2_ROADMAP.md
 */

#include "morph_mem_v2.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

// Platform-specific includes for RAM detection
#ifdef __linux__
#include <sys/sysinfo.h>
#elif __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#elif _WIN32
#include <windows.h>
#endif

//=============================================================================
// PRESET CONFIGURATIONS
//=============================================================================

const MorphMemConfig MORPH_CONFIG_COMPILER = {
    .mode = MORPH_MODE_COMPILER,
    .heap_size_hint = 256 * 1024 * 1024,  // 256MB hint
    .gc_threshold = 0,                     // No GC during compilation
    .gc_pause_target_ms = 0,
    .enable_generational = 0,
    .enable_compaction = 0,
    .enable_metrics = 0,
    .enable_debug = 0
};

const MorphMemConfig MORPH_CONFIG_RUNTIME = {
    .mode = MORPH_MODE_RUNTIME,
    .heap_size_hint = 512 * 1024 * 1024,  // 512MB hint
    .gc_threshold = 64 * 1024 * 1024,     // 64MB threshold
    .gc_pause_target_ms = 10,              // 10ms target
    .enable_generational = 1,
    .enable_compaction = 0,
    .enable_metrics = 1,
    .enable_debug = 0
};

const MorphMemConfig MORPH_CONFIG_VM = {
    .mode = MORPH_MODE_VM,
    .heap_size_hint = 2ULL * 1024 * 1024 * 1024,  // 2GB hint
    .gc_threshold = 128 * 1024 * 1024,             // 128MB threshold
    .gc_pause_target_ms = 5,                       // 5ms target
    .enable_generational = 1,
    .enable_compaction = 1,
    .enable_metrics = 1,
    .enable_debug = 0
};

const MorphMemConfig MORPH_CONFIG_SERVER = {
    .mode = MORPH_MODE_SERVER,
    .heap_size_hint = 1024 * 1024 * 1024,  // 1GB bounded
    .gc_threshold = 128 * 1024 * 1024,     // 128MB threshold
    .gc_pause_target_ms = 5,               // 5ms target
    .enable_generational = 1,
    .enable_compaction = 1,
    .enable_metrics = 1,
    .enable_debug = 0
};

//=============================================================================
// INTERNAL CONTEXT STRUCTURE
//=============================================================================

struct MorphContextV2 {
    MorphMemConfig config;

    // Allocator state (Week 3-4: Arena + Pool)
    Arena* arena;              // Arena allocator (COMPILER mode)
    PoolManager* pool_manager; // Pool allocator (small objects)
    void* gc_heap;             // GC heap (RUNTIME/VM mode, Week 7+)

    // Statistics
    MorphMemStats stats;

    // Type registry
    const char* type_names[128];

    // Shadow stack for GC roots
    void** root_stack;
    size_t root_count;
    size_t root_capacity;

    // Thread safety
    pthread_mutex_t lock;
};

//=============================================================================
// UTILITY - RAM Detection
//=============================================================================

size_t morph_mem_get_available_ram(void) {
#ifdef __linux__
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return (size_t)info.freeram * (size_t)info.mem_unit;
    }
#elif __APPLE__
    uint64_t memsize;
    size_t len = sizeof(memsize);
    if (sysctlbyname("hw.memsize", &memsize, &len, NULL, 0) == 0) {
        // Return 50% of total RAM as "available"
        return (size_t)(memsize / 2);
    }
#elif _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return (size_t)status.ullAvailPhys;
    }
#endif

    // Fallback: assume 1GB available
    return 1024 * 1024 * 1024;
}

MorphMemConfig morph_mem_detect_config(void) {
    size_t available_ram = morph_mem_get_available_ram();

    // Low memory system (<512MB free)
    if (available_ram < 512 * 1024 * 1024) {
        MorphMemConfig config = MORPH_CONFIG_COMPILER;  // Use compiler mode (no GC)
        config.heap_size_hint = available_ram / 4;
        return config;
    }

    // Medium memory (512MB - 4GB free)
    if (available_ram < 4ULL * 1024 * 1024 * 1024) {
        MorphMemConfig config = MORPH_CONFIG_RUNTIME;
        config.heap_size_hint = available_ram / 2;
        config.gc_threshold = available_ram / 8;
        return config;
    }

    // High memory (>4GB free)
    MorphMemConfig config = MORPH_CONFIG_RUNTIME;
    config.heap_size_hint = 2ULL * 1024 * 1024 * 1024;  // Cap at 2GB
    config.gc_threshold = 256 * 1024 * 1024;            // 256MB threshold
    return config;
}

//=============================================================================
// ARENA ALLOCATOR - Week 2
//=============================================================================

Arena* arena_create(size_t block_size) {
    if (block_size == 0) {
        block_size = ARENA_BLOCK_SIZE;  // Default 2MB
    }

    Arena* arena = (Arena*)malloc(sizeof(Arena));
    if (!arena) {
        fprintf(stderr, "FATAL: Failed to allocate Arena\n");
        abort();
    }

    // Allocate first block
    ArenaBlock* block = (ArenaBlock*)malloc(sizeof(ArenaBlock) + block_size);
    if (!block) {
        free(arena);
        fprintf(stderr, "FATAL: Failed to allocate ArenaBlock\n");
        abort();
    }

    block->next = NULL;
    block->capacity = block_size;
    block->used = 0;

    arena->current = block;
    arena->first = block;
    arena->block_size = block_size;
    arena->total_allocated = block_size;
    arena->total_used = 0;

    return arena;
}

void* arena_alloc(Arena* arena, size_t size) {
    return arena_alloc_aligned(arena, size, OBJECT_ALIGNMENT);
}

void* arena_alloc_aligned(Arena* arena, size_t size, size_t alignment) {
    if (size == 0) return NULL;

    ArenaBlock* block = arena->current;

    // Calculate aligned offset
    size_t current_ptr = (size_t)(&block->data[block->used]);
    size_t aligned_ptr = ALIGN_UP(current_ptr, alignment);
    size_t padding = aligned_ptr - current_ptr;
    size_t total_needed = padding + size;

    // Check if current block has enough space
    if (block->used + total_needed > block->capacity) {
        // Need new block
        size_t new_block_size = arena->block_size;

        // If allocation is larger than default block size, use larger block
        if (total_needed > new_block_size) {
            new_block_size = ALIGN_UP(total_needed, arena->block_size);
        }

        ArenaBlock* new_block = (ArenaBlock*)malloc(sizeof(ArenaBlock) + new_block_size);
        if (!new_block) {
            fprintf(stderr, "ERROR: Failed to allocate new ArenaBlock (%zu bytes)\n",
                    new_block_size);
            return NULL;
        }

        new_block->next = NULL;
        new_block->capacity = new_block_size;
        new_block->used = 0;

        // Link to arena
        block->next = new_block;
        arena->current = new_block;
        arena->total_allocated += new_block_size;

        // Recalculate with new block
        block = new_block;
        current_ptr = (size_t)(&block->data[block->used]);
        aligned_ptr = ALIGN_UP(current_ptr, alignment);
        padding = aligned_ptr - current_ptr;
        total_needed = padding + size;
    }

    // Allocate from current block (bump pointer!)
    void* ptr = &block->data[block->used + padding];
    block->used += total_needed;
    arena->total_used += total_needed;

    return ptr;
}

void arena_reset(Arena* arena) {
    // Reset all blocks without freeing them
    ArenaBlock* block = arena->first;
    while (block) {
        block->used = 0;
        block = block->next;
    }

    arena->current = arena->first;
    arena->total_used = 0;
}

void arena_destroy(Arena* arena) {
    if (!arena) return;

    // Free all blocks
    ArenaBlock* block = arena->first;
    while (block) {
        ArenaBlock* next = block->next;
        free(block);
        block = next;
    }

    free(arena);
}

//=============================================================================
// POOL ALLOCATOR - Week 3-4
//=============================================================================

// Helper: Get pool index for size class
int pool_get_size_class(size_t size) {
    if (size <= POOL_SIZE_16)  return 0;
    if (size <= POOL_SIZE_32)  return 1;
    if (size <= POOL_SIZE_64)  return 2;
    if (size <= POOL_SIZE_128) return 3;
    if (size <= POOL_SIZE_256) return 4;
    return -1;  // Too large for pool
}

// Initialize a single pool
static void pool_init(Pool* pool, size_t object_size, size_t objects_per_slab) {
    pool->object_size = object_size;
    pool->objects_per_slab = objects_per_slab;
    pool->slabs = NULL;
    pool->free_list = NULL;
    pool->total_allocated = 0;
    pool->total_free = 0;
    pool->total_used = 0;
}

// Allocate a new slab for a pool
static PoolSlab* pool_alloc_slab(Pool* pool) {
    size_t slab_data_size = pool->object_size * pool->objects_per_slab;
    PoolSlab* slab = (PoolSlab*)malloc(sizeof(PoolSlab) + slab_data_size);
    if (!slab) {
        fprintf(stderr, "ERROR: Failed to allocate pool slab (%zu bytes)\n",
                sizeof(PoolSlab) + slab_data_size);
        return NULL;
    }

    slab->next = pool->slabs;
    slab->object_size = pool->object_size;
    slab->num_objects = pool->objects_per_slab;
    slab->num_free = pool->objects_per_slab;

    // Build free list from slab objects
    for (size_t i = 0; i < pool->objects_per_slab; i++) {
        uint8_t* obj = &slab->data[i * pool->object_size];
        PoolFreeNode* node = (PoolFreeNode*)obj;
        node->next = pool->free_list;
        pool->free_list = node;
    }

    pool->slabs = slab;
    pool->total_allocated += slab_data_size;
    pool->total_free += pool->objects_per_slab;

    return slab;
}

PoolManager* pool_manager_create(void) {
    PoolManager* mgr = (PoolManager*)malloc(sizeof(PoolManager));
    if (!mgr) {
        fprintf(stderr, "FATAL: Failed to allocate PoolManager\n");
        abort();
    }

    // Initialize size classes
    mgr->size_classes[0] = POOL_SIZE_16;
    mgr->size_classes[1] = POOL_SIZE_32;
    mgr->size_classes[2] = POOL_SIZE_64;
    mgr->size_classes[3] = POOL_SIZE_128;
    mgr->size_classes[4] = POOL_SIZE_256;

    // Initialize pools
    pool_init(&mgr->pools[0], POOL_SIZE_16,  POOL_OBJECTS_PER_SLAB_16);
    pool_init(&mgr->pools[1], POOL_SIZE_32,  POOL_OBJECTS_PER_SLAB_32);
    pool_init(&mgr->pools[2], POOL_SIZE_64,  POOL_OBJECTS_PER_SLAB_64);
    pool_init(&mgr->pools[3], POOL_SIZE_128, POOL_OBJECTS_PER_SLAB_128);
    pool_init(&mgr->pools[4], POOL_SIZE_256, POOL_OBJECTS_PER_SLAB_256);

    return mgr;
}

void* pool_alloc(PoolManager* mgr, size_t size) {
    int idx = pool_get_size_class(size);
    if (idx < 0) {
        return NULL;  // Too large, use arena/malloc
    }

    Pool* pool = &mgr->pools[idx];

    // If free list empty, allocate new slab
    if (pool->free_list == NULL) {
        if (!pool_alloc_slab(pool)) {
            return NULL;
        }
    }

    // Pop from free list (O(1)!)
    PoolFreeNode* node = pool->free_list;
    pool->free_list = node->next;
    pool->total_free--;
    pool->total_used++;

    return (void*)node;
}

void pool_free(PoolManager* mgr, void* ptr, size_t size) {
    if (!ptr) return;

    int idx = pool_get_size_class(size);
    if (idx < 0) {
        return;  // Not a pool object
    }

    Pool* pool = &mgr->pools[idx];

    // Push to free list (O(1)!)
    PoolFreeNode* node = (PoolFreeNode*)ptr;
    node->next = pool->free_list;
    pool->free_list = node;
    pool->total_free++;
    pool->total_used--;
}

void pool_manager_destroy(PoolManager* mgr) {
    if (!mgr) return;

    // Free all slabs in all pools
    for (int i = 0; i < POOL_NUM_SIZES; i++) {
        Pool* pool = &mgr->pools[i];
        PoolSlab* slab = pool->slabs;
        while (slab) {
            PoolSlab* next = slab->next;
            free(slab);
            slab = next;
        }
    }

    free(mgr);
}

//=============================================================================
// INITIALIZATION & CLEANUP
//=============================================================================

MorphContextV2* morph_mem_init(MorphMemConfig config) {
    MorphContextV2* ctx = (MorphContextV2*)malloc(sizeof(MorphContextV2));
    if (!ctx) {
        fprintf(stderr, "FATAL: Failed to allocate MorphContextV2\n");
        abort();
    }

    // Initialize context
    memset(ctx, 0, sizeof(MorphContextV2));
    ctx->config = config;

    // Initialize statistics
    memset(&ctx->stats, 0, sizeof(MorphMemStats));

    // Initialize type registry
    memset(ctx->type_names, 0, sizeof(ctx->type_names));

    // Initialize shadow stack for GC roots
    ctx->root_capacity = 1024;  // Start with 1K roots
    ctx->root_stack = (void**)malloc(sizeof(void*) * ctx->root_capacity);
    ctx->root_count = 0;

    // Initialize lock
    pthread_mutex_init(&ctx->lock, NULL);

    // Week 3-4: Initialize allocators based on mode
    if (config.mode == MORPH_MODE_COMPILER) {
        // COMPILER mode: Arena + Pool
        ctx->arena = arena_create(ARENA_BLOCK_SIZE);
        ctx->pool_manager = pool_manager_create();
        ctx->gc_heap = NULL;

        if (config.enable_debug) {
            printf("[MemV2] COMPILER mode initialized:\n");
            printf("  - Arena: %zu MB blocks\n", ARENA_BLOCK_SIZE / (1024 * 1024));
            printf("  - Pool: 5 size classes (16, 32, 64, 128, 256 bytes)\n");
        }
    } else {
        // RUNTIME/VM/SERVER: Generational GC (Week 7+)
        ctx->arena = NULL;
        ctx->pool_manager = NULL;
        ctx->gc_heap = NULL;

        if (config.enable_debug) {
            printf("[MemV2] Mode %d - GC allocator (not implemented yet)\n", config.mode);
        }
    }

    if (config.enable_debug) {
        printf("[MemV2] Initialized - Mode: %d, Heap Hint: %zu MB\n",
               config.mode, config.heap_size_hint / (1024 * 1024));
    }

    return ctx;
}

MorphContextV2* morph_mem_init_auto(void) {
    MorphMemConfig config = morph_mem_detect_config();
    return morph_mem_init(config);
}

void morph_mem_destroy(MorphContextV2* ctx) {
    if (!ctx) return;

    if (ctx->config.enable_debug) {
        printf("[MemV2] Destroying context - Objects allocated: %lu\n",
               ctx->stats.object_count);
    }

    // Week 3-4: Destroy allocators
    if (ctx->config.mode == MORPH_MODE_COMPILER) {
        // Destroy arena
        if (ctx->arena) {
            if (ctx->config.enable_debug) {
                printf("[MemV2] Arena stats - Allocated: %zu KB, Used: %zu KB (%.1f%% utilization)\n",
                       ctx->arena->total_allocated / 1024,
                       ctx->arena->total_used / 1024,
                       ctx->arena->total_allocated > 0 ?
                           (100.0 * ctx->arena->total_used / ctx->arena->total_allocated) : 0.0);
            }
            arena_destroy(ctx->arena);
        }

        // Destroy pool manager
        if (ctx->pool_manager) {
            if (ctx->config.enable_debug) {
                printf("[MemV2] Pool stats:\n");
                for (int i = 0; i < POOL_NUM_SIZES; i++) {
                    Pool* pool = &ctx->pool_manager->pools[i];
                    if (pool->total_allocated > 0) {
                        printf("  - %zu bytes: %zu KB allocated, %zu used, %zu free\n",
                               pool->object_size,
                               pool->total_allocated / 1024,
                               pool->total_used,
                               pool->total_free);
                    }
                }
            }
            pool_manager_destroy(ctx->pool_manager);
        }
    } else {
        // Future: Destroy GC heap (Week 7+)
        if (ctx->gc_heap) {
            // TODO
        }
    }

    // Free shadow stack
    free(ctx->root_stack);

    // Destroy lock
    pthread_mutex_destroy(&ctx->lock);

    // Free context
    free(ctx);
}

//=============================================================================
// ALLOCATION - Week 2 (Arena-based for COMPILER mode)
//=============================================================================

void* morph_mem_alloc(MorphContextV2* ctx, size_t size, uint8_t type_id) {
    pthread_mutex_lock(&ctx->lock);

    if (size > OBJECT_MAX_SIZE) {
        fprintf(stderr, "ERROR: Object size %zu exceeds max %u\n",
                size, OBJECT_MAX_SIZE);
        pthread_mutex_unlock(&ctx->lock);
        return NULL;
    }

    if (type_id > OBJECT_MAX_TYPE_ID) {
        fprintf(stderr, "ERROR: Type ID %u exceeds max %u\n",
                type_id, OBJECT_MAX_TYPE_ID);
        pthread_mutex_unlock(&ctx->lock);
        return NULL;
    }

    ObjectHeader* header = NULL;
    size_t total_size = morph_v2_total_size(size);

    // Week 3-4: Hybrid pool+arena allocation
    if (ctx->config.mode == MORPH_MODE_COMPILER) {
        // Strategy: Small objects (≤256B) use pool, larger use arena
        if (total_size <= POOL_SIZE_256 && ctx->pool_manager) {
            // Try pool allocation (O(1) alloc/free, cache-friendly)
            header = (ObjectHeader*)pool_alloc(ctx->pool_manager, total_size);
            if (header) {
                ctx->stats.pool_bytes += size;
            }
        }

        // Fallback to arena if pool failed or object too large
        if (!header && ctx->arena) {
            header = (ObjectHeader*)arena_alloc(ctx->arena, total_size);
            if (header) {
                ctx->stats.arena_bytes += size;
            }
        }
    } else {
        // RUNTIME/VM/SERVER: Malloc fallback (Week 7+: use generational GC)
        header = (ObjectHeader*)malloc(total_size);
    }

    if (!header) {
        fprintf(stderr, "ERROR: Failed to allocate %zu bytes\n", total_size);
        pthread_mutex_unlock(&ctx->lock);
        return NULL;
    }

    // Initialize header
    memset(header, 0, sizeof(ObjectHeader));
    header->size = size;
    header->type_id = type_id;
    header->marked = 0;
    header->generation = GEN_YOUNG;
    header->flags = 0;
    header->reserved = 0;

    // Update stats
    ctx->stats.total_allocated += size;
    ctx->stats.current_live += size;
    ctx->stats.object_count++;
    ctx->stats.object_count_by_type[type_id]++;
    ctx->stats.bytes_by_type[type_id] += size;

    if (ctx->stats.current_live > ctx->stats.peak_live) {
        ctx->stats.peak_live = ctx->stats.current_live;
    }

    pthread_mutex_unlock(&ctx->lock);

    return morph_v2_get_payload(header);
}

void* morph_mem_alloc_zeroed(MorphContextV2* ctx, size_t size, uint8_t type_id) {
    void* ptr = morph_mem_alloc(ctx, size, type_id);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void morph_mem_free(MorphContextV2* ctx, void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&ctx->lock);

    ObjectHeader* header = morph_v2_get_header(ptr);
    size_t total_size = morph_v2_total_size(header->size);

    // Update stats
    ctx->stats.total_freed += header->size;
    ctx->stats.current_live -= header->size;

    // Week 3-4: Route free to pool if small object
    if (ctx->config.mode == MORPH_MODE_COMPILER && ctx->pool_manager) {
        if (total_size <= POOL_SIZE_256) {
            // Return to pool (O(1)!)
            pool_free(ctx->pool_manager, header, total_size);
            pthread_mutex_unlock(&ctx->lock);
            return;
        }
        // Note: Arena objects are not individually freed (bulk free at end)
    } else {
        // RUNTIME/VM/SERVER: Free individual objects
        free(header);
    }

    pthread_mutex_unlock(&ctx->lock);
}

//=============================================================================
// GC CONTROL (Stub - Week 7+)
//=============================================================================

void morph_mem_gc_collect(MorphContextV2* ctx) {
    // Week 1: No-op
    // Week 7+: Implement generational GC
    if (ctx->config.enable_debug) {
        printf("[MemV2] GC collect (not implemented yet)\n");
    }
}

void morph_mem_gc_push_root(MorphContextV2* ctx, void** ptr) {
    pthread_mutex_lock(&ctx->lock);

    // Expand capacity if needed
    if (ctx->root_count >= ctx->root_capacity) {
        ctx->root_capacity *= 2;
        ctx->root_stack = (void**)realloc(ctx->root_stack,
                                          sizeof(void*) * ctx->root_capacity);
    }

    ctx->root_stack[ctx->root_count++] = ptr;

    pthread_mutex_unlock(&ctx->lock);
}

void morph_mem_gc_pop_roots(MorphContextV2* ctx, int count) {
    pthread_mutex_lock(&ctx->lock);

    if ((size_t)count > ctx->root_count) {
        fprintf(stderr, "ERROR: Trying to pop %d roots but only %zu exist\n",
                count, ctx->root_count);
        count = (int)ctx->root_count;
    }

    ctx->root_count -= count;

    pthread_mutex_unlock(&ctx->lock);
}

//=============================================================================
// TYPE REGISTRY
//=============================================================================

void morph_mem_register_type(MorphContextV2* ctx, uint8_t type_id, const char* name) {
    if (type_id > OBJECT_MAX_TYPE_ID) {
        fprintf(stderr, "ERROR: Type ID %u exceeds max %u\n",
                type_id, OBJECT_MAX_TYPE_ID);
        return;
    }

    pthread_mutex_lock(&ctx->lock);
    ctx->type_names[type_id] = name;
    pthread_mutex_unlock(&ctx->lock);
}

//=============================================================================
// STATISTICS & METRICS
//=============================================================================

MorphMemStats morph_mem_get_stats(MorphContextV2* ctx) {
    pthread_mutex_lock(&ctx->lock);
    MorphMemStats stats = ctx->stats;
    pthread_mutex_unlock(&ctx->lock);
    return stats;
}

void morph_mem_print_stats(MorphContextV2* ctx) {
    MorphMemStats stats = morph_mem_get_stats(ctx);

    printf("=== Morph Memory V2 Statistics ===\n");
    printf("Mode: ");
    switch (ctx->config.mode) {
        case MORPH_MODE_COMPILER: printf("COMPILER\n"); break;
        case MORPH_MODE_RUNTIME:  printf("RUNTIME\n"); break;
        case MORPH_MODE_VM:       printf("VM\n"); break;
        case MORPH_MODE_SERVER:   printf("SERVER\n"); break;
    }

    printf("\nAllocations:\n");
    printf("  Total:    %lu objects (%lu MB)\n",
           stats.object_count, stats.total_allocated / (1024 * 1024));
    printf("  Live:     %lu MB\n", stats.current_live / (1024 * 1024));
    printf("  Peak:     %lu MB\n", stats.peak_live / (1024 * 1024));
    printf("  Freed:    %lu MB\n", stats.total_freed / (1024 * 1024));

    printf("\nGC:\n");
    printf("  Collections: %lu\n", stats.gc_count);
    printf("  Time:        %lu ms\n", stats.gc_time_us / 1000);

    if (stats.gc_count > 0) {
        printf("  Avg Pause:   %lu ms\n", stats.gc_pause_avg_us / 1000);
        printf("  Max Pause:   %lu ms\n", stats.gc_pause_max_us / 1000);
    }

    printf("\nTop Types:\n");
    for (int i = 0; i < 128; i++) {
        if (stats.object_count_by_type[i] > 0) {
            const char* name = ctx->type_names[i] ? ctx->type_names[i] : "unknown";
            printf("  [%3d] %-20s: %lu objects (%lu KB)\n",
                   i, name,
                   stats.object_count_by_type[i],
                   stats.bytes_by_type[i] / 1024);
        }
    }

    printf("===================================\n");
}

void morph_mem_dump_stats(MorphContextV2* ctx, const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "ERROR: Failed to open %s for writing\n", path);
        return;
    }

    MorphMemStats stats = morph_mem_get_stats(ctx);

    fprintf(f, "{\n");
    fprintf(f, "  \"mode\": \"%d\",\n", ctx->config.mode);
    fprintf(f, "  \"total_allocated\": %lu,\n", stats.total_allocated);
    fprintf(f, "  \"total_freed\": %lu,\n", stats.total_freed);
    fprintf(f, "  \"current_live\": %lu,\n", stats.current_live);
    fprintf(f, "  \"peak_live\": %lu,\n", stats.peak_live);
    fprintf(f, "  \"object_count\": %lu,\n", stats.object_count);
    fprintf(f, "  \"gc_count\": %lu,\n", stats.gc_count);
    fprintf(f, "  \"gc_time_us\": %lu,\n", stats.gc_time_us);
    fprintf(f, "  \"gc_pause_max_us\": %lu,\n", stats.gc_pause_max_us);
    fprintf(f, "  \"gc_pause_avg_us\": %lu\n", stats.gc_pause_avg_us);
    fprintf(f, "}\n");

    fclose(f);

    if (ctx->config.enable_debug) {
        printf("[MemV2] Stats dumped to %s\n", path);
    }
}
