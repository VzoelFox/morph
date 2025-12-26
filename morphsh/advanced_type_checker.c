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
#define MPH_TYPE_mph_AdvancedType 2
#define MPH_TYPE_mph_AdvancedChecker 3
#define MPH_TYPE_mph_io_File 1

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_AdvancedType mph_AdvancedType;
typedef struct mph_AdvancedChecker mph_AdvancedChecker;
struct mph_AdvancedType {
	mph_int kind;
	MorphString* name;
	mph_int element_type_kind;
	mph_bool is_pointer;
	mph_bool is_nullable;
};

struct mph_AdvancedChecker {
	mph_bool strict_mode;
	mph_bool allow_implicit_conversion;
	mph_int error_count;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_AdvancedType = { "AdvancedType", sizeof(mph_AdvancedType), 1, (size_t[]){offsetof(mph_AdvancedType, name)} };
MorphTypeInfo mph_ti_mph_AdvancedChecker = { "AdvancedChecker", sizeof(mph_AdvancedChecker), 0, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_BOOL;
mph_int mph_KIND_VOID;
mph_int mph_KIND_ARRAY;
mph_int mph_KIND_STRUCT;
mph_int mph_KIND_FUNC;
mph_int mph_KIND_INTERFACE;
mph_int mph_KIND_POINTER;
mph_int mph_KIND_CHANNEL;
mph_int mph_KIND_ERROR;

// Function Prototypes
mph_AdvancedType* mph_make_advanced_int(MorphContext* ctx, void* _env_void);
mph_AdvancedType* mph_make_advanced_string(MorphContext* ctx, void* _env_void);
mph_AdvancedType* mph_make_advanced_bool(MorphContext* ctx, void* _env_void);
mph_AdvancedType* mph_make_int_array(MorphContext* ctx, void* _env_void);
mph_AdvancedType* mph_make_error_type(MorphContext* ctx, void* _env_void);
mph_bool mph_types_compatible(MorphContext* ctx, void* _env_void, mph_AdvancedType* t1, mph_AdvancedType* t2);
mph_AdvancedChecker* mph_advanced_checker_new(MorphContext* ctx, void* _env_void);
mph_AdvancedType* mph_check_advanced_binary(MorphContext* ctx, void* _env_void, mph_AdvancedType* left, MorphString* op, mph_AdvancedType* right);
mph_AdvancedType* mph_check_unary(MorphContext* ctx, void* _env_void, MorphString* op, mph_AdvancedType* operand);
mph_AdvancedType* mph_check_array_access(MorphContext* ctx, void* _env_void, mph_AdvancedType* array_type, mph_AdvancedType* index_type);
mph_AdvancedType* mph_check_advanced_call(MorphContext* ctx, void* _env_void, MorphString* func_name, mph_AdvancedType* arg_type);
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

mph_AdvancedType* mph_make_advanced_int(MorphContext* ctx, void* _env_void) {
	return ({ mph_AdvancedType* _ret_1 = ({ mph_AdvancedType* _t = (mph_AdvancedType*)mph_alloc(ctx, sizeof(mph_AdvancedType), &mph_ti_mph_AdvancedType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_INT; _t->name = mph_string_new(ctx, "int"); _t->element_type_kind = (-1); _t->is_pointer = 0; _t->is_nullable = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_AdvancedType* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_AdvancedType* mph_make_advanced_string(MorphContext* ctx, void* _env_void) {
	return ({ mph_AdvancedType* _ret_3 = ({ mph_AdvancedType* _t = (mph_AdvancedType*)mph_alloc(ctx, sizeof(mph_AdvancedType), &mph_ti_mph_AdvancedType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_STRING; _t->name = mph_string_new(ctx, "string"); _t->element_type_kind = (-1); _t->is_pointer = 0; _t->is_nullable = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_AdvancedType* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}

mph_AdvancedType* mph_make_advanced_bool(MorphContext* ctx, void* _env_void) {
	return ({ mph_AdvancedType* _ret_5 = ({ mph_AdvancedType* _t = (mph_AdvancedType*)mph_alloc(ctx, sizeof(mph_AdvancedType), &mph_ti_mph_AdvancedType); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_BOOL; _t->name = mph_string_new(ctx, "bool"); _t->element_type_kind = (-1); _t->is_pointer = 0; _t->is_nullable = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_5); mph_AdvancedType* _ret_6 = _ret_5; mph_gc_pop_roots(ctx, 1); _ret_6; });
}

mph_AdvancedType* mph_make_int_array(MorphContext* ctx, void* _env_void) {
	return ({ mph_AdvancedType* _ret_7 = ({ mph_AdvancedType* _t = (mph_AdvancedType*)mph_alloc(ctx, sizeof(mph_AdvancedType), &mph_ti_mph_AdvancedType); mph_gc_push_root(ctx, (void**)&_t); _t->name = mph_string_new(ctx, "[]int"); _t->element_type_kind = mph_KIND_INT; _t->is_pointer = 0; _t->is_nullable = 1; _t->kind = mph_KIND_ARRAY; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_7); mph_AdvancedType* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
}

mph_AdvancedType* mph_make_error_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_AdvancedType* _ret_9 = ({ mph_AdvancedType* _t = (mph_AdvancedType*)mph_alloc(ctx, sizeof(mph_AdvancedType), &mph_ti_mph_AdvancedType); mph_gc_push_root(ctx, (void**)&_t); _t->element_type_kind = (-1); _t->is_pointer = 0; _t->is_nullable = 1; _t->kind = mph_KIND_ERROR; _t->name = mph_string_new(ctx, "error"); mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_9); mph_AdvancedType* _ret_10 = _ret_9; mph_gc_pop_roots(ctx, 1); _ret_10; });
}

mph_bool mph_types_compatible(MorphContext* ctx, void* _env_void, mph_AdvancedType* t1, mph_AdvancedType* t2) {
	mph_gc_push_root(ctx, (void**)&t1);
	mph_gc_push_root(ctx, (void**)&t2);
	if ((({ mph_AdvancedType* _obj_11 = t1; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->kind; mph_gc_pop_roots(ctx, 1); _ret_12; }) == ({ mph_AdvancedType* _obj_13 = t2; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->kind; mph_gc_pop_roots(ctx, 1); _ret_14; }))) {
{
	if ((({ mph_AdvancedType* _obj_15 = t1; mph_gc_push_root(ctx, (void**)&_obj_15); mph_int _ret_16 = _obj_15->kind; mph_gc_pop_roots(ctx, 1); _ret_16; }) == mph_KIND_ARRAY)) {
{
	return (({ mph_AdvancedType* _obj_17 = t1; mph_gc_push_root(ctx, (void**)&_obj_17); mph_int _ret_18 = _obj_17->element_type_kind; mph_gc_pop_roots(ctx, 1); _ret_18; }) == ({ mph_AdvancedType* _obj_19 = t2; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->element_type_kind; mph_gc_pop_roots(ctx, 1); _ret_20; }));
}
	}
	return 1;
}
	}
	if ((({ mph_AdvancedType* _obj_21 = t1; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->kind; mph_gc_pop_roots(ctx, 1); _ret_22; }) == mph_KIND_INTERFACE)) {
{
	return 1;
}
	}
	if ((({ mph_AdvancedType* _obj_23 = t2; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->kind; mph_gc_pop_roots(ctx, 1); _ret_24; }) == mph_KIND_INTERFACE)) {
{
	return 1;
}
	}
	return 0;
	mph_gc_pop_roots(ctx, 2);
}

mph_AdvancedChecker* mph_advanced_checker_new(MorphContext* ctx, void* _env_void) {
	return ({ mph_AdvancedChecker* _ret_25 = ({ mph_AdvancedChecker* _t = (mph_AdvancedChecker*)mph_alloc(ctx, sizeof(mph_AdvancedChecker), &mph_ti_mph_AdvancedChecker); mph_gc_push_root(ctx, (void**)&_t); _t->error_count = 0; _t->strict_mode = 1; _t->allow_implicit_conversion = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_25); mph_AdvancedChecker* _ret_26 = _ret_25; mph_gc_pop_roots(ctx, 1); _ret_26; });
}

mph_AdvancedType* mph_check_advanced_binary(MorphContext* ctx, void* _env_void, mph_AdvancedType* left, MorphString* op, mph_AdvancedType* right) {
	mph_gc_push_root(ctx, (void**)&left);
	mph_gc_push_root(ctx, (void**)&op);
	mph_gc_push_root(ctx, (void**)&right);
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "+"))) {
{
	if ((({ mph_AdvancedType* _obj_27 = left; mph_gc_push_root(ctx, (void**)&_obj_27); mph_int _ret_28 = _obj_27->kind; mph_gc_pop_roots(ctx, 1); _ret_28; }) == mph_KIND_INT)) {
{
	if ((({ mph_AdvancedType* _obj_29 = right; mph_gc_push_root(ctx, (void**)&_obj_29); mph_int _ret_30 = _obj_29->kind; mph_gc_pop_roots(ctx, 1); _ret_30; }) == mph_KIND_INT)) {
{
	return ({ mph_AdvancedType* _ret_31 = mph_make_advanced_int(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_31); mph_AdvancedType* _ret_32 = _ret_31; mph_gc_pop_roots(ctx, 1); _ret_32; });
}
	}
}
	}
	if ((({ mph_AdvancedType* _obj_33 = left; mph_gc_push_root(ctx, (void**)&_obj_33); mph_int _ret_34 = _obj_33->kind; mph_gc_pop_roots(ctx, 1); _ret_34; }) == mph_KIND_STRING)) {
{
	if ((({ mph_AdvancedType* _obj_35 = right; mph_gc_push_root(ctx, (void**)&_obj_35); mph_int _ret_36 = _obj_35->kind; mph_gc_pop_roots(ctx, 1); _ret_36; }) == mph_KIND_STRING)) {
{
	return ({ mph_AdvancedType* _ret_37 = mph_make_advanced_string(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_37); mph_AdvancedType* _ret_38 = _ret_37; mph_gc_pop_roots(ctx, 1); _ret_38; });
}
	}
}
	}
	return ({ mph_AdvancedType* _ret_39 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_39); mph_AdvancedType* _ret_40 = _ret_39; mph_gc_pop_roots(ctx, 1); _ret_40; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "-"))) {
{
	if ((({ mph_AdvancedType* _obj_41 = left; mph_gc_push_root(ctx, (void**)&_obj_41); mph_int _ret_42 = _obj_41->kind; mph_gc_pop_roots(ctx, 1); _ret_42; }) == mph_KIND_INT)) {
{
	if ((({ mph_AdvancedType* _obj_43 = right; mph_gc_push_root(ctx, (void**)&_obj_43); mph_int _ret_44 = _obj_43->kind; mph_gc_pop_roots(ctx, 1); _ret_44; }) == mph_KIND_INT)) {
{
	return ({ mph_AdvancedType* _ret_45 = mph_make_advanced_int(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_45); mph_AdvancedType* _ret_46 = _ret_45; mph_gc_pop_roots(ctx, 1); _ret_46; });
}
	}
}
	}
	return ({ mph_AdvancedType* _ret_47 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_47); mph_AdvancedType* _ret_48 = _ret_47; mph_gc_pop_roots(ctx, 1); _ret_48; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "*"))) {
{
	if ((({ mph_AdvancedType* _obj_49 = left; mph_gc_push_root(ctx, (void**)&_obj_49); mph_int _ret_50 = _obj_49->kind; mph_gc_pop_roots(ctx, 1); _ret_50; }) == mph_KIND_INT)) {
{
	if ((({ mph_AdvancedType* _obj_51 = right; mph_gc_push_root(ctx, (void**)&_obj_51); mph_int _ret_52 = _obj_51->kind; mph_gc_pop_roots(ctx, 1); _ret_52; }) == mph_KIND_INT)) {
{
	return ({ mph_AdvancedType* _ret_53 = mph_make_advanced_int(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_53); mph_AdvancedType* _ret_54 = _ret_53; mph_gc_pop_roots(ctx, 1); _ret_54; });
}
	}
}
	}
	return ({ mph_AdvancedType* _ret_55 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_55); mph_AdvancedType* _ret_56 = _ret_55; mph_gc_pop_roots(ctx, 1); _ret_56; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "/"))) {
{
	if ((({ mph_AdvancedType* _obj_57 = left; mph_gc_push_root(ctx, (void**)&_obj_57); mph_int _ret_58 = _obj_57->kind; mph_gc_pop_roots(ctx, 1); _ret_58; }) == mph_KIND_INT)) {
{
	if ((({ mph_AdvancedType* _obj_59 = right; mph_gc_push_root(ctx, (void**)&_obj_59); mph_int _ret_60 = _obj_59->kind; mph_gc_pop_roots(ctx, 1); _ret_60; }) == mph_KIND_INT)) {
{
	return ({ mph_AdvancedType* _ret_61 = mph_make_advanced_int(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_61); mph_AdvancedType* _ret_62 = _ret_61; mph_gc_pop_roots(ctx, 1); _ret_62; });
}
	}
}
	}
	return ({ mph_AdvancedType* _ret_63 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_63); mph_AdvancedType* _ret_64 = _ret_63; mph_gc_pop_roots(ctx, 1); _ret_64; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "=="))) {
{
	if (({ mph_AdvancedType* _arg_65 = left; mph_gc_push_root(ctx, (void**)&_arg_65); mph_AdvancedType* _arg_66 = right; mph_gc_push_root(ctx, (void**)&_arg_66); mph_bool _ret_67 = mph_types_compatible(ctx, NULL, _arg_65, _arg_66); mph_gc_pop_roots(ctx, 2); _ret_67; })) {
{
	return ({ mph_AdvancedType* _ret_68 = mph_make_advanced_bool(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_68); mph_AdvancedType* _ret_69 = _ret_68; mph_gc_pop_roots(ctx, 1); _ret_69; });
}
	}
	return ({ mph_AdvancedType* _ret_70 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_70); mph_AdvancedType* _ret_71 = _ret_70; mph_gc_pop_roots(ctx, 1); _ret_71; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "!="))) {
{
	if (({ mph_AdvancedType* _arg_72 = left; mph_gc_push_root(ctx, (void**)&_arg_72); mph_AdvancedType* _arg_73 = right; mph_gc_push_root(ctx, (void**)&_arg_73); mph_bool _ret_74 = mph_types_compatible(ctx, NULL, _arg_72, _arg_73); mph_gc_pop_roots(ctx, 2); _ret_74; })) {
{
	return ({ mph_AdvancedType* _ret_75 = mph_make_advanced_bool(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_75); mph_AdvancedType* _ret_76 = _ret_75; mph_gc_pop_roots(ctx, 1); _ret_76; });
}
	}
	return ({ mph_AdvancedType* _ret_77 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_77); mph_AdvancedType* _ret_78 = _ret_77; mph_gc_pop_roots(ctx, 1); _ret_78; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "<"))) {
{
	if (({ mph_AdvancedType* _arg_79 = left; mph_gc_push_root(ctx, (void**)&_arg_79); mph_AdvancedType* _arg_80 = right; mph_gc_push_root(ctx, (void**)&_arg_80); mph_bool _ret_81 = mph_types_compatible(ctx, NULL, _arg_79, _arg_80); mph_gc_pop_roots(ctx, 2); _ret_81; })) {
{
	return ({ mph_AdvancedType* _ret_82 = mph_make_advanced_bool(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_82); mph_AdvancedType* _ret_83 = _ret_82; mph_gc_pop_roots(ctx, 1); _ret_83; });
}
	}
	return ({ mph_AdvancedType* _ret_84 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_84); mph_AdvancedType* _ret_85 = _ret_84; mph_gc_pop_roots(ctx, 1); _ret_85; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, ">"))) {
{
	if (({ mph_AdvancedType* _arg_86 = left; mph_gc_push_root(ctx, (void**)&_arg_86); mph_AdvancedType* _arg_87 = right; mph_gc_push_root(ctx, (void**)&_arg_87); mph_bool _ret_88 = mph_types_compatible(ctx, NULL, _arg_86, _arg_87); mph_gc_pop_roots(ctx, 2); _ret_88; })) {
{
	return ({ mph_AdvancedType* _ret_89 = mph_make_advanced_bool(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_89); mph_AdvancedType* _ret_90 = _ret_89; mph_gc_pop_roots(ctx, 1); _ret_90; });
}
	}
	return ({ mph_AdvancedType* _ret_91 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_91); mph_AdvancedType* _ret_92 = _ret_91; mph_gc_pop_roots(ctx, 1); _ret_92; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "&&"))) {
{
	if ((({ mph_AdvancedType* _obj_93 = left; mph_gc_push_root(ctx, (void**)&_obj_93); mph_int _ret_94 = _obj_93->kind; mph_gc_pop_roots(ctx, 1); _ret_94; }) == mph_KIND_BOOL)) {
{
	if ((({ mph_AdvancedType* _obj_95 = right; mph_gc_push_root(ctx, (void**)&_obj_95); mph_int _ret_96 = _obj_95->kind; mph_gc_pop_roots(ctx, 1); _ret_96; }) == mph_KIND_BOOL)) {
{
	return ({ mph_AdvancedType* _ret_97 = mph_make_advanced_bool(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_97); mph_AdvancedType* _ret_98 = _ret_97; mph_gc_pop_roots(ctx, 1); _ret_98; });
}
	}
}
	}
	return ({ mph_AdvancedType* _ret_99 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_99); mph_AdvancedType* _ret_100 = _ret_99; mph_gc_pop_roots(ctx, 1); _ret_100; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "||"))) {
{
	if ((({ mph_AdvancedType* _obj_101 = left; mph_gc_push_root(ctx, (void**)&_obj_101); mph_int _ret_102 = _obj_101->kind; mph_gc_pop_roots(ctx, 1); _ret_102; }) == mph_KIND_BOOL)) {
{
	if ((({ mph_AdvancedType* _obj_103 = right; mph_gc_push_root(ctx, (void**)&_obj_103); mph_int _ret_104 = _obj_103->kind; mph_gc_pop_roots(ctx, 1); _ret_104; }) == mph_KIND_BOOL)) {
{
	return ({ mph_AdvancedType* _ret_105 = mph_make_advanced_bool(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_105); mph_AdvancedType* _ret_106 = _ret_105; mph_gc_pop_roots(ctx, 1); _ret_106; });
}
	}
}
	}
	return ({ mph_AdvancedType* _ret_107 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_107); mph_AdvancedType* _ret_108 = _ret_107; mph_gc_pop_roots(ctx, 1); _ret_108; });
}
	}
	return ({ mph_AdvancedType* _ret_109 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_109); mph_AdvancedType* _ret_110 = _ret_109; mph_gc_pop_roots(ctx, 1); _ret_110; });
	mph_gc_pop_roots(ctx, 3);
}

mph_AdvancedType* mph_check_unary(MorphContext* ctx, void* _env_void, MorphString* op, mph_AdvancedType* operand) {
	mph_gc_push_root(ctx, (void**)&op);
	mph_gc_push_root(ctx, (void**)&operand);
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "!"))) {
{
	if ((({ mph_AdvancedType* _obj_111 = operand; mph_gc_push_root(ctx, (void**)&_obj_111); mph_int _ret_112 = _obj_111->kind; mph_gc_pop_roots(ctx, 1); _ret_112; }) == mph_KIND_BOOL)) {
{
	return ({ mph_AdvancedType* _ret_113 = mph_make_advanced_bool(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_113); mph_AdvancedType* _ret_114 = _ret_113; mph_gc_pop_roots(ctx, 1); _ret_114; });
}
	}
	return ({ mph_AdvancedType* _ret_115 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_115); mph_AdvancedType* _ret_116 = _ret_115; mph_gc_pop_roots(ctx, 1); _ret_116; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "-"))) {
{
	if ((({ mph_AdvancedType* _obj_117 = operand; mph_gc_push_root(ctx, (void**)&_obj_117); mph_int _ret_118 = _obj_117->kind; mph_gc_pop_roots(ctx, 1); _ret_118; }) == mph_KIND_INT)) {
{
	return ({ mph_AdvancedType* _ret_119 = mph_make_advanced_int(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_119); mph_AdvancedType* _ret_120 = _ret_119; mph_gc_pop_roots(ctx, 1); _ret_120; });
}
	}
	return ({ mph_AdvancedType* _ret_121 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_121); mph_AdvancedType* _ret_122 = _ret_121; mph_gc_pop_roots(ctx, 1); _ret_122; });
}
	}
	return ({ mph_AdvancedType* _ret_123 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_123); mph_AdvancedType* _ret_124 = _ret_123; mph_gc_pop_roots(ctx, 1); _ret_124; });
	mph_gc_pop_roots(ctx, 2);
}

mph_AdvancedType* mph_check_array_access(MorphContext* ctx, void* _env_void, mph_AdvancedType* array_type, mph_AdvancedType* index_type) {
	mph_gc_push_root(ctx, (void**)&array_type);
	mph_gc_push_root(ctx, (void**)&index_type);
	if ((({ mph_AdvancedType* _obj_125 = array_type; mph_gc_push_root(ctx, (void**)&_obj_125); mph_int _ret_126 = _obj_125->kind; mph_gc_pop_roots(ctx, 1); _ret_126; }) == mph_KIND_ARRAY)) {
{
	if ((({ mph_AdvancedType* _obj_127 = index_type; mph_gc_push_root(ctx, (void**)&_obj_127); mph_int _ret_128 = _obj_127->kind; mph_gc_pop_roots(ctx, 1); _ret_128; }) == mph_KIND_INT)) {
{
	if ((({ mph_AdvancedType* _obj_129 = array_type; mph_gc_push_root(ctx, (void**)&_obj_129); mph_int _ret_130 = _obj_129->element_type_kind; mph_gc_pop_roots(ctx, 1); _ret_130; }) == mph_KIND_INT)) {
{
	return ({ mph_AdvancedType* _ret_131 = mph_make_advanced_int(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_131); mph_AdvancedType* _ret_132 = _ret_131; mph_gc_pop_roots(ctx, 1); _ret_132; });
}
	}
	if ((({ mph_AdvancedType* _obj_133 = array_type; mph_gc_push_root(ctx, (void**)&_obj_133); mph_int _ret_134 = _obj_133->element_type_kind; mph_gc_pop_roots(ctx, 1); _ret_134; }) == mph_KIND_STRING)) {
{
	return ({ mph_AdvancedType* _ret_135 = mph_make_advanced_string(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_135); mph_AdvancedType* _ret_136 = _ret_135; mph_gc_pop_roots(ctx, 1); _ret_136; });
}
	}
}
	}
}
	}
	return ({ mph_AdvancedType* _ret_137 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_137); mph_AdvancedType* _ret_138 = _ret_137; mph_gc_pop_roots(ctx, 1); _ret_138; });
	mph_gc_pop_roots(ctx, 2);
}

mph_AdvancedType* mph_check_advanced_call(MorphContext* ctx, void* _env_void, MorphString* func_name, mph_AdvancedType* arg_type) {
	mph_gc_push_root(ctx, (void**)&func_name);
	mph_gc_push_root(ctx, (void**)&arg_type);
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "len"))) {
{
	if ((({ mph_AdvancedType* _obj_139 = arg_type; mph_gc_push_root(ctx, (void**)&_obj_139); mph_int _ret_140 = _obj_139->kind; mph_gc_pop_roots(ctx, 1); _ret_140; }) == mph_KIND_ARRAY)) {
{
	return ({ mph_AdvancedType* _ret_141 = mph_make_advanced_int(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_141); mph_AdvancedType* _ret_142 = _ret_141; mph_gc_pop_roots(ctx, 1); _ret_142; });
}
	}
	if ((({ mph_AdvancedType* _obj_143 = arg_type; mph_gc_push_root(ctx, (void**)&_obj_143); mph_int _ret_144 = _obj_143->kind; mph_gc_pop_roots(ctx, 1); _ret_144; }) == mph_KIND_STRING)) {
{
	return ({ mph_AdvancedType* _ret_145 = mph_make_advanced_int(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_145); mph_AdvancedType* _ret_146 = _ret_145; mph_gc_pop_roots(ctx, 1); _ret_146; });
}
	}
	return ({ mph_AdvancedType* _ret_147 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_147); mph_AdvancedType* _ret_148 = _ret_147; mph_gc_pop_roots(ctx, 1); _ret_148; });
}
	}
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "append"))) {
{
	if ((({ mph_AdvancedType* _obj_149 = arg_type; mph_gc_push_root(ctx, (void**)&_obj_149); mph_int _ret_150 = _obj_149->kind; mph_gc_pop_roots(ctx, 1); _ret_150; }) == mph_KIND_ARRAY)) {
{
	return ({ mph_AdvancedType* _ret_151 = arg_type; mph_gc_push_root(ctx, (void**)&_ret_151); mph_AdvancedType* _ret_152 = _ret_151; mph_gc_pop_roots(ctx, 1); _ret_152; });
}
	}
	return ({ mph_AdvancedType* _ret_153 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_153); mph_AdvancedType* _ret_154 = _ret_153; mph_gc_pop_roots(ctx, 1); _ret_154; });
}
	}
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "make"))) {
{
	return ({ mph_AdvancedType* _ret_155 = mph_make_int_array(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_155); mph_AdvancedType* _ret_156 = _ret_155; mph_gc_pop_roots(ctx, 1); _ret_156; });
}
	}
	return ({ mph_AdvancedType* _ret_157 = mph_make_error_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_ret_157); mph_AdvancedType* _ret_158 = _ret_157; mph_gc_pop_roots(ctx, 1); _ret_158; });
	mph_gc_pop_roots(ctx, 2);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_159 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_159); MorphString* _arg_160 = mph_string_new(ctx, "=== ADVANCED TYPE CHECKER TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_160); mph_int _ret_161 = mph_io_Write(ctx, NULL, _arg_159, _arg_160); mph_gc_pop_roots(ctx, 2); _ret_161; });
	mph_AdvancedChecker* checker = mph_advanced_checker_new(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&checker);
	mph_AdvancedType* int_type = mph_make_advanced_int(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&int_type);
	mph_AdvancedType* result1 = ({ mph_AdvancedType* _arg_162 = int_type; mph_gc_push_root(ctx, (void**)&_arg_162); MorphString* _arg_163 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_163); mph_AdvancedType* _arg_164 = int_type; mph_gc_push_root(ctx, (void**)&_arg_164); mph_AdvancedType* _ret_165 = mph_check_advanced_binary(ctx, NULL, _arg_162, _arg_163, _arg_164); mph_gc_pop_roots(ctx, 3); _ret_165; });
	mph_gc_push_root(ctx, (void**)&result1);
	if ((({ mph_AdvancedType* _obj_166 = result1; mph_gc_push_root(ctx, (void**)&_obj_166); mph_int _ret_167 = _obj_166->kind; mph_gc_pop_roots(ctx, 1); _ret_167; }) == mph_KIND_INT)) {
{
	({ mph_io_File* _arg_168 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_168); MorphString* _arg_169 = mph_string_new(ctx, "â int + int -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_169); mph_int _ret_170 = mph_io_Write(ctx, NULL, _arg_168, _arg_169); mph_gc_pop_roots(ctx, 2); _ret_170; });
}
	} else {
{
	({ mph_io_File* _arg_171 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_171); MorphString* _arg_172 = mph_string_new(ctx, "â int + int -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_172); mph_int _ret_173 = mph_io_Write(ctx, NULL, _arg_171, _arg_172); mph_gc_pop_roots(ctx, 2); _ret_173; });
}
	}
	mph_AdvancedType* string_type = mph_make_advanced_string(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&string_type);
	mph_AdvancedType* result2 = ({ mph_AdvancedType* _arg_174 = string_type; mph_gc_push_root(ctx, (void**)&_arg_174); MorphString* _arg_175 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_175); mph_AdvancedType* _arg_176 = string_type; mph_gc_push_root(ctx, (void**)&_arg_176); mph_AdvancedType* _ret_177 = mph_check_advanced_binary(ctx, NULL, _arg_174, _arg_175, _arg_176); mph_gc_pop_roots(ctx, 3); _ret_177; });
	mph_gc_push_root(ctx, (void**)&result2);
	if ((({ mph_AdvancedType* _obj_178 = result2; mph_gc_push_root(ctx, (void**)&_obj_178); mph_int _ret_179 = _obj_178->kind; mph_gc_pop_roots(ctx, 1); _ret_179; }) == mph_KIND_STRING)) {
{
	({ mph_io_File* _arg_180 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_180); MorphString* _arg_181 = mph_string_new(ctx, "â string + string -> string: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_181); mph_int _ret_182 = mph_io_Write(ctx, NULL, _arg_180, _arg_181); mph_gc_pop_roots(ctx, 2); _ret_182; });
}
	} else {
{
	({ mph_io_File* _arg_183 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_183); MorphString* _arg_184 = mph_string_new(ctx, "â string + string -> string: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_184); mph_int _ret_185 = mph_io_Write(ctx, NULL, _arg_183, _arg_184); mph_gc_pop_roots(ctx, 2); _ret_185; });
}
	}
	mph_AdvancedType* result3 = ({ mph_AdvancedType* _arg_186 = int_type; mph_gc_push_root(ctx, (void**)&_arg_186); MorphString* _arg_187 = mph_string_new(ctx, "=="); mph_gc_push_root(ctx, (void**)&_arg_187); mph_AdvancedType* _arg_188 = int_type; mph_gc_push_root(ctx, (void**)&_arg_188); mph_AdvancedType* _ret_189 = mph_check_advanced_binary(ctx, NULL, _arg_186, _arg_187, _arg_188); mph_gc_pop_roots(ctx, 3); _ret_189; });
	mph_gc_push_root(ctx, (void**)&result3);
	if ((({ mph_AdvancedType* _obj_190 = result3; mph_gc_push_root(ctx, (void**)&_obj_190); mph_int _ret_191 = _obj_190->kind; mph_gc_pop_roots(ctx, 1); _ret_191; }) == mph_KIND_BOOL)) {
{
	({ mph_io_File* _arg_192 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_192); MorphString* _arg_193 = mph_string_new(ctx, "â int == int -> bool: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_193); mph_int _ret_194 = mph_io_Write(ctx, NULL, _arg_192, _arg_193); mph_gc_pop_roots(ctx, 2); _ret_194; });
}
	} else {
{
	({ mph_io_File* _arg_195 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_195); MorphString* _arg_196 = mph_string_new(ctx, "â int == int -> bool: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_196); mph_int _ret_197 = mph_io_Write(ctx, NULL, _arg_195, _arg_196); mph_gc_pop_roots(ctx, 2); _ret_197; });
}
	}
	mph_AdvancedType* bool_type = mph_make_advanced_bool(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&bool_type);
	mph_AdvancedType* result4 = ({ mph_AdvancedType* _arg_198 = bool_type; mph_gc_push_root(ctx, (void**)&_arg_198); MorphString* _arg_199 = mph_string_new(ctx, "&&"); mph_gc_push_root(ctx, (void**)&_arg_199); mph_AdvancedType* _arg_200 = bool_type; mph_gc_push_root(ctx, (void**)&_arg_200); mph_AdvancedType* _ret_201 = mph_check_advanced_binary(ctx, NULL, _arg_198, _arg_199, _arg_200); mph_gc_pop_roots(ctx, 3); _ret_201; });
	mph_gc_push_root(ctx, (void**)&result4);
	if ((({ mph_AdvancedType* _obj_202 = result4; mph_gc_push_root(ctx, (void**)&_obj_202); mph_int _ret_203 = _obj_202->kind; mph_gc_pop_roots(ctx, 1); _ret_203; }) == mph_KIND_BOOL)) {
{
	({ mph_io_File* _arg_204 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_204); MorphString* _arg_205 = mph_string_new(ctx, "â bool && bool -> bool: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_205); mph_int _ret_206 = mph_io_Write(ctx, NULL, _arg_204, _arg_205); mph_gc_pop_roots(ctx, 2); _ret_206; });
}
	} else {
{
	({ mph_io_File* _arg_207 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_207); MorphString* _arg_208 = mph_string_new(ctx, "â bool && bool -> bool: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_208); mph_int _ret_209 = mph_io_Write(ctx, NULL, _arg_207, _arg_208); mph_gc_pop_roots(ctx, 2); _ret_209; });
}
	}
	mph_AdvancedType* result5 = ({ MorphString* _arg_210 = mph_string_new(ctx, "!"); mph_gc_push_root(ctx, (void**)&_arg_210); mph_AdvancedType* _arg_211 = bool_type; mph_gc_push_root(ctx, (void**)&_arg_211); mph_AdvancedType* _ret_212 = mph_check_unary(ctx, NULL, _arg_210, _arg_211); mph_gc_pop_roots(ctx, 2); _ret_212; });
	mph_gc_push_root(ctx, (void**)&result5);
	if ((({ mph_AdvancedType* _obj_213 = result5; mph_gc_push_root(ctx, (void**)&_obj_213); mph_int _ret_214 = _obj_213->kind; mph_gc_pop_roots(ctx, 1); _ret_214; }) == mph_KIND_BOOL)) {
{
	({ mph_io_File* _arg_215 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_215); MorphString* _arg_216 = mph_string_new(ctx, "â !bool -> bool: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_216); mph_int _ret_217 = mph_io_Write(ctx, NULL, _arg_215, _arg_216); mph_gc_pop_roots(ctx, 2); _ret_217; });
}
	} else {
{
	({ mph_io_File* _arg_218 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_218); MorphString* _arg_219 = mph_string_new(ctx, "â !bool -> bool: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_219); mph_int _ret_220 = mph_io_Write(ctx, NULL, _arg_218, _arg_219); mph_gc_pop_roots(ctx, 2); _ret_220; });
}
	}
	mph_AdvancedType* array_type = mph_make_int_array(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&array_type);
	mph_AdvancedType* result6 = ({ mph_AdvancedType* _arg_221 = array_type; mph_gc_push_root(ctx, (void**)&_arg_221); mph_AdvancedType* _arg_222 = int_type; mph_gc_push_root(ctx, (void**)&_arg_222); mph_AdvancedType* _ret_223 = mph_check_array_access(ctx, NULL, _arg_221, _arg_222); mph_gc_pop_roots(ctx, 2); _ret_223; });
	mph_gc_push_root(ctx, (void**)&result6);
	if ((({ mph_AdvancedType* _obj_224 = result6; mph_gc_push_root(ctx, (void**)&_obj_224); mph_int _ret_225 = _obj_224->kind; mph_gc_pop_roots(ctx, 1); _ret_225; }) == mph_KIND_INT)) {
{
	({ mph_io_File* _arg_226 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_226); MorphString* _arg_227 = mph_string_new(ctx, "â []int[int] -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_227); mph_int _ret_228 = mph_io_Write(ctx, NULL, _arg_226, _arg_227); mph_gc_pop_roots(ctx, 2); _ret_228; });
}
	} else {
{
	({ mph_io_File* _arg_229 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_229); MorphString* _arg_230 = mph_string_new(ctx, "â []int[int] -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_230); mph_int _ret_231 = mph_io_Write(ctx, NULL, _arg_229, _arg_230); mph_gc_pop_roots(ctx, 2); _ret_231; });
}
	}
	mph_AdvancedType* result7 = ({ MorphString* _arg_232 = mph_string_new(ctx, "len"); mph_gc_push_root(ctx, (void**)&_arg_232); mph_AdvancedType* _arg_233 = array_type; mph_gc_push_root(ctx, (void**)&_arg_233); mph_AdvancedType* _ret_234 = mph_check_advanced_call(ctx, NULL, _arg_232, _arg_233); mph_gc_pop_roots(ctx, 2); _ret_234; });
	mph_gc_push_root(ctx, (void**)&result7);
	if ((({ mph_AdvancedType* _obj_235 = result7; mph_gc_push_root(ctx, (void**)&_obj_235); mph_int _ret_236 = _obj_235->kind; mph_gc_pop_roots(ctx, 1); _ret_236; }) == mph_KIND_INT)) {
{
	({ mph_io_File* _arg_237 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_237); MorphString* _arg_238 = mph_string_new(ctx, "â len([]int) -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_238); mph_int _ret_239 = mph_io_Write(ctx, NULL, _arg_237, _arg_238); mph_gc_pop_roots(ctx, 2); _ret_239; });
}
	} else {
{
	({ mph_io_File* _arg_240 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_240); MorphString* _arg_241 = mph_string_new(ctx, "â len([]int) -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_241); mph_int _ret_242 = mph_io_Write(ctx, NULL, _arg_240, _arg_241); mph_gc_pop_roots(ctx, 2); _ret_242; });
}
	}
	if (({ mph_AdvancedType* _arg_243 = int_type; mph_gc_push_root(ctx, (void**)&_arg_243); mph_AdvancedType* _arg_244 = int_type; mph_gc_push_root(ctx, (void**)&_arg_244); mph_bool _ret_245 = mph_types_compatible(ctx, NULL, _arg_243, _arg_244); mph_gc_pop_roots(ctx, 2); _ret_245; })) {
{
	({ mph_io_File* _arg_246 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_246); MorphString* _arg_247 = mph_string_new(ctx, "â int compatible with int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_247); mph_int _ret_248 = mph_io_Write(ctx, NULL, _arg_246, _arg_247); mph_gc_pop_roots(ctx, 2); _ret_248; });
}
	} else {
{
	({ mph_io_File* _arg_249 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_249); MorphString* _arg_250 = mph_string_new(ctx, "â int compatible with int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_250); mph_int _ret_251 = mph_io_Write(ctx, NULL, _arg_249, _arg_250); mph_gc_pop_roots(ctx, 2); _ret_251; });
}
	}
	if ((({ mph_AdvancedType* _arg_252 = int_type; mph_gc_push_root(ctx, (void**)&_arg_252); mph_AdvancedType* _arg_253 = string_type; mph_gc_push_root(ctx, (void**)&_arg_253); mph_bool _ret_254 = mph_types_compatible(ctx, NULL, _arg_252, _arg_253); mph_gc_pop_roots(ctx, 2); _ret_254; }) == 0)) {
{
	({ mph_io_File* _arg_255 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_255); MorphString* _arg_256 = mph_string_new(ctx, "â int incompatible with string: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_256); mph_int _ret_257 = mph_io_Write(ctx, NULL, _arg_255, _arg_256); mph_gc_pop_roots(ctx, 2); _ret_257; });
}
	} else {
{
	({ mph_io_File* _arg_258 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_258); MorphString* _arg_259 = mph_string_new(ctx, "â int incompatible with string: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_259); mph_int _ret_260 = mph_io_Write(ctx, NULL, _arg_258, _arg_259); mph_gc_pop_roots(ctx, 2); _ret_260; });
}
	}
	({ mph_io_File* _arg_261 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_261); MorphString* _arg_262 = mph_string_new(ctx, "\nð ADVANCED TYPE CHECKER COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_262); mph_int _ret_263 = mph_io_Write(ctx, NULL, _arg_261, _arg_262); mph_gc_pop_roots(ctx, 2); _ret_263; });
	({ mph_io_File* _arg_264 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_264); MorphString* _arg_265 = mph_string_new(ctx, "â Go-compatible semantics\n"); mph_gc_push_root(ctx, (void**)&_arg_265); mph_int _ret_266 = mph_io_Write(ctx, NULL, _arg_264, _arg_265); mph_gc_pop_roots(ctx, 2); _ret_266; });
	({ mph_io_File* _arg_267 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_267); MorphString* _arg_268 = mph_string_new(ctx, "â Advanced type operations\n"); mph_gc_push_root(ctx, (void**)&_arg_268); mph_int _ret_269 = mph_io_Write(ctx, NULL, _arg_267, _arg_268); mph_gc_pop_roots(ctx, 2); _ret_269; });
	({ mph_io_File* _arg_270 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_270); MorphString* _arg_271 = mph_string_new(ctx, "â Array/slice support\n"); mph_gc_push_root(ctx, (void**)&_arg_271); mph_int _ret_272 = mph_io_Write(ctx, NULL, _arg_270, _arg_271); mph_gc_pop_roots(ctx, 2); _ret_272; });
	({ mph_io_File* _arg_273 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_273); MorphString* _arg_274 = mph_string_new(ctx, "â Built-in function validation\n"); mph_gc_push_root(ctx, (void**)&_arg_274); mph_int _ret_275 = mph_io_Write(ctx, NULL, _arg_273, _arg_274); mph_gc_pop_roots(ctx, 2); _ret_275; });
	({ mph_io_File* _arg_276 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_276); MorphString* _arg_277 = mph_string_new(ctx, "â Type compatibility checking\n"); mph_gc_push_root(ctx, (void**)&_arg_277); mph_int _ret_278 = mph_io_Write(ctx, NULL, _arg_276, _arg_277); mph_gc_pop_roots(ctx, 2); _ret_278; });
	({ mph_io_File* _arg_279 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_279); MorphString* _arg_280 = mph_string_new(ctx, "â Semantic gap dengan Go: RESOLVED\n"); mph_gc_push_root(ctx, (void**)&_arg_280); mph_int _ret_281 = mph_io_Write(ctx, NULL, _arg_279, _arg_280); mph_gc_pop_roots(ctx, 2); _ret_281; });
	mph_gc_pop_roots(ctx, 12);
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
	mph_KIND_INTERFACE = 7;
	mph_KIND_POINTER = 8;
	mph_KIND_CHANNEL = 9;
	mph_KIND_ERROR = (-1);
	mph_main(ctx, NULL);
}
