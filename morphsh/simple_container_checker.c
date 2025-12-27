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

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
mph_int mph_validate_array_access(MorphContext* ctx, void* _env_void, mph_int array_kind, mph_int index_kind);
mph_int mph_validate_map_access(MorphContext* ctx, void* _env_void, mph_int map_kind, mph_int key_kind, mph_int expected_key_kind);
mph_bool mph_validate_array_assignment(MorphContext* ctx, void* _env_void, mph_int array_kind, mph_int index_kind, mph_int value_kind, mph_int expected_element_kind);
mph_int mph_validate_len_call(MorphContext* ctx, void* _env_void, mph_int container_kind);
mph_int mph_validate_array_concat(MorphContext* ctx, void* _env_void, mph_int left_kind, mph_int right_kind, mph_int left_element, mph_int right_element);
mph_int mph_validate_map_key_check(MorphContext* ctx, void* _env_void, mph_int map_kind, mph_int key_kind, mph_int expected_key_kind);
mph_int mph_validate_range_iteration(MorphContext* ctx, void* _env_void, mph_int container_kind);
void mph_test_container_validation(MorphContext* ctx, void* _env_void);
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

mph_int mph_validate_array_access(MorphContext* ctx, void* _env_void, mph_int array_kind, mph_int index_kind) {
	if ((array_kind == 5)) {
{
	if ((index_kind == 0)) {
{
	return 0;
}
	}
}
	}
	return 8;
}

mph_int mph_validate_map_access(MorphContext* ctx, void* _env_void, mph_int map_kind, mph_int key_kind, mph_int expected_key_kind) {
	if ((map_kind == 6)) {
{
	if ((key_kind == expected_key_kind)) {
{
	return 0;
}
	}
}
	}
	return 8;
}

mph_bool mph_validate_array_assignment(MorphContext* ctx, void* _env_void, mph_int array_kind, mph_int index_kind, mph_int value_kind, mph_int expected_element_kind) {
	mph_int access_result = mph_validate_array_access(ctx, NULL, array_kind, index_kind);
	if ((access_result == 8)) {
{
	return 0;
}
	}
	return (value_kind == expected_element_kind);
}

mph_int mph_validate_len_call(MorphContext* ctx, void* _env_void, mph_int container_kind) {
	if ((container_kind == 5)) {
{
	return 0;
}
	}
	if ((container_kind == 2)) {
{
	return 0;
}
	}
	if ((container_kind == 6)) {
{
	return 0;
}
	}
	return 8;
}

mph_int mph_validate_array_concat(MorphContext* ctx, void* _env_void, mph_int left_kind, mph_int right_kind, mph_int left_element, mph_int right_element) {
	if ((left_kind != 5)) {
{
	return 8;
}
	}
	if ((right_kind != 5)) {
{
	return 8;
}
	}
	if ((left_element != right_element)) {
{
	return 8;
}
	}
	return 5;
}

mph_int mph_validate_map_key_check(MorphContext* ctx, void* _env_void, mph_int map_kind, mph_int key_kind, mph_int expected_key_kind) {
	if ((map_kind == 6)) {
{
	if ((key_kind == expected_key_kind)) {
{
	return 3;
}
	}
}
	}
	return 8;
}

mph_int mph_validate_range_iteration(MorphContext* ctx, void* _env_void, mph_int container_kind) {
	if ((container_kind == 5)) {
{
	return 0;
}
	}
	if ((container_kind == 2)) {
{
	return 0;
}
	}
	if ((container_kind == 6)) {
{
	return 2;
}
	}
	return 8;
}

