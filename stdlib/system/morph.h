#ifndef MORPH_H
#define MORPH_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>

// --- Constants ---
#define GC_THRESHOLD (64 * 1024 * 1024) // 64MB
#define GC_MIN_THRESHOLD (8 * 1024 * 1024) // 8MB
#define DAEMON_SLEEP_MS 100
#define SWAP_AGE_THRESHOLD_SEC 10

// --- Types ---
typedef int64_t mph_int;
typedef double  mph_float;
typedef int     mph_bool;
typedef void    mph_void;

// --- Memory Structures (Tiered Memory / GC) ---

// Forward declarations
struct MorphTypeInfo;
struct ObjectHeader;
struct MphPage;
typedef struct MorphContext MorphContext; // Pre-declare context

typedef struct MorphTypeInfo {
    const char* name;
    size_t size;            // Size of payload
    int num_pointers;       // How many pointers in payload
    size_t* pointer_offsets; // Offsets of pointers relative to payload start
    void (*mark_fn)(MorphContext*, void*); // Custom marking function (e.g. for Containers)
} MorphTypeInfo;

// --- Memory Safety & Validation ---
#define MEMORY_CANARY_MAGIC 0xDEADBEEF
#define MEMORY_FREE_MAGIC   0xFEEDFACE

typedef struct MemoryTracker {
    void* address;
    size_t size;
    uint64_t alloc_time;
    char zone_id[16];
    struct MemoryTracker* next;
} MemoryTracker;

typedef struct ObjectHeader {
    struct ObjectHeader* next;  // Global GC list
    MorphTypeInfo* type;        // RTTI
    uint8_t flags;              // 0x1: Marked, 0x2: Swapped, 0x4: Valid
    uint64_t last_access;       // Timestamp (ms) for LRU Eviction
    uint64_t swap_id;           // ID for swap file
    size_t size;                // Payload size
    struct MphPage* page;       // Owning page (fast lookup)
    struct ObjectHeader* free_next;
    struct ObjectHeader* free_prev;
    struct ObjectHeader* page_free_next;
    struct ObjectHeader* page_free_prev;
    char zone_id[16];           // Zone ownership
    int required_clearance;     // Minimum clearance to access
    uint32_t canary_start;      // Memory corruption detection
    // Payload follows here
    // uint32_t canary_end;     // At end of payload
} ObjectHeader;

typedef struct MphPage {
    void* start_addr;
    size_t used_offset;
    int flags; // 0=RAM, 1=DISK
    uint64_t last_access;
    struct MphPage* next;
    uint64_t swap_id;
    size_t live_bytes;
    size_t size;
    ObjectHeader* free_list;
} MphPage;

// --- Forward Declarations ---
typedef enum SwapResult SwapResult;
typedef struct SwapRequest SwapRequest;
typedef struct SwapSlot SwapSlot;
typedef struct SwapPool SwapPool;

// --- Swap Pool System ---
#define SWAP_POOL_SIZE (256 * 1024 * 1024) // 256MB pool
#define SWAP_SLOT_SIZE (64 * 1024)          // 64KB slots
#define SWAP_SLOTS_COUNT (SWAP_POOL_SIZE / SWAP_SLOT_SIZE)

typedef enum SwapResult {
    SWAP_SUCCESS = 0,
    SWAP_ERROR_NULL_POOL,
    SWAP_ERROR_INVALID_SLOT,
    SWAP_ERROR_ZONE_MISMATCH,
    SWAP_ERROR_QUEUE_FULL,
    SWAP_ERROR_THREAD_FAILED,
    SWAP_ERROR_IO_FAILED
} SwapResult;

typedef struct SwapSlot {
    uint64_t slot_id;
    mph_bool is_free;
    void* data;
    size_t size;
    char zone_id[16];
    struct SwapSlot* next_free;
} SwapSlot;

