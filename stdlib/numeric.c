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
mph_int mph_abs(MorphContext* ctx, void* _env_void, mph_int x);
mph_float mph_abs_float(MorphContext* ctx, void* _env_void, mph_float x);
mph_int mph_min(MorphContext* ctx, void* _env_void, mph_int a, mph_int b);
mph_int mph_max(MorphContext* ctx, void* _env_void, mph_int a, mph_int b);
mph_int mph_pow(MorphContext* ctx, void* _env_void, mph_int base, mph_int exp);
mph_int mph_round_int(MorphContext* ctx, void* _env_void, mph_int x);
mph_int mph_sum_three(MorphContext* ctx, void* _env_void, mph_int a, mph_int b, mph_int c);
mph_int mph_sum_five(MorphContext* ctx, void* _env_void, mph_int a, mph_int b, mph_int c, mph_int d, mph_int e);
void mph_test_numeric_functions(MorphContext* ctx, void* _env_void);
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

mph_int mph_abs(MorphContext* ctx, void* _env_void, mph_int x) {
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

mph_float mph_abs_float(MorphContext* ctx, void* _env_void, mph_float x) {
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

mph_int mph_min(MorphContext* ctx, void* _env_void, mph_int a, mph_int b) {
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

mph_int mph_max(MorphContext* ctx, void* _env_void, mph_int a, mph_int b) {
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

mph_int mph_pow(MorphContext* ctx, void* _env_void, mph_int base, mph_int exp) {
	if ((exp == 0)) {
{
	return 1;
}
	} else if ((exp == 1)) {
{
	return base;
}
	} else if ((exp == 2)) {
{
	return (base * base);
}
	} else if ((exp == 3)) {
{
	return ((base * base) * base);
}
	} else {
{
	mph_int result = 1;
	mph_int i = 0;
	while ((i < exp)) {
{
	result = (result * base);
	i = (i + 1);
}
	}
	return result;
}
	}
}

mph_int mph_round_int(MorphContext* ctx, void* _env_void, mph_int x) {
	return x;
}

mph_int mph_sum_three(MorphContext* ctx, void* _env_void, mph_int a, mph_int b, mph_int c) {
	return ((a + b) + c);
}

mph_int mph_sum_five(MorphContext* ctx, void* _env_void, mph_int a, mph_int b, mph_int c, mph_int d, mph_int e) {
	return ((((a + b) + c) + d) + e);
}

void mph_test_numeric_functions(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_1 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_1); MorphString* _arg_2 = mph_string_new(ctx, "=== NUMERIC FUNCTIONS TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_2); mph_int _ret_3 = mph_io_Write(ctx, NULL, _arg_1, _arg_2); mph_gc_pop_roots(ctx, 2); _ret_3; });
	mph_int abs_result = mph_abs(ctx, NULL, (-42));
	if ((abs_result == 42)) {
{
	({ mph_io_File* _arg_4 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_4); MorphString* _arg_5 = mph_string_new(ctx, "abs(-42) = 42: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_int _ret_6 = mph_io_Write(ctx, NULL, _arg_4, _arg_5); mph_gc_pop_roots(ctx, 2); _ret_6; });
}
	} else {
{
	({ mph_io_File* _arg_7 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_7); MorphString* _arg_8 = mph_string_new(ctx, "abs(-42): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_8); mph_int _ret_9 = mph_io_Write(ctx, NULL, _arg_7, _arg_8); mph_gc_pop_roots(ctx, 2); _ret_9; });
}
	}
	mph_int min_result = mph_min(ctx, NULL, 10, 5);
	if ((min_result == 5)) {
{
	({ mph_io_File* _arg_10 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_10); MorphString* _arg_11 = mph_string_new(ctx, "min(10, 5) = 5: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_11); mph_int _ret_12 = mph_io_Write(ctx, NULL, _arg_10, _arg_11); mph_gc_pop_roots(ctx, 2); _ret_12; });
}
	} else {
{
	({ mph_io_File* _arg_13 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_13); MorphString* _arg_14 = mph_string_new(ctx, "min(10, 5): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_14); mph_int _ret_15 = mph_io_Write(ctx, NULL, _arg_13, _arg_14); mph_gc_pop_roots(ctx, 2); _ret_15; });
}
	}
	mph_int max_result = mph_max(ctx, NULL, 10, 5);
	if ((max_result == 10)) {
{
	({ mph_io_File* _arg_16 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_16); MorphString* _arg_17 = mph_string_new(ctx, "max(10, 5) = 10: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_17); mph_int _ret_18 = mph_io_Write(ctx, NULL, _arg_16, _arg_17); mph_gc_pop_roots(ctx, 2); _ret_18; });
}
	} else {
{
	({ mph_io_File* _arg_19 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_19); MorphString* _arg_20 = mph_string_new(ctx, "max(10, 5): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_20); mph_int _ret_21 = mph_io_Write(ctx, NULL, _arg_19, _arg_20); mph_gc_pop_roots(ctx, 2); _ret_21; });
}
	}
	mph_int pow_result = mph_pow(ctx, NULL, 2, 3);
	if ((pow_result == 8)) {
{
	({ mph_io_File* _arg_22 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_22); MorphString* _arg_23 = mph_string_new(ctx, "pow(2, 3) = 8: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_23); mph_int _ret_24 = mph_io_Write(ctx, NULL, _arg_22, _arg_23); mph_gc_pop_roots(ctx, 2); _ret_24; });
}
	} else {
{
	({ mph_io_File* _arg_25 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_25); MorphString* _arg_26 = mph_string_new(ctx, "pow(2, 3): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_26); mph_int _ret_27 = mph_io_Write(ctx, NULL, _arg_25, _arg_26); mph_gc_pop_roots(ctx, 2); _ret_27; });
}
	}
	mph_int sum_result = mph_sum_five(ctx, NULL, 1, 2, 3, 4, 5);
	if ((sum_result == 15)) {
{
	({ mph_io_File* _arg_28 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_28); MorphString* _arg_29 = mph_string_new(ctx, "sum_five(1,2,3,4,5) = 15: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_29); mph_int _ret_30 = mph_io_Write(ctx, NULL, _arg_28, _arg_29); mph_gc_pop_roots(ctx, 2); _ret_30; });
}
	} else {
{
	({ mph_io_File* _arg_31 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_31); MorphString* _arg_32 = mph_string_new(ctx, "sum_five(1,2,3,4,5): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_32); mph_int _ret_33 = mph_io_Write(ctx, NULL, _arg_31, _arg_32); mph_gc_pop_roots(ctx, 2); _ret_33; });
}
	}
	({ mph_io_File* _arg_34 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_34); MorphString* _arg_35 = mph_string_new(ctx, "Numeric functions implemented successfully!\n"); mph_gc_push_root(ctx, (void**)&_arg_35); mph_int _ret_36 = mph_io_Write(ctx, NULL, _arg_34, _arg_35); mph_gc_pop_roots(ctx, 2); _ret_36; });
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_test_numeric_functions(ctx, NULL);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
