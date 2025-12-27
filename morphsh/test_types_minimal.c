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
#define MPH_TYPE_mph_Type 2

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_Type mph_Type;
struct mph_Type {
	mph_int kind;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_Type = { "Type", sizeof(mph_Type), 0, NULL };
typedef struct Env_mph__type_int {
	mph_Type* Type;
} Env_mph__type_int;

MorphTypeInfo mph_ti_Env_mph__type_int = { "Env_mph__type_int", sizeof(Env_mph__type_int), 1, (size_t[]){offsetof(Env_mph__type_int, Type)}, NULL };
typedef struct Env_mph__type_bool {
	mph_Type* Type;
} Env_mph__type_bool;

MorphTypeInfo mph_ti_Env_mph__type_bool = { "Env_mph__type_bool", sizeof(Env_mph__type_bool), 1, (size_t[]){offsetof(Env_mph__type_bool, Type)}, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
mph_Type* mph_type_int(MorphContext* ctx, void* _env_void);
mph_Type* mph_type_bool(MorphContext* ctx, void* _env_void);
mph_bool mph_types_equal(MorphContext* ctx, void* _env_void, mph_Type* t1, mph_Type* t2);
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

mph_Type* mph_type_int(MorphContext* ctx, void* _env_void) {
	Env_mph__type_int* _env = (Env_mph__type_int*)_env_void;
	return ({ mph_Type* _ret_1 = ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_Type* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_Type* mph_type_bool(MorphContext* ctx, void* _env_void) {
	Env_mph__type_bool* _env = (Env_mph__type_bool*)_env_void;
	return ({ mph_Type* _ret_3 = ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 2; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_Type* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}

mph_bool mph_types_equal(MorphContext* ctx, void* _env_void, mph_Type* t1, mph_Type* t2) {
	mph_gc_push_root(ctx, (void**)&t1);
	mph_gc_push_root(ctx, (void**)&t2);
	return (({ mph_Type* _obj_5 = t1; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->kind; mph_gc_pop_roots(ctx, 1); _ret_6; }) == ({ mph_Type* _obj_7 = t2; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->kind; mph_gc_pop_roots(ctx, 1); _ret_8; }));
	mph_gc_pop_roots(ctx, 2);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_9 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_9); MorphString* _arg_10 = mph_string_new(ctx, "=== TYPE SYSTEM TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_10); mph_int _ret_11 = mph_io_Write(ctx, NULL, _arg_9, _arg_10); mph_gc_pop_roots(ctx, 2); _ret_11; });
	mph_Type* t1 = mph_type_int(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&t1);
	mph_Type* t2 = mph_type_int(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&t2);
	mph_Type* t3 = mph_type_bool(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&t3);
	if (({ mph_Type* _arg_12 = t1; mph_gc_push_root(ctx, (void**)&_arg_12); mph_Type* _arg_13 = t2; mph_gc_push_root(ctx, (void**)&_arg_13); mph_bool _ret_14 = mph_types_equal(ctx, NULL, _arg_12, _arg_13); mph_gc_pop_roots(ctx, 2); _ret_14; })) {
{
	({ mph_io_File* _arg_15 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_15); MorphString* _arg_16 = mph_string_new(ctx, "Int == Int: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_16); mph_int _ret_17 = mph_io_Write(ctx, NULL, _arg_15, _arg_16); mph_gc_pop_roots(ctx, 2); _ret_17; });
}
	}
	if ((!({ mph_Type* _arg_18 = t1; mph_gc_push_root(ctx, (void**)&_arg_18); mph_Type* _arg_19 = t3; mph_gc_push_root(ctx, (void**)&_arg_19); mph_bool _ret_20 = mph_types_equal(ctx, NULL, _arg_18, _arg_19); mph_gc_pop_roots(ctx, 2); _ret_20; }))) {
{
	({ mph_io_File* _arg_21 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_21); MorphString* _arg_22 = mph_string_new(ctx, "Int != Bool: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_22); mph_int _ret_23 = mph_io_Write(ctx, NULL, _arg_21, _arg_22); mph_gc_pop_roots(ctx, 2); _ret_23; });
}
	}
	({ mph_io_File* _arg_24 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_24); MorphString* _arg_25 = mph_string_new(ctx, "=== TEST PASSED ===\n"); mph_gc_push_root(ctx, (void**)&_arg_25); mph_int _ret_26 = mph_io_Write(ctx, NULL, _arg_24, _arg_25); mph_gc_pop_roots(ctx, 2); _ret_26; });
	mph_gc_pop_roots(ctx, 3);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
