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
mph_int mph_K_INT;
mph_int mph_K_STR;
mph_int mph_K_BOOL;
mph_int mph_K_ERR;

// Function Prototypes
mph_int mph_tc_bin(MorphContext* ctx, void* _env_void, mph_int l, MorphString* op, mph_int r);
mph_int mph_tc_un(MorphContext* ctx, void* _env_void, MorphString* op, mph_int t);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_int mph_tc_bin(MorphContext* ctx, void* _env_void, mph_int l, MorphString* op, mph_int r) {
	mph_gc_push_root(ctx, (void**)&op);
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "+"))) {
{
	if ((l == mph_K_INT)) {
{
	if ((r == mph_K_INT)) {
{
	return mph_K_INT;
}
	}
}
	}
	if ((l == mph_K_STR)) {
{
	if ((r == mph_K_STR)) {
{
	return mph_K_STR;
}
	}
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "-"))) {
{
	if ((l == mph_K_INT)) {
{
	if ((r == mph_K_INT)) {
{
	return mph_K_INT;
}
	}
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "*"))) {
{
	if ((l == mph_K_INT)) {
{
	if ((r == mph_K_INT)) {
{
	return mph_K_INT;
}
	}
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "/"))) {
{
	if ((l == mph_K_INT)) {
{
	if ((r == mph_K_INT)) {
{
	return mph_K_INT;
}
	}
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "=="))) {
{
	if ((l == r)) {
{
	return mph_K_BOOL;
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "!="))) {
{
	if ((l == r)) {
{
	return mph_K_BOOL;
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "<"))) {
{
	if ((l == mph_K_INT)) {
{
	if ((r == mph_K_INT)) {
{
	return mph_K_BOOL;
}
	}
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, ">"))) {
{
	if ((l == mph_K_INT)) {
{
	if ((r == mph_K_INT)) {
{
	return mph_K_BOOL;
}
	}
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "<="))) {
{
	if ((l == mph_K_INT)) {
{
	if ((r == mph_K_INT)) {
{
	return mph_K_BOOL;
}
	}
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, ">="))) {
{
	if ((l == mph_K_INT)) {
{
	if ((r == mph_K_INT)) {
{
	return mph_K_BOOL;
}
	}
}
	}
	return mph_K_ERR;
}
	}
	return mph_K_ERR;
	mph_gc_pop_roots(ctx, 1);
}

mph_int mph_tc_un(MorphContext* ctx, void* _env_void, MorphString* op, mph_int t) {
	mph_gc_push_root(ctx, (void**)&op);
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "-"))) {
{
	if ((t == mph_K_INT)) {
{
	return mph_K_INT;
}
	}
	return mph_K_ERR;
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "!"))) {
{
	if ((t == mph_K_BOOL)) {
{
	return mph_K_BOOL;
}
	}
	return mph_K_ERR;
}
	}
	return mph_K_ERR;
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "TC v2\n"));
	mph_int t1 = ({ MorphString* _arg_1 = mph_string_new(ctx, "-"); mph_gc_push_root(ctx, (void**)&_arg_1); mph_int _ret_2 = mph_tc_bin(ctx, NULL, mph_K_INT, _arg_1, mph_K_INT); mph_gc_pop_roots(ctx, 1); _ret_2; });
	if ((t1 == mph_K_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "1.OK -\n"));
}
	}
	mph_int t2 = ({ MorphString* _arg_3 = mph_string_new(ctx, "*"); mph_gc_push_root(ctx, (void**)&_arg_3); mph_int _ret_4 = mph_tc_bin(ctx, NULL, mph_K_INT, _arg_3, mph_K_INT); mph_gc_pop_roots(ctx, 1); _ret_4; });
	if ((t2 == mph_K_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "2.OK *\n"));
}
	}
	mph_int t3 = ({ MorphString* _arg_5 = mph_string_new(ctx, "/"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_int _ret_6 = mph_tc_bin(ctx, NULL, mph_K_INT, _arg_5, mph_K_INT); mph_gc_pop_roots(ctx, 1); _ret_6; });
	if ((t3 == mph_K_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "3.OK /\n"));
}
	}
	mph_int t4 = ({ MorphString* _arg_7 = mph_string_new(ctx, "!="); mph_gc_push_root(ctx, (void**)&_arg_7); mph_int _ret_8 = mph_tc_bin(ctx, NULL, mph_K_INT, _arg_7, mph_K_INT); mph_gc_pop_roots(ctx, 1); _ret_8; });
	if ((t4 == mph_K_BOOL)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "4.OK !=\n"));
}
	}
	mph_int t5 = ({ MorphString* _arg_9 = mph_string_new(ctx, "<="); mph_gc_push_root(ctx, (void**)&_arg_9); mph_int _ret_10 = mph_tc_bin(ctx, NULL, mph_K_INT, _arg_9, mph_K_INT); mph_gc_pop_roots(ctx, 1); _ret_10; });
	if ((t5 == mph_K_BOOL)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "5.OK <=\n"));
}
	}
	mph_int t6 = ({ MorphString* _arg_11 = mph_string_new(ctx, ">="); mph_gc_push_root(ctx, (void**)&_arg_11); mph_int _ret_12 = mph_tc_bin(ctx, NULL, mph_K_INT, _arg_11, mph_K_INT); mph_gc_pop_roots(ctx, 1); _ret_12; });
	if ((t6 == mph_K_BOOL)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "6.OK >=\n"));
}
	}
	mph_int u1 = ({ MorphString* _arg_13 = mph_string_new(ctx, "-"); mph_gc_push_root(ctx, (void**)&_arg_13); mph_int _ret_14 = mph_tc_un(ctx, NULL, _arg_13, mph_K_INT); mph_gc_pop_roots(ctx, 1); _ret_14; });
	if ((u1 == mph_K_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "7.OK -int\n"));
}
	}
	mph_int u2 = ({ MorphString* _arg_15 = mph_string_new(ctx, "!"); mph_gc_push_root(ctx, (void**)&_arg_15); mph_int _ret_16 = mph_tc_un(ctx, NULL, _arg_15, mph_K_BOOL); mph_gc_pop_roots(ctx, 1); _ret_16; });
	if ((u2 == mph_K_BOOL)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "8.OK !bool\n"));
}
	}
	mph_int u3 = ({ MorphString* _arg_17 = mph_string_new(ctx, "-"); mph_gc_push_root(ctx, (void**)&_arg_17); mph_int _ret_18 = mph_tc_un(ctx, NULL, _arg_17, mph_K_STR); mph_gc_pop_roots(ctx, 1); _ret_18; });
	if ((u3 == mph_K_ERR)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "9.OK -str=err\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Done\n"));
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_K_INT = 0;
	mph_K_STR = 2;
	mph_K_BOOL = 3;
	mph_K_ERR = (-1);
	mph_main(ctx, NULL);
}
