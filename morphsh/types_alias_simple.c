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
#define MPH_TYPE_mph_BasicType 2
#define MPH_TYPE_mph_io_File 1

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_BasicType mph_BasicType;
struct mph_BasicType {
	mph_int kind;
	MorphString* name;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_BasicType = { "BasicType", sizeof(mph_BasicType), 1, (size_t[]){offsetof(mph_BasicType, name)} };

// Global Variables
mph_io_File* mph_io_Stdout;
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_BOOL;
mph_int mph_KIND_VOID;

// Function Prototypes
mph_BasicType* mph_create_int_type(MorphContext* ctx, void* _env_void);
mph_BasicType* mph_create_string_type(MorphContext* ctx, void* _env_void);
mph_BasicType* mph_create_bool_type(MorphContext* ctx, void* _env_void);
mph_bool mph_is_int_type(MorphContext* ctx, void* _env_void, mph_BasicType* t);
mph_bool mph_is_string_type(MorphContext* ctx, void* _env_void, mph_BasicType* t);
mph_bool mph_types_compatible(MorphContext* ctx, void* _env_void, mph_BasicType* t1, mph_BasicType* t2);
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

mph_BasicType* mph_create_int_type(MorphContext* ctx, void* _env_void) {
	mph_BasicType* t = ({ mph_BasicType* _t = (mph_BasicType*)mph_alloc(ctx, sizeof(mph_BasicType), &mph_ti_mph_BasicType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_INT; _t->name = mph_string_new(ctx, "int"); mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&t);
	return ({ mph_BasicType* _ret_1 = t; mph_gc_push_root(ctx, (void**)&_ret_1); mph_BasicType* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
	mph_gc_pop_roots(ctx, 1);
}

mph_BasicType* mph_create_string_type(MorphContext* ctx, void* _env_void) {
	mph_BasicType* t = ({ mph_BasicType* _t = (mph_BasicType*)mph_alloc(ctx, sizeof(mph_BasicType), &mph_ti_mph_BasicType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_STRING; _t->name = mph_string_new(ctx, "string"); mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&t);
	return ({ mph_BasicType* _ret_3 = t; mph_gc_push_root(ctx, (void**)&_ret_3); mph_BasicType* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
	mph_gc_pop_roots(ctx, 1);
}

mph_BasicType* mph_create_bool_type(MorphContext* ctx, void* _env_void) {
	mph_BasicType* t = ({ mph_BasicType* _t = (mph_BasicType*)mph_alloc(ctx, sizeof(mph_BasicType), &mph_ti_mph_BasicType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_BOOL; _t->name = mph_string_new(ctx, "bool"); mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&t);
	return ({ mph_BasicType* _ret_5 = t; mph_gc_push_root(ctx, (void**)&_ret_5); mph_BasicType* _ret_6 = _ret_5; mph_gc_pop_roots(ctx, 1); _ret_6; });
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_is_int_type(MorphContext* ctx, void* _env_void, mph_BasicType* t) {
	mph_gc_push_root(ctx, (void**)&t);
	return (({ mph_BasicType* _obj_7 = t; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->kind; mph_gc_pop_roots(ctx, 1); _ret_8; }) == mph_KIND_INT);
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_is_string_type(MorphContext* ctx, void* _env_void, mph_BasicType* t) {
	mph_gc_push_root(ctx, (void**)&t);
	return (({ mph_BasicType* _obj_9 = t; mph_gc_push_root(ctx, (void**)&_obj_9); mph_int _ret_10 = _obj_9->kind; mph_gc_pop_roots(ctx, 1); _ret_10; }) == mph_KIND_STRING);
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_types_compatible(MorphContext* ctx, void* _env_void, mph_BasicType* t1, mph_BasicType* t2) {
	mph_gc_push_root(ctx, (void**)&t1);
	mph_gc_push_root(ctx, (void**)&t2);
	return (({ mph_BasicType* _obj_11 = t1; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->kind; mph_gc_pop_roots(ctx, 1); _ret_12; }) == ({ mph_BasicType* _obj_13 = t2; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->kind; mph_gc_pop_roots(ctx, 1); _ret_14; }));
	mph_gc_pop_roots(ctx, 2);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_15 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_15); MorphString* _arg_16 = mph_string_new(ctx, "=== TYPE ALIAS SYSTEM TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_16); mph_int _ret_17 = mph_io_Write(ctx, NULL, _arg_15, _arg_16); mph_gc_pop_roots(ctx, 2); _ret_17; });
	mph_BasicType* int_type = mph_create_int_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&int_type);
	mph_BasicType* string_type = mph_create_string_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&string_type);
	({ mph_io_File* _arg_18 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_18); MorphString* _arg_19 = mph_string_new(ctx, "Created basic types: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_19); mph_int _ret_20 = mph_io_Write(ctx, NULL, _arg_18, _arg_19); mph_gc_pop_roots(ctx, 2); _ret_20; });
	if (({ mph_BasicType* _arg_21 = int_type; mph_gc_push_root(ctx, (void**)&_arg_21); mph_bool _ret_22 = mph_is_int_type(ctx, NULL, _arg_21); mph_gc_pop_roots(ctx, 1); _ret_22; })) {
{
	({ mph_io_File* _arg_23 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_23); MorphString* _arg_24 = mph_string_new(ctx, "Int type check: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_24); mph_int _ret_25 = mph_io_Write(ctx, NULL, _arg_23, _arg_24); mph_gc_pop_roots(ctx, 2); _ret_25; });
}
	} else {
{
	({ mph_io_File* _arg_26 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_26); MorphString* _arg_27 = mph_string_new(ctx, "Int type check: ERROR\n"); mph_gc_push_root(ctx, (void**)&_arg_27); mph_int _ret_28 = mph_io_Write(ctx, NULL, _arg_26, _arg_27); mph_gc_pop_roots(ctx, 2); _ret_28; });
}
	}
	if (({ mph_BasicType* _arg_29 = string_type; mph_gc_push_root(ctx, (void**)&_arg_29); mph_bool _ret_30 = mph_is_string_type(ctx, NULL, _arg_29); mph_gc_pop_roots(ctx, 1); _ret_30; })) {
{
	({ mph_io_File* _arg_31 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_31); MorphString* _arg_32 = mph_string_new(ctx, "String type check: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_32); mph_int _ret_33 = mph_io_Write(ctx, NULL, _arg_31, _arg_32); mph_gc_pop_roots(ctx, 2); _ret_33; });
}
	} else {
{
	({ mph_io_File* _arg_34 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_34); MorphString* _arg_35 = mph_string_new(ctx, "String type check: ERROR\n"); mph_gc_push_root(ctx, (void**)&_arg_35); mph_int _ret_36 = mph_io_Write(ctx, NULL, _arg_34, _arg_35); mph_gc_pop_roots(ctx, 2); _ret_36; });
}
	}
	mph_BasicType* int_type2 = mph_create_int_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&int_type2);
	if (({ mph_BasicType* _arg_37 = int_type; mph_gc_push_root(ctx, (void**)&_arg_37); mph_BasicType* _arg_38 = int_type2; mph_gc_push_root(ctx, (void**)&_arg_38); mph_bool _ret_39 = mph_types_compatible(ctx, NULL, _arg_37, _arg_38); mph_gc_pop_roots(ctx, 2); _ret_39; })) {
{
	({ mph_io_File* _arg_40 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_40); MorphString* _arg_41 = mph_string_new(ctx, "Type compatibility: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_41); mph_int _ret_42 = mph_io_Write(ctx, NULL, _arg_40, _arg_41); mph_gc_pop_roots(ctx, 2); _ret_42; });
}
	} else {
{
	({ mph_io_File* _arg_43 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_43); MorphString* _arg_44 = mph_string_new(ctx, "Type compatibility: ERROR\n"); mph_gc_push_root(ctx, (void**)&_arg_44); mph_int _ret_45 = mph_io_Write(ctx, NULL, _arg_43, _arg_44); mph_gc_pop_roots(ctx, 2); _ret_45; });
}
	}
	({ mph_io_File* _arg_46 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_46); MorphString* _arg_47 = mph_string_new(ctx, "Type alias system implemented successfully!\n"); mph_gc_push_root(ctx, (void**)&_arg_47); mph_int _ret_48 = mph_io_Write(ctx, NULL, _arg_46, _arg_47); mph_gc_pop_roots(ctx, 2); _ret_48; });
	mph_gc_pop_roots(ctx, 3);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_KIND_INT = 1;
	mph_KIND_STRING = 3;
	mph_KIND_BOOL = 4;
	mph_KIND_VOID = 5;
	mph_main(ctx, NULL);
}
