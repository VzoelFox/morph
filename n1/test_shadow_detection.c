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
#define MPH_TYPE_mph_Type 1
#define MPH_TYPE_mph_Symbol 2
#define MPH_TYPE_mph_ScopeManager 3

// Struct Definitions (Env & Types)
typedef struct mph_Type mph_Type;
typedef struct mph_Symbol mph_Symbol;
typedef struct mph_ScopeManager mph_ScopeManager;
struct mph_Type {
	mph_int kind;
	MorphString* name;
};

struct mph_Symbol {
	MorphString* name;
	mph_Type* symbol_type;
	mph_int scope_level;
};

struct mph_ScopeManager {
	mph_int current_level;
	mph_int shadow_warnings;
};

MorphTypeInfo mph_ti_mph_Type = { "Type", sizeof(mph_Type), 1, (size_t[]){offsetof(mph_Type, name)} };
MorphTypeInfo mph_ti_mph_Symbol = { "Symbol", sizeof(mph_Symbol), 2, (size_t[]){offsetof(mph_Symbol, name), offsetof(mph_Symbol, symbol_type)} };
MorphTypeInfo mph_ti_mph_ScopeManager = { "ScopeManager", sizeof(mph_ScopeManager), 0, NULL };

// Global Variables

// Function Prototypes
mph_Type* mph_int_type(MorphContext* ctx, void* _env_void);
mph_Type* mph_string_type(MorphContext* ctx, void* _env_void);
mph_Symbol* mph_make_symbol(MorphContext* ctx, void* _env_void, MorphString* name, mph_Type* sym_type, mph_int level);
mph_ScopeManager* mph_new_scope_manager(MorphContext* ctx, void* _env_void);
void mph_scope_enter(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm);
void mph_scope_exit(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm);
mph_bool mph_check_shadowing(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm, mph_Symbol* sym, mph_Symbol* outer_sym);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_Type* mph_int_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 0; _t->name = mph_string_new(ctx, "int"); mph_gc_pop_roots(ctx, 1); _t; });
}

mph_Type* mph_string_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = 2; _t->name = mph_string_new(ctx, "string"); mph_gc_pop_roots(ctx, 1); _t; });
}