typedef struct SwapRequest {
    enum { SWAP_IN, SWAP_OUT, CHECKPOINT } type;
    void* page_addr;
    size_t size;
    uint64_t slot_id;
    char zone_id[16];
    SwapResult* result;        // Error as value
    pthread_mutex_t* done_mutex;
    pthread_cond_t* done_cond;
    mph_bool completed;
    struct SwapRequest* next;
} SwapRequest;

typedef struct SwapPool {
    void* pool_base;           // Pre-allocated pool
    SwapSlot* slots;           // Slot metadata
    SwapSlot* free_head;       // Free list
    pthread_mutex_t pool_lock;
    size_t free_count;
    
    // Async Worker Thread
    pthread_t async_thread;
    int async_running;
    SwapRequest* request_queue_head;
    SwapRequest* request_queue_tail;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;
    size_t queue_size;
    size_t max_queue_size;     // Safety limit
    
    // Error tracking
    size_t error_count;
    SwapResult last_error;
} SwapPool;

// --- .z System Logging ---
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;

typedef struct ZLogEntry {
    uint64_t timestamp_ms;
    LogLevel level;
    char zone_id[16];
    char worker_id[16];
    SwapResult error_code;
    char message[256];
    char function[64];
    int line;
    struct ZLogEntry* next;
} ZLogEntry;

typedef struct ZLogger {
    FILE* z_file;
    pthread_mutex_t log_lock;
    ZLogEntry* log_head;
    size_t log_count;
    size_t max_entries;
    mph_bool system_only;
} ZLogger;

// Shadow Stack for Roots
typedef struct StackRoot {
    void** ptr; // Pointer to the local variable (which is a pointer to object)
    struct StackRoot* next;
} StackRoot;

#define MARK_STACK_BLOCK_SIZE 1024

typedef struct MarkStackBlock {
    void* items[MARK_STACK_BLOCK_SIZE];
    size_t count;
    struct MarkStackBlock* next;
    struct MarkStackBlock* prev;
} MarkStackBlock;

typedef struct MarkStack {
    MarkStackBlock* head;
    MarkStackBlock* current;
    size_t count;
} MarkStack;

struct MorphContext {
    ObjectHeader* heap_head;    // Linked list of all allocated objects
    size_t allocated_bytes;     // Total bytes currently allocated
    size_t next_gc_threshold;   // When to trigger next GC

    StackRoot* stack_top;       // Top of Shadow Stack
    MarkStack mark_stack;       // Stack for iterative GC marking

    MphPage* page_head;         // Head of page list (per-context)
    MphPage* current_alloc_page; // Current page used for allocations
    pthread_mutex_t page_lock;  // Lock for page list/swap operations
    ObjectHeader* free_list;    // Reusable freed objects (exact size)

    pthread_t daemon_thread;    // GC/Swap Daemon
    volatile int daemon_running; // Atomic flag
    pthread_mutex_t gc_lock;    // Dedicated GC lock (separate from memory_lock)
    pthread_mutex_t memory_lock; // Lock for heap access (Daemon vs Main)
    uint64_t last_alloc_time;

    // Memory Zone Isolation
    char zone_id[16];           // "ZONE_A", "ZONE_B", etc
    int clearance_level;        // Security clearance (1, 2, 3...)
    void* zone_base_addr;       // Base address for this zone
    size_t zone_size_limit;     // Hard limit for zone memory
    size_t zone_allocated;      // Current zone usage

    // Robust Swap System
    SwapPool* swap_pool;        // Dedicated swap pool
    mph_bool swap_enabled;      // Enable/disable swapping
    
    // Worker Checkpoint
    char worker_id[16];         // Worker identifier
    void* checkpoint_data;      // Serialized worker state
    size_t checkpoint_size;     // Size of checkpoint

    // .z System Logging
    ZLogger* z_logger;          // System-only logger
    mph_bool logging_enabled;   // Enable/disable logging

    // Memory Safety & Validation
    MemoryTracker* alloc_tracker; // Track all allocations
    pthread_mutex_t tracker_lock; // Lock for allocation tracking
    size_t total_allocations;     // Total allocation count
    size_t active_allocations;    // Currently active allocations
    mph_bool debug_mode;          // Enable debug checks

