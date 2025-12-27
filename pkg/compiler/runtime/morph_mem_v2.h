/*
 * Morph Memory System V2 - Production Header
 * Week 1 Implementation: ObjectHeader + Config + Basic Types
 *
 * Design: See MEMORY_ARCHITECTURE_V2.md
 * Roadmap: See MEMORY_V2_ROADMAP.md
 *
 * Status: Week 9-10 - GC Optimization (Precise Tracing, Compaction, Resizing)
 */

#ifndef MORPH_MEM_V2_H
#define MORPH_MEM_V2_H

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

//=============================================================================
// CONFIGURATION & CONSTANTS
//=============================================================================

// Workload modes (determines allocation strategy)
typedef enum {
    MORPH_MODE_COMPILER,    // Compilation: Arena-based, no GC
    MORPH_MODE_RUNTIME,     // Balanced: Generational GC
    MORPH_MODE_VM,          // VM: Advanced GC with JIT cache
    MORPH_MODE_SERVER       // Server: Bounded heap, predictable pauses
} MorphWorkloadMode;

// Memory configuration
typedef struct {
    MorphWorkloadMode mode;
    size_t heap_size_hint;      // Expected max heap (0 = auto-detect)
    size_t gc_threshold;         // GC trigger threshold (0 = disabled)
    uint8_t gc_pause_target_ms;  // Target GC pause time (0 = best effort)
    uint8_t enable_generational; // Use generational GC
    uint8_t enable_compaction;   // Use mark-compact GC
    uint8_t enable_metrics;      // Collect statistics
    uint8_t enable_debug;        // Debug logging
} MorphMemConfig;

// Preset configurations (defined in .c file)
extern const MorphMemConfig MORPH_CONFIG_COMPILER;
extern const MorphMemConfig MORPH_CONFIG_RUNTIME;
extern const MorphMemConfig MORPH_CONFIG_VM;
extern const MorphMemConfig MORPH_CONFIG_SERVER;

//=============================================================================
// OBJECT HEADER V2 (16 bytes - down from V1's 72 bytes!)
//=============================================================================

typedef struct ObjectHeader {
    // Bitfield layout (4 bytes total):
    // - size: 24 bits (0-16,777,215 bytes = 16MB max)
    // - type_id: 7 bits (0-127 types)
    // - marked: 1 bit (GC mark flag)
    uint32_t size : 24;
    uint32_t type_id : 7;
    uint32_t marked : 1;

    // Additional metadata (4 bytes total):
    uint8_t  generation;     // 0=young, 1=old, 2=permanent, 255=large
    uint8_t  flags;          // Custom flags (pinned, finalizable, etc)
    uint16_t reserved;       // Future use / alignment padding
} ObjectHeader;

// Verify size at compile time
_Static_assert(sizeof(ObjectHeader) == 16, "ObjectHeader must be exactly 16 bytes");

// Object constants
#define OBJECT_MAX_SIZE ((1 << 24) - 1)  // 16MB - 1 byte
#define OBJECT_MAX_TYPE_ID ((1 << 7) - 1) // 127
#define IS_LARGE_OBJECT(size) ((size) > OBJECT_MAX_SIZE)

// Generation constants
#define GEN_YOUNG 0
#define GEN_OLD 1
#define GEN_PERMANENT 2
#define GEN_LARGE 255

// Alignment helper
#define ALIGN_UP(n, align) (((n) + (align) - 1) & ~((align) - 1))
#define OBJECT_ALIGNMENT 8

//=============================================================================
// HELPER FUNCTIONS
//=============================================================================

// Get header from payload pointer
static inline ObjectHeader* morph_v2_get_header(void* payload) {
    return ((ObjectHeader*)payload) - 1;
}

// Get payload from header pointer
static inline void* morph_v2_get_payload(ObjectHeader* header) {
    return (void*)(header + 1);
}

// Get total size (header + payload + alignment)
static inline size_t morph_v2_total_size(size_t payload_size) {
    return ALIGN_UP(sizeof(ObjectHeader) + payload_size, OBJECT_ALIGNMENT);
}

