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
#define MPH_TYPE_mph_MapType 2
#define MPH_TYPE_mph_FunctionType 3
#define MPH_TYPE_mph_PointerType 4
#define MPH_TYPE_mph_MultiType 5
#define MPH_TYPE_mph_ArrayType 1

// Struct Definitions (Env & Types)
typedef struct mph_ArrayType mph_ArrayType;
typedef struct mph_MapType mph_MapType;
typedef struct mph_FunctionType mph_FunctionType;
typedef struct mph_PointerType mph_PointerType;
typedef struct mph_MultiType mph_MultiType;
struct mph_ArrayType {
	mph_int element_kind;
};

struct mph_MapType {
	mph_int key_kind;
	mph_int value_kind;
};

struct mph_FunctionType {
	mph_int param_count;
	mph_int p0_kind;
	mph_int p1_kind;
	mph_int return_kind;
};

struct mph_PointerType {
	mph_int base_kind;
};

struct mph_MultiType {
	mph_int count;
	mph_int t0;
	mph_int t1;
};

MorphTypeInfo mph_ti_mph_ArrayType = { "ArrayType", sizeof(mph_ArrayType), 0, NULL };
MorphTypeInfo mph_ti_mph_MapType = { "MapType", sizeof(mph_MapType), 0, NULL };
MorphTypeInfo mph_ti_mph_FunctionType = { "FunctionType", sizeof(mph_FunctionType), 0, NULL };
MorphTypeInfo mph_ti_mph_PointerType = { "PointerType", sizeof(mph_PointerType), 0, NULL };
MorphTypeInfo mph_ti_mph_MultiType = { "MultiType", sizeof(mph_MultiType), 0, NULL };

// Global Variables
mph_int mph_KIND_INT;
mph_int mph_KIND_FLOAT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_BOOL;
mph_int mph_KIND_UNKNOWN;
mph_int mph_KIND_USER_ERROR;
mph_int mph_passed;
mph_int mph_failed;