    void* scheduler;            // Placeholder
};

typedef void (*MorphEntryFunction)(MorphContext* ctx, void* arg);

typedef struct MorphChannel {
    pthread_mutex_t lock;
    pthread_cond_t cond_send;
    pthread_cond_t cond_recv;

    int64_t* buffer;
    int capacity;
    int count;
    int head;
    int tail;
} MorphChannel;

// --- Object Structures ---

typedef struct MorphString {
    char* data;    // Pointer to C string
    size_t length;
} MorphString;

typedef struct MorphArray {
    void* data;    // Pointer to data block
    size_t length;
    size_t capacity;
    size_t element_size;
    mph_bool elements_are_pointers; // Flag for GC
} MorphArray;

typedef enum {
    MPH_KEY_INT,
    MPH_KEY_STRING,
    MPH_KEY_PTR
} MorphKeyKind;

typedef struct MorphMapEntry {
    void* key;
    void* value;
    mph_bool occupied;
    mph_bool deleted;
} MorphMapEntry;

typedef struct MorphMap {
    MorphMapEntry* entries;
    size_t capacity;
    size_t count;
    size_t deleted_count;
    MorphKeyKind key_kind;
    mph_bool values_are_pointers; // Flag for GC
} MorphMap;

typedef struct MorphInterface {
    void* instance;
    void** vtable;
    mph_int type_id;
} MorphInterface;

typedef struct MorphClosure {
    void* function;
    void* env;
} MorphClosure;

typedef void* (*MorphClosureFunc)(MorphContext*, void*, ...);

typedef struct MorphError {
    MorphString* message;
} MorphError;

// --- API ---

// Memory
void mph_init_memory(MorphContext* ctx);
void mph_init_memory_zone(MorphContext* ctx, const char* zone_id, int clearance, size_t zone_limit);
void mph_destroy_memory(MorphContext* ctx);

// Zone Security
mph_bool mph_can_access_memory(MorphContext* ctx, void* ptr);
void* mph_alloc_secure(MorphContext* ctx, size_t size, MorphTypeInfo* type_info, int required_clearance);

// Shadow Stack
void mph_gc_push_root(MorphContext* ctx, void** ptr);
void mph_gc_pop_roots(MorphContext* ctx, int count);

// Allocator
void* mph_alloc(MorphContext* ctx, size_t size, MorphTypeInfo* type_info);
void* mph_alloc_shared(size_t size, MorphTypeInfo* type_info);

// GC & Tiered Memory
void mph_gc_collect(MorphContext* ctx);
void mph_start_daemon(MorphContext* ctx);
void mph_stop_daemon(MorphContext* ctx);
void mph_swap_in(MorphContext* ctx, void* obj); // Ensure object is in RAM

// Swap Pool
SwapPool* mph_swap_pool_create(size_t pool_size);
void mph_swap_pool_destroy(SwapPool* pool);
uint64_t mph_swap_pool_alloc_slot(SwapPool* pool, const char* zone_id, size_t size);
void mph_swap_pool_free_slot(SwapPool* pool, uint64_t slot_id);

// Async Swap Operations (Error as Value)
SwapResult mph_swap_async_request(SwapPool* pool, int type, void* addr, size_t size, const char* zone_id);
SwapResult mph_swap_async_wait(SwapPool* pool, SwapRequest* req);
void* mph_swap_worker_thread(void* arg);
SwapResult mph_swap_start_worker(SwapPool* pool);
SwapResult mph_swap_stop_worker(SwapPool* pool);

// Worker Checkpoint
void mph_worker_checkpoint_save(MorphContext* ctx);
mph_bool mph_worker_checkpoint_restore(MorphContext* ctx);
void mph_worker_set_id(MorphContext* ctx, const char* worker_id);

