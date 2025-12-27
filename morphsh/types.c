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
#define MPH_TYPE_mph_Type 1

// Struct Definitions (Env & Types)
typedef struct mph_Type mph_Type;
struct mph_Type {
	mph_int kind;
	MorphString* name;
};

MorphTypeInfo mph_ti_mph_Type = { "Type", sizeof(mph_Type), 1, (size_t[]){offsetof(mph_Type, name)} };
typedef struct Env_mph__type_int {
	mph_Type* Type;
} Env_mph__type_int;

MorphTypeInfo mph_ti_Env_mph__type_int = { "Env_mph__type_int", sizeof(Env_mph__type_int), 1, (size_t[]){offsetof(Env_mph__type_int, Type)}, NULL };
typedef struct Env_mph__type_string {
	mph_Type* Type;
} Env_mph__type_string;

MorphTypeInfo mph_ti_Env_mph__type_string = { "Env_mph__type_string", sizeof(Env_mph__type_string), 1, (size_t[]){offsetof(Env_mph__type_string, Type)}, NULL };
typedef struct Env_mph__type_bool {
	mph_Type* Type;
} Env_mph__type_bool;

MorphTypeInfo mph_ti_Env_mph__type_bool = { "Env_mph__type_bool", sizeof(Env_mph__type_bool), 1, (size_t[]){offsetof(Env_mph__type_bool, Type)}, NULL };
typedef struct Env_mph__type_void {
	mph_Type* Type;
} Env_mph__type_void;

MorphTypeInfo mph_ti_Env_mph__type_void = { "Env_mph__type_void", sizeof(Env_mph__type_void), 1, (size_t[]){offsetof(Env_mph__type_void, Type)}, NULL };
typedef struct Env_mph__type_error {
	mph_Type* Type;
} Env_mph__type_error;

MorphTypeInfo mph_ti_Env_mph__type_error = { "Env_mph__type_error", sizeof(Env_mph__type_error), 1, (size_t[]){offsetof(Env_mph__type_error, Type)}, NULL };

// Global Variables

// Function Prototypes
mph_Type* mph_type_int(MorphContext* ctx, void* _env_void);
mph_Type* mph_type_string(MorphContext* ctx, void* _env_void);
mph_Type* mph_type_bool(MorphContext* ctx, void* _env_void);
mph_Type* mph_type_void(MorphContext* ctx, void* _env_void);
mph_Type* mph_type_error(MorphContext* ctx, void* _env_void);
mph_bool mph_types_equal(MorphContext* ctx, void* _env_void, mph_Type* t1, mph_Type* t2);
MorphString* mph_type_to_string(MorphContext* ctx, void* _env_void, mph_Type* t);