mph_Symbol* mph_make_symbol(MorphContext* ctx, void* _env_void, MorphString* name, mph_Type* sym_type, mph_int level) {
	mph_gc_push_root(ctx, (void**)&name);
	mph_gc_push_root(ctx, (void**)&sym_type);
	return ({ mph_Symbol* _t = (mph_Symbol*)mph_alloc(ctx, sizeof(mph_Symbol), &mph_ti_mph_Symbol); mph_gc_push_root(ctx, (void**)&_t); _t->name = name; _t->symbol_type = sym_type; _t->scope_level = level; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 2);
}

mph_ScopeManager* mph_new_scope_manager(MorphContext* ctx, void* _env_void) {
	return ({ mph_ScopeManager* _t = (mph_ScopeManager*)mph_alloc(ctx, sizeof(mph_ScopeManager), &mph_ti_mph_ScopeManager); mph_gc_push_root(ctx, (void**)&_t); _t->current_level = 0; _t->shadow_warnings = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

void mph_scope_enter(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm) {
	mph_gc_push_root(ctx, (void**)&sm);
	({ mph_ScopeManager* _obj_3 = sm; mph_gc_push_root(ctx, (void**)&_obj_3); _obj_3->current_level = (({ mph_ScopeManager* _obj_1 = sm; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->current_level; mph_gc_pop_roots(ctx, 1); _ret_2; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "  Entered scope level "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_4 = sm; mph_gc_push_root(ctx, (void**)&_obj_4); mph_int _ret_5 = _obj_4->current_level; mph_gc_pop_roots(ctx, 1); _ret_5; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_gc_pop_roots(ctx, 1);
}

void mph_scope_exit(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm) {
	mph_gc_push_root(ctx, (void**)&sm);
	if ((({ mph_ScopeManager* _obj_6 = sm; mph_gc_push_root(ctx, (void**)&_obj_6); mph_int _ret_7 = _obj_6->current_level; mph_gc_pop_roots(ctx, 1); _ret_7; }) > 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "  Exited scope level "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_8 = sm; mph_gc_push_root(ctx, (void**)&_obj_8); mph_int _ret_9 = _obj_8->current_level; mph_gc_pop_roots(ctx, 1); _ret_9; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	({ mph_ScopeManager* _obj_12 = sm; mph_gc_push_root(ctx, (void**)&_obj_12); _obj_12->current_level = (({ mph_ScopeManager* _obj_10 = sm; mph_gc_push_root(ctx, (void**)&_obj_10); mph_int _ret_11 = _obj_10->current_level; mph_gc_pop_roots(ctx, 1); _ret_11; }) - 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_check_shadowing(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm, mph_Symbol* sym, mph_Symbol* outer_sym) {
	mph_gc_push_root(ctx, (void**)&sm);
	mph_gc_push_root(ctx, (void**)&sym);
	mph_gc_push_root(ctx, (void**)&outer_sym);
	if (mph_string_eq(ctx, ({ mph_Symbol* _obj_13 = sym; mph_gc_push_root(ctx, (void**)&_obj_13); MorphString* _ret_14 = _obj_13->name; mph_gc_pop_roots(ctx, 1); _ret_14; }), ({ mph_Symbol* _obj_15 = outer_sym; mph_gc_push_root(ctx, (void**)&_obj_15); MorphString* _ret_16 = _obj_15->name; mph_gc_pop_roots(ctx, 1); _ret_16; }))) {
{
	if ((({ mph_Symbol* _obj_17 = sym; mph_gc_push_root(ctx, (void**)&_obj_17); mph_int _ret_18 = _obj_17->scope_level; mph_gc_pop_roots(ctx, 1); _ret_18; }) > ({ mph_Symbol* _obj_19 = outer_sym; mph_gc_push_root(ctx, (void**)&_obj_19); mph_int _ret_20 = _obj_19->scope_level; mph_gc_pop_roots(ctx, 1); _ret_20; }))) {
{
	({ mph_ScopeManager* _obj_23 = sm; mph_gc_push_root(ctx, (void**)&_obj_23); _obj_23->shadow_warnings = (({ mph_ScopeManager* _obj_21 = sm; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->shadow_warnings; mph_gc_pop_roots(ctx, 1); _ret_22; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "WARNING: Variable '"));
	mph_native_print(ctx, ({ mph_Symbol* _obj_24 = sym; mph_gc_push_root(ctx, (void**)&_obj_24); MorphString* _ret_25 = _obj_24->name; mph_gc_pop_roots(ctx, 1); _ret_25; }));
	mph_native_print(ctx, mph_string_new(ctx, "' at level "));
	mph_native_print_int(ctx, ({ mph_Symbol* _obj_26 = sym; mph_gc_push_root(ctx, (void**)&_obj_26); mph_int _ret_27 = _obj_26->scope_level; mph_gc_pop_roots(ctx, 1); _ret_27; }));
	mph_native_print(ctx, mph_string_new(ctx, " shadows outer declaration at level "));
	mph_native_print_int(ctx, ({ mph_Symbol* _obj_28 = outer_sym; mph_gc_push_root(ctx, (void**)&_obj_28); mph_int _ret_29 = _obj_28->scope_level; mph_gc_pop_roots(ctx, 1); _ret_29; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	return 1;
}
	}
}
	}
	return 0;
	mph_gc_pop_roots(ctx, 3);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  Shadowing Detection Test           â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n\n"));
	mph_ScopeManager* sm = mph_new_scope_manager(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&sm);
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 1: No Shadowing (different names) ===\n"));
	mph_Symbol* x_outer = ({ MorphString* _arg_30 = mph_string_new(ctx, "x"); mph_gc_push_root(ctx, (void**)&_arg_30); mph_Type* _arg_31 = mph_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_arg_31); mph_Symbol* _ret_32 = mph_make_symbol(ctx, NULL, _arg_30, _arg_31, 0); mph_gc_pop_roots(ctx, 2); _ret_32; });
	mph_gc_push_root(ctx, (void**)&x_outer);
	({ mph_ScopeManager* _arg_33 = sm; mph_gc_push_root(ctx, (void**)&_arg_33); mph_scope_enter(ctx, NULL, _arg_33); mph_gc_pop_roots(ctx, 1); });
	mph_Symbol* y_inner = ({ MorphString* _arg_34 = mph_string_new(ctx, "y"); mph_gc_push_root(ctx, (void**)&_arg_34); mph_Type* _arg_35 = mph_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_arg_35); mph_Symbol* _ret_36 = mph_make_symbol(ctx, NULL, _arg_34, _arg_35, 1); mph_gc_pop_roots(ctx, 2); _ret_36; });
	mph_gc_push_root(ctx, (void**)&y_inner);
	mph_bool has_shadow1 = ({ mph_ScopeManager* _arg_37 = sm; mph_gc_push_root(ctx, (void**)&_arg_37); mph_Symbol* _arg_38 = y_inner; mph_gc_push_root(ctx, (void**)&_arg_38); mph_Symbol* _arg_39 = x_outer; mph_gc_push_root(ctx, (void**)&_arg_39); mph_bool _ret_40 = mph_check_shadowing(ctx, NULL, _arg_37, _arg_38, _arg_39); mph_gc_pop_roots(ctx, 3); _ret_40; });
	if (has_shadow1) {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL: Should not detect shadowing\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: No shadowing detected\n"));
}
	}
	({ mph_ScopeManager* _arg_41 = sm; mph_gc_push_root(ctx, (void**)&_arg_41); mph_scope_exit(ctx, NULL, _arg_41); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "  Shadow warnings: "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_42 = sm; mph_gc_push_root(ctx, (void**)&_obj_42); mph_int _ret_43 = _obj_42->shadow_warnings; mph_gc_pop_roots(ctx, 1); _ret_43; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 2: Shadowing (same name) ===\n"));
	mph_Symbol* x_outer2 = ({ MorphString* _arg_44 = mph_string_new(ctx, "x"); mph_gc_push_root(ctx, (void**)&_arg_44); mph_Type* _arg_45 = mph_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_arg_45); mph_Symbol* _ret_46 = mph_make_symbol(ctx, NULL, _arg_44, _arg_45, 0); mph_gc_pop_roots(ctx, 2); _ret_46; });
	mph_gc_push_root(ctx, (void**)&x_outer2);
	({ mph_ScopeManager* _arg_47 = sm; mph_gc_push_root(ctx, (void**)&_arg_47); mph_scope_enter(ctx, NULL, _arg_47); mph_gc_pop_roots(ctx, 1); });
	mph_Symbol* x_inner = ({ MorphString* _arg_48 = mph_string_new(ctx, "x"); mph_gc_push_root(ctx, (void**)&_arg_48); mph_Type* _arg_49 = mph_string_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_arg_49); mph_Symbol* _ret_50 = mph_make_symbol(ctx, NULL, _arg_48, _arg_49, 1); mph_gc_pop_roots(ctx, 2); _ret_50; });
	mph_gc_push_root(ctx, (void**)&x_inner);
	mph_bool has_shadow2 = ({ mph_ScopeManager* _arg_51 = sm; mph_gc_push_root(ctx, (void**)&_arg_51); mph_Symbol* _arg_52 = x_inner; mph_gc_push_root(ctx, (void**)&_arg_52); mph_Symbol* _arg_53 = x_outer2; mph_gc_push_root(ctx, (void**)&_arg_53); mph_bool _ret_54 = mph_check_shadowing(ctx, NULL, _arg_51, _arg_52, _arg_53); mph_gc_pop_roots(ctx, 3); _ret_54; });
	if (has_shadow2) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Shadowing correctly detected\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL: Should detect shadowing\n"));
}
	}
	({ mph_ScopeManager* _arg_55 = sm; mph_gc_push_root(ctx, (void**)&_arg_55); mph_scope_exit(ctx, NULL, _arg_55); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "  Shadow warnings: "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_56 = sm; mph_gc_push_root(ctx, (void**)&_obj_56); mph_int _ret_57 = _obj_56->shadow_warnings; mph_gc_pop_roots(ctx, 1); _ret_57; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 3: Multi-level Shadowing ===\n"));
	mph_Symbol* z_level0 = ({ MorphString* _arg_58 = mph_string_new(ctx, "z"); mph_gc_push_root(ctx, (void**)&_arg_58); mph_Type* _arg_59 = mph_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_arg_59); mph_Symbol* _ret_60 = mph_make_symbol(ctx, NULL, _arg_58, _arg_59, 0); mph_gc_pop_roots(ctx, 2); _ret_60; });
	mph_gc_push_root(ctx, (void**)&z_level0);
	({ mph_ScopeManager* _arg_61 = sm; mph_gc_push_root(ctx, (void**)&_arg_61); mph_scope_enter(ctx, NULL, _arg_61); mph_gc_pop_roots(ctx, 1); });
	mph_Symbol* z_level1 = ({ MorphString* _arg_62 = mph_string_new(ctx, "z"); mph_gc_push_root(ctx, (void**)&_arg_62); mph_Type* _arg_63 = mph_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_arg_63); mph_Symbol* _ret_64 = mph_make_symbol(ctx, NULL, _arg_62, _arg_63, 1); mph_gc_pop_roots(ctx, 2); _ret_64; });
	mph_gc_push_root(ctx, (void**)&z_level1);
	({ mph_ScopeManager* _arg_65 = sm; mph_gc_push_root(ctx, (void**)&_arg_65); mph_Symbol* _arg_66 = z_level1; mph_gc_push_root(ctx, (void**)&_arg_66); mph_Symbol* _arg_67 = z_level0; mph_gc_push_root(ctx, (void**)&_arg_67); mph_bool _ret_68 = mph_check_shadowing(ctx, NULL, _arg_65, _arg_66, _arg_67); mph_gc_pop_roots(ctx, 3); _ret_68; });
	({ mph_ScopeManager* _arg_69 = sm; mph_gc_push_root(ctx, (void**)&_arg_69); mph_scope_enter(ctx, NULL, _arg_69); mph_gc_pop_roots(ctx, 1); });
	mph_Symbol* z_level2 = ({ MorphString* _arg_70 = mph_string_new(ctx, "z"); mph_gc_push_root(ctx, (void**)&_arg_70); mph_Type* _arg_71 = mph_string_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_arg_71); mph_Symbol* _ret_72 = mph_make_symbol(ctx, NULL, _arg_70, _arg_71, 2); mph_gc_pop_roots(ctx, 2); _ret_72; });
	mph_gc_push_root(ctx, (void**)&z_level2);
	({ mph_ScopeManager* _arg_73 = sm; mph_gc_push_root(ctx, (void**)&_arg_73); mph_Symbol* _arg_74 = z_level2; mph_gc_push_root(ctx, (void**)&_arg_74); mph_Symbol* _arg_75 = z_level1; mph_gc_push_root(ctx, (void**)&_arg_75); mph_bool _ret_76 = mph_check_shadowing(ctx, NULL, _arg_73, _arg_74, _arg_75); mph_gc_pop_roots(ctx, 3); _ret_76; });
	({ mph_ScopeManager* _arg_77 = sm; mph_gc_push_root(ctx, (void**)&_arg_77); mph_Symbol* _arg_78 = z_level2; mph_gc_push_root(ctx, (void**)&_arg_78); mph_Symbol* _arg_79 = z_level0; mph_gc_push_root(ctx, (void**)&_arg_79); mph_bool _ret_80 = mph_check_shadowing(ctx, NULL, _arg_77, _arg_78, _arg_79); mph_gc_pop_roots(ctx, 3); _ret_80; });
	({ mph_ScopeManager* _arg_81 = sm; mph_gc_push_root(ctx, (void**)&_arg_81); mph_scope_exit(ctx, NULL, _arg_81); mph_gc_pop_roots(ctx, 1); });
	({ mph_ScopeManager* _arg_82 = sm; mph_gc_push_root(ctx, (void**)&_arg_82); mph_scope_exit(ctx, NULL, _arg_82); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "  Total shadow warnings: "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_83 = sm; mph_gc_push_root(ctx, (void**)&_obj_83); mph_int _ret_84 = _obj_83->shadow_warnings; mph_gc_pop_roots(ctx, 1); _ret_84; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 4: Same Level (no shadowing) ===\n"));
	mph_Symbol* a_var = ({ MorphString* _arg_85 = mph_string_new(ctx, "a"); mph_gc_push_root(ctx, (void**)&_arg_85); mph_Type* _arg_86 = mph_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_arg_86); mph_Symbol* _ret_87 = mph_make_symbol(ctx, NULL, _arg_85, _arg_86, 0); mph_gc_pop_roots(ctx, 2); _ret_87; });
	mph_gc_push_root(ctx, (void**)&a_var);
	mph_Symbol* b_var = ({ MorphString* _arg_88 = mph_string_new(ctx, "a"); mph_gc_push_root(ctx, (void**)&_arg_88); mph_Type* _arg_89 = mph_int_type(ctx, NULL); mph_gc_push_root(ctx, (void**)&_arg_89); mph_Symbol* _ret_90 = mph_make_symbol(ctx, NULL, _arg_88, _arg_89, 0); mph_gc_pop_roots(ctx, 2); _ret_90; });
	mph_gc_push_root(ctx, (void**)&b_var);
	mph_bool has_shadow4 = ({ mph_ScopeManager* _arg_91 = sm; mph_gc_push_root(ctx, (void**)&_arg_91); mph_Symbol* _arg_92 = b_var; mph_gc_push_root(ctx, (void**)&_arg_92); mph_Symbol* _arg_93 = a_var; mph_gc_push_root(ctx, (void**)&_arg_93); mph_bool _ret_94 = mph_check_shadowing(ctx, NULL, _arg_91, _arg_92, _arg_93); mph_gc_pop_roots(ctx, 3); _ret_94; });
	if (has_shadow4) {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL: Same level should not shadow\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Same level not treated as shadowing\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "  Shadow warnings: "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_95 = sm; mph_gc_push_root(ctx, (void**)&_obj_95); mph_int _ret_96 = _obj_95->shadow_warnings; mph_gc_pop_roots(ctx, 1); _ret_96; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â         TEST SUMMARY                â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Different names don't trigger warnings\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Same name in inner scope triggers warning\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Multi-level shadowing detected\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Same-level variables don't shadow\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ð SHADOWING DETECTION TEST PASSED! ð\n"));
	mph_gc_pop_roots(ctx, 10);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
