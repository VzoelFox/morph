#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "morph.h"

// --- Tiered Memory & GC Implementation ---

#define GC_THRESHOLD (1024 * 1024) // 1MB for testing
#define FLAG_MARKED 0x01
#define FLAG_SWAPPED 0x02

// Simple TypeInfos for built-ins
MorphTypeInfo mph_ti_string = { "string", sizeof(MorphString), 0, NULL };
size_t string_ptr_offsets[] = { offsetof(MorphString, data) };
MorphTypeInfo mph_ti_string_real = { "string", sizeof(MorphString), 1, string_ptr_offsets };
MorphTypeInfo mph_ti_raw = { "raw", 0, 0, NULL };

// --- Memory ---

void mph_init_memory(MorphContext* ctx) {
    ctx->heap_head = NULL;
    ctx->allocated_bytes = 0;
    ctx->next_gc_threshold = GC_THRESHOLD;
    ctx->scheduler = NULL;

    // Shadow Stack
    ctx->stack_capacity = STACK_INITIAL_CAPACITY;
    ctx->stack_top = 0;
    ctx->shadow_stack = (void***)malloc(sizeof(void**) * ctx->stack_capacity);
}

void mph_destroy_memory(MorphContext* ctx) {
    free(ctx->shadow_stack);
    ObjectHeader* current = ctx->heap_head;
    while (current != NULL) {
        ObjectHeader* next = current->next;
        free(current);
        current = next;
    }
    ctx->heap_head = NULL;
}

// Shadow Stack API
void mph_gc_push_root(MorphContext* ctx, void* ptr_to_ptr) {
    if (ctx->stack_top >= ctx->stack_capacity) {
        ctx->stack_capacity *= 2;
        ctx->shadow_stack = (void***)realloc(ctx->shadow_stack, sizeof(void**) * ctx->stack_capacity);
    }
    ctx->shadow_stack[ctx->stack_top++] = (void**)ptr_to_ptr;
}

void mph_gc_pop_roots(MorphContext* ctx, int count) {
    ctx->stack_top -= count;
    if (ctx->stack_top < 0) {
        printf("Panic: Shadow stack underflow\n");
        exit(1);
    }
}

// Helper to get header from payload
ObjectHeader* mph_get_header(void* obj) {
    return ((ObjectHeader*)obj) - 1;
}

void mph_gc_mark(void* obj) {
    if (!obj) return;
    ObjectHeader* header = mph_get_header(obj);

    if (header->flags & FLAG_MARKED) return;
    header->flags |= FLAG_MARKED;

    MorphTypeInfo* type = header->type;
    if (!type) return;

    // Recursively mark children
    char* payload = (char*)obj;
    for (int i = 0; i < type->num_pointers; i++) {
        size_t offset = type->pointer_offsets[i];
        void** child_ptr = (void**)(payload + offset);
        if (*child_ptr) {
            mph_gc_mark(*child_ptr);
        }
    }
    // Special handling for Arrays with ElementType
    if (type->name && strcmp(type->name, "array") == 0) {
        MorphArray* arr = (MorphArray*)obj;
        if (arr->element_type) {
            void** data_ptr = (void**)arr->data;
            mph_gc_mark(arr->data);
            for (size_t i = 0; i < arr->length; i++) {
                 if (data_ptr[i]) mph_gc_mark(data_ptr[i]);
            }
        } else {
             mph_gc_mark(arr->data);
        }
    }
}

void mph_gc_sweep(MorphContext* ctx) {
    ObjectHeader** curr_ptr = &ctx->heap_head;
    size_t freed_count = 0;

    while (*curr_ptr) {
        ObjectHeader* curr = *curr_ptr;
        if (curr->flags & FLAG_MARKED) {
            curr->flags &= ~FLAG_MARKED;
            curr_ptr = &curr->next;
        } else {
            *curr_ptr = curr->next;
            if (curr->type) ctx->allocated_bytes -= curr->type->size;
            free(curr);
            freed_count++;
        }
    }
}

void mph_gc_collect(MorphContext* ctx) {
    // 1. Mark Roots from Shadow Stack
    for (int i = 0; i < ctx->stack_top; i++) {
        void** root = ctx->shadow_stack[i];
        if (*root) {
            mph_gc_mark(*root);
        }
    }

    // 2. Sweep
    mph_gc_sweep(ctx);
}

