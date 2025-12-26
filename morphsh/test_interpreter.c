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
#define MPH_TYPE_mph_Env 3

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_Value mph_Value;
typedef struct mph_Env mph_Env;
struct mph_Value {
	mph_int kind;
	mph_int int_val;
};

struct mph_Env {
	mph_int x;
	mph_int y;
	mph_int result;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_Value = { "Value", sizeof(mph_Value), 0, NULL };
MorphTypeInfo mph_ti_mph_Env = { "Env", sizeof(mph_Env), 0, NULL };
typedef struct Env_mph__value_int {
	mph_Value* Value;
} Env_mph__value_int;

MorphTypeInfo mph_ti_Env_mph__value_int = { "Env_mph__value_int", sizeof(Env_mph__value_int), 1, (size_t[]){offsetof(Env_mph__value_int, Value)}, NULL };
typedef struct Env_mph__main {
	mph_Env* Env;
} Env_mph__main;

MorphTypeInfo mph_ti_Env_mph__main = { "Env_mph__main", sizeof(Env_mph__main), 1, (size_t[]){offsetof(Env_mph__main, Env)}, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
mph_Value* mph_value_int(MorphContext* ctx, void* _env_void, mph_int n);
mph_Value* mph_eval_add(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right);
mph_Value* mph_eval_mul(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right);
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
	return ({ mph_Value* _ret_1 = ({ mph_Value* _t = (mph_Value*)mph_alloc(ctx, sizeof(mph_Value), &mph_ti_mph_Value); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 0; _t->int_val = n; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_Value* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_Value* mph_eval_add(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right) {
	mph_gc_push_root(ctx, (void**)&left);
	mph_gc_push_root(ctx, (void**)&right);
	return ({ mph_Value* _ret_7 = mph_value_int(ctx, NULL, (({ mph_Value* _obj_3 = left; mph_gc_push_root(ctx, (void**)&_obj_3); mph_int _ret_4 = _obj_3->int_val; mph_gc_pop_roots(ctx, 1); _ret_4; }) + ({ mph_Value* _obj_5 = right; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->int_val; mph_gc_pop_roots(ctx, 1); _ret_6; }))); mph_gc_push_root(ctx, (void**)&_ret_7); mph_Value* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
	mph_gc_pop_roots(ctx, 2);
}

mph_Value* mph_eval_mul(MorphContext* ctx, void* _env_void, mph_Value* left, mph_Value* right) {
	mph_gc_push_root(ctx, (void**)&left);
	mph_gc_push_root(ctx, (void**)&right);
	return ({ mph_Value* _ret_13 = mph_value_int(ctx, NULL, (({ mph_Value* _obj_9 = left; mph_gc_push_root(ctx, (void**)&_obj_9); mph_int _ret_10 = _obj_9->int_val; mph_gc_pop_roots(ctx, 1); _ret_10; }) * ({ mph_Value* _obj_11 = right; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->int_val; mph_gc_pop_roots(ctx, 1); _ret_12; }))); mph_gc_push_root(ctx, (void**)&_ret_13); mph_Value* _ret_14 = _ret_13; mph_gc_pop_roots(ctx, 1); _ret_14; });
	mph_gc_pop_roots(ctx, 2);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	Env_mph__main* _env = (Env_mph__main*)_env_void;
	({ mph_io_File* _arg_15 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_15); MorphString* _arg_16 = mph_string_new(ctx, "=== SIMPLE INTERPRETER TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_16); mph_int _ret_17 = mph_io_Write(ctx, NULL, _arg_15, _arg_16); mph_gc_pop_roots(ctx, 2); _ret_17; });
	mph_Env* env = ({ mph_Env* _t = (mph_Env*)mph_alloc(ctx, sizeof(mph_Env), &mph_ti_mph_Env); mph_gc_push_root(ctx, (void**)&_t); _t->x = 10; _t->y = 0; _t->result = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&env);
	({ mph_io_File* _arg_18 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_18); MorphString* _arg_19 = mph_string_new(ctx, "var x = 10\n"); mph_gc_push_root(ctx, (void**)&_arg_19); mph_int _ret_20 = mph_io_Write(ctx, NULL, _arg_18, _arg_19); mph_gc_pop_roots(ctx, 2); _ret_20; });
	({ mph_Env* _obj_21 = env; mph_gc_push_root(ctx, (void**)&_obj_21); _obj_21->y = 5; mph_gc_pop_roots(ctx, 1); });
	({ mph_io_File* _arg_22 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_22); MorphString* _arg_23 = mph_string_new(ctx, "var y = 5\n"); mph_gc_push_root(ctx, (void**)&_arg_23); mph_int _ret_24 = mph_io_Write(ctx, NULL, _arg_22, _arg_23); mph_gc_pop_roots(ctx, 2); _ret_24; });
	mph_Value* v_x = mph_value_int(ctx, NULL, ({ mph_Env* _obj_25 = env; mph_gc_push_root(ctx, (void**)&_obj_25); mph_int _ret_26 = _obj_25->x; mph_gc_pop_roots(ctx, 1); _ret_26; }));
	mph_gc_push_root(ctx, (void**)&v_x);
	mph_Value* v_y = mph_value_int(ctx, NULL, ({ mph_Env* _obj_27 = env; mph_gc_push_root(ctx, (void**)&_obj_27); mph_int _ret_28 = _obj_27->y; mph_gc_pop_roots(ctx, 1); _ret_28; }));
	mph_gc_push_root(ctx, (void**)&v_y);
	mph_Value* v_add = ({ mph_Value* _arg_29 = v_x; mph_gc_push_root(ctx, (void**)&_arg_29); mph_Value* _arg_30 = v_y; mph_gc_push_root(ctx, (void**)&_arg_30); mph_Value* _ret_31 = mph_eval_add(ctx, NULL, _arg_29, _arg_30); mph_gc_pop_roots(ctx, 2); _ret_31; });
	mph_gc_push_root(ctx, (void**)&v_add);
	({ mph_Env* _obj_34 = env; mph_gc_push_root(ctx, (void**)&_obj_34); _obj_34->result = ({ mph_Value* _obj_32 = v_add; mph_gc_push_root(ctx, (void**)&_obj_32); mph_int _ret_33 = _obj_32->int_val; mph_gc_pop_roots(ctx, 1); _ret_33; }); mph_gc_pop_roots(ctx, 1); });
	({ mph_io_File* _arg_35 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_35); MorphString* _arg_36 = mph_string_new(ctx, "var result = x + y\n"); mph_gc_push_root(ctx, (void**)&_arg_36); mph_int _ret_37 = mph_io_Write(ctx, NULL, _arg_35, _arg_36); mph_gc_pop_roots(ctx, 2); _ret_37; });
	if ((({ mph_Env* _obj_38 = env; mph_gc_push_root(ctx, (void**)&_obj_38); mph_int _ret_39 = _obj_38->result; mph_gc_pop_roots(ctx, 1); _ret_39; }) == 15)) {
{
	({ mph_io_File* _arg_40 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_40); MorphString* _arg_41 = mph_string_new(ctx, "result = 15: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_41); mph_int _ret_42 = mph_io_Write(ctx, NULL, _arg_40, _arg_41); mph_gc_pop_roots(ctx, 2); _ret_42; });
}
	}
	mph_Value* v_result = mph_value_int(ctx, NULL, ({ mph_Env* _obj_43 = env; mph_gc_push_root(ctx, (void**)&_obj_43); mph_int _ret_44 = _obj_43->result; mph_gc_pop_roots(ctx, 1); _ret_44; }));
	mph_gc_push_root(ctx, (void**)&v_result);
	mph_Value* v_two = mph_value_int(ctx, NULL, 2);
	mph_gc_push_root(ctx, (void**)&v_two);
	mph_Value* v_mul = ({ mph_Value* _arg_45 = v_result; mph_gc_push_root(ctx, (void**)&_arg_45); mph_Value* _arg_46 = v_two; mph_gc_push_root(ctx, (void**)&_arg_46); mph_Value* _ret_47 = mph_eval_mul(ctx, NULL, _arg_45, _arg_46); mph_gc_pop_roots(ctx, 2); _ret_47; });
	mph_gc_push_root(ctx, (void**)&v_mul);
	({ mph_Env* _obj_50 = env; mph_gc_push_root(ctx, (void**)&_obj_50); _obj_50->result = ({ mph_Value* _obj_48 = v_mul; mph_gc_push_root(ctx, (void**)&_obj_48); mph_int _ret_49 = _obj_48->int_val; mph_gc_pop_roots(ctx, 1); _ret_49; }); mph_gc_pop_roots(ctx, 1); });
	({ mph_io_File* _arg_51 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_51); MorphString* _arg_52 = mph_string_new(ctx, "result = result * 2\n"); mph_gc_push_root(ctx, (void**)&_arg_52); mph_int _ret_53 = mph_io_Write(ctx, NULL, _arg_51, _arg_52); mph_gc_pop_roots(ctx, 2); _ret_53; });
	if ((({ mph_Env* _obj_54 = env; mph_gc_push_root(ctx, (void**)&_obj_54); mph_int _ret_55 = _obj_54->result; mph_gc_pop_roots(ctx, 1); _ret_55; }) == 30)) {
{
	({ mph_io_File* _arg_56 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_56); MorphString* _arg_57 = mph_string_new(ctx, "result = 30: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_57); mph_int _ret_58 = mph_io_Write(ctx, NULL, _arg_56, _arg_57); mph_gc_pop_roots(ctx, 2); _ret_58; });
}
	}
	({ mph_io_File* _arg_59 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_59); MorphString* _arg_60 = mph_string_new(ctx, "=== INTERPRETER WORKING ===\n"); mph_gc_push_root(ctx, (void**)&_arg_60); mph_int _ret_61 = mph_io_Write(ctx, NULL, _arg_59, _arg_60); mph_gc_pop_roots(ctx, 2); _ret_61; });
	mph_gc_pop_roots(ctx, 7);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
