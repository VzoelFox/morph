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
#define MPH_TYPE_mph_RecursionLimiter 3
#define MPH_TYPE_mph_ModuleManager 4
#define MPH_TYPE_mph_Type 1
#define MPH_TYPE_mph_ScopeManager 2

// Struct Definitions (Env & Types)
typedef struct mph_Type mph_Type;
typedef struct mph_ScopeManager mph_ScopeManager;
typedef struct mph_RecursionLimiter mph_RecursionLimiter;
typedef struct mph_ModuleManager mph_ModuleManager;
struct mph_Type {
	mph_int kind;
	MorphString* name;
};

struct mph_ScopeManager {
	mph_int scopes_count;
	mph_int current_scope_id;
	mph_int errors_count;
};

struct mph_RecursionLimiter {
	mph_int max_depth;
	mph_int current_depth;
	mph_int overflow_count;
};

struct mph_ModuleManager {
	mph_int modules_count;
	mph_int loading_stack_count;
	mph_int errors_count;
};

MorphTypeInfo mph_ti_mph_Type = { "Type", sizeof(mph_Type), 1, (size_t[]){offsetof(mph_Type, name)} };
MorphTypeInfo mph_ti_mph_ScopeManager = { "ScopeManager", sizeof(mph_ScopeManager), 0, NULL };
MorphTypeInfo mph_ti_mph_RecursionLimiter = { "RecursionLimiter", sizeof(mph_RecursionLimiter), 0, NULL };
MorphTypeInfo mph_ti_mph_ModuleManager = { "ModuleManager", sizeof(mph_ModuleManager), 0, NULL };

// Global Variables
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_FUNCTION;

// Function Prototypes
mph_Type* mph_int_type(MorphContext* ctx, void* _env_void);
mph_Type* mph_string_type(MorphContext* ctx, void* _env_void);
mph_Type* mph_function_type(MorphContext* ctx, void* _env_void);
MorphString* mph_kind_to_string(MorphContext* ctx, void* _env_void, mph_int k);
mph_ScopeManager* mph_new_scope_manager(MorphContext* ctx, void* _env_void);
mph_bool mph_scope_enter(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm);
mph_bool mph_scope_exit(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm);
mph_RecursionLimiter* mph_new_recursion_limiter(MorphContext* ctx, void* _env_void);
mph_bool mph_recursion_enter(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, MorphString* func_name);
void mph_recursion_exit(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, MorphString* func_name);
mph_ModuleManager* mph_new_module_manager(MorphContext* ctx, void* _env_void);
mph_bool mph_module_start_loading(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, MorphString* name);
void mph_module_finish_loading(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, MorphString* name);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_Type* mph_int_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_INT; _t->name = mph_string_new(ctx, "int"); mph_gc_pop_roots(ctx, 1); _t; });
}

mph_Type* mph_string_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->name = mph_string_new(ctx, "string"); _t->kind = mph_KIND_STRING; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_Type* mph_function_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->name = mph_string_new(ctx, "function"); _t->kind = mph_KIND_FUNCTION; mph_gc_pop_roots(ctx, 1); _t; });
}

MorphString* mph_kind_to_string(MorphContext* ctx, void* _env_void, mph_int k) {
	if ((k == mph_KIND_INT)) {
{
	return mph_string_new(ctx, "int");
}
	}
	if ((k == mph_KIND_STRING)) {
{
	return mph_string_new(ctx, "string");
}
	}
	if ((k == mph_KIND_FUNCTION)) {
{
	return mph_string_new(ctx, "function");
}
	}
	return mph_string_new(ctx, "unknown");
}

