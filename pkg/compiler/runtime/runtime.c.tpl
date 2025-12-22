#include <stdlib.h>
#include <stdio.h>
#include "morph.h"

// Option A: Leaky Allocator (Direct malloc)
void* mph_malloc(MorphContext* ctx, size_t size) {
    return malloc(size);
}

// Runtime Entry Point
int main() {
    // Initialize Context
    MorphContext ctx;
    ctx.allocator = NULL; // Todo: Init Arena
    ctx.scheduler = NULL; // Todo: Init Scheduler

    // Call Generated Entry Point
    morph_entry_point(&ctx);

    return 0;
}

// Temporary Native Bindings (until stdlib is mapped)
// native fungsi print(s String) Void
void mph_native_print(MorphContext* ctx, const char* s) {
    printf("%s\n", s);
}
