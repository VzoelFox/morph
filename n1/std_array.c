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
#define MPH_TYPE_mph_IntArr 1

// Struct Definitions (Env & Types)
typedef struct mph_IntArr mph_IntArr;
struct mph_IntArr {
	mph_int v0;
	mph_int v1;
	mph_int v2;
	mph_int v3;
	mph_int len;
};

MorphTypeInfo mph_ti_mph_IntArr = { "IntArr", sizeof(mph_IntArr), 0, NULL };

// Global Variables

// Function Prototypes
mph_IntArr* mph_arr_new(MorphContext* ctx, void* _env_void);
mph_IntArr* mph_arr_push(MorphContext* ctx, void* _env_void, mph_IntArr* a, mph_int v);
mph_int mph_arr_get(MorphContext* ctx, void* _env_void, mph_IntArr* a, mph_int i);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_IntArr* mph_arr_new(MorphContext* ctx, void* _env_void) {
	return ({ mph_IntArr* _t = (mph_IntArr*)mph_alloc(ctx, sizeof(mph_IntArr), &mph_ti_mph_IntArr); mph_gc_push_root(ctx, (void**)&_t); _t->len = 0; _t->v0 = 0; _t->v1 = 0; _t->v2 = 0; _t->v3 = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_IntArr* mph_arr_push(MorphContext* ctx, void* _env_void, mph_IntArr* a, mph_int v) {
	mph_gc_push_root(ctx, (void**)&a);
	if ((({ mph_IntArr* _obj_1 = a; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->len; mph_gc_pop_roots(ctx, 1); _ret_2; }) == 0)) {
{
	({ mph_IntArr* _obj_3 = a; mph_gc_push_root(ctx, (void**)&_obj_3); _obj_3->v0 = v; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntArr* _obj_4 = a; mph_gc_push_root(ctx, (void**)&_obj_4); _obj_4->len = 1; mph_gc_pop_roots(ctx, 1); });
	return a;
}
	}
	if ((({ mph_IntArr* _obj_5 = a; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->len; mph_gc_pop_roots(ctx, 1); _ret_6; }) == 1)) {
{
	({ mph_IntArr* _obj_7 = a; mph_gc_push_root(ctx, (void**)&_obj_7); _obj_7->v1 = v; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntArr* _obj_8 = a; mph_gc_push_root(ctx, (void**)&_obj_8); _obj_8->len = 2; mph_gc_pop_roots(ctx, 1); });
	return a;
}
	}
	if ((({ mph_IntArr* _obj_9 = a; mph_gc_push_root(ctx, (void**)&_obj_9); mph_int _ret_10 = _obj_9->len; mph_gc_pop_roots(ctx, 1); _ret_10; }) == 2)) {
{
	({ mph_IntArr* _obj_11 = a; mph_gc_push_root(ctx, (void**)&_obj_11); _obj_11->v2 = v; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntArr* _obj_12 = a; mph_gc_push_root(ctx, (void**)&_obj_12); _obj_12->len = 3; mph_gc_pop_roots(ctx, 1); });
	return a;
}
	}
	if ((({ mph_IntArr* _obj_13 = a; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->len; mph_gc_pop_roots(ctx, 1); _ret_14; }) == 3)) {
{
	({ mph_IntArr* _obj_15 = a; mph_gc_push_root(ctx, (void**)&_obj_15); _obj_15->v3 = v; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntArr* _obj_16 = a; mph_gc_push_root(ctx, (void**)&_obj_16); _obj_16->len = 4; mph_gc_pop_roots(ctx, 1); });
}
	}
	return a;
	mph_gc_pop_roots(ctx, 1);
}

mph_int mph_arr_get(MorphContext* ctx, void* _env_void, mph_IntArr* a, mph_int i) {
	mph_gc_push_root(ctx, (void**)&a);
	if ((i == 0)) {
{
	return ({ mph_IntArr* _obj_17 = a; mph_gc_push_root(ctx, (void**)&_obj_17); mph_int _ret_18 = _obj_17->v0; mph_gc_pop_roots(ctx, 1); _ret_18; });
}
	}
	if ((i == 1)) {
{
	return ({ mph_IntArr* _obj_19 = a; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->v1; mph_gc_pop_roots(ctx, 1); _ret_20; });
}
	}
	if ((i == 2)) {
{
	return ({ mph_IntArr* _obj_21 = a; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->v2; mph_gc_pop_roots(ctx, 1); _ret_22; });
}
	}
	if ((i == 3)) {
{
	return ({ mph_IntArr* _obj_23 = a; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->v3; mph_gc_pop_roots(ctx, 1); _ret_24; });
}
	}
	return (-1);
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "Arr\n"));
	mph_IntArr* a = mph_arr_new(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&a);
	a = ({ mph_IntArr* _arg_25 = a; mph_gc_push_root(ctx, (void**)&_arg_25); mph_IntArr* _ret_26 = mph_arr_push(ctx, NULL, _arg_25, 10); mph_gc_pop_roots(ctx, 1); _ret_26; });
	a = ({ mph_IntArr* _arg_27 = a; mph_gc_push_root(ctx, (void**)&_arg_27); mph_IntArr* _ret_28 = mph_arr_push(ctx, NULL, _arg_27, 20); mph_gc_pop_roots(ctx, 1); _ret_28; });
	a = ({ mph_IntArr* _arg_29 = a; mph_gc_push_root(ctx, (void**)&_arg_29); mph_IntArr* _ret_30 = mph_arr_push(ctx, NULL, _arg_29, 30); mph_gc_pop_roots(ctx, 1); _ret_30; });
	mph_native_print_int(ctx, ({ mph_IntArr* _arg_31 = a; mph_gc_push_root(ctx, (void**)&_arg_31); mph_int _ret_32 = mph_arr_get(ctx, NULL, _arg_31, 0); mph_gc_pop_roots(ctx, 1); _ret_32; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print_int(ctx, ({ mph_IntArr* _arg_33 = a; mph_gc_push_root(ctx, (void**)&_arg_33); mph_int _ret_34 = mph_arr_get(ctx, NULL, _arg_33, 1); mph_gc_pop_roots(ctx, 1); _ret_34; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print_int(ctx, ({ mph_IntArr* _obj_35 = a; mph_gc_push_root(ctx, (void**)&_obj_35); mph_int _ret_36 = _obj_35->len; mph_gc_pop_roots(ctx, 1); _ret_36; }));
	mph_native_print(ctx, mph_string_new(ctx, "\nOK\n"));
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