//=============================================================================
// ARENA ALLOCATOR (Week 2)
//=============================================================================

// Arena block size (2MB default - balances memory waste vs syscall overhead)
#define ARENA_BLOCK_SIZE (2 * 1024 * 1024)  // 2MB

// Arena block structure (linked list of memory blocks)
typedef struct ArenaBlock {
    struct ArenaBlock* next;  // Next block in chain
    size_t capacity;          // Total capacity of this block
    size_t used;              // Bytes used so far
    uint8_t data[];           // Flexible array member for actual data
} ArenaBlock;

// Arena allocator (bump-pointer allocation)
typedef struct {
    ArenaBlock* current;      // Current allocation block
    ArenaBlock* first;        // First block (for iteration/reset)
    size_t block_size;        // Size of each block (default ARENA_BLOCK_SIZE)
    size_t total_allocated;   // Total bytes allocated (stats)
    size_t total_used;        // Total bytes used (stats)
} Arena;

// Arena API
Arena* arena_create(size_t block_size);
void* arena_alloc(Arena* arena, size_t size);
void* arena_alloc_aligned(Arena* arena, size_t size, size_t alignment);
void arena_reset(Arena* arena);  // Reset without freeing blocks
void arena_destroy(Arena* arena); // Free all blocks

//=============================================================================
// POOL ALLOCATOR (Week 3-4)
//=============================================================================

// Pool size classes (power-of-2 for alignment)
#define POOL_SIZE_16   16
#define POOL_SIZE_32   32
#define POOL_SIZE_64   64
#define POOL_SIZE_128  128
#define POOL_SIZE_256  256
#define POOL_NUM_SIZES 5

// Objects per slab (tuned for 64KB slabs)
#define POOL_OBJECTS_PER_SLAB_16   4000   // 64KB / 16
#define POOL_OBJECTS_PER_SLAB_32   2000   // 64KB / 32
#define POOL_OBJECTS_PER_SLAB_64   1000   // 64KB / 64
#define POOL_OBJECTS_PER_SLAB_128  500    // 64KB / 128
#define POOL_OBJECTS_PER_SLAB_256  250    // 64KB / 256

// Free list node (embedded in free objects)
typedef struct PoolFreeNode {
    struct PoolFreeNode* next;
} PoolFreeNode;

// Pool slab (chunk of pre-allocated objects)
typedef struct PoolSlab {
    struct PoolSlab* next;      // Next slab in chain
    size_t object_size;         // Size of each object
    size_t num_objects;         // Total objects in slab
    size_t num_free;            // Free objects remaining
    uint8_t data[];             // Flexible array for objects
} PoolSlab;

// Pool allocator (one per size class)
typedef struct {
    size_t object_size;         // Size class
    size_t objects_per_slab;    // Objects per slab
    PoolSlab* slabs;            // Linked list of slabs
    PoolFreeNode* free_list;    // Free list head
    size_t total_allocated;     // Total bytes allocated (stats)
    size_t total_free;          // Free objects count (stats)
    size_t total_used;          // Used objects count (stats)
} Pool;

// Pool allocator manager (manages all size classes)
typedef struct {
    Pool pools[POOL_NUM_SIZES]; // One pool per size class
    size_t size_classes[POOL_NUM_SIZES];
} PoolManager;

// Pool API
PoolManager* pool_manager_create(void);
void* pool_alloc(PoolManager* mgr, size_t size);
void pool_free(PoolManager* mgr, void* ptr, size_t size);
void pool_manager_destroy(PoolManager* mgr);

// Helper: Get pool index for size
int pool_get_size_class(size_t size);

//=============================================================================
// GENERATIONAL GC (Week 7-8)
//=============================================================================

// Forward declarations for Week 9-10 features
typedef struct TypeDescriptor TypeDescriptor;
typedef struct HeapResizeConfig HeapResizeConfig;

// GC generation sizes
#define GC_YOUNG_GEN_SIZE (2 * 1024 * 1024)   // 2MB young generation
#define GC_OLD_GEN_SIZE   (32 * 1024 * 1024)  // 32MB old generation

