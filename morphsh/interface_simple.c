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
#define MPH_TYPE_mph_SimpleInterface 1
#define MPH_TYPE_mph_SimpleStruct 2

// Struct Definitions (Env & Types)
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

MorphTypeInfo mph_ti_mph_SimpleInterface = { "SimpleInterface", sizeof(mph_SimpleInterface), 2, (size_t[]){offsetof(mph_SimpleInterface, name), offsetof(mph_SimpleInterface, method_name)} };
MorphTypeInfo mph_ti_mph_SimpleStruct = { "SimpleStruct", sizeof(mph_SimpleStruct), 2, (size_t[]){offsetof(mph_SimpleStruct, name), offsetof(mph_SimpleStruct, method_name)} };

// Global Variables
mph_int mph_io_Stdout;
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_INTERFACE;
mph_int mph_KIND_ERROR;

// Function Prototypes
mph_SimpleInterface* mph_make_interface(MorphContext* ctx, void* _env_void, MorphString* name, MorphString* method_name);
mph_SimpleStruct* mph_make_struct(MorphContext* ctx, void* _env_void, MorphString* name, MorphString* method_name);
mph_bool mph_implements_interface(MorphContext* ctx, void* _env_void, mph_SimpleStruct* struct_type, mph_SimpleInterface* interface_type);
void mph_main(MorphContext* ctx, void* _env_void);
mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_int fd, MorphString* data);
MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_int fd, mph_int size);
void mph_io_Print(MorphContext* ctx, void* _env_void, MorphString* data);
void mph_io_Println(MorphContext* ctx, void* _env_void, MorphString* data);

// Function Definitions
mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_int fd, MorphString* data);

MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_int fd, mph_int size);

void mph_io_Print(MorphContext* ctx, void* _env_void, MorphString* data);

void mph_io_Println(MorphContext* ctx, void* _env_void, MorphString* data);

