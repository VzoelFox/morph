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
#define MPH_TYPE_mph_SimpleType 2
#define MPH_TYPE_mph_ConsistentChecker 3

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_SimpleType mph_SimpleType;
typedef struct mph_ConsistentChecker mph_ConsistentChecker;
struct mph_SimpleType {
	mph_int kind;
	MorphString* name;
};

struct mph_ConsistentChecker {
	mph_int error_count;
	mph_int warning_count;
	mph_bool has_builtin_len;
	mph_bool has_builtin_print;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_SimpleType = { "SimpleType", sizeof(mph_SimpleType), 1, (size_t[]){offsetof(mph_SimpleType, name)} };
MorphTypeInfo mph_ti_mph_ConsistentChecker = { "ConsistentChecker", sizeof(mph_ConsistentChecker), 0, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;
mph_int mph_TYPE_INT;
mph_int mph_TYPE_FLOAT;
mph_int mph_TYPE_STRING;
mph_int mph_TYPE_BOOL;
mph_int mph_TYPE_VOID;
mph_int mph_TYPE_ARRAY;
mph_int mph_TYPE_ERROR;

// Function Prototypes
mph_ConsistentChecker* mph_init_consistent_checker(MorphContext* ctx, void* _env_void);
mph_SimpleType* mph_make_int_type(MorphContext* ctx, void* _env_void);
mph_SimpleType* mph_make_string_type(MorphContext* ctx, void* _env_void);
mph_SimpleType* mph_make_bool_type(MorphContext* ctx, void* _env_void);
mph_SimpleType* mph_make_error_type(MorphContext* ctx, void* _env_void);
mph_SimpleType* mph_check_binary_op(MorphContext* ctx, void* _env_void, mph_SimpleType* left, MorphString* op, mph_SimpleType* right);
mph_SimpleType* mph_check_unary_op(MorphContext* ctx, void* _env_void, MorphString* op, mph_SimpleType* operand);
mph_SimpleType* mph_check_array_index(MorphContext* ctx, void* _env_void, mph_SimpleType* array_type, mph_SimpleType* index_type);
mph_SimpleType* mph_check_builtin_call(MorphContext* ctx, void* _env_void, mph_ConsistentChecker* checker, MorphString* func_name, mph_SimpleType* arg_type);
mph_bool mph_check_assignment(MorphContext* ctx, void* _env_void, mph_SimpleType* var_type, mph_SimpleType* value_type);
mph_bool mph_check_if_condition(MorphContext* ctx, void* _env_void, mph_SimpleType* condition_type);
mph_bool mph_check_while_condition(MorphContext* ctx, void* _env_void, mph_SimpleType* condition_type);
void mph_test_consistent_checker(MorphContext* ctx, void* _env_void);
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

mph_ConsistentChecker* mph_init_consistent_checker(MorphContext* ctx, void* _env_void) {
	mph_ConsistentChecker* checker = ({ mph_ConsistentChecker* _t = (mph_ConsistentChecker*)mph_alloc(ctx, sizeof(mph_ConsistentChecker), &mph_ti_mph_ConsistentChecker); mph_gc_push_root(ctx, (void**)&_t); _t->has_builtin_print = 1; _t->error_count = 0; _t->warning_count = 0; _t->has_builtin_len = 1; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&checker);
	return ({ mph_ConsistentChecker* _ret_1 = checker; mph_gc_push_root(ctx, (void**)&_ret_1); mph_ConsistentChecker* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
	mph_gc_pop_roots(ctx, 1);
}

mph_SimpleType* mph_make_int_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_SimpleType* _ret_3 = ({ mph_SimpleType* _t = (mph_SimpleType*)mph_alloc(ctx, sizeof(mph_SimpleType), &mph_ti_mph_SimpleType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_TYPE_INT; _t->name = mph_string_new(ctx, "int"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_SimpleType* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}

mph_SimpleType* mph_make_string_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_SimpleType* _ret_5 = ({ mph_SimpleType* _t = (mph_SimpleType*)mph_alloc(ctx, sizeof(mph_SimpleType), &mph_ti_mph_SimpleType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_TYPE_STRING; _t->name = mph_string_new(ctx, "string"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_5); mph_SimpleType* _ret_6 = _ret_5; mph_gc_pop_roots(ctx, 1); _ret_6; });
}

mph_SimpleType* mph_make_bool_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_SimpleType* _ret_7 = ({ mph_SimpleType* _t = (mph_SimpleType*)mph_alloc(ctx, sizeof(mph_SimpleType), &mph_ti_mph_SimpleType); mph_gc_push_root(ctx, (void**)&_t); _t->name = mph_string_new(ctx, "bool"); _t->kind = mph_TYPE_BOOL; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_7); mph_SimpleType* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
}

mph_SimpleType* mph_make_error_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_SimpleType* _ret_9 = ({ mph_SimpleType* _t = (mph_SimpleType*)mph_alloc(ctx, sizeof(mph_SimpleType), &mph_ti_mph_SimpleType); mph_gc_push_root(ctx, (void**)&_t); _t->name = mph_string_new(ctx, "error"); _t->kind = mph_TYPE_ERROR; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_9); mph_SimpleType* _ret_10 = _ret_9; mph_gc_pop_roots(ctx, 1); _ret_10; });
}

mph_SimpleType* mph_check_binary_op(MorphContext* ctx, void* _env_void, mph_SimpleType* left, MorphString* op, mph_SimpleType* right) {
	mph_gc_push_root(ctx, (void**)&left);
	mph_gc_push_root(ctx, (void**)&op);
	mph_gc_push_root(ctx, (void**)&right);
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "+"))) {
{
	if ((({ mph_SimpleType* _obj_11 = left; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->kind; mph_gc_pop_roots(ctx, 1); _ret_12; }) == mph_TYPE_INT)) {
{
	if ((({ mph_SimpleType* _obj_13 = right; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->kind; mph_gc_pop_roots(ctx, 1); _ret_14; }) == mph_TYPE_INT)) {
{
	return ({ mph_SimpleType* _ret_15 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_15); mph_SimpleType* _ret_16 = _ret_15; mph_gc_pop_roots(ctx, 1); _ret_16; });
}
	}
}
	}
	if ((({ mph_SimpleType* _obj_17 = left; mph_gc_push_root(ctx, (void**)&_obj_17); mph_int _ret_18 = _obj_17->kind; mph_gc_pop_roots(ctx, 1); _ret_18; }) == mph_TYPE_STRING)) {
{
	if ((({ mph_SimpleType* _obj_19 = right; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->kind; mph_gc_pop_roots(ctx, 1); _ret_20; }) == mph_TYPE_STRING)) {
{
	return ({ mph_SimpleType* _ret_21 = mph_make_string_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_21); mph_SimpleType* _ret_22 = _ret_21; mph_gc_pop_roots(ctx, 1); _ret_22; });
}
	}
}
	}
	return ({ mph_SimpleType* _ret_23 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_23); mph_SimpleType* _ret_24 = _ret_23; mph_gc_pop_roots(ctx, 1); _ret_24; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "-"))) {
{
	if ((({ mph_SimpleType* _obj_25 = left; mph_gc_push_root(ctx, (void**)&_obj_25); mph_int _ret_26 = _obj_25->kind; mph_gc_pop_roots(ctx, 1); _ret_26; }) == mph_TYPE_INT)) {
{
	if ((({ mph_SimpleType* _obj_27 = right; mph_gc_push_root(ctx, (void**)&_obj_27); mph_int _ret_28 = _obj_27->kind; mph_gc_pop_roots(ctx, 1); _ret_28; }) == mph_TYPE_INT)) {
{
	return ({ mph_SimpleType* _ret_29 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_29); mph_SimpleType* _ret_30 = _ret_29; mph_gc_pop_roots(ctx, 1); _ret_30; });
}
	}
}
	}
	return ({ mph_SimpleType* _ret_31 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_31); mph_SimpleType* _ret_32 = _ret_31; mph_gc_pop_roots(ctx, 1); _ret_32; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "*"))) {
{
	if ((({ mph_SimpleType* _obj_33 = left; mph_gc_push_root(ctx, (void**)&_obj_33); mph_int _ret_34 = _obj_33->kind; mph_gc_pop_roots(ctx, 1); _ret_34; }) == mph_TYPE_INT)) {
{
	if ((({ mph_SimpleType* _obj_35 = right; mph_gc_push_root(ctx, (void**)&_obj_35); mph_int _ret_36 = _obj_35->kind; mph_gc_pop_roots(ctx, 1); _ret_36; }) == mph_TYPE_INT)) {
{
	return ({ mph_SimpleType* _ret_37 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_37); mph_SimpleType* _ret_38 = _ret_37; mph_gc_pop_roots(ctx, 1); _ret_38; });
}
	}
}
	}
	return ({ mph_SimpleType* _ret_39 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_39); mph_SimpleType* _ret_40 = _ret_39; mph_gc_pop_roots(ctx, 1); _ret_40; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "/"))) {
{
	if ((({ mph_SimpleType* _obj_41 = left; mph_gc_push_root(ctx, (void**)&_obj_41); mph_int _ret_42 = _obj_41->kind; mph_gc_pop_roots(ctx, 1); _ret_42; }) == mph_TYPE_INT)) {
{
	if ((({ mph_SimpleType* _obj_43 = right; mph_gc_push_root(ctx, (void**)&_obj_43); mph_int _ret_44 = _obj_43->kind; mph_gc_pop_roots(ctx, 1); _ret_44; }) == mph_TYPE_INT)) {
{
	return ({ mph_SimpleType* _ret_45 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_45); mph_SimpleType* _ret_46 = _ret_45; mph_gc_pop_roots(ctx, 1); _ret_46; });
}
	}
}
	}
	return ({ mph_SimpleType* _ret_47 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_47); mph_SimpleType* _ret_48 = _ret_47; mph_gc_pop_roots(ctx, 1); _ret_48; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "=="))) {
{
	if ((({ mph_SimpleType* _obj_49 = left; mph_gc_push_root(ctx, (void**)&_obj_49); mph_int _ret_50 = _obj_49->kind; mph_gc_pop_roots(ctx, 1); _ret_50; }) == ({ mph_SimpleType* _obj_51 = right; mph_gc_push_root(ctx, (void**)&_obj_51); mph_int _ret_52 = _obj_51->kind; mph_gc_pop_roots(ctx, 1); _ret_52; }))) {
{
	return ({ mph_SimpleType* _ret_53 = mph_make_bool_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_53); mph_SimpleType* _ret_54 = _ret_53; mph_gc_pop_roots(ctx, 1); _ret_54; });
}
	}
	return ({ mph_SimpleType* _ret_55 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_55); mph_SimpleType* _ret_56 = _ret_55; mph_gc_pop_roots(ctx, 1); _ret_56; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "!="))) {
{
	if ((({ mph_SimpleType* _obj_57 = left; mph_gc_push_root(ctx, (void**)&_obj_57); mph_int _ret_58 = _obj_57->kind; mph_gc_pop_roots(ctx, 1); _ret_58; }) == ({ mph_SimpleType* _obj_59 = right; mph_gc_push_root(ctx, (void**)&_obj_59); mph_int _ret_60 = _obj_59->kind; mph_gc_pop_roots(ctx, 1); _ret_60; }))) {
{
	return ({ mph_SimpleType* _ret_61 = mph_make_bool_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_61); mph_SimpleType* _ret_62 = _ret_61; mph_gc_pop_roots(ctx, 1); _ret_62; });
}
	}
	return ({ mph_SimpleType* _ret_63 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_63); mph_SimpleType* _ret_64 = _ret_63; mph_gc_pop_roots(ctx, 1); _ret_64; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "<"))) {
{
	if ((({ mph_SimpleType* _obj_65 = left; mph_gc_push_root(ctx, (void**)&_obj_65); mph_int _ret_66 = _obj_65->kind; mph_gc_pop_roots(ctx, 1); _ret_66; }) == ({ mph_SimpleType* _obj_67 = right; mph_gc_push_root(ctx, (void**)&_obj_67); mph_int _ret_68 = _obj_67->kind; mph_gc_pop_roots(ctx, 1); _ret_68; }))) {
{
	return ({ mph_SimpleType* _ret_69 = mph_make_bool_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_69); mph_SimpleType* _ret_70 = _ret_69; mph_gc_pop_roots(ctx, 1); _ret_70; });
}
	}
	return ({ mph_SimpleType* _ret_71 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_71); mph_SimpleType* _ret_72 = _ret_71; mph_gc_pop_roots(ctx, 1); _ret_72; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, ">"))) {
{
	if ((({ mph_SimpleType* _obj_73 = left; mph_gc_push_root(ctx, (void**)&_obj_73); mph_int _ret_74 = _obj_73->kind; mph_gc_pop_roots(ctx, 1); _ret_74; }) == ({ mph_SimpleType* _obj_75 = right; mph_gc_push_root(ctx, (void**)&_obj_75); mph_int _ret_76 = _obj_75->kind; mph_gc_pop_roots(ctx, 1); _ret_76; }))) {
{
	return ({ mph_SimpleType* _ret_77 = mph_make_bool_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_77); mph_SimpleType* _ret_78 = _ret_77; mph_gc_pop_roots(ctx, 1); _ret_78; });
}
	}
	return ({ mph_SimpleType* _ret_79 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_79); mph_SimpleType* _ret_80 = _ret_79; mph_gc_pop_roots(ctx, 1); _ret_80; });
}
	}
	return ({ mph_SimpleType* _ret_81 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_81); mph_SimpleType* _ret_82 = _ret_81; mph_gc_pop_roots(ctx, 1); _ret_82; });
	mph_gc_pop_roots(ctx, 3);
}