// GC promotion threshold (age in minor collections)
#define GC_PROMOTION_AGE 3

// Remembered set entry (for write barriers)
typedef struct RememberedSetEntry {
    void** field_addr;                    // Address of pointer field in old object
    struct RememberedSetEntry* next;      // Next entry in set
} RememberedSetEntry;

// Free list node for old generation
typedef struct GCFreeNode {
    size_t size;                          // Size of this free block
    struct GCFreeNode* next;              // Next free block
} GCFreeNode;

// Young generation (bump-pointer allocation)
typedef struct {
    uint8_t* start;                       // Start of young gen memory
    uint8_t* end;                         // End of young gen memory
    uint8_t* current;                     // Current allocation pointer
    size_t size;                          // Total size (2MB)
    size_t used;                          // Bytes used
    uint32_t gc_count;                    // Number of minor collections
} YoungGen;

// Old generation (free-list allocation)
typedef struct {
    uint8_t* start;                       // Start of old gen memory
    uint8_t* end;                         // End of old gen memory
    size_t size;                          // Total size (32MB)
    size_t used;                          // Bytes used
    GCFreeNode* free_list;                // Free list head
    uint32_t gc_count;                    // Number of major collections
} OldGen;

// Generational GC heap
typedef struct {
    YoungGen young;                       // Young generation
    OldGen old;                           // Old generation
    RememberedSetEntry* remembered_set;   // Write barrier tracking

    // GC metadata
    void** gray_stack;                    // Gray objects during marking
    size_t gray_count;
    size_t gray_capacity;

    // Week 9-10: Type descriptors for precise tracing
    TypeDescriptor* type_descriptors[MAX_TYPE_DESCRIPTORS];  // Type registry
    uint8_t num_type_descriptors;                             // Number registered

    // Week 9-10: Heap resizing
    HeapResizeConfig* resize_config;      // Resizing configuration
    size_t young_target_size;             // Target young gen size
    size_t old_target_size;               // Target old gen size

    // Statistics
    uint64_t total_minor_collections;
    uint64_t total_major_collections;
    uint64_t minor_gc_time_us;
    uint64_t major_gc_time_us;
    uint64_t bytes_promoted;
    uint64_t bytes_reclaimed;
    uint64_t bytes_compacted;              // Week 9-10: Compaction stats
} GCHeap;

// GC API (internal - used by morph_mem_* functions)
GCHeap* gc_heap_create(void);
void gc_heap_destroy(GCHeap* heap);
void* gc_alloc_young(GCHeap* heap, size_t size, uint8_t type_id);
void* gc_alloc_old(GCHeap* heap, size_t size, uint8_t type_id);
void gc_minor_collect(GCHeap* heap, void** roots, size_t root_count);
void gc_major_collect(GCHeap* heap, void** roots, size_t root_count);
void gc_write_barrier(GCHeap* heap, void* old_obj, void** field_addr);

//=============================================================================
// PRECISE TRACING (Week 9-10)
//=============================================================================

// Type descriptor for precise GC tracing
struct TypeDescriptor {
    uint8_t type_id;                  // Type ID (matches ObjectHeader.type_id)
    const char* name;                 // Type name (for debugging)
    size_t size;                      // Object size in bytes
    uint8_t num_pointers;             // Number of pointer fields
    uint16_t pointer_offsets[16];     // Offsets of pointer fields (max 16)
};

// Type descriptor registry (max 128 types)
#define MAX_TYPE_DESCRIPTORS 128

// Register type descriptor for precise tracing
void gc_register_type_descriptor(GCHeap* heap, const TypeDescriptor* desc);

// Get type descriptor by type_id
const TypeDescriptor* gc_get_type_descriptor(GCHeap* heap, uint8_t type_id);

//=============================================================================
// MARK-COMPACT GC (Week 9-10)
//=============================================================================

// Compact old generation (eliminate fragmentation)
void gc_compact_old_generation(GCHeap* heap);

