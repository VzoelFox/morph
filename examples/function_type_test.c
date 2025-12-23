#include "morph.h"

// Native bindings
void mph_native_print(MorphContext* ctx, MorphString* s);
void mph_native_print_int(MorphContext* ctx, mph_int n);

// Struct Definitions

// Type IDs

// Struct Definitions (Env & Types)
typedef struct Env_mph__main {
	MorphClosure* native_print_int;
} Env_mph__main;


// Function Definitions
void mph_main(MorphContext* ctx, void* _env_void) {
	Env_mph__main* _env = (Env_mph__main*)_env_void;
mph_int mph_anon_0xc00007a140(MorphContext* ctx, void* _env_void, mph_int a, mph_int b) {
	return (a + b);
}

	MorphClosure* fn = ({ mph_closure_new(ctx, (void*)mph_anon_0xc00007a140, NULL);  });
	mph_int x = ((mph_int (*)(MorphContext*, void*, mph_int, mph_int))fn->function)(ctx, fn->env, 1, 2);
	mph_native_print_int(ctx, x);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
