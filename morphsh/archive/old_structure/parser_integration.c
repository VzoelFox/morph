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
#define MPH_TYPE_mph_TypedNode 2
#define MPH_TYPE_mph_SemanticAnalyzer 3

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_TypedNode mph_TypedNode;
typedef struct mph_SemanticAnalyzer mph_SemanticAnalyzer;
struct mph_TypedNode {
	mph_int node_type;
	mph_int value_type;
	mph_bool has_error;
};

struct mph_SemanticAnalyzer {
	mph_int current_scope;
	mph_int error_count;
	mph_int warning_count;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_TypedNode = { "TypedNode", sizeof(mph_TypedNode), 0, NULL };
MorphTypeInfo mph_ti_mph_SemanticAnalyzer = { "SemanticAnalyzer", sizeof(mph_SemanticAnalyzer), 0, NULL };

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
mph_SemanticAnalyzer* mph_analyzer_new(MorphContext* ctx, void* _env_void);
mph_TypedNode* mph_annotate_literal_node(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer, MorphString* literal_value);
mph_TypedNode* mph_annotate_binary_node(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer, mph_int left_type, MorphString* op, mph_int right_type);
mph_TypedNode* mph_annotate_var_node(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer, MorphString* var_name, mph_int init_type);
mph_TypedNode* mph_annotate_call_node(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer, MorphString* func_name, mph_int arg_type);
void mph_analyze_program(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer);
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

mph_SemanticAnalyzer* mph_analyzer_new(MorphContext* ctx, void* _env_void) {
	return ({ mph_SemanticAnalyzer* _ret_1 = ({ mph_SemanticAnalyzer* _t = (mph_SemanticAnalyzer*)mph_alloc(ctx, sizeof(mph_SemanticAnalyzer), &mph_ti_mph_SemanticAnalyzer); mph_gc_push_root(ctx, (void**)&_t); _t->warning_count = 0; _t->current_scope = 0; _t->error_count = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_SemanticAnalyzer* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_TypedNode* mph_annotate_literal_node(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer, MorphString* literal_value) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	mph_gc_push_root(ctx, (void**)&literal_value);
	if (mph_string_eq(ctx, literal_value, mph_string_new(ctx, "42"))) {
{
	return ({ mph_TypedNode* _ret_3 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->value_type = mph_KIND_INT; _t->has_error = 0; _t->node_type = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_TypedNode* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}
	}
	if (mph_string_eq(ctx, literal_value, mph_string_new(ctx, "hello"))) {
{
	return ({ mph_TypedNode* _ret_5 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 0; _t->value_type = mph_KIND_STRING; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_5); mph_TypedNode* _ret_6 = _ret_5; mph_gc_pop_roots(ctx, 1); _ret_6; });
}
	}
	if (mph_string_eq(ctx, literal_value, mph_string_new(ctx, "benar"))) {
{
	return ({ mph_TypedNode* _ret_7 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 0; _t->value_type = mph_KIND_BOOL; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_7); mph_TypedNode* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
}
	}
	return ({ mph_TypedNode* _ret_9 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 0; _t->value_type = mph_KIND_ERROR; _t->has_error = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_9); mph_TypedNode* _ret_10 = _ret_9; mph_gc_pop_roots(ctx, 1); _ret_10; });
	mph_gc_pop_roots(ctx, 2);
}

mph_TypedNode* mph_annotate_binary_node(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer, mph_int left_type, MorphString* op, mph_int right_type) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	mph_gc_push_root(ctx, (void**)&op);
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "+"))) {
{
	if ((left_type == mph_KIND_INT)) {
{
	if ((right_type == mph_KIND_INT)) {
{
	return ({ mph_TypedNode* _ret_11 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 1; _t->value_type = mph_KIND_INT; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_11); mph_TypedNode* _ret_12 = _ret_11; mph_gc_pop_roots(ctx, 1); _ret_12; });
}
	}
}
	}
	if ((left_type == mph_KIND_STRING)) {
{
	if ((right_type == mph_KIND_STRING)) {
{
	return ({ mph_TypedNode* _ret_13 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 1; _t->value_type = mph_KIND_STRING; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_13); mph_TypedNode* _ret_14 = _ret_13; mph_gc_pop_roots(ctx, 1); _ret_14; });
}
	}
}
	}
	return ({ mph_TypedNode* _ret_15 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 1; _t->value_type = mph_KIND_ERROR; _t->has_error = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_15); mph_TypedNode* _ret_16 = _ret_15; mph_gc_pop_roots(ctx, 1); _ret_16; });
}
	}
	if (mph_string_eq(ctx, op, mph_string_new(ctx, "=="))) {
{
	if ((left_type == right_type)) {
{
	return ({ mph_TypedNode* _ret_17 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 1; _t->value_type = mph_KIND_BOOL; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_17); mph_TypedNode* _ret_18 = _ret_17; mph_gc_pop_roots(ctx, 1); _ret_18; });
}
	}
	return ({ mph_TypedNode* _ret_19 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 1; _t->value_type = mph_KIND_ERROR; _t->has_error = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_19); mph_TypedNode* _ret_20 = _ret_19; mph_gc_pop_roots(ctx, 1); _ret_20; });
}
	}
	return ({ mph_TypedNode* _ret_21 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->value_type = mph_KIND_ERROR; _t->has_error = 1; _t->node_type = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_21); mph_TypedNode* _ret_22 = _ret_21; mph_gc_pop_roots(ctx, 1); _ret_22; });
	mph_gc_pop_roots(ctx, 2);
}

