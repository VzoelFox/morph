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
#define MPH_TYPE_mph_Value 2

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_Value mph_Value;
struct mph_Value {
	mph_int kind;
	mph_int int_val;
	mph_bool bool_val;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_Value = { "Value", sizeof(mph_Value), 0, NULL };
typedef struct Env_mph__value_int {
	mph_Value* Value;
} Env_mph__value_int;

MorphTypeInfo mph_ti_Env_mph__value_int = { "Env_mph__value_int", sizeof(Env_mph__value_int), 1, (size_t[]){offsetof(Env_mph__value_int, Value)}, NULL };
typedef struct Env_mph__value_bool {
	mph_Value* Value;
} Env_mph__value_bool;

MorphTypeInfo mph_ti_Env_mph__value_bool = { "Env_mph__value_bool", sizeof(Env_mph__value_bool), 1, (size_t[]){offsetof(Env_mph__value_bool, Value)}, NULL };
typedef struct Env_mph__eval_add {
	mph_Value* Value;
} Env_mph__eval_add;

MorphTypeInfo mph_ti_Env_mph__eval_add = { "Env_mph__eval_add", sizeof(Env_mph__eval_add), 1, (size_t[]){offsetof(Env_mph__eval_add, Value)}, NULL };
typedef struct Env_mph__eval_mul {
	mph_Value* Value;
} Env_mph__eval_mul;

MorphTypeInfo mph_ti_Env_mph__eval_mul = { "Env_mph__eval_mul", sizeof(Env_mph__eval_mul), 1, (size_t[]){offsetof(Env_mph__eval_mul, Value)}, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
mph_Value* mph_value_int(MorphContext* ctx, void* _env_void, mph_int n);
mph_Value* mph_value_bool(MorphContext* ctx, void* _env_void, mph_bool b);
mph_Value* mph_eval_add(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right);
mph_Value* mph_eval_mul(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right);
mph_Value* mph_eval_lt(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right);
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

mph_Value* mph_value_int(MorphContext* ctx, void* _env_void, mph_int n) {
	Env_mph__value_int* _env = (Env_mph__value_int*)_env_void;
	return ({ mph_Value* _ret_1 = ({ mph_Value* _t = (mph_Value*)mph_alloc(ctx, sizeof(mph_Value), &mph_ti_mph_Value); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 0; _t->int_val = n; _t->bool_val = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_Value* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_Value* mph_value_bool(MorphContext* ctx, void* _env_void, mph_bool b) {
	Env_mph__value_bool* _env = (Env_mph__value_bool*)_env_void;
	return ({ mph_Value* _ret_3 = ({ mph_Value* _t = (mph_Value*)mph_alloc(ctx, sizeof(mph_Value), &mph_ti_mph_Value); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 2; _t->int_val = 0; _t->bool_val = b; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_Value* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}

mph_Value* mph_eval_add(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right) {
	mph_gc_push_root(ctx, (void**)&left);
	mph_gc_push_root(ctx, (void**)&right);
	Env_mph__eval_add* _env = (Env_mph__eval_add*)_env_void;
	if ((({ mph_Value* _obj_5 = left; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->kind; mph_gc_pop_roots(ctx, 1); _ret_6; }) == 0)) {
{
	if ((({ mph_Value* _obj_7 = right; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->kind; mph_gc_pop_roots(ctx, 1); _ret_8; }) == 0)) {
{
	return ({ mph_Value* _ret_13 = mph_value_int(ctx, NULL, (({ mph_Value* _obj_9 = left; mph_gc_push_root(ctx, (void**)&_obj_9); mph_int _ret_10 = _obj_9->int_val; mph_gc_pop_roots(ctx, 1); _ret_10; }) + ({ mph_Value* _obj_11 = right; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->int_val; mph_gc_pop_roots(ctx, 1); _ret_12; }))); mph_gc_push_root(ctx, (void**)&_ret_13); mph_Value* _ret_14 = _ret_13; mph_gc_pop_roots(ctx, 1); _ret_14; });
}
	}
}
	}
	return ({ mph_Value* _ret_15 = ({ mph_Value* _t = (mph_Value*)mph_alloc(ctx, sizeof(mph_Value), &mph_ti_mph_Value); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 3; _t->int_val = 0; _t->bool_val = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_15); mph_Value* _ret_16 = _ret_15; mph_gc_pop_roots(ctx, 1); _ret_16; });
	mph_gc_pop_roots(ctx, 2);
}

mph_Value* mph_eval_mul(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right) {
	mph_gc_push_root(ctx, (void**)&left);
	mph_gc_push_root(ctx, (void**)&right);
	Env_mph__eval_mul* _env = (Env_mph__eval_mul*)_env_void;
	if ((({ mph_Value* _obj_17 = left; mph_gc_push_root(ctx, (void**)&_obj_17); mph_int _ret_18 = _obj_17->kind; mph_gc_pop_roots(ctx, 1); _ret_18; }) == 0)) {
{
	if ((({ mph_Value* _obj_19 = right; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->kind; mph_gc_pop_roots(ctx, 1); _ret_20; }) == 0)) {
{
	return ({ mph_Value* _ret_25 = mph_value_int(ctx, NULL, (({ mph_Value* _obj_21 = left; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->int_val; mph_gc_pop_roots(ctx, 1); _ret_22; }) * ({ mph_Value* _obj_23 = right; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->int_val; mph_gc_pop_roots(ctx, 1); _ret_24; }))); mph_gc_push_root(ctx, (void**)&_ret_25); mph_Value* _ret_26 = _ret_25; mph_gc_pop_roots(ctx, 1); _ret_26; });
}
	}
}
	}
	return ({ mph_Value* _ret_27 = ({ mph_Value* _t = (mph_Value*)mph_alloc(ctx, sizeof(mph_Value), &mph_ti_mph_Value); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 3; _t->int_val = 0; _t->bool_val = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_27); mph_Value* _ret_28 = _ret_27; mph_gc_pop_roots(ctx, 1); _ret_28; });
	mph_gc_pop_roots(ctx, 2);
}

mph_Value* mph_eval_lt(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right) {
	mph_gc_push_root(ctx, (void**)&left);
	mph_gc_push_root(ctx, (void**)&right);
	if ((({ mph_Value* _obj_29 = left; mph_gc_push_root(ctx, (void**)&_obj_29); mph_int _ret_30 = _obj_29->kind; mph_gc_pop_roots(ctx, 1); _ret_30; }) == 0)) {
{
	if ((({ mph_Value* _obj_31 = right; mph_gc_push_root(ctx, (void**)&_obj_31); mph_int _ret_32 = _obj_31->kind; mph_gc_pop_roots(ctx, 1); _ret_32; }) == 0)) {
{
	return ({ mph_Value* _ret_37 = mph_value_bool(ctx, NULL, (({ mph_Value* _obj_33 = left; mph_gc_push_root(ctx, (void**)&_obj_33); mph_int _ret_34 = _obj_33->int_val; mph_gc_pop_roots(ctx, 1); _ret_34; }) < ({ mph_Value* _obj_35 = right; mph_gc_push_root(ctx, (void**)&_obj_35); mph_int _ret_36 = _obj_35->int_val; mph_gc_pop_roots(ctx, 1); _ret_36; }))); mph_gc_push_root(ctx, (void**)&_ret_37); mph_Value* _ret_38 = _ret_37; mph_gc_pop_roots(ctx, 1); _ret_38; });
}
	}
}
	}
	return ({ mph_Value* _ret_39 = mph_value_bool(ctx, NULL, 0); mph_gc_push_root(ctx, (void**)&_ret_39); mph_Value* _ret_40 = _ret_39; mph_gc_pop_roots(ctx, 1); _ret_40; });
	mph_gc_pop_roots(ctx, 2);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_41 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_41); MorphString* _arg_42 = mph_string_new(ctx, "=== EVALUATOR TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_42); mph_int _ret_43 = mph_io_Write(ctx, NULL, _arg_41, _arg_42); mph_gc_pop_roots(ctx, 2); _ret_43; });
	mph_Value* v1 = mph_value_int(ctx, NULL, 10);
	mph_gc_push_root(ctx, (void**)&v1);
	mph_Value* v2 = mph_value_int(ctx, NULL, 5);
	mph_gc_push_root(ctx, (void**)&v2);
	mph_Value* add_result = ({ mph_Value* _arg_44 = v1; mph_gc_push_root(ctx, (void**)&_arg_44); mph_Value* _arg_45 = v2; mph_gc_push_root(ctx, (void**)&_arg_45); mph_Value* _ret_46 = mph_eval_add(ctx, NULL, _arg_44, _arg_45); mph_gc_pop_roots(ctx, 2); _ret_46; });
	mph_gc_push_root(ctx, (void**)&add_result);
	if ((({ mph_Value* _obj_47 = add_result; mph_gc_push_root(ctx, (void**)&_obj_47); mph_int _ret_48 = _obj_47->int_val; mph_gc_pop_roots(ctx, 1); _ret_48; }) == 15)) {
{
	({ mph_io_File* _arg_49 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_49); MorphString* _arg_50 = mph_string_new(ctx, "10 + 5 = 15: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_50); mph_int _ret_51 = mph_io_Write(ctx, NULL, _arg_49, _arg_50); mph_gc_pop_roots(ctx, 2); _ret_51; });
}
	}
	mph_Value* mul_result = ({ mph_Value* _arg_52 = v1; mph_gc_push_root(ctx, (void**)&_arg_52); mph_Value* _arg_53 = v2; mph_gc_push_root(ctx, (void**)&_arg_53); mph_Value* _ret_54 = mph_eval_mul(ctx, NULL, _arg_52, _arg_53); mph_gc_pop_roots(ctx, 2); _ret_54; });
	mph_gc_push_root(ctx, (void**)&mul_result);
	if ((({ mph_Value* _obj_55 = mul_result; mph_gc_push_root(ctx, (void**)&_obj_55); mph_int _ret_56 = _obj_55->int_val; mph_gc_pop_roots(ctx, 1); _ret_56; }) == 50)) {
{
	({ mph_io_File* _arg_57 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_57); MorphString* _arg_58 = mph_string_new(ctx, "10 * 5 = 50: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_58); mph_int _ret_59 = mph_io_Write(ctx, NULL, _arg_57, _arg_58); mph_gc_pop_roots(ctx, 2); _ret_59; });
}
	}
	mph_Value* lt_result = ({ mph_Value* _arg_60 = v2; mph_gc_push_root(ctx, (void**)&_arg_60); mph_Value* _arg_61 = v1; mph_gc_push_root(ctx, (void**)&_arg_61); mph_Value* _ret_62 = mph_eval_lt(ctx, NULL, _arg_60, _arg_61); mph_gc_pop_roots(ctx, 2); _ret_62; });
	mph_gc_push_root(ctx, (void**)&lt_result);
	if (({ mph_Value* _obj_63 = lt_result; mph_gc_push_root(ctx, (void**)&_obj_63); mph_bool _ret_64 = _obj_63->bool_val; mph_gc_pop_roots(ctx, 1); _ret_64; })) {
{
	({ mph_io_File* _arg_65 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_65); MorphString* _arg_66 = mph_string_new(ctx, "5 < 10 = true: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_66); mph_int _ret_67 = mph_io_Write(ctx, NULL, _arg_65, _arg_66); mph_gc_pop_roots(ctx, 2); _ret_67; });
}
	}
	({ mph_io_File* _arg_68 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_68); MorphString* _arg_69 = mph_string_new(ctx, "=== TEST PASSED ===\n"); mph_gc_push_root(ctx, (void**)&_arg_69); mph_int _ret_70 = mph_io_Write(ctx, NULL, _arg_68, _arg_69); mph_gc_pop_roots(ctx, 2); _ret_70; });
	mph_gc_pop_roots(ctx, 5);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
