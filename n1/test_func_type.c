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
#define MPH_TYPE_mph_FunctionType 1

// Struct Definitions (Env & Types)
typedef struct mph_FunctionType mph_FunctionType;
struct mph_FunctionType {
	MorphString* name;
	mph_int param_count;
	mph_int p0_kind;
	mph_int p1_kind;
	mph_int return_kind;
};

MorphTypeInfo mph_ti_mph_FunctionType = { "FunctionType", sizeof(mph_FunctionType), 1, (size_t[]){offsetof(mph_FunctionType, name)} };

// Global Variables
mph_int mph_KIND_INT;
mph_int mph_KIND_BOOL;

// Function Prototypes
mph_FunctionType* mph_make_func(MorphContext* ctx, void* _env_void, MorphString* n, mph_int ret);
mph_FunctionType* mph_add_param(MorphContext* ctx, void* _env_void, mph_FunctionType* f, mph_int pkind);
mph_bool mph_check_call(MorphContext* ctx, void* _env_void, mph_FunctionType* f, mph_int a0, mph_int a1);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_FunctionType* mph_make_func(MorphContext* ctx, void* _env_void, MorphString* n, mph_int ret) {
	mph_gc_push_root(ctx, (void**)&n);
	return ({ mph_FunctionType* _t = (mph_FunctionType*)mph_alloc(ctx, sizeof(mph_FunctionType), &mph_ti_mph_FunctionType); mph_gc_push_root(ctx, (void**)&_t); _t->p0_kind = 0; _t->p1_kind = 0; _t->return_kind = ret; _t->name = n; _t->param_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

mph_FunctionType* mph_add_param(MorphContext* ctx, void* _env_void, mph_FunctionType* f, mph_int pkind) {
	mph_gc_push_root(ctx, (void**)&f);
	if ((({ mph_FunctionType* _obj_1 = f; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->param_count; mph_gc_pop_roots(ctx, 1); _ret_2; }) == 0)) {
{
	({ mph_FunctionType* _obj_3 = f; mph_gc_push_root(ctx, (void**)&_obj_3); _obj_3->p0_kind = pkind; mph_gc_pop_roots(ctx, 1); });
}
	} else {
{
	({ mph_FunctionType* _obj_4 = f; mph_gc_push_root(ctx, (void**)&_obj_4); _obj_4->p1_kind = pkind; mph_gc_pop_roots(ctx, 1); });
}
	}
	({ mph_FunctionType* _obj_7 = f; mph_gc_push_root(ctx, (void**)&_obj_7); _obj_7->param_count = (({ mph_FunctionType* _obj_5 = f; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->param_count; mph_gc_pop_roots(ctx, 1); _ret_6; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return f;
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_check_call(MorphContext* ctx, void* _env_void, mph_FunctionType* f, mph_int a0, mph_int a1) {
	mph_gc_push_root(ctx, (void**)&f);
	if ((({ mph_FunctionType* _obj_8 = f; mph_gc_push_root(ctx, (void**)&_obj_8); mph_int _ret_9 = _obj_8->param_count; mph_gc_pop_roots(ctx, 1); _ret_9; }) == 2)) {
{
	if ((({ mph_FunctionType* _obj_10 = f; mph_gc_push_root(ctx, (void**)&_obj_10); mph_int _ret_11 = _obj_10->p0_kind; mph_gc_pop_roots(ctx, 1); _ret_11; }) == a0)) {
{
	if ((({ mph_FunctionType* _obj_12 = f; mph_gc_push_root(ctx, (void**)&_obj_12); mph_int _ret_13 = _obj_12->p1_kind; mph_gc_pop_roots(ctx, 1); _ret_13; }) == a1)) {
{
	return 1;
}
	}
}
	}
}
	}
	return 0;
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "FunctionType Test\n"));
	mph_FunctionType* add = ({ MorphString* _arg_14 = mph_string_new(ctx, "add"); mph_gc_push_root(ctx, (void**)&_arg_14); mph_FunctionType* _ret_15 = mph_make_func(ctx, NULL, _arg_14, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_15; });
	mph_gc_push_root(ctx, (void**)&add);
	add = ({ mph_FunctionType* _arg_16 = add; mph_gc_push_root(ctx, (void**)&_arg_16); mph_FunctionType* _ret_17 = mph_add_param(ctx, NULL, _arg_16, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_17; });
	add = ({ mph_FunctionType* _arg_18 = add; mph_gc_push_root(ctx, (void**)&_arg_18); mph_FunctionType* _ret_19 = mph_add_param(ctx, NULL, _arg_18, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_19; });
	if ((({ mph_FunctionType* _obj_20 = add; mph_gc_push_root(ctx, (void**)&_obj_20); mph_int _ret_21 = _obj_20->param_count; mph_gc_pop_roots(ctx, 1); _ret_21; }) == 2)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â param_count OK\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â param_count FAIL\n"));
}
	}
	if ((({ mph_FunctionType* _obj_22 = add; mph_gc_push_root(ctx, (void**)&_obj_22); mph_int _ret_23 = _obj_22->return_kind; mph_gc_pop_roots(ctx, 1); _ret_23; }) == mph_KIND_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â return_kind OK\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â return_kind FAIL\n"));
}
	}
	if (({ mph_FunctionType* _arg_24 = add; mph_gc_push_root(ctx, (void**)&_arg_24); mph_bool _ret_25 = mph_check_call(ctx, NULL, _arg_24, mph_KIND_INT, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_25; })) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â call check OK\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â call check FAIL\n"));
}
	}
	if ((({ mph_FunctionType* _arg_26 = add; mph_gc_push_root(ctx, (void**)&_arg_26); mph_bool _ret_27 = mph_check_call(ctx, NULL, _arg_26, mph_KIND_BOOL, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_27; }) == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â wrong type rejected\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â should reject\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "FunctionType OK!\n"));
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_BOOL = 3;
	mph_main(ctx, NULL);
}
