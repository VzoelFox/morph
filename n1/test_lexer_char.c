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
typedef struct Env_mph__main {
	MorphClosure* substring;
} Env_mph__main;

MorphTypeInfo mph_ti_Env_mph__main = { "Env_mph__main", sizeof(Env_mph__main), 1, (size_t[]){offsetof(Env_mph__main, substring)}, NULL };

// Global Variables

// Function Prototypes
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
void mph_main(MorphContext* ctx, void* _env_void) {
	Env_mph__main* _env = (Env_mph__main*)_env_void;
	mph_native_print(ctx, mph_string_new(ctx, "Lexer Char Test\n"));
	MorphString* s = mph_string_new(ctx, "var");
	mph_gc_push_root(ctx, (void**)&s);
	MorphString* c = ({ MorphString* _arg_1 = s; mph_gc_push_root(ctx, (void**)&_arg_1); MorphString* _ret_2 = ((MorphString* (*)(MorphContext*, void*, MorphString*, mph_int, mph_int))_env->substring->function)(ctx, _env->substring->env, _arg_1, 0, 1); mph_gc_pop_roots(ctx, 1); _ret_2; });
	mph_gc_push_root(ctx, (void**)&c);
	if (mph_string_eq(ctx, c, mph_string_new(ctx, "v"))) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â first char = v\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â first char\n"));
}
	}
	mph_int len = ({ MorphString* _len_arg_3 = s; mph_gc_push_root(ctx, (void**)&_len_arg_3); mph_int _ret_4 = ((MorphString*)_len_arg_3)->length; mph_gc_pop_roots(ctx, 1); _ret_4; });
	if ((len == 3)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â length = 3\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â length\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "OK!\n"));
	mph_gc_pop_roots(ctx, 2);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