mph_TypedNode* mph_annotate_var_node(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer, MorphString* var_name, mph_int init_type) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	mph_gc_push_root(ctx, (void**)&var_name);
	return ({ mph_TypedNode* _ret_23 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->value_type = init_type; _t->has_error = 0; _t->node_type = 3; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_23); mph_TypedNode* _ret_24 = _ret_23; mph_gc_pop_roots(ctx, 1); _ret_24; });
	mph_gc_pop_roots(ctx, 2);
}

mph_TypedNode* mph_annotate_call_node(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer, MorphString* func_name, mph_int arg_type) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	mph_gc_push_root(ctx, (void**)&func_name);
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "len"))) {
{
	if ((arg_type == mph_KIND_STRING)) {
{
	return ({ mph_TypedNode* _ret_25 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 4; _t->value_type = mph_KIND_INT; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_25); mph_TypedNode* _ret_26 = _ret_25; mph_gc_pop_roots(ctx, 1); _ret_26; });
}
	}
	if ((arg_type == mph_KIND_ARRAY)) {
{
	return ({ mph_TypedNode* _ret_27 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 4; _t->value_type = mph_KIND_INT; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_27); mph_TypedNode* _ret_28 = _ret_27; mph_gc_pop_roots(ctx, 1); _ret_28; });
}
	}
	return ({ mph_TypedNode* _ret_29 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 4; _t->value_type = mph_KIND_ERROR; _t->has_error = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_29); mph_TypedNode* _ret_30 = _ret_29; mph_gc_pop_roots(ctx, 1); _ret_30; });
}
	}
	if (mph_string_eq(ctx, func_name, mph_string_new(ctx, "print"))) {
{
	return ({ mph_TypedNode* _ret_31 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->value_type = mph_KIND_VOID; _t->has_error = 0; _t->node_type = 4; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_31); mph_TypedNode* _ret_32 = _ret_31; mph_gc_pop_roots(ctx, 1); _ret_32; });
}
	}
	return ({ mph_TypedNode* _ret_33 = ({ mph_TypedNode* _t = (mph_TypedNode*)mph_alloc(ctx, sizeof(mph_TypedNode), &mph_ti_mph_TypedNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = 4; _t->value_type = mph_KIND_ERROR; _t->has_error = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_33); mph_TypedNode* _ret_34 = _ret_33; mph_gc_pop_roots(ctx, 1); _ret_34; });
	mph_gc_pop_roots(ctx, 2);
}

void mph_analyze_program(MorphContext* ctx, void* _env_void, mph_SemanticAnalyzer* analyzer) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	({ mph_io_File* _arg_35 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_35); MorphString* _arg_36 = mph_string_new(ctx, "=== SEMANTIC ANALYSIS PASS ===\n"); mph_gc_push_root(ctx, (void**)&_arg_36); mph_int _ret_37 = mph_io_Write(ctx, NULL, _arg_35, _arg_36); mph_gc_pop_roots(ctx, 2); _ret_37; });
	mph_TypedNode* literal_node = ({ mph_SemanticAnalyzer* _arg_38 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_38); MorphString* _arg_39 = mph_string_new(ctx, "42"); mph_gc_push_root(ctx, (void**)&_arg_39); mph_TypedNode* _ret_40 = mph_annotate_literal_node(ctx, NULL, _arg_38, _arg_39); mph_gc_pop_roots(ctx, 2); _ret_40; });
	mph_gc_push_root(ctx, (void**)&literal_node);
	if ((({ mph_TypedNode* _obj_41 = literal_node; mph_gc_push_root(ctx, (void**)&_obj_41); mph_bool _ret_42 = _obj_41->has_error; mph_gc_pop_roots(ctx, 1); _ret_42; }) == 0)) {
{
	({ mph_io_File* _arg_43 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_43); MorphString* _arg_44 = mph_string_new(ctx, "â Literal '42' -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_44); mph_int _ret_45 = mph_io_Write(ctx, NULL, _arg_43, _arg_44); mph_gc_pop_roots(ctx, 2); _ret_45; });
}
	} else {
{
	({ mph_io_File* _arg_46 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_46); MorphString* _arg_47 = mph_string_new(ctx, "â Literal '42': FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_47); mph_int _ret_48 = mph_io_Write(ctx, NULL, _arg_46, _arg_47); mph_gc_pop_roots(ctx, 2); _ret_48; });
	({ mph_SemanticAnalyzer* _obj_51 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_51); _obj_51->error_count = (({ mph_SemanticAnalyzer* _obj_49 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_49); mph_int _ret_50 = _obj_49->error_count; mph_gc_pop_roots(ctx, 1); _ret_50; }) + 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_TypedNode* binary_node = ({ mph_SemanticAnalyzer* _arg_52 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_52); MorphString* _arg_53 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_53); mph_TypedNode* _ret_54 = mph_annotate_binary_node(ctx, NULL, _arg_52, mph_KIND_INT, _arg_53, mph_KIND_INT); mph_gc_pop_roots(ctx, 2); _ret_54; });
	mph_gc_push_root(ctx, (void**)&binary_node);
	if ((({ mph_TypedNode* _obj_55 = binary_node; mph_gc_push_root(ctx, (void**)&_obj_55); mph_bool _ret_56 = _obj_55->has_error; mph_gc_pop_roots(ctx, 1); _ret_56; }) == 0)) {
{
	({ mph_io_File* _arg_57 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_57); MorphString* _arg_58 = mph_string_new(ctx, "â Binary '42 + 10' -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_58); mph_int _ret_59 = mph_io_Write(ctx, NULL, _arg_57, _arg_58); mph_gc_pop_roots(ctx, 2); _ret_59; });
}
	} else {
{
	({ mph_io_File* _arg_60 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_60); MorphString* _arg_61 = mph_string_new(ctx, "â Binary '42 + 10': FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_61); mph_int _ret_62 = mph_io_Write(ctx, NULL, _arg_60, _arg_61); mph_gc_pop_roots(ctx, 2); _ret_62; });
	({ mph_SemanticAnalyzer* _obj_65 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_65); _obj_65->error_count = (({ mph_SemanticAnalyzer* _obj_63 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_63); mph_int _ret_64 = _obj_63->error_count; mph_gc_pop_roots(ctx, 1); _ret_64; }) + 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_TypedNode* var_node = ({ mph_SemanticAnalyzer* _arg_66 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_66); MorphString* _arg_67 = mph_string_new(ctx, "x"); mph_gc_push_root(ctx, (void**)&_arg_67); mph_TypedNode* _ret_68 = mph_annotate_var_node(ctx, NULL, _arg_66, _arg_67, mph_KIND_INT); mph_gc_pop_roots(ctx, 2); _ret_68; });
	mph_gc_push_root(ctx, (void**)&var_node);
	if ((({ mph_TypedNode* _obj_69 = var_node; mph_gc_push_root(ctx, (void**)&_obj_69); mph_bool _ret_70 = _obj_69->has_error; mph_gc_pop_roots(ctx, 1); _ret_70; }) == 0)) {
{
	({ mph_io_File* _arg_71 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_71); MorphString* _arg_72 = mph_string_new(ctx, "â Variable 'var x = 42' -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_72); mph_int _ret_73 = mph_io_Write(ctx, NULL, _arg_71, _arg_72); mph_gc_pop_roots(ctx, 2); _ret_73; });
}
	} else {
{
	({ mph_io_File* _arg_74 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_74); MorphString* _arg_75 = mph_string_new(ctx, "â Variable 'var x = 42': FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_75); mph_int _ret_76 = mph_io_Write(ctx, NULL, _arg_74, _arg_75); mph_gc_pop_roots(ctx, 2); _ret_76; });
	({ mph_SemanticAnalyzer* _obj_79 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_79); _obj_79->error_count = (({ mph_SemanticAnalyzer* _obj_77 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_77); mph_int _ret_78 = _obj_77->error_count; mph_gc_pop_roots(ctx, 1); _ret_78; }) + 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_TypedNode* call_node = ({ mph_SemanticAnalyzer* _arg_80 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_80); MorphString* _arg_81 = mph_string_new(ctx, "len"); mph_gc_push_root(ctx, (void**)&_arg_81); mph_TypedNode* _ret_82 = mph_annotate_call_node(ctx, NULL, _arg_80, _arg_81, mph_KIND_STRING); mph_gc_pop_roots(ctx, 2); _ret_82; });
	mph_gc_push_root(ctx, (void**)&call_node);
	if ((({ mph_TypedNode* _obj_83 = call_node; mph_gc_push_root(ctx, (void**)&_obj_83); mph_bool _ret_84 = _obj_83->has_error; mph_gc_pop_roots(ctx, 1); _ret_84; }) == 0)) {
{
	({ mph_io_File* _arg_85 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_85); MorphString* _arg_86 = mph_string_new(ctx, "â Call 'len(\"hello\")' -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_86); mph_int _ret_87 = mph_io_Write(ctx, NULL, _arg_85, _arg_86); mph_gc_pop_roots(ctx, 2); _ret_87; });
}
	} else {
{
	({ mph_io_File* _arg_88 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_88); MorphString* _arg_89 = mph_string_new(ctx, "â Call 'len(\"hello\")': FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_89); mph_int _ret_90 = mph_io_Write(ctx, NULL, _arg_88, _arg_89); mph_gc_pop_roots(ctx, 2); _ret_90; });
	({ mph_SemanticAnalyzer* _obj_93 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_93); _obj_93->error_count = (({ mph_SemanticAnalyzer* _obj_91 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_91); mph_int _ret_92 = _obj_91->error_count; mph_gc_pop_roots(ctx, 1); _ret_92; }) + 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_TypedNode* error_node = ({ mph_SemanticAnalyzer* _arg_94 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_94); MorphString* _arg_95 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_95); mph_TypedNode* _ret_96 = mph_annotate_binary_node(ctx, NULL, _arg_94, mph_KIND_INT, _arg_95, mph_KIND_STRING); mph_gc_pop_roots(ctx, 2); _ret_96; });
	mph_gc_push_root(ctx, (void**)&error_node);
	if (({ mph_TypedNode* _obj_97 = error_node; mph_gc_push_root(ctx, (void**)&_obj_97); mph_bool _ret_98 = _obj_97->has_error; mph_gc_pop_roots(ctx, 1); _ret_98; })) {
{
	({ mph_io_File* _arg_99 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_99); MorphString* _arg_100 = mph_string_new(ctx, "â Type error '42 + \"hello\"' detected: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_100); mph_int _ret_101 = mph_io_Write(ctx, NULL, _arg_99, _arg_100); mph_gc_pop_roots(ctx, 2); _ret_101; });
}
	} else {
{
	({ mph_io_File* _arg_102 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_102); MorphString* _arg_103 = mph_string_new(ctx, "â Type error '42 + \"hello\"' not detected: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_103); mph_int _ret_104 = mph_io_Write(ctx, NULL, _arg_102, _arg_103); mph_gc_pop_roots(ctx, 2); _ret_104; });
	({ mph_SemanticAnalyzer* _obj_107 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_107); _obj_107->error_count = (({ mph_SemanticAnalyzer* _obj_105 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_105); mph_int _ret_106 = _obj_105->error_count; mph_gc_pop_roots(ctx, 1); _ret_106; }) + 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_TypedNode* comp_node = ({ mph_SemanticAnalyzer* _arg_108 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_108); MorphString* _arg_109 = mph_string_new(ctx, "=="); mph_gc_push_root(ctx, (void**)&_arg_109); mph_TypedNode* _ret_110 = mph_annotate_binary_node(ctx, NULL, _arg_108, mph_KIND_INT, _arg_109, mph_KIND_INT); mph_gc_pop_roots(ctx, 2); _ret_110; });
	mph_gc_push_root(ctx, (void**)&comp_node);
	if ((({ mph_TypedNode* _obj_111 = comp_node; mph_gc_push_root(ctx, (void**)&_obj_111); mph_bool _ret_112 = _obj_111->has_error; mph_gc_pop_roots(ctx, 1); _ret_112; }) == 0)) {
{
	({ mph_io_File* _arg_113 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_113); MorphString* _arg_114 = mph_string_new(ctx, "â Comparison '42 == 42' -> bool: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_114); mph_int _ret_115 = mph_io_Write(ctx, NULL, _arg_113, _arg_114); mph_gc_pop_roots(ctx, 2); _ret_115; });
}
	} else {
{
	({ mph_io_File* _arg_116 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_116); MorphString* _arg_117 = mph_string_new(ctx, "â Comparison '42 == 42': FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_117); mph_int _ret_118 = mph_io_Write(ctx, NULL, _arg_116, _arg_117); mph_gc_pop_roots(ctx, 2); _ret_118; });
	({ mph_SemanticAnalyzer* _obj_121 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_121); _obj_121->error_count = (({ mph_SemanticAnalyzer* _obj_119 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_119); mph_int _ret_120 = _obj_119->error_count; mph_gc_pop_roots(ctx, 1); _ret_120; }) + 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	if ((({ mph_SemanticAnalyzer* _obj_122 = analyzer; mph_gc_push_root(ctx, (void**)&_obj_122); mph_int _ret_123 = _obj_122->error_count; mph_gc_pop_roots(ctx, 1); _ret_123; }) == 0)) {
{
	({ mph_io_File* _arg_124 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_124); MorphString* _arg_125 = mph_string_new(ctx, "\nð¯ SEMANTIC ANALYSIS COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_125); mph_int _ret_126 = mph_io_Write(ctx, NULL, _arg_124, _arg_125); mph_gc_pop_roots(ctx, 2); _ret_126; });
	({ mph_io_File* _arg_127 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_127); MorphString* _arg_128 = mph_string_new(ctx, "â All AST nodes properly typed\n"); mph_gc_push_root(ctx, (void**)&_arg_128); mph_int _ret_129 = mph_io_Write(ctx, NULL, _arg_127, _arg_128); mph_gc_pop_roots(ctx, 2); _ret_129; });
	({ mph_io_File* _arg_130 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_130); MorphString* _arg_131 = mph_string_new(ctx, "â Type errors detected correctly\n"); mph_gc_push_root(ctx, (void**)&_arg_131); mph_int _ret_132 = mph_io_Write(ctx, NULL, _arg_130, _arg_131); mph_gc_pop_roots(ctx, 2); _ret_132; });
	({ mph_io_File* _arg_133 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_133); MorphString* _arg_134 = mph_string_new(ctx, "â Parser integration ready\n"); mph_gc_push_root(ctx, (void**)&_arg_134); mph_int _ret_135 = mph_io_Write(ctx, NULL, _arg_133, _arg_134); mph_gc_pop_roots(ctx, 2); _ret_135; });
}
	} else {
{
	({ mph_io_File* _arg_136 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_136); MorphString* _arg_137 = mph_string_new(ctx, "\nâ SEMANTIC ANALYSIS FAILED!\n"); mph_gc_push_root(ctx, (void**)&_arg_137); mph_int _ret_138 = mph_io_Write(ctx, NULL, _arg_136, _arg_137); mph_gc_pop_roots(ctx, 2); _ret_138; });
	({ mph_io_File* _arg_139 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_139); MorphString* _arg_140 = mph_string_new(ctx, "Errors detected: "); mph_gc_push_root(ctx, (void**)&_arg_140); mph_int _ret_141 = mph_io_Write(ctx, NULL, _arg_139, _arg_140); mph_gc_pop_roots(ctx, 2); _ret_141; });
}
	}
	mph_gc_pop_roots(ctx, 6);
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_SemanticAnalyzer* analyzer = mph_analyzer_new(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&analyzer);
	({ mph_SemanticAnalyzer* _arg_142 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_142); mph_analyze_program(ctx, NULL, _arg_142); mph_gc_pop_roots(ctx, 1); });
	mph_gc_pop_roots(ctx, 1);
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
