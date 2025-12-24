#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "morph.h"

// --- Tiered Memory & GC Implementation ---

#define FLAG_MARKED 0x01
#define FLAG_SWAPPED 0x02

// Simple TypeInfos for built-ins
MorphTypeInfo mph_ti_string = { "string", sizeof(MorphString), 0, NULL };
size_t string_ptr_offsets[] = { offsetof(MorphString, data) };
MorphTypeInfo mph_ti_string_real = { "string", sizeof(MorphString), 1, string_ptr_offsets };
MorphTypeInfo mph_ti_raw = { "raw", 0, 0, NULL };

// --- Utils ---
uint64_t mph_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// --- Memory ---

void mph_init_memory(MorphContext* ctx) {
    ctx->heap_head = NULL;
    ctx->allocated_bytes = 0;
    ctx->next_gc_threshold = GC_THRESHOLD;
    ctx->stack_top = NULL;
    ctx->daemon_running = 0;
    pthread_mutex_init(&ctx->memory_lock, NULL);

    // Create swap dir
    mkdir(".morph.vz", 0755);
    mkdir(".morph.vz/swap", 0755);

    mph_start_daemon(ctx);
}

void mph_destroy_memory(MorphContext* ctx) {
    mph_stop_daemon(ctx);

    pthread_mutex_lock(&ctx->memory_lock);
    ObjectHeader* current = ctx->heap_head;
    while (current != NULL) {
        ObjectHeader* next = current->next;
        // Free payload if not swapped
        // (If swapped, payload pointer might be invalid or NULL)
        free(current);
        current = next;
    }
    ctx->heap_head = NULL;
    pthread_mutex_unlock(&ctx->memory_lock);
    pthread_mutex_destroy(&ctx->memory_lock);
}

// Helper to get header from payload
ObjectHeader* mph_get_header(void* obj) {
    return ((ObjectHeader*)obj) - 1;
}

// Shadow Stack
void mph_gc_push_root(MorphContext* ctx, void** ptr) {
    StackRoot* node = (StackRoot*)malloc(sizeof(StackRoot));
    node->ptr = ptr;
    node->next = ctx->stack_top;
    ctx->stack_top = node;
}

void mph_gc_pop_roots(MorphContext* ctx, int count) {
    for (int i = 0; i < count; i++) {
        if (ctx->stack_top) {
            StackRoot* temp = ctx->stack_top;
            ctx->stack_top = ctx->stack_top->next;
            free(temp);
        }
    }
}

// Swap Logic (Lockless Helper)
void mph_swap_in_locked(MorphContext* ctx, void* obj) {
    if (!obj) return;
    ObjectHeader* header = mph_get_header(obj);

    header->last_access = mph_time_ms();

    if (header->flags & FLAG_SWAPPED) {
        char path[256];
        sprintf(path, ".morph.vz/swap/%lu.bin", header->swap_id);

        FILE* f = fopen(path, "rb");
        if (f) {
            void* payload = (void*)(header + 1);
            fread(payload, header->type->size, 1, f);
            fclose(f);
            unlink(path); // Remove swap file
        }
        header->flags &= ~FLAG_SWAPPED;
    }
}

void mph_swap_in(MorphContext* ctx, void* obj) {
    if (!obj) return;
    pthread_mutex_lock(&ctx->memory_lock);
    mph_swap_in_locked(ctx, obj);
    pthread_mutex_unlock(&ctx->memory_lock);
}

void mph_swap_out_locked(MorphContext* ctx, ObjectHeader* header) {
    if (header->flags & FLAG_SWAPPED) return;
    if (!header->type || header->type->size == 0) return; // Don't swap stateless/raw if risky

    // Generate Swap ID (timestamp + address)
    header->swap_id = mph_time_ms() + (uint64_t)header;
    char path[256];
    sprintf(path, ".morph.vz/swap/%lu.bin", header->swap_id);

    FILE* f = fopen(path, "wb");
    if (!f) return;

    void* payload = (void*)(header + 1);
    fwrite(payload, header->type->size, 1, f);
    fclose(f);

    header->flags |= FLAG_SWAPPED;
    memset(payload, 0, header->type->size);
}

void mph_swap_out(MorphContext* ctx, ObjectHeader* header) {
    pthread_mutex_lock(&ctx->memory_lock);
    mph_swap_out_locked(ctx, header);
    pthread_mutex_unlock(&ctx->memory_lock);
}


