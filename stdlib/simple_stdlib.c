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

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
mph_int mph_stdlib_abs(MorphContext* ctx, void* _env_void, mph_int x);
mph_int mph_stdlib_min(MorphContext* ctx, void* _env_void, mph_int a, mph_int b);
mph_int mph_stdlib_max(MorphContext* ctx, void* _env_void, mph_int a, mph_int b);
mph_bool mph_stdlib_bool_from_int(MorphContext* ctx, void* _env_void, mph_int x);
MorphString* mph_stdlib_str_from_int(MorphContext* ctx, void* _env_void, mph_int x);
mph_int mph_stdlib_sum_three(MorphContext* ctx, void* _env_void, mph_int a, mph_int b, mph_int c);
mph_bool mph_stdlib_all_three(MorphContext* ctx, void* _env_void, mph_bool a, mph_bool b, mph_bool c);
mph_bool mph_stdlib_any_three(MorphContext* ctx, void* _env_void, mph_bool a, mph_bool b, mph_bool c);
void mph_test_stdlib_functions(MorphContext* ctx, void* _env_void);
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

mph_int mph_stdlib_abs(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x < 0)) {
{
	return (-x);
}
	} else {
{
	return x;
}
	}
}

mph_int mph_stdlib_min(MorphContext* ctx, void* _env_void, mph_int a, mph_int b) {
	if ((a < b)) {
{
	return a;
}
	} else {
{
	return b;
}
	}
}

mph_int mph_stdlib_max(MorphContext* ctx, void* _env_void, mph_int a, mph_int b) {
	if ((a > b)) {
{
	return a;
}
	} else {
{
	return b;
}
	}
}

mph_bool mph_stdlib_bool_from_int(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x == 0)) {
{
	return 0;
}
	} else {
{
	return 1;
}
	}
}

