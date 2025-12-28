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
#define MPH_TYPE_mph_InfixExpression 14
#define MPH_TYPE_mph_MemberExpression 18
#define MPH_TYPE_mph_WhileStatement 21
#define MPH_TYPE_mph_Node 1
#define MPH_TYPE_mph_VarStatement 3
#define MPH_TYPE_mph_FunctionStatement 4
#define MPH_TYPE_mph_BlockStatement 7
#define MPH_TYPE_mph_CallExpression 16
#define MPH_TYPE_mph_IndexExpression 17
#define MPH_TYPE_mph_SwitchStatement 20
#define MPH_TYPE_mph_StructStatement 22
#define MPH_TYPE_mph_Program 2
#define MPH_TYPE_mph_BooleanLiteral 12
#define MPH_TYPE_mph_IfExpression 15
#define MPH_TYPE_mph_CaseClause 19
#define MPH_TYPE_mph_ImportStatement 23
#define MPH_TYPE_mph_ReturnStatement 5
#define MPH_TYPE_mph_ExpressionStatement 6
#define MPH_TYPE_mph_FloatLiteral 10
#define MPH_TYPE_mph_BreakStatement 24
#define MPH_TYPE_mph_ContinueStatement 25
#define MPH_TYPE_mph_Identifier 8
#define MPH_TYPE_mph_IntegerLiteral 9
#define MPH_TYPE_mph_StringLiteral 11
#define MPH_TYPE_mph_PrefixExpression 13

// Struct Definitions (Env & Types)
typedef struct mph_Node mph_Node;
typedef struct mph_Program mph_Program;
typedef struct mph_VarStatement mph_VarStatement;
typedef struct mph_FunctionStatement mph_FunctionStatement;
typedef struct mph_ReturnStatement mph_ReturnStatement;
typedef struct mph_ExpressionStatement mph_ExpressionStatement;
typedef struct mph_BlockStatement mph_BlockStatement;
typedef struct mph_Identifier mph_Identifier;
typedef struct mph_IntegerLiteral mph_IntegerLiteral;
typedef struct mph_FloatLiteral mph_FloatLiteral;
typedef struct mph_StringLiteral mph_StringLiteral;
typedef struct mph_BooleanLiteral mph_BooleanLiteral;
typedef struct mph_PrefixExpression mph_PrefixExpression;
typedef struct mph_InfixExpression mph_InfixExpression;
typedef struct mph_IfExpression mph_IfExpression;
typedef struct mph_CallExpression mph_CallExpression;
typedef struct mph_IndexExpression mph_IndexExpression;
typedef struct mph_MemberExpression mph_MemberExpression;
typedef struct mph_CaseClause mph_CaseClause;
typedef struct mph_SwitchStatement mph_SwitchStatement;
typedef struct mph_WhileStatement mph_WhileStatement;
typedef struct mph_StructStatement mph_StructStatement;
typedef struct mph_ImportStatement mph_ImportStatement;
typedef struct mph_BreakStatement mph_BreakStatement;
typedef struct mph_ContinueStatement mph_ContinueStatement;
struct mph_Node {
	mph_int node_type;
	MorphString* token_literal;
	mph_int line;
	mph_int column;
};

struct mph_Program {
	mph_Node* base;
	mph_int statements_count;
};

struct mph_VarStatement {
	mph_Node* base;
	MorphString* name;
	MorphString* value_type;
	mph_bool has_value;
};

struct mph_FunctionStatement {
	mph_Node* base;
	MorphString* name;
	mph_int parameters_count;
	MorphString* return_type;
};

struct mph_ReturnStatement {
	mph_Node* base;
	mph_bool has_value;
};

struct mph_ExpressionStatement {
	mph_Node* base;
};

struct mph_BlockStatement {
	mph_Node* base;
	mph_int statements_count;
};

struct mph_Identifier {
	mph_Node* base;
	MorphString* value;
};

struct mph_IntegerLiteral {
	mph_Node* base;
	mph_int value;
};

struct mph_FloatLiteral {
	mph_Node* base;
	MorphString* value_str;
};

struct mph_StringLiteral {
	mph_Node* base;
	MorphString* value;
};

struct mph_BooleanLiteral {
	mph_Node* base;
	mph_bool value;
};

struct mph_PrefixExpression {
	mph_Node* base;
	MorphString* operator;
};

struct mph_InfixExpression {
	mph_Node* base;
	MorphString* operator;
};

struct mph_IfExpression {
	mph_Node* base;
	mph_bool has_alternative;
};

struct mph_CallExpression {
	mph_Node* base;
	mph_int arguments_count;
};

struct mph_IndexExpression {
	mph_Node* base;
};

struct mph_MemberExpression {
	mph_Node* base;
	MorphString* member_name;
};

struct mph_CaseClause {
	mph_Node* base;
	mph_int values_count;
	mph_int v0;
	mph_int v1;
	mph_int v2;
	mph_int v3;
	mph_bool has_body;
};

struct mph_SwitchStatement {
	mph_Node* base;
	mph_int cases_count;
	mph_bool has_default;
};

struct mph_WhileStatement {
	mph_Node* base;
	mph_bool has_body;
};

struct mph_StructStatement {
	mph_Node* base;
	MorphString* name;
	mph_int fields_count;
};

struct mph_ImportStatement {
	mph_Node* base;
	MorphString* path;
};

