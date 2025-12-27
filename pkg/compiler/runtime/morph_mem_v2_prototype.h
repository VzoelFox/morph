/*
 * Morph Memory System V2 - Prototype Header
 *
 * This is a prototype implementation of the V2 memory architecture.
 * See MEMORY_ARCHITECTURE_V2.md for full design.
 *
 * Key improvements over V1:
 * - 16-byte ObjectHeader (vs 72 bytes) = 78% reduction
 * - Mode-based allocation strategies
 * - Generational GC for runtime
 * - Arena allocator for compilation
 * - No swap/daemon overhead
 */

#ifndef MORPH_MEM_V2_H
#define MORPH_MEM_V2_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

//=============================================================================
// CONFIGURATION
//=============================================================================

typedef enum {
    MORPH_MODE_COMPILER,    // Short-lived, high-churn (arena-based, no GC)
    MORPH_MODE_RUNTIME,     // Balanced workload (generational GC)
    MORPH_MODE_VM,          // Long-lived, low-churn (advanced GC)
    MORPH_MODE_SERVER       // Continuous, bounded memory
} MorphWorkloadMode;

typedef struct MorphMemConfig {
    MorphWorkloadMode mode;
    size_t heap_size_hint;      // Expected max heap size (0 = auto)
    size_t gc_threshold;         // GC trigger threshold (0 = disabled)
    uint8_t gc_pause_target_ms;  // Target pause time (0 = best effort)
    uint8_t enable_generational; // Use generational GC
    uint8_t enable_compaction;   // Use mark-compact
    uint8_t enable_metrics;      // Collect statistics
    uint8_t enable_debug;        // Debug logging
} MorphMemConfig;

// Preset configurations
extern MorphMemConfig MORPH_CONFIG_COMPILER;
extern MorphMemConfig MORPH_CONFIG_RUNTIME;
extern MorphMemConfig MORPH_CONFIG_VM;
extern MorphMemConfig MORPH_CONFIG_SERVER;

//=============================================================================
// OBJECT HEADER V2 (16 bytes - down from 72!)
//=============================================================================

typedef struct ObjectHeader {
    uint32_t size : 24;        // 16MB max object (0-16,777,215 bytes)
    uint32_t type_id : 7;      // 128 types (0-127)
    uint32_t marked : 1;       // GC mark bit
    uint8_t  generation;       // 0=young, 1=old, 2=permanent, 255=large
    uint8_t  flags;            // Custom flags (pinned, finalized, etc)
    uint16_t reserved;         // Future use / alignment
} ObjectHeader;

// Extended header for free lists / special cases (24 bytes total)
typedef struct ObjectHeaderExt {
    ObjectHeader base;
    struct ObjectHeaderExt* next;  // For linked lists
    uint64_t metadata;             // App-specific data
} ObjectHeaderExt;

static_assert(sizeof(ObjectHeader) == 16, "ObjectHeader must be 16 bytes");
static_assert(sizeof(ObjectHeaderExt) == 24, "ObjectHeaderExt must be 24 bytes");

// Helper macros
#define ALIGN_UP(n, align) (((n) + (align) - 1) & ~((align) - 1))
#define OBJECT_MAX_SIZE ((1 << 24) - 1)  // 16MB - 1
#define IS_LARGE_OBJECT(size) ((size) > OBJECT_MAX_SIZE)

//=============================================================================
// ARENA ALLOCATOR (Compiler Mode)
//=============================================================================

#define ARENA_DEFAULT_SIZE (2 * 1024 * 1024)  // 2MB per arena

typedef struct Arena {
    void* buffer;              // Memory block
    size_t capacity;           // Total size
    size_t offset;             // Current allocation point
    struct Arena* next;        // Next arena in chain
} Arena;

