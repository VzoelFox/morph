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
#define MPH_TYPE_mph_CaseClause 1
#define MPH_TYPE_mph_SwitchStatement 2

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
	mph_CaseClause* c0;
	mph_CaseClause* c1;
	mph_bool has_default;
};

MorphTypeInfo mph_ti_mph_CaseClause = { "CaseClause", sizeof(mph_CaseClause), 0, NULL };
MorphTypeInfo mph_ti_mph_SwitchStatement = { "SwitchStatement", sizeof(mph_SwitchStatement), 2, (size_t[]){offsetof(mph_SwitchStatement, c0), offsetof(mph_SwitchStatement, c1)} };

// Global Variables
mph_int mph_NODE_SWITCH;
mph_int mph_NODE_CASE;

// Function Prototypes
mph_CaseClause* mph_make_case(MorphContext* ctx, void* _env_void, mph_int line);
mph_CaseClause* mph_case_add(MorphContext* ctx, void* _env_void, mph_CaseClause* c, mph_int val);
mph_bool mph_case_match(MorphContext* ctx, void* _env_void, mph_CaseClause* c, mph_int val);
mph_SwitchStatement* mph_make_switch(MorphContext* ctx, void* _env_void);
mph_SwitchStatement* mph_switch_add(MorphContext* ctx, void* _env_void, mph_SwitchStatement* sw, mph_CaseClause* c);
mph_int mph_switch_eval(MorphContext* ctx, void* _env_void, mph_SwitchStatement* sw, mph_int val);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_CaseClause* mph_make_case(MorphContext* ctx, void* _env_void, mph_int line) {
	return ({ mph_CaseClause* _t = (mph_CaseClause*)mph_alloc(ctx, sizeof(mph_CaseClause), &mph_ti_mph_CaseClause); mph_gc_push_root(ctx, (void**)&_t); _t->values_count = 0; _t->v0 = 0; _t->v1 = 0; _t->v2 = 0; _t->node_type = mph_NODE_CASE; mph_gc_pop_roots(ctx, 1); _t; });
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

mph_bool mph_case_match(MorphContext* ctx, void* _env_void, mph_CaseClause* c, mph_int val) {
	mph_gc_push_root(ctx, (void**)&c);
	if ((({ mph_CaseClause* _obj_13 = c; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->values_count; mph_gc_pop_roots(ctx, 1); _ret_14; }) >= 1)) {
{
	if ((({ mph_CaseClause* _obj_15 = c; mph_gc_push_root(ctx, (void**)&_obj_15); mph_int _ret_16 = _obj_15->v0; mph_gc_pop_roots(ctx, 1); _ret_16; }) == val)) {
{
	return 1;
}
	}
}
	}
	if ((({ mph_CaseClause* _obj_17 = c; mph_gc_push_root(ctx, (void**)&_obj_17); mph_int _ret_18 = _obj_17->values_count; mph_gc_pop_roots(ctx, 1); _ret_18; }) >= 2)) {
{
	if ((({ mph_CaseClause* _obj_19 = c; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->v1; mph_gc_pop_roots(ctx, 1); _ret_20; }) == val)) {
{
	return 1;
}
	}
}
	}
	if ((({ mph_CaseClause* _obj_21 = c; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->values_count; mph_gc_pop_roots(ctx, 1); _ret_22; }) >= 3)) {
{
	if ((({ mph_CaseClause* _obj_23 = c; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->v2; mph_gc_pop_roots(ctx, 1); _ret_24; }) == val)) {
{
	return 1;
}
	}
}
	}
	return 0;
	mph_gc_pop_roots(ctx, 1);
}

mph_SwitchStatement* mph_make_switch(MorphContext* ctx, void* _env_void) {
	mph_CaseClause* empty = mph_make_case(ctx, NULL, 0);
	mph_gc_push_root(ctx, (void**)&empty);
	return ({ mph_SwitchStatement* _t = (mph_SwitchStatement*)mph_alloc(ctx, sizeof(mph_SwitchStatement), &mph_ti_mph_SwitchStatement); mph_gc_push_root(ctx, (void**)&_t); _t->c0 = empty; _t->c1 = empty; _t->has_default = 0; _t->node_type = mph_NODE_SWITCH; _t->cases_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

mph_SwitchStatement* mph_switch_add(MorphContext* ctx, void* _env_void, mph_SwitchStatement* sw, mph_CaseClause* c) {
	mph_gc_push_root(ctx, (void**)&sw);
	mph_gc_push_root(ctx, (void**)&c);
	if ((({ mph_SwitchStatement* _obj_25 = sw; mph_gc_push_root(ctx, (void**)&_obj_25); mph_int _ret_26 = _obj_25->cases_count; mph_gc_pop_roots(ctx, 1); _ret_26; }) == 0)) {
{
	({ mph_SwitchStatement* _obj_27 = sw; mph_gc_push_root(ctx, (void**)&_obj_27); mph_CaseClause* _val_28 = c; mph_gc_push_root(ctx, (void**)&_val_28); _obj_27->c0 = _val_28; mph_gc_pop_roots(ctx, 2); });
}
	} else {
{
	({ mph_SwitchStatement* _obj_29 = sw; mph_gc_push_root(ctx, (void**)&_obj_29); mph_CaseClause* _val_30 = c; mph_gc_push_root(ctx, (void**)&_val_30); _obj_29->c1 = _val_30; mph_gc_pop_roots(ctx, 2); });
}
	}
	({ mph_SwitchStatement* _obj_33 = sw; mph_gc_push_root(ctx, (void**)&_obj_33); _obj_33->cases_count = (({ mph_SwitchStatement* _obj_31 = sw; mph_gc_push_root(ctx, (void**)&_obj_31); mph_int _ret_32 = _obj_31->cases_count; mph_gc_pop_roots(ctx, 1); _ret_32; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return sw;
	mph_gc_pop_roots(ctx, 2);
}

mph_int mph_switch_eval(MorphContext* ctx, void* _env_void, mph_SwitchStatement* sw, mph_int val) {
	mph_gc_push_root(ctx, (void**)&sw);
	if ((({ mph_SwitchStatement* _obj_34 = sw; mph_gc_push_root(ctx, (void**)&_obj_34); mph_int _ret_35 = _obj_34->cases_count; mph_gc_pop_roots(ctx, 1); _ret_35; }) >= 1)) {
{
	if (({ mph_CaseClause* _arg_38 = ({ mph_SwitchStatement* _obj_36 = sw; mph_gc_push_root(ctx, (void**)&_obj_36); mph_CaseClause* _ret_37 = _obj_36->c0; mph_gc_pop_roots(ctx, 1); _ret_37; }); mph_gc_push_root(ctx, (void**)&_arg_38); mph_bool _ret_39 = mph_case_match(ctx, NULL, _arg_38, val); mph_gc_pop_roots(ctx, 1); _ret_39; })) {
{
	return 0;
}
	}
}
	}
	if ((({ mph_SwitchStatement* _obj_40 = sw; mph_gc_push_root(ctx, (void**)&_obj_40); mph_int _ret_41 = _obj_40->cases_count; mph_gc_pop_roots(ctx, 1); _ret_41; }) >= 2)) {
{
	if (({ mph_CaseClause* _arg_44 = ({ mph_SwitchStatement* _obj_42 = sw; mph_gc_push_root(ctx, (void**)&_obj_42); mph_CaseClause* _ret_43 = _obj_42->c1; mph_gc_pop_roots(ctx, 1); _ret_43; }); mph_gc_push_root(ctx, (void**)&_arg_44); mph_bool _ret_45 = mph_case_match(ctx, NULL, _arg_44, val); mph_gc_pop_roots(ctx, 1); _ret_45; })) {
{
	return 1;
}
	}
}
	}
	if (({ mph_SwitchStatement* _obj_46 = sw; mph_gc_push_root(ctx, (void**)&_obj_46); mph_bool _ret_47 = _obj_46->has_default; mph_gc_pop_roots(ctx, 1); _ret_47; })) {
{
	return (-1);
}
	}
	return (-2);
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "N1 Switch Multi-Case Test\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=========================\n\n"));
	mph_SwitchStatement* sw = mph_make_switch(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&sw);
	mph_CaseClause* c0 = mph_make_case(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&c0);
	c0 = ({ mph_CaseClause* _arg_48 = c0; mph_gc_push_root(ctx, (void**)&_arg_48); mph_CaseClause* _ret_49 = mph_case_add(ctx, NULL, _arg_48, 1); mph_gc_pop_roots(ctx, 1); _ret_49; });
	c0 = ({ mph_CaseClause* _arg_50 = c0; mph_gc_push_root(ctx, (void**)&_arg_50); mph_CaseClause* _ret_51 = mph_case_add(ctx, NULL, _arg_50, 2); mph_gc_pop_roots(ctx, 1); _ret_51; });
	c0 = ({ mph_CaseClause* _arg_52 = c0; mph_gc_push_root(ctx, (void**)&_arg_52); mph_CaseClause* _ret_53 = mph_case_add(ctx, NULL, _arg_52, 3); mph_gc_pop_roots(ctx, 1); _ret_53; });
	sw = ({ mph_SwitchStatement* _arg_54 = sw; mph_gc_push_root(ctx, (void**)&_arg_54); mph_CaseClause* _arg_55 = c0; mph_gc_push_root(ctx, (void**)&_arg_55); mph_SwitchStatement* _ret_56 = mph_switch_add(ctx, NULL, _arg_54, _arg_55); mph_gc_pop_roots(ctx, 2); _ret_56; });
	mph_CaseClause* c1 = mph_make_case(ctx, NULL, 2);
	mph_gc_push_root(ctx, (void**)&c1);
	c1 = ({ mph_CaseClause* _arg_57 = c1; mph_gc_push_root(ctx, (void**)&_arg_57); mph_CaseClause* _ret_58 = mph_case_add(ctx, NULL, _arg_57, 10); mph_gc_pop_roots(ctx, 1); _ret_58; });
	sw = ({ mph_SwitchStatement* _arg_59 = sw; mph_gc_push_root(ctx, (void**)&_arg_59); mph_CaseClause* _arg_60 = c1; mph_gc_push_root(ctx, (void**)&_arg_60); mph_SwitchStatement* _ret_61 = mph_switch_add(ctx, NULL, _arg_59, _arg_60); mph_gc_pop_roots(ctx, 2); _ret_61; });
	({ mph_SwitchStatement* _obj_62 = sw; mph_gc_push_root(ctx, (void**)&_obj_62); _obj_62->has_default = 1; mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "Multi-case test (kasus 1, 2, 3):\n"));
	if ((({ mph_SwitchStatement* _arg_63 = sw; mph_gc_push_root(ctx, (void**)&_arg_63); mph_int _ret_64 = mph_switch_eval(ctx, NULL, _arg_63, 1); mph_gc_pop_roots(ctx, 1); _ret_64; }) == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=1 matches case 0\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=1 should match case 0\n"));
}
	}
	if ((({ mph_SwitchStatement* _arg_65 = sw; mph_gc_push_root(ctx, (void**)&_arg_65); mph_int _ret_66 = mph_switch_eval(ctx, NULL, _arg_65, 2); mph_gc_pop_roots(ctx, 1); _ret_66; }) == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=2 matches case 0\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=2 should match case 0\n"));
}
	}
	if ((({ mph_SwitchStatement* _arg_67 = sw; mph_gc_push_root(ctx, (void**)&_arg_67); mph_int _ret_68 = mph_switch_eval(ctx, NULL, _arg_67, 3); mph_gc_pop_roots(ctx, 1); _ret_68; }) == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=3 matches case 0\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=3 should match case 0\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "\nSingle case test (kasus 10):\n"));
	if ((({ mph_SwitchStatement* _arg_69 = sw; mph_gc_push_root(ctx, (void**)&_arg_69); mph_int _ret_70 = mph_switch_eval(ctx, NULL, _arg_69, 10); mph_gc_pop_roots(ctx, 1); _ret_70; }) == 1)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=10 matches case 1\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=10 should match case 1\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "\nDefault test:\n"));
	if ((({ mph_SwitchStatement* _arg_71 = sw; mph_gc_push_root(ctx, (void**)&_arg_71); mph_int _ret_72 = mph_switch_eval(ctx, NULL, _arg_71, 99); mph_gc_pop_roots(ctx, 1); _ret_72; }) == (-1))) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=99 goes to default\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â val=99 should go to default\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "\nSwitch Multi-Case OK!\n"));
	mph_gc_pop_roots(ctx, 3);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_NODE_SWITCH = 21;
	mph_NODE_CASE = 22;
	mph_main(ctx, NULL);
}