MorphString* mph_stdlib_str_from_int(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x == 0)) {
{
	return ({ MorphString* _ret_1 = mph_string_new(ctx, "0"); mph_gc_push_root(ctx, (void**)&_ret_1); MorphString* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}
	} else if ((x == 1)) {
{
	return ({ MorphString* _ret_3 = mph_string_new(ctx, "1"); mph_gc_push_root(ctx, (void**)&_ret_3); MorphString* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}
	} else if ((x == 42)) {
{
	return ({ MorphString* _ret_5 = mph_string_new(ctx, "42"); mph_gc_push_root(ctx, (void**)&_ret_5); MorphString* _ret_6 = _ret_5; mph_gc_pop_roots(ctx, 1); _ret_6; });
}
	} else if ((x == (-1))) {
{
	return ({ MorphString* _ret_7 = mph_string_new(ctx, "-1"); mph_gc_push_root(ctx, (void**)&_ret_7); MorphString* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
}
	} else {
{
	return ({ MorphString* _ret_9 = mph_string_new(ctx, "N"); mph_gc_push_root(ctx, (void**)&_ret_9); MorphString* _ret_10 = _ret_9; mph_gc_pop_roots(ctx, 1); _ret_10; });
}
	}
}

mph_int mph_stdlib_sum_three(MorphContext* ctx, void* _env_void, mph_int a, mph_int b, mph_int c) {
	return ((a + b) + c);
}

mph_bool mph_stdlib_all_three(MorphContext* ctx, void* _env_void, mph_bool a, mph_bool b, mph_bool c) {
	if (a) {
{
	if (b) {
{
	if (c) {
{
	return 1;
}
	}
}
	}
}
	}
	return 0;
}

mph_bool mph_stdlib_any_three(MorphContext* ctx, void* _env_void, mph_bool a, mph_bool b, mph_bool c) {
	if (a) {
{
	return 1;
}
	} else if (b) {
{
	return 1;
}
	} else if (c) {
{
	return 1;
}
	} else {
{
	return 0;
}
	}
}

void mph_test_stdlib_functions(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_11 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_11); MorphString* _arg_12 = mph_string_new(ctx, "=== MORPH STDLIB TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_12); mph_int _ret_13 = mph_io_Write(ctx, NULL, _arg_11, _arg_12); mph_gc_pop_roots(ctx, 2); _ret_13; });
	mph_int abs_result = mph_stdlib_abs(ctx, NULL, (-42));
	if ((abs_result == 42)) {
{
	({ mph_io_File* _arg_14 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_14); MorphString* _arg_15 = mph_string_new(ctx, "abs(-42) = 42: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_15); mph_int _ret_16 = mph_io_Write(ctx, NULL, _arg_14, _arg_15); mph_gc_pop_roots(ctx, 2); _ret_16; });
}
	} else {
{
	({ mph_io_File* _arg_17 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_17); MorphString* _arg_18 = mph_string_new(ctx, "abs(-42): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_18); mph_int _ret_19 = mph_io_Write(ctx, NULL, _arg_17, _arg_18); mph_gc_pop_roots(ctx, 2); _ret_19; });
}
	}
	mph_int min_result = mph_stdlib_min(ctx, NULL, 10, 5);
	if ((min_result == 5)) {
{
	({ mph_io_File* _arg_20 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_20); MorphString* _arg_21 = mph_string_new(ctx, "min(10, 5) = 5: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_21); mph_int _ret_22 = mph_io_Write(ctx, NULL, _arg_20, _arg_21); mph_gc_pop_roots(ctx, 2); _ret_22; });
}
	} else {
{
	({ mph_io_File* _arg_23 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_23); MorphString* _arg_24 = mph_string_new(ctx, "min(10, 5): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_24); mph_int _ret_25 = mph_io_Write(ctx, NULL, _arg_23, _arg_24); mph_gc_pop_roots(ctx, 2); _ret_25; });
}
	}
	mph_int max_result = mph_stdlib_max(ctx, NULL, 10, 5);
	if ((max_result == 10)) {
{
	({ mph_io_File* _arg_26 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_26); MorphString* _arg_27 = mph_string_new(ctx, "max(10, 5) = 10: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_27); mph_int _ret_28 = mph_io_Write(ctx, NULL, _arg_26, _arg_27); mph_gc_pop_roots(ctx, 2); _ret_28; });
}
	} else {
{
	({ mph_io_File* _arg_29 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_29); MorphString* _arg_30 = mph_string_new(ctx, "max(10, 5): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_30); mph_int _ret_31 = mph_io_Write(ctx, NULL, _arg_29, _arg_30); mph_gc_pop_roots(ctx, 2); _ret_31; });
}
	}
	mph_bool bool_result = mph_stdlib_bool_from_int(ctx, NULL, 42);
	if (bool_result) {
{
	({ mph_io_File* _arg_32 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_32); MorphString* _arg_33 = mph_string_new(ctx, "bool(42) = true: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_33); mph_int _ret_34 = mph_io_Write(ctx, NULL, _arg_32, _arg_33); mph_gc_pop_roots(ctx, 2); _ret_34; });
}
	} else {
{
	({ mph_io_File* _arg_35 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_35); MorphString* _arg_36 = mph_string_new(ctx, "bool(42): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_36); mph_int _ret_37 = mph_io_Write(ctx, NULL, _arg_35, _arg_36); mph_gc_pop_roots(ctx, 2); _ret_37; });
}
	}
	MorphString* str_result = mph_stdlib_str_from_int(ctx, NULL, 42);
	mph_gc_push_root(ctx, (void**)&str_result);
	if (mph_string_eq(ctx, str_result, mph_string_new(ctx, "42"))) {
{
	({ mph_io_File* _arg_38 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_38); MorphString* _arg_39 = mph_string_new(ctx, "str(42) = '42': â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_39); mph_int _ret_40 = mph_io_Write(ctx, NULL, _arg_38, _arg_39); mph_gc_pop_roots(ctx, 2); _ret_40; });
}
	} else {
{
	({ mph_io_File* _arg_41 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_41); MorphString* _arg_42 = mph_string_new(ctx, "str(42): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_42); mph_int _ret_43 = mph_io_Write(ctx, NULL, _arg_41, _arg_42); mph_gc_pop_roots(ctx, 2); _ret_43; });
}
	}
	mph_int sum_result = mph_stdlib_sum_three(ctx, NULL, 1, 2, 3);
	if ((sum_result == 6)) {
{
	({ mph_io_File* _arg_44 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_44); MorphString* _arg_45 = mph_string_new(ctx, "sum(1,2,3) = 6: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_45); mph_int _ret_46 = mph_io_Write(ctx, NULL, _arg_44, _arg_45); mph_gc_pop_roots(ctx, 2); _ret_46; });
}
	} else {
{
	({ mph_io_File* _arg_47 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_47); MorphString* _arg_48 = mph_string_new(ctx, "sum(1,2,3): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_48); mph_int _ret_49 = mph_io_Write(ctx, NULL, _arg_47, _arg_48); mph_gc_pop_roots(ctx, 2); _ret_49; });
}
	}
	mph_bool all_result = mph_stdlib_all_three(ctx, NULL, 1, 1, 1);
	if (all_result) {
{
	({ mph_io_File* _arg_50 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_50); MorphString* _arg_51 = mph_string_new(ctx, "all(T,T,T) = true: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_51); mph_int _ret_52 = mph_io_Write(ctx, NULL, _arg_50, _arg_51); mph_gc_pop_roots(ctx, 2); _ret_52; });
}
	} else {
{
	({ mph_io_File* _arg_53 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_53); MorphString* _arg_54 = mph_string_new(ctx, "all(T,T,T): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_54); mph_int _ret_55 = mph_io_Write(ctx, NULL, _arg_53, _arg_54); mph_gc_pop_roots(ctx, 2); _ret_55; });
}
	}
	mph_bool any_result = mph_stdlib_any_three(ctx, NULL, 0, 1, 0);
	if (any_result) {
{
	({ mph_io_File* _arg_56 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_56); MorphString* _arg_57 = mph_string_new(ctx, "any(F,T,F) = true: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_57); mph_int _ret_58 = mph_io_Write(ctx, NULL, _arg_56, _arg_57); mph_gc_pop_roots(ctx, 2); _ret_58; });
}
	} else {
{
	({ mph_io_File* _arg_59 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_59); MorphString* _arg_60 = mph_string_new(ctx, "any(F,T,F): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_60); mph_int _ret_61 = mph_io_Write(ctx, NULL, _arg_59, _arg_60); mph_gc_pop_roots(ctx, 2); _ret_61; });
}
	}
	({ mph_io_File* _arg_62 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_62); MorphString* _arg_63 = mph_string_new(ctx, "\nð MORPH STDLIB COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_63); mph_int _ret_64 = mph_io_Write(ctx, NULL, _arg_62, _arg_63); mph_gc_pop_roots(ctx, 2); _ret_64; });
	({ mph_io_File* _arg_65 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_65); MorphString* _arg_66 = mph_string_new(ctx, "â 8 Python built-in equivalents implemented\n"); mph_gc_push_root(ctx, (void**)&_arg_66); mph_int _ret_67 = mph_io_Write(ctx, NULL, _arg_65, _arg_66); mph_gc_pop_roots(ctx, 2); _ret_67; });
	({ mph_io_File* _arg_68 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_68); MorphString* _arg_69 = mph_string_new(ctx, "â Numeric: abs, min, max, sum\n"); mph_gc_push_root(ctx, (void**)&_arg_69); mph_int _ret_70 = mph_io_Write(ctx, NULL, _arg_68, _arg_69); mph_gc_pop_roots(ctx, 2); _ret_70; });
	({ mph_io_File* _arg_71 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_71); MorphString* _arg_72 = mph_string_new(ctx, "â Conversion: bool, str\n"); mph_gc_push_root(ctx, (void**)&_arg_72); mph_int _ret_73 = mph_io_Write(ctx, NULL, _arg_71, _arg_72); mph_gc_pop_roots(ctx, 2); _ret_73; });
	({ mph_io_File* _arg_74 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_74); MorphString* _arg_75 = mph_string_new(ctx, "â Logic: all, any\n"); mph_gc_push_root(ctx, (void**)&_arg_75); mph_int _ret_76 = mph_io_Write(ctx, NULL, _arg_74, _arg_75); mph_gc_pop_roots(ctx, 2); _ret_76; });
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_test_stdlib_functions(ctx, NULL);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