typedef struct ArenaAllocator {
    Arena* current;            // Current arena being filled
    Arena* free_list;          // Reusable arenas
    size_t default_size;       // Default arena size
    size_t total_allocated;    // Total bytes allocated
    size_t arena_count;        // Number of arenas
} ArenaAllocator;

// API
ArenaAllocator* arena_allocator_new(size_t default_size);
void* arena_alloc(ArenaAllocator* aa, size_t size);
void arena_reset(ArenaAllocator* aa);  // Reset all arenas (keep memory)
void arena_destroy(ArenaAllocator* aa); // Free all memory

//=============================================================================
// POOL ALLOCATOR (Fixed-Size Objects)
//=============================================================================

#define POOL_BLOCK_SIZE 4096
#define POOL_MAX_OBJECT_SIZE 512

typedef struct PoolBlock {
    void* memory;              // Memory block
    uint32_t* free_bitmap;     // Bitmap of free slots
    uint16_t free_count;       // Number of free slots
    uint16_t next_hint;        // Hint for fast allocation
    struct PoolBlock* next;    // Next block in chain
} PoolBlock;

typedef struct Pool {
    size_t object_size;        // Size of objects in this pool
    size_t objects_per_block;  // Objects per block
    PoolBlock* blocks;         // List of blocks
    PoolBlock* current;        // Block with free slots
    size_t total_objects;      // Total objects allocated
} Pool;

// API
Pool* pool_new(size_t object_size);
void* pool_alloc(Pool* p);
void pool_free(Pool* p, void* ptr);
void pool_destroy(Pool* p);

//=============================================================================
// GENERATIONAL HEAP (Runtime Mode)
//=============================================================================

#define YOUNG_GEN_SIZE (2 * 1024 * 1024)   // 2MB young generation
#define OLD_GEN_SIZE   (32 * 1024 * 1024)  // 32MB old generation
#define GEN_PROMOTION_THRESHOLD 3           // Promote after 3 GCs

typedef struct GenerationalHeap {
    // Young generation (bump pointer + evacuation)
    void* young_start;
    void* young_current;       // Allocation point
    void* young_end;
    size_t young_size;

    // Old generation (mark-sweep)
    ObjectHeaderExt* old_objects;
    size_t old_size;
    size_t old_capacity;

    // Remembered set (old → young pointers)
    void** remembered_set;
    size_t remembered_count;
    size_t remembered_capacity;

    // Large objects (>1MB)
    ObjectHeaderExt* large_objects;

    // Statistics
    uint64_t young_gc_count;
    uint64_t old_gc_count;
    uint64_t promoted_objects;
    uint64_t promoted_bytes;
} GenerationalHeap;

// API
GenerationalHeap* gen_heap_new(size_t young_size, size_t old_size);
void* gen_heap_alloc(GenerationalHeap* gh, size_t size, uint8_t type_id);
void gen_heap_gc_minor(GenerationalHeap* gh);  // Young gen only
void gen_heap_gc_major(GenerationalHeap* gh);  // Full collection
void gen_heap_destroy(GenerationalHeap* gh);

// Write barrier (call when old object points to young object)
void gen_heap_write_barrier(GenerationalHeap* gh, void* old_obj, void* young_obj);

//=============================================================================
// UNIFIED MEMORY CONTEXT V2
//=============================================================================

typedef struct MorphContextV2 {
    MorphMemConfig config;

    // Mode-specific allocators (only one active at a time)
    union {
        ArenaAllocator* arena;      // COMPILER mode
        GenerationalHeap* gen_heap; // RUNTIME/VM mode
    };

    // Shared pools for common objects
    Pool* pools[8];                 // Size classes: 32, 64, 128, 256, ...
    uint8_t pool_count;

    // Shadow stack (roots for GC)
    void** stack_roots;
    size_t stack_count;
    size_t stack_capacity;

    // Statistics
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t current_live;
    uint64_t peak_live;
    uint64_t gc_count;
    uint64_t gc_time_us;

    // Object type registry
    const char* type_names[128];

    // Metrics buffer (optional)
    void* metrics_buffer;

    // Lock for thread safety (if needed)
    pthread_mutex_t lock;
} MorphContextV2;