void* mph_alloc(MorphContext* ctx, size_t size, MorphTypeInfo* type_info) {
    if (ctx->allocated_bytes > ctx->next_gc_threshold) {
        mph_gc_collect(ctx);
        ctx->next_gc_threshold = ctx->allocated_bytes + GC_THRESHOLD;
    }

    size_t total_size = sizeof(ObjectHeader) + size;
    ObjectHeader* header = (ObjectHeader*)malloc(total_size);
    if (!header) {
        printf("Fatal: Out of memory\n");
        exit(1);
    }

    memset(header, 0, total_size);
    header->type = type_info;
    header->flags = 0;

    header->next = ctx->heap_head;
    ctx->heap_head = header;
    ctx->allocated_bytes += size;

    return (void*)(header + 1);
}

// --- Strings ---

MorphString* mph_string_new(MorphContext* ctx, const char* literal) {
    size_t len = strlen(literal);
    MorphString* str = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
    str->length = len;

    char* data = (char*)mph_alloc(ctx, len + 1, &mph_ti_raw);
    memcpy(data, literal, len);
    data[len] = '\0';
    str->data = data;
    return str;
}

MorphString* mph_string_concat(MorphContext* ctx, MorphString* a, MorphString* b) {
    size_t len = a->length + b->length;
    MorphString* str = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
    str->length = len;

    char* data = (char*)mph_alloc(ctx, len + 1, &mph_ti_raw);
    memcpy(data, a->data, a->length);
    memcpy(data + a->length, b->data, b->length);
    data[len] = '\0';
    str->data = data;
    return str;
}

mph_bool mph_string_eq(MorphString* a, MorphString* b) {
    if (a->length != b->length) return 0;
    return memcmp(a->data, b->data, a->length) == 0;
}

// --- Arrays ---

MorphTypeInfo mph_ti_array = { "array", sizeof(MorphArray), 0, NULL }; // Special marking

MorphArray* mph_array_new(MorphContext* ctx, size_t capacity, size_t element_size, MorphTypeInfo* element_type) {
    MorphArray* arr = (MorphArray*)mph_alloc(ctx, sizeof(MorphArray), &mph_ti_array);
    arr->length = capacity;
    arr->capacity = capacity;
    arr->element_size = element_size;
    arr->element_type = element_type;

    arr->data = mph_alloc(ctx, capacity * element_size, &mph_ti_raw);
    return arr;
}

void* mph_array_at(MorphContext* ctx, MorphArray* arr, mph_int index) {
    if (index < 0 || index >= arr->length) {
        printf("Panic: Array index out of bounds\n");
        exit(1);
    }
    return (uint8_t*)arr->data + (index * arr->element_size);
}

// --- Maps ---

MorphMap* mph_map_new(MorphContext* ctx, MorphKeyKind kind) {
    MorphMap* map = (MorphMap*)mph_alloc(ctx, sizeof(MorphMap), NULL);
    map->capacity = 16;
    map->count = 0;
    map->key_kind = kind;
    map->entries = (MorphMapEntry*)mph_alloc(ctx, sizeof(MorphMapEntry) * map->capacity, &mph_ti_raw);
    memset(map->entries, 0, sizeof(MorphMapEntry) * map->capacity);
    return map;
}

// Simplified Linear Probe Map Logic
uint64_t mph_hash_bytes(const void* data, size_t len) {
    uint64_t hash = 14695981039346656037ULL;
    const uint8_t* p = (const uint8_t*)data;
    for (size_t i = 0; i < len; i++) { hash ^= p[i]; hash *= 1099511628211ULL; }
    return hash;
}
uint64_t mph_hash_key(void* key, MorphKeyKind kind) {
    if (kind == MPH_KEY_INT) return (uint64_t)key; // Simple identity
    if (kind == MPH_KEY_STRING) { MorphString* s = (MorphString*)key; return mph_hash_bytes(s->data, s->length); }
    return (uint64_t)key;
}
mph_bool mph_key_eq(void* k1, void* k2, MorphKeyKind kind) {
    if (kind == MPH_KEY_INT) return k1 == k2;
    if (kind == MPH_KEY_STRING) return mph_string_eq((MorphString*)k1, (MorphString*)k2);
    return k1 == k2;
}

