/*
 * Morph Memory System V2 - Implementation
 * Week 7-8: Generational GC (Young + Old generations)
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
        // RUNTIME/VM/SERVER: Generational GC (Week 7-8)
        ctx->arena = NULL;
        ctx->pool_manager = NULL;
        ctx->gc_heap = gc_heap_create();

        if (config.enable_debug) {
            printf("[MemV2] Mode %d - Generational GC allocator\n", config.mode);
            printf("  - Young gen: %zu MB (bump-pointer)\n", GC_YOUNG_GEN_SIZE / (1024 * 1024));
            printf("  - Old gen: %zu MB (free-list)\n", GC_OLD_GEN_SIZE / (1024 * 1024));
            printf("  - Promotion age: %d\n", GC_PROMOTION_AGE);
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
        // Destroy GC heap (Week 7-8)
        if (ctx->gc_heap) {
            GCHeap* heap = (GCHeap*)ctx->gc_heap;

            if (ctx->config.enable_debug) {
                printf("[MemV2] GC heap stats:\n");
                printf("  - Young gen: %zu KB used\n", heap->young.used / 1024);
                printf("  - Old gen: %zu KB used\n", heap->old.used / 1024);
                printf("  - Minor GCs: %lu (%lu ms total)\n",
                       heap->total_minor_collections,
                       heap->minor_gc_time_us / 1000);
                printf("  - Major GCs: %lu (%lu ms total)\n",
                       heap->total_major_collections,
                       heap->major_gc_time_us / 1000);
                printf("  - Bytes promoted: %zu KB\n", heap->bytes_promoted / 1024);
                printf("  - Bytes reclaimed: %zu KB\n", heap->bytes_reclaimed / 1024);
            }

            gc_heap_destroy(heap);
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
        // RUNTIME/VM/SERVER: Generational GC (Week 7-8)
        if (ctx->gc_heap) {
            GCHeap* heap = (GCHeap*)ctx->gc_heap;

            // Try young generation first
            void* payload = gc_alloc_young(heap, size, type_id);

            if (!payload) {
                // Young gen full - trigger minor GC and retry
                gc_minor_collect(heap, ctx->root_stack, ctx->root_count);
                ctx->stats.gc_count++;

                payload = gc_alloc_young(heap, size, type_id);

                if (!payload) {
                    // Still full - try old generation
                    payload = gc_alloc_old(heap, size, type_id);

                    if (!payload) {
                        // Old gen full too - trigger major GC and retry
                        gc_major_collect(heap, ctx->root_stack, ctx->root_count);
                        ctx->stats.gc_count++;

                        payload = gc_alloc_old(heap, size, type_id);

                        if (!payload) {
                            fprintf(stderr, "ERROR: GC heap exhausted (young + old full)\n");
                            pthread_mutex_unlock(&ctx->lock);
                            return NULL;
                        }
                    }
                }
            }

            // GC allocation succeeded - payload already has header
            pthread_mutex_unlock(&ctx->lock);
            return payload;
        } else {
            // Fallback to malloc (shouldn't happen if initialized correctly)
            header = (ObjectHeader*)malloc(total_size);
        }
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
// GENERATIONAL GC - Week 7-8
//=============================================================================

#include <time.h>
#include <sys/time.h>

// Helper: Get current time in microseconds
static uint64_t gc_get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
}

//-----------------------------------------------------------------------------
// GC Heap Creation & Destruction
//-----------------------------------------------------------------------------

GCHeap* gc_heap_create(void) {
    GCHeap* heap = (GCHeap*)malloc(sizeof(GCHeap));
    if (!heap) {
        fprintf(stderr, "FATAL: Failed to allocate GCHeap\n");
        abort();
    }

    // Initialize young generation (2MB, bump-pointer allocation)
    heap->young.size = GC_YOUNG_GEN_SIZE;
    heap->young.start = (uint8_t*)malloc(GC_YOUNG_GEN_SIZE);
    if (!heap->young.start) {
        free(heap);
        fprintf(stderr, "FATAL: Failed to allocate young generation (%zu MB)\n",
                GC_YOUNG_GEN_SIZE / (1024 * 1024));
        abort();
    }
    heap->young.end = heap->young.start + GC_YOUNG_GEN_SIZE;
    heap->young.current = heap->young.start;
    heap->young.used = 0;
    heap->young.gc_count = 0;

    // Initialize old generation (32MB, free-list allocation)
    heap->old.size = GC_OLD_GEN_SIZE;
    heap->old.start = (uint8_t*)malloc(GC_OLD_GEN_SIZE);
    if (!heap->old.start) {
        free(heap->young.start);
        free(heap);
        fprintf(stderr, "FATAL: Failed to allocate old generation (%zu MB)\n",
                GC_OLD_GEN_SIZE / (1024 * 1024));
        abort();
    }
    heap->old.end = heap->old.start + GC_OLD_GEN_SIZE;
    heap->old.used = 0;
    heap->old.gc_count = 0;

    // Initialize free list with entire old generation
    heap->old.free_list = (GCFreeNode*)heap->old.start;
    heap->old.free_list->size = GC_OLD_GEN_SIZE;
    heap->old.free_list->next = NULL;

    // Initialize remembered set (empty)
    heap->remembered_set = NULL;

    // Initialize gray stack for marking
    heap->gray_capacity = 1024;
    heap->gray_stack = (void**)malloc(sizeof(void*) * heap->gray_capacity);
    heap->gray_count = 0;

    // Initialize statistics
    heap->total_minor_collections = 0;
    heap->total_major_collections = 0;
    heap->minor_gc_time_us = 0;
    heap->major_gc_time_us = 0;
    heap->bytes_promoted = 0;
    heap->bytes_reclaimed = 0;

    // Week 11: Initialize type descriptors
    for (int i = 0; i < MAX_TYPE_DESCRIPTORS; i++) {
        heap->type_descriptors[i] = NULL;
    }
    heap->num_type_descriptors = 0;

    // Week 11: Initialize heap resizing (default: no auto-resize)
    heap->resize_config = NULL;
    heap->young_target_size = GC_YOUNG_GEN_SIZE;
    heap->old_target_size = GC_OLD_GEN_SIZE;

    // Week 11: Compaction stats
    heap->bytes_compacted = 0;

    return heap;
}

void gc_heap_destroy(GCHeap* heap) {
    if (!heap) return;

    free(heap->young.start);
    free(heap->old.start);
    free(heap->gray_stack);

    // Free remembered set
    RememberedSetEntry* entry = heap->remembered_set;
    while (entry) {
        RememberedSetEntry* next = entry->next;
        free(entry);
        entry = next;
    }

    // Week 11: Free type descriptors
    for (int i = 0; i < MAX_TYPE_DESCRIPTORS; i++) {
        if (heap->type_descriptors[i]) {
            free(heap->type_descriptors[i]);
        }
    }

    // Week 11: Free resize config if allocated
    if (heap->resize_config) {
        free(heap->resize_config);
    }

    free(heap);
}

//-----------------------------------------------------------------------------
// Young Generation Allocation (Bump-Pointer)
//-----------------------------------------------------------------------------

void* gc_alloc_young(GCHeap* heap, size_t size, uint8_t type_id) {
    size_t total_size = morph_v2_total_size(size);

    // Check if young generation has space
    if (heap->young.current + total_size > heap->young.end) {
        // Young generation full - trigger minor GC
        return NULL;  // Caller will trigger GC and retry
    }

    // Allocate header + payload (bump-pointer)
    ObjectHeader* header = (ObjectHeader*)heap->young.current;
    heap->young.current += total_size;
    heap->young.used += total_size;

    // Initialize header
    header->size = size;
    header->type_id = type_id;
    header->marked = 0;
    header->generation = GEN_YOUNG;
    header->flags = 0;
    header->reserved = 0;

    return morph_v2_get_payload(header);
}

//-----------------------------------------------------------------------------
// Old Generation Allocation (First-Fit Free-List)
//-----------------------------------------------------------------------------

void* gc_alloc_old(GCHeap* heap, size_t size, uint8_t type_id) {
    size_t total_size = morph_v2_total_size(size);

    // Search free list for first-fit block
    GCFreeNode** prev_next_ptr = &heap->old.free_list;
    GCFreeNode* node = heap->old.free_list;

    while (node) {
        if (node->size >= total_size) {
            // Found suitable block
            ObjectHeader* header = (ObjectHeader*)node;

            // If block is much larger, split it
            if (node->size >= total_size + sizeof(GCFreeNode) + 64) {
                // Split block: [allocated][remaining free block]
                GCFreeNode* remaining = (GCFreeNode*)((uint8_t*)node + total_size);
                remaining->size = node->size - total_size;
                remaining->next = node->next;
                *prev_next_ptr = remaining;
            } else {
                // Use entire block (no split)
                *prev_next_ptr = node->next;
            }

            // Initialize header
            header->size = size;
            header->type_id = type_id;
            header->marked = 0;
            header->generation = GEN_OLD;
            header->flags = 0;
            header->reserved = 0;

            heap->old.used += total_size;

            return morph_v2_get_payload(header);
        }

        prev_next_ptr = &node->next;
        node = node->next;
    }

    // No suitable block found - need major GC or heap expansion
    return NULL;
}

//-----------------------------------------------------------------------------
// GC Marking Phase
//-----------------------------------------------------------------------------

static void gc_mark_object(GCHeap* heap, void* ptr) {
    if (!ptr) return;

    ObjectHeader* header = morph_v2_get_header(ptr);

    // Already marked? Skip
    if (header->marked) return;

    // Mark object
    header->marked = 1;

    // Push to gray stack for further processing
    if (heap->gray_count >= heap->gray_capacity) {
        heap->gray_capacity *= 2;
        heap->gray_stack = (void**)realloc(heap->gray_stack,
                                            sizeof(void*) * heap->gray_capacity);
    }
    heap->gray_stack[heap->gray_count++] = ptr;
}

static void gc_mark_from_roots(GCHeap* heap, void** roots, size_t root_count) {
    // Mark all root objects
    for (size_t i = 0; i < root_count; i++) {
        void* root_ptr = *roots[i];
        if (root_ptr) {
            gc_mark_object(heap, root_ptr);
        }
    }

    // Process gray stack (trace object graph)
    while (heap->gray_count > 0) {
        void* obj = heap->gray_stack[--heap->gray_count];

        // TODO: Trace pointers within object
        // For now, this is a conservative collector (marks but doesn't trace fields)
        // Week 8+: Add pointer tracing using type information
    }
}

//-----------------------------------------------------------------------------
// GC Sweeping Phase (Young Generation)
//-----------------------------------------------------------------------------

static size_t gc_sweep_young(GCHeap* heap) {
    size_t reclaimed = 0;
    uint8_t* scan = heap->young.start;

    // Compact young generation: Move live objects to front
    uint8_t* dest = heap->young.start;

    while (scan < heap->young.current) {
        ObjectHeader* header = (ObjectHeader*)scan;
        size_t total_size = morph_v2_total_size(header->size);

        if (header->marked) {
            // Object survived - move to front (if needed)
            if (dest != scan) {
                memmove(dest, scan, total_size);
            }

            // Update header metadata
            ObjectHeader* new_header = (ObjectHeader*)dest;
            new_header->marked = 0;  // Clear mark for next GC

            // Increment age
            new_header->flags++;

            dest += total_size;
        } else {
            // Object dead - reclaim
            reclaimed += total_size;
        }

        scan += total_size;
    }

    // Update young generation pointer
    heap->young.current = dest;
    heap->young.used = dest - heap->young.start;

    return reclaimed;
}

//-----------------------------------------------------------------------------
// GC Promotion (Young → Old)
//-----------------------------------------------------------------------------

static size_t gc_promote_survivors(GCHeap* heap) {
    size_t promoted = 0;
    uint8_t* scan = heap->young.start;
    uint8_t* dest = heap->young.start;

    while (scan < heap->young.current) {
        ObjectHeader* header = (ObjectHeader*)scan;
        size_t total_size = morph_v2_total_size(header->size);

        // Promote if object is old enough
        if (header->flags >= GC_PROMOTION_AGE) {
            // Allocate in old generation
            void* old_ptr = gc_alloc_old(heap, header->size, header->type_id);

            if (old_ptr) {
                // Copy object data to old generation
                memcpy(old_ptr, morph_v2_get_payload(header), header->size);
                promoted += total_size;
            } else {
                // Old gen full - keep in young gen
                if (dest != scan) {
                    memmove(dest, scan, total_size);
                }
                dest += total_size;
            }
        } else {
            // Keep in young generation
            if (dest != scan) {
                memmove(dest, scan, total_size);
            }
            dest += total_size;
        }

        scan += total_size;
    }

    heap->young.current = dest;
    heap->young.used = dest - heap->young.start;

    return promoted;
}

//-----------------------------------------------------------------------------
// Minor GC (Young Generation Only)
//-----------------------------------------------------------------------------

void gc_minor_collect(GCHeap* heap, void** roots, size_t root_count) {
    uint64_t start_time = gc_get_time_us();

    // Clear gray stack
    heap->gray_count = 0;

    // Mark phase: Mark reachable objects from roots
    gc_mark_from_roots(heap, roots, root_count);

    // Also mark objects referenced from old generation (remembered set)
    RememberedSetEntry* entry = heap->remembered_set;
    while (entry) {
        void* old_to_young_ptr = *entry->field_addr;
        if (old_to_young_ptr) {
            gc_mark_object(heap, old_to_young_ptr);
        }
        entry = entry->next;
    }

    // Sweep phase: Reclaim unmarked objects
    size_t reclaimed = gc_sweep_young(heap);
    heap->bytes_reclaimed += reclaimed;

    // Promotion: Move old survivors to old generation
    size_t promoted = gc_promote_survivors(heap);
    heap->bytes_promoted += promoted;

    // Update statistics
    heap->young.gc_count++;
    heap->total_minor_collections++;
    uint64_t elapsed = gc_get_time_us() - start_time;
    heap->minor_gc_time_us += elapsed;
}

//-----------------------------------------------------------------------------
// Major GC (Full Heap Collection)
//-----------------------------------------------------------------------------

void gc_major_collect(GCHeap* heap, void** roots, size_t root_count) {
    uint64_t start_time = gc_get_time_us();

    // Clear gray stack
    heap->gray_count = 0;

    // Mark phase: Mark all reachable objects (young + old)
    gc_mark_from_roots(heap, roots, root_count);

    // Sweep young generation
    size_t young_reclaimed = gc_sweep_young(heap);

    // Sweep old generation: Rebuild free list from unmarked objects
    GCFreeNode* new_free_list = NULL;
    uint8_t* scan = heap->old.start;
    uint8_t* end_of_used = heap->old.start + heap->old.used;
    size_t old_reclaimed = 0;

    while (scan < end_of_used) {
        ObjectHeader* header = (ObjectHeader*)scan;
        size_t total_size = morph_v2_total_size(header->size);

        if (!header->marked) {
            // Dead object - add to free list
            GCFreeNode* node = (GCFreeNode*)scan;
            node->size = total_size;
            node->next = new_free_list;
            new_free_list = node;
            old_reclaimed += total_size;
        } else {
            // Live object - clear mark for next GC
            header->marked = 0;
        }

        scan += total_size;
    }

    heap->old.free_list = new_free_list;
    heap->bytes_reclaimed += young_reclaimed + old_reclaimed;

    // Update statistics
    heap->old.gc_count++;
    heap->total_major_collections++;
    uint64_t elapsed = gc_get_time_us() - start_time;
    heap->major_gc_time_us += elapsed;
}

//-----------------------------------------------------------------------------
// Write Barrier (Old Object → Young Object Reference)
//-----------------------------------------------------------------------------

void gc_write_barrier(GCHeap* heap, void* old_obj, void** field_addr) {
    if (!old_obj) return;

    ObjectHeader* header = morph_v2_get_header(old_obj);

    // Only track old → young references
    if (header->generation != GEN_OLD) return;

    void* pointed_obj = *field_addr;
    if (!pointed_obj) return;

    ObjectHeader* pointed_header = morph_v2_get_header(pointed_obj);
    if (pointed_header->generation != GEN_YOUNG) return;

    // Add to remembered set
    RememberedSetEntry* entry = (RememberedSetEntry*)malloc(sizeof(RememberedSetEntry));
    entry->field_addr = field_addr;
    entry->next = heap->remembered_set;
    heap->remembered_set = entry;
}

//=============================================================================
// PRECISE TRACING - Week 11
//=============================================================================

void gc_register_type_descriptor(GCHeap* heap, const TypeDescriptor* desc) {
    if (!heap || !desc) return;
    if (desc->type_id >= MAX_TYPE_DESCRIPTORS) return;

    // Allocate and copy descriptor
    TypeDescriptor* copy = (TypeDescriptor*)malloc(sizeof(TypeDescriptor));
    if (!copy) return;

    memcpy(copy, desc, sizeof(TypeDescriptor));
    heap->type_descriptors[desc->type_id] = copy;

    if (heap->num_type_descriptors <= desc->type_id) {
        heap->num_type_descriptors = desc->type_id + 1;
    }
}

const TypeDescriptor* gc_get_type_descriptor(GCHeap* heap, uint8_t type_id) {
    if (!heap || type_id >= MAX_TYPE_DESCRIPTORS) return NULL;
    return heap->type_descriptors[type_id];
}

// Precise marking using type information
static void gc_mark_object_precise(GCHeap* heap, void* ptr) {
    if (!ptr) return;

    ObjectHeader* header = morph_v2_get_header(ptr);
    if (header->marked) return;

    // Mark object
    header->marked = 1;

    // Push to gray stack
    if (heap->gray_count >= heap->gray_capacity) {
        heap->gray_capacity *= 2;
        heap->gray_stack = (void**)realloc(heap->gray_stack,
                                            sizeof(void*) * heap->gray_capacity);
    }
    heap->gray_stack[heap->gray_count++] = ptr;

    // Get type descriptor for precise tracing
    const TypeDescriptor* desc = gc_get_type_descriptor(heap, header->type_id);
    if (!desc || desc->num_pointers == 0) return;

    // Trace pointer fields only
    uint8_t* obj_data = (uint8_t*)ptr;
    for (uint8_t i = 0; i < desc->num_pointers; i++) {
        uint16_t offset = desc->pointer_offsets[i];
        void** field_ptr = (void**)(obj_data + offset);
        void* child_ptr = *field_ptr;

        if (child_ptr) {
            gc_mark_object_precise(heap, child_ptr);
        }
    }
}

//=============================================================================
// MARK-COMPACT - Week 11
//=============================================================================

// Compute forwarding addresses (first pass)
static void gc_compute_forwarding_addresses(GCHeap* heap) {
    uint8_t* scan = heap->old.start;
    uint8_t* dest = heap->old.start;  // Compaction destination

    while (scan < heap->old.start + heap->old.used) {
        ObjectHeader* header = (ObjectHeader*)scan;
        size_t total_size = morph_v2_total_size(header->size);

        if (header->marked) {
            // Live object - compute forwarding address
            // Store in reserved field (temporary)
            header->reserved = (uint16_t)(dest - scan);  // Offset to new location
            dest += total_size;
        }

        scan += total_size;
    }

    // New used size after compaction
    heap->old.used = dest - heap->old.start;
}

// Update all references after compaction (second pass)
static void gc_update_references_compact(GCHeap* heap, void** roots, size_t root_count) {
    // Update roots
    for (size_t i = 0; i < root_count; i++) {
        void* obj = *roots[i];
        if (!obj) continue;

        ObjectHeader* header = morph_v2_get_header(obj);
        if (header->generation == GEN_OLD && header->marked) {
            // Update root pointer to new location
            uint8_t* old_addr = (uint8_t*)header;
            uint8_t* new_addr = old_addr + header->reserved;
            *roots[i] = morph_v2_get_payload((ObjectHeader*)new_addr);
        }
    }

    // Update inter-object pointers (scan all live objects)
    uint8_t* scan = heap->old.start;
    while (scan < heap->old.start + heap->old.used) {
        ObjectHeader* header = (ObjectHeader*)scan;
        if (!header->marked) {
            scan += morph_v2_total_size(header->size);
            continue;
        }

        // Get type descriptor for precise pointer scanning
        const TypeDescriptor* desc = gc_get_type_descriptor(heap, header->type_id);
        if (desc && desc->num_pointers > 0) {
            void* obj = morph_v2_get_payload(header);
            uint8_t* obj_data = (uint8_t*)obj;

            // Update each pointer field
            for (uint8_t i = 0; i < desc->num_pointers; i++) {
                uint16_t offset = desc->pointer_offsets[i];
                void** field_ptr = (void**)(obj_data + offset);
                void* child_ptr = *field_ptr;

                if (child_ptr) {
                    ObjectHeader* child_header = morph_v2_get_header(child_ptr);
                    if (child_header->generation == GEN_OLD && child_header->marked) {
                        // Update pointer to new location
                        uint8_t* old_child = (uint8_t*)child_header;
                        uint8_t* new_child = old_child + child_header->reserved;
                        *field_ptr = morph_v2_get_payload((ObjectHeader*)new_child);
                    }
                }
            }
        }

        scan += morph_v2_total_size(header->size);
    }
}

// Move live objects to compacted locations (third pass)
static void gc_move_objects(GCHeap* heap) {
    uint8_t* scan = heap->old.start;
    uint8_t* dest = heap->old.start;

    while (scan < heap->old.start + heap->old.used) {
        ObjectHeader* header = (ObjectHeader*)scan;
        size_t total_size = morph_v2_total_size(header->size);

        if (header->marked) {
            // Move object to new location
            if (scan != dest) {
                memmove(dest, scan, total_size);
            }

            // Clear mark and reserved field
            ObjectHeader* new_header = (ObjectHeader*)dest;
            new_header->marked = 0;
            new_header->reserved = 0;

            dest += total_size;
        }

        scan += total_size;
    }

    heap->bytes_compacted += (scan - dest);
}

void gc_compact_old_generation(GCHeap* heap) {
    if (!heap) return;

    uint64_t start_time = gc_get_time_us();

    // Three-pass mark-compact algorithm
    gc_compute_forwarding_addresses(heap);  // Pass 1: Compute new addresses
    // Note: Pass 2 needs roots, done in gc_update_references
    gc_move_objects(heap);                   // Pass 3: Move objects

    // Rebuild free list (single large free block at end)
    heap->old.free_list = NULL;
    if (heap->old.used < heap->old.size) {
        GCFreeNode* free_block = (GCFreeNode*)(heap->old.start + heap->old.used);
        free_block->size = heap->old.size - heap->old.used;
        free_block->next = NULL;
        heap->old.free_list = free_block;
    }

    uint64_t elapsed = gc_get_time_us() - start_time;
    // Track compaction time (for stats)
    (void)elapsed;  // Unused for now
}

void gc_update_references(GCHeap* heap, void** roots, size_t root_count) {
    if (!heap) return;
    gc_update_references_compact(heap, roots, root_count);
}

//=============================================================================
// DYNAMIC HEAP RESIZING - Week 11
//=============================================================================

void gc_resize_young_generation(GCHeap* heap, size_t new_size) {
    if (!heap || new_size == heap->young.size) return;
    if (new_size < heap->young.used) return;  // Can't shrink below used

    // Allocate new young generation
    uint8_t* new_start = (uint8_t*)malloc(new_size);
    if (!new_start) return;

    // Copy live objects to new generation
    memcpy(new_start, heap->young.start, heap->young.used);

    // Free old generation
    free(heap->young.start);

    // Update pointers
    heap->young.start = new_start;
    heap->young.end = new_start + new_size;
    heap->young.current = new_start + heap->young.used;
    heap->young.size = new_size;
}

void gc_resize_old_generation(GCHeap* heap, size_t new_size) {
    if (!heap || new_size == heap->old.size) return;
    if (new_size < heap->old.used) return;  // Can't shrink below used

    // Allocate new old generation
    uint8_t* new_start = (uint8_t*)malloc(new_size);
    if (!new_start) return;

    // Copy old generation to new space
    memcpy(new_start, heap->old.start, heap->old.used);

    // Free old space
    free(heap->old.start);

    // Update pointers
    ptrdiff_t offset = new_start - heap->old.start;
    heap->old.start = new_start;
    heap->old.end = new_start + new_size;
    heap->old.size = new_size;

    // Fix free list pointers
    GCFreeNode* node = heap->old.free_list;
    GCFreeNode* new_free_list = NULL;
    GCFreeNode** prev_next = &new_free_list;

    while (node) {
        GCFreeNode* new_node = (GCFreeNode*)((uint8_t*)node + offset);
        *prev_next = new_node;
        prev_next = &new_node->next;
        node = node->next;
    }

    heap->old.free_list = new_free_list;

    // Add remaining space to free list
    if (new_size > heap->old.used) {
        GCFreeNode* extra = (GCFreeNode*)(new_start + heap->old.used);
        extra->size = new_size - heap->old.used;
        extra->next = heap->old.free_list;
        heap->old.free_list = extra;
    }
}

void gc_auto_resize_heap(GCHeap* heap, const HeapResizeConfig* config) {
    if (!heap || !config) return;

    // Check young generation
    float young_usage = (float)heap->young.used / heap->young.size;
    if (young_usage > config->grow_threshold && heap->young.size < config->max_young_size) {
        // Grow young generation
        size_t new_size = heap->young.size * 2;
        if (new_size > config->max_young_size) {
            new_size = config->max_young_size;
        }
        gc_resize_young_generation(heap, new_size);
    } else if (young_usage < config->shrink_threshold && heap->young.size > config->min_young_size) {
        // Shrink young generation
        size_t new_size = heap->young.size / 2;
        if (new_size < config->min_young_size) {
            new_size = config->min_young_size;
        }
        gc_resize_young_generation(heap, new_size);
    }

    // Check old generation
    float old_usage = (float)heap->old.used / heap->old.size;
    if (old_usage > config->grow_threshold && heap->old.size < config->max_old_size) {
        // Grow old generation
        size_t new_size = heap->old.size * 2;
        if (new_size > config->max_old_size) {
            new_size = config->max_old_size;
        }
        gc_resize_old_generation(heap, new_size);
    } else if (old_usage < config->shrink_threshold && heap->old.size > config->min_old_size) {
        // Shrink old generation
        size_t new_size = heap->old.size / 2;
        if (new_size < config->min_old_size) {
            new_size = config->min_old_size;
        }
        gc_resize_old_generation(heap, new_size);
    }
}

//=============================================================================
// GC CONTROL (Week 7-8 Implementation)
//=============================================================================

void morph_mem_gc_collect(MorphContextV2* ctx) {
    if (!ctx->gc_heap) {
        // No GC heap (COMPILER mode) - no-op
        if (ctx->config.enable_debug) {
            printf("[MemV2] GC collect skipped (COMPILER mode)\n");
        }
        return;
    }

    pthread_mutex_lock(&ctx->lock);

    GCHeap* heap = (GCHeap*)ctx->gc_heap;

    // Decide: Minor or Major GC?
    if (heap->old.used > heap->old.size * 0.8) {
        // Old generation >80% full - trigger major GC
        if (ctx->config.enable_debug) {
            printf("[MemV2] Triggering MAJOR GC (old gen %zu/%zu MB)\n",
                   heap->old.used / (1024 * 1024),
                   heap->old.size / (1024 * 1024));
        }

        gc_major_collect(heap, ctx->root_stack, ctx->root_count);

        // Update stats
        ctx->stats.gc_count++;
        ctx->stats.gc_time_us += heap->major_gc_time_us;

        if (ctx->config.enable_debug) {
            printf("[MemV2] Major GC complete - Reclaimed %zu KB\n",
                   heap->bytes_reclaimed / 1024);
        }
    } else {
        // Trigger minor GC (young generation only)
        if (ctx->config.enable_debug) {
            printf("[MemV2] Triggering MINOR GC (young gen %zu/%zu MB)\n",
                   heap->young.used / (1024 * 1024),
                   heap->young.size / (1024 * 1024));
        }

        gc_minor_collect(heap, ctx->root_stack, ctx->root_count);

        // Update stats
        ctx->stats.gc_count++;
        ctx->stats.gc_time_us += heap->minor_gc_time_us;

        if (ctx->config.enable_debug) {
            printf("[MemV2] Minor GC complete - Reclaimed %zu KB, Promoted %zu KB\n",
                   heap->bytes_reclaimed / 1024,
                   heap->bytes_promoted / 1024);
        }
    }

    pthread_mutex_unlock(&ctx->lock);
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