mph_SimpleType* mph_check_unary_op(MorphContext* ctx, void* _env_void, MorphString* op, mph_SimpleType* operand) {
	mph_gc_push_root(ctx, (void**)&op);
	mph_gc_push_root(ctx, (void**)&operand);
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "-"))) {
{
	if ((({ mph_SimpleType* _obj_83 = operand; mph_gc_push_root(ctx, (void**)&_obj_83); mph_int _ret_84 = _obj_83->kind; mph_gc_pop_roots(ctx, 1); _ret_84; }) == mph_TYPE_INT)) {
{
	return ({ mph_SimpleType* _ret_85 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_85); mph_SimpleType* _ret_86 = _ret_85; mph_gc_pop_roots(ctx, 1); _ret_86; });
}
	}
	return ({ mph_SimpleType* _ret_87 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_87); mph_SimpleType* _ret_88 = _ret_87; mph_gc_pop_roots(ctx, 1); _ret_88; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "!"))) {
{
	if ((({ mph_SimpleType* _obj_89 = operand; mph_gc_push_root(ctx, (void**)&_obj_89); mph_int _ret_90 = _obj_89->kind; mph_gc_pop_roots(ctx, 1); _ret_90; }) == mph_TYPE_BOOL)) {
{
	return ({ mph_SimpleType* _ret_91 = mph_make_bool_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_91); mph_SimpleType* _ret_92 = _ret_91; mph_gc_pop_roots(ctx, 1); _ret_92; });
}
	}
	return ({ mph_SimpleType* _ret_93 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_93); mph_SimpleType* _ret_94 = _ret_93; mph_gc_pop_roots(ctx, 1); _ret_94; });
}
	}
	return ({ mph_SimpleType* _ret_95 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_95); mph_SimpleType* _ret_96 = _ret_95; mph_gc_pop_roots(ctx, 1); _ret_96; });
	mph_gc_pop_roots(ctx, 2);
}

