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
mph_int mph_str_len(MorphContext* ctx, void* _env_void, MorphString* s);
mph_bool mph_str_eq(MorphContext* ctx, void* _env_void, MorphString* a, MorphString* b);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_int mph_str_len(MorphContext* ctx, void* _env_void, MorphString* s) {
	mph_gc_push_root(ctx, (void**)&s);
	return ({ MorphString* _len_arg_1 = s; mph_gc_push_root(ctx, (void**)&_len_arg_1); mph_int _ret_2 = ((MorphString*)_len_arg_1)->length; mph_gc_pop_roots(ctx, 1); _ret_2; });
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_str_eq(MorphContext* ctx, void* _env_void, MorphString* a, MorphString* b) {
	mph_gc_push_root(ctx, (void**)&a);
	mph_gc_push_root(ctx, (void**)&b);
	return mph_string_eq(ctx, a, b);
	mph_gc_pop_roots(ctx, 2);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "Str\n"));
	MorphString* s = mph_string_new(ctx, "hello");
	mph_gc_push_root(ctx, (void**)&s);
	mph_int l = ({ MorphString* _arg_3 = s; mph_gc_push_root(ctx, (void**)&_arg_3); mph_int _ret_4 = mph_str_len(ctx, NULL, _arg_3); mph_gc_pop_roots(ctx, 1); _ret_4; });
	mph_native_print_int(ctx, l);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if (({ MorphString* _arg_5 = mph_string_new(ctx, "a"); mph_gc_push_root(ctx, (void**)&_arg_5); MorphString* _arg_6 = mph_string_new(ctx, "a"); mph_gc_push_root(ctx, (void**)&_arg_6); mph_bool _ret_7 = mph_str_eq(ctx, NULL, _arg_5, _arg_6); mph_gc_pop_roots(ctx, 2); _ret_7; })) {
{
	mph_native_print(ctx, mph_string_new(ctx, "OK\n"));
}
	}
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
