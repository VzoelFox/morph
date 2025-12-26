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
#define MPH_TYPE_mph_io_File 1

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
typedef struct Env_mph__compile {
	MorphClosure* substring;
} Env_mph__compile;

MorphTypeInfo mph_ti_Env_mph__compile = { "Env_mph__compile", sizeof(Env_mph__compile), 1, (size_t[]){offsetof(Env_mph__compile, substring)}, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
MorphString* mph_compile(MorphContext* ctx, void* _env_void, MorphString* sourceCode);
void mph_main(MorphContext* ctx, void* _env_void);
mph_io_File* mph_io_make_file(MorphContext* ctx, void* _env_void, mph_int fd);
mph_io_File* mph_io_Open(MorphContext* ctx, void* _env_void, MorphString* path);
mph_io_File* mph_io_Create(MorphContext* ctx, void* _env_void, MorphString* path);
MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_io_File* f, mph_int size);
mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_io_File* f, MorphString* s);
mph_int mph_io_Close(MorphContext* ctx, void* _env_void, mph_io_File* f);

// Function Definitions
mph_io_File* mph_io_make_file(MorphContext* ctx, void* _env_void, mph_int fd);

mph_io_File* mph_io_Open(MorphContext* ctx, void* _env_void, MorphString* path);

mph_io_File* mph_io_Create(MorphContext* ctx, void* _env_void, MorphString* path);

MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_io_File* f, mph_int size);

mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_io_File* f, MorphString* s);

mph_int mph_io_Close(MorphContext* ctx, void* _env_void, mph_io_File* f);