struct mph_BreakStatement {
	mph_Node* base;
};

struct mph_ContinueStatement {
	mph_Node* base;
};

MorphTypeInfo mph_ti_mph_Node = { "Node", sizeof(mph_Node), 1, (size_t[]){offsetof(mph_Node, token_literal)} };
MorphTypeInfo mph_ti_mph_Program = { "Program", sizeof(mph_Program), 1, (size_t[]){offsetof(mph_Program, base)} };
MorphTypeInfo mph_ti_mph_VarStatement = { "VarStatement", sizeof(mph_VarStatement), 3, (size_t[]){offsetof(mph_VarStatement, base), offsetof(mph_VarStatement, name), offsetof(mph_VarStatement, value_type)} };
MorphTypeInfo mph_ti_mph_FunctionStatement = { "FunctionStatement", sizeof(mph_FunctionStatement), 3, (size_t[]){offsetof(mph_FunctionStatement, base), offsetof(mph_FunctionStatement, name), offsetof(mph_FunctionStatement, return_type)} };
MorphTypeInfo mph_ti_mph_ReturnStatement = { "ReturnStatement", sizeof(mph_ReturnStatement), 1, (size_t[]){offsetof(mph_ReturnStatement, base)} };
MorphTypeInfo mph_ti_mph_ExpressionStatement = { "ExpressionStatement", sizeof(mph_ExpressionStatement), 1, (size_t[]){offsetof(mph_ExpressionStatement, base)} };
MorphTypeInfo mph_ti_mph_BlockStatement = { "BlockStatement", sizeof(mph_BlockStatement), 1, (size_t[]){offsetof(mph_BlockStatement, base)} };
MorphTypeInfo mph_ti_mph_Identifier = { "Identifier", sizeof(mph_Identifier), 2, (size_t[]){offsetof(mph_Identifier, base), offsetof(mph_Identifier, value)} };
MorphTypeInfo mph_ti_mph_IntegerLiteral = { "IntegerLiteral", sizeof(mph_IntegerLiteral), 1, (size_t[]){offsetof(mph_IntegerLiteral, base)} };
MorphTypeInfo mph_ti_mph_FloatLiteral = { "FloatLiteral", sizeof(mph_FloatLiteral), 2, (size_t[]){offsetof(mph_FloatLiteral, base), offsetof(mph_FloatLiteral, value_str)} };
MorphTypeInfo mph_ti_mph_StringLiteral = { "StringLiteral", sizeof(mph_StringLiteral), 2, (size_t[]){offsetof(mph_StringLiteral, base), offsetof(mph_StringLiteral, value)} };
MorphTypeInfo mph_ti_mph_BooleanLiteral = { "BooleanLiteral", sizeof(mph_BooleanLiteral), 1, (size_t[]){offsetof(mph_BooleanLiteral, base)} };
MorphTypeInfo mph_ti_mph_PrefixExpression = { "PrefixExpression", sizeof(mph_PrefixExpression), 2, (size_t[]){offsetof(mph_PrefixExpression, base), offsetof(mph_PrefixExpression, operator)} };
MorphTypeInfo mph_ti_mph_InfixExpression = { "InfixExpression", sizeof(mph_InfixExpression), 2, (size_t[]){offsetof(mph_InfixExpression, base), offsetof(mph_InfixExpression, operator)} };
MorphTypeInfo mph_ti_mph_IfExpression = { "IfExpression", sizeof(mph_IfExpression), 1, (size_t[]){offsetof(mph_IfExpression, base)} };
MorphTypeInfo mph_ti_mph_CallExpression = { "CallExpression", sizeof(mph_CallExpression), 1, (size_t[]){offsetof(mph_CallExpression, base)} };
MorphTypeInfo mph_ti_mph_IndexExpression = { "IndexExpression", sizeof(mph_IndexExpression), 1, (size_t[]){offsetof(mph_IndexExpression, base)} };
MorphTypeInfo mph_ti_mph_MemberExpression = { "MemberExpression", sizeof(mph_MemberExpression), 2, (size_t[]){offsetof(mph_MemberExpression, base), offsetof(mph_MemberExpression, member_name)} };
MorphTypeInfo mph_ti_mph_CaseClause = { "CaseClause", sizeof(mph_CaseClause), 1, (size_t[]){offsetof(mph_CaseClause, base)} };
MorphTypeInfo mph_ti_mph_SwitchStatement = { "SwitchStatement", sizeof(mph_SwitchStatement), 1, (size_t[]){offsetof(mph_SwitchStatement, base)} };
MorphTypeInfo mph_ti_mph_WhileStatement = { "WhileStatement", sizeof(mph_WhileStatement), 1, (size_t[]){offsetof(mph_WhileStatement, base)} };
MorphTypeInfo mph_ti_mph_StructStatement = { "StructStatement", sizeof(mph_StructStatement), 2, (size_t[]){offsetof(mph_StructStatement, base), offsetof(mph_StructStatement, name)} };
MorphTypeInfo mph_ti_mph_ImportStatement = { "ImportStatement", sizeof(mph_ImportStatement), 2, (size_t[]){offsetof(mph_ImportStatement, base), offsetof(mph_ImportStatement, path)} };
MorphTypeInfo mph_ti_mph_BreakStatement = { "BreakStatement", sizeof(mph_BreakStatement), 1, (size_t[]){offsetof(mph_BreakStatement, base)} };
MorphTypeInfo mph_ti_mph_ContinueStatement = { "ContinueStatement", sizeof(mph_ContinueStatement), 1, (size_t[]){offsetof(mph_ContinueStatement, base)} };

