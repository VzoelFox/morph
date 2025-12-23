#ifndef MORPH_H
#define MORPH_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

// --- Constants ---
#define DRAWER_SIZE (128 * 1024)
#define TRAY_SIZE   (64 * 1024)
#define MAX_DRAWERS 1024

// --- Types ---
typedef int64_t mph_int;
typedef double  mph_float;
typedef int     mph_bool;
typedef void    mph_void;

// --- Memory Structures ---

typedef struct Tray {
    uint8_t* start;
    uint8_t* current;
    uint8_t* end;
} Tray;

typedef struct Drawer {
    uint32_t id;
    Tray primary;
    // secondary reserved for GC
    uint8_t* data; // Allocated region (128KB)
} Drawer;

typedef struct Cabinet {
    Drawer* drawers[MAX_DRAWERS];
    int active_drawer_index;
    int drawer_count;
} Cabinet;

typedef struct MorphContext {
    Cabinet* cabinet;
    void* scheduler; // Placeholder
} MorphContext;

typedef void (*MorphEntryFunction)(MorphContext* ctx, void* arg);

typedef struct MorphChannel {
    pthread_mutex_t lock;
    pthread_cond_t cond_send; // Wait if full
    pthread_cond_t cond_recv; // Wait if empty

    // Circular Buffer for Ints
    int64_t* buffer;
    int capacity;
    int count;
    int head;
    int tail;
} MorphChannel;

// --- Object Structures ---

typedef struct MorphString {
    const char* data;
    size_t length;
} MorphString;

typedef struct MorphArray {
    void* data;
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
    void* instance;      // Pointer to the struct data
    void** vtable;       // Array of function pointers
    mph_int type_id;     // For Type Assertion (Unique ID)
} MorphInterface;

typedef struct MorphClosure {
    void* function; // Generic function pointer (MorphClosureFunc)
    void* env;      // Pointer to captured environment struct
} MorphClosure;

// Standard generic function signature for closures:
// Returns void*, takes (Context, Env, ...Params)
typedef void* (*MorphClosureFunc)(MorphContext*, void*, ...);


// --- API ---

// Memory
void mph_init_memory(MorphContext* ctx);
void mph_destroy_memory(MorphContext* ctx);
void* mph_alloc(MorphContext* ctx, size_t size);
Drawer* mph_new_drawer(Cabinet* cab);

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
MorphClosure* mph_closure_new(MorphContext* ctx, void* fn, void* env);

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