mph_SimpleType* mph_check_array_index(MorphContext* ctx, void* _env_void, mph_SimpleType* array_type, mph_SimpleType* index_type) {
	mph_gc_push_root(ctx, (void**)&array_type);
	mph_gc_push_root(ctx, (void**)&index_type);
	if ((({ mph_SimpleType* _obj_97 = index_type; mph_gc_push_root(ctx, (void**)&_obj_97); mph_int _ret_98 = _obj_97->kind; mph_gc_pop_roots(ctx, 1); _ret_98; }) != mph_TYPE_INT)) {
{
	return ({ mph_SimpleType* _ret_99 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_99); mph_SimpleType* _ret_100 = _ret_99; mph_gc_pop_roots(ctx, 1); _ret_100; });
}
	}
	if ((({ mph_SimpleType* _obj_101 = array_type; mph_gc_push_root(ctx, (void**)&_obj_101); mph_int _ret_102 = _obj_101->kind; mph_gc_pop_roots(ctx, 1); _ret_102; }) == mph_TYPE_ARRAY)) {
{
	return ({ mph_SimpleType* _ret_103 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_103); mph_SimpleType* _ret_104 = _ret_103; mph_gc_pop_roots(ctx, 1); _ret_104; });
}
	}
	if ((({ mph_SimpleType* _obj_105 = array_type; mph_gc_push_root(ctx, (void**)&_obj_105); mph_int _ret_106 = _obj_105->kind; mph_gc_pop_roots(ctx, 1); _ret_106; }) == mph_TYPE_STRING)) {
{
	return ({ mph_SimpleType* _ret_107 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_107); mph_SimpleType* _ret_108 = _ret_107; mph_gc_pop_roots(ctx, 1); _ret_108; });
}
	}
	return ({ mph_SimpleType* _ret_109 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_109); mph_SimpleType* _ret_110 = _ret_109; mph_gc_pop_roots(ctx, 1); _ret_110; });
	mph_gc_pop_roots(ctx, 2);
}

