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
#define MPH_TYPE_mph_Env 2

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_Env mph_Env;
struct mph_Env {
	mph_int x;
	mph_int y;
	mph_int z;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_Env = { "Env", sizeof(mph_Env), 0, NULL };
typedef struct Env_mph__env_new {
	mph_Env* Env;
} Env_mph__env_new;

MorphTypeInfo mph_ti_Env_mph__env_new = { "Env_mph__env_new", sizeof(Env_mph__env_new), 1, (size_t[]){offsetof(Env_mph__env_new, Env)}, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
mph_Env* mph_env_new(MorphContext* ctx, void* _env_void);
mph_Env* mph_env_set_x(MorphContext* ctx, void* _env_void, mph_Env* e, mph_int val);
mph_Env* mph_env_set_y(MorphContext* ctx, void* _env_void, mph_Env* e, mph_int val);
mph_Env* mph_env_set_z(MorphContext* ctx, void* _env_void, mph_Env* e, mph_int val);
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

mph_Env* mph_env_new(MorphContext* ctx, void* _env_void) {
	Env_mph__env_new* _env = (Env_mph__env_new*)_env_void;
	return ({ mph_Env* _ret_1 = ({ mph_Env* _t = (mph_Env*)mph_alloc(ctx, sizeof(mph_Env), &mph_ti_mph_Env); mph_gc_push_root(ctx, (void**)&_t); _t->x = 0; _t->y = 0; _t->z = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_Env* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_Env* mph_env_set_x(MorphContext* ctx, void* _env_void, mph_Env* e, mph_int val) {
	mph_gc_push_root(ctx, (void**)&e);
	({ mph_Env* _obj_3 = e; mph_gc_push_root(ctx, (void**)&_obj_3); _obj_3->x = val; mph_gc_pop_roots(ctx, 1); });
	return ({ mph_Env* _ret_4 = e; mph_gc_push_root(ctx, (void**)&_ret_4); mph_Env* _ret_5 = _ret_4; mph_gc_pop_roots(ctx, 1); _ret_5; });
	mph_gc_pop_roots(ctx, 1);
}

mph_Env* mph_env_set_y(MorphContext* ctx, void* _env_void, mph_Env* e, mph_int val) {
	mph_gc_push_root(ctx, (void**)&e);
	({ mph_Env* _obj_6 = e; mph_gc_push_root(ctx, (void**)&_obj_6); _obj_6->y = val; mph_gc_pop_roots(ctx, 1); });
	return ({ mph_Env* _ret_7 = e; mph_gc_push_root(ctx, (void**)&_ret_7); mph_Env* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
	mph_gc_pop_roots(ctx, 1);
}

mph_Env* mph_env_set_z(MorphContext* ctx, void* _env_void, mph_Env* e, mph_int val) {
	mph_gc_push_root(ctx, (void**)&e);
	({ mph_Env* _obj_9 = e; mph_gc_push_root(ctx, (void**)&_obj_9); _obj_9->z = val; mph_gc_pop_roots(ctx, 1); });
	return ({ mph_Env* _ret_10 = e; mph_gc_push_root(ctx, (void**)&_ret_10); mph_Env* _ret_11 = _ret_10; mph_gc_pop_roots(ctx, 1); _ret_11; });
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_12 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_12); MorphString* _arg_16 = ({ MorphString* _lhs_13 = mph_string_new(ctx, "=== ENV TEST ==="); MorphString* _rhs_14 = mph_string_new(ctx, "\n"); mph_gc_push_root(ctx, (void**)&_lhs_13); mph_gc_push_root(ctx, (void**)&_rhs_14); MorphString* _ret_15 = mph_string_concat(ctx, _lhs_13, _rhs_14); mph_gc_pop_roots(ctx, 2); _ret_15; }); mph_gc_push_root(ctx, (void**)&_arg_16); mph_int _ret_17 = mph_io_Write(ctx, NULL, _arg_12, _arg_16); mph_gc_pop_roots(ctx, 2); _ret_17; });
	mph_Env* e = mph_env_new(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&e);
	e = ({ mph_Env* _arg_18 = e; mph_gc_push_root(ctx, (void**)&_arg_18); mph_Env* _ret_19 = mph_env_set_x(ctx, NULL, _arg_18, 42); mph_gc_pop_roots(ctx, 1); _ret_19; });
	if ((({ mph_Env* _obj_20 = e; mph_gc_push_root(ctx, (void**)&_obj_20); mph_int _ret_21 = _obj_20->x; mph_gc_pop_roots(ctx, 1); _ret_21; }) == 42)) {
{
	({ mph_io_File* _arg_22 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_22); MorphString* _arg_26 = ({ MorphString* _lhs_23 = mph_string_new(ctx, "x=42: OK"); MorphString* _rhs_24 = mph_string_new(ctx, "\n"); mph_gc_push_root(ctx, (void**)&_lhs_23); mph_gc_push_root(ctx, (void**)&_rhs_24); MorphString* _ret_25 = mph_string_concat(ctx, _lhs_23, _rhs_24); mph_gc_pop_roots(ctx, 2); _ret_25; }); mph_gc_push_root(ctx, (void**)&_arg_26); mph_int _ret_27 = mph_io_Write(ctx, NULL, _arg_22, _arg_26); mph_gc_pop_roots(ctx, 2); _ret_27; });
}
	}
	e = ({ mph_Env* _arg_28 = e; mph_gc_push_root(ctx, (void**)&_arg_28); mph_Env* _ret_29 = mph_env_set_y(ctx, NULL, _arg_28, 10); mph_gc_pop_roots(ctx, 1); _ret_29; });
	e = ({ mph_Env* _arg_30 = e; mph_gc_push_root(ctx, (void**)&_arg_30); mph_Env* _ret_31 = mph_env_set_z(ctx, NULL, _arg_30, 20); mph_gc_pop_roots(ctx, 1); _ret_31; });
	mph_int sum = (({ mph_Env* _obj_32 = e; mph_gc_push_root(ctx, (void**)&_obj_32); mph_int _ret_33 = _obj_32->y; mph_gc_pop_roots(ctx, 1); _ret_33; }) + ({ mph_Env* _obj_34 = e; mph_gc_push_root(ctx, (void**)&_obj_34); mph_int _ret_35 = _obj_34->z; mph_gc_pop_roots(ctx, 1); _ret_35; }));
	if ((sum == 30)) {
{
	({ mph_io_File* _arg_36 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_36); MorphString* _arg_40 = ({ MorphString* _lhs_37 = mph_string_new(ctx, "y+z=30: OK"); MorphString* _rhs_38 = mph_string_new(ctx, "\n"); mph_gc_push_root(ctx, (void**)&_lhs_37); mph_gc_push_root(ctx, (void**)&_rhs_38); MorphString* _ret_39 = mph_string_concat(ctx, _lhs_37, _rhs_38); mph_gc_pop_roots(ctx, 2); _ret_39; }); mph_gc_push_root(ctx, (void**)&_arg_40); mph_int _ret_41 = mph_io_Write(ctx, NULL, _arg_36, _arg_40); mph_gc_pop_roots(ctx, 2); _ret_41; });
}
	}
	({ mph_io_File* _arg_42 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_42); MorphString* _arg_46 = ({ MorphString* _lhs_43 = mph_string_new(ctx, "=== PASSED ==="); MorphString* _rhs_44 = mph_string_new(ctx, "\n"); mph_gc_push_root(ctx, (void**)&_lhs_43); mph_gc_push_root(ctx, (void**)&_rhs_44); MorphString* _ret_45 = mph_string_concat(ctx, _lhs_43, _rhs_44); mph_gc_pop_roots(ctx, 2); _ret_45; }); mph_gc_push_root(ctx, (void**)&_arg_46); mph_int _ret_47 = mph_io_Write(ctx, NULL, _arg_42, _arg_46); mph_gc_pop_roots(ctx, 2); _ret_47; });
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