// Global Variables
mph_int mph_NODE_PROGRAM;
mph_int mph_NODE_VAR_STATEMENT;
mph_int mph_NODE_FUNCTION_STATEMENT;
mph_int mph_NODE_RETURN_STATEMENT;
mph_int mph_NODE_EXPRESSION_STATEMENT;
mph_int mph_NODE_BLOCK_STATEMENT;
mph_int mph_NODE_IF_EXPRESSION;
mph_int mph_NODE_IDENTIFIER;
mph_int mph_NODE_INTEGER_LITERAL;
mph_int mph_NODE_FLOAT_LITERAL;
mph_int mph_NODE_STRING_LITERAL;
mph_int mph_NODE_BOOLEAN_LITERAL;
mph_int mph_NODE_ARRAY_LITERAL;
mph_int mph_NODE_HASH_LITERAL;
mph_int mph_NODE_FUNCTION_LITERAL;
mph_int mph_NODE_PREFIX_EXPRESSION;
mph_int mph_NODE_INFIX_EXPRESSION;
mph_int mph_NODE_CALL_EXPRESSION;
mph_int mph_NODE_INDEX_EXPRESSION;
mph_int mph_NODE_MEMBER_EXPRESSION;
mph_int mph_NODE_ASSIGNMENT_EXPRESSION;
mph_int mph_NODE_SWITCH_STATEMENT;
mph_int mph_NODE_CASE_CLAUSE;
mph_int mph_NODE_WHILE_STATEMENT;
mph_int mph_NODE_STRUCT_STATEMENT;
mph_int mph_NODE_STRUCT_LITERAL;
mph_int mph_NODE_IMPORT_STATEMENT;
mph_int mph_NODE_INTERFACE_STATEMENT;
mph_int mph_NODE_BREAK_STATEMENT;
mph_int mph_NODE_CONTINUE_STATEMENT;

// Function Prototypes
mph_Node* mph_make_node(MorphContext* ctx, void* _env_void, mph_int node_type, MorphString* literal, mph_int line, mph_int column);
mph_Program* mph_make_program(MorphContext* ctx, void* _env_void);
mph_Identifier* mph_make_identifier(MorphContext* ctx, void* _env_void, MorphString* name, mph_int line, mph_int column);
mph_IntegerLiteral* mph_make_integer_literal(MorphContext* ctx, void* _env_void, mph_int value, MorphString* literal, mph_int line, mph_int column);
mph_StringLiteral* mph_make_string_literal(MorphContext* ctx, void* _env_void, MorphString* value, mph_int line, mph_int column);
mph_BooleanLiteral* mph_make_boolean_literal(MorphContext* ctx, void* _env_void, mph_bool value, MorphString* literal, mph_int line, mph_int column);
mph_VarStatement* mph_make_var_statement(MorphContext* ctx, void* _env_void, MorphString* name, MorphString* type_str, mph_int line, mph_int column);
mph_PrefixExpression* mph_make_prefix_expression(MorphContext* ctx, void* _env_void, MorphString* operator, mph_int line, mph_int column);
mph_InfixExpression* mph_make_infix_expression(MorphContext* ctx, void* _env_void, MorphString* operator, mph_int line, mph_int column);
mph_CaseClause* mph_make_case_clause(MorphContext* ctx, void* _env_void, mph_int line, mph_int column);
mph_CaseClause* mph_case_add_value(MorphContext* ctx, void* _env_void, mph_CaseClause* c, mph_int val);
mph_SwitchStatement* mph_make_switch_statement(MorphContext* ctx, void* _env_void, mph_int line, mph_int column);
mph_WhileStatement* mph_make_while_statement(MorphContext* ctx, void* _env_void, mph_int line, mph_int column);
mph_StructStatement* mph_make_struct_statement(MorphContext* ctx, void* _env_void, MorphString* name, mph_int line, mph_int column);
mph_ImportStatement* mph_make_import_statement(MorphContext* ctx, void* _env_void, MorphString* path, mph_int line, mph_int column);
mph_BreakStatement* mph_make_break_statement(MorphContext* ctx, void* _env_void, mph_int line, mph_int column);
mph_ContinueStatement* mph_make_continue_statement(MorphContext* ctx, void* _env_void, mph_int line, mph_int column);
MorphString* mph_node_type_string(MorphContext* ctx, void* _env_void, mph_int node_type);
void mph_visit_node(MorphContext* ctx, void* _env_void, mph_Node* node);
void mph_print_identifier(MorphContext* ctx, void* _env_void, mph_Identifier* ident);
void mph_print_integer_literal(MorphContext* ctx, void* _env_void, mph_IntegerLiteral* lit);
void mph_print_string_literal(MorphContext* ctx, void* _env_void, mph_StringLiteral* lit);
void mph_print_boolean_literal(MorphContext* ctx, void* _env_void, mph_BooleanLiteral* lit);

