#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "morph.h"

// --- Memory Implementation ---

Drawer* mph_new_drawer(Cabinet* cab) {
    if (cab->drawer_count >= MAX_DRAWERS) {
        printf("Fatal: Out of memory (Max Drawers reached)\n");
        exit(1);
    }

    Drawer* d = (Drawer*)malloc(sizeof(Drawer));
    d->id = cab->drawer_count;
    d->data = (uint8_t*)malloc(DRAWER_SIZE);

    // Init Primary Tray (Lower 64KB)
    d->primary.start = d->data;
    d->primary.current = d->data;
    d->primary.end = d->data + TRAY_SIZE;

    cab->drawers[d->id] = d;
    cab->drawer_count++;
    cab->active_drawer_index = d->id;

    return d;
}

void mph_init_memory(MorphContext* ctx) {
    ctx->cabinet = (Cabinet*)malloc(sizeof(Cabinet));
    ctx->cabinet->drawer_count = 0;
    ctx->cabinet->active_drawer_index = -1;

    // Create first drawer
    mph_new_drawer(ctx->cabinet);
}

void mph_destroy_memory(MorphContext* ctx) {
    if (ctx->cabinet == NULL) return;

    Cabinet* cab = ctx->cabinet;
    for (int i = 0; i < cab->drawer_count; i++) {
        Drawer* d = cab->drawers[i];
        if (d->data) free(d->data);
        free(d);
    }
    free(cab);
    ctx->cabinet = NULL;
}

void* mph_alloc(MorphContext* ctx, size_t size) {
    // 8-byte alignment
    size = (size + 7) & ~7;

    Cabinet* cab = ctx->cabinet;
    Drawer* active = cab->drawers[cab->active_drawer_index];

    // Check capacity
    if (active->primary.current + size > active->primary.end) {
        active = mph_new_drawer(cab);
    }

    void* ptr = active->primary.current;
    active->primary.current += size;
    return ptr;
}

// --- String Implementation ---

MorphString* mph_string_new(MorphContext* ctx, const char* literal) {
    size_t len = strlen(literal);

    // Alloc Struct
    MorphString* str = (MorphString*)mph_alloc(ctx, sizeof(MorphString));
    str->length = len;

    // Alloc Data
    char* data = (char*)mph_alloc(ctx, len + 1);
    memcpy(data, literal, len);
    data[len] = '\0'; // Null terminate for C compat

    str->data = data;
    return str;
}