mph_SimpleType* mph_check_builtin_call(MorphContext* ctx, void* _env_void, mph_ConsistentChecker* checker, MorphString* func_name, mph_SimpleType* arg_type) {
	mph_gc_push_root(ctx, (void**)&checker);
	mph_gc_push_root(ctx, (void**)&func_name);
	mph_gc_push_root(ctx, (void**)&arg_type);
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "len"))) {
{
	if ((({ mph_SimpleType* _obj_111 = arg_type; mph_gc_push_root(ctx, (void**)&_obj_111); mph_int _ret_112 = _obj_111->kind; mph_gc_pop_roots(ctx, 1); _ret_112; }) == mph_TYPE_STRING)) {
{
	return ({ mph_SimpleType* _ret_113 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_113); mph_SimpleType* _ret_114 = _ret_113; mph_gc_pop_roots(ctx, 1); _ret_114; });
}
	}
	if ((({ mph_SimpleType* _obj_115 = arg_type; mph_gc_push_root(ctx, (void**)&_obj_115); mph_int _ret_116 = _obj_115->kind; mph_gc_pop_roots(ctx, 1); _ret_116; }) == mph_TYPE_ARRAY)) {
{
	return ({ mph_SimpleType* _ret_117 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_117); mph_SimpleType* _ret_118 = _ret_117; mph_gc_pop_roots(ctx, 1); _ret_118; });
}
	}
	return ({ mph_SimpleType* _ret_119 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_119); mph_SimpleType* _ret_120 = _ret_119; mph_gc_pop_roots(ctx, 1); _ret_120; });
}
	}
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "print"))) {
{
	return ({ mph_SimpleType* _ret_121 = ({ mph_SimpleType* _t = (mph_SimpleType*)mph_alloc(ctx, sizeof(mph_SimpleType), &mph_ti_mph_SimpleType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_TYPE_VOID; _t->name = mph_string_new(ctx, "void"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_121); mph_SimpleType* _ret_122 = _ret_121; mph_gc_pop_roots(ctx, 1); _ret_122; });
}
	}
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "native_print"))) {
{
	return ({ mph_SimpleType* _ret_123 = ({ mph_SimpleType* _t = (mph_SimpleType*)mph_alloc(ctx, sizeof(mph_SimpleType), &mph_ti_mph_SimpleType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_TYPE_VOID; _t->name = mph_string_new(ctx, "void"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_123); mph_SimpleType* _ret_124 = _ret_123; mph_gc_pop_roots(ctx, 1); _ret_124; });
}
	}
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "parse_int"))) {
{
	if ((({ mph_SimpleType* _obj_125 = arg_type; mph_gc_push_root(ctx, (void**)&_obj_125); mph_int _ret_126 = _obj_125->kind; mph_gc_pop_roots(ctx, 1); _ret_126; }) == mph_TYPE_STRING)) {
{
	return ({ mph_SimpleType* _ret_127 = mph_make_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_127); mph_SimpleType* _ret_128 = _ret_127; mph_gc_pop_roots(ctx, 1); _ret_128; });
}
	}
	return ({ mph_SimpleType* _ret_129 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_129); mph_SimpleType* _ret_130 = _ret_129; mph_gc_pop_roots(ctx, 1); _ret_130; });
}
	}
	return ({ mph_SimpleType* _ret_131 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_131); mph_SimpleType* _ret_132 = _ret_131; mph_gc_pop_roots(ctx, 1); _ret_132; });
	mph_gc_pop_roots(ctx, 3);
}