// Update pointers after compaction
void gc_update_references(GCHeap* heap, void** roots, size_t root_count);

//=============================================================================
// DYNAMIC HEAP RESIZING (Week 9-10)
//=============================================================================

// Heap resizing configuration
struct HeapResizeConfig {
    size_t min_young_size;      // Minimum young gen size (default 2MB)
    size_t max_young_size;      // Maximum young gen size (default 8MB)
    size_t min_old_size;        // Minimum old gen size (default 32MB)
    size_t max_old_size;        // Maximum old gen size (default 128MB)
    float grow_threshold;       // Grow when >X% full (default 0.8)
    float shrink_threshold;     // Shrink when <X% used (default 0.3)
};

// Resize young generation
void gc_resize_young_generation(GCHeap* heap, size_t new_size);

// Resize old generation
void gc_resize_old_generation(GCHeap* heap, size_t new_size);

// Auto-adjust heap sizes based on usage
void gc_auto_resize_heap(GCHeap* heap, const HeapResizeConfig* config);

//=============================================================================
// MEMORY STATISTICS
//=============================================================================

typedef struct {
    // Allocation stats
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t current_live;
    uint64_t peak_live;

    // GC stats
    uint64_t gc_count;
    uint64_t gc_time_us;
    uint64_t gc_pause_max_us;
    uint64_t gc_pause_avg_us;

    // Object stats
    uint64_t object_count;
    uint64_t object_count_by_type[128];
    uint64_t bytes_by_type[128];

    // Heap breakdown
    uint64_t arena_bytes;
    uint64_t pool_bytes;
    uint64_t heap_bytes;
} MorphMemStats;

//=============================================================================
// MEMORY CONTEXT V2 (Forward Declaration)
//=============================================================================

// Full definition in morph_mem_v2.c
typedef struct MorphContextV2 MorphContextV2;

//=============================================================================
// PUBLIC API - Initialization
//=============================================================================

// Initialize memory system with given config
MorphContextV2* morph_mem_init(MorphMemConfig config);

// Initialize with auto-detected config (based on available RAM)
MorphContextV2* morph_mem_init_auto(void);

// Destroy memory system and free all resources
void morph_mem_destroy(MorphContextV2* ctx);

//=============================================================================
// PUBLIC API - Allocation
//=============================================================================

// Allocate object of given size and type
void* morph_mem_alloc(MorphContextV2* ctx, size_t size, uint8_t type_id);

// Allocate zero-initialized object
void* morph_mem_alloc_zeroed(MorphContextV2* ctx, size_t size, uint8_t type_id);

// Free object (optional - for non-GC modes like COMPILER)
void morph_mem_free(MorphContextV2* ctx, void* ptr);

//=============================================================================
// PUBLIC API - GC Control
//=============================================================================

// Manually trigger garbage collection
void morph_mem_gc_collect(MorphContextV2* ctx);

// Register root pointer for GC (push onto shadow stack)
void morph_mem_gc_push_root(MorphContextV2* ctx, void** ptr);

// Unregister N root pointers (pop from shadow stack)
void morph_mem_gc_pop_roots(MorphContextV2* ctx, int count);

//=============================================================================
// PUBLIC API - Type Registry
//=============================================================================

// Register type name for debugging/metrics
void morph_mem_register_type(MorphContextV2* ctx, uint8_t type_id, const char* name);

//=============================================================================
// PUBLIC API - Statistics & Metrics
//=============================================================================

// Get current memory statistics
MorphMemStats morph_mem_get_stats(MorphContextV2* ctx);

// Print statistics to stdout
void morph_mem_print_stats(MorphContextV2* ctx);

// Dump statistics to file (JSON format)
void morph_mem_dump_stats(MorphContextV2* ctx, const char* path);

//=============================================================================
// UTILITY FUNCTIONS
//=============================================================================

// Get available system RAM in bytes
size_t morph_mem_get_available_ram(void);

// Auto-detect optimal config based on environment
MorphMemConfig morph_mem_detect_config(void);

#endif // MORPH_MEM_V2_H
