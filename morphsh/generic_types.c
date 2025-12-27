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
#define MPH_TYPE_mph_GenericType 1

// Struct Definitions (Env & Types)
typedef struct mph_GenericType mph_GenericType;
struct mph_GenericType {
	mph_int base_kind;
	mph_int element_kind;
	mph_int key_kind;
	mph_int value_kind;
	mph_bool is_generic;
};

MorphTypeInfo mph_ti_mph_GenericType = { "GenericType", sizeof(mph_GenericType), 0, NULL };

// Global Variables
mph_int mph_io_Stdout;
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_BOOL;
mph_int mph_KIND_ARRAY;
mph_int mph_KIND_MAP;
mph_int mph_KIND_GENERIC;
mph_int mph_KIND_ERROR;

// Function Prototypes
mph_GenericType* mph_make_array_type(MorphContext* ctx, void* _env_void, mph_int element_type);
mph_GenericType* mph_make_map_type(MorphContext* ctx, void* _env_void, mph_int key_type, mph_int value_type);
mph_bool mph_types_compatible(MorphContext* ctx, void* _env_void, mph_GenericType* t1, mph_GenericType* t2);
mph_int mph_check_array_access(MorphContext* ctx, void* _env_void, mph_GenericType* array_type, mph_int index_type);
mph_int mph_check_map_access(MorphContext* ctx, void* _env_void, mph_GenericType* map_type, mph_int key_type);
mph_int mph_check_len_function(MorphContext* ctx, void* _env_void, mph_GenericType* arg_type);
mph_GenericType* mph_check_append_function(MorphContext* ctx, void* _env_void, mph_GenericType* array_type, mph_int element_type);
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