MorphString* mph_string_concat(MorphContext* ctx, MorphString* a, MorphString* b) {
    size_t len = a->length + b->length;
    MorphString* str = (MorphString*)mph_alloc(ctx, sizeof(MorphString));
    str->length = len;

    char* data = (char*)mph_alloc(ctx, len + 1);
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

// --- Array Implementation ---

MorphArray* mph_array_new(MorphContext* ctx, size_t capacity, size_t element_size) {
    MorphArray* arr = (MorphArray*)mph_alloc(ctx, sizeof(MorphArray));
    arr->length = capacity;
    arr->capacity = capacity;
    arr->element_size = element_size;

    arr->data = mph_alloc(ctx, capacity * element_size);
    return arr;
}

void* mph_array_at(MorphContext* ctx, MorphArray* arr, mph_int index) {
    if (index < 0 || index >= arr->length) {
        printf("Panic: Array index out of bounds\n");
        exit(1);
    }
    return (uint8_t*)arr->data + (index * arr->element_size);
}

// --- I/O Implementation ---

#define MAX_FILES 1024
FILE* mph_file_table[MAX_FILES];
int mph_file_count = 3; // 0,1,2 reserved
int mph_files_initialized = 0;

void mph_init_files() {
    if (mph_files_initialized) return;
    mph_file_table[0] = stdin;
    mph_file_table[1] = stdout;
    mph_file_table[2] = stderr;
    mph_files_initialized = 1;
}

typedef struct InternalFile {
    mph_int fd;
} InternalFile;

void* mph_io_make_file(MorphContext* ctx, mph_int fd) {
    mph_init_files();
    InternalFile* f = (InternalFile*)mph_alloc(ctx, sizeof(InternalFile));
    f->fd = fd;
    return f;
}

void* mph_io_Open(MorphContext* ctx, MorphString* path) {
    mph_init_files();
    if (mph_file_count >= MAX_FILES) return NULL;

    FILE* handle = fopen(path->data, "r");
    if (!handle) return NULL;

    int fd = mph_file_count++;
    mph_file_table[fd] = handle;

    InternalFile* f = (InternalFile*)mph_alloc(ctx, sizeof(InternalFile));
    f->fd = fd;
    return f;
}

void* mph_io_Create(MorphContext* ctx, MorphString* path) {
    mph_init_files();
    if (mph_file_count >= MAX_FILES) return NULL;

    FILE* handle = fopen(path->data, "w+");
    if (!handle) return NULL;

    int fd = mph_file_count++;
    mph_file_table[fd] = handle;

    InternalFile* f = (InternalFile*)mph_alloc(ctx, sizeof(InternalFile));
    f->fd = fd;
    return f;
}

MorphString* mph_io_Read(MorphContext* ctx, void* f, mph_int size) {
    mph_init_files();
    if (!f) return mph_string_new(ctx, "");
    mph_int fd = ((InternalFile*)f)->fd;
    if (fd < 0 || fd >= MAX_FILES || !mph_file_table[fd]) return mph_string_new(ctx, "");

    char* buf = (char*)mph_alloc(ctx, size + 1);
    size_t read = fread(buf, 1, size, mph_file_table[fd]);
    buf[read] = '\0';

    MorphString* s = (MorphString*)mph_alloc(ctx, sizeof(MorphString));
    s->data = buf;
    s->length = read;
    return s;
}

mph_int mph_io_Write(MorphContext* ctx, void* f, MorphString* s) {
    mph_init_files();
    if (!f) return -1;
    mph_int fd = ((InternalFile*)f)->fd;
    if (fd < 0 || fd >= MAX_FILES || !mph_file_table[fd]) return -1;

    return fwrite(s->data, 1, s->length, mph_file_table[fd]);
}

mph_int mph_io_Close(MorphContext* ctx, void* f) {
    mph_init_files();
    if (!f) return -1;
    mph_int fd = ((InternalFile*)f)->fd;
    if (fd < 0 || fd >= MAX_FILES || !mph_file_table[fd]) return -1;

    if (fd <= 2) return 0;

    int res = fclose(mph_file_table[fd]);
    mph_file_table[fd] = NULL;
    return res;
}

void mph_native_print_int(MorphContext* ctx, mph_int n) {
    printf("%ld\n", n);
}

// --- Entry Point ---

int main() {
    MorphContext ctx;
    mph_init_memory(&ctx);

    morph_entry_point(&ctx);

    mph_destroy_memory(&ctx);
    return 0;
}

// --- Native Bindings ---

void mph_native_print(MorphContext* ctx, MorphString* s) {
    printf("%s\n", s->data);
}

// --- Concurrency Implementation ---

typedef struct ThreadArgs {
    MorphEntryFunction fn;
    void* arg;
    MorphContext ctx;
} ThreadArgs;

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
    args->fn = fn;
    args->arg = arg;

    pthread_create(&thread, NULL, mph_thread_wrapper, args);
    pthread_detach(thread);
}

MorphChannel* mph_channel_new(MorphContext* ctx) {
    MorphChannel* c = (MorphChannel*)malloc(sizeof(MorphChannel));
    pthread_mutex_init(&c->lock, NULL);
    pthread_cond_init(&c->cond_send, NULL);
    pthread_cond_init(&c->cond_recv, NULL);

    c->capacity = 5; // Small buffer
    c->buffer = (int64_t*)malloc(sizeof(int64_t) * c->capacity);
    c->count = 0;
    c->head = 0;
    c->tail = 0;

    return c;
}

void mph_channel_destroy(MorphContext* ctx, MorphChannel* c) {
    pthread_mutex_destroy(&c->lock);
    pthread_cond_destroy(&c->cond_send);
    pthread_cond_destroy(&c->cond_recv);
    free(c->buffer);
    free(c);
}

void mph_channel_send(MorphContext* ctx, MorphChannel* c, mph_int val) {
    pthread_mutex_lock(&c->lock);

    while (c->count == c->capacity) {
        pthread_cond_wait(&c->cond_send, &c->lock);
    }

    c->buffer[c->tail] = val;
    c->tail = (c->tail + 1) % c->capacity;
    c->count++;

    pthread_cond_signal(&c->cond_recv);
    pthread_mutex_unlock(&c->lock);
}

mph_int mph_channel_recv(MorphContext* ctx, MorphChannel* c) {
    pthread_mutex_lock(&c->lock);

    while (c->count == 0) {
        pthread_cond_wait(&c->cond_recv, &c->lock);
    }

    mph_int val = c->buffer[c->head];
    c->head = (c->head + 1) % c->capacity;
    c->count--;

    pthread_cond_signal(&c->cond_send);
    pthread_mutex_unlock(&c->lock);

    return val;
}
