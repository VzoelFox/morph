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
typedef struct Env_mph__compile {
	MorphClosure* substring;
} Env_mph__compile;

MorphTypeInfo mph_ti_Env_mph__compile = { "Env_mph__compile", sizeof(Env_mph__compile), 1, (size_t[]){offsetof(Env_mph__compile, substring)}, NULL };

// Global Variables

// Function Prototypes
MorphString* mph_compile(MorphContext* ctx, void* _env_void, MorphString* sourceCode);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
MorphString* mph_compile(MorphContext* ctx, void* _env_void, MorphString* sourceCode) {
	mph_gc_push_root(ctx, (void**)&sourceCode);
	Env_mph__compile* _env = (Env_mph__compile*)_env_void;
	mph_native_print(ctx, mph_string_new(ctx, "Starting compilation pipeline..."));
	mph_native_print(ctx, mph_string_new(ctx, "Tokenizing..."));
	mph_int tokenCount = 0;
	mph_int pos = 0;
	while ((pos < ({ MorphString* _len_arg_1 = sourceCode; mph_gc_push_root(ctx, (void**)&_len_arg_1); mph_int _ret_2 = ((MorphString*)_len_arg_1)->length; mph_gc_pop_roots(ctx, 1); _ret_2; }))) {
{
	mph_int ch = ({ MorphString* _str_3 = sourceCode; mph_gc_push_root(ctx, (void**)&_str_3); mph_int _ret_4 = ((MorphString*)_str_3)->data[pos]; mph_gc_pop_roots(ctx, 1); _ret_4; });
	if ((ch == 32)) {
{
	pos = (pos + 1);
}
	} else if ((ch == 9)) {
{
	pos = (pos + 1);
}
	} else if ((ch == 10)) {
{
	pos = (pos + 1);
}
	} else {
{
	tokenCount = (tokenCount + 1);
	pos = (pos + 1);
}
	}
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Tokens found:"));
	mph_native_print_int(ctx, tokenCount);
	mph_native_print(ctx, mph_string_new(ctx, "Parsing..."));
	mph_bool hasFunction = 0;
	if ((({ MorphString* _len_arg_5 = sourceCode; mph_gc_push_root(ctx, (void**)&_len_arg_5); mph_int _ret_6 = ((MorphString*)_len_arg_5)->length; mph_gc_pop_roots(ctx, 1); _ret_6; }) > 6)) {
{
	MorphString* start = mph_string_substring(ctx, sourceCode, 0, 6);
	mph_gc_push_root(ctx, (void**)&start);
	if (mph_string_eq(ctx, start, mph_string_new(ctx, "fungsi"))) {
{
	hasFunction = 1;
}
	}
	mph_gc_pop_roots(ctx, 1);
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Generating C code..."));
	MorphString* cCode = mph_string_new(ctx, "#include <stdio.h>\n");
	mph_gc_push_root(ctx, (void**)&cCode);
	if (hasFunction) {
{
	cCode = ({ MorphString* _lhs_7 = cCode; MorphString* _rhs_8 = mph_string_new(ctx, "int main() {\n"); mph_gc_push_root(ctx, (void**)&_lhs_7); mph_gc_push_root(ctx, (void**)&_rhs_8); MorphString* _ret_9 = mph_string_concat(ctx, _lhs_7, _rhs_8); mph_gc_pop_roots(ctx, 2); _ret_9; });
	cCode = ({ MorphString* _lhs_10 = cCode; MorphString* _rhs_11 = mph_string_new(ctx, "    printf(\"Hello from compiled Morph!\n\");\n"); mph_gc_push_root(ctx, (void**)&_lhs_10); mph_gc_push_root(ctx, (void**)&_rhs_11); MorphString* _ret_12 = mph_string_concat(ctx, _lhs_10, _rhs_11); mph_gc_pop_roots(ctx, 2); _ret_12; });
	cCode = ({ MorphString* _lhs_13 = cCode; MorphString* _rhs_14 = mph_string_new(ctx, "    return 0;\n"); mph_gc_push_root(ctx, (void**)&_lhs_13); mph_gc_push_root(ctx, (void**)&_rhs_14); MorphString* _ret_15 = mph_string_concat(ctx, _lhs_13, _rhs_14); mph_gc_pop_roots(ctx, 2); _ret_15; });
	cCode = ({ MorphString* _lhs_16 = cCode; MorphString* _rhs_17 = mph_string_new(ctx, "}\n"); mph_gc_push_root(ctx, (void**)&_lhs_16); mph_gc_push_root(ctx, (void**)&_rhs_17); MorphString* _ret_18 = mph_string_concat(ctx, _lhs_16, _rhs_17); mph_gc_pop_roots(ctx, 2); _ret_18; });
}
	} else {
{
	cCode = ({ MorphString* _lhs_19 = cCode; MorphString* _rhs_20 = mph_string_new(ctx, "int main() { return 0; }\n"); mph_gc_push_root(ctx, (void**)&_lhs_19); mph_gc_push_root(ctx, (void**)&_rhs_20); MorphString* _ret_21 = mph_string_concat(ctx, _lhs_19, _rhs_20); mph_gc_pop_roots(ctx, 2); _ret_21; });
}
	}
	return ({ MorphString* _ret_22 = cCode; mph_gc_push_root(ctx, (void**)&_ret_22); MorphString* _ret_23 = _ret_22; mph_gc_pop_roots(ctx, 1); _ret_23; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Morph Integrated Compiler ==="));
	MorphString* input = mph_string_new(ctx, "fungsi hello() void native_print(\"test\") akhir");
	mph_gc_push_root(ctx, (void**)&input);
	mph_native_print(ctx, mph_string_new(ctx, "Source:"));
	mph_native_print(ctx, input);
	MorphString* result = ({ MorphString* _arg_24 = input; mph_gc_push_root(ctx, (void**)&_arg_24); MorphString* _ret_25 = mph_compile(ctx, NULL, _arg_24); mph_gc_pop_roots(ctx, 1); _ret_25; });
	mph_gc_push_root(ctx, (void**)&result);
	mph_native_print(ctx, mph_string_new(ctx, "Generated C:"));
	mph_native_print(ctx, result);
	mph_native_print(ctx, mph_string_new(ctx, "=== Integration successful! ==="));
	mph_gc_pop_roots(ctx, 2);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
	mph_main(ctx, NULL);
}
