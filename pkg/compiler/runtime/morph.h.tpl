#ifndef MORPH_H
#define MORPH_H

#include <stddef.h>
#include <stdint.h>

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

// --- Object Structures ---

typedef struct MorphString {
    const char* data;
    size_t length;
} MorphString;

// --- API ---

// Memory
void mph_init_memory(MorphContext* ctx);
void* mph_alloc(MorphContext* ctx, size_t size);
Drawer* mph_new_drawer(Cabinet* cab);

// Strings
MorphString* mph_string_new(MorphContext* ctx, const char* literal);

// Entry Point
void morph_entry_point(MorphContext* ctx);

#endif // MORPH_H
