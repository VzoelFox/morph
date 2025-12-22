#ifndef MORPH_H
#define MORPH_H

#include <stddef.h>
#include <stdint.h>

// Basic Types
typedef int64_t mph_int;
typedef double  mph_float;
typedef int     mph_bool; // 0 or 1
typedef void    mph_void;

// Context
typedef struct MorphContext {
    void* allocator; // Placeholder for Arena
    void* scheduler; // Placeholder for Coroutines
} MorphContext;

// String Structure (Simple)
typedef struct MorphString {
    const char* data;
    size_t length;
} MorphString;

// Memory API
void* mph_malloc(MorphContext* ctx, size_t size);

// Entry Point for generated code
void morph_entry_point(MorphContext* ctx);

#endif // MORPH_H
