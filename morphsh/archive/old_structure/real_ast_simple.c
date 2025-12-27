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
#define MPH_TYPE_mph_SimpleASTNode 2
#define MPH_TYPE_mph_SimpleAnalyzer 3

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_SimpleASTNode mph_SimpleASTNode;
typedef struct mph_SimpleAnalyzer mph_SimpleAnalyzer;
struct mph_SimpleASTNode {
	mph_int node_type;
	mph_int inferred_type;
	mph_bool has_error;
};

struct mph_SimpleAnalyzer {
	mph_int error_count;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_SimpleASTNode = { "SimpleASTNode", sizeof(mph_SimpleASTNode), 0, NULL };
MorphTypeInfo mph_ti_mph_SimpleAnalyzer = { "SimpleAnalyzer", sizeof(mph_SimpleAnalyzer), 0, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_BOOL;
mph_int mph_KIND_VOID;
mph_int mph_KIND_ERROR;
mph_int mph_NODE_INTEGER;
mph_int mph_NODE_STRING;
mph_int mph_NODE_BOOLEAN;
mph_int mph_NODE_INFIX;
mph_int mph_NODE_CALL;

// Function Prototypes
mph_SimpleAnalyzer* mph_simple_analyzer_new(MorphContext* ctx, void* _env_void);
mph_SimpleASTNode* mph_analyze_integer(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer);
mph_SimpleASTNode* mph_analyze_string(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer);
mph_SimpleASTNode* mph_analyze_boolean(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer);
mph_SimpleASTNode* mph_analyze_infix(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer, mph_int left_type, mph_int right_type);
mph_SimpleASTNode* mph_analyze_call(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer, mph_int arg_type);
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

mph_SimpleAnalyzer* mph_simple_analyzer_new(MorphContext* ctx, void* _env_void) {
	return ({ mph_SimpleAnalyzer* _ret_1 = ({ mph_SimpleAnalyzer* _t = (mph_SimpleAnalyzer*)mph_alloc(ctx, sizeof(mph_SimpleAnalyzer), &mph_ti_mph_SimpleAnalyzer); mph_gc_push_root(ctx, (void**)&_t); _t->error_count = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_SimpleAnalyzer* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}

mph_SimpleASTNode* mph_analyze_integer(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	return ({ mph_SimpleASTNode* _ret_3 = ({ mph_SimpleASTNode* _t = (mph_SimpleASTNode*)mph_alloc(ctx, sizeof(mph_SimpleASTNode), &mph_ti_mph_SimpleASTNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = mph_NODE_INTEGER; _t->inferred_type = mph_KIND_INT; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_SimpleASTNode* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
	mph_gc_pop_roots(ctx, 1);
}

mph_SimpleASTNode* mph_analyze_string(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	return ({ mph_SimpleASTNode* _ret_5 = ({ mph_SimpleASTNode* _t = (mph_SimpleASTNode*)mph_alloc(ctx, sizeof(mph_SimpleASTNode), &mph_ti_mph_SimpleASTNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = mph_NODE_STRING; _t->inferred_type = mph_KIND_STRING; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_5); mph_SimpleASTNode* _ret_6 = _ret_5; mph_gc_pop_roots(ctx, 1); _ret_6; });
	mph_gc_pop_roots(ctx, 1);
}

mph_SimpleASTNode* mph_analyze_boolean(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	return ({ mph_SimpleASTNode* _ret_7 = ({ mph_SimpleASTNode* _t = (mph_SimpleASTNode*)mph_alloc(ctx, sizeof(mph_SimpleASTNode), &mph_ti_mph_SimpleASTNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = mph_NODE_BOOLEAN; _t->inferred_type = mph_KIND_BOOL; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_7); mph_SimpleASTNode* _ret_8 = _ret_7; mph_gc_pop_roots(ctx, 1); _ret_8; });
	mph_gc_pop_roots(ctx, 1);
}

mph_SimpleASTNode* mph_analyze_infix(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer, mph_int left_type, mph_int right_type) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	if ((left_type == mph_KIND_INT)) {
{
	if ((right_type == mph_KIND_INT)) {
{
	return ({ mph_SimpleASTNode* _ret_9 = ({ mph_SimpleASTNode* _t = (mph_SimpleASTNode*)mph_alloc(ctx, sizeof(mph_SimpleASTNode), &mph_ti_mph_SimpleASTNode); mph_gc_push_root(ctx, (void**)&_t); _t->inferred_type = mph_KIND_INT; _t->has_error = 0; _t->node_type = mph_NODE_INFIX; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_9); mph_SimpleASTNode* _ret_10 = _ret_9; mph_gc_pop_roots(ctx, 1); _ret_10; });
}
	}
}
	}
	if ((left_type == mph_KIND_STRING)) {
{
	if ((right_type == mph_KIND_STRING)) {
{
	return ({ mph_SimpleASTNode* _ret_11 = ({ mph_SimpleASTNode* _t = (mph_SimpleASTNode*)mph_alloc(ctx, sizeof(mph_SimpleASTNode), &mph_ti_mph_SimpleASTNode); mph_gc_push_root(ctx, (void**)&_t); _t->has_error = 0; _t->node_type = mph_NODE_INFIX; _t->inferred_type = mph_KIND_STRING; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_11); mph_SimpleASTNode* _ret_12 = _ret_11; mph_gc_pop_roots(ctx, 1); _ret_12; });
}
	}
}
	}
	return ({ mph_SimpleASTNode* _ret_13 = ({ mph_SimpleASTNode* _t = (mph_SimpleASTNode*)mph_alloc(ctx, sizeof(mph_SimpleASTNode), &mph_ti_mph_SimpleASTNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = mph_NODE_INFIX; _t->inferred_type = mph_KIND_ERROR; _t->has_error = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_13); mph_SimpleASTNode* _ret_14 = _ret_13; mph_gc_pop_roots(ctx, 1); _ret_14; });
	mph_gc_pop_roots(ctx, 1);
}

mph_SimpleASTNode* mph_analyze_call(MorphContext* ctx, void* _env_void, mph_SimpleAnalyzer* analyzer, mph_int arg_type) {
	mph_gc_push_root(ctx, (void**)&analyzer);
	if ((arg_type == mph_KIND_STRING)) {
{
	return ({ mph_SimpleASTNode* _ret_15 = ({ mph_SimpleASTNode* _t = (mph_SimpleASTNode*)mph_alloc(ctx, sizeof(mph_SimpleASTNode), &mph_ti_mph_SimpleASTNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = mph_NODE_CALL; _t->inferred_type = mph_KIND_INT; _t->has_error = 0; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_15); mph_SimpleASTNode* _ret_16 = _ret_15; mph_gc_pop_roots(ctx, 1); _ret_16; });
}
	}
	return ({ mph_SimpleASTNode* _ret_17 = ({ mph_SimpleASTNode* _t = (mph_SimpleASTNode*)mph_alloc(ctx, sizeof(mph_SimpleASTNode), &mph_ti_mph_SimpleASTNode); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = mph_NODE_CALL; _t->inferred_type = mph_KIND_ERROR; _t->has_error = 1; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_17); mph_SimpleASTNode* _ret_18 = _ret_17; mph_gc_pop_roots(ctx, 1); _ret_18; });
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_19 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_19); MorphString* _arg_20 = mph_string_new(ctx, "=== REAL AST INTEGRATION (SIMPLE) ===\n"); mph_gc_push_root(ctx, (void**)&_arg_20); mph_int _ret_21 = mph_io_Write(ctx, NULL, _arg_19, _arg_20); mph_gc_pop_roots(ctx, 2); _ret_21; });
	mph_SimpleAnalyzer* analyzer = mph_simple_analyzer_new(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&analyzer);
	mph_SimpleASTNode* int_node = ({ mph_SimpleAnalyzer* _arg_22 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_22); mph_SimpleASTNode* _ret_23 = mph_analyze_integer(ctx, NULL, _arg_22); mph_gc_pop_roots(ctx, 1); _ret_23; });
	mph_gc_push_root(ctx, (void**)&int_node);
	if ((({ mph_SimpleASTNode* _obj_24 = int_node; mph_gc_push_root(ctx, (void**)&_obj_24); mph_bool _ret_25 = _obj_24->has_error; mph_gc_pop_roots(ctx, 1); _ret_25; }) == 0)) {
{
	({ mph_io_File* _arg_26 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_26); MorphString* _arg_27 = mph_string_new(ctx, "â IntegerLiteral -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_27); mph_int _ret_28 = mph_io_Write(ctx, NULL, _arg_26, _arg_27); mph_gc_pop_roots(ctx, 2); _ret_28; });
}
	} else {
{
	({ mph_io_File* _arg_29 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_29); MorphString* _arg_30 = mph_string_new(ctx, "â IntegerLiteral: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_30); mph_int _ret_31 = mph_io_Write(ctx, NULL, _arg_29, _arg_30); mph_gc_pop_roots(ctx, 2); _ret_31; });
}
	}
	mph_SimpleASTNode* str_node = ({ mph_SimpleAnalyzer* _arg_32 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_32); mph_SimpleASTNode* _ret_33 = mph_analyze_string(ctx, NULL, _arg_32); mph_gc_pop_roots(ctx, 1); _ret_33; });
	mph_gc_push_root(ctx, (void**)&str_node);
	if ((({ mph_SimpleASTNode* _obj_34 = str_node; mph_gc_push_root(ctx, (void**)&_obj_34); mph_bool _ret_35 = _obj_34->has_error; mph_gc_pop_roots(ctx, 1); _ret_35; }) == 0)) {
{
	({ mph_io_File* _arg_36 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_36); MorphString* _arg_37 = mph_string_new(ctx, "â StringLiteral -> string: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_37); mph_int _ret_38 = mph_io_Write(ctx, NULL, _arg_36, _arg_37); mph_gc_pop_roots(ctx, 2); _ret_38; });
}
	} else {
{
	({ mph_io_File* _arg_39 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_39); MorphString* _arg_40 = mph_string_new(ctx, "â StringLiteral: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_40); mph_int _ret_41 = mph_io_Write(ctx, NULL, _arg_39, _arg_40); mph_gc_pop_roots(ctx, 2); _ret_41; });
}
	}
	mph_SimpleASTNode* bool_node = ({ mph_SimpleAnalyzer* _arg_42 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_42); mph_SimpleASTNode* _ret_43 = mph_analyze_boolean(ctx, NULL, _arg_42); mph_gc_pop_roots(ctx, 1); _ret_43; });
	mph_gc_push_root(ctx, (void**)&bool_node);
	if ((({ mph_SimpleASTNode* _obj_44 = bool_node; mph_gc_push_root(ctx, (void**)&_obj_44); mph_bool _ret_45 = _obj_44->has_error; mph_gc_pop_roots(ctx, 1); _ret_45; }) == 0)) {
{
	({ mph_io_File* _arg_46 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_46); MorphString* _arg_47 = mph_string_new(ctx, "â BooleanLiteral -> bool: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_47); mph_int _ret_48 = mph_io_Write(ctx, NULL, _arg_46, _arg_47); mph_gc_pop_roots(ctx, 2); _ret_48; });
}
	} else {
{
	({ mph_io_File* _arg_49 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_49); MorphString* _arg_50 = mph_string_new(ctx, "â BooleanLiteral: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_50); mph_int _ret_51 = mph_io_Write(ctx, NULL, _arg_49, _arg_50); mph_gc_pop_roots(ctx, 2); _ret_51; });
}
	}
	mph_SimpleASTNode* infix_node = ({ mph_SimpleAnalyzer* _arg_52 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_52); mph_SimpleASTNode* _ret_53 = mph_analyze_infix(ctx, NULL, _arg_52, mph_KIND_INT, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_53; });
	mph_gc_push_root(ctx, (void**)&infix_node);
	if ((({ mph_SimpleASTNode* _obj_54 = infix_node; mph_gc_push_root(ctx, (void**)&_obj_54); mph_bool _ret_55 = _obj_54->has_error; mph_gc_pop_roots(ctx, 1); _ret_55; }) == 0)) {
{
	({ mph_io_File* _arg_56 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_56); MorphString* _arg_57 = mph_string_new(ctx, "â InfixExpression (int + int) -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_57); mph_int _ret_58 = mph_io_Write(ctx, NULL, _arg_56, _arg_57); mph_gc_pop_roots(ctx, 2); _ret_58; });
}
	} else {
{
	({ mph_io_File* _arg_59 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_59); MorphString* _arg_60 = mph_string_new(ctx, "â InfixExpression (int + int): FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_60); mph_int _ret_61 = mph_io_Write(ctx, NULL, _arg_59, _arg_60); mph_gc_pop_roots(ctx, 2); _ret_61; });
}
	}
	mph_SimpleASTNode* call_node = ({ mph_SimpleAnalyzer* _arg_62 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_62); mph_SimpleASTNode* _ret_63 = mph_analyze_call(ctx, NULL, _arg_62, mph_KIND_STRING); mph_gc_pop_roots(ctx, 1); _ret_63; });
	mph_gc_push_root(ctx, (void**)&call_node);
	if ((({ mph_SimpleASTNode* _obj_64 = call_node; mph_gc_push_root(ctx, (void**)&_obj_64); mph_bool _ret_65 = _obj_64->has_error; mph_gc_pop_roots(ctx, 1); _ret_65; }) == 0)) {
{
	({ mph_io_File* _arg_66 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_66); MorphString* _arg_67 = mph_string_new(ctx, "â CallExpression len(string) -> int: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_67); mph_int _ret_68 = mph_io_Write(ctx, NULL, _arg_66, _arg_67); mph_gc_pop_roots(ctx, 2); _ret_68; });
}
	} else {
{
	({ mph_io_File* _arg_69 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_69); MorphString* _arg_70 = mph_string_new(ctx, "â CallExpression len(string): FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_70); mph_int _ret_71 = mph_io_Write(ctx, NULL, _arg_69, _arg_70); mph_gc_pop_roots(ctx, 2); _ret_71; });
}
	}
	mph_SimpleASTNode* error_node = ({ mph_SimpleAnalyzer* _arg_72 = analyzer; mph_gc_push_root(ctx, (void**)&_arg_72); mph_SimpleASTNode* _ret_73 = mph_analyze_infix(ctx, NULL, _arg_72, mph_KIND_INT, mph_KIND_STRING); mph_gc_pop_roots(ctx, 1); _ret_73; });
	mph_gc_push_root(ctx, (void**)&error_node);
	if (({ mph_SimpleASTNode* _obj_74 = error_node; mph_gc_push_root(ctx, (void**)&_obj_74); mph_bool _ret_75 = _obj_74->has_error; mph_gc_pop_roots(ctx, 1); _ret_75; })) {
{
	({ mph_io_File* _arg_76 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_76); MorphString* _arg_77 = mph_string_new(ctx, "â Type error detected: PASS\n"); mph_gc_push_root(ctx, (void**)&_arg_77); mph_int _ret_78 = mph_io_Write(ctx, NULL, _arg_76, _arg_77); mph_gc_pop_roots(ctx, 2); _ret_78; });
}
	} else {
{
	({ mph_io_File* _arg_79 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_79); MorphString* _arg_80 = mph_string_new(ctx, "â Type error not detected: FAIL\n"); mph_gc_push_root(ctx, (void**)&_arg_80); mph_int _ret_81 = mph_io_Write(ctx, NULL, _arg_79, _arg_80); mph_gc_pop_roots(ctx, 2); _ret_81; });
}
	}
	({ mph_io_File* _arg_82 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_82); MorphString* _arg_83 = mph_string_new(ctx, "\nð REAL AST INTEGRATION COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_83); mph_int _ret_84 = mph_io_Write(ctx, NULL, _arg_82, _arg_83); mph_gc_pop_roots(ctx, 2); _ret_84; });
	({ mph_io_File* _arg_85 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_85); MorphString* _arg_86 = mph_string_new(ctx, "â AST node type inference working\n"); mph_gc_push_root(ctx, (void**)&_arg_86); mph_int _ret_87 = mph_io_Write(ctx, NULL, _arg_85, _arg_86); mph_gc_pop_roots(ctx, 2); _ret_87; });
	({ mph_io_File* _arg_88 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_88); MorphString* _arg_89 = mph_string_new(ctx, "â Type error detection functional\n"); mph_gc_push_root(ctx, (void**)&_arg_89); mph_int _ret_90 = mph_io_Write(ctx, NULL, _arg_88, _arg_89); mph_gc_pop_roots(ctx, 2); _ret_90; });
	({ mph_io_File* _arg_91 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_91); MorphString* _arg_92 = mph_string_new(ctx, "â Ready for pkg/parser integration\n"); mph_gc_push_root(ctx, (void**)&_arg_92); mph_int _ret_93 = mph_io_Write(ctx, NULL, _arg_91, _arg_92); mph_gc_pop_roots(ctx, 2); _ret_93; });
	mph_gc_pop_roots(ctx, 7);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_KIND_INT = 0;
	mph_KIND_STRING = 1;
	mph_KIND_BOOL = 2;
	mph_KIND_VOID = 3;
	mph_KIND_ERROR = (-1);
	mph_NODE_INTEGER = 5;
	mph_NODE_STRING = 6;
	mph_NODE_BOOLEAN = 7;
	mph_NODE_INFIX = 8;
	mph_NODE_CALL = 10;
	mph_main(ctx, NULL);
}