// Helper: Resize
void mph_map_resize(MorphContext* ctx, MorphMap* map) {
    size_t new_cap = map->capacity * 2;
    MorphMapEntry* new_entries = (MorphMapEntry*)mph_alloc(ctx, sizeof(MorphMapEntry) * new_cap, &mph_ti_raw);
    memset(new_entries, 0, sizeof(MorphMapEntry) * new_cap);

    MorphMapEntry* old_entries = map->entries;
    size_t old_cap = map->capacity;

    // Swap
    map->entries = new_entries;
    map->capacity = new_cap;
    map->count = 0; // Re-insert

    for (size_t i=0; i < old_cap; i++) {
        if (old_entries[i].occupied && !old_entries[i].deleted) {
             mph_map_set(ctx, map, old_entries[i].key, old_entries[i].value);
        }
    }
}

void mph_map_set(MorphContext* ctx, MorphMap* map, void* key, void* value) {
    if (map->count >= map->capacity * 0.75) {
        mph_map_resize(ctx, map);
    }
    uint64_t hash = mph_hash_key(key, map->key_kind);
    size_t idx = hash % map->capacity;
    size_t start = idx;
    while (1) {
        MorphMapEntry* e = &map->entries[idx];
        if (!e->occupied) {
            e->key = key; e->value = value; e->occupied = 1; map->count++; return;
        }
        if (!e->deleted && mph_key_eq(e->key, key, map->key_kind)) {
            e->value = value; return;
        }
        idx = (idx + 1) % map->capacity;
        if (idx == start) {
            // Should be impossible with resize, but safe fallback
            mph_map_resize(ctx, map);
            mph_map_set(ctx, map, key, value);
            return;
        }
    }
}
void* mph_map_get(MorphContext* ctx, MorphMap* map, void* key) {
    uint64_t hash = mph_hash_key(key, map->key_kind);
    size_t idx = hash % map->capacity;
    size_t start = idx;
    while (1) {
        MorphMapEntry* e = &map->entries[idx];
        if (!e->occupied) return NULL;
        if (!e->deleted && mph_key_eq(e->key, key, map->key_kind)) return e->value;
        idx = (idx + 1) % map->capacity;
        if (idx == start) return NULL;
    }
}
void mph_map_delete(MorphContext* ctx, MorphMap* map, void* key) {
     uint64_t hash = mph_hash_key(key, map->key_kind);
    size_t idx = hash % map->capacity;
    size_t start = idx;
    while (1) {
        MorphMapEntry* e = &map->entries[idx];
        if (!e->occupied) return;
        if (!e->deleted && mph_key_eq(e->key, key, map->key_kind)) { e->deleted = 1; map->count--; return; }
        idx = (idx + 1) % map->capacity;
        if (idx == start) return;
    }
}
mph_int mph_map_len(MorphContext* ctx, MorphMap* map) { return map ? map->count : 0; }

// --- Closures ---

MorphClosure* mph_closure_new(MorphContext* ctx, void* fn, void* env, int env_size) {
    MorphClosure* c = (MorphClosure*)mph_alloc(ctx, sizeof(MorphClosure), NULL);
    c->function = fn;
    if (env_size > 0) {
        c->env = mph_alloc(ctx, env_size, &mph_ti_raw);
        memcpy(c->env, env, env_size);
    } else {
        c->env = NULL;
    }
    return c;
}

