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
#define MPH_TYPE_mph_TypeChecker 1

// Struct Definitions (Env & Types)
typedef struct mph_TypeChecker mph_TypeChecker;
struct mph_TypeChecker {
	mph_int scope_level;
	mph_int error_count;
};

MorphTypeInfo mph_ti_mph_TypeChecker = { "TypeChecker", sizeof(mph_TypeChecker), 0, NULL };

// Global Variables
mph_int mph_KIND_INT;
mph_int mph_KIND_FLOAT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_BOOL;
mph_int mph_KIND_VOID;
mph_int mph_KIND_ERROR;

// Function Prototypes
mph_TypeChecker* mph_tc_new(MorphContext* ctx, void* _env_void);
mph_int mph_tc_check_binary(MorphContext* ctx, void* _env_void, mph_int left, MorphString* op, mph_int right);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_TypeChecker* mph_tc_new(MorphContext* ctx, void* _env_void) {
	return ({ mph_TypeChecker* _t = (mph_TypeChecker*)mph_alloc(ctx, sizeof(mph_TypeChecker), &mph_ti_mph_TypeChecker); mph_gc_push_root(ctx, (void**)&_t); _t->scope_level = 0; _t->error_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_int mph_tc_check_binary(MorphContext* ctx, void* _env_void, mph_int left, MorphString* op, mph_int right) {
	mph_gc_push_root(ctx, (void**)&op);
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "+"))) {
{
	if ((left == mph_KIND_INT)) {
{
	if ((right == mph_KIND_INT)) {
{
	return mph_KIND_INT;
}
	}
}
	}
	if ((left == mph_KIND_STRING)) {
{
	if ((right == mph_KIND_STRING)) {
{
	return mph_KIND_STRING;
}
	}
}
	}
	return mph_KIND_ERROR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "-"))) {
{
	if ((left == mph_KIND_INT)) {
{
	if ((right == mph_KIND_INT)) {
{
	return mph_KIND_INT;
}
	}
}
	}
	return mph_KIND_ERROR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "*"))) {
{
	if ((left == mph_KIND_INT)) {
{
	if ((right == mph_KIND_INT)) {
{
	return mph_KIND_INT;
}
	}
}
	}
	return mph_KIND_ERROR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "/"))) {
{
	if ((left == mph_KIND_INT)) {
{
	if ((right == mph_KIND_INT)) {
{
	return mph_KIND_INT;
}
	}
}
	}
	return mph_KIND_ERROR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "=="))) {
{
	if ((left == right)) {
{
	return mph_KIND_BOOL;
}
	}
	return mph_KIND_ERROR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "!="))) {
{
	if ((left == right)) {
{
	return mph_KIND_BOOL;
}
	}
	return mph_KIND_ERROR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "<"))) {
{
	if ((left == mph_KIND_INT)) {
{
	if ((right == mph_KIND_INT)) {
{
	return mph_KIND_BOOL;
}
	}
}
	}
	return mph_KIND_ERROR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, ">"))) {
{
	if ((left == mph_KIND_INT)) {
{
	if ((right == mph_KIND_INT)) {
{
	return mph_KIND_BOOL;
}
	}
}
	}
	return mph_KIND_ERROR;
}
	}
	return mph_KIND_ERROR;
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_TypeChecker* tc = mph_tc_new(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&tc);
	mph_native_print(ctx, mph_string_new(ctx, "=== N1 Type Checker Test ===\n"));
	mph_int r1 = ({ MorphString* _arg_1 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_1); mph_int _ret_2 = mph_tc_check_binary(ctx, NULL, mph_KIND_INT, _arg_1, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_2; });
	if ((r1 == mph_KIND_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â int + int = int\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â int + int failed\n"));
}
	}
	mph_int r2 = ({ MorphString* _arg_3 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_3); mph_int _ret_4 = mph_tc_check_binary(ctx, NULL, mph_KIND_STRING, _arg_3, mph_KIND_STRING); mph_gc_pop_roots(ctx, 1); _ret_4; });
	if ((r2 == mph_KIND_STRING)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â string + string = string\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â string + string failed\n"));
}
	}
	mph_int r3 = ({ MorphString* _arg_5 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_int _ret_6 = mph_tc_check_binary(ctx, NULL, mph_KIND_INT, _arg_5, mph_KIND_STRING); mph_gc_pop_roots(ctx, 1); _ret_6; });
	if ((r3 == mph_KIND_ERROR)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â int + string = error\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â int + string should error\n"));
}
	}
	mph_int r4 = ({ MorphString* _arg_7 = mph_string_new(ctx, "=="); mph_gc_push_root(ctx, (void**)&_arg_7); mph_int _ret_8 = mph_tc_check_binary(ctx, NULL, mph_KIND_INT, _arg_7, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_8; });
	if ((r4 == mph_KIND_BOOL)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â int == int = bool\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â int == int failed\n"));
}
	}
	mph_int r5 = ({ MorphString* _arg_9 = mph_string_new(ctx, "<"); mph_gc_push_root(ctx, (void**)&_arg_9); mph_int _ret_10 = mph_tc_check_binary(ctx, NULL, mph_KIND_INT, _arg_9, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_10; });
	if ((r5 == mph_KIND_BOOL)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â int < int = bool\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â int < int failed\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "=== Done ===\n"));
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_FLOAT = 1;
	mph_KIND_STRING = 2;
	mph_KIND_BOOL = 3;
	mph_KIND_VOID = 4;
	mph_KIND_ERROR = (-1);
	mph_main(ctx, NULL);
}