void mph_test_container_validation(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_1 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_1); MorphString* _arg_2 = mph_string_new(ctx, "=== CONTAINER VALIDATION TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_2); mph_int _ret_3 = mph_io_Write(ctx, NULL, _arg_1, _arg_2); mph_gc_pop_roots(ctx, 2); _ret_3; });
	mph_int INT_TYPE = 0;
	mph_int STRING_TYPE = 2;
	mph_int ARRAY_TYPE = 5;
	mph_int MAP_TYPE = 6;
	mph_int ERROR_TYPE = 8;
	mph_int array_access = mph_validate_array_access(ctx, NULL, ARRAY_TYPE, INT_TYPE);
	if ((array_access == INT_TYPE)) {
{
	({ mph_io_File* _arg_4 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_4); MorphString* _arg_5 = mph_string_new(ctx, "[]int[int] â int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_int _ret_6 = mph_io_Write(ctx, NULL, _arg_4, _arg_5); mph_gc_pop_roots(ctx, 2); _ret_6; });
}
	} else {
{
	({ mph_io_File* _arg_7 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_7); MorphString* _arg_8 = mph_string_new(ctx, "[]int[int]: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_8); mph_int _ret_9 = mph_io_Write(ctx, NULL, _arg_7, _arg_8); mph_gc_pop_roots(ctx, 2); _ret_9; });
}
	}
	mph_int bad_array_access = mph_validate_array_access(ctx, NULL, ARRAY_TYPE, STRING_TYPE);
	if ((bad_array_access == ERROR_TYPE)) {
{
	({ mph_io_File* _arg_10 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_10); MorphString* _arg_11 = mph_string_new(ctx, "[]int[string] â error: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_11); mph_int _ret_12 = mph_io_Write(ctx, NULL, _arg_10, _arg_11); mph_gc_pop_roots(ctx, 2); _ret_12; });
}
	} else {
{
	({ mph_io_File* _arg_13 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_13); MorphString* _arg_14 = mph_string_new(ctx, "[]int[string]: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_14); mph_int _ret_15 = mph_io_Write(ctx, NULL, _arg_13, _arg_14); mph_gc_pop_roots(ctx, 2); _ret_15; });
}
	}
	mph_int map_access = mph_validate_map_access(ctx, NULL, MAP_TYPE, STRING_TYPE, STRING_TYPE);
	if ((map_access == INT_TYPE)) {
{
	({ mph_io_File* _arg_16 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_16); MorphString* _arg_17 = mph_string_new(ctx, "map[string]int[string] â int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_17); mph_int _ret_18 = mph_io_Write(ctx, NULL, _arg_16, _arg_17); mph_gc_pop_roots(ctx, 2); _ret_18; });
}
	} else {
{
	({ mph_io_File* _arg_19 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_19); MorphString* _arg_20 = mph_string_new(ctx, "map[string]int[string]: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_20); mph_int _ret_21 = mph_io_Write(ctx, NULL, _arg_19, _arg_20); mph_gc_pop_roots(ctx, 2); _ret_21; });
}
	}
	mph_int bad_map_access = mph_validate_map_access(ctx, NULL, MAP_TYPE, INT_TYPE, STRING_TYPE);
	if ((bad_map_access == ERROR_TYPE)) {
{
	({ mph_io_File* _arg_22 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_22); MorphString* _arg_23 = mph_string_new(ctx, "map[string]int[int] â error: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_23); mph_int _ret_24 = mph_io_Write(ctx, NULL, _arg_22, _arg_23); mph_gc_pop_roots(ctx, 2); _ret_24; });
}
	} else {
{
	({ mph_io_File* _arg_25 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_25); MorphString* _arg_26 = mph_string_new(ctx, "map[string]int[int]: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_26); mph_int _ret_27 = mph_io_Write(ctx, NULL, _arg_25, _arg_26); mph_gc_pop_roots(ctx, 2); _ret_27; });
}
	}
	mph_bool assign_ok = mph_validate_array_assignment(ctx, NULL, ARRAY_TYPE, INT_TYPE, INT_TYPE, INT_TYPE);
	if (assign_ok) {
{
	({ mph_io_File* _arg_28 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_28); MorphString* _arg_29 = mph_string_new(ctx, "[]int[int] = int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_29); mph_int _ret_30 = mph_io_Write(ctx, NULL, _arg_28, _arg_29); mph_gc_pop_roots(ctx, 2); _ret_30; });
}
	} else {
{
	({ mph_io_File* _arg_31 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_31); MorphString* _arg_32 = mph_string_new(ctx, "[]int[int] = int: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_32); mph_int _ret_33 = mph_io_Write(ctx, NULL, _arg_31, _arg_32); mph_gc_pop_roots(ctx, 2); _ret_33; });
}
	}
	mph_bool assign_bad = mph_validate_array_assignment(ctx, NULL, ARRAY_TYPE, INT_TYPE, STRING_TYPE, INT_TYPE);
	if ((!assign_bad)) {
{
	({ mph_io_File* _arg_34 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_34); MorphString* _arg_35 = mph_string_new(ctx, "[]int[int] = string (rejected): â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_35); mph_int _ret_36 = mph_io_Write(ctx, NULL, _arg_34, _arg_35); mph_gc_pop_roots(ctx, 2); _ret_36; });
}
	} else {
{
	({ mph_io_File* _arg_37 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_37); MorphString* _arg_38 = mph_string_new(ctx, "[]int[int] = string: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_38); mph_int _ret_39 = mph_io_Write(ctx, NULL, _arg_37, _arg_38); mph_gc_pop_roots(ctx, 2); _ret_39; });
}
	}
	mph_int len_array = mph_validate_len_call(ctx, NULL, ARRAY_TYPE);
	if ((len_array == INT_TYPE)) {
{
	({ mph_io_File* _arg_40 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_40); MorphString* _arg_41 = mph_string_new(ctx, "len([]int) â int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_41); mph_int _ret_42 = mph_io_Write(ctx, NULL, _arg_40, _arg_41); mph_gc_pop_roots(ctx, 2); _ret_42; });
}
	} else {
{
	({ mph_io_File* _arg_43 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_43); MorphString* _arg_44 = mph_string_new(ctx, "len([]int): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_44); mph_int _ret_45 = mph_io_Write(ctx, NULL, _arg_43, _arg_44); mph_gc_pop_roots(ctx, 2); _ret_45; });
}
	}
	mph_int len_string = mph_validate_len_call(ctx, NULL, STRING_TYPE);
	if ((len_string == INT_TYPE)) {
{
	({ mph_io_File* _arg_46 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_46); MorphString* _arg_47 = mph_string_new(ctx, "len(string) â int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_47); mph_int _ret_48 = mph_io_Write(ctx, NULL, _arg_46, _arg_47); mph_gc_pop_roots(ctx, 2); _ret_48; });
}
	} else {
{
	({ mph_io_File* _arg_49 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_49); MorphString* _arg_50 = mph_string_new(ctx, "len(string): â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_50); mph_int _ret_51 = mph_io_Write(ctx, NULL, _arg_49, _arg_50); mph_gc_pop_roots(ctx, 2); _ret_51; });
}
	}
	mph_int concat_result = mph_validate_array_concat(ctx, NULL, ARRAY_TYPE, ARRAY_TYPE, INT_TYPE, INT_TYPE);
	if ((concat_result == ARRAY_TYPE)) {
{
	({ mph_io_File* _arg_52 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_52); MorphString* _arg_53 = mph_string_new(ctx, "[]int + []int â []int: â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_53); mph_int _ret_54 = mph_io_Write(ctx, NULL, _arg_52, _arg_53); mph_gc_pop_roots(ctx, 2); _ret_54; });
}
	} else {
{
	({ mph_io_File* _arg_55 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_55); MorphString* _arg_56 = mph_string_new(ctx, "[]int + []int: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_56); mph_int _ret_57 = mph_io_Write(ctx, NULL, _arg_55, _arg_56); mph_gc_pop_roots(ctx, 2); _ret_57; });
}
	}
	mph_int range_result = mph_validate_range_iteration(ctx, NULL, ARRAY_TYPE);
	if ((range_result == INT_TYPE)) {
{
	({ mph_io_File* _arg_58 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_58); MorphString* _arg_59 = mph_string_new(ctx, "range []int â (int, element): â PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_59); mph_int _ret_60 = mph_io_Write(ctx, NULL, _arg_58, _arg_59); mph_gc_pop_roots(ctx, 2); _ret_60; });
}
	} else {
{
	({ mph_io_File* _arg_61 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_61); MorphString* _arg_62 = mph_string_new(ctx, "range []int: â FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_62); mph_int _ret_63 = mph_io_Write(ctx, NULL, _arg_61, _arg_62); mph_gc_pop_roots(ctx, 2); _ret_63; });
}
	}
	({ mph_io_File* _arg_64 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_64); MorphString* _arg_65 = mph_string_new(ctx, "\nð CONTAINER VALIDATION COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_65); mph_int _ret_66 = mph_io_Write(ctx, NULL, _arg_64, _arg_65); mph_gc_pop_roots(ctx, 2); _ret_66; });
	({ mph_io_File* _arg_67 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_67); MorphString* _arg_68 = mph_string_new(ctx, "â Go-consistent array semantics\n"); mph_gc_push_root(ctx, (void**)&_arg_68); mph_int _ret_69 = mph_io_Write(ctx, NULL, _arg_67, _arg_68); mph_gc_pop_roots(ctx, 2); _ret_69; });
	({ mph_io_File* _arg_70 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_70); MorphString* _arg_71 = mph_string_new(ctx, "â Go-consistent map semantics\n"); mph_gc_push_root(ctx, (void**)&_arg_71); mph_int _ret_72 = mph_io_Write(ctx, NULL, _arg_70, _arg_71); mph_gc_pop_roots(ctx, 2); _ret_72; });
	({ mph_io_File* _arg_73 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_73); MorphString* _arg_74 = mph_string_new(ctx, "â Strict type validation\n"); mph_gc_push_root(ctx, (void**)&_arg_74); mph_int _ret_75 = mph_io_Write(ctx, NULL, _arg_73, _arg_74); mph_gc_pop_roots(ctx, 2); _ret_75; });
	({ mph_io_File* _arg_76 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_76); MorphString* _arg_77 = mph_string_new(ctx, "â Built-in function checking\n"); mph_gc_push_root(ctx, (void**)&_arg_77); mph_int _ret_78 = mph_io_Write(ctx, NULL, _arg_76, _arg_77); mph_gc_pop_roots(ctx, 2); _ret_78; });
	({ mph_io_File* _arg_79 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_79); MorphString* _arg_80 = mph_string_new(ctx, "â Container operations validation\n"); mph_gc_push_root(ctx, (void**)&_arg_80); mph_int _ret_81 = mph_io_Write(ctx, NULL, _arg_79, _arg_80); mph_gc_pop_roots(ctx, 2); _ret_81; });
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_test_container_validation(ctx, NULL);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