// Function Prototypes
mph_ArrayType* mph_make_array(MorphContext* ctx, void* _env_void, mph_int k);
mph_bool mph_array_index_ok(MorphContext* ctx, void* _env_void, mph_ArrayType* arr, mph_int key_kind);
mph_MapType* mph_make_map(MorphContext* ctx, void* _env_void, mph_int k, mph_int v);
mph_bool mph_map_key_ok(MorphContext* ctx, void* _env_void, mph_MapType* m, mph_int k);
mph_FunctionType* mph_make_func(MorphContext* ctx, void* _env_void, mph_int ret);
mph_FunctionType* mph_func_add_p(MorphContext* ctx, void* _env_void, mph_FunctionType* f, mph_int k);
mph_PointerType* mph_make_ptr(MorphContext* ctx, void* _env_void, mph_int k);
mph_MultiType* mph_make_multi(MorphContext* ctx, void* _env_void);
mph_MultiType* mph_multi_add(MorphContext* ctx, void* _env_void, mph_MultiType* m, mph_int k);
mph_bool mph_can_cast(MorphContext* ctx, void* _env_void, mph_int target, mph_int source);
void mph_test(MorphContext* ctx, void* _env_void, MorphString* name, mph_bool result);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_ArrayType* mph_make_array(MorphContext* ctx, void* _env_void, mph_int k) {
	return ({ mph_ArrayType* _t = (mph_ArrayType*)mph_alloc(ctx, sizeof(mph_ArrayType), &mph_ti_mph_ArrayType); mph_gc_push_root(ctx, (void**)&_t); _t->element_kind = k; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_array_index_ok(MorphContext* ctx, void* _env_void, mph_ArrayType* arr, mph_int key_kind) {
	mph_gc_push_root(ctx, (void**)&arr);
	return (key_kind == mph_KIND_INT);
	mph_gc_pop_roots(ctx, 1);
}

mph_MapType* mph_make_map(MorphContext* ctx, void* _env_void, mph_int k, mph_int v) {
	return ({ mph_MapType* _t = (mph_MapType*)mph_alloc(ctx, sizeof(mph_MapType), &mph_ti_mph_MapType); mph_gc_push_root(ctx, (void**)&_t); _t->key_kind = k; _t->value_kind = v; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_map_key_ok(MorphContext* ctx, void* _env_void, mph_MapType* m, mph_int k) {
	mph_gc_push_root(ctx, (void**)&m);
	return (({ mph_MapType* _obj_1 = m; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->key_kind; mph_gc_pop_roots(ctx, 1); _ret_2; }) == k);
	mph_gc_pop_roots(ctx, 1);
}

mph_FunctionType* mph_make_func(MorphContext* ctx, void* _env_void, mph_int ret) {
	return ({ mph_FunctionType* _t = (mph_FunctionType*)mph_alloc(ctx, sizeof(mph_FunctionType), &mph_ti_mph_FunctionType); mph_gc_push_root(ctx, (void**)&_t); _t->param_count = 0; _t->p0_kind = 0; _t->p1_kind = 0; _t->return_kind = ret; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_FunctionType* mph_func_add_p(MorphContext* ctx, void* _env_void, mph_FunctionType* f, mph_int k) {
	mph_gc_push_root(ctx, (void**)&f);
	if ((({ mph_FunctionType* _obj_3 = f; mph_gc_push_root(ctx, (void**)&_obj_3); mph_int _ret_4 = _obj_3->param_count; mph_gc_pop_roots(ctx, 1); _ret_4; }) == 0)) {
{
	({ mph_FunctionType* _obj_5 = f; mph_gc_push_root(ctx, (void**)&_obj_5); _obj_5->p0_kind = k; mph_gc_pop_roots(ctx, 1); });
}
	} else {
{
	({ mph_FunctionType* _obj_6 = f; mph_gc_push_root(ctx, (void**)&_obj_6); _obj_6->p1_kind = k; mph_gc_pop_roots(ctx, 1); });
}
	}
	({ mph_FunctionType* _obj_9 = f; mph_gc_push_root(ctx, (void**)&_obj_9); _obj_9->param_count = (({ mph_FunctionType* _obj_7 = f; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->param_count; mph_gc_pop_roots(ctx, 1); _ret_8; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return f;
	mph_gc_pop_roots(ctx, 1);
}

mph_PointerType* mph_make_ptr(MorphContext* ctx, void* _env_void, mph_int k) {
	return ({ mph_PointerType* _t = (mph_PointerType*)mph_alloc(ctx, sizeof(mph_PointerType), &mph_ti_mph_PointerType); mph_gc_push_root(ctx, (void**)&_t); _t->base_kind = k; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_MultiType* mph_make_multi(MorphContext* ctx, void* _env_void) {
	return ({ mph_MultiType* _t = (mph_MultiType*)mph_alloc(ctx, sizeof(mph_MultiType), &mph_ti_mph_MultiType); mph_gc_push_root(ctx, (void**)&_t); _t->t1 = 0; _t->count = 0; _t->t0 = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_MultiType* mph_multi_add(MorphContext* ctx, void* _env_void, mph_MultiType* m, mph_int k) {
	mph_gc_push_root(ctx, (void**)&m);
	if ((({ mph_MultiType* _obj_10 = m; mph_gc_push_root(ctx, (void**)&_obj_10); mph_int _ret_11 = _obj_10->count; mph_gc_pop_roots(ctx, 1); _ret_11; }) == 0)) {
{
	({ mph_MultiType* _obj_12 = m; mph_gc_push_root(ctx, (void**)&_obj_12); _obj_12->t0 = k; mph_gc_pop_roots(ctx, 1); });
}
	} else {
{
	({ mph_MultiType* _obj_13 = m; mph_gc_push_root(ctx, (void**)&_obj_13); _obj_13->t1 = k; mph_gc_pop_roots(ctx, 1); });
}
	}
	({ mph_MultiType* _obj_16 = m; mph_gc_push_root(ctx, (void**)&_obj_16); _obj_16->count = (({ mph_MultiType* _obj_14 = m; mph_gc_push_root(ctx, (void**)&_obj_14); mph_int _ret_15 = _obj_14->count; mph_gc_pop_roots(ctx, 1); _ret_15; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return m;
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_can_cast(MorphContext* ctx, void* _env_void, mph_int target, mph_int source) {
	if ((source == mph_KIND_UNKNOWN)) {
{
	return 1;
}
	}
	if ((target == mph_KIND_INT)) {
{
	if ((source == mph_KIND_FLOAT)) {
{
	return 1;
}
	}
}
	}
	if ((target == mph_KIND_FLOAT)) {
{
	if ((source == mph_KIND_INT)) {
{
	return 1;
}
	}
}
	}
	if ((target == mph_KIND_USER_ERROR)) {
{
	if ((source == mph_KIND_STRING)) {
{
	return 1;
}
	}
}
	}
	return (target == source);
}

void mph_test(MorphContext* ctx, void* _env_void, MorphString* name, mph_bool result) {
	mph_gc_push_root(ctx, (void**)&name);
	if (result) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â "));
	mph_native_print(ctx, name);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_passed = (mph_passed + 1);
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â "));
	mph_native_print(ctx, name);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_failed = (mph_failed + 1);
}
	}
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "N1 Enhanced Type System Test\n"));
	mph_native_print(ctx, mph_string_new(ctx, "============================\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ArrayType:\n"));
	mph_ArrayType* arr = mph_make_array(ctx, NULL, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&arr);
	({ MorphString* _arg_17 = mph_string_new(ctx, "create []int"); mph_gc_push_root(ctx, (void**)&_arg_17); mph_test(ctx, NULL, _arg_17, (({ mph_ArrayType* _obj_18 = arr; mph_gc_push_root(ctx, (void**)&_obj_18); mph_int _ret_19 = _obj_18->element_kind; mph_gc_pop_roots(ctx, 1); _ret_19; }) == mph_KIND_INT)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_20 = mph_string_new(ctx, "index with int OK"); mph_gc_push_root(ctx, (void**)&_arg_20); mph_test(ctx, NULL, _arg_20, ({ mph_ArrayType* _arg_21 = arr; mph_gc_push_root(ctx, (void**)&_arg_21); mph_bool _ret_22 = mph_array_index_ok(ctx, NULL, _arg_21, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_22; })); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_23 = mph_string_new(ctx, "index with string FAIL"); mph_gc_push_root(ctx, (void**)&_arg_23); mph_test(ctx, NULL, _arg_23, (({ mph_ArrayType* _arg_24 = arr; mph_gc_push_root(ctx, (void**)&_arg_24); mph_bool _ret_25 = mph_array_index_ok(ctx, NULL, _arg_24, mph_KIND_STRING); mph_gc_pop_roots(ctx, 1); _ret_25; }) == 0)); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "\nMapType:\n"));
	mph_MapType* m = mph_make_map(ctx, NULL, mph_KIND_STRING, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&m);
	({ MorphString* _arg_26 = mph_string_new(ctx, "create map[string]int"); mph_gc_push_root(ctx, (void**)&_arg_26); mph_test(ctx, NULL, _arg_26, (({ mph_MapType* _obj_27 = m; mph_gc_push_root(ctx, (void**)&_obj_27); mph_int _ret_28 = _obj_27->key_kind; mph_gc_pop_roots(ctx, 1); _ret_28; }) == mph_KIND_STRING)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_29 = mph_string_new(ctx, "key check string OK"); mph_gc_push_root(ctx, (void**)&_arg_29); mph_test(ctx, NULL, _arg_29, ({ mph_MapType* _arg_30 = m; mph_gc_push_root(ctx, (void**)&_arg_30); mph_bool _ret_31 = mph_map_key_ok(ctx, NULL, _arg_30, mph_KIND_STRING); mph_gc_pop_roots(ctx, 1); _ret_31; })); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_32 = mph_string_new(ctx, "key check int FAIL"); mph_gc_push_root(ctx, (void**)&_arg_32); mph_test(ctx, NULL, _arg_32, (({ mph_MapType* _arg_33 = m; mph_gc_push_root(ctx, (void**)&_arg_33); mph_bool _ret_34 = mph_map_key_ok(ctx, NULL, _arg_33, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_34; }) == 0)); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "\nFunctionType:\n"));
	mph_FunctionType* f = mph_make_func(ctx, NULL, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&f);
	f = ({ mph_FunctionType* _arg_35 = f; mph_gc_push_root(ctx, (void**)&_arg_35); mph_FunctionType* _ret_36 = mph_func_add_p(ctx, NULL, _arg_35, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_36; });
	f = ({ mph_FunctionType* _arg_37 = f; mph_gc_push_root(ctx, (void**)&_arg_37); mph_FunctionType* _ret_38 = mph_func_add_p(ctx, NULL, _arg_37, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_38; });
	({ MorphString* _arg_39 = mph_string_new(ctx, "param_count = 2"); mph_gc_push_root(ctx, (void**)&_arg_39); mph_test(ctx, NULL, _arg_39, (({ mph_FunctionType* _obj_40 = f; mph_gc_push_root(ctx, (void**)&_obj_40); mph_int _ret_41 = _obj_40->param_count; mph_gc_pop_roots(ctx, 1); _ret_41; }) == 2)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_42 = mph_string_new(ctx, "return_kind = int"); mph_gc_push_root(ctx, (void**)&_arg_42); mph_test(ctx, NULL, _arg_42, (({ mph_FunctionType* _obj_43 = f; mph_gc_push_root(ctx, (void**)&_obj_43); mph_int _ret_44 = _obj_43->return_kind; mph_gc_pop_roots(ctx, 1); _ret_44; }) == mph_KIND_INT)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_45 = mph_string_new(ctx, "p0 = int"); mph_gc_push_root(ctx, (void**)&_arg_45); mph_test(ctx, NULL, _arg_45, (({ mph_FunctionType* _obj_46 = f; mph_gc_push_root(ctx, (void**)&_obj_46); mph_int _ret_47 = _obj_46->p0_kind; mph_gc_pop_roots(ctx, 1); _ret_47; }) == mph_KIND_INT)); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "\nPointerType:\n"));
	mph_PointerType* ptr = mph_make_ptr(ctx, NULL, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&ptr);
	({ MorphString* _arg_48 = mph_string_new(ctx, "*int base = int"); mph_gc_push_root(ctx, (void**)&_arg_48); mph_test(ctx, NULL, _arg_48, (({ mph_PointerType* _obj_49 = ptr; mph_gc_push_root(ctx, (void**)&_obj_49); mph_int _ret_50 = _obj_49->base_kind; mph_gc_pop_roots(ctx, 1); _ret_50; }) == mph_KIND_INT)); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "\nMultiType:\n"));
	mph_MultiType* mt = mph_make_multi(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&mt);
	mt = ({ mph_MultiType* _arg_51 = mt; mph_gc_push_root(ctx, (void**)&_arg_51); mph_MultiType* _ret_52 = mph_multi_add(ctx, NULL, _arg_51, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_52; });
	mt = ({ mph_MultiType* _arg_53 = mt; mph_gc_push_root(ctx, (void**)&_arg_53); mph_MultiType* _ret_54 = mph_multi_add(ctx, NULL, _arg_53, mph_KIND_USER_ERROR); mph_gc_pop_roots(ctx, 1); _ret_54; });
	({ MorphString* _arg_55 = mph_string_new(ctx, "count = 2"); mph_gc_push_root(ctx, (void**)&_arg_55); mph_test(ctx, NULL, _arg_55, (({ mph_MultiType* _obj_56 = mt; mph_gc_push_root(ctx, (void**)&_obj_56); mph_int _ret_57 = _obj_56->count; mph_gc_pop_roots(ctx, 1); _ret_57; }) == 2)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_58 = mph_string_new(ctx, "t0 = int"); mph_gc_push_root(ctx, (void**)&_arg_58); mph_test(ctx, NULL, _arg_58, (({ mph_MultiType* _obj_59 = mt; mph_gc_push_root(ctx, (void**)&_obj_59); mph_int _ret_60 = _obj_59->t0; mph_gc_pop_roots(ctx, 1); _ret_60; }) == mph_KIND_INT)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_61 = mph_string_new(ctx, "t1 = error"); mph_gc_push_root(ctx, (void**)&_arg_61); mph_test(ctx, NULL, _arg_61, (({ mph_MultiType* _obj_62 = mt; mph_gc_push_root(ctx, (void**)&_obj_62); mph_int _ret_63 = _obj_62->t1; mph_gc_pop_roots(ctx, 1); _ret_63; }) == mph_KIND_USER_ERROR)); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "\nType Casting:\n"));
	({ MorphString* _arg_64 = mph_string_new(ctx, "float->int OK"); mph_gc_push_root(ctx, (void**)&_arg_64); mph_test(ctx, NULL, _arg_64, mph_can_cast(ctx, NULL, mph_KIND_INT, mph_KIND_FLOAT)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_65 = mph_string_new(ctx, "int->float OK"); mph_gc_push_root(ctx, (void**)&_arg_65); mph_test(ctx, NULL, _arg_65, mph_can_cast(ctx, NULL, mph_KIND_FLOAT, mph_KIND_INT)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_66 = mph_string_new(ctx, "string->error OK"); mph_gc_push_root(ctx, (void**)&_arg_66); mph_test(ctx, NULL, _arg_66, mph_can_cast(ctx, NULL, mph_KIND_USER_ERROR, mph_KIND_STRING)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_67 = mph_string_new(ctx, "string->int FAIL"); mph_gc_push_root(ctx, (void**)&_arg_67); mph_test(ctx, NULL, _arg_67, (mph_can_cast(ctx, NULL, mph_KIND_INT, mph_KIND_STRING) == 0)); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_68 = mph_string_new(ctx, "unknown->any OK"); mph_gc_push_root(ctx, (void**)&_arg_68); mph_test(ctx, NULL, _arg_68, mph_can_cast(ctx, NULL, mph_KIND_INT, mph_KIND_UNKNOWN)); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "\n============================\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Passed: "));
	mph_native_print_int(ctx, mph_passed);
	mph_native_print(ctx, mph_string_new(ctx, "\nFailed: "));
	mph_native_print_int(ctx, mph_failed);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if ((mph_failed == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "ALL TESTS PASSED!\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "SOME TESTS FAILED\n"));
}
	}
	mph_gc_pop_roots(ctx, 5);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_FLOAT = 1;
	mph_KIND_STRING = 2;
	mph_KIND_BOOL = 3;
	mph_KIND_UNKNOWN = 12;
	mph_KIND_USER_ERROR = 15;
	mph_passed = 0;
	mph_failed = 0;
	mph_main(ctx, NULL);
}
