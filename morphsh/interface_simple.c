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
#define MPH_TYPE_mph_SimpleInterface 2
#define MPH_TYPE_mph_SimpleStruct 3

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_SimpleInterface mph_SimpleInterface;
typedef struct mph_SimpleStruct mph_SimpleStruct;
struct mph_SimpleInterface {
	MorphString* name;
	MorphString* method_name;
	mph_bool has_method;
};

struct mph_SimpleStruct {
	MorphString* name;
	MorphString* method_name;
	mph_bool has_method;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_SimpleInterface = { "SimpleInterface", sizeof(mph_SimpleInterface), 2, (size_t[]){offsetof(mph_SimpleInterface, name), offsetof(mph_SimpleInterface, method_name)} };
MorphTypeInfo mph_ti_mph_SimpleStruct = { "SimpleStruct", sizeof(mph_SimpleStruct), 2, (size_t[]){offsetof(mph_SimpleStruct, name), offsetof(mph_SimpleStruct, method_name)} };

// Global Variables
mph_io_File* mph_io_Stdout;
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_INTERFACE;
mph_int mph_KIND_ERROR;

// Function Prototypes
mph_SimpleInterface* mph_make_interface(MorphContext* ctx, void* _env_void, MorphString* name, MorphString* method_name);
mph_SimpleStruct* mph_make_struct(MorphContext* ctx, void* _env_void, MorphString* name, MorphString* method_name);
mph_bool mph_implements_interface(MorphContext* ctx, void* _env_void, mph_SimpleStruct* struct_type, mph_SimpleInterface* interface_type);
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

mph_SimpleInterface* mph_make_interface(MorphContext* ctx, void* _env_void, MorphString* name, MorphString* method_name) {
	mph_gc_push_root(ctx, (void**)&name);
	mph_gc_push_root(ctx, (void**)&method_name);
	return ({ mph_SimpleInterface* _ret_1 = ({ mph_SimpleInterface* _t = (mph_SimpleInterface*)mph_alloc(ctx, sizeof(mph_SimpleInterface), &mph_ti_mph_SimpleInterface); mph_gc_push_root(ctx, (void**)&_t); _t->name = name; _t->method_name = method_name; _t->has_method = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_SimpleInterface* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
	mph_gc_pop_roots(ctx, 2);
}

mph_SimpleStruct* mph_make_struct(MorphContext* ctx, void* _env_void, MorphString* name, MorphString* method_name) {
	mph_gc_push_root(ctx, (void**)&name);
	mph_gc_push_root(ctx, (void**)&method_name);
	return ({ mph_SimpleStruct* _ret_3 = ({ mph_SimpleStruct* _t = (mph_SimpleStruct*)mph_alloc(ctx, sizeof(mph_SimpleStruct), &mph_ti_mph_SimpleStruct); mph_gc_push_root(ctx, (void**)&_t); _t->name = name; _t->method_name = method_name; _t->has_method = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_SimpleStruct* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
	mph_gc_pop_roots(ctx, 2);
}

mph_bool mph_implements_interface(MorphContext* ctx, void* _env_void, mph_SimpleStruct* struct_type, mph_SimpleInterface* interface_type) {
	mph_gc_push_root(ctx, (void**)&struct_type);
	mph_gc_push_root(ctx, (void**)&interface_type);
	if (mph_string_eq(ctx, ({ mph_SimpleStruct* _obj_5 = struct_type; mph_gc_push_root(ctx, (void**)&_obj_5); MorphString* _ret_6 = _obj_5->method_name; mph_gc_pop_roots(ctx, 1); _ret_6; }), ({ mph_SimpleInterface* _obj_7 = interface_type; mph_gc_push_root(ctx, (void**)&_obj_7); MorphString* _ret_8 = _obj_7->method_name; mph_gc_pop_roots(ctx, 1); _ret_8; }))) {
{
	return 1;
}
	}
	return 0;
	mph_gc_pop_roots(ctx, 2);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_9 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_9); MorphString* _arg_10 = mph_string_new(ctx, "=== INTERFACE SIMPLE TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_10); mph_int _ret_11 = mph_io_Write(ctx, NULL, _arg_9, _arg_10); mph_gc_pop_roots(ctx, 2); _ret_11; });
	mph_SimpleInterface* writer = ({ MorphString* _arg_12 = mph_string_new(ctx, "Writer"); mph_gc_push_root(ctx, (void**)&_arg_12); MorphString* _arg_13 = mph_string_new(ctx, "Write"); mph_gc_push_root(ctx, (void**)&_arg_13); mph_SimpleInterface* _ret_14 = mph_make_interface(ctx, NULL, _arg_12, _arg_13); mph_gc_pop_roots(ctx, 2); _ret_14; });
	mph_gc_push_root(ctx, (void**)&writer);
	if (({ mph_SimpleInterface* _obj_15 = writer; mph_gc_push_root(ctx, (void**)&_obj_15); mph_bool _ret_16 = _obj_15->has_method; mph_gc_pop_roots(ctx, 1); _ret_16; })) {
{
	({ mph_io_File* _arg_17 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_17); MorphString* _arg_18 = mph_string_new(ctx, "â Create Writer interface: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_18); mph_int _ret_19 = mph_io_Write(ctx, NULL, _arg_17, _arg_18); mph_gc_pop_roots(ctx, 2); _ret_19; });
}
	} else {
{
	({ mph_io_File* _arg_20 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_20); MorphString* _arg_21 = mph_string_new(ctx, "â Create Writer interface: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_21); mph_int _ret_22 = mph_io_Write(ctx, NULL, _arg_20, _arg_21); mph_gc_pop_roots(ctx, 2); _ret_22; });
}
	}
	mph_SimpleStruct* file = ({ MorphString* _arg_23 = mph_string_new(ctx, "File"); mph_gc_push_root(ctx, (void**)&_arg_23); MorphString* _arg_24 = mph_string_new(ctx, "Write"); mph_gc_push_root(ctx, (void**)&_arg_24); mph_SimpleStruct* _ret_25 = mph_make_struct(ctx, NULL, _arg_23, _arg_24); mph_gc_pop_roots(ctx, 2); _ret_25; });
	mph_gc_push_root(ctx, (void**)&file);
	if (({ mph_SimpleStruct* _obj_26 = file; mph_gc_push_root(ctx, (void**)&_obj_26); mph_bool _ret_27 = _obj_26->has_method; mph_gc_pop_roots(ctx, 1); _ret_27; })) {
{
	({ mph_io_File* _arg_28 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_28); MorphString* _arg_29 = mph_string_new(ctx, "â Create File struct: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_29); mph_int _ret_30 = mph_io_Write(ctx, NULL, _arg_28, _arg_29); mph_gc_pop_roots(ctx, 2); _ret_30; });
}
	} else {
{
	({ mph_io_File* _arg_31 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_31); MorphString* _arg_32 = mph_string_new(ctx, "â Create File struct: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_32); mph_int _ret_33 = mph_io_Write(ctx, NULL, _arg_31, _arg_32); mph_gc_pop_roots(ctx, 2); _ret_33; });
}
	}
	if (({ mph_SimpleStruct* _arg_34 = file; mph_gc_push_root(ctx, (void**)&_arg_34); mph_SimpleInterface* _arg_35 = writer; mph_gc_push_root(ctx, (void**)&_arg_35); mph_bool _ret_36 = mph_implements_interface(ctx, NULL, _arg_34, _arg_35); mph_gc_pop_roots(ctx, 2); _ret_36; })) {
{
	({ mph_io_File* _arg_37 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_37); MorphString* _arg_38 = mph_string_new(ctx, "â File implements Writer: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_38); mph_int _ret_39 = mph_io_Write(ctx, NULL, _arg_37, _arg_38); mph_gc_pop_roots(ctx, 2); _ret_39; });
}
	} else {
{
	({ mph_io_File* _arg_40 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_40); MorphString* _arg_41 = mph_string_new(ctx, "â File implements Writer: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_41); mph_int _ret_42 = mph_io_Write(ctx, NULL, _arg_40, _arg_41); mph_gc_pop_roots(ctx, 2); _ret_42; });
}
	}
	mph_SimpleStruct* bad_file = ({ MorphString* _arg_43 = mph_string_new(ctx, "BadFile"); mph_gc_push_root(ctx, (void**)&_arg_43); MorphString* _arg_44 = mph_string_new(ctx, "Read"); mph_gc_push_root(ctx, (void**)&_arg_44); mph_SimpleStruct* _ret_45 = mph_make_struct(ctx, NULL, _arg_43, _arg_44); mph_gc_pop_roots(ctx, 2); _ret_45; });
	mph_gc_push_root(ctx, (void**)&bad_file);
	if ((({ mph_SimpleStruct* _arg_46 = bad_file; mph_gc_push_root(ctx, (void**)&_arg_46); mph_SimpleInterface* _arg_47 = writer; mph_gc_push_root(ctx, (void**)&_arg_47); mph_bool _ret_48 = mph_implements_interface(ctx, NULL, _arg_46, _arg_47); mph_gc_pop_roots(ctx, 2); _ret_48; }) == 0)) {
{
	({ mph_io_File* _arg_49 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_49); MorphString* _arg_50 = mph_string_new(ctx, "â BadFile doesn't implement Writer: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_50); mph_int _ret_51 = mph_io_Write(ctx, NULL, _arg_49, _arg_50); mph_gc_pop_roots(ctx, 2); _ret_51; });
}
	} else {
{
	({ mph_io_File* _arg_52 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_52); MorphString* _arg_53 = mph_string_new(ctx, "â BadFile doesn't implement Writer: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_53); mph_int _ret_54 = mph_io_Write(ctx, NULL, _arg_52, _arg_53); mph_gc_pop_roots(ctx, 2); _ret_54; });
}
	}
	({ mph_io_File* _arg_55 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_55); MorphString* _arg_56 = mph_string_new(ctx, "\nð¯ INTERFACE SIMPLE COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_56); mph_int _ret_57 = mph_io_Write(ctx, NULL, _arg_55, _arg_56); mph_gc_pop_roots(ctx, 2); _ret_57; });
	({ mph_io_File* _arg_58 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_58); MorphString* _arg_59 = mph_string_new(ctx, "â Interface creation working\n"); mph_gc_push_root(ctx, (void**)&_arg_59); mph_int _ret_60 = mph_io_Write(ctx, NULL, _arg_58, _arg_59); mph_gc_pop_roots(ctx, 2); _ret_60; });
	({ mph_io_File* _arg_61 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_61); MorphString* _arg_62 = mph_string_new(ctx, "â Duck typing working\n"); mph_gc_push_root(ctx, (void**)&_arg_62); mph_int _ret_63 = mph_io_Write(ctx, NULL, _arg_61, _arg_62); mph_gc_pop_roots(ctx, 2); _ret_63; });
	({ mph_io_File* _arg_64 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_64); MorphString* _arg_65 = mph_string_new(ctx, "â Implementation checking\n"); mph_gc_push_root(ctx, (void**)&_arg_65); mph_int _ret_66 = mph_io_Write(ctx, NULL, _arg_64, _arg_65); mph_gc_pop_roots(ctx, 2); _ret_66; });
	mph_gc_pop_roots(ctx, 3);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_KIND_INT = 0;
	mph_KIND_STRING = 1;
	mph_KIND_INTERFACE = 7;
	mph_KIND_ERROR = (-1);
	mph_main(ctx, NULL);
}
