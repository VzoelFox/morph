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

void mph_native_print_int(MorphContext* ctx, mph_int n) {
    printf("%ld\n", n);
}

// --- Entry Point ---

int main() {
    MorphContext ctx;
    mph_init_memory(&ctx);

    morph_entry_point(&ctx);

    return 0;
}

// --- Native Bindings ---

void mph_native_print(MorphContext* ctx, MorphString* s) {
    printf("%s\n", s->data);
}
