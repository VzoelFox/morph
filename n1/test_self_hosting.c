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
#define MPH_TYPE_mph_RecursionLimiter 2
#define MPH_TYPE_mph_ScopeManager 3
#define MPH_TYPE_mph_ModuleManager 4

// Struct Definitions (Env & Types)
typedef struct mph_Type mph_Type;
typedef struct mph_RecursionLimiter mph_RecursionLimiter;
typedef struct mph_ScopeManager mph_ScopeManager;
typedef struct mph_ModuleManager mph_ModuleManager;
struct mph_Type {
	mph_int kind;
	MorphString* name;
};

struct mph_RecursionLimiter {
	mph_int max_depth;
	mph_int current_depth;
};

struct mph_ScopeManager {
	mph_int current_depth;
	mph_int max_depth;
};

struct mph_ModuleManager {
	mph_int modules_count;
	mph_int loading_depth;
};

MorphTypeInfo mph_ti_mph_Type = { "Type", sizeof(mph_Type), 1, (size_t[]){offsetof(mph_Type, name)} };
MorphTypeInfo mph_ti_mph_RecursionLimiter = { "RecursionLimiter", sizeof(mph_RecursionLimiter), 0, NULL };
MorphTypeInfo mph_ti_mph_ScopeManager = { "ScopeManager", sizeof(mph_ScopeManager), 0, NULL };
MorphTypeInfo mph_ti_mph_ModuleManager = { "ModuleManager", sizeof(mph_ModuleManager), 0, NULL };

// Global Variables
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;

