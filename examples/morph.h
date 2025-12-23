#ifndef MORPH_H
#define MORPH_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

// --- Constants ---
// Removed Drawer/Tray constants as we are moving to Malloc + GC

// --- Types ---
typedef int64_t mph_int;
typedef double  mph_float;
typedef int     mph_bool;
typedef void    mph_void;

// --- Memory Structures (Tiered Memory / GC) ---

// Forward declarations
struct MorphTypeInfo;
struct ObjectHeader;

typedef struct MorphTypeInfo {
    const char* name;
    size_t size;            // Size of payload
    int num_pointers;       // How many pointers in payload
    size_t* pointer_offsets; // Offsets of pointers relative to payload start
} MorphTypeInfo;

typedef struct ObjectHeader {
    struct ObjectHeader* next;  // Global GC list
    MorphTypeInfo* type;        // RTTI
    uint8_t flags;              // 0x1: Marked, 0x2: Swapped
    uint32_t last_access;       // For LRU Eviction
} ObjectHeader;

// Memory Context now tracks the Heap Head
typedef struct MorphContext {
    ObjectHeader* heap_head;    // Linked list of all allocated objects
    size_t allocated_bytes;     // Total bytes currently allocated
    size_t next_gc_threshold;   // When to trigger next GC
    void* scheduler;            // Placeholder
} MorphContext;

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
// NOTE: These are the "Payloads" after the ObjectHeader

typedef struct MorphString {
    char* data;    // Pointer to C string (Malloc-ed separately for now, or embedded?)
                   // Optimization: data usually points to separate malloc block.
                   // The String struct itself is on Heap.
    size_t length;
} MorphString;

typedef struct MorphArray {
    void* data;    // Pointer to data block
    size_t length;
    size_t capacity;
    size_t element_size;
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
    MorphKeyKind key_kind;
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


// --- API ---

// Memory
void mph_init_memory(MorphContext* ctx);
void mph_destroy_memory(MorphContext* ctx);

// New Allocator: requires TypeInfo for GC
// Generic allocations (arrays/maps internals) can pass NULL type_info,
// but they won't be scanned by GC (must be leaves or managed manually).
void* mph_alloc(MorphContext* ctx, size_t size, MorphTypeInfo* type_info);

// GC
void mph_gc_collect(MorphContext* ctx);

// Strings
MorphString* mph_string_new(MorphContext* ctx, const char* literal);
MorphString* mph_string_concat(MorphContext* ctx, MorphString* a, MorphString* b);
mph_bool mph_string_eq(MorphString* a, MorphString* b);

// Arrays
MorphArray* mph_array_new(MorphContext* ctx, size_t capacity, size_t element_size);
void* mph_array_at(MorphContext* ctx, MorphArray* arr, mph_int index);

// Maps
MorphMap* mph_map_new(MorphContext* ctx, MorphKeyKind kind);
void mph_map_set(MorphContext* ctx, MorphMap* map, void* key, void* value);
void* mph_map_get(MorphContext* ctx, MorphMap* map, void* key);
void mph_map_delete(MorphContext* ctx, MorphMap* map, void* key);
mph_int mph_map_len(MorphContext* ctx, MorphMap* map);

// Interfaces
void* mph_assert_type(MorphContext* ctx, MorphInterface iface, mph_int expected_id);

// Closures
MorphClosure* mph_closure_new(MorphContext* ctx, void* fn, void* env, int env_size);

// Debug
void mph_native_print_int(MorphContext* ctx, mph_int n);

// Concurrency
MorphChannel* mph_channel_new(MorphContext* ctx);
void mph_channel_destroy(MorphContext* ctx, MorphChannel* c);
void mph_channel_send(MorphContext* ctx, MorphChannel* c, mph_int val);
mph_int mph_channel_recv(MorphContext* ctx, MorphChannel* c);
void mph_thread_spawn(MorphEntryFunction fn, void* arg);

// Entry Point
void morph_entry_point(MorphContext* ctx);

#endif // MORPH_H
