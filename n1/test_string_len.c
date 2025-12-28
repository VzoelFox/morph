#include "morph.h"

// Native bindings
void mph_native_print(MorphContext* ctx, MorphString* s);
void mph_native_print_int(MorphContext* ctx, mph_int n);
mph_int mph_string_index(MorphContext* ctx, MorphString* s, MorphString* sub);
MorphString* mph_string_trim(MorphContext* ctx, MorphString* s, MorphString* cut);
MorphArray* mph_string_split(MorphContext* ctx, MorphString* s, MorphString* sep);

// Struct Definitions

// RTTI Definitions

// Type IDs

// Struct Definitions (Env & Types)

// Global Variables

// Function Prototypes
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "String Test\n"));
	MorphString* s = mph_string_new(ctx, "hello");
	mph_gc_push_root(ctx, (void**)&s);
	mph_int len = ({ MorphString* _len_arg_1 = s; mph_gc_push_root(ctx, (void**)&_len_arg_1); mph_int _ret_2 = ((MorphString*)_len_arg_1)->length; mph_gc_pop_roots(ctx, 1); _ret_2; });
	mph_native_print(ctx, mph_string_new(ctx, "length: "));
	mph_native_print_int(ctx, len);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if ((len == 5)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â OK\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â FAIL\n"));
}
	}
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
