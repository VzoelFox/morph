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
mph_int mph_k0;
mph_int mph_k1;
mph_int mph_k2;
mph_int mph_k3;

// Function Prototypes
mph_int mph_tc_bin(MorphContext* ctx, void* _env_void, mph_int l, MorphString* op, mph_int r);
mph_int mph_tc_un(MorphContext* ctx, void* _env_void, MorphString* op, mph_int t);
void mph_sym_def(MorphContext* ctx, void* _env_void, mph_int id, mph_int k);
mph_int mph_sym_get(MorphContext* ctx, void* _env_void, mph_int id);
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
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "!"))) {
{
	if ((t == mph_K_BOOL)) {
{
	return mph_K_BOOL;
}
	}
}
	}
	return mph_K_ERR;
	mph_gc_pop_roots(ctx, 1);
}

void mph_sym_def(MorphContext* ctx, void* _env_void, mph_int id, mph_int k) {
	if ((id == 0)) {
{
	mph_k0 = k;
}
	}
	if ((id == 1)) {
{
	mph_k1 = k;
}
	}
	if ((id == 2)) {
{
	mph_k2 = k;
}
	}
	if ((id == 3)) {
{
	mph_k3 = k;
}
	}
}

mph_int mph_sym_get(MorphContext* ctx, void* _env_void, mph_int id) {
	if ((id == 0)) {
{
	return mph_k0;
}
	}
	if ((id == 1)) {
{
	return mph_k1;
}
	}
	if ((id == 2)) {
{
	return mph_k2;
}
	}
	if ((id == 3)) {
{
	return mph_k3;
}
	}
	return (-1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "N1 TC Integrated\n"));
	mph_sym_def(ctx, NULL, 0, mph_K_INT);
	mph_sym_def(ctx, NULL, 1, mph_K_STR);
	mph_sym_def(ctx, NULL, 2, mph_K_BOOL);
	mph_int xk = mph_sym_get(ctx, NULL, 0);
	if ((xk == mph_K_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "1.OK x:int\n"));
}
	}
	mph_int r1 = ({ MorphString* _arg_1 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_1); mph_int _ret_2 = mph_tc_bin(ctx, NULL, xk, _arg_1, mph_K_INT); mph_gc_pop_roots(ctx, 1); _ret_2; });
	if ((r1 == mph_K_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "2.OK x+1\n"));
}
	}
	mph_int sk = mph_sym_get(ctx, NULL, 1);
	mph_int r2 = ({ MorphString* _arg_3 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_3); mph_int _ret_4 = mph_tc_bin(ctx, NULL, xk, _arg_3, sk); mph_gc_pop_roots(ctx, 1); _ret_4; });
	if ((r2 == mph_K_ERR)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "3.OK x+s=err\n"));
}
	}
	mph_int r3 = ({ MorphString* _arg_5 = mph_string_new(ctx, "-"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_int _ret_6 = mph_tc_un(ctx, NULL, _arg_5, xk); mph_gc_pop_roots(ctx, 1); _ret_6; });
	if ((r3 == mph_K_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "4.OK -x\n"));
}
	}
	mph_int bk = mph_sym_get(ctx, NULL, 2);
	mph_int r4 = ({ MorphString* _arg_7 = mph_string_new(ctx, "!"); mph_gc_push_root(ctx, (void**)&_arg_7); mph_int _ret_8 = mph_tc_un(ctx, NULL, _arg_7, bk); mph_gc_pop_roots(ctx, 1); _ret_8; });
	if ((r4 == mph_K_BOOL)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "5.OK !b\n"));
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
	mph_k0 = (-1);
	mph_k1 = (-1);
	mph_k2 = (-1);
	mph_k3 = (-1);
	mph_main(ctx, NULL);
}
