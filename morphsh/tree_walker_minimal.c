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
#define MPH_TYPE_mph_Value 1

// Struct Definitions (Env & Types)
typedef struct mph_Value mph_Value;
struct mph_Value {
	MorphString* type;
	mph_int intVal;
	mph_bool boolVal;
};

MorphTypeInfo mph_ti_mph_Value = { "Value", sizeof(mph_Value), 1, (size_t[]){offsetof(mph_Value, type)} };

// Global Variables

// Function Prototypes
mph_Value* mph_NewIntValue(MorphContext* ctx, void* _env_void, mph_int val);
mph_Value* mph_NewBoolValue(MorphContext* ctx, void* _env_void, mph_bool val);
mph_Value* mph_EvalAdd(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right);
mph_Value* mph_EvalCompare(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right);
void mph_TestTreeWalker(MorphContext* ctx, void* _env_void);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_Value* mph_NewIntValue(MorphContext* ctx, void* _env_void, mph_int val) {
	return ({ mph_Value* _ret_1 = ({ mph_Value* _t = (mph_Value*)mph_alloc(ctx, sizeof(mph_Value), &mph_ti_mph_Value); mph_gc_push_root(ctx, (void**)&_t); _t->type = mph_string_new(ctx, "INT"); _t->intVal = val; _t->boolVal = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_Value* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_Value* mph_NewBoolValue(MorphContext* ctx, void* _env_void, mph_bool val) {
	return ({ mph_Value* _ret_3 = ({ mph_Value* _t = (mph_Value*)mph_alloc(ctx, sizeof(mph_Value), &mph_ti_mph_Value); mph_gc_push_root(ctx, (void**)&_t); _t->intVal = 0; _t->boolVal = val; _t->type = mph_string_new(ctx, "BOOL"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_Value* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}

mph_Value* mph_EvalAdd(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right) {
	mph_gc_push_root(ctx, (void**)&left);
	mph_gc_push_root(ctx, (void**)&right);
	if (mph_string_eq(ctx, ({ mph_Value* _obj_5 = left; mph_gc_push_root(ctx, (void**)&_obj_5); MorphString* _ret_6 = _obj_5->type; mph_gc_pop_roots(ctx, 1); _ret_6; }), mph_string_new(ctx, "INT"))) {
{
	if (mph_string_eq(ctx, ({ mph_Value* _obj_7 = right; mph_gc_push_root(ctx, (void**)&_obj_7); MorphString* _ret_8 = _obj_7->type; mph_gc_pop_roots(ctx, 1); _ret_8; }), mph_string_new(ctx, "INT"))) {
{
	return ({ mph_Value* _ret_13 = mph_NewIntValue(ctx, NULL, (({ mph_Value* _obj_9 = left; mph_gc_push_root(ctx, (void**)&_obj_9); mph_int _ret_10 = _obj_9->intVal; mph_gc_pop_roots(ctx, 1); _ret_10; }) + ({ mph_Value* _obj_11 = right; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->intVal; mph_gc_pop_roots(ctx, 1); _ret_12; }))); mph_gc_push_root(ctx, (void**)&_ret_13); mph_Value* _ret_14 = _ret_13; mph_gc_pop_roots(ctx, 1); _ret_14; });
}
	}
}
	}
	return ({ mph_Value* _ret_15 = mph_NewIntValue(ctx, NULL, 0); mph_gc_push_root(ctx, (void**)&_ret_15); mph_Value* _ret_16 = _ret_15; mph_gc_pop_roots(ctx, 1); _ret_16; });
	mph_gc_pop_roots(ctx, 2);
}

mph_Value* mph_EvalCompare(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right) {
	mph_gc_push_root(ctx, (void**)&left);
	mph_gc_push_root(ctx, (void**)&right);
	if (mph_string_eq(ctx, ({ mph_Value* _obj_17 = left; mph_gc_push_root(ctx, (void**)&_obj_17); MorphString* _ret_18 = _obj_17->type; mph_gc_pop_roots(ctx, 1); _ret_18; }), mph_string_new(ctx, "INT"))) {
{
	if (mph_string_eq(ctx, ({ mph_Value* _obj_19 = right; mph_gc_push_root(ctx, (void**)&_obj_19); MorphString* _ret_20 = _obj_19->type; mph_gc_pop_roots(ctx, 1); _ret_20; }), mph_string_new(ctx, "INT"))) {
{
	return ({ mph_Value* _ret_25 = mph_NewBoolValue(ctx, NULL, (({ mph_Value* _obj_21 = left; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->intVal; mph_gc_pop_roots(ctx, 1); _ret_22; }) > ({ mph_Value* _obj_23 = right; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->intVal; mph_gc_pop_roots(ctx, 1); _ret_24; }))); mph_gc_push_root(ctx, (void**)&_ret_25); mph_Value* _ret_26 = _ret_25; mph_gc_pop_roots(ctx, 1); _ret_26; });
}
	}
}
	}
	return ({ mph_Value* _ret_27 = mph_NewBoolValue(ctx, NULL, 0); mph_gc_push_root(ctx, (void**)&_ret_27); mph_Value* _ret_28 = _ret_27; mph_gc_pop_roots(ctx, 1); _ret_28; });
	mph_gc_pop_roots(ctx, 2);
}

void mph_TestTreeWalker(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Tree Walker Test ==="));
	mph_Value* a = mph_NewIntValue(ctx, NULL, 10);
	mph_gc_push_root(ctx, (void**)&a);
	mph_Value* b = mph_NewIntValue(ctx, NULL, 5);
	mph_gc_push_root(ctx, (void**)&b);
	mph_Value* sum = ({ mph_Value* _arg_29 = a; mph_gc_push_root(ctx, (void**)&_arg_29); mph_Value* _arg_30 = b; mph_gc_push_root(ctx, (void**)&_arg_30); mph_Value* _ret_31 = mph_EvalAdd(ctx, NULL, _arg_29, _arg_30); mph_gc_pop_roots(ctx, 2); _ret_31; });
	mph_gc_push_root(ctx, (void**)&sum);
	mph_native_print(ctx, mph_string_new(ctx, "10 + 5 ="));
	mph_native_print_int(ctx, ({ mph_Value* _obj_32 = sum; mph_gc_push_root(ctx, (void**)&_obj_32); mph_int _ret_33 = _obj_32->intVal; mph_gc_pop_roots(ctx, 1); _ret_33; }));
	mph_Value* cmp = ({ mph_Value* _arg_34 = a; mph_gc_push_root(ctx, (void**)&_arg_34); mph_Value* _arg_35 = b; mph_gc_push_root(ctx, (void**)&_arg_35); mph_Value* _ret_36 = mph_EvalCompare(ctx, NULL, _arg_34, _arg_35); mph_gc_pop_roots(ctx, 2); _ret_36; });
	mph_gc_push_root(ctx, (void**)&cmp);
	mph_native_print(ctx, mph_string_new(ctx, "10 > 5:"));
	if (({ mph_Value* _obj_37 = cmp; mph_gc_push_root(ctx, (void**)&_obj_37); mph_bool _ret_38 = _obj_37->boolVal; mph_gc_pop_roots(ctx, 1); _ret_38; })) {
{
	mph_native_print(ctx, mph_string_new(ctx, "true"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "false"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "=== Tree Walker Works ==="));
	mph_gc_pop_roots(ctx, 4);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Morph Tree Walker ==="));
	mph_TestTreeWalker(ctx, NULL);
	mph_native_print(ctx, mph_string_new(ctx, "Tree walker ready!"));
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
	mph_main(ctx, NULL);
}
