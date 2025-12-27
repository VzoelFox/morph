#ifndef MORPH_H
#define MORPH_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

// --- Constants ---
#define GC_THRESHOLD (8UL * 1024 * 1024 * 1024) // 8GB (optimized for 32GB RAM)
#define GC_MIN_THRESHOLD (512 * 1024 * 1024) // 512MB
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
    int is_ref_type;        // Pointer semantics
} MorphTypeInfo;

// Object header for GC
typedef struct ObjectHeader {
    struct ObjectHeader* next;       // Link in GC list
    MorphTypeInfo* type;
    uint8_t flags;
    uint64_t last_access;           // Timestamp untuk LRU daemon
    uint64_t swap_id;               // Offset di .z file
    size_t size;                    // Size in bytes

    struct MphPage* page;           // Page tempat object ini berada

    // Free-list linking (for this size class)
    struct ObjectHeader* free_next;
    struct ObjectHeader* free_prev;

    // Per-page free-list linking (untuk partial pages)
    struct ObjectHeader* page_free_next;
    struct ObjectHeader* page_free_prev;
} ObjectHeader;

#define MEMORY_CANARY_MAGIC 0xDEADBEEF
#define MEMORY_FREE_MAGIC   0xFEEDFACE

// Page = 4KB container
#define PAGE_SIZE (4 * 1024)

typedef struct MphPage {
    void* memory;                     // malloc(PAGE_SIZE)
    size_t used_bytes;                // Berapa bytes yang sudah dialokasi
    struct MphPage* next;             // Link to next page

    // Per-page free list
    ObjectHeader* free_head;
    size_t free_count;                // Berapa objects free di page ini
} MphPage;

typedef struct MorphContext {
    ObjectHeader* gc_head;          // Start GC list
    ObjectHeader* gc_tail;          // End GC list (untuk append cepat)
    size_t allocated_bytes;
    size_t next_gc_threshold;

    // Stack untuk GC marking
    void** stack_top;               // Simpan stack pointer untuk scan roots

    // LRU daemon state
    int daemon_running;
    pthread_t daemon_thread;
    pthread_mutex_t alloc_mutex;

    // Page management
    MphPage* page_head;             // Linked list of pages
    size_t page_count;              // Jumlah pages

    // Free-list per size class (powers of 2)
    ObjectHeader* free_lists[32];   // free_lists[k] = free blocks of size ~2^k
    size_t free_counts[32];
} MorphContext;

// --- Core API ---
MorphContext* mph_context_new();
void* mph_alloc(MorphContext* ctx, size_t bytes, MorphTypeInfo* type);
void mph_gc_collect(MorphContext* ctx);

// --- String Type ---
typedef struct MorphString {
    char* data;
    size_t length;
    size_t capacity;
} MorphString;

// String functions (declarations only, defined in runtime.c)
MorphString* mph_string_new(MorphContext* ctx, const char* data);
MorphString* mph_string_concat(MorphContext* ctx, MorphString* a, MorphString* b);
MorphString* mph_string_from_int(MorphContext* ctx, mph_int value);
mph_int mph_string_index(MorphContext* ctx, MorphString* s, MorphString* sub);
MorphString* mph_string_trim(MorphContext* ctx, MorphString* s, MorphString* cut);
MorphString* mph_string_substr(MorphContext* ctx, MorphString* s, mph_int start, mph_int len);
int mph_string_eq(MorphString* a, MorphString* b);

// --- Array Type ---
typedef struct MorphArray {
    void** elements;
    size_t length;
    size_t capacity;
    MorphTypeInfo* element_type;
} MorphArray;

MorphArray* mph_array_new(MorphContext* ctx, MorphTypeInfo* elem_type);
void mph_array_push(MorphContext* ctx, MorphArray* arr, void* elem);
void* mph_array_get(MorphArray* arr, mph_int index);

// Split function
MorphArray* mph_string_split(MorphContext* ctx, MorphString* s, MorphString* sep);

// --- Map Type ---
typedef struct MorphMapBucket {
    MorphString* key;
    void* value;
    struct MorphMapBucket* next;
} MorphMapBucket;

typedef struct MorphMap {
    MorphMapBucket** buckets;
    size_t bucket_count;
    size_t size;
    MorphTypeInfo* key_type;
    MorphTypeInfo* value_type;
} MorphMap;

MorphMap* mph_map_new(MorphContext* ctx, MorphTypeInfo* key_type, MorphTypeInfo* value_type);
void mph_map_set(MorphContext* ctx, MorphMap* map, MorphString* key, void* value);
void* mph_map_get(MorphMap* map, MorphString* key);

// --- Error Type (Error as Value) ---
typedef struct MorphError {
    MorphString* message;
    int code;
} MorphError;

MorphError* mph_error_new(MorphContext* ctx, const char* msg, int code);

// --- Result Type ---
typedef struct MorphResult {
    void* value;
    MorphError* error;
    int is_error;
} MorphResult;

MorphResult mph_result_ok(void* value);
MorphResult mph_result_err(MorphError* error);

// --- Morphroutines (Green Threads) ---
typedef struct MorphUnit {
    void* (*func)(void*);
    void* arg;
    void* result;
    int done;
    pthread_t thread;
} MorphUnit;

MorphUnit* mph_spawn(void* (*func)(void*), void* arg);
void* mph_wait(MorphUnit* unit);

// --- Channel (untuk komunikasi antar morphroutines) ---
typedef struct MorphChannel {
    void** buffer;
    size_t capacity;
    size_t size;
    size_t read_pos;
    size_t write_pos;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} MorphChannel;

MorphChannel* mph_channel_new(size_t capacity);
void mph_channel_send(MorphChannel* ch, void* value);
void* mph_channel_receive(MorphChannel* ch);

// --- Native Functions (linked dari generated C) ---
void mph_native_print(MorphContext* ctx, MorphString* s);
void mph_native_print_int(MorphContext* ctx, mph_int n);
mph_int mph_native_readInt();
MorphString* mph_native_readString(MorphContext* ctx);

#endif // MORPH_H