//=============================================================================
// PUBLIC API
//=============================================================================

// Initialization
MorphContextV2* morph_mem_init(MorphMemConfig config);
MorphContextV2* morph_mem_init_auto(void);  // Auto-detect config
void morph_mem_destroy(MorphContextV2* ctx);

// Allocation
void* morph_alloc(MorphContextV2* ctx, size_t size, uint8_t type_id);
void* morph_alloc_zeroed(MorphContextV2* ctx, size_t size, uint8_t type_id);
void morph_free(MorphContextV2* ctx, void* ptr);  // Optional (for non-GC modes)

// GC control
void morph_gc_collect(MorphContextV2* ctx);       // Trigger GC
void morph_gc_push_root(MorphContextV2* ctx, void** ptr);
void morph_gc_pop_roots(MorphContextV2* ctx, int count);

// Type registration
void morph_register_type(MorphContextV2* ctx, uint8_t type_id, const char* name);

// Statistics
typedef struct MorphMemStats {
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t current_live;
    uint64_t peak_live;
    uint64_t gc_count;
    uint64_t gc_time_us;
    uint64_t gc_pause_max_us;
    size_t arena_count;
    size_t arena_bytes;
    size_t pool_bytes;
    size_t heap_bytes;
} MorphMemStats;

MorphMemStats morph_mem_get_stats(MorphContextV2* ctx);
void morph_mem_print_stats(MorphContextV2* ctx);
void morph_mem_dump_stats(MorphContextV2* ctx, const char* path);

//=============================================================================
// UTILITY FUNCTIONS
//=============================================================================

// Get available RAM (for auto-tuning)
size_t morph_get_available_ram(void);

// Auto-detect optimal config
MorphMemConfig morph_detect_config(void);

// Helper to get header from payload pointer
static inline ObjectHeader* morph_get_header(void* ptr) {
    return ((ObjectHeader*)ptr) - 1;
}

// Helper to get payload from header
static inline void* morph_get_payload(ObjectHeader* header) {
    return (void*)(header + 1);
}

//=============================================================================
// PRESET CONFIGURATIONS (Implementation in .c file)
//=============================================================================

// Compiler mode: Arena-based, no GC, fast compilation
static const MorphMemConfig MORPH_CONFIG_COMPILER_INIT = {
    .mode = MORPH_MODE_COMPILER,
    .heap_size_hint = 256 * 1024 * 1024,  // 256MB
    .gc_threshold = 0,                     // No GC
    .gc_pause_target_ms = 0,
    .enable_generational = 0,
    .enable_compaction = 0,
    .enable_metrics = 0,
    .enable_debug = 0
};

// Runtime mode: Generational GC, balanced
static const MorphMemConfig MORPH_CONFIG_RUNTIME_INIT = {
    .mode = MORPH_MODE_RUNTIME,
    .heap_size_hint = 512 * 1024 * 1024,  // 512MB
    .gc_threshold = 64 * 1024 * 1024,     // 64MB
    .gc_pause_target_ms = 10,
    .enable_generational = 1,
    .enable_compaction = 0,
    .enable_metrics = 1,
    .enable_debug = 0
};

// VM mode: Advanced GC, long-running
static const MorphMemConfig MORPH_CONFIG_VM_INIT = {
    .mode = MORPH_MODE_VM,
    .heap_size_hint = 2ULL * 1024 * 1024 * 1024,  // 2GB
    .gc_threshold = 128 * 1024 * 1024,             // 128MB
    .gc_pause_target_ms = 5,
    .enable_generational = 1,
    .enable_compaction = 1,
    .enable_metrics = 1,
    .enable_debug = 0
};

#endif // MORPH_MEM_V2_H