// .z System Logging (System-Only Access)
ZLogger* mph_z_logger_create(const char* zone_id);
void mph_z_logger_destroy(ZLogger* logger);
void mph_z_log(ZLogger* logger, LogLevel level, const char* zone_id, const char* worker_id, 
               SwapResult error_code, const char* function, int line, const char* format, ...);
void mph_z_log_error_detail(ZLogger* logger, const char* zone_id, const char* worker_id,
                           SwapResult error_code, const char* operation, const char* reason);
mph_bool mph_z_is_system_accessible(void);

#define Z_LOG_DEBUG(logger, zone, worker, code, ...) \
    mph_z_log(logger, LOG_LEVEL_DEBUG, zone, worker, code, __FUNCTION__, __LINE__, __VA_ARGS__)
#define Z_LOG_ERROR(logger, zone, worker, code, ...) \
    mph_z_log(logger, LOG_LEVEL_ERROR, zone, worker, code, __FUNCTION__, __LINE__, __VA_ARGS__)
#define Z_LOG_FATAL(logger, zone, worker, code, ...) \
    mph_z_log(logger, LOG_LEVEL_FATAL, zone, worker, code, __FUNCTION__, __LINE__, __VA_ARGS__)

// Memory Safety & Validation API
mph_bool mph_is_valid_object(MorphContext* ctx, void* obj);
mph_bool mph_validate_memory_integrity(MorphContext* ctx);
void mph_track_allocation(MorphContext* ctx, void* obj, size_t size);
void mph_untrack_allocation(MorphContext* ctx, void* obj);
void mph_memory_barrier(void);
void mph_set_debug_mode(MorphContext* ctx, mph_bool enabled);
size_t mph_get_memory_stats(MorphContext* ctx, size_t* active, size_t* total);

// Strings
MorphString* mph_string_new(MorphContext* ctx, const char* literal);
MorphString* mph_string_concat(MorphContext* ctx, MorphString* a, MorphString* b);
mph_bool mph_string_eq(MorphContext* ctx, MorphString* a, MorphString* b);
MorphString* mph_string_substring(MorphContext* ctx, MorphString* s, mph_int start, mph_int end);

// Arrays
MorphArray* mph_array_new(MorphContext* ctx, size_t capacity, size_t element_size, mph_bool is_ptr);
void* mph_array_at(MorphContext* ctx, MorphArray* arr, mph_int index);
MorphArray* mph_array_concat(MorphContext* ctx, MorphArray* a, MorphArray* b);

// Maps
MorphMap* mph_map_new(MorphContext* ctx, MorphKeyKind kind, mph_bool val_is_ptr);
void mph_map_set(MorphContext* ctx, MorphMap* map, void* key, void* value);
void* mph_map_get(MorphContext* ctx, MorphMap* map, void* key);
void mph_map_delete(MorphContext* ctx, MorphMap* map, void* key);
mph_int mph_map_len(MorphContext* ctx, MorphMap* map);

// Interfaces
void* mph_assert_type(MorphContext* ctx, MorphInterface iface, mph_int expected_id);

// Closures
MorphClosure* mph_closure_new(MorphContext* ctx, void* fn, void* env, int env_size);

// Error
MorphError* mph_error_new(MorphContext* ctx, MorphString* msg);

// Time
mph_int mph_time_Now(MorphContext* ctx, void* _env);
void mph_time_Sleep(MorphContext* ctx, void* _env, mph_int ms);

// Debug
void mph_native_print_int(MorphContext* ctx, mph_int n);
void mph_native_print_error(MorphContext* ctx, MorphError* err);

// Concurrency
MorphChannel* mph_channel_new(MorphContext* ctx);
void mph_channel_destroy(MorphContext* ctx, MorphChannel* c);
void mph_channel_send(MorphContext* ctx, MorphChannel* c, mph_int val);
mph_int mph_channel_recv(MorphContext* ctx, MorphChannel* c);
void mph_thread_spawn(MorphEntryFunction fn, void* arg);

// Entry Point
void morph_entry_point(MorphContext* ctx);

#endif // MORPH_H