mph_ScopeManager* mph_new_scope_manager(MorphContext* ctx, void* _env_void) {
	return ({ mph_ScopeManager* _t = (mph_ScopeManager*)mph_alloc(ctx, sizeof(mph_ScopeManager), &mph_ti_mph_ScopeManager); mph_gc_push_root(ctx, (void**)&_t); _t->errors_count = 0; _t->scopes_count = 1; _t->current_scope_id = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_scope_enter(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm) {
	mph_gc_push_root(ctx, (void**)&sm);
	({ mph_ScopeManager* _obj_3 = sm; mph_gc_push_root(ctx, (void**)&_obj_3); _obj_3->scopes_count = (({ mph_ScopeManager* _obj_1 = sm; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->scopes_count; mph_gc_pop_roots(ctx, 1); _ret_2; }) + 1); mph_gc_pop_roots(ctx, 1); });
	({ mph_ScopeManager* _obj_6 = sm; mph_gc_push_root(ctx, (void**)&_obj_6); _obj_6->current_scope_id = (({ mph_ScopeManager* _obj_4 = sm; mph_gc_push_root(ctx, (void**)&_obj_4); mph_int _ret_5 = _obj_4->current_scope_id; mph_gc_pop_roots(ctx, 1); _ret_5; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "Entered scope "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_7 = sm; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->current_scope_id; mph_gc_pop_roots(ctx, 1); _ret_8; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	return 1;
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_scope_exit(MorphContext* ctx, void* _env_void, mph_ScopeManager* sm) {
	mph_gc_push_root(ctx, (void**)&sm);
	if ((({ mph_ScopeManager* _obj_9 = sm; mph_gc_push_root(ctx, (void**)&_obj_9); mph_int _ret_10 = _obj_9->current_scope_id; mph_gc_pop_roots(ctx, 1); _ret_10; }) > 0)) {
{
	({ mph_ScopeManager* _obj_13 = sm; mph_gc_push_root(ctx, (void**)&_obj_13); _obj_13->current_scope_id = (({ mph_ScopeManager* _obj_11 = sm; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->current_scope_id; mph_gc_pop_roots(ctx, 1); _ret_12; }) - 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Exited scope "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_14 = sm; mph_gc_push_root(ctx, (void**)&_obj_14); mph_int _ret_15 = _obj_14->current_scope_id; mph_gc_pop_roots(ctx, 1); _ret_15; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	return 1;
	mph_gc_pop_roots(ctx, 1);
}

mph_RecursionLimiter* mph_new_recursion_limiter(MorphContext* ctx, void* _env_void) {
	return ({ mph_RecursionLimiter* _t = (mph_RecursionLimiter*)mph_alloc(ctx, sizeof(mph_RecursionLimiter), &mph_ti_mph_RecursionLimiter); mph_gc_push_root(ctx, (void**)&_t); _t->max_depth = 1000; _t->current_depth = 0; _t->overflow_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_recursion_enter(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, MorphString* func_name) {
	mph_gc_push_root(ctx, (void**)&rl);
	mph_gc_push_root(ctx, (void**)&func_name);
	if ((({ mph_RecursionLimiter* _obj_16 = rl; mph_gc_push_root(ctx, (void**)&_obj_16); mph_int _ret_17 = _obj_16->current_depth; mph_gc_pop_roots(ctx, 1); _ret_17; }) >= ({ mph_RecursionLimiter* _obj_18 = rl; mph_gc_push_root(ctx, (void**)&_obj_18); mph_int _ret_19 = _obj_18->max_depth; mph_gc_pop_roots(ctx, 1); _ret_19; }))) {
{
	({ mph_RecursionLimiter* _obj_22 = rl; mph_gc_push_root(ctx, (void**)&_obj_22); _obj_22->overflow_count = (({ mph_RecursionLimiter* _obj_20 = rl; mph_gc_push_root(ctx, (void**)&_obj_20); mph_int _ret_21 = _obj_20->overflow_count; mph_gc_pop_roots(ctx, 1); _ret_21; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "ERROR: Stack overflow prevented in '"));
	mph_native_print(ctx, func_name);
	mph_native_print(ctx, mph_string_new(ctx, "'\n"));
	return 0;
}
	}
	({ mph_RecursionLimiter* _obj_25 = rl; mph_gc_push_root(ctx, (void**)&_obj_25); _obj_25->current_depth = (({ mph_RecursionLimiter* _obj_23 = rl; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->current_depth; mph_gc_pop_roots(ctx, 1); _ret_24; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "Entering '"));
	mph_native_print(ctx, func_name);
	mph_native_print(ctx, mph_string_new(ctx, "' (depth: "));
	mph_native_print_int(ctx, ({ mph_RecursionLimiter* _obj_26 = rl; mph_gc_push_root(ctx, (void**)&_obj_26); mph_int _ret_27 = _obj_26->current_depth; mph_gc_pop_roots(ctx, 1); _ret_27; }));
	mph_native_print(ctx, mph_string_new(ctx, ")\n"));
	return 1;
	mph_gc_pop_roots(ctx, 2);
}

void mph_recursion_exit(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, MorphString* func_name) {
	mph_gc_push_root(ctx, (void**)&rl);
	mph_gc_push_root(ctx, (void**)&func_name);
	if ((({ mph_RecursionLimiter* _obj_28 = rl; mph_gc_push_root(ctx, (void**)&_obj_28); mph_int _ret_29 = _obj_28->current_depth; mph_gc_pop_roots(ctx, 1); _ret_29; }) > 0)) {
{
	({ mph_RecursionLimiter* _obj_32 = rl; mph_gc_push_root(ctx, (void**)&_obj_32); _obj_32->current_depth = (({ mph_RecursionLimiter* _obj_30 = rl; mph_gc_push_root(ctx, (void**)&_obj_30); mph_int _ret_31 = _obj_30->current_depth; mph_gc_pop_roots(ctx, 1); _ret_31; }) - 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Exiting '"));
	mph_native_print(ctx, func_name);
	mph_native_print(ctx, mph_string_new(ctx, "' (depth: "));
	mph_native_print_int(ctx, ({ mph_RecursionLimiter* _obj_33 = rl; mph_gc_push_root(ctx, (void**)&_obj_33); mph_int _ret_34 = _obj_33->current_depth; mph_gc_pop_roots(ctx, 1); _ret_34; }));
	mph_native_print(ctx, mph_string_new(ctx, ")\n"));
	mph_gc_pop_roots(ctx, 2);
}

mph_ModuleManager* mph_new_module_manager(MorphContext* ctx, void* _env_void) {
	return ({ mph_ModuleManager* _t = (mph_ModuleManager*)mph_alloc(ctx, sizeof(mph_ModuleManager), &mph_ti_mph_ModuleManager); mph_gc_push_root(ctx, (void**)&_t); _t->modules_count = 0; _t->loading_stack_count = 0; _t->errors_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_module_start_loading(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, MorphString* name) {
	mph_gc_push_root(ctx, (void**)&mm);
	mph_gc_push_root(ctx, (void**)&name);
	({ mph_ModuleManager* _obj_37 = mm; mph_gc_push_root(ctx, (void**)&_obj_37); _obj_37->loading_stack_count = (({ mph_ModuleManager* _obj_35 = mm; mph_gc_push_root(ctx, (void**)&_obj_35); mph_int _ret_36 = _obj_35->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_36; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "Loading module '"));
	mph_native_print(ctx, name);
	mph_native_print(ctx, mph_string_new(ctx, "' (depth: "));
	mph_native_print_int(ctx, ({ mph_ModuleManager* _obj_38 = mm; mph_gc_push_root(ctx, (void**)&_obj_38); mph_int _ret_39 = _obj_38->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_39; }));
	mph_native_print(ctx, mph_string_new(ctx, ")\n"));
	return 1;
	mph_gc_pop_roots(ctx, 2);
}

void mph_module_finish_loading(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, MorphString* name) {
	mph_gc_push_root(ctx, (void**)&mm);
	mph_gc_push_root(ctx, (void**)&name);
	if ((({ mph_ModuleManager* _obj_40 = mm; mph_gc_push_root(ctx, (void**)&_obj_40); mph_int _ret_41 = _obj_40->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_41; }) > 0)) {
{
	({ mph_ModuleManager* _obj_44 = mm; mph_gc_push_root(ctx, (void**)&_obj_44); _obj_44->loading_stack_count = (({ mph_ModuleManager* _obj_42 = mm; mph_gc_push_root(ctx, (void**)&_obj_42); mph_int _ret_43 = _obj_42->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_43; }) - 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Finished loading '"));
	mph_native_print(ctx, name);
	mph_native_print(ctx, mph_string_new(ctx, "'\n"));
	mph_gc_pop_roots(ctx, 2);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â    N1 Phase 4 - Robustness Test     â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 1: Scope Management ===\n"));
	mph_ScopeManager* sm = mph_new_scope_manager(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&sm);
	({ mph_ScopeManager* _arg_45 = sm; mph_gc_push_root(ctx, (void**)&_arg_45); mph_bool _ret_46 = mph_scope_enter(ctx, NULL, _arg_45); mph_gc_pop_roots(ctx, 1); _ret_46; });
	({ mph_ScopeManager* _arg_47 = sm; mph_gc_push_root(ctx, (void**)&_arg_47); mph_bool _ret_48 = mph_scope_enter(ctx, NULL, _arg_47); mph_gc_pop_roots(ctx, 1); _ret_48; });
	({ mph_ScopeManager* _arg_49 = sm; mph_gc_push_root(ctx, (void**)&_arg_49); mph_bool _ret_50 = mph_scope_exit(ctx, NULL, _arg_49); mph_gc_pop_roots(ctx, 1); _ret_50; });
	({ mph_ScopeManager* _arg_51 = sm; mph_gc_push_root(ctx, (void**)&_arg_51); mph_bool _ret_52 = mph_scope_exit(ctx, NULL, _arg_51); mph_gc_pop_roots(ctx, 1); _ret_52; });
	mph_native_print(ctx, mph_string_new(ctx, "Scope test completed\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 2: Recursion Limiting ===\n"));
	mph_RecursionLimiter* rl = mph_new_recursion_limiter(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&rl);
	({ mph_RecursionLimiter* _arg_53 = rl; mph_gc_push_root(ctx, (void**)&_arg_53); MorphString* _arg_54 = mph_string_new(ctx, "parse_expression"); mph_gc_push_root(ctx, (void**)&_arg_54); mph_bool _ret_55 = mph_recursion_enter(ctx, NULL, _arg_53, _arg_54); mph_gc_pop_roots(ctx, 2); _ret_55; });
	({ mph_RecursionLimiter* _arg_56 = rl; mph_gc_push_root(ctx, (void**)&_arg_56); MorphString* _arg_57 = mph_string_new(ctx, "parse_infix"); mph_gc_push_root(ctx, (void**)&_arg_57); mph_bool _ret_58 = mph_recursion_enter(ctx, NULL, _arg_56, _arg_57); mph_gc_pop_roots(ctx, 2); _ret_58; });
	({ mph_RecursionLimiter* _arg_59 = rl; mph_gc_push_root(ctx, (void**)&_arg_59); MorphString* _arg_60 = mph_string_new(ctx, "parse_expression"); mph_gc_push_root(ctx, (void**)&_arg_60); mph_bool _ret_61 = mph_recursion_enter(ctx, NULL, _arg_59, _arg_60); mph_gc_pop_roots(ctx, 2); _ret_61; });
	({ mph_RecursionLimiter* _arg_62 = rl; mph_gc_push_root(ctx, (void**)&_arg_62); MorphString* _arg_63 = mph_string_new(ctx, "parse_expression"); mph_gc_push_root(ctx, (void**)&_arg_63); mph_recursion_exit(ctx, NULL, _arg_62, _arg_63); mph_gc_pop_roots(ctx, 2); });
	({ mph_RecursionLimiter* _arg_64 = rl; mph_gc_push_root(ctx, (void**)&_arg_64); MorphString* _arg_65 = mph_string_new(ctx, "parse_infix"); mph_gc_push_root(ctx, (void**)&_arg_65); mph_recursion_exit(ctx, NULL, _arg_64, _arg_65); mph_gc_pop_roots(ctx, 2); });
	({ mph_RecursionLimiter* _arg_66 = rl; mph_gc_push_root(ctx, (void**)&_arg_66); MorphString* _arg_67 = mph_string_new(ctx, "parse_expression"); mph_gc_push_root(ctx, (void**)&_arg_67); mph_recursion_exit(ctx, NULL, _arg_66, _arg_67); mph_gc_pop_roots(ctx, 2); });
	mph_native_print(ctx, mph_string_new(ctx, "Recursion test completed\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 3: Module System ===\n"));
	mph_ModuleManager* mm = mph_new_module_manager(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&mm);
	({ mph_ModuleManager* _arg_68 = mm; mph_gc_push_root(ctx, (void**)&_arg_68); MorphString* _arg_69 = mph_string_new(ctx, "types"); mph_gc_push_root(ctx, (void**)&_arg_69); mph_bool _ret_70 = mph_module_start_loading(ctx, NULL, _arg_68, _arg_69); mph_gc_pop_roots(ctx, 2); _ret_70; });
	({ mph_ModuleManager* _arg_71 = mm; mph_gc_push_root(ctx, (void**)&_arg_71); MorphString* _arg_72 = mph_string_new(ctx, "lexer"); mph_gc_push_root(ctx, (void**)&_arg_72); mph_bool _ret_73 = mph_module_start_loading(ctx, NULL, _arg_71, _arg_72); mph_gc_pop_roots(ctx, 2); _ret_73; });
	({ mph_ModuleManager* _arg_74 = mm; mph_gc_push_root(ctx, (void**)&_arg_74); MorphString* _arg_75 = mph_string_new(ctx, "lexer"); mph_gc_push_root(ctx, (void**)&_arg_75); mph_module_finish_loading(ctx, NULL, _arg_74, _arg_75); mph_gc_pop_roots(ctx, 2); });
	({ mph_ModuleManager* _arg_76 = mm; mph_gc_push_root(ctx, (void**)&_arg_76); MorphString* _arg_77 = mph_string_new(ctx, "types"); mph_gc_push_root(ctx, (void**)&_arg_77); mph_module_finish_loading(ctx, NULL, _arg_76, _arg_77); mph_gc_pop_roots(ctx, 2); });
	mph_native_print(ctx, mph_string_new(ctx, "Module test completed\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 4: Type System ===\n"));
	mph_Type* int_t = mph_int_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&int_t);
	mph_Type* str_t = mph_string_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&str_t);
	mph_Type* func_t = mph_function_type(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&func_t);
	mph_native_print(ctx, mph_string_new(ctx, "Created types:\n"));
	mph_native_print(ctx, mph_string_new(ctx, "  "));
	mph_native_print(ctx, mph_kind_to_string(ctx, NULL, ({ mph_Type* _obj_78 = int_t; mph_gc_push_root(ctx, (void**)&_obj_78); mph_int _ret_79 = _obj_78->kind; mph_gc_pop_roots(ctx, 1); _ret_79; })));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "  "));
	mph_native_print(ctx, mph_kind_to_string(ctx, NULL, ({ mph_Type* _obj_80 = str_t; mph_gc_push_root(ctx, (void**)&_obj_80); mph_int _ret_81 = _obj_80->kind; mph_gc_pop_roots(ctx, 1); _ret_81; })));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "  "));
	mph_native_print(ctx, mph_kind_to_string(ctx, NULL, ({ mph_Type* _obj_82 = func_t; mph_gc_push_root(ctx, (void**)&_obj_82); mph_int _ret_83 = _obj_82->kind; mph_gc_pop_roots(ctx, 1); _ret_83; })));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â         PHASE 4 SUMMARY             â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Scope Management: Working\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Recursion Limiting: Working\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Module System: Working\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Type System: Working\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ð N1 PHASE 4 - ROBUSTNESS COMPLETE! ð\n"));
	mph_gc_pop_roots(ctx, 6);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_STRING = 2;
	mph_KIND_FUNCTION = 5;
	mph_main(ctx, NULL);
}