// Function Definitions
mph_Node* mph_make_node(MorphContext* ctx, void* _env_void, mph_int node_type, MorphString* literal, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&literal);
	return ({ mph_Node* _t = (mph_Node*)mph_alloc(ctx, sizeof(mph_Node), &mph_ti_mph_Node); mph_gc_push_root(ctx, (void**)&_t); _t->token_literal = literal; _t->line = line; _t->column = column; _t->node_type = node_type; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

mph_Program* mph_make_program(MorphContext* ctx, void* _env_void) {
	mph_Node* base = ({ MorphString* _arg_1 = mph_string_new(ctx, "program"); mph_gc_push_root(ctx, (void**)&_arg_1); mph_Node* _ret_2 = mph_make_node(ctx, NULL, mph_NODE_PROGRAM, _arg_1, 1, 1); mph_gc_pop_roots(ctx, 1); _ret_2; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_Program* _t = (mph_Program*)mph_alloc(ctx, sizeof(mph_Program), &mph_ti_mph_Program); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->statements_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

mph_Identifier* mph_make_identifier(MorphContext* ctx, void* _env_void, MorphString* name, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&name);
	mph_Node* base = ({ MorphString* _arg_3 = name; mph_gc_push_root(ctx, (void**)&_arg_3); mph_Node* _ret_4 = mph_make_node(ctx, NULL, mph_NODE_IDENTIFIER, _arg_3, line, column); mph_gc_pop_roots(ctx, 1); _ret_4; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_Identifier* _t = (mph_Identifier*)mph_alloc(ctx, sizeof(mph_Identifier), &mph_ti_mph_Identifier); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->value = name; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

mph_IntegerLiteral* mph_make_integer_literal(MorphContext* ctx, void* _env_void, mph_int value, MorphString* literal, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&literal);
	mph_Node* base = ({ MorphString* _arg_5 = literal; mph_gc_push_root(ctx, (void**)&_arg_5); mph_Node* _ret_6 = mph_make_node(ctx, NULL, mph_NODE_INTEGER_LITERAL, _arg_5, line, column); mph_gc_pop_roots(ctx, 1); _ret_6; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_IntegerLiteral* _t = (mph_IntegerLiteral*)mph_alloc(ctx, sizeof(mph_IntegerLiteral), &mph_ti_mph_IntegerLiteral); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->value = value; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

mph_StringLiteral* mph_make_string_literal(MorphContext* ctx, void* _env_void, MorphString* value, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&value);
	mph_Node* base = ({ MorphString* _arg_7 = value; mph_gc_push_root(ctx, (void**)&_arg_7); mph_Node* _ret_8 = mph_make_node(ctx, NULL, mph_NODE_STRING_LITERAL, _arg_7, line, column); mph_gc_pop_roots(ctx, 1); _ret_8; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_StringLiteral* _t = (mph_StringLiteral*)mph_alloc(ctx, sizeof(mph_StringLiteral), &mph_ti_mph_StringLiteral); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->value = value; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

mph_BooleanLiteral* mph_make_boolean_literal(MorphContext* ctx, void* _env_void, mph_bool value, MorphString* literal, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&literal);
	mph_Node* base = ({ MorphString* _arg_9 = literal; mph_gc_push_root(ctx, (void**)&_arg_9); mph_Node* _ret_10 = mph_make_node(ctx, NULL, mph_NODE_BOOLEAN_LITERAL, _arg_9, line, column); mph_gc_pop_roots(ctx, 1); _ret_10; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_BooleanLiteral* _t = (mph_BooleanLiteral*)mph_alloc(ctx, sizeof(mph_BooleanLiteral), &mph_ti_mph_BooleanLiteral); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->value = value; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

mph_VarStatement* mph_make_var_statement(MorphContext* ctx, void* _env_void, MorphString* name, MorphString* type_str, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&name);
	mph_gc_push_root(ctx, (void**)&type_str);
	mph_Node* base = ({ MorphString* _arg_11 = mph_string_new(ctx, "var"); mph_gc_push_root(ctx, (void**)&_arg_11); mph_Node* _ret_12 = mph_make_node(ctx, NULL, mph_NODE_VAR_STATEMENT, _arg_11, line, column); mph_gc_pop_roots(ctx, 1); _ret_12; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_VarStatement* _t = (mph_VarStatement*)mph_alloc(ctx, sizeof(mph_VarStatement), &mph_ti_mph_VarStatement); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->name = name; _t->value_type = type_str; _t->has_value = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 2);
}

mph_PrefixExpression* mph_make_prefix_expression(MorphContext* ctx, void* _env_void, MorphString* operator, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&operator);
	mph_Node* base = ({ MorphString* _arg_13 = operator; mph_gc_push_root(ctx, (void**)&_arg_13); mph_Node* _ret_14 = mph_make_node(ctx, NULL, mph_NODE_PREFIX_EXPRESSION, _arg_13, line, column); mph_gc_pop_roots(ctx, 1); _ret_14; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_PrefixExpression* _t = (mph_PrefixExpression*)mph_alloc(ctx, sizeof(mph_PrefixExpression), &mph_ti_mph_PrefixExpression); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->operator = operator; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

mph_InfixExpression* mph_make_infix_expression(MorphContext* ctx, void* _env_void, MorphString* operator, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&operator);
	mph_Node* base = ({ MorphString* _arg_15 = operator; mph_gc_push_root(ctx, (void**)&_arg_15); mph_Node* _ret_16 = mph_make_node(ctx, NULL, mph_NODE_INFIX_EXPRESSION, _arg_15, line, column); mph_gc_pop_roots(ctx, 1); _ret_16; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_InfixExpression* _t = (mph_InfixExpression*)mph_alloc(ctx, sizeof(mph_InfixExpression), &mph_ti_mph_InfixExpression); mph_gc_push_root(ctx, (void**)&_t); _t->operator = operator; _t->base = base; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

mph_CaseClause* mph_make_case_clause(MorphContext* ctx, void* _env_void, mph_int line, mph_int column) {
	mph_Node* base = ({ MorphString* _arg_17 = mph_string_new(ctx, "kasus"); mph_gc_push_root(ctx, (void**)&_arg_17); mph_Node* _ret_18 = mph_make_node(ctx, NULL, mph_NODE_CASE_CLAUSE, _arg_17, line, column); mph_gc_pop_roots(ctx, 1); _ret_18; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_CaseClause* _t = (mph_CaseClause*)mph_alloc(ctx, sizeof(mph_CaseClause), &mph_ti_mph_CaseClause); mph_gc_push_root(ctx, (void**)&_t); _t->v2 = 0; _t->v3 = 0; _t->has_body = 0; _t->base = base; _t->values_count = 0; _t->v0 = 0; _t->v1 = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

mph_CaseClause* mph_case_add_value(MorphContext* ctx, void* _env_void, mph_CaseClause* c, mph_int val) {
	mph_gc_push_root(ctx, (void**)&c);
	if ((({ mph_CaseClause* _obj_19 = c; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->values_count; mph_gc_pop_roots(ctx, 1); _ret_20; }) == 0)) {
{
	({ mph_CaseClause* _obj_21 = c; mph_gc_push_root(ctx, (void**)&_obj_21); _obj_21->v0 = val; mph_gc_pop_roots(ctx, 1); });
}
	}
	if ((({ mph_CaseClause* _obj_22 = c; mph_gc_push_root(ctx, (void**)&_obj_22); mph_int _ret_23 = _obj_22->values_count; mph_gc_pop_roots(ctx, 1); _ret_23; }) == 1)) {
{
	({ mph_CaseClause* _obj_24 = c; mph_gc_push_root(ctx, (void**)&_obj_24); _obj_24->v1 = val; mph_gc_pop_roots(ctx, 1); });
}
	}
	if ((({ mph_CaseClause* _obj_25 = c; mph_gc_push_root(ctx, (void**)&_obj_25); mph_int _ret_26 = _obj_25->values_count; mph_gc_pop_roots(ctx, 1); _ret_26; }) == 2)) {
{
	({ mph_CaseClause* _obj_27 = c; mph_gc_push_root(ctx, (void**)&_obj_27); _obj_27->v2 = val; mph_gc_pop_roots(ctx, 1); });
}
	}
	if ((({ mph_CaseClause* _obj_28 = c; mph_gc_push_root(ctx, (void**)&_obj_28); mph_int _ret_29 = _obj_28->values_count; mph_gc_pop_roots(ctx, 1); _ret_29; }) == 3)) {
{
	({ mph_CaseClause* _obj_30 = c; mph_gc_push_root(ctx, (void**)&_obj_30); _obj_30->v3 = val; mph_gc_pop_roots(ctx, 1); });
}
	}
	({ mph_CaseClause* _obj_33 = c; mph_gc_push_root(ctx, (void**)&_obj_33); _obj_33->values_count = (({ mph_CaseClause* _obj_31 = c; mph_gc_push_root(ctx, (void**)&_obj_31); mph_int _ret_32 = _obj_31->values_count; mph_gc_pop_roots(ctx, 1); _ret_32; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return c;
	mph_gc_pop_roots(ctx, 1);
}

mph_SwitchStatement* mph_make_switch_statement(MorphContext* ctx, void* _env_void, mph_int line, mph_int column) {
	mph_Node* base = ({ MorphString* _arg_34 = mph_string_new(ctx, "pilih"); mph_gc_push_root(ctx, (void**)&_arg_34); mph_Node* _ret_35 = mph_make_node(ctx, NULL, mph_NODE_SWITCH_STATEMENT, _arg_34, line, column); mph_gc_pop_roots(ctx, 1); _ret_35; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_SwitchStatement* _t = (mph_SwitchStatement*)mph_alloc(ctx, sizeof(mph_SwitchStatement), &mph_ti_mph_SwitchStatement); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->cases_count = 0; _t->has_default = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

mph_WhileStatement* mph_make_while_statement(MorphContext* ctx, void* _env_void, mph_int line, mph_int column) {
	mph_Node* base = ({ MorphString* _arg_36 = mph_string_new(ctx, "selama"); mph_gc_push_root(ctx, (void**)&_arg_36); mph_Node* _ret_37 = mph_make_node(ctx, NULL, mph_NODE_WHILE_STATEMENT, _arg_36, line, column); mph_gc_pop_roots(ctx, 1); _ret_37; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_WhileStatement* _t = (mph_WhileStatement*)mph_alloc(ctx, sizeof(mph_WhileStatement), &mph_ti_mph_WhileStatement); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->has_body = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

mph_StructStatement* mph_make_struct_statement(MorphContext* ctx, void* _env_void, MorphString* name, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&name);
	mph_Node* base = ({ MorphString* _arg_38 = mph_string_new(ctx, "struktur"); mph_gc_push_root(ctx, (void**)&_arg_38); mph_Node* _ret_39 = mph_make_node(ctx, NULL, mph_NODE_STRUCT_STATEMENT, _arg_38, line, column); mph_gc_pop_roots(ctx, 1); _ret_39; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_StructStatement* _t = (mph_StructStatement*)mph_alloc(ctx, sizeof(mph_StructStatement), &mph_ti_mph_StructStatement); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->name = name; _t->fields_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

mph_ImportStatement* mph_make_import_statement(MorphContext* ctx, void* _env_void, MorphString* path, mph_int line, mph_int column) {
	mph_gc_push_root(ctx, (void**)&path);
	mph_Node* base = ({ MorphString* _arg_40 = mph_string_new(ctx, "ambil"); mph_gc_push_root(ctx, (void**)&_arg_40); mph_Node* _ret_41 = mph_make_node(ctx, NULL, mph_NODE_IMPORT_STATEMENT, _arg_40, line, column); mph_gc_pop_roots(ctx, 1); _ret_41; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_ImportStatement* _t = (mph_ImportStatement*)mph_alloc(ctx, sizeof(mph_ImportStatement), &mph_ti_mph_ImportStatement); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; _t->path = path; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

mph_BreakStatement* mph_make_break_statement(MorphContext* ctx, void* _env_void, mph_int line, mph_int column) {
	mph_Node* base = ({ MorphString* _arg_42 = mph_string_new(ctx, "putus"); mph_gc_push_root(ctx, (void**)&_arg_42); mph_Node* _ret_43 = mph_make_node(ctx, NULL, mph_NODE_BREAK_STATEMENT, _arg_42, line, column); mph_gc_pop_roots(ctx, 1); _ret_43; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_BreakStatement* _t = (mph_BreakStatement*)mph_alloc(ctx, sizeof(mph_BreakStatement), &mph_ti_mph_BreakStatement); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

mph_ContinueStatement* mph_make_continue_statement(MorphContext* ctx, void* _env_void, mph_int line, mph_int column) {
	mph_Node* base = ({ MorphString* _arg_44 = mph_string_new(ctx, "lanjut"); mph_gc_push_root(ctx, (void**)&_arg_44); mph_Node* _ret_45 = mph_make_node(ctx, NULL, mph_NODE_CONTINUE_STATEMENT, _arg_44, line, column); mph_gc_pop_roots(ctx, 1); _ret_45; });
	mph_gc_push_root(ctx, (void**)&base);
	return ({ mph_ContinueStatement* _t = (mph_ContinueStatement*)mph_alloc(ctx, sizeof(mph_ContinueStatement), &mph_ti_mph_ContinueStatement); mph_gc_push_root(ctx, (void**)&_t); _t->base = base; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

MorphString* mph_node_type_string(MorphContext* ctx, void* _env_void, mph_int node_type) {
	if ((node_type == mph_NODE_PROGRAM)) {
{
	return mph_string_new(ctx, "Program");
}
	}
	if ((node_type == mph_NODE_VAR_STATEMENT)) {
{
	return mph_string_new(ctx, "VarStatement");
}
	}
	if ((node_type == mph_NODE_FUNCTION_STATEMENT)) {
{
	return mph_string_new(ctx, "FunctionStatement");
}
	}
	if ((node_type == mph_NODE_RETURN_STATEMENT)) {
{
	return mph_string_new(ctx, "ReturnStatement");
}
	}
	if ((node_type == mph_NODE_EXPRESSION_STATEMENT)) {
{
	return mph_string_new(ctx, "ExpressionStatement");
}
	}
	if ((node_type == mph_NODE_BLOCK_STATEMENT)) {
{
	return mph_string_new(ctx, "BlockStatement");
}
	}
	if ((node_type == mph_NODE_IF_EXPRESSION)) {
{
	return mph_string_new(ctx, "IfExpression");
}
	}
	if ((node_type == mph_NODE_IDENTIFIER)) {
{
	return mph_string_new(ctx, "Identifier");
}
	}
	if ((node_type == mph_NODE_INTEGER_LITERAL)) {
{
	return mph_string_new(ctx, "IntegerLiteral");
}
	}
	if ((node_type == mph_NODE_FLOAT_LITERAL)) {
{
	return mph_string_new(ctx, "FloatLiteral");
}
	}
	if ((node_type == mph_NODE_STRING_LITERAL)) {
{
	return mph_string_new(ctx, "StringLiteral");
}
	}
	if ((node_type == mph_NODE_BOOLEAN_LITERAL)) {
{
	return mph_string_new(ctx, "BooleanLiteral");
}
	}
	if ((node_type == mph_NODE_ARRAY_LITERAL)) {
{
	return mph_string_new(ctx, "ArrayLiteral");
}
	}
	if ((node_type == mph_NODE_HASH_LITERAL)) {
{
	return mph_string_new(ctx, "HashLiteral");
}
	}
	if ((node_type == mph_NODE_FUNCTION_LITERAL)) {
{
	return mph_string_new(ctx, "FunctionLiteral");
}
	}
	if ((node_type == mph_NODE_PREFIX_EXPRESSION)) {
{
	return mph_string_new(ctx, "PrefixExpression");
}
	}
	if ((node_type == mph_NODE_INFIX_EXPRESSION)) {
{
	return mph_string_new(ctx, "InfixExpression");
}
	}
	if ((node_type == mph_NODE_CALL_EXPRESSION)) {
{
	return mph_string_new(ctx, "CallExpression");
}
	}
	if ((node_type == mph_NODE_INDEX_EXPRESSION)) {
{
	return mph_string_new(ctx, "IndexExpression");
}
	}
	if ((node_type == mph_NODE_MEMBER_EXPRESSION)) {
{
	return mph_string_new(ctx, "MemberExpression");
}
	}
	if ((node_type == mph_NODE_ASSIGNMENT_EXPRESSION)) {
{
	return mph_string_new(ctx, "AssignmentExpression");
}
	}
	return mph_string_new(ctx, "Unknown");
}

void mph_visit_node(MorphContext* ctx, void* _env_void, mph_Node* node) {
	mph_gc_push_root(ctx, (void**)&node);
	mph_native_print(ctx, mph_string_new(ctx, "Visiting "));
	mph_native_print(ctx, mph_node_type_string(ctx, NULL, ({ mph_Node* _obj_46 = node; mph_gc_push_root(ctx, (void**)&_obj_46); mph_int _ret_47 = _obj_46->node_type; mph_gc_pop_roots(ctx, 1); _ret_47; })));
	mph_native_print(ctx, mph_string_new(ctx, " at line "));
	mph_native_print_int(ctx, ({ mph_Node* _obj_48 = node; mph_gc_push_root(ctx, (void**)&_obj_48); mph_int _ret_49 = _obj_48->line; mph_gc_pop_roots(ctx, 1); _ret_49; }));
	mph_native_print(ctx, mph_string_new(ctx, ", column "));
	mph_native_print_int(ctx, ({ mph_Node* _obj_50 = node; mph_gc_push_root(ctx, (void**)&_obj_50); mph_int _ret_51 = _obj_50->column; mph_gc_pop_roots(ctx, 1); _ret_51; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_gc_pop_roots(ctx, 1);
}

void mph_print_identifier(MorphContext* ctx, void* _env_void, mph_Identifier* ident) {
	mph_gc_push_root(ctx, (void**)&ident);
	mph_native_print(ctx, mph_string_new(ctx, "Identifier: "));
	mph_native_print(ctx, ({ mph_Identifier* _obj_52 = ident; mph_gc_push_root(ctx, (void**)&_obj_52); MorphString* _ret_53 = _obj_52->value; mph_gc_pop_roots(ctx, 1); _ret_53; }));
	mph_native_print(ctx, mph_string_new(ctx, " ("));
	mph_native_print_int(ctx, ({ mph_Node* _obj_56 = ({ mph_Identifier* _obj_54 = ident; mph_gc_push_root(ctx, (void**)&_obj_54); mph_Node* _ret_55 = _obj_54->base; mph_gc_pop_roots(ctx, 1); _ret_55; }); mph_gc_push_root(ctx, (void**)&_obj_56); mph_int _ret_57 = _obj_56->line; mph_gc_pop_roots(ctx, 1); _ret_57; }));
	mph_native_print(ctx, mph_string_new(ctx, ":"));
	mph_native_print_int(ctx, ({ mph_Node* _obj_60 = ({ mph_Identifier* _obj_58 = ident; mph_gc_push_root(ctx, (void**)&_obj_58); mph_Node* _ret_59 = _obj_58->base; mph_gc_pop_roots(ctx, 1); _ret_59; }); mph_gc_push_root(ctx, (void**)&_obj_60); mph_int _ret_61 = _obj_60->column; mph_gc_pop_roots(ctx, 1); _ret_61; }));
	mph_native_print(ctx, mph_string_new(ctx, ")\n"));
	mph_gc_pop_roots(ctx, 1);
}

void mph_print_integer_literal(MorphContext* ctx, void* _env_void, mph_IntegerLiteral* lit) {
	mph_gc_push_root(ctx, (void**)&lit);
	mph_native_print(ctx, mph_string_new(ctx, "IntegerLiteral: "));
	mph_native_print_int(ctx, ({ mph_IntegerLiteral* _obj_62 = lit; mph_gc_push_root(ctx, (void**)&_obj_62); mph_int _ret_63 = _obj_62->value; mph_gc_pop_roots(ctx, 1); _ret_63; }));
	mph_native_print(ctx, mph_string_new(ctx, " ("));
	mph_native_print_int(ctx, ({ mph_Node* _obj_66 = ({ mph_IntegerLiteral* _obj_64 = lit; mph_gc_push_root(ctx, (void**)&_obj_64); mph_Node* _ret_65 = _obj_64->base; mph_gc_pop_roots(ctx, 1); _ret_65; }); mph_gc_push_root(ctx, (void**)&_obj_66); mph_int _ret_67 = _obj_66->line; mph_gc_pop_roots(ctx, 1); _ret_67; }));
	mph_native_print(ctx, mph_string_new(ctx, ":"));
	mph_native_print_int(ctx, ({ mph_Node* _obj_70 = ({ mph_IntegerLiteral* _obj_68 = lit; mph_gc_push_root(ctx, (void**)&_obj_68); mph_Node* _ret_69 = _obj_68->base; mph_gc_pop_roots(ctx, 1); _ret_69; }); mph_gc_push_root(ctx, (void**)&_obj_70); mph_int _ret_71 = _obj_70->column; mph_gc_pop_roots(ctx, 1); _ret_71; }));
	mph_native_print(ctx, mph_string_new(ctx, ")\n"));
	mph_gc_pop_roots(ctx, 1);
}

void mph_print_string_literal(MorphContext* ctx, void* _env_void, mph_StringLiteral* lit) {
	mph_gc_push_root(ctx, (void**)&lit);
	mph_native_print(ctx, mph_string_new(ctx, "StringLiteral: \""));
	mph_native_print(ctx, ({ mph_StringLiteral* _obj_72 = lit; mph_gc_push_root(ctx, (void**)&_obj_72); MorphString* _ret_73 = _obj_72->value; mph_gc_pop_roots(ctx, 1); _ret_73; }));
	mph_native_print(ctx, mph_string_new(ctx, "\" ("));
	mph_native_print_int(ctx, ({ mph_Node* _obj_76 = ({ mph_StringLiteral* _obj_74 = lit; mph_gc_push_root(ctx, (void**)&_obj_74); mph_Node* _ret_75 = _obj_74->base; mph_gc_pop_roots(ctx, 1); _ret_75; }); mph_gc_push_root(ctx, (void**)&_obj_76); mph_int _ret_77 = _obj_76->line; mph_gc_pop_roots(ctx, 1); _ret_77; }));
	mph_native_print(ctx, mph_string_new(ctx, ":"));
	mph_native_print_int(ctx, ({ mph_Node* _obj_80 = ({ mph_StringLiteral* _obj_78 = lit; mph_gc_push_root(ctx, (void**)&_obj_78); mph_Node* _ret_79 = _obj_78->base; mph_gc_pop_roots(ctx, 1); _ret_79; }); mph_gc_push_root(ctx, (void**)&_obj_80); mph_int _ret_81 = _obj_80->column; mph_gc_pop_roots(ctx, 1); _ret_81; }));
	mph_native_print(ctx, mph_string_new(ctx, ")\n"));
	mph_gc_pop_roots(ctx, 1);
}

void mph_print_boolean_literal(MorphContext* ctx, void* _env_void, mph_BooleanLiteral* lit) {
	mph_gc_push_root(ctx, (void**)&lit);
	mph_native_print(ctx, mph_string_new(ctx, "BooleanLiteral: "));
	if (({ mph_BooleanLiteral* _obj_82 = lit; mph_gc_push_root(ctx, (void**)&_obj_82); mph_bool _ret_83 = _obj_82->value; mph_gc_pop_roots(ctx, 1); _ret_83; })) {
{
	mph_native_print(ctx, mph_string_new(ctx, "benar"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "salah"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, " ("));
	mph_native_print_int(ctx, ({ mph_Node* _obj_86 = ({ mph_BooleanLiteral* _obj_84 = lit; mph_gc_push_root(ctx, (void**)&_obj_84); mph_Node* _ret_85 = _obj_84->base; mph_gc_pop_roots(ctx, 1); _ret_85; }); mph_gc_push_root(ctx, (void**)&_obj_86); mph_int _ret_87 = _obj_86->line; mph_gc_pop_roots(ctx, 1); _ret_87; }));
	mph_native_print(ctx, mph_string_new(ctx, ":"));
	mph_native_print_int(ctx, ({ mph_Node* _obj_90 = ({ mph_BooleanLiteral* _obj_88 = lit; mph_gc_push_root(ctx, (void**)&_obj_88); mph_Node* _ret_89 = _obj_88->base; mph_gc_pop_roots(ctx, 1); _ret_89; }); mph_gc_push_root(ctx, (void**)&_obj_90); mph_int _ret_91 = _obj_90->column; mph_gc_pop_roots(ctx, 1); _ret_91; }));
	mph_native_print(ctx, mph_string_new(ctx, ")\n"));
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_NODE_PROGRAM = 0;
	mph_NODE_VAR_STATEMENT = 1;
	mph_NODE_FUNCTION_STATEMENT = 2;
	mph_NODE_RETURN_STATEMENT = 3;
	mph_NODE_EXPRESSION_STATEMENT = 4;
	mph_NODE_BLOCK_STATEMENT = 5;
	mph_NODE_IF_EXPRESSION = 6;
	mph_NODE_IDENTIFIER = 7;
	mph_NODE_INTEGER_LITERAL = 8;
	mph_NODE_FLOAT_LITERAL = 9;
	mph_NODE_STRING_LITERAL = 10;
	mph_NODE_BOOLEAN_LITERAL = 11;
	mph_NODE_ARRAY_LITERAL = 12;
	mph_NODE_HASH_LITERAL = 13;
	mph_NODE_FUNCTION_LITERAL = 14;
	mph_NODE_PREFIX_EXPRESSION = 15;
	mph_NODE_INFIX_EXPRESSION = 16;
	mph_NODE_CALL_EXPRESSION = 17;
	mph_NODE_INDEX_EXPRESSION = 18;
	mph_NODE_MEMBER_EXPRESSION = 19;
	mph_NODE_ASSIGNMENT_EXPRESSION = 20;
	mph_NODE_SWITCH_STATEMENT = 21;
	mph_NODE_CASE_CLAUSE = 22;
	mph_NODE_WHILE_STATEMENT = 23;
	mph_NODE_STRUCT_STATEMENT = 24;
	mph_NODE_STRUCT_LITERAL = 25;
	mph_NODE_IMPORT_STATEMENT = 26;
	mph_NODE_INTERFACE_STATEMENT = 27;
	mph_NODE_BREAK_STATEMENT = 28;
	mph_NODE_CONTINUE_STATEMENT = 29;
}
