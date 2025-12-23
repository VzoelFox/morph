#include "morph.h"

// Native bindings
void mph_native_print(MorphContext* ctx, MorphString* s);
void mph_native_print_int(MorphContext* ctx, mph_int n);

// Struct Definitions

// RTTI Definitions

// Type IDs

// Struct Definitions (Env & Types)

// Function Definitions
void mph_main(MorphContext* ctx, void* _env_void) {
	mph_int i = 0;
	while ((i < 100000)) {
	MorphString* s = mph_string_concat(ctx, mph_string_new(ctx, "temp string "), mph_string_new(ctx, "created"));
	mph_gc_push_root(ctx, &s);
	MorphArray* a = ({ MorphArray* _a = mph_array_new(ctx, 3, sizeof(mph_int), NULL); ((mph_int*)_a->data)[0] = i; ((mph_int*)_a->data)[1] = (i + 1); ((mph_int*)_a->data)[2] = (i + 2); _a; });
	mph_gc_push_root(ctx, &a);
	i = (i + 1);
	mph_gc_pop_roots(ctx, 2);
	}
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
	mph_main(ctx, NULL);
}
