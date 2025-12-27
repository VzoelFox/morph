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
#define MPH_TYPE_mph_BootstrapChecker 3
#define MPH_TYPE_mph_io_File 1
#define MPH_TYPE_mph_BootstrapType 2

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_BootstrapType mph_BootstrapType;
typedef struct mph_BootstrapChecker mph_BootstrapChecker;
struct mph_BootstrapType {
	mph_int kind;
	MorphString* name;
};

struct mph_BootstrapChecker {
	mph_int symbol_count;
	mph_bool has_errors;
	mph_int error_count;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_BootstrapType = { "BootstrapType", sizeof(mph_BootstrapType), 1, (size_t[]){offsetof(mph_BootstrapType, name)} };
MorphTypeInfo mph_ti_mph_BootstrapChecker = { "BootstrapChecker", sizeof(mph_BootstrapChecker), 0, NULL };

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
mph_BootstrapChecker* mph_checker_new(MorphContext* ctx, void* _env_void);
mph_BootstrapType* mph_check_binary_expr(MorphContext* ctx, void* _env_void, mph_BootstrapType* left_type, MorphString* op, mph_BootstrapType* right_type);
mph_bool mph_check_assignment(MorphContext* ctx, void* _env_void, mph_BootstrapType* var_type, mph_BootstrapType* value_type);
mph_BootstrapType* mph_check_function_call(MorphContext* ctx, void* _env_void, MorphString* func_name);
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
	return ({ mph_BootstrapType* _ret_7 = ({ mph_BootstrapType* _t = (mph_BootstrapType*)mph_alloc(ctx, sizeof(mph_BootstrapType), &mph_ti_mph_BootstrapType); mph_gc_push_root(ctx, (void**)&_t); _t->name = mph_string_new(ctx, "void"); _t->kind = mph_KIND_VOID; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_7); mph_BootstrapType* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
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

mph_BootstrapChecker* mph_checker_new(MorphContext* ctx, void* _env_void) {
	return ({ mph_BootstrapChecker* _ret_18 = ({ mph_BootstrapChecker* _t = (mph_BootstrapChecker*)mph_alloc(ctx, sizeof(mph_BootstrapChecker), &mph_ti_mph_BootstrapChecker); mph_gc_push_root(ctx, (void**)&_t); _t->symbol_count = 0; _t->has_errors = 0; _t->error_count = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_18); mph_BootstrapChecker* _ret_19 = _ret_18; mph_gc_pop_roots(ctx, 1); _ret_19; });
}

mph_BootstrapType* mph_check_binary_expr(MorphContext* ctx, void* _env_void, mph_BootstrapType* left_type, MorphString* op, mph_BootstrapType* right_type) {
	mph_gc_push_root(ctx, (void**)&left_type);
	mph_gc_push_root(ctx, (void**)&op);
	mph_gc_push_root(ctx, (void**)&right_type);
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "+"))) {
{
	if ((({ mph_BootstrapType* _obj_20 = left_type; mph_gc_push_root(ctx, (void**)&_obj_20); mph_int _ret_21 = _obj_20->kind; mph_gc_pop_roots(ctx, 1); _ret_21; }) == mph_KIND_INT)) {
{
	if ((({ mph_BootstrapType* _obj_22 = right_type; mph_gc_push_root(ctx, (void**)&_obj_22); mph_int _ret_23 = _obj_22->kind; mph_gc_pop_roots(ctx, 1); _ret_23; }) == mph_KIND_INT)) {
{
	return ({ mph_BootstrapType* _ret_24 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_24); mph_BootstrapType* _ret_25 = _ret_24; mph_gc_pop_roots(ctx, 1); _ret_25; });
}
	}
}
	}
	if ((({ mph_BootstrapType* _obj_26 = left_type; mph_gc_push_root(ctx, (void**)&_obj_26); mph_int _ret_27 = _obj_26->kind; mph_gc_pop_roots(ctx, 1); _ret_27; }) == mph_KIND_STRING)) {
{
	if ((({ mph_BootstrapType* _obj_28 = right_type; mph_gc_push_root(ctx, (void**)&_obj_28); mph_int _ret_29 = _obj_28->kind; mph_gc_pop_roots(ctx, 1); _ret_29; }) == mph_KIND_STRING)) {
{
	return ({ mph_BootstrapType* _ret_30 = mph_make_string_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_30); mph_BootstrapType* _ret_31 = _ret_30; mph_gc_pop_roots(ctx, 1); _ret_31; });
}
	}
}
	}
	return ({ mph_BootstrapType* _ret_32 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_32); mph_BootstrapType* _ret_33 = _ret_32; mph_gc_pop_roots(ctx, 1); _ret_33; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "=="))) {
{
	if (({ mph_BootstrapType* _arg_34 = left_type; mph_gc_push_root(ctx, (void**)&_arg_34); mph_BootstrapType* _arg_35 = right_type; mph_gc_push_root(ctx, (void**)&_arg_35); mph_bool _ret_36 = mph_types_equal(ctx, NULL, _arg_34, _arg_35); mph_gc_pop_roots(ctx, 2); _ret_36; })) {
{
	return ({ mph_BootstrapType* _ret_37 = mph_make_bool_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_37); mph_BootstrapType* _ret_38 = _ret_37; mph_gc_pop_roots(ctx, 1); _ret_38; });
}
	}
	return ({ mph_BootstrapType* _ret_39 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_39); mph_BootstrapType* _ret_40 = _ret_39; mph_gc_pop_roots(ctx, 1); _ret_40; });
}
	}
	return ({ mph_BootstrapType* _ret_41 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_41); mph_BootstrapType* _ret_42 = _ret_41; mph_gc_pop_roots(ctx, 1); _ret_42; });
	mph_gc_pop_roots(ctx, 3);
}