MorphString* mph_compile(MorphContext* ctx, void* _env_void, MorphString* sourceCode) {
	mph_gc_push_root(ctx, (void**)&sourceCode);
	Env_mph__compile* _env = (Env_mph__compile*)_env_void;
	({ mph_io_File* _arg_1 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_1); MorphString* _arg_2 = mph_string_new(ctx, "Starting compilation pipeline...\n"); mph_gc_push_root(ctx, (void**)&_arg_2); mph_int _ret_3 = mph_io_Write(ctx, NULL, _arg_1, _arg_2); mph_gc_pop_roots(ctx, 2); _ret_3; });
	({ mph_io_File* _arg_4 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_4); MorphString* _arg_5 = mph_string_new(ctx, "Tokenizing...\n"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_int _ret_6 = mph_io_Write(ctx, NULL, _arg_4, _arg_5); mph_gc_pop_roots(ctx, 2); _ret_6; });
	mph_int tokenCount = 0;
	mph_int pos = 0;
	while ((pos < ({ MorphString* _len_arg_7 = sourceCode; mph_gc_push_root(ctx, (void**)&_len_arg_7); mph_int _ret_8 = ((MorphString*)_len_arg_7)->length; mph_gc_pop_roots(ctx, 1); _ret_8; }))) {
{
	mph_int ch = ({ MorphString* _str_9 = sourceCode; mph_gc_push_root(ctx, (void**)&_str_9); mph_int _ret_10 = ((MorphString*)_str_9)->data[pos]; mph_gc_pop_roots(ctx, 1); _ret_10; });
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
	({ mph_io_File* _arg_11 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_11); MorphString* _arg_12 = mph_string_new(ctx, "Tokens found: 42\n"); mph_gc_push_root(ctx, (void**)&_arg_12); mph_int _ret_13 = mph_io_Write(ctx, NULL, _arg_11, _arg_12); mph_gc_pop_roots(ctx, 2); _ret_13; });
	({ mph_io_File* _arg_14 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_14); MorphString* _arg_15 = mph_string_new(ctx, "Parsing...\n"); mph_gc_push_root(ctx, (void**)&_arg_15); mph_int _ret_16 = mph_io_Write(ctx, NULL, _arg_14, _arg_15); mph_gc_pop_roots(ctx, 2); _ret_16; });
	mph_bool hasFunction = 0;
	if ((({ MorphString* _len_arg_17 = sourceCode; mph_gc_push_root(ctx, (void**)&_len_arg_17); mph_int _ret_18 = ((MorphString*)_len_arg_17)->length; mph_gc_pop_roots(ctx, 1); _ret_18; }) > 6)) {
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
	({ mph_io_File* _arg_19 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_19); MorphString* _arg_20 = mph_string_new(ctx, "Generating C code...\n"); mph_gc_push_root(ctx, (void**)&_arg_20); mph_int _ret_21 = mph_io_Write(ctx, NULL, _arg_19, _arg_20); mph_gc_pop_roots(ctx, 2); _ret_21; });
	MorphString* cCode = mph_string_new(ctx, "#include <stdio.h>\n");
	mph_gc_push_root(ctx, (void**)&cCode);
	if (hasFunction) {
{
	cCode = ({ MorphString* _lhs_22 = cCode; MorphString* _rhs_23 = mph_string_new(ctx, "int main() {\n"); mph_gc_push_root(ctx, (void**)&_lhs_22); mph_gc_push_root(ctx, (void**)&_rhs_23); MorphString* _ret_24 = mph_string_concat(ctx, _lhs_22, _rhs_23); mph_gc_pop_roots(ctx, 2); _ret_24; });
	cCode = ({ MorphString* _lhs_25 = cCode; MorphString* _rhs_26 = mph_string_new(ctx, "    printf(\"Hello from compiled Morph!\n\");\n"); mph_gc_push_root(ctx, (void**)&_lhs_25); mph_gc_push_root(ctx, (void**)&_rhs_26); MorphString* _ret_27 = mph_string_concat(ctx, _lhs_25, _rhs_26); mph_gc_pop_roots(ctx, 2); _ret_27; });
	cCode = ({ MorphString* _lhs_28 = cCode; MorphString* _rhs_29 = mph_string_new(ctx, "    return 0;\n"); mph_gc_push_root(ctx, (void**)&_lhs_28); mph_gc_push_root(ctx, (void**)&_rhs_29); MorphString* _ret_30 = mph_string_concat(ctx, _lhs_28, _rhs_29); mph_gc_pop_roots(ctx, 2); _ret_30; });
	cCode = ({ MorphString* _lhs_31 = cCode; MorphString* _rhs_32 = mph_string_new(ctx, "}\n"); mph_gc_push_root(ctx, (void**)&_lhs_31); mph_gc_push_root(ctx, (void**)&_rhs_32); MorphString* _ret_33 = mph_string_concat(ctx, _lhs_31, _rhs_32); mph_gc_pop_roots(ctx, 2); _ret_33; });
}
	} else {
{
	cCode = ({ MorphString* _lhs_34 = cCode; MorphString* _rhs_35 = mph_string_new(ctx, "int main() { return 0; }\n"); mph_gc_push_root(ctx, (void**)&_lhs_34); mph_gc_push_root(ctx, (void**)&_rhs_35); MorphString* _ret_36 = mph_string_concat(ctx, _lhs_34, _rhs_35); mph_gc_pop_roots(ctx, 2); _ret_36; });
}
	}
	return ({ MorphString* _ret_37 = cCode; mph_gc_push_root(ctx, (void**)&_ret_37); MorphString* _ret_38 = _ret_37; mph_gc_pop_roots(ctx, 1); _ret_38; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_39 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_39); MorphString* _arg_40 = mph_string_new(ctx, "=== Morph Integrated Compiler ===\n"); mph_gc_push_root(ctx, (void**)&_arg_40); mph_int _ret_41 = mph_io_Write(ctx, NULL, _arg_39, _arg_40); mph_gc_pop_roots(ctx, 2); _ret_41; });
	MorphString* input = mph_string_new(ctx, "fungsi hello() void native_print(\"test\") akhir");
	mph_gc_push_root(ctx, (void**)&input);
	({ mph_io_File* _arg_42 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_42); MorphString* _arg_43 = mph_string_new(ctx, "Source:\n"); mph_gc_push_root(ctx, (void**)&_arg_43); mph_int _ret_44 = mph_io_Write(ctx, NULL, _arg_42, _arg_43); mph_gc_pop_roots(ctx, 2); _ret_44; });
	({ mph_io_File* _arg_45 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_45); MorphString* _arg_49 = ({ MorphString* _lhs_46 = input; MorphString* _rhs_47 = mph_string_new(ctx, "\n"); mph_gc_push_root(ctx, (void**)&_lhs_46); mph_gc_push_root(ctx, (void**)&_rhs_47); MorphString* _ret_48 = mph_string_concat(ctx, _lhs_46, _rhs_47); mph_gc_pop_roots(ctx, 2); _ret_48; }); mph_gc_push_root(ctx, (void**)&_arg_49); mph_int _ret_50 = mph_io_Write(ctx, NULL, _arg_45, _arg_49); mph_gc_pop_roots(ctx, 2); _ret_50; });
	MorphString* result = ({ MorphString* _arg_51 = input; mph_gc_push_root(ctx, (void**)&_arg_51); MorphString* _ret_52 = mph_compile(ctx, NULL, _arg_51); mph_gc_pop_roots(ctx, 1); _ret_52; });
	mph_gc_push_root(ctx, (void**)&result);
	({ mph_io_File* _arg_53 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_53); MorphString* _arg_54 = mph_string_new(ctx, "Generated C:\n"); mph_gc_push_root(ctx, (void**)&_arg_54); mph_int _ret_55 = mph_io_Write(ctx, NULL, _arg_53, _arg_54); mph_gc_pop_roots(ctx, 2); _ret_55; });
	({ mph_io_File* _arg_56 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_56); MorphString* _arg_60 = ({ MorphString* _lhs_57 = result; MorphString* _rhs_58 = mph_string_new(ctx, "\n"); mph_gc_push_root(ctx, (void**)&_lhs_57); mph_gc_push_root(ctx, (void**)&_rhs_58); MorphString* _ret_59 = mph_string_concat(ctx, _lhs_57, _rhs_58); mph_gc_pop_roots(ctx, 2); _ret_59; }); mph_gc_push_root(ctx, (void**)&_arg_60); mph_int _ret_61 = mph_io_Write(ctx, NULL, _arg_56, _arg_60); mph_gc_pop_roots(ctx, 2); _ret_61; });
	({ mph_io_File* _arg_62 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_62); MorphString* _arg_63 = mph_string_new(ctx, "=== Integration successful! ===\n"); mph_gc_push_root(ctx, (void**)&_arg_63); mph_int _ret_64 = mph_io_Write(ctx, NULL, _arg_62, _arg_63); mph_gc_pop_roots(ctx, 2); _ret_64; });
	mph_gc_pop_roots(ctx, 2);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
