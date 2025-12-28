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
#define MPH_TYPE_mph_SwitchStatement 2
#define MPH_TYPE_mph_CaseClause 1

// Struct Definitions (Env & Types)
typedef struct mph_CaseClause mph_CaseClause;
typedef struct mph_SwitchStatement mph_SwitchStatement;
struct mph_CaseClause {
	mph_int node_type;
	mph_int values_count;
	mph_int v0;
	mph_int v1;
	mph_int v2;
};

struct mph_SwitchStatement {
	mph_int node_type;
	mph_int cases_count;
	mph_bool has_default;
};

MorphTypeInfo mph_ti_mph_CaseClause = { "CaseClause", sizeof(mph_CaseClause), 0, NULL };
MorphTypeInfo mph_ti_mph_SwitchStatement = { "SwitchStatement", sizeof(mph_SwitchStatement), 0, NULL };

// Global Variables
mph_int mph_NODE_SWITCH_STATEMENT;
mph_int mph_NODE_CASE_CLAUSE;

// Function Prototypes
mph_CaseClause* mph_make_case(MorphContext* ctx, void* _env_void);
mph_CaseClause* mph_case_add(MorphContext* ctx, void* _env_void, mph_CaseClause* c, mph_int val);
mph_SwitchStatement* mph_make_switch(MorphContext* ctx, void* _env_void);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_CaseClause* mph_make_case(MorphContext* ctx, void* _env_void) {
	return ({ mph_CaseClause* _t = (mph_CaseClause*)mph_alloc(ctx, sizeof(mph_CaseClause), &mph_ti_mph_CaseClause); mph_gc_push_root(ctx, (void**)&_t); _t->v2 = 0; _t->node_type = mph_NODE_CASE_CLAUSE; _t->values_count = 0; _t->v0 = 0; _t->v1 = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_CaseClause* mph_case_add(MorphContext* ctx, void* _env_void, mph_CaseClause* c, mph_int val) {
	mph_gc_push_root(ctx, (void**)&c);
	if ((({ mph_CaseClause* _obj_1 = c; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->values_count; mph_gc_pop_roots(ctx, 1); _ret_2; }) == 0)) {
{
	({ mph_CaseClause* _obj_3 = c; mph_gc_push_root(ctx, (void**)&_obj_3); _obj_3->v0 = val; mph_gc_pop_roots(ctx, 1); });
}
	}
	if ((({ mph_CaseClause* _obj_4 = c; mph_gc_push_root(ctx, (void**)&_obj_4); mph_int _ret_5 = _obj_4->values_count; mph_gc_pop_roots(ctx, 1); _ret_5; }) == 1)) {
{
	({ mph_CaseClause* _obj_6 = c; mph_gc_push_root(ctx, (void**)&_obj_6); _obj_6->v1 = val; mph_gc_pop_roots(ctx, 1); });
}
	}
	if ((({ mph_CaseClause* _obj_7 = c; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->values_count; mph_gc_pop_roots(ctx, 1); _ret_8; }) == 2)) {
{
	({ mph_CaseClause* _obj_9 = c; mph_gc_push_root(ctx, (void**)&_obj_9); _obj_9->v2 = val; mph_gc_pop_roots(ctx, 1); });
}
	}
	({ mph_CaseClause* _obj_12 = c; mph_gc_push_root(ctx, (void**)&_obj_12); _obj_12->values_count = (({ mph_CaseClause* _obj_10 = c; mph_gc_push_root(ctx, (void**)&_obj_10); mph_int _ret_11 = _obj_10->values_count; mph_gc_pop_roots(ctx, 1); _ret_11; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return c;
	mph_gc_pop_roots(ctx, 1);
}

mph_SwitchStatement* mph_make_switch(MorphContext* ctx, void* _env_void) {
	return ({ mph_SwitchStatement* _t = (mph_SwitchStatement*)mph_alloc(ctx, sizeof(mph_SwitchStatement), &mph_ti_mph_SwitchStatement); mph_gc_push_root(ctx, (void**)&_t); _t->node_type = mph_NODE_SWITCH_STATEMENT; _t->cases_count = 0; _t->has_default = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "N1 Switch/Case AST Test\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=======================\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "CaseClause Test:\n"));
	mph_CaseClause* c = mph_make_case(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&c);
	c = ({ mph_CaseClause* _arg_13 = c; mph_gc_push_root(ctx, (void**)&_arg_13); mph_CaseClause* _ret_14 = mph_case_add(ctx, NULL, _arg_13, 1); mph_gc_pop_roots(ctx, 1); _ret_14; });
	c = ({ mph_CaseClause* _arg_15 = c; mph_gc_push_root(ctx, (void**)&_arg_15); mph_CaseClause* _ret_16 = mph_case_add(ctx, NULL, _arg_15, 2); mph_gc_pop_roots(ctx, 1); _ret_16; });
	c = ({ mph_CaseClause* _arg_17 = c; mph_gc_push_root(ctx, (void**)&_arg_17); mph_CaseClause* _ret_18 = mph_case_add(ctx, NULL, _arg_17, 3); mph_gc_pop_roots(ctx, 1); _ret_18; });
	if ((({ mph_CaseClause* _obj_19 = c; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->values_count; mph_gc_pop_roots(ctx, 1); _ret_20; }) == 3)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â multi-case values_count = 3\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â values_count wrong\n"));
}
	}
	if ((({ mph_CaseClause* _obj_21 = c; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->v0; mph_gc_pop_roots(ctx, 1); _ret_22; }) == 1)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â v0 = 1\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â v0 wrong\n"));
}
	}
	if ((({ mph_CaseClause* _obj_23 = c; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->v1; mph_gc_pop_roots(ctx, 1); _ret_24; }) == 2)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â v1 = 2\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â v1 wrong\n"));
}
	}
	if ((({ mph_CaseClause* _obj_25 = c; mph_gc_push_root(ctx, (void**)&_obj_25); mph_int _ret_26 = _obj_25->v2; mph_gc_pop_roots(ctx, 1); _ret_26; }) == 3)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â v2 = 3\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â v2 wrong\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "\nSwitchStatement Test:\n"));
	mph_SwitchStatement* sw = mph_make_switch(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&sw);
	({ mph_SwitchStatement* _obj_27 = sw; mph_gc_push_root(ctx, (void**)&_obj_27); _obj_27->cases_count = 3; mph_gc_pop_roots(ctx, 1); });
	({ mph_SwitchStatement* _obj_28 = sw; mph_gc_push_root(ctx, (void**)&_obj_28); _obj_28->has_default = 1; mph_gc_pop_roots(ctx, 1); });
	if ((({ mph_SwitchStatement* _obj_29 = sw; mph_gc_push_root(ctx, (void**)&_obj_29); mph_int _ret_30 = _obj_29->node_type; mph_gc_pop_roots(ctx, 1); _ret_30; }) == mph_NODE_SWITCH_STATEMENT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â node_type = SWITCH\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â node_type wrong\n"));
}
	}
	if ((({ mph_SwitchStatement* _obj_31 = sw; mph_gc_push_root(ctx, (void**)&_obj_31); mph_int _ret_32 = _obj_31->cases_count; mph_gc_pop_roots(ctx, 1); _ret_32; }) == 3)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â cases_count = 3\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â cases_count wrong\n"));
}
	}
	if (({ mph_SwitchStatement* _obj_33 = sw; mph_gc_push_root(ctx, (void**)&_obj_33); mph_bool _ret_34 = _obj_33->has_default; mph_gc_pop_roots(ctx, 1); _ret_34; })) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â has_default = true\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â has_default wrong\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "\nSwitch/Case AST OK!\n"));
	mph_gc_pop_roots(ctx, 2);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_NODE_SWITCH_STATEMENT = 21;
	mph_NODE_CASE_CLAUSE = 22;
	mph_main(ctx, NULL);
}