mph_bool mph_check_assignment(MorphContext* ctx, void* _env_void, mph_BootstrapType* var_type, mph_BootstrapType* value_type) {
	mph_gc_push_root(ctx, (void**)&var_type);
	mph_gc_push_root(ctx, (void**)&value_type);
	return ({ mph_BootstrapType* _arg_43 = value_type; mph_gc_push_root(ctx, (void**)&_arg_43); mph_BootstrapType* _arg_44 = var_type; mph_gc_push_root(ctx, (void**)&_arg_44); mph_bool _ret_45 = mph_types_assignable(ctx, NULL, _arg_43, _arg_44); mph_gc_pop_roots(ctx, 2); _ret_45; });
	mph_gc_pop_roots(ctx, 2);
}

mph_BootstrapType* mph_check_function_call(MorphContext* ctx, void* _env_void, MorphString* func_name) {
	mph_gc_push_root(ctx, (void**)&func_name);
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "len"))) {
{
	return ({ mph_BootstrapType* _ret_46 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_46); mph_BootstrapType* _ret_47 = _ret_46; mph_gc_pop_roots(ctx, 1); _ret_47; });
}
	}
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "print"))) {
{
	return ({ mph_BootstrapType* _ret_48 = mph_make_void_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_48); mph_BootstrapType* _ret_49 = _ret_48; mph_gc_pop_roots(ctx, 1); _ret_49; });
}
	}
	return ({ mph_BootstrapType* _ret_50 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_50); mph_BootstrapType* _ret_51 = _ret_50; mph_gc_pop_roots(ctx, 1); _ret_51; });
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_52 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_52); MorphString* _arg_53 = mph_string_new(ctx, "=== BOOTSTRAP TYPE CHECKER TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_53); mph_int _ret_54 = mph_io_Write(ctx, NULL, _arg_52, _arg_53); mph_gc_pop_roots(ctx, 2); _ret_54; });
	mph_BootstrapChecker* checker = mph_checker_new(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&checker);
	mph_BootstrapType* int_type = mph_make_int_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&int_type);
	mph_BootstrapType* result1 = ({ mph_BootstrapType* _arg_55 = int_type; mph_gc_push_root(ctx, (void**)&_arg_55); MorphString* _arg_56 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_56); mph_BootstrapType* _arg_57 = int_type; mph_gc_push_root(ctx, (void**)&_arg_57); mph_BootstrapType* _ret_58 = mph_check_binary_expr(ctx, NULL, _arg_55, _arg_56, _arg_57); mph_gc_pop_roots(ctx, 3); _ret_58; });
	mph_gc_push_root(ctx, (void**)&result1);
	if ((({ mph_BootstrapType* _obj_59 = result1; mph_gc_push_root(ctx, (void**)&_obj_59); mph_int _ret_60 = _obj_59->kind; mph_gc_pop_roots(ctx, 1); _ret_60; }) == mph_KIND_INT)) {
{
	({ mph_io_File* _arg_61 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_61); MorphString* _arg_62 = mph_string_new(ctx, "â int + int -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_62); mph_int _ret_63 = mph_io_Write(ctx, NULL, _arg_61, _arg_62); mph_gc_pop_roots(ctx, 2); _ret_63; });
}
	} else {
{
	({ mph_io_File* _arg_64 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_64); MorphString* _arg_65 = mph_string_new(ctx, "â int + int -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_65); mph_int _ret_66 = mph_io_Write(ctx, NULL, _arg_64, _arg_65); mph_gc_pop_roots(ctx, 2); _ret_66; });
}
	}
	mph_BootstrapType* string_type = mph_make_string_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&string_type);
	mph_BootstrapType* result2 = ({ mph_BootstrapType* _arg_67 = string_type; mph_gc_push_root(ctx, (void**)&_arg_67); MorphString* _arg_68 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_68); mph_BootstrapType* _arg_69 = string_type; mph_gc_push_root(ctx, (void**)&_arg_69); mph_BootstrapType* _ret_70 = mph_check_binary_expr(ctx, NULL, _arg_67, _arg_68, _arg_69); mph_gc_pop_roots(ctx, 3); _ret_70; });
	mph_gc_push_root(ctx, (void**)&result2);
	if ((({ mph_BootstrapType* _obj_71 = result2; mph_gc_push_root(ctx, (void**)&_obj_71); mph_int _ret_72 = _obj_71->kind; mph_gc_pop_roots(ctx, 1); _ret_72; }) == mph_KIND_STRING)) {
{
	({ mph_io_File* _arg_73 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_73); MorphString* _arg_74 = mph_string_new(ctx, "â string + string -> string: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_74); mph_int _ret_75 = mph_io_Write(ctx, NULL, _arg_73, _arg_74); mph_gc_pop_roots(ctx, 2); _ret_75; });
}
	} else {
{
	({ mph_io_File* _arg_76 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_76); MorphString* _arg_77 = mph_string_new(ctx, "â string + string -> string: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_77); mph_int _ret_78 = mph_io_Write(ctx, NULL, _arg_76, _arg_77); mph_gc_pop_roots(ctx, 2); _ret_78; });
}
	}
	mph_BootstrapType* result3 = ({ mph_BootstrapType* _arg_79 = int_type; mph_gc_push_root(ctx, (void**)&_arg_79); MorphString* _arg_80 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_80); mph_BootstrapType* _arg_81 = string_type; mph_gc_push_root(ctx, (void**)&_arg_81); mph_BootstrapType* _ret_82 = mph_check_binary_expr(ctx, NULL, _arg_79, _arg_80, _arg_81); mph_gc_pop_roots(ctx, 3); _ret_82; });
	mph_gc_push_root(ctx, (void**)&result3);
	if ((({ mph_BootstrapType* _obj_83 = result3; mph_gc_push_root(ctx, (void**)&_obj_83); mph_int _ret_84 = _obj_83->kind; mph_gc_pop_roots(ctx, 1); _ret_84; }) == mph_KIND_ERROR)) {
{
	({ mph_io_File* _arg_85 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_85); MorphString* _arg_86 = mph_string_new(ctx, "â int + string -> error: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_86); mph_int _ret_87 = mph_io_Write(ctx, NULL, _arg_85, _arg_86); mph_gc_pop_roots(ctx, 2); _ret_87; });
}
	} else {
{
	({ mph_io_File* _arg_88 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_88); MorphString* _arg_89 = mph_string_new(ctx, "â int + string -> error: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_89); mph_int _ret_90 = mph_io_Write(ctx, NULL, _arg_88, _arg_89); mph_gc_pop_roots(ctx, 2); _ret_90; });
}
	}
	mph_BootstrapType* result4 = ({ mph_BootstrapType* _arg_91 = int_type; mph_gc_push_root(ctx, (void**)&_arg_91); MorphString* _arg_92 = mph_string_new(ctx, "=="); mph_gc_push_root(ctx, (void**)&_arg_92); mph_BootstrapType* _arg_93 = int_type; mph_gc_push_root(ctx, (void**)&_arg_93); mph_BootstrapType* _ret_94 = mph_check_binary_expr(ctx, NULL, _arg_91, _arg_92, _arg_93); mph_gc_pop_roots(ctx, 3); _ret_94; });
	mph_gc_push_root(ctx, (void**)&result4);
	if ((({ mph_BootstrapType* _obj_95 = result4; mph_gc_push_root(ctx, (void**)&_obj_95); mph_int _ret_96 = _obj_95->kind; mph_gc_pop_roots(ctx, 1); _ret_96; }) == mph_KIND_BOOL)) {
{
	({ mph_io_File* _arg_97 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_97); MorphString* _arg_98 = mph_string_new(ctx, "â int == int -> bool: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_98); mph_int _ret_99 = mph_io_Write(ctx, NULL, _arg_97, _arg_98); mph_gc_pop_roots(ctx, 2); _ret_99; });
}
	} else {
{
	({ mph_io_File* _arg_100 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_100); MorphString* _arg_101 = mph_string_new(ctx, "â int == int -> bool: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_101); mph_int _ret_102 = mph_io_Write(ctx, NULL, _arg_100, _arg_101); mph_gc_pop_roots(ctx, 2); _ret_102; });
}
	}
	mph_bool assign_ok = ({ mph_BootstrapType* _arg_103 = int_type; mph_gc_push_root(ctx, (void**)&_arg_103); mph_BootstrapType* _arg_104 = int_type; mph_gc_push_root(ctx, (void**)&_arg_104); mph_bool _ret_105 = mph_check_assignment(ctx, NULL, _arg_103, _arg_104); mph_gc_pop_roots(ctx, 2); _ret_105; });
	if (assign_ok) {
{
	({ mph_io_File* _arg_106 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_106); MorphString* _arg_107 = mph_string_new(ctx, "â int = int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_107); mph_int _ret_108 = mph_io_Write(ctx, NULL, _arg_106, _arg_107); mph_gc_pop_roots(ctx, 2); _ret_108; });
}
	} else {
{
	({ mph_io_File* _arg_109 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_109); MorphString* _arg_110 = mph_string_new(ctx, "â int = int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_110); mph_int _ret_111 = mph_io_Write(ctx, NULL, _arg_109, _arg_110); mph_gc_pop_roots(ctx, 2); _ret_111; });
}
	}
	mph_bool assign_fail = ({ mph_BootstrapType* _arg_112 = int_type; mph_gc_push_root(ctx, (void**)&_arg_112); mph_BootstrapType* _arg_113 = string_type; mph_gc_push_root(ctx, (void**)&_arg_113); mph_bool _ret_114 = mph_check_assignment(ctx, NULL, _arg_112, _arg_113); mph_gc_pop_roots(ctx, 2); _ret_114; });
	if ((assign_fail == 0)) {
{
	({ mph_io_File* _arg_115 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_115); MorphString* _arg_116 = mph_string_new(ctx, "â int = string (rejected): PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_116); mph_int _ret_117 = mph_io_Write(ctx, NULL, _arg_115, _arg_116); mph_gc_pop_roots(ctx, 2); _ret_117; });
}
	} else {
{
	({ mph_io_File* _arg_118 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_118); MorphString* _arg_119 = mph_string_new(ctx, "â int = string (rejected): FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_119); mph_int _ret_120 = mph_io_Write(ctx, NULL, _arg_118, _arg_119); mph_gc_pop_roots(ctx, 2); _ret_120; });
}
	}
	mph_BootstrapType* len_result = ({ MorphString* _arg_121 = mph_string_new(ctx, "len"); mph_gc_push_root(ctx, (void**)&_arg_121); mph_BootstrapType* _ret_122 = mph_check_function_call(ctx, NULL, _arg_121); mph_gc_pop_roots(ctx, 1); _ret_122; });
	mph_gc_push_root(ctx, (void**)&len_result);
	if ((({ mph_BootstrapType* _obj_123 = len_result; mph_gc_push_root(ctx, (void**)&_obj_123); mph_int _ret_124 = _obj_123->kind; mph_gc_pop_roots(ctx, 1); _ret_124; }) == mph_KIND_INT)) {
{
	({ mph_io_File* _arg_125 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_125); MorphString* _arg_126 = mph_string_new(ctx, "â len() -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_126); mph_int _ret_127 = mph_io_Write(ctx, NULL, _arg_125, _arg_126); mph_gc_pop_roots(ctx, 2); _ret_127; });
}
	} else {
{
	({ mph_io_File* _arg_128 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_128); MorphString* _arg_129 = mph_string_new(ctx, "â len() -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_129); mph_int _ret_130 = mph_io_Write(ctx, NULL, _arg_128, _arg_129); mph_gc_pop_roots(ctx, 2); _ret_130; });
}
	}
	({ mph_io_File* _arg_131 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_131); MorphString* _arg_132 = mph_string_new(ctx, "\nð¯ BOOTSTRAP SEMANTIC ANALYSIS COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_132); mph_int _ret_133 = mph_io_Write(ctx, NULL, _arg_131, _arg_132); mph_gc_pop_roots(ctx, 2); _ret_133; });
	({ mph_io_File* _arg_134 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_134); MorphString* _arg_135 = mph_string_new(ctx, "â Go-compatible type rules\n"); mph_gc_push_root(ctx, (void**)&_arg_135); mph_int _ret_136 = mph_io_Write(ctx, NULL, _arg_134, _arg_135); mph_gc_pop_roots(ctx, 2); _ret_136; });
	({ mph_io_File* _arg_137 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_137); MorphString* _arg_138 = mph_string_new(ctx, "â Assignment compatibility\n"); mph_gc_push_root(ctx, (void**)&_arg_138); mph_int _ret_139 = mph_io_Write(ctx, NULL, _arg_137, _arg_138); mph_gc_pop_roots(ctx, 2); _ret_139; });
	({ mph_io_File* _arg_140 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_140); MorphString* _arg_141 = mph_string_new(ctx, "â Binary expression checking\n"); mph_gc_push_root(ctx, (void**)&_arg_141); mph_int _ret_142 = mph_io_Write(ctx, NULL, _arg_140, _arg_141); mph_gc_pop_roots(ctx, 2); _ret_142; });
	({ mph_io_File* _arg_143 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_143); MorphString* _arg_144 = mph_string_new(ctx, "â Built-in function validation\n"); mph_gc_push_root(ctx, (void**)&_arg_144); mph_int _ret_145 = mph_io_Write(ctx, NULL, _arg_143, _arg_144); mph_gc_pop_roots(ctx, 2); _ret_145; });
	mph_gc_pop_roots(ctx, 8);
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
	mph_main(ctx, NULL);
}