// --- Channels ---
MorphChannel* mph_channel_new(MorphContext* ctx) {
    MorphChannel* c = (MorphChannel*)malloc(sizeof(MorphChannel));
    pthread_mutex_init(&c->lock, NULL);
    pthread_cond_init(&c->cond_send, NULL);
    pthread_cond_init(&c->cond_recv, NULL);
    c->capacity = 5;
    c->buffer = (int64_t*)malloc(sizeof(int64_t) * c->capacity);
    c->count = 0; c->head = 0; c->tail = 0;
    return c;
}
void mph_channel_destroy(MorphContext* ctx, MorphChannel* c) {
    pthread_mutex_destroy(&c->lock);
    pthread_cond_destroy(&c->cond_send);
    pthread_cond_destroy(&c->cond_recv);
    free(c->buffer); free(c);
}
void mph_channel_send(MorphContext* ctx, MorphChannel* c, mph_int val) {
    pthread_mutex_lock(&c->lock);
    while (c->count == c->capacity) pthread_cond_wait(&c->cond_send, &c->lock);
    c->buffer[c->tail] = val;
    c->tail = (c->tail + 1) % c->capacity;
    c->count++;
    pthread_cond_signal(&c->cond_recv);
    pthread_mutex_unlock(&c->lock);
}
mph_int mph_channel_recv(MorphContext* ctx, MorphChannel* c) {
    pthread_mutex_lock(&c->lock);
    while (c->count == 0) pthread_cond_wait(&c->cond_recv, &c->lock);
    mph_int val = c->buffer[c->head];
    c->head = (c->head + 1) % c->capacity;
    c->count--;
    pthread_cond_signal(&c->cond_send);
    pthread_mutex_unlock(&c->lock);
    return val;
}
typedef struct ThreadArgs { MorphEntryFunction fn; void* arg; MorphContext ctx; } ThreadArgs;
void* mph_thread_wrapper(void* ptr) {
    ThreadArgs* args = (ThreadArgs*)ptr;
    mph_init_memory(&args->ctx);
    args->fn(&args->ctx, args->arg);
    mph_destroy_memory(&args->ctx);
    free(args);
    return NULL;
}
void mph_thread_spawn(MorphEntryFunction fn, void* arg) {
    pthread_t thread;
    ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
    args->fn = fn; args->arg = arg;
    pthread_create(&thread, NULL, mph_thread_wrapper, args);
    pthread_detach(thread);
}

// --- Interfaces ---
void* mph_assert_type(MorphContext* ctx, MorphInterface iface, mph_int expected_id) {
    if (iface.type_id == expected_id) return iface.instance;
    exit(1); return NULL;
}

// --- IO (Minimal for Phase 1) ---
#define MAX_FILES 1024
FILE* mph_file_table[MAX_FILES];
int mph_file_count = 3;
typedef struct InternalFile { mph_int fd; } InternalFile;
void mph_init_files() { mph_file_table[0] = stdin; mph_file_table[1] = stdout; mph_file_table[2] = stderr; }
void mph_native_print_int(MorphContext* ctx, mph_int n) { printf("%ld\n", n); }
void mph_native_print(MorphContext* ctx, MorphString* s) { printf("%s\n", s->data); }

void* mph_io_Open(MorphContext* ctx, MorphString* path) {
    mph_init_files();
    FILE* h = fopen(path->data, "r"); if(!h) return NULL;
    int fd = mph_file_count++; mph_file_table[fd] = h;
    InternalFile* f = (InternalFile*)mph_alloc(ctx, sizeof(InternalFile), NULL); f->fd = fd; return f;
}
void* mph_io_Create(MorphContext* ctx, MorphString* path) {
    mph_init_files();
    FILE* h = fopen(path->data, "w+"); if(!h) return NULL;
    int fd = mph_file_count++; mph_file_table[fd] = h;
    InternalFile* f = (InternalFile*)mph_alloc(ctx, sizeof(InternalFile), NULL); f->fd = fd; return f;
}
MorphString* mph_io_Read(MorphContext* ctx, void* f, mph_int size) {
    mph_init_files();
    if (!f) return mph_string_new(ctx, "");
    int fd = ((InternalFile*)f)->fd;
    char* buf = (char*)mph_alloc(ctx, size+1, &mph_ti_raw);
    size_t r = fread(buf, 1, size, mph_file_table[fd]);
    buf[r] = 0;
    MorphString* s = (MorphString*)mph_alloc(ctx, sizeof(MorphString), &mph_ti_string_real);
    s->data = buf; s->length = r; return s;
}
mph_int mph_io_Write(MorphContext* ctx, void* f, MorphString* s) {
    mph_init_files();
    if (!f) return -1;
    int fd = ((InternalFile*)f)->fd;
    return fwrite(s->data, 1, s->length, mph_file_table[fd]);
}
mph_int mph_io_Close(MorphContext* ctx, void* f) {
    mph_init_files();
    if (!f) return -1;
    int fd = ((InternalFile*)f)->fd;
    if (fd <= 2) return 0;
    return fclose(mph_file_table[fd]);
}

// Entry point wrapper (if not provided by generated code)
int main() {
    MorphContext ctx;
    mph_init_memory(&ctx);
    morph_entry_point(&ctx);
    mph_destroy_memory(&ctx);
    return 0;
}