mph_SimpleInterface* mph_make_interface(MorphContext* ctx, void* _env_void, MorphString* name, MorphString* method_name) {
	mph_gc_push_root(ctx, (void**)&name);
	mph_gc_push_root(ctx, (void**)&method_name);
	return ({ mph_SimpleInterface* _ret_1 = ({ mph_SimpleInterface* _t = (mph_SimpleInterface*)mph_alloc(ctx, sizeof(mph_SimpleInterface), &mph_ti_mph_SimpleInterface); mph_gc_push_root(ctx, (void**)&_t); _t->method_name = method_name; _t->has_method = 1; _t->name = name; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_SimpleInterface* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
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
	({ MorphString* _arg_9 = mph_string_new(ctx, "=== INTERFACE SIMPLE TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_9); mph_int _ret_10 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_9); mph_gc_pop_roots(ctx, 1); _ret_10; });
	mph_SimpleInterface* writer = ({ MorphString* _arg_11 = mph_string_new(ctx, "Writer"); mph_gc_push_root(ctx, (void**)&_arg_11); MorphString* _arg_12 = mph_string_new(ctx, "Write"); mph_gc_push_root(ctx, (void**)&_arg_12); mph_SimpleInterface* _ret_13 = mph_make_interface(ctx, NULL, _arg_11, _arg_12); mph_gc_pop_roots(ctx, 2); _ret_13; });
	mph_gc_push_root(ctx, (void**)&writer);
	if (({ mph_SimpleInterface* _obj_14 = writer; mph_gc_push_root(ctx, (void**)&_obj_14); mph_bool _ret_15 = _obj_14->has_method; mph_gc_pop_roots(ctx, 1); _ret_15; })) {
{
	({ MorphString* _arg_16 = mph_string_new(ctx, "â Create Writer interface: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_16); mph_int _ret_17 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_16); mph_gc_pop_roots(ctx, 1); _ret_17; });
}
	} else {
{
	({ MorphString* _arg_18 = mph_string_new(ctx, "â Create Writer interface: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_18); mph_int _ret_19 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_18); mph_gc_pop_roots(ctx, 1); _ret_19; });
}
	}
	mph_SimpleStruct* file = ({ MorphString* _arg_20 = mph_string_new(ctx, "File"); mph_gc_push_root(ctx, (void**)&_arg_20); MorphString* _arg_21 = mph_string_new(ctx, "Write"); mph_gc_push_root(ctx, (void**)&_arg_21); mph_SimpleStruct* _ret_22 = mph_make_struct(ctx, NULL, _arg_20, _arg_21); mph_gc_pop_roots(ctx, 2); _ret_22; });
	mph_gc_push_root(ctx, (void**)&file);
	if (({ mph_SimpleStruct* _obj_23 = file; mph_gc_push_root(ctx, (void**)&_obj_23); mph_bool _ret_24 = _obj_23->has_method; mph_gc_pop_roots(ctx, 1); _ret_24; })) {
{
	({ MorphString* _arg_25 = mph_string_new(ctx, "â Create File struct: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_25); mph_int _ret_26 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_25); mph_gc_pop_roots(ctx, 1); _ret_26; });
}
	} else {
{
	({ MorphString* _arg_27 = mph_string_new(ctx, "â Create File struct: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_27); mph_int _ret_28 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_27); mph_gc_pop_roots(ctx, 1); _ret_28; });
}
	}
	if (({ mph_SimpleStruct* _arg_29 = file; mph_gc_push_root(ctx, (void**)&_arg_29); mph_SimpleInterface* _arg_30 = writer; mph_gc_push_root(ctx, (void**)&_arg_30); mph_bool _ret_31 = mph_implements_interface(ctx, NULL, _arg_29, _arg_30); mph_gc_pop_roots(ctx, 2); _ret_31; })) {
{
	({ MorphString* _arg_32 = mph_string_new(ctx, "â File implements Writer: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_32); mph_int _ret_33 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_32); mph_gc_pop_roots(ctx, 1); _ret_33; });
}
	} else {
{
	({ MorphString* _arg_34 = mph_string_new(ctx, "â File implements Writer: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_34); mph_int _ret_35 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_34); mph_gc_pop_roots(ctx, 1); _ret_35; });
}
	}
	mph_SimpleStruct* bad_file = ({ MorphString* _arg_36 = mph_string_new(ctx, "BadFile"); mph_gc_push_root(ctx, (void**)&_arg_36); MorphString* _arg_37 = mph_string_new(ctx, "Read"); mph_gc_push_root(ctx, (void**)&_arg_37); mph_SimpleStruct* _ret_38 = mph_make_struct(ctx, NULL, _arg_36, _arg_37); mph_gc_pop_roots(ctx, 2); _ret_38; });
	mph_gc_push_root(ctx, (void**)&bad_file);
	if ((({ mph_SimpleStruct* _arg_39 = bad_file; mph_gc_push_root(ctx, (void**)&_arg_39); mph_SimpleInterface* _arg_40 = writer; mph_gc_push_root(ctx, (void**)&_arg_40); mph_bool _ret_41 = mph_implements_interface(ctx, NULL, _arg_39, _arg_40); mph_gc_pop_roots(ctx, 2); _ret_41; }) == 0)) {
{
	({ MorphString* _arg_42 = mph_string_new(ctx, "â BadFile doesn't implement Writer: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_42); mph_int _ret_43 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_42); mph_gc_pop_roots(ctx, 1); _ret_43; });
}
	} else {
{
	({ MorphString* _arg_44 = mph_string_new(ctx, "â BadFile doesn't implement Writer: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_44); mph_int _ret_45 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_44); mph_gc_pop_roots(ctx, 1); _ret_45; });
}
	}
	({ MorphString* _arg_46 = mph_string_new(ctx, "\nð¯ INTERFACE SIMPLE COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_46); mph_int _ret_47 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_46); mph_gc_pop_roots(ctx, 1); _ret_47; });
	({ MorphString* _arg_48 = mph_string_new(ctx, "â Interface creation working\n"); mph_gc_push_root(ctx, (void**)&_arg_48); mph_int _ret_49 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_48); mph_gc_pop_roots(ctx, 1); _ret_49; });
	({ MorphString* _arg_50 = mph_string_new(ctx, "â Duck typing working\n"); mph_gc_push_root(ctx, (void**)&_arg_50); mph_int _ret_51 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_50); mph_gc_pop_roots(ctx, 1); _ret_51; });
	({ MorphString* _arg_52 = mph_string_new(ctx, "â Implementation checking\n"); mph_gc_push_root(ctx, (void**)&_arg_52); mph_int _ret_53 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_52); mph_gc_pop_roots(ctx, 1); _ret_53; });
	mph_gc_pop_roots(ctx, 3);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = 1;
	mph_KIND_INT = 0;
	mph_KIND_STRING = 1;
	mph_KIND_INTERFACE = 7;
	mph_KIND_ERROR = (-1);
	mph_main(ctx, NULL);
}
