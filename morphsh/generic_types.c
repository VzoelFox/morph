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
#define MPH_TYPE_mph_GenericType 2

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_GenericType mph_GenericType;
struct mph_GenericType {
	mph_int base_kind;
	mph_int element_kind;
	mph_int key_kind;
	mph_int value_kind;
	mph_bool is_generic;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_GenericType = { "GenericType", sizeof(mph_GenericType), 0, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;
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

mph_GenericType* mph_make_array_type(MorphContext* ctx, void* _env_void, mph_int element_type) {
	return ({ mph_GenericType* _ret_1 = ({ mph_GenericType* _t = (mph_GenericType*)mph_alloc(ctx, sizeof(mph_GenericType), &mph_ti_mph_GenericType); mph_gc_push_root(ctx, (void**)&_t); _t->key_kind = mph_KIND_ERROR; _t->value_kind = mph_KIND_ERROR; _t->is_generic = 1; _t->base_kind = mph_KIND_ARRAY; _t->element_kind = element_type; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_GenericType* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_GenericType* mph_make_map_type(MorphContext* ctx, void* _env_void, mph_int key_type, mph_int value_type) {
	return ({ mph_GenericType* _ret_3 = ({ mph_GenericType* _t = (mph_GenericType*)mph_alloc(ctx, sizeof(mph_GenericType), &mph_ti_mph_GenericType); mph_gc_push_root(ctx, (void**)&_t); _t->key_kind = key_type; _t->value_kind = value_type; _t->is_generic = 1; _t->base_kind = mph_KIND_MAP; _t->element_kind = mph_KIND_ERROR; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_GenericType* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
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
	return ({ mph_GenericType* _ret_45 = ({ mph_GenericType* _t = (mph_GenericType*)mph_alloc(ctx, sizeof(mph_GenericType), &mph_ti_mph_GenericType); mph_gc_push_root(ctx, (void**)&_t); _t->base_kind = mph_KIND_ERROR; _t->element_kind = mph_KIND_ERROR; _t->key_kind = mph_KIND_ERROR; _t->value_kind = mph_KIND_ERROR; _t->is_generic = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_45); mph_GenericType* _ret_46 = _ret_45; mph_gc_pop_roots(ctx, 1); _ret_46; });
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_47 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_47); MorphString* _arg_48 = mph_string_new(ctx, "=== GENERIC TYPES TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_48); mph_int _ret_49 = mph_io_Write(ctx, NULL, _arg_47, _arg_48); mph_gc_pop_roots(ctx, 2); _ret_49; });
	mph_GenericType* int_array = mph_make_array_type(ctx, NULL, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&int_array);
	if ((({ mph_GenericType* _obj_50 = int_array; mph_gc_push_root(ctx, (void**)&_obj_50); mph_int _ret_51 = _obj_50->base_kind; mph_gc_pop_roots(ctx, 1); _ret_51; }) == mph_KIND_ARRAY)) {
{
	({ mph_io_File* _arg_52 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_52); MorphString* _arg_53 = mph_string_new(ctx, "â Create []int type: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_53); mph_int _ret_54 = mph_io_Write(ctx, NULL, _arg_52, _arg_53); mph_gc_pop_roots(ctx, 2); _ret_54; });
}
	} else {
{
	({ mph_io_File* _arg_55 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_55); MorphString* _arg_56 = mph_string_new(ctx, "â Create []int type: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_56); mph_int _ret_57 = mph_io_Write(ctx, NULL, _arg_55, _arg_56); mph_gc_pop_roots(ctx, 2); _ret_57; });
}
	}
	mph_GenericType* string_int_map = mph_make_map_type(ctx, NULL, mph_KIND_STRING, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&string_int_map);
	if ((({ mph_GenericType* _obj_58 = string_int_map; mph_gc_push_root(ctx, (void**)&_obj_58); mph_int _ret_59 = _obj_58->base_kind; mph_gc_pop_roots(ctx, 1); _ret_59; }) == mph_KIND_MAP)) {
{
	({ mph_io_File* _arg_60 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_60); MorphString* _arg_61 = mph_string_new(ctx, "â Create map[string]int type: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_61); mph_int _ret_62 = mph_io_Write(ctx, NULL, _arg_60, _arg_61); mph_gc_pop_roots(ctx, 2); _ret_62; });
}
	} else {
{
	({ mph_io_File* _arg_63 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_63); MorphString* _arg_64 = mph_string_new(ctx, "â Create map[string]int type: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_64); mph_int _ret_65 = mph_io_Write(ctx, NULL, _arg_63, _arg_64); mph_gc_pop_roots(ctx, 2); _ret_65; });
}
	}
	mph_int access_result = ({ mph_GenericType* _arg_66 = int_array; mph_gc_push_root(ctx, (void**)&_arg_66); mph_int _ret_67 = mph_check_array_access(ctx, NULL, _arg_66, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_67; });
	if ((access_result == mph_KIND_INT)) {
{
	({ mph_io_File* _arg_68 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_68); MorphString* _arg_69 = mph_string_new(ctx, "â Array access []int[int] -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_69); mph_int _ret_70 = mph_io_Write(ctx, NULL, _arg_68, _arg_69); mph_gc_pop_roots(ctx, 2); _ret_70; });
}
	} else {
{
	({ mph_io_File* _arg_71 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_71); MorphString* _arg_72 = mph_string_new(ctx, "â Array access []int[int] -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_72); mph_int _ret_73 = mph_io_Write(ctx, NULL, _arg_71, _arg_72); mph_gc_pop_roots(ctx, 2); _ret_73; });
}
	}
	mph_int map_access_result = ({ mph_GenericType* _arg_74 = string_int_map; mph_gc_push_root(ctx, (void**)&_arg_74); mph_int _ret_75 = mph_check_map_access(ctx, NULL, _arg_74, mph_KIND_STRING); mph_gc_pop_roots(ctx, 1); _ret_75; });
	if ((map_access_result == mph_KIND_INT)) {
{
	({ mph_io_File* _arg_76 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_76); MorphString* _arg_77 = mph_string_new(ctx, "â Map access map[string]int[string] -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_77); mph_int _ret_78 = mph_io_Write(ctx, NULL, _arg_76, _arg_77); mph_gc_pop_roots(ctx, 2); _ret_78; });
}
	} else {
{
	({ mph_io_File* _arg_79 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_79); MorphString* _arg_80 = mph_string_new(ctx, "â Map access map[string]int[string] -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_80); mph_int _ret_81 = mph_io_Write(ctx, NULL, _arg_79, _arg_80); mph_gc_pop_roots(ctx, 2); _ret_81; });
}
	}
	mph_int len_result = ({ mph_GenericType* _arg_82 = int_array; mph_gc_push_root(ctx, (void**)&_arg_82); mph_int _ret_83 = mph_check_len_function(ctx, NULL, _arg_82); mph_gc_pop_roots(ctx, 1); _ret_83; });
	if ((len_result == mph_KIND_INT)) {
{
	({ mph_io_File* _arg_84 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_84); MorphString* _arg_85 = mph_string_new(ctx, "â len([]int) -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_85); mph_int _ret_86 = mph_io_Write(ctx, NULL, _arg_84, _arg_85); mph_gc_pop_roots(ctx, 2); _ret_86; });
}
	} else {
{
	({ mph_io_File* _arg_87 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_87); MorphString* _arg_88 = mph_string_new(ctx, "â len([]int) -> int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_88); mph_int _ret_89 = mph_io_Write(ctx, NULL, _arg_87, _arg_88); mph_gc_pop_roots(ctx, 2); _ret_89; });
}
	}
	mph_GenericType* append_result = ({ mph_GenericType* _arg_90 = int_array; mph_gc_push_root(ctx, (void**)&_arg_90); mph_GenericType* _ret_91 = mph_check_append_function(ctx, NULL, _arg_90, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_91; });
	mph_gc_push_root(ctx, (void**)&append_result);
	if ((({ mph_GenericType* _obj_92 = append_result; mph_gc_push_root(ctx, (void**)&_obj_92); mph_int _ret_93 = _obj_92->base_kind; mph_gc_pop_roots(ctx, 1); _ret_93; }) == mph_KIND_ARRAY)) {
{
	({ mph_io_File* _arg_94 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_94); MorphString* _arg_95 = mph_string_new(ctx, "â append([]int, int) -> []int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_95); mph_int _ret_96 = mph_io_Write(ctx, NULL, _arg_94, _arg_95); mph_gc_pop_roots(ctx, 2); _ret_96; });
}
	} else {
{
	({ mph_io_File* _arg_97 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_97); MorphString* _arg_98 = mph_string_new(ctx, "â append([]int, int) -> []int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_98); mph_int _ret_99 = mph_io_Write(ctx, NULL, _arg_97, _arg_98); mph_gc_pop_roots(ctx, 2); _ret_99; });
}
	}
	mph_GenericType* int_array2 = mph_make_array_type(ctx, NULL, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&int_array2);
	if (({ mph_GenericType* _arg_100 = int_array; mph_gc_push_root(ctx, (void**)&_arg_100); mph_GenericType* _arg_101 = int_array2; mph_gc_push_root(ctx, (void**)&_arg_101); mph_bool _ret_102 = mph_types_compatible(ctx, NULL, _arg_100, _arg_101); mph_gc_pop_roots(ctx, 2); _ret_102; })) {
{
	({ mph_io_File* _arg_103 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_103); MorphString* _arg_104 = mph_string_new(ctx, "â Type compatibility []int == []int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_104); mph_int _ret_105 = mph_io_Write(ctx, NULL, _arg_103, _arg_104); mph_gc_pop_roots(ctx, 2); _ret_105; });
}
	} else {
{
	({ mph_io_File* _arg_106 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_106); MorphString* _arg_107 = mph_string_new(ctx, "â Type compatibility []int == []int: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_107); mph_int _ret_108 = mph_io_Write(ctx, NULL, _arg_106, _arg_107); mph_gc_pop_roots(ctx, 2); _ret_108; });
}
	}
	mph_GenericType* string_array = mph_make_array_type(ctx, NULL, mph_KIND_STRING);
	mph_gc_push_root(ctx, (void**)&string_array);
	if ((({ mph_GenericType* _arg_109 = int_array; mph_gc_push_root(ctx, (void**)&_arg_109); mph_GenericType* _arg_110 = string_array; mph_gc_push_root(ctx, (void**)&_arg_110); mph_bool _ret_111 = mph_types_compatible(ctx, NULL, _arg_109, _arg_110); mph_gc_pop_roots(ctx, 2); _ret_111; }) == 0)) {
{
	({ mph_io_File* _arg_112 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_112); MorphString* _arg_113 = mph_string_new(ctx, "â Type incompatibility []int != []string: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_113); mph_int _ret_114 = mph_io_Write(ctx, NULL, _arg_112, _arg_113); mph_gc_pop_roots(ctx, 2); _ret_114; });
}
	} else {
{
	({ mph_io_File* _arg_115 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_115); MorphString* _arg_116 = mph_string_new(ctx, "â Type incompatibility []int != []string: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_116); mph_int _ret_117 = mph_io_Write(ctx, NULL, _arg_115, _arg_116); mph_gc_pop_roots(ctx, 2); _ret_117; });
}
	}
	mph_int wrong_key_access = ({ mph_GenericType* _arg_118 = string_int_map; mph_gc_push_root(ctx, (void**)&_arg_118); mph_int _ret_119 = mph_check_map_access(ctx, NULL, _arg_118, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_119; });
	if ((wrong_key_access == mph_KIND_ERROR)) {
{
	({ mph_io_File* _arg_120 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_120); MorphString* _arg_121 = mph_string_new(ctx, "â Wrong key type error detected: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_121); mph_int _ret_122 = mph_io_Write(ctx, NULL, _arg_120, _arg_121); mph_gc_pop_roots(ctx, 2); _ret_122; });
}
	} else {
{
	({ mph_io_File* _arg_123 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_123); MorphString* _arg_124 = mph_string_new(ctx, "â Wrong key type error not detected: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_124); mph_int _ret_125 = mph_io_Write(ctx, NULL, _arg_123, _arg_124); mph_gc_pop_roots(ctx, 2); _ret_125; });
}
	}
	({ mph_io_File* _arg_126 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_126); MorphString* _arg_127 = mph_string_new(ctx, "\nð GENERIC TYPES COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_127); mph_int _ret_128 = mph_io_Write(ctx, NULL, _arg_126, _arg_127); mph_gc_pop_roots(ctx, 2); _ret_128; });
	({ mph_io_File* _arg_129 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_129); MorphString* _arg_130 = mph_string_new(ctx, "â Generic array types working\n"); mph_gc_push_root(ctx, (void**)&_arg_130); mph_int _ret_131 = mph_io_Write(ctx, NULL, _arg_129, _arg_130); mph_gc_pop_roots(ctx, 2); _ret_131; });
	({ mph_io_File* _arg_132 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_132); MorphString* _arg_133 = mph_string_new(ctx, "â Generic map types working\n"); mph_gc_push_root(ctx, (void**)&_arg_133); mph_int _ret_134 = mph_io_Write(ctx, NULL, _arg_132, _arg_133); mph_gc_pop_roots(ctx, 2); _ret_134; });
	({ mph_io_File* _arg_135 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_135); MorphString* _arg_136 = mph_string_new(ctx, "â Generic function checking\n"); mph_gc_push_root(ctx, (void**)&_arg_136); mph_int _ret_137 = mph_io_Write(ctx, NULL, _arg_135, _arg_136); mph_gc_pop_roots(ctx, 2); _ret_137; });
	({ mph_io_File* _arg_138 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_138); MorphString* _arg_139 = mph_string_new(ctx, "â Type compatibility checking\n"); mph_gc_push_root(ctx, (void**)&_arg_139); mph_int _ret_140 = mph_io_Write(ctx, NULL, _arg_138, _arg_139); mph_gc_pop_roots(ctx, 2); _ret_140; });
	({ mph_io_File* _arg_141 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_141); MorphString* _arg_142 = mph_string_new(ctx, "â Error detection functional\n"); mph_gc_push_root(ctx, (void**)&_arg_142); mph_int _ret_143 = mph_io_Write(ctx, NULL, _arg_141, _arg_142); mph_gc_pop_roots(ctx, 2); _ret_143; });
	mph_gc_pop_roots(ctx, 5);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_KIND_INT = 0;
	mph_KIND_STRING = 1;
	mph_KIND_BOOL = 2;
	mph_KIND_ARRAY = 4;
	mph_KIND_MAP = 5;
	mph_KIND_GENERIC = 6;
	mph_KIND_ERROR = (-1);
	mph_main(ctx, NULL);
}
