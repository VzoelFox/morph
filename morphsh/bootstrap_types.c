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
#define MPH_TYPE_mph_BootstrapType 2

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_BootstrapType mph_BootstrapType;
struct mph_BootstrapType {
	mph_int kind;
	MorphString* name;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_BootstrapType = { "BootstrapType", sizeof(mph_BootstrapType), 1, (size_t[]){offsetof(mph_BootstrapType, name)} };

// Global Variables
mph_io_File* mph_io_Stdout;
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_BOOL;
mph_int mph_KIND_VOID;
mph_int mph_KIND_ARRAY;
mph_int mph_KIND_STRUCT;
mph_int mph_KIND_FUNC;
mph_int mph_KIND_ERROR;

// Function Prototypes
mph_BootstrapType* mph_make_int_type(MorphContext* ctx, void* _env_void);
mph_BootstrapType* mph_make_string_type(MorphContext* ctx, void* _env_void);
mph_BootstrapType* mph_make_bool_type(MorphContext* ctx, void* _env_void);
mph_BootstrapType* mph_make_void_type(MorphContext* ctx, void* _env_void);
mph_BootstrapType* mph_make_error_type(MorphContext* ctx, void* _env_void);
mph_bool mph_types_equal(MorphContext* ctx, void* _env_void, mph_BootstrapType* t1, mph_BootstrapType* t2);
mph_bool mph_types_assignable(MorphContext* ctx, void* _env_void, mph_BootstrapType* from, mph_BootstrapType* to);
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

mph_BootstrapType* mph_make_int_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_BootstrapType* _ret_1 = ({ mph_BootstrapType* _t = (mph_BootstrapType*)mph_alloc(ctx, sizeof(mph_BootstrapType), &mph_ti_mph_BootstrapType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_INT; _t->name = mph_string_new(ctx, "int"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_BootstrapType* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_BootstrapType* mph_make_string_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_BootstrapType* _ret_3 = ({ mph_BootstrapType* _t = (mph_BootstrapType*)mph_alloc(ctx, sizeof(mph_BootstrapType), &mph_ti_mph_BootstrapType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_STRING; _t->name = mph_string_new(ctx, "string"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_BootstrapType* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}

mph_BootstrapType* mph_make_bool_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_BootstrapType* _ret_5 = ({ mph_BootstrapType* _t = (mph_BootstrapType*)mph_alloc(ctx, sizeof(mph_BootstrapType), &mph_ti_mph_BootstrapType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_BOOL; _t->name = mph_string_new(ctx, "bool"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_5); mph_BootstrapType* _ret_6 = _ret_5; mph_gc_pop_roots(ctx, 1); _ret_6; });
}

mph_BootstrapType* mph_make_void_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_BootstrapType* _ret_7 = ({ mph_BootstrapType* _t = (mph_BootstrapType*)mph_alloc(ctx, sizeof(mph_BootstrapType), &mph_ti_mph_BootstrapType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_VOID; _t->name = mph_string_new(ctx, "void"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_7); mph_BootstrapType* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
}

mph_BootstrapType* mph_make_error_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_BootstrapType* _ret_9 = ({ mph_BootstrapType* _t = (mph_BootstrapType*)mph_alloc(ctx, sizeof(mph_BootstrapType), &mph_ti_mph_BootstrapType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_ERROR; _t->name = mph_string_new(ctx, "error"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_9); mph_BootstrapType* _ret_10 = _ret_9; mph_gc_pop_roots(ctx, 1); _ret_10; });
}

mph_bool mph_types_equal(MorphContext* ctx, void* _env_void, mph_BootstrapType* t1, mph_BootstrapType* t2) {
	mph_gc_push_root(ctx, (void**)&t1);
	mph_gc_push_root(ctx, (void**)&t2);
	return (({ mph_BootstrapType* _obj_11 = t1; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->kind; mph_gc_pop_roots(ctx, 1); _ret_12; }) == ({ mph_BootstrapType* _obj_13 = t2; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->kind; mph_gc_pop_roots(ctx, 1); _ret_14; }));
	mph_gc_pop_roots(ctx, 2);
}

mph_bool mph_types_assignable(MorphContext* ctx, void* _env_void, mph_BootstrapType* from, mph_BootstrapType* to) {
	mph_gc_push_root(ctx, (void**)&from);
	mph_gc_push_root(ctx, (void**)&to);
	return ({ mph_BootstrapType* _arg_15 = from; mph_gc_push_root(ctx, (void**)&_arg_15); mph_BootstrapType* _arg_16 = to; mph_gc_push_root(ctx, (void**)&_arg_16); mph_bool _ret_17 = mph_types_equal(ctx, NULL, _arg_15, _arg_16); mph_gc_pop_roots(ctx, 2); _ret_17; });
	mph_gc_pop_roots(ctx, 2);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_KIND_INT = 0;
	mph_KIND_STRING = 1;
	mph_KIND_BOOL = 2;
	mph_KIND_VOID = 3;
	mph_KIND_ARRAY = 4;
	mph_KIND_STRUCT = 5;
	mph_KIND_FUNC = 6;
	mph_KIND_ERROR = (-1);
}