mph_bool mph_check_assignment(MorphContext* ctx, void* _env_void, mph_SimpleType* var_type, mph_SimpleType* value_type) {
	mph_gc_push_root(ctx, (void**)&var_type);
	mph_gc_push_root(ctx, (void**)&value_type);
	return (({ mph_SimpleType* _obj_133 = var_type; mph_gc_push_root(ctx, (void**)&_obj_133); mph_int _ret_134 = _obj_133->kind; mph_gc_pop_roots(ctx, 1); _ret_134; }) == ({ mph_SimpleType* _obj_135 = value_type; mph_gc_push_root(ctx, (void**)&_obj_135); mph_int _ret_136 = _obj_135->kind; mph_gc_pop_roots(ctx, 1); _ret_136; }));
	mph_gc_pop_roots(ctx, 2);
}

mph_bool mph_check_if_condition(MorphContext* ctx, void* _env_void, mph_SimpleType* condition_type) {
	mph_gc_push_root(ctx, (void**)&condition_type);
	return (({ mph_SimpleType* _obj_137 = condition_type; mph_gc_push_root(ctx, (void**)&_obj_137); mph_int _ret_138 = _obj_137->kind; mph_gc_pop_roots(ctx, 1); _ret_138; }) == mph_TYPE_BOOL);
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_check_while_condition(MorphContext* ctx, void* _env_void, mph_SimpleType* condition_type) {
	mph_gc_push_root(ctx, (void**)&condition_type);
	return (({ mph_SimpleType* _obj_139 = condition_type; mph_gc_push_root(ctx, (void**)&_obj_139); mph_int _ret_140 = _obj_139->kind; mph_gc_pop_roots(ctx, 1); _ret_140; }) == mph_TYPE_BOOL);
	mph_gc_pop_roots(ctx, 1);
}

void mph_test_consistent_checker(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_141 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_141); MorphString* _arg_142 = mph_string_new(ctx, "=== CONSISTENT TYPE CHECKER TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_142); mph_int _ret_143 = mph_io_Write(ctx, NULL, _arg_141, _arg_142); mph_gc_pop_roots(ctx, 2); _ret_143; });
	mph_ConsistentChecker* checker = mph_init_consistent_checker(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&checker);
	mph_SimpleType* int_type = mph_make_int_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&int_type);
	mph_SimpleType* string_type = mph_make_string_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&string_type);
	mph_SimpleType* bool_type = mph_make_bool_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&bool_type);
	mph_SimpleType* add_result = ({ mph_SimpleType* _arg_144 = int_type; mph_gc_push_root(ctx, (void**)&_arg_144); MorphString* _arg_145 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_145); mph_SimpleType* _arg_146 = int_type; mph_gc_push_root(ctx, (void**)&_arg_146); mph_SimpleType* _ret_147 = mph_check_binary_op(ctx, NULL, _arg_144, _arg_145, _arg_146); mph_gc_pop_roots(ctx, 3); _ret_147; });
	mph_gc_push_root(ctx, (void**)&add_result);
	if ((({ mph_SimpleType* _obj_148 = add_result; mph_gc_push_root(ctx, (void**)&_obj_148); mph_int _ret_149 = _obj_148->kind; mph_gc_pop_roots(ctx, 1); _ret_149; }) == mph_TYPE_INT)) {
{
	({ mph_io_File* _arg_150 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_150); MorphString* _arg_151 = mph_string_new(ctx, "int + int â int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_151); mph_int _ret_152 = mph_io_Write(ctx, NULL, _arg_150, _arg_151); mph_gc_pop_roots(ctx, 2); _ret_152; });
}
	} else {
{
	({ mph_io_File* _arg_153 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_153); MorphString* _arg_154 = mph_string_new(ctx, "int + int: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_154); mph_int _ret_155 = mph_io_Write(ctx, NULL, _arg_153, _arg_154); mph_gc_pop_roots(ctx, 2); _ret_155; });
}
	}
	mph_SimpleType* concat_result = ({ mph_SimpleType* _arg_156 = string_type; mph_gc_push_root(ctx, (void**)&_arg_156); MorphString* _arg_157 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_157); mph_SimpleType* _arg_158 = string_type; mph_gc_push_root(ctx, (void**)&_arg_158); mph_SimpleType* _ret_159 = mph_check_binary_op(ctx, NULL, _arg_156, _arg_157, _arg_158); mph_gc_pop_roots(ctx, 3); _ret_159; });
	mph_gc_push_root(ctx, (void**)&concat_result);
	if ((({ mph_SimpleType* _obj_160 = concat_result; mph_gc_push_root(ctx, (void**)&_obj_160); mph_int _ret_161 = _obj_160->kind; mph_gc_pop_roots(ctx, 1); _ret_161; }) == mph_TYPE_STRING)) {
{
	({ mph_io_File* _arg_162 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_162); MorphString* _arg_163 = mph_string_new(ctx, "string + string â string: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_163); mph_int _ret_164 = mph_io_Write(ctx, NULL, _arg_162, _arg_163); mph_gc_pop_roots(ctx, 2); _ret_164; });
}
	} else {
{
	({ mph_io_File* _arg_165 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_165); MorphString* _arg_166 = mph_string_new(ctx, "string + string: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_166); mph_int _ret_167 = mph_io_Write(ctx, NULL, _arg_165, _arg_166); mph_gc_pop_roots(ctx, 2); _ret_167; });
}
	}
	mph_SimpleType* compare_result = ({ mph_SimpleType* _arg_168 = int_type; mph_gc_push_root(ctx, (void**)&_arg_168); MorphString* _arg_169 = mph_string_new(ctx, "=="); mph_gc_push_root(ctx, (void**)&_arg_169); mph_SimpleType* _arg_170 = int_type; mph_gc_push_root(ctx, (void**)&_arg_170); mph_SimpleType* _ret_171 = mph_check_binary_op(ctx, NULL, _arg_168, _arg_169, _arg_170); mph_gc_pop_roots(ctx, 3); _ret_171; });
	mph_gc_push_root(ctx, (void**)&compare_result);
	if ((({ mph_SimpleType* _obj_172 = compare_result; mph_gc_push_root(ctx, (void**)&_obj_172); mph_int _ret_173 = _obj_172->kind; mph_gc_pop_roots(ctx, 1); _ret_173; }) == mph_TYPE_BOOL)) {
{
	({ mph_io_File* _arg_174 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_174); MorphString* _arg_175 = mph_string_new(ctx, "int == int â bool: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_175); mph_int _ret_176 = mph_io_Write(ctx, NULL, _arg_174, _arg_175); mph_gc_pop_roots(ctx, 2); _ret_176; });
}
	} else {
{
	({ mph_io_File* _arg_177 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_177); MorphString* _arg_178 = mph_string_new(ctx, "int == int: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_178); mph_int _ret_179 = mph_io_Write(ctx, NULL, _arg_177, _arg_178); mph_gc_pop_roots(ctx, 2); _ret_179; });
}
	}
	mph_SimpleType* mismatch_result = ({ mph_SimpleType* _arg_180 = int_type; mph_gc_push_root(ctx, (void**)&_arg_180); MorphString* _arg_181 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_181); mph_SimpleType* _arg_182 = string_type; mph_gc_push_root(ctx, (void**)&_arg_182); mph_SimpleType* _ret_183 = mph_check_binary_op(ctx, NULL, _arg_180, _arg_181, _arg_182); mph_gc_pop_roots(ctx, 3); _ret_183; });
	mph_gc_push_root(ctx, (void**)&mismatch_result);
	if ((({ mph_SimpleType* _obj_184 = mismatch_result; mph_gc_push_root(ctx, (void**)&_obj_184); mph_int _ret_185 = _obj_184->kind; mph_gc_pop_roots(ctx, 1); _ret_185; }) == mph_TYPE_ERROR)) {
{
	({ mph_io_File* _arg_186 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_186); MorphString* _arg_187 = mph_string_new(ctx, "int + string â error: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_187); mph_int _ret_188 = mph_io_Write(ctx, NULL, _arg_186, _arg_187); mph_gc_pop_roots(ctx, 2); _ret_188; });
}
	} else {
{
	({ mph_io_File* _arg_189 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_189); MorphString* _arg_190 = mph_string_new(ctx, "int + string: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_190); mph_int _ret_191 = mph_io_Write(ctx, NULL, _arg_189, _arg_190); mph_gc_pop_roots(ctx, 2); _ret_191; });
}
	}
	mph_SimpleType* neg_result = ({ MorphString* _arg_192 = mph_string_new(ctx, "-"); mph_gc_push_root(ctx, (void**)&_arg_192); mph_SimpleType* _arg_193 = int_type; mph_gc_push_root(ctx, (void**)&_arg_193); mph_SimpleType* _ret_194 = mph_check_unary_op(ctx, NULL, _arg_192, _arg_193); mph_gc_pop_roots(ctx, 2); _ret_194; });
	mph_gc_push_root(ctx, (void**)&neg_result);
	if ((({ mph_SimpleType* _obj_195 = neg_result; mph_gc_push_root(ctx, (void**)&_obj_195); mph_int _ret_196 = _obj_195->kind; mph_gc_pop_roots(ctx, 1); _ret_196; }) == mph_TYPE_INT)) {
{
	({ mph_io_File* _arg_197 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_197); MorphString* _arg_198 = mph_string_new(ctx, "-int â int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_198); mph_int _ret_199 = mph_io_Write(ctx, NULL, _arg_197, _arg_198); mph_gc_pop_roots(ctx, 2); _ret_199; });
}
	} else {
{
	({ mph_io_File* _arg_200 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_200); MorphString* _arg_201 = mph_string_new(ctx, "-int: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_201); mph_int _ret_202 = mph_io_Write(ctx, NULL, _arg_200, _arg_201); mph_gc_pop_roots(ctx, 2); _ret_202; });
}
	}
	mph_SimpleType* not_result = ({ MorphString* _arg_203 = mph_string_new(ctx, "!"); mph_gc_push_root(ctx, (void**)&_arg_203); mph_SimpleType* _arg_204 = bool_type; mph_gc_push_root(ctx, (void**)&_arg_204); mph_SimpleType* _ret_205 = mph_check_unary_op(ctx, NULL, _arg_203, _arg_204); mph_gc_pop_roots(ctx, 2); _ret_205; });
	mph_gc_push_root(ctx, (void**)&not_result);
	if ((({ mph_SimpleType* _obj_206 = not_result; mph_gc_push_root(ctx, (void**)&_obj_206); mph_int _ret_207 = _obj_206->kind; mph_gc_pop_roots(ctx, 1); _ret_207; }) == mph_TYPE_BOOL)) {
{
	({ mph_io_File* _arg_208 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_208); MorphString* _arg_209 = mph_string_new(ctx, "!bool â bool: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_209); mph_int _ret_210 = mph_io_Write(ctx, NULL, _arg_208, _arg_209); mph_gc_pop_roots(ctx, 2); _ret_210; });
}
	} else {
{
	({ mph_io_File* _arg_211 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_211); MorphString* _arg_212 = mph_string_new(ctx, "!bool: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_212); mph_int _ret_213 = mph_io_Write(ctx, NULL, _arg_211, _arg_212); mph_gc_pop_roots(ctx, 2); _ret_213; });
}
	}
	mph_SimpleType* len_result = ({ mph_ConsistentChecker* _arg_214 = checker; mph_gc_push_root(ctx, (void**)&_arg_214); MorphString* _arg_215 = mph_string_new(ctx, "len"); mph_gc_push_root(ctx, (void**)&_arg_215); mph_SimpleType* _arg_216 = string_type; mph_gc_push_root(ctx, (void**)&_arg_216); mph_SimpleType* _ret_217 = mph_check_builtin_call(ctx, NULL, _arg_214, _arg_215, _arg_216); mph_gc_pop_roots(ctx, 3); _ret_217; });
	mph_gc_push_root(ctx, (void**)&len_result);
	if ((({ mph_SimpleType* _obj_218 = len_result; mph_gc_push_root(ctx, (void**)&_obj_218); mph_int _ret_219 = _obj_218->kind; mph_gc_pop_roots(ctx, 1); _ret_219; }) == mph_TYPE_INT)) {
{
	({ mph_io_File* _arg_220 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_220); MorphString* _arg_221 = mph_string_new(ctx, "len(string) â int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_221); mph_int _ret_222 = mph_io_Write(ctx, NULL, _arg_220, _arg_221); mph_gc_pop_roots(ctx, 2); _ret_222; });
}
	} else {
{
	({ mph_io_File* _arg_223 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_223); MorphString* _arg_224 = mph_string_new(ctx, "len(string): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_224); mph_int _ret_225 = mph_io_Write(ctx, NULL, _arg_223, _arg_224); mph_gc_pop_roots(ctx, 2); _ret_225; });
}
	}
	mph_bool assign_ok = ({ mph_SimpleType* _arg_226 = int_type; mph_gc_push_root(ctx, (void**)&_arg_226); mph_SimpleType* _arg_227 = int_type; mph_gc_push_root(ctx, (void**)&_arg_227); mph_bool _ret_228 = mph_check_assignment(ctx, NULL, _arg_226, _arg_227); mph_gc_pop_roots(ctx, 2); _ret_228; });
	if (assign_ok) {
{
	({ mph_io_File* _arg_229 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_229); MorphString* _arg_230 = mph_string_new(ctx, "int = int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_230); mph_int _ret_231 = mph_io_Write(ctx, NULL, _arg_229, _arg_230); mph_gc_pop_roots(ctx, 2); _ret_231; });
}
	} else {
{
	({ mph_io_File* _arg_232 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_232); MorphString* _arg_233 = mph_string_new(ctx, "int = int: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_233); mph_int _ret_234 = mph_io_Write(ctx, NULL, _arg_232, _arg_233); mph_gc_pop_roots(ctx, 2); _ret_234; });
}
	}
	mph_bool assign_bad = ({ mph_SimpleType* _arg_235 = int_type; mph_gc_push_root(ctx, (void**)&_arg_235); mph_SimpleType* _arg_236 = string_type; mph_gc_push_root(ctx, (void**)&_arg_236); mph_bool _ret_237 = mph_check_assignment(ctx, NULL, _arg_235, _arg_236); mph_gc_pop_roots(ctx, 2); _ret_237; });
	if ((!assign_bad)) {
{
	({ mph_io_File* _arg_238 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_238); MorphString* _arg_239 = mph_string_new(ctx, "int = string (rejected): â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_239); mph_int _ret_240 = mph_io_Write(ctx, NULL, _arg_238, _arg_239); mph_gc_pop_roots(ctx, 2); _ret_240; });
}
	} else {
{
	({ mph_io_File* _arg_241 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_241); MorphString* _arg_242 = mph_string_new(ctx, "int = string: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_242); mph_int _ret_243 = mph_io_Write(ctx, NULL, _arg_241, _arg_242); mph_gc_pop_roots(ctx, 2); _ret_243; });
}
	}
	mph_bool if_ok = ({ mph_SimpleType* _arg_244 = bool_type; mph_gc_push_root(ctx, (void**)&_arg_244); mph_bool _ret_245 = mph_check_if_condition(ctx, NULL, _arg_244); mph_gc_pop_roots(ctx, 1); _ret_245; });
	if (if_ok) {
{
	({ mph_io_File* _arg_246 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_246); MorphString* _arg_247 = mph_string_new(ctx, "if(bool): â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_247); mph_int _ret_248 = mph_io_Write(ctx, NULL, _arg_246, _arg_247); mph_gc_pop_roots(ctx, 2); _ret_248; });
}
	} else {
{
	({ mph_io_File* _arg_249 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_249); MorphString* _arg_250 = mph_string_new(ctx, "if(bool): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_250); mph_int _ret_251 = mph_io_Write(ctx, NULL, _arg_249, _arg_250); mph_gc_pop_roots(ctx, 2); _ret_251; });
}
	}
	({ mph_io_File* _arg_252 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_252); MorphString* _arg_253 = mph_string_new(ctx, "\nð CONSISTENT CHECKER COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_253); mph_int _ret_254 = mph_io_Write(ctx, NULL, _arg_252, _arg_253); mph_gc_pop_roots(ctx, 2); _ret_254; });
	({ mph_io_File* _arg_255 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_255); MorphString* _arg_256 = mph_string_new(ctx, "â Go-consistent type rules\n"); mph_gc_push_root(ctx, (void**)&_arg_256); mph_int _ret_257 = mph_io_Write(ctx, NULL, _arg_255, _arg_256); mph_gc_pop_roots(ctx, 2); _ret_257; });
	({ mph_io_File* _arg_258 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_258); MorphString* _arg_259 = mph_string_new(ctx, "â Strict type checking\n"); mph_gc_push_root(ctx, (void**)&_arg_259); mph_int _ret_260 = mph_io_Write(ctx, NULL, _arg_258, _arg_259); mph_gc_pop_roots(ctx, 2); _ret_260; });
	({ mph_io_File* _arg_261 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_261); MorphString* _arg_262 = mph_string_new(ctx, "â Built-in function validation\n"); mph_gc_push_root(ctx, (void**)&_arg_262); mph_int _ret_263 = mph_io_Write(ctx, NULL, _arg_261, _arg_262); mph_gc_pop_roots(ctx, 2); _ret_263; });
	({ mph_io_File* _arg_264 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_264); MorphString* _arg_265 = mph_string_new(ctx, "â Assignment compatibility\n"); mph_gc_push_root(ctx, (void**)&_arg_265); mph_int _ret_266 = mph_io_Write(ctx, NULL, _arg_264, _arg_265); mph_gc_pop_roots(ctx, 2); _ret_266; });
	({ mph_io_File* _arg_267 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_267); MorphString* _arg_268 = mph_string_new(ctx, "â Control flow validation\n"); mph_gc_push_root(ctx, (void**)&_arg_268); mph_int _ret_269 = mph_io_Write(ctx, NULL, _arg_267, _arg_268); mph_gc_pop_roots(ctx, 2); _ret_269; });
	mph_gc_pop_roots(ctx, 11);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_test_consistent_checker(ctx, NULL);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_TYPE_INT = 0;
	mph_TYPE_FLOAT = 1;
	mph_TYPE_STRING = 2;
	mph_TYPE_BOOL = 3;
	mph_TYPE_VOID = 4;
	mph_TYPE_ARRAY = 5;
	mph_TYPE_ERROR = 8;
	mph_main(ctx, NULL);
}