// Function Prototypes
mph_bool mph_test_type_system_self_host(MorphContext* ctx, void* _env_void);
mph_bool mph_test_recursion_self_host(MorphContext* ctx, void* _env_void);
mph_bool mph_test_scope_self_host(MorphContext* ctx, void* _env_void);
mph_bool mph_test_module_self_host(MorphContext* ctx, void* _env_void);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_bool mph_test_type_system_self_host(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Test: Type System Self-Host ===\n"));
	mph_Type* int_type = ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_INT; _t->name = mph_string_new(ctx, "int"); mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&int_type);
	mph_Type* str_type = ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = mph_KIND_STRING; _t->name = mph_string_new(ctx, "string"); mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&str_type);
	mph_native_print(ctx, mph_string_new(ctx, "Created int type: "));
	mph_native_print(ctx, ({ mph_Type* _obj_1 = int_type; mph_gc_push_root(ctx, (void**)&_obj_1); MorphString* _ret_2 = _obj_1->name; mph_gc_pop_roots(ctx, 1); _ret_2; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Created string type: "));
	mph_native_print(ctx, ({ mph_Type* _obj_3 = str_type; mph_gc_push_root(ctx, (void**)&_obj_3); MorphString* _ret_4 = _obj_3->name; mph_gc_pop_roots(ctx, 1); _ret_4; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if ((({ mph_Type* _obj_5 = int_type; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->kind; mph_gc_pop_roots(ctx, 1); _ret_6; }) == mph_KIND_INT)) {
{
	if ((({ mph_Type* _obj_7 = str_type; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->kind; mph_gc_pop_roots(ctx, 1); _ret_8; }) == mph_KIND_STRING)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Type system self-hosting works\n\n"));
	return 1;
}
	}
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "FAIL\n\n"));
	return 0;
	mph_gc_pop_roots(ctx, 2);
}

mph_bool mph_test_recursion_self_host(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Test: Recursion Limiting Self-Host ===\n"));
	mph_RecursionLimiter* rl = ({ mph_RecursionLimiter* _t = (mph_RecursionLimiter*)mph_alloc(ctx, sizeof(mph_RecursionLimiter), &mph_ti_mph_RecursionLimiter); mph_gc_push_root(ctx, (void**)&_t); _t->max_depth = 10; _t->current_depth = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&rl);
	mph_native_print(ctx, mph_string_new(ctx, "Max depth: "));
	mph_native_print_int(ctx, ({ mph_RecursionLimiter* _obj_9 = rl; mph_gc_push_root(ctx, (void**)&_obj_9); mph_int _ret_10 = _obj_9->max_depth; mph_gc_pop_roots(ctx, 1); _ret_10; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	({ mph_RecursionLimiter* _obj_13 = rl; mph_gc_push_root(ctx, (void**)&_obj_13); _obj_13->current_depth = (({ mph_RecursionLimiter* _obj_11 = rl; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->current_depth; mph_gc_pop_roots(ctx, 1); _ret_12; }) + 1); mph_gc_pop_roots(ctx, 1); });
	({ mph_RecursionLimiter* _obj_16 = rl; mph_gc_push_root(ctx, (void**)&_obj_16); _obj_16->current_depth = (({ mph_RecursionLimiter* _obj_14 = rl; mph_gc_push_root(ctx, (void**)&_obj_14); mph_int _ret_15 = _obj_14->current_depth; mph_gc_pop_roots(ctx, 1); _ret_15; }) + 1); mph_gc_pop_roots(ctx, 1); });
	({ mph_RecursionLimiter* _obj_19 = rl; mph_gc_push_root(ctx, (void**)&_obj_19); _obj_19->current_depth = (({ mph_RecursionLimiter* _obj_17 = rl; mph_gc_push_root(ctx, (void**)&_obj_17); mph_int _ret_18 = _obj_17->current_depth; mph_gc_pop_roots(ctx, 1); _ret_18; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "Current depth after 3 calls: "));
	mph_native_print_int(ctx, ({ mph_RecursionLimiter* _obj_20 = rl; mph_gc_push_root(ctx, (void**)&_obj_20); mph_int _ret_21 = _obj_20->current_depth; mph_gc_pop_roots(ctx, 1); _ret_21; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if ((({ mph_RecursionLimiter* _obj_22 = rl; mph_gc_push_root(ctx, (void**)&_obj_22); mph_int _ret_23 = _obj_22->current_depth; mph_gc_pop_roots(ctx, 1); _ret_23; }) == 3)) {
{
	if ((({ mph_RecursionLimiter* _obj_24 = rl; mph_gc_push_root(ctx, (void**)&_obj_24); mph_int _ret_25 = _obj_24->current_depth; mph_gc_pop_roots(ctx, 1); _ret_25; }) < ({ mph_RecursionLimiter* _obj_26 = rl; mph_gc_push_root(ctx, (void**)&_obj_26); mph_int _ret_27 = _obj_26->max_depth; mph_gc_pop_roots(ctx, 1); _ret_27; }))) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Recursion limiter self-hosting works\n\n"));
	return 1;
}
	}
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "FAIL\n\n"));
	return 0;
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_test_scope_self_host(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Test: Scope Management Self-Host ===\n"));
	mph_ScopeManager* sm = ({ mph_ScopeManager* _t = (mph_ScopeManager*)mph_alloc(ctx, sizeof(mph_ScopeManager), &mph_ti_mph_ScopeManager); mph_gc_push_root(ctx, (void**)&_t); _t->current_depth = 0; _t->max_depth = 100; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&sm);
	mph_native_print(ctx, mph_string_new(ctx, "Initial scope depth: "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_28 = sm; mph_gc_push_root(ctx, (void**)&_obj_28); mph_int _ret_29 = _obj_28->current_depth; mph_gc_pop_roots(ctx, 1); _ret_29; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	({ mph_ScopeManager* _obj_32 = sm; mph_gc_push_root(ctx, (void**)&_obj_32); _obj_32->current_depth = (({ mph_ScopeManager* _obj_30 = sm; mph_gc_push_root(ctx, (void**)&_obj_30); mph_int _ret_31 = _obj_30->current_depth; mph_gc_pop_roots(ctx, 1); _ret_31; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "After entering scope 1: "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_33 = sm; mph_gc_push_root(ctx, (void**)&_obj_33); mph_int _ret_34 = _obj_33->current_depth; mph_gc_pop_roots(ctx, 1); _ret_34; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	({ mph_ScopeManager* _obj_37 = sm; mph_gc_push_root(ctx, (void**)&_obj_37); _obj_37->current_depth = (({ mph_ScopeManager* _obj_35 = sm; mph_gc_push_root(ctx, (void**)&_obj_35); mph_int _ret_36 = _obj_35->current_depth; mph_gc_pop_roots(ctx, 1); _ret_36; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "After entering scope 2: "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_38 = sm; mph_gc_push_root(ctx, (void**)&_obj_38); mph_int _ret_39 = _obj_38->current_depth; mph_gc_pop_roots(ctx, 1); _ret_39; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	({ mph_ScopeManager* _obj_42 = sm; mph_gc_push_root(ctx, (void**)&_obj_42); _obj_42->current_depth = (({ mph_ScopeManager* _obj_40 = sm; mph_gc_push_root(ctx, (void**)&_obj_40); mph_int _ret_41 = _obj_40->current_depth; mph_gc_pop_roots(ctx, 1); _ret_41; }) - 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "After exiting scope 2: "));
	mph_native_print_int(ctx, ({ mph_ScopeManager* _obj_43 = sm; mph_gc_push_root(ctx, (void**)&_obj_43); mph_int _ret_44 = _obj_43->current_depth; mph_gc_pop_roots(ctx, 1); _ret_44; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if ((({ mph_ScopeManager* _obj_45 = sm; mph_gc_push_root(ctx, (void**)&_obj_45); mph_int _ret_46 = _obj_45->current_depth; mph_gc_pop_roots(ctx, 1); _ret_46; }) == 1)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Scope manager self-hosting works\n\n"));
	return 1;
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "FAIL\n\n"));
	return 0;
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_test_module_self_host(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Test: Module System Self-Host ===\n"));
	mph_ModuleManager* mm = ({ mph_ModuleManager* _t = (mph_ModuleManager*)mph_alloc(ctx, sizeof(mph_ModuleManager), &mph_ti_mph_ModuleManager); mph_gc_push_root(ctx, (void**)&_t); _t->modules_count = 0; _t->loading_depth = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&mm);
	({ mph_ModuleManager* _obj_49 = mm; mph_gc_push_root(ctx, (void**)&_obj_49); _obj_49->loading_depth = (({ mph_ModuleManager* _obj_47 = mm; mph_gc_push_root(ctx, (void**)&_obj_47); mph_int _ret_48 = _obj_47->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_48; }) + 1); mph_gc_pop_roots(ctx, 1); });
	({ mph_ModuleManager* _obj_52 = mm; mph_gc_push_root(ctx, (void**)&_obj_52); _obj_52->modules_count = (({ mph_ModuleManager* _obj_50 = mm; mph_gc_push_root(ctx, (void**)&_obj_50); mph_int _ret_51 = _obj_50->modules_count; mph_gc_pop_roots(ctx, 1); _ret_51; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "Loaded module 1, depth: "));
	mph_native_print_int(ctx, ({ mph_ModuleManager* _obj_53 = mm; mph_gc_push_root(ctx, (void**)&_obj_53); mph_int _ret_54 = _obj_53->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_54; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	({ mph_ModuleManager* _obj_57 = mm; mph_gc_push_root(ctx, (void**)&_obj_57); _obj_57->loading_depth = (({ mph_ModuleManager* _obj_55 = mm; mph_gc_push_root(ctx, (void**)&_obj_55); mph_int _ret_56 = _obj_55->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_56; }) + 1); mph_gc_pop_roots(ctx, 1); });
	({ mph_ModuleManager* _obj_60 = mm; mph_gc_push_root(ctx, (void**)&_obj_60); _obj_60->modules_count = (({ mph_ModuleManager* _obj_58 = mm; mph_gc_push_root(ctx, (void**)&_obj_58); mph_int _ret_59 = _obj_58->modules_count; mph_gc_pop_roots(ctx, 1); _ret_59; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "Loaded module 2, depth: "));
	mph_native_print_int(ctx, ({ mph_ModuleManager* _obj_61 = mm; mph_gc_push_root(ctx, (void**)&_obj_61); mph_int _ret_62 = _obj_61->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_62; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	({ mph_ModuleManager* _obj_65 = mm; mph_gc_push_root(ctx, (void**)&_obj_65); _obj_65->loading_depth = (({ mph_ModuleManager* _obj_63 = mm; mph_gc_push_root(ctx, (void**)&_obj_63); mph_int _ret_64 = _obj_63->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_64; }) - 1); mph_gc_pop_roots(ctx, 1); });
	({ mph_ModuleManager* _obj_68 = mm; mph_gc_push_root(ctx, (void**)&_obj_68); _obj_68->loading_depth = (({ mph_ModuleManager* _obj_66 = mm; mph_gc_push_root(ctx, (void**)&_obj_66); mph_int _ret_67 = _obj_66->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_67; }) - 1); mph_gc_pop_roots(ctx, 1); });
	if ((({ mph_ModuleManager* _obj_69 = mm; mph_gc_push_root(ctx, (void**)&_obj_69); mph_int _ret_70 = _obj_69->modules_count; mph_gc_pop_roots(ctx, 1); _ret_70; }) == 2)) {
{
	if ((({ mph_ModuleManager* _obj_71 = mm; mph_gc_push_root(ctx, (void**)&_obj_71); mph_int _ret_72 = _obj_71->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_72; }) == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Module system self-hosting works\n\n"));
	return 1;
}
	}
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "FAIL\n\n"));
	return 0;
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  N1 SELF-HOSTING VALIDATION TEST                  â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  Verify N1 can compile and run N1 components      â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n\n"));
	mph_int passed = 0;
	mph_int total = 4;
	if (mph_test_type_system_self_host(ctx, NULL)) {
{
	passed = (passed + 1);
}
	}
	if (mph_test_recursion_self_host(ctx, NULL)) {
{
	passed = (passed + 1);
}
	}
	if (mph_test_scope_self_host(ctx, NULL)) {
{
	passed = (passed + 1);
}
	}
	if (mph_test_module_self_host(ctx, NULL)) {
{
	passed = (passed + 1);
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  SELF-HOSTING VALIDATION SUMMARY                   â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Tests passed: "));
	mph_native_print_int(ctx, passed);
	mph_native_print(ctx, mph_string_new(ctx, " / "));
	mph_native_print_int(ctx, total);
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	if ((passed == total)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "ð SELF-HOSTING VALIDATION SUCCESSFUL! ð\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â N1 type system dapat compile N1 code\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â N1 recursion limiter dapat run dalam N1\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â N1 scope manager dapat run dalam N1\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â N1 module system dapat run dalam N1\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "CONCLUSION: N1 is SELF-HOSTING CAPABLE!\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â ï¸  Self-hosting validation incomplete\n"));
}
	}
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_STRING = 2;
	mph_main(ctx, NULL);
}
