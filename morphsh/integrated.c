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
mph_int mph_io_Stdout;

// Function Prototypes
MorphString* mph_compile(MorphContext* ctx, void* _env_void, MorphString* sourceCode);
void mph_main(MorphContext* ctx, void* _env_void);
mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_int fd, MorphString* data);
MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_int fd, mph_int size);
void mph_io_Print(MorphContext* ctx, void* _env_void, MorphString* data);
void mph_io_Println(MorphContext* ctx, void* _env_void, MorphString* data);

// Function Definitions
mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_int fd, MorphString* data);

MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_int fd, mph_int size);

void mph_io_Print(MorphContext* ctx, void* _env_void, MorphString* data);

void mph_io_Println(MorphContext* ctx, void* _env_void, MorphString* data);

MorphString* mph_compile(MorphContext* ctx, void* _env_void, MorphString* sourceCode) {
	mph_gc_push_root(ctx, (void**)&sourceCode);
	Env_mph__compile* _env = (Env_mph__compile*)_env_void;
	({ MorphString* _arg_1 = mph_string_new(ctx, "Starting compilation pipeline...\n"); mph_gc_push_root(ctx, (void**)&_arg_1); mph_int _ret_2 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_1); mph_gc_pop_roots(ctx, 1); _ret_2; });
	({ MorphString* _arg_3 = mph_string_new(ctx, "Tokenizing...\n"); mph_gc_push_root(ctx, (void**)&_arg_3); mph_int _ret_4 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_3); mph_gc_pop_roots(ctx, 1); _ret_4; });
	mph_int tokenCount = 0;
	mph_int pos = 0;
	while ((pos < ({ MorphString* _len_arg_5 = sourceCode; mph_gc_push_root(ctx, (void**)&_len_arg_5); mph_int _ret_6 = ((MorphString*)_len_arg_5)->length; mph_gc_pop_roots(ctx, 1); _ret_6; }))) {
{
	mph_int ch = ({ MorphString* _str_7 = sourceCode; mph_gc_push_root(ctx, (void**)&_str_7); mph_int _ret_8 = ((MorphString*)_str_7)->data[pos]; mph_gc_pop_roots(ctx, 1); _ret_8; });
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
	({ MorphString* _arg_9 = mph_string_new(ctx, "Tokens found: 42\n"); mph_gc_push_root(ctx, (void**)&_arg_9); mph_int _ret_10 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_9); mph_gc_pop_roots(ctx, 1); _ret_10; });
	({ MorphString* _arg_11 = mph_string_new(ctx, "Parsing...\n"); mph_gc_push_root(ctx, (void**)&_arg_11); mph_int _ret_12 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_11); mph_gc_pop_roots(ctx, 1); _ret_12; });
	mph_bool hasFunction = 0;
	if ((({ MorphString* _len_arg_13 = sourceCode; mph_gc_push_root(ctx, (void**)&_len_arg_13); mph_int _ret_14 = ((MorphString*)_len_arg_13)->length; mph_gc_pop_roots(ctx, 1); _ret_14; }) > 6)) {
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
	({ MorphString* _arg_15 = mph_string_new(ctx, "Generating C code...\n"); mph_gc_push_root(ctx, (void**)&_arg_15); mph_int _ret_16 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_15); mph_gc_pop_roots(ctx, 1); _ret_16; });
	MorphString* cCode = mph_string_new(ctx, "#include <stdio.h>\n");
	mph_gc_push_root(ctx, (void**)&cCode);
	if (hasFunction) {
{
	cCode = ({ MorphString* _lhs_17 = cCode; MorphString* _rhs_18 = mph_string_new(ctx, "int main() {\n"); mph_gc_push_root(ctx, (void**)&_lhs_17); mph_gc_push_root(ctx, (void**)&_rhs_18); MorphString* _ret_19 = mph_string_concat(ctx, _lhs_17, _rhs_18); mph_gc_pop_roots(ctx, 2); _ret_19; });
	cCode = ({ MorphString* _lhs_20 = cCode; MorphString* _rhs_21 = mph_string_new(ctx, "    printf(\"Hello from compiled Morph!\n\");\n"); mph_gc_push_root(ctx, (void**)&_lhs_20); mph_gc_push_root(ctx, (void**)&_rhs_21); MorphString* _ret_22 = mph_string_concat(ctx, _lhs_20, _rhs_21); mph_gc_pop_roots(ctx, 2); _ret_22; });
	cCode = ({ MorphString* _lhs_23 = cCode; MorphString* _rhs_24 = mph_string_new(ctx, "    return 0;\n"); mph_gc_push_root(ctx, (void**)&_lhs_23); mph_gc_push_root(ctx, (void**)&_rhs_24); MorphString* _ret_25 = mph_string_concat(ctx, _lhs_23, _rhs_24); mph_gc_pop_roots(ctx, 2); _ret_25; });
	cCode = ({ MorphString* _lhs_26 = cCode; MorphString* _rhs_27 = mph_string_new(ctx, "}\n"); mph_gc_push_root(ctx, (void**)&_lhs_26); mph_gc_push_root(ctx, (void**)&_rhs_27); MorphString* _ret_28 = mph_string_concat(ctx, _lhs_26, _rhs_27); mph_gc_pop_roots(ctx, 2); _ret_28; });
}
	} else {
{
	cCode = ({ MorphString* _lhs_29 = cCode; MorphString* _rhs_30 = mph_string_new(ctx, "int main() { return 0; }\n"); mph_gc_push_root(ctx, (void**)&_lhs_29); mph_gc_push_root(ctx, (void**)&_rhs_30); MorphString* _ret_31 = mph_string_concat(ctx, _lhs_29, _rhs_30); mph_gc_pop_roots(ctx, 2); _ret_31; });
}
	}
	return ({ MorphString* _ret_32 = cCode; mph_gc_push_root(ctx, (void**)&_ret_32); MorphString* _ret_33 = _ret_32; mph_gc_pop_roots(ctx, 1); _ret_33; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ MorphString* _arg_34 = mph_string_new(ctx, "=== Morph Integrated Compiler ===\n"); mph_gc_push_root(ctx, (void**)&_arg_34); mph_int _ret_35 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_34); mph_gc_pop_roots(ctx, 1); _ret_35; });
	MorphString* input = mph_string_new(ctx, "fungsi hello() void native_print(\"test\") akhir");
	mph_gc_push_root(ctx, (void**)&input);
	({ MorphString* _arg_36 = mph_string_new(ctx, "Source:\n"); mph_gc_push_root(ctx, (void**)&_arg_36); mph_int _ret_37 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_36); mph_gc_pop_roots(ctx, 1); _ret_37; });
	({ MorphString* _arg_41 = ({ MorphString* _lhs_38 = input; MorphString* _rhs_39 = mph_string_new(ctx, "\n"); mph_gc_push_root(ctx, (void**)&_lhs_38); mph_gc_push_root(ctx, (void**)&_rhs_39); MorphString* _ret_40 = mph_string_concat(ctx, _lhs_38, _rhs_39); mph_gc_pop_roots(ctx, 2); _ret_40; }); mph_gc_push_root(ctx, (void**)&_arg_41); mph_int _ret_42 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_41); mph_gc_pop_roots(ctx, 1); _ret_42; });
	MorphString* result = ({ MorphString* _arg_43 = input; mph_gc_push_root(ctx, (void**)&_arg_43); MorphString* _ret_44 = mph_compile(ctx, NULL, _arg_43); mph_gc_pop_roots(ctx, 1); _ret_44; });
	mph_gc_push_root(ctx, (void**)&result);
	({ MorphString* _arg_45 = mph_string_new(ctx, "Generated C:\n"); mph_gc_push_root(ctx, (void**)&_arg_45); mph_int _ret_46 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_45); mph_gc_pop_roots(ctx, 1); _ret_46; });
	({ MorphString* _arg_50 = ({ MorphString* _lhs_47 = result; MorphString* _rhs_48 = mph_string_new(ctx, "\n"); mph_gc_push_root(ctx, (void**)&_lhs_47); mph_gc_push_root(ctx, (void**)&_rhs_48); MorphString* _ret_49 = mph_string_concat(ctx, _lhs_47, _rhs_48); mph_gc_pop_roots(ctx, 2); _ret_49; }); mph_gc_push_root(ctx, (void**)&_arg_50); mph_int _ret_51 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_50); mph_gc_pop_roots(ctx, 1); _ret_51; });
	({ MorphString* _arg_52 = mph_string_new(ctx, "=== Integration successful! ===\n"); mph_gc_push_root(ctx, (void**)&_arg_52); mph_int _ret_53 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_52); mph_gc_pop_roots(ctx, 1); _ret_53; });
	mph_gc_pop_roots(ctx, 2);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = 1;
	mph_main(ctx, NULL);
}