void mph_gc_mark(MorphContext* ctx, void* obj) {
    if (!obj) return;

    // Ensure object is in RAM (must be called with lock held)
    mph_swap_in_locked(ctx, obj);

    ObjectHeader* header = mph_get_header(obj);
    if (header->flags & FLAG_MARKED) return;
    header->flags |= FLAG_MARKED;

    MorphTypeInfo* type = header->type;
    if (!type) return;

    char* payload = (char*)obj;
    for (int i = 0; i < type->num_pointers; i++) {
        size_t offset = type->pointer_offsets[i];
        void** child_ptr = (void**)(payload + offset);
        if (*child_ptr) {
            mph_gc_mark(ctx, *child_ptr);
        }
    }
}

void mph_gc_sweep(MorphContext* ctx) {
    ObjectHeader** curr_ptr = &ctx->heap_head;

    while (*curr_ptr) {
        ObjectHeader* curr = *curr_ptr;
        if (curr->flags & FLAG_MARKED) {
            curr->flags &= ~FLAG_MARKED;
            curr_ptr = &curr->next;
        } else {
            *curr_ptr = curr->next;
            // Free it
            if (curr->flags & FLAG_SWAPPED) {
                 char path[256];
                 sprintf(path, ".morph.vz/swap/%lu.bin", curr->swap_id);
                 unlink(path);
            }
            if (curr->type) ctx->allocated_bytes -= curr->type->size;
            free(curr);
        }
    }
}

void mph_gc_collect(MorphContext* ctx) {
    pthread_mutex_lock(&ctx->memory_lock);
    // Mark Roots from Shadow Stack
    StackRoot* root = ctx->stack_top;
    while (root) {
        if (root->ptr && *root->ptr) {
            mph_gc_mark(ctx, *root->ptr);
        }
        root = root->next;
    }

    // Sweep
    mph_gc_sweep(ctx);
    pthread_mutex_unlock(&ctx->memory_lock);
}

// Daemon
void* mph_daemon_loop(void* arg) {
    MorphContext* ctx = (MorphContext*)arg;
    while (ctx->daemon_running) {
        usleep(DAEMON_SLEEP_MS * 1000);

        // Tiered Memory Scan
        pthread_mutex_lock(&ctx->memory_lock);
        uint64_t now = mph_time_ms();
        ObjectHeader* curr = ctx->heap_head;
        while (curr) {
            if (!(curr->flags & FLAG_SWAPPED) && (now - curr->last_access > SWAP_AGE_THRESHOLD_SEC * 1000)) {
                mph_swap_out_locked(ctx, curr);
            }
            curr = curr->next;
        }
        pthread_mutex_unlock(&ctx->memory_lock);
    }
    return NULL;
}

void mph_start_daemon(MorphContext* ctx) {
    ctx->daemon_running = 1;
    pthread_create(&ctx->daemon_thread, NULL, mph_daemon_loop, ctx);
}

void mph_stop_daemon(MorphContext* ctx) {
    ctx->daemon_running = 0;
    pthread_join(ctx->daemon_thread, NULL);
}

