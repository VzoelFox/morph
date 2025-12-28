#ifndef MORPH_H
#define MORPH_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

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

typedef struct ObjectHeader {
    struct ObjectHeader* next;  // Global GC list
    MorphTypeInfo* type;        // RTTI
    uint8_t flags;              // 0x1: Marked, 0x2: Swapped
    uint64_t last_access;       // Timestamp (ms) for LRU Eviction
    uint64_t swap_id;           // ID for swap file
    size_t size;                // Payload size
    struct MphPage* page;       // Owning page (fast lookup)
    struct ObjectHeader* free_next;
    struct ObjectHeader* free_prev;
    struct ObjectHeader* page_free_next;
    struct ObjectHeader* page_free_prev;
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

// Shadow Stack for Roots
typedef struct StackRoot {
    void** ptr; // Pointer to the local variable (which is a pointer to object)
    struct StackRoot* next;
} StackRoot;

// Iterative GC Mark Stack
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
    int daemon_running;
    pthread_mutex_t memory_lock; // Lock for heap access (Daemon vs Main)
    uint64_t last_alloc_time;

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
void mph_destroy_memory(MorphContext* ctx);

// Shadow Stack
void mph_gc_push_root(MorphContext* ctx, void** ptr);
void mph_gc_pop_roots(MorphContext* ctx, int count);

// Allocator
void* mph_alloc(MorphContext* ctx, size_t size, MorphTypeInfo* type_info);

// GC & Tiered Memory
void mph_gc_collect(MorphContext* ctx);
void mph_start_daemon(MorphContext* ctx);
void mph_stop_daemon(MorphContext* ctx);
void mph_swap_in(MorphContext* ctx, void* obj); // Ensure object is in RAM

// Strings
MorphString* mph_string_new(MorphContext* ctx, const char* literal);
MorphString* mph_string_concat(MorphContext* ctx, MorphString* a, MorphString* b);
MorphString* mph_string_substring(MorphContext* ctx, MorphString* s, mph_int start, mph_int end);
mph_bool mph_string_eq(MorphContext* ctx, MorphString* a, MorphString* b);

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