// Function Definitions
mph_Type* mph_type_int(MorphContext* ctx, void* _env_void) {
	Env_mph__type_int* _env = (Env_mph__type_int*)_env_void;
	return ({ mph_Type* _ret_1 = ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 0; _t->name = mph_string_new(ctx, "int"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_Type* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_Type* mph_type_string(MorphContext* ctx, void* _env_void) {
	Env_mph__type_string* _env = (Env_mph__type_string*)_env_void;
	return ({ mph_Type* _ret_3 = ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 1; _t->name = mph_string_new(ctx, "string"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_Type* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}

mph_Type* mph_type_bool(MorphContext* ctx, void* _env_void) {
	Env_mph__type_bool* _env = (Env_mph__type_bool*)_env_void;
	return ({ mph_Type* _ret_5 = ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 2; _t->name = mph_string_new(ctx, "bool"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_5); mph_Type* _ret_6 = _ret_5; mph_gc_pop_roots(ctx, 1); _ret_6; });
}

mph_Type* mph_type_void(MorphContext* ctx, void* _env_void) {
	Env_mph__type_void* _env = (Env_mph__type_void*)_env_void;
	return ({ mph_Type* _ret_7 = ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 3; _t->name = mph_string_new(ctx, "void"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_7); mph_Type* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
}

mph_Type* mph_type_error(MorphContext* ctx, void* _env_void) {
	Env_mph__type_error* _env = (Env_mph__type_error*)_env_void;
	return ({ mph_Type* _ret_9 = ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = (-1); _t->name = mph_string_new(ctx, "error"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_9); mph_Type* _ret_10 = _ret_9; mph_gc_pop_roots(ctx, 1); _ret_10; });
}

mph_bool mph_types_equal(MorphContext* ctx, void* _env_void, mph_Type* t1, mph_Type* t2) {
	mph_gc_push_root(ctx, (void**)&t1);
	mph_gc_push_root(ctx, (void**)&t2);
	return (({ mph_Type* _obj_11 = t1; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->kind; mph_gc_pop_roots(ctx, 1); _ret_12; }) == ({ mph_Type* _obj_13 = t2; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->kind; mph_gc_pop_roots(ctx, 1); _ret_14; }));
	mph_gc_pop_roots(ctx, 2);
}

MorphString* mph_type_to_string(MorphContext* ctx, void* _env_void, mph_Type* t) {
	mph_gc_push_root(ctx, (void**)&t);
	if ((({ mph_Type* _obj_15 = t; mph_gc_push_root(ctx, (void**)&_obj_15); mph_int _ret_16 = _obj_15->kind; mph_gc_pop_roots(ctx, 1); _ret_16; }) == 0)) {
{
	return ({ MorphString* _ret_17 = mph_string_new(ctx, "int"); mph_gc_push_root(ctx, (void**)&_ret_17); MorphString* _ret_18 = _ret_17; mph_gc_pop_roots(ctx, 1); _ret_18; });
}
	}
	if ((({ mph_Type* _obj_19 = t; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->kind; mph_gc_pop_roots(ctx, 1); _ret_20; }) == 1)) {
{
	return ({ MorphString* _ret_21 = mph_string_new(ctx, "string"); mph_gc_push_root(ctx, (void**)&_ret_21); MorphString* _ret_22 = _ret_21; mph_gc_pop_roots(ctx, 1); _ret_22; });
}
	}
	if ((({ mph_Type* _obj_23 = t; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->kind; mph_gc_pop_roots(ctx, 1); _ret_24; }) == 2)) {
{
	return ({ MorphString* _ret_25 = mph_string_new(ctx, "bool"); mph_gc_push_root(ctx, (void**)&_ret_25); MorphString* _ret_26 = _ret_25; mph_gc_pop_roots(ctx, 1); _ret_26; });
}
	}
	if ((({ mph_Type* _obj_27 = t; mph_gc_push_root(ctx, (void**)&_obj_27); mph_int _ret_28 = _obj_27->kind; mph_gc_pop_roots(ctx, 1); _ret_28; }) == 3)) {
{
	return ({ MorphString* _ret_29 = mph_string_new(ctx, "void"); mph_gc_push_root(ctx, (void**)&_ret_29); MorphString* _ret_30 = _ret_29; mph_gc_pop_roots(ctx, 1); _ret_30; });
}
	}
	if ((({ mph_Type* _obj_31 = t; mph_gc_push_root(ctx, (void**)&_obj_31); mph_int _ret_32 = _obj_31->kind; mph_gc_pop_roots(ctx, 1); _ret_32; }) == (-1))) {
{
	return ({ MorphString* _ret_33 = mph_string_new(ctx, "error"); mph_gc_push_root(ctx, (void**)&_ret_33); MorphString* _ret_34 = _ret_33; mph_gc_pop_roots(ctx, 1); _ret_34; });
}
	}
	return ({ MorphString* _ret_35 = mph_string_new(ctx, "unknown"); mph_gc_push_root(ctx, (void**)&_ret_35); MorphString* _ret_36 = _ret_35; mph_gc_pop_roots(ctx, 1); _ret_36; });
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
}