void* mph_alloc(MorphContext* ctx, size_t size, MorphTypeInfo* type_info) {
    // Check GC threshold
    if (ctx->allocated_bytes > ctx->next_gc_threshold) {
        mph_gc_collect(ctx);
        ctx->next_gc_threshold = ctx->allocated_bytes + GC_THRESHOLD;
    }

    pthread_mutex_lock(&ctx->memory_lock);
    size_t total_size = sizeof(ObjectHeader) + size;
    ObjectHeader* header = (ObjectHeader*)malloc(total_size);
    if (!header) {
        printf("Fatal: Out of memory\n");
        exit(1);
    }

    memset(header, 0, total_size);
    header->type = type_info;
    header->flags = 0;
    header->last_access = mph_time_ms();

    header->next = ctx->heap_head;
    ctx->heap_head = header;
    ctx->allocated_bytes += size;
    pthread_mutex_unlock(&ctx->memory_lock);

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
    mph_swap_in(ctx, a);
    mph_swap_in(ctx, b);
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

mph_bool mph_string_eq(MorphContext* ctx, MorphString* a, MorphString* b) {
    mph_swap_in(ctx, a);
    mph_swap_in(ctx, b);
    if (a->length != b->length) return 0;
    return memcmp(a->data, b->data, a->length) == 0;
}

// --- Arrays ---

MorphArray* mph_array_new(MorphContext* ctx, size_t capacity, size_t element_size) {
    MorphArray* arr = (MorphArray*)mph_alloc(ctx, sizeof(MorphArray), NULL);
    arr->length = capacity;
    arr->capacity = capacity;
    arr->element_size = element_size;
    arr->data = mph_alloc(ctx, capacity * element_size, &mph_ti_raw);
    return arr;
}

void* mph_array_at(MorphContext* ctx, MorphArray* arr, mph_int index) {
    mph_swap_in(ctx, arr);
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
// We update hash_key to take ctx
uint64_t mph_hash_key(MorphContext* ctx, void* key, MorphKeyKind kind) {
    if (kind == MPH_KEY_INT) return (uint64_t)key;
    if (kind == MPH_KEY_STRING) {
        MorphString* s = (MorphString*)key;
        mph_swap_in(ctx, s);
        return mph_hash_bytes(s->data, s->length);
    }
    return (uint64_t)key;
}

mph_bool mph_key_eq(MorphContext* ctx, void* k1, void* k2, MorphKeyKind kind) {
    if (kind == MPH_KEY_INT) return k1 == k2;
    if (kind == MPH_KEY_STRING) return mph_string_eq(ctx, (MorphString*)k1, (MorphString*)k2);
    return k1 == k2;
}

void mph_map_set(MorphContext* ctx, MorphMap* map, void* key, void* value) {
    mph_swap_in(ctx, map);
    if (map->count >= map->capacity * 0.75) {
        // Resize logic omitted
    }
    uint64_t hash = mph_hash_key(ctx, key, map->key_kind);
    size_t idx = hash % map->capacity;
    size_t start = idx;
    while (1) {
        MorphMapEntry* e = &map->entries[idx];
        if (!e->occupied) {
            e->key = key; e->value = value; e->occupied = 1; map->count++; return;
        }
        if (!e->deleted && mph_key_eq(ctx, e->key, key, map->key_kind)) {
            e->value = value; return;
        }
        idx = (idx + 1) % map->capacity;
        if (idx == start) return;
    }
}
void* mph_map_get(MorphContext* ctx, MorphMap* map, void* key) {
    mph_swap_in(ctx, map);
    uint64_t hash = mph_hash_key(ctx, key, map->key_kind);
    size_t idx = hash % map->capacity;
    size_t start = idx;
    while (1) {
        MorphMapEntry* e = &map->entries[idx];
        if (!e->occupied) return NULL;
        if (!e->deleted && mph_key_eq(ctx, e->key, key, map->key_kind)) return e->value;
        idx = (idx + 1) % map->capacity;
        if (idx == start) return NULL;
    }
}
void mph_map_delete(MorphContext* ctx, MorphMap* map, void* key) {
     mph_swap_in(ctx, map);
     uint64_t hash = mph_hash_key(ctx, key, map->key_kind);
    size_t idx = hash % map->capacity;
    size_t start = idx;
    while (1) {
        MorphMapEntry* e = &map->entries[idx];
        if (!e->occupied) return;
        if (!e->deleted && mph_key_eq(ctx, e->key, key, map->key_kind)) { e->deleted = 1; map->count--; return; }
        idx = (idx + 1) % map->capacity;
        if (idx == start) return;
    }
}
mph_int mph_map_len(MorphContext* ctx, MorphMap* map) {
    mph_swap_in(ctx, map);
    return map ? map->count : 0;
}

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
    if (iface.type_id == expected_id) {
         mph_swap_in(ctx, iface.instance);
         return iface.instance;
    }
    exit(1); return NULL;
}

// --- IO (Minimal for Phase 1) ---
#define MAX_FILES 1024
FILE* mph_file_table[MAX_FILES];
int mph_file_count = 3;
typedef struct InternalFile { mph_int fd; } InternalFile;
void mph_init_files() { mph_file_table[0] = stdin; mph_file_table[1] = stdout; mph_file_table[2] = stderr; }
void mph_native_print_int(MorphContext* ctx, mph_int n) { printf("%ld\n", n); }
void mph_native_print(MorphContext* ctx, MorphString* s) {
    mph_swap_in(ctx, s);
    printf("%s\n", s->data);
}

void* mph_io_Open(MorphContext* ctx, MorphString* path) {
    mph_swap_in(ctx, path);
    mph_init_files();
    FILE* h = fopen(path->data, "r"); if(!h) return NULL;
    int fd = mph_file_count++; mph_file_table[fd] = h;
    InternalFile* f = (InternalFile*)mph_alloc(ctx, sizeof(InternalFile), NULL); f->fd = fd; return f;
}
void* mph_io_Create(MorphContext* ctx, MorphString* path) {
    mph_swap_in(ctx, path);
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
    mph_swap_in(ctx, s);
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

// --- Native Implementations for Stdlib ---
MorphString* mph_conv_Itoa(MorphContext* ctx, void* _env, mph_int i) {
    char buf[32];
    sprintf(buf, "%ld", i);
    return mph_string_new(ctx, buf);
}

mph_int mph_conv_Atoi(MorphContext* ctx, void* _env, MorphString* s) {
    // Stub implementation to return 0 and match the changed signature
    return 0;
}

// Entry point wrapper (if not provided by generated code)
int main() {
    MorphContext ctx;
    mph_init_memory(&ctx);
    morph_entry_point(&ctx);
    mph_destroy_memory(&ctx);
    return 0;
}