mph_GenericType* mph_make_array_type(MorphContext* ctx, void* _env_void, mph_int element_type) {
	return ({ mph_GenericType* _ret_1 = ({ mph_GenericType* _t = (mph_GenericType*)mph_alloc(ctx, sizeof(mph_GenericType), &mph_ti_mph_GenericType); mph_gc_push_root(ctx, (void**)&_t); _t->element_kind = element_type; _t->key_kind = mph_KIND_ERROR; _t->value_kind = mph_KIND_ERROR; _t->is_generic = 1; _t->base_kind = mph_KIND_ARRAY; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_GenericType* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_GenericType* mph_make_map_type(MorphContext* ctx, void* _env_void, mph_int key_type, mph_int value_type) {
	return ({ mph_GenericType* _ret_3 = ({ mph_GenericType* _t = (mph_GenericType*)mph_alloc(ctx, sizeof(mph_GenericType), &mph_ti_mph_GenericType); mph_gc_push_root(ctx, (void**)&_t); _t->base_kind = mph_KIND_MAP; _t->element_kind = mph_KIND_ERROR; _t->key_kind = key_type; _t->value_kind = value_type; _t->is_generic = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_GenericType* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}

mph_bool mph_types_compatible(MorphContext* ctx, void* _env_void, mph_GenericType* t1, mph_GenericType* t2) {
	mph_gc_push_root(ctx, (void**)&t1);
	mph_gc_push_root(ctx, (void**)&t2);
	if ((({ mph_GenericType* _obj_5 = t1; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->base_kind; mph_gc_pop_roots(ctx, 1); _ret_6; }) != ({ mph_GenericType* _obj_7 = t2; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->base_kind; mph_gc_pop_roots(ctx, 1); _ret_8; }))) {
{
	return 0;
}
	}
	if ((({ mph_GenericType* _obj_9 = t1; mph_gc_push_root(ctx, (void**)&_obj_9); mph_int _ret_10 = _obj_9->base_kind; mph_gc_pop_roots(ctx, 1); _ret_10; }) == mph_KIND_ARRAY)) {
{
	return (({ mph_GenericType* _obj_11 = t1; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->element_kind; mph_gc_pop_roots(ctx, 1); _ret_12; }) == ({ mph_GenericType* _obj_13 = t2; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->element_kind; mph_gc_pop_roots(ctx, 1); _ret_14; }));
}
	}
	if ((({ mph_GenericType* _obj_15 = t1; mph_gc_push_root(ctx, (void**)&_obj_15); mph_int _ret_16 = _obj_15->base_kind; mph_gc_pop_roots(ctx, 1); _ret_16; }) == mph_KIND_MAP)) {
{
	if ((({ mph_GenericType* _obj_17 = t1; mph_gc_push_root(ctx, (void**)&_obj_17); mph_int _ret_18 = _obj_17->key_kind; mph_gc_pop_roots(ctx, 1); _ret_18; }) == ({ mph_GenericType* _obj_19 = t2; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->key_kind; mph_gc_pop_roots(ctx, 1); _ret_20; }))) {
{
	return (({ mph_GenericType* _obj_21 = t1; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->value_kind; mph_gc_pop_roots(ctx, 1); _ret_22; }) == ({ mph_GenericType* _obj_23 = t2; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->value_kind; mph_gc_pop_roots(ctx, 1); _ret_24; }));
}
	}
	return 0;
}
	}
	return 1;
	mph_gc_pop_roots(ctx, 2);
}

mph_int mph_check_array_access(MorphContext* ctx, void* _env_void, mph_GenericType* array_type, mph_int index_type) {
	mph_gc_push_root(ctx, (void**)&array_type);
	if ((({ mph_GenericType* _obj_25 = array_type; mph_gc_push_root(ctx, (void**)&_obj_25); mph_int _ret_26 = _obj_25->base_kind; mph_gc_pop_roots(ctx, 1); _ret_26; }) == mph_KIND_ARRAY)) {
{
	if ((index_type == mph_KIND_INT)) {
{
	return ({ mph_GenericType* _obj_27 = array_type; mph_gc_push_root(ctx, (void**)&_obj_27); mph_int _ret_28 = _obj_27->element_kind; mph_gc_pop_roots(ctx, 1); _ret_28; });
}
	}
}
	}
	return mph_KIND_ERROR;
	mph_gc_pop_roots(ctx, 1);
}

mph_int mph_check_map_access(MorphContext* ctx, void* _env_void, mph_GenericType* map_type, mph_int key_type) {
	mph_gc_push_root(ctx, (void**)&map_type);
	if ((({ mph_GenericType* _obj_29 = map_type; mph_gc_push_root(ctx, (void**)&_obj_29); mph_int _ret_30 = _obj_29->base_kind; mph_gc_pop_roots(ctx, 1); _ret_30; }) == mph_KIND_MAP)) {
{
	if ((key_type == ({ mph_GenericType* _obj_31 = map_type; mph_gc_push_root(ctx, (void**)&_obj_31); mph_int _ret_32 = _obj_31->key_kind; mph_gc_pop_roots(ctx, 1); _ret_32; }))) {
{
	return ({ mph_GenericType* _obj_33 = map_type; mph_gc_push_root(ctx, (void**)&_obj_33); mph_int _ret_34 = _obj_33->value_kind; mph_gc_pop_roots(ctx, 1); _ret_34; });
}
	}
}
	}
	return mph_KIND_ERROR;
	mph_gc_pop_roots(ctx, 1);
}

mph_int mph_check_len_function(MorphContext* ctx, void* _env_void, mph_GenericType* arg_type) {
	mph_gc_push_root(ctx, (void**)&arg_type);
	if ((({ mph_GenericType* _obj_35 = arg_type; mph_gc_push_root(ctx, (void**)&_obj_35); mph_int _ret_36 = _obj_35->base_kind; mph_gc_pop_roots(ctx, 1); _ret_36; }) == mph_KIND_ARRAY)) {
{
	return mph_KIND_INT;
}
	}
	if ((({ mph_GenericType* _obj_37 = arg_type; mph_gc_push_root(ctx, (void**)&_obj_37); mph_int _ret_38 = _obj_37->base_kind; mph_gc_pop_roots(ctx, 1); _ret_38; }) == mph_KIND_MAP)) {
{
	return mph_KIND_INT;
}
	}
	return mph_KIND_ERROR;
	mph_gc_pop_roots(ctx, 1);
}

mph_GenericType* mph_check_append_function(MorphContext* ctx, void* _env_void, mph_GenericType* array_type, mph_int element_type) {
	mph_gc_push_root(ctx, (void**)&array_type);
	if ((({ mph_GenericType* _obj_39 = array_type; mph_gc_push_root(ctx, (void**)&_obj_39); mph_int _ret_40 = _obj_39->base_kind; mph_gc_pop_roots(ctx, 1); _ret_40; }) == mph_KIND_ARRAY)) {
{
	if ((element_type == ({ mph_GenericType* _obj_41 = array_type; mph_gc_push_root(ctx, (void**)&_obj_41); mph_int _ret_42 = _obj_41->element_kind; mph_gc_pop_roots(ctx, 1); _ret_42; }))) {
{
	return ({ mph_GenericType* _ret_43 = array_type; mph_gc_push_root(ctx, (void**)&_ret_43); mph_GenericType* _ret_44 = _ret_43; mph_gc_pop_roots(ctx, 1); _ret_44; });
}
	}
}
	}
	return ({ mph_GenericType* _ret_45 = ({ mph_GenericType* _t = (mph_GenericType*)mph_alloc(ctx, sizeof(mph_GenericType), &mph_ti_mph_GenericType); mph_gc_push_root(ctx, (void**)&_t); _t->is_generic = 0; _t->base_kind = mph_KIND_ERROR; _t->element_kind = mph_KIND_ERROR; _t->key_kind = mph_KIND_ERROR; _t->value_kind = mph_KIND_ERROR; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_45); mph_GenericType* _ret_46 = _ret_45; mph_gc_pop_roots(ctx, 1); _ret_46; });
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ MorphString* _arg_47 = mph_string_new(ctx, "=== GENERIC TYPES TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_47); mph_int _ret_48 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_47); mph_gc_pop_roots(ctx, 1); _ret_48; });
	mph_GenericType* int_array = mph_make_array_type(ctx, NULL, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&int_array);
	if ((({ mph_GenericType* _obj_49 = int_array; mph_gc_push_root(ctx, (void**)&_obj_49); mph_int _ret_50 = _obj_49->base_kind; mph_gc_pop_roots(ctx, 1); _ret_50; }) == mph_KIND_ARRAY)) {
{
	({ MorphString* _arg_51 = mph_string_new(ctx, "â Create []int type: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_51); mph_int _ret_52 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_51); mph_gc_pop_roots(ctx, 1); _ret_52; });
}
	} else {
{
	({ MorphString* _arg_53 = mph_string_new(ctx, "â Create []int type: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_53); mph_int _ret_54 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_53); mph_gc_pop_roots(ctx, 1); _ret_54; });
}
	}
	mph_GenericType* string_int_map = mph_make_map_type(ctx, NULL, mph_KIND_STRING, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&string_int_map);
	if ((({ mph_GenericType* _obj_55 = string_int_map; mph_gc_push_root(ctx, (void**)&_obj_55); mph_int _ret_56 = _obj_55->base_kind; mph_gc_pop_roots(ctx, 1); _ret_56; }) == mph_KIND_MAP)) {
{
	({ MorphString* _arg_57 = mph_string_new(ctx, "â Create map[string]int type: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_57); mph_int _ret_58 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_57); mph_gc_pop_roots(ctx, 1); _ret_58; });
}
	} else {
{
	({ MorphString* _arg_59 = mph_string_new(ctx, "â Create map[string]int type: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_59); mph_int _ret_60 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_59); mph_gc_pop_roots(ctx, 1); _ret_60; });
}
	}
	mph_int access_result = ({ mph_GenericType* _arg_61 = int_array; mph_gc_push_root(ctx, (void**)&_arg_61); mph_int _ret_62 = mph_check_array_access(ctx, NULL, _arg_61, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_62; });
	if ((access_result == mph_KIND_INT)) {
{
	({ MorphString* _arg_63 = mph_string_new(ctx, "â Array access []int[int] -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_63); mph_int _ret_64 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_63); mph_gc_pop_roots(ctx, 1); _ret_64; });
}
	} else {
{
	({ MorphString* _arg_65 = mph_string_new(ctx, "â Array access []int[int] -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_65); mph_int _ret_66 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_65); mph_gc_pop_roots(ctx, 1); _ret_66; });
}
	}
	mph_int map_access_result = ({ mph_GenericType* _arg_67 = string_int_map; mph_gc_push_root(ctx, (void**)&_arg_67); mph_int _ret_68 = mph_check_map_access(ctx, NULL, _arg_67, mph_KIND_STRING); mph_gc_pop_roots(ctx, 1); _ret_68; });
	if ((map_access_result == mph_KIND_INT)) {
{
	({ MorphString* _arg_69 = mph_string_new(ctx, "â Map access map[string]int[string] -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_69); mph_int _ret_70 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_69); mph_gc_pop_roots(ctx, 1); _ret_70; });
}
	} else {
{
	({ MorphString* _arg_71 = mph_string_new(ctx, "â Map access map[string]int[string] -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_71); mph_int _ret_72 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_71); mph_gc_pop_roots(ctx, 1); _ret_72; });
}
	}
	mph_int len_result = ({ mph_GenericType* _arg_73 = int_array; mph_gc_push_root(ctx, (void**)&_arg_73); mph_int _ret_74 = mph_check_len_function(ctx, NULL, _arg_73); mph_gc_pop_roots(ctx, 1); _ret_74; });
	if ((len_result == mph_KIND_INT)) {
{
	({ MorphString* _arg_75 = mph_string_new(ctx, "â len([]int) -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_75); mph_int _ret_76 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_75); mph_gc_pop_roots(ctx, 1); _ret_76; });
}
	} else {
{
	({ MorphString* _arg_77 = mph_string_new(ctx, "â len([]int) -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_77); mph_int _ret_78 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_77); mph_gc_pop_roots(ctx, 1); _ret_78; });
}
	}
	mph_GenericType* append_result = ({ mph_GenericType* _arg_79 = int_array; mph_gc_push_root(ctx, (void**)&_arg_79); mph_GenericType* _ret_80 = mph_check_append_function(ctx, NULL, _arg_79, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_80; });
	mph_gc_push_root(ctx, (void**)&append_result);
	if ((({ mph_GenericType* _obj_81 = append_result; mph_gc_push_root(ctx, (void**)&_obj_81); mph_int _ret_82 = _obj_81->base_kind; mph_gc_pop_roots(ctx, 1); _ret_82; }) == mph_KIND_ARRAY)) {
{
	({ MorphString* _arg_83 = mph_string_new(ctx, "â append([]int, int) -> []int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_83); mph_int _ret_84 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_83); mph_gc_pop_roots(ctx, 1); _ret_84; });
}
	} else {
{
	({ MorphString* _arg_85 = mph_string_new(ctx, "â append([]int, int) -> []int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_85); mph_int _ret_86 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_85); mph_gc_pop_roots(ctx, 1); _ret_86; });
}
	}
	mph_GenericType* int_array2 = mph_make_array_type(ctx, NULL, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&int_array2);
	if (({ mph_GenericType* _arg_87 = int_array; mph_gc_push_root(ctx, (void**)&_arg_87); mph_GenericType* _arg_88 = int_array2; mph_gc_push_root(ctx, (void**)&_arg_88); mph_bool _ret_89 = mph_types_compatible(ctx, NULL, _arg_87, _arg_88); mph_gc_pop_roots(ctx, 2); _ret_89; })) {
{
	({ MorphString* _arg_90 = mph_string_new(ctx, "â Type compatibility []int == []int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_90); mph_int _ret_91 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_90); mph_gc_pop_roots(ctx, 1); _ret_91; });
}
	} else {
{
	({ MorphString* _arg_92 = mph_string_new(ctx, "â Type compatibility []int == []int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_92); mph_int _ret_93 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_92); mph_gc_pop_roots(ctx, 1); _ret_93; });
}
	}
	mph_GenericType* string_array = mph_make_array_type(ctx, NULL, mph_KIND_STRING);
	mph_gc_push_root(ctx, (void**)&string_array);
	if ((({ mph_GenericType* _arg_94 = int_array; mph_gc_push_root(ctx, (void**)&_arg_94); mph_GenericType* _arg_95 = string_array; mph_gc_push_root(ctx, (void**)&_arg_95); mph_bool _ret_96 = mph_types_compatible(ctx, NULL, _arg_94, _arg_95); mph_gc_pop_roots(ctx, 2); _ret_96; }) == 0)) {
{
	({ MorphString* _arg_97 = mph_string_new(ctx, "â Type incompatibility []int != []string: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_97); mph_int _ret_98 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_97); mph_gc_pop_roots(ctx, 1); _ret_98; });
}
	} else {
{
	({ MorphString* _arg_99 = mph_string_new(ctx, "â Type incompatibility []int != []string: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_99); mph_int _ret_100 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_99); mph_gc_pop_roots(ctx, 1); _ret_100; });
}
	}
	mph_int wrong_key_access = ({ mph_GenericType* _arg_101 = string_int_map; mph_gc_push_root(ctx, (void**)&_arg_101); mph_int _ret_102 = mph_check_map_access(ctx, NULL, _arg_101, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_102; });
	if ((wrong_key_access == mph_KIND_ERROR)) {
{
	({ MorphString* _arg_103 = mph_string_new(ctx, "â Wrong key type error detected: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_103); mph_int _ret_104 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_103); mph_gc_pop_roots(ctx, 1); _ret_104; });
}
	} else {
{
	({ MorphString* _arg_105 = mph_string_new(ctx, "â Wrong key type error not detected: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_105); mph_int _ret_106 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_105); mph_gc_pop_roots(ctx, 1); _ret_106; });
}
	}
	({ MorphString* _arg_107 = mph_string_new(ctx, "\nð GENERIC TYPES COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_107); mph_int _ret_108 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_107); mph_gc_pop_roots(ctx, 1); _ret_108; });
	({ MorphString* _arg_109 = mph_string_new(ctx, "â Generic array types working\n"); mph_gc_push_root(ctx, (void**)&_arg_109); mph_int _ret_110 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_109); mph_gc_pop_roots(ctx, 1); _ret_110; });
	({ MorphString* _arg_111 = mph_string_new(ctx, "â Generic map types working\n"); mph_gc_push_root(ctx, (void**)&_arg_111); mph_int _ret_112 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_111); mph_gc_pop_roots(ctx, 1); _ret_112; });
	({ MorphString* _arg_113 = mph_string_new(ctx, "â Generic function checking\n"); mph_gc_push_root(ctx, (void**)&_arg_113); mph_int _ret_114 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_113); mph_gc_pop_roots(ctx, 1); _ret_114; });
	({ MorphString* _arg_115 = mph_string_new(ctx, "â Type compatibility checking\n"); mph_gc_push_root(ctx, (void**)&_arg_115); mph_int _ret_116 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_115); mph_gc_pop_roots(ctx, 1); _ret_116; });
	({ MorphString* _arg_117 = mph_string_new(ctx, "â Error detection functional\n"); mph_gc_push_root(ctx, (void**)&_arg_117); mph_int _ret_118 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_117); mph_gc_pop_roots(ctx, 1); _ret_118; });
	mph_gc_pop_roots(ctx, 5);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = 1;
	mph_KIND_INT = 0;
	mph_KIND_STRING = 1;
	mph_KIND_BOOL = 2;
	mph_KIND_ARRAY = 4;
	mph_KIND_MAP = 5;
	mph_KIND_GENERIC = 6;
	mph_KIND_ERROR = (-1);
	mph_main(ctx, NULL);
}
