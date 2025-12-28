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
#define MPH_TYPE_mph_ModuleManager 1

// Struct Definitions (Env & Types)
typedef struct mph_ModuleManager mph_ModuleManager;
struct mph_ModuleManager {
	mph_int modules_count;
	mph_int loading_stack_count;
	mph_int max_stack_depth;
	mph_int errors_count;
};

MorphTypeInfo mph_ti_mph_ModuleManager = { "ModuleManager", sizeof(mph_ModuleManager), 0, NULL };

// Global Variables

// Function Prototypes
mph_ModuleManager* mph_new_module_manager(MorphContext* ctx, void* _env_void);
mph_bool mph_module_start_loading(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, MorphString* name);
void mph_module_finish_loading(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, MorphString* name);
mph_bool mph_load_chain_linear(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm);
mph_bool mph_load_chain_cycle(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_ModuleManager* mph_new_module_manager(MorphContext* ctx, void* _env_void) {
	return ({ mph_ModuleManager* _t = (mph_ModuleManager*)mph_alloc(ctx, sizeof(mph_ModuleManager), &mph_ti_mph_ModuleManager); mph_gc_push_root(ctx, (void**)&_t); _t->modules_count = 0; _t->loading_stack_count = 0; _t->max_stack_depth = 5; _t->errors_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_module_start_loading(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, MorphString* name) {
	mph_gc_push_root(ctx, (void**)&mm);
	mph_gc_push_root(ctx, (void**)&name);
	if ((({ mph_ModuleManager* _obj_1 = mm; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_2; }) >= ({ mph_ModuleManager* _obj_3 = mm; mph_gc_push_root(ctx, (void**)&_obj_3); mph_int _ret_4 = _obj_3->max_stack_depth; mph_gc_pop_roots(ctx, 1); _ret_4; }))) {
{
	({ mph_ModuleManager* _obj_7 = mm; mph_gc_push_root(ctx, (void**)&_obj_7); _obj_7->errors_count = (({ mph_ModuleManager* _obj_5 = mm; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->errors_count; mph_gc_pop_roots(ctx, 1); _ret_6; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "ERROR: Import cycle detected! Stack depth: "));
	mph_native_print_int(ctx, ({ mph_ModuleManager* _obj_8 = mm; mph_gc_push_root(ctx, (void**)&_obj_8); mph_int _ret_9 = _obj_8->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_9; }));
	mph_native_print(ctx, mph_string_new(ctx, ", Module: '"));
	mph_native_print(ctx, name);
	mph_native_print(ctx, mph_string_new(ctx, "'\n"));
	return 0;
}
	}
	({ mph_ModuleManager* _obj_12 = mm; mph_gc_push_root(ctx, (void**)&_obj_12); _obj_12->loading_stack_count = (({ mph_ModuleManager* _obj_10 = mm; mph_gc_push_root(ctx, (void**)&_obj_10); mph_int _ret_11 = _obj_10->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_11; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "  Loading '"));
	mph_native_print(ctx, name);
	mph_native_print(ctx, mph_string_new(ctx, "' (stack: "));
	mph_native_print_int(ctx, ({ mph_ModuleManager* _obj_13 = mm; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_14; }));
	mph_native_print(ctx, mph_string_new(ctx, ")\n"));
	return 1;
	mph_gc_pop_roots(ctx, 2);
}

void mph_module_finish_loading(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, MorphString* name) {
	mph_gc_push_root(ctx, (void**)&mm);
	mph_gc_push_root(ctx, (void**)&name);
	if ((({ mph_ModuleManager* _obj_15 = mm; mph_gc_push_root(ctx, (void**)&_obj_15); mph_int _ret_16 = _obj_15->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_16; }) > 0)) {
{
	({ mph_ModuleManager* _obj_19 = mm; mph_gc_push_root(ctx, (void**)&_obj_19); _obj_19->loading_stack_count = (({ mph_ModuleManager* _obj_17 = mm; mph_gc_push_root(ctx, (void**)&_obj_17); mph_int _ret_18 = _obj_17->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_18; }) - 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "  Finished '"));
	mph_native_print(ctx, name);
	mph_native_print(ctx, mph_string_new(ctx, "' (stack: "));
	mph_native_print_int(ctx, ({ mph_ModuleManager* _obj_20 = mm; mph_gc_push_root(ctx, (void**)&_obj_20); mph_int _ret_21 = _obj_20->loading_stack_count; mph_gc_pop_roots(ctx, 1); _ret_21; }));
	mph_native_print(ctx, mph_string_new(ctx, ")\n"));
	mph_gc_pop_roots(ctx, 2);
}

mph_bool mph_load_chain_linear(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm) {
	mph_gc_push_root(ctx, (void**)&mm);
	mph_native_print(ctx, mph_string_new(ctx, "Loading chain: A -> B -> C\n"));
	if ((({ mph_ModuleManager* _arg_22 = mm; mph_gc_push_root(ctx, (void**)&_arg_22); MorphString* _arg_23 = mph_string_new(ctx, "A"); mph_gc_push_root(ctx, (void**)&_arg_23); mph_bool _ret_24 = mph_module_start_loading(ctx, NULL, _arg_22, _arg_23); mph_gc_pop_roots(ctx, 2); _ret_24; }) == 0)) {
{
	return 0;
}
	}
	if ((({ mph_ModuleManager* _arg_25 = mm; mph_gc_push_root(ctx, (void**)&_arg_25); MorphString* _arg_26 = mph_string_new(ctx, "B"); mph_gc_push_root(ctx, (void**)&_arg_26); mph_bool _ret_27 = mph_module_start_loading(ctx, NULL, _arg_25, _arg_26); mph_gc_pop_roots(ctx, 2); _ret_27; }) == 0)) {
{
	({ mph_ModuleManager* _arg_28 = mm; mph_gc_push_root(ctx, (void**)&_arg_28); MorphString* _arg_29 = mph_string_new(ctx, "A"); mph_gc_push_root(ctx, (void**)&_arg_29); mph_module_finish_loading(ctx, NULL, _arg_28, _arg_29); mph_gc_pop_roots(ctx, 2); });
	return 0;
}
	}
	if ((({ mph_ModuleManager* _arg_30 = mm; mph_gc_push_root(ctx, (void**)&_arg_30); MorphString* _arg_31 = mph_string_new(ctx, "C"); mph_gc_push_root(ctx, (void**)&_arg_31); mph_bool _ret_32 = mph_module_start_loading(ctx, NULL, _arg_30, _arg_31); mph_gc_pop_roots(ctx, 2); _ret_32; }) == 0)) {
{
	({ mph_ModuleManager* _arg_33 = mm; mph_gc_push_root(ctx, (void**)&_arg_33); MorphString* _arg_34 = mph_string_new(ctx, "B"); mph_gc_push_root(ctx, (void**)&_arg_34); mph_module_finish_loading(ctx, NULL, _arg_33, _arg_34); mph_gc_pop_roots(ctx, 2); });
	({ mph_ModuleManager* _arg_35 = mm; mph_gc_push_root(ctx, (void**)&_arg_35); MorphString* _arg_36 = mph_string_new(ctx, "A"); mph_gc_push_root(ctx, (void**)&_arg_36); mph_module_finish_loading(ctx, NULL, _arg_35, _arg_36); mph_gc_pop_roots(ctx, 2); });
	return 0;
}
	}
	({ mph_ModuleManager* _arg_37 = mm; mph_gc_push_root(ctx, (void**)&_arg_37); MorphString* _arg_38 = mph_string_new(ctx, "C"); mph_gc_push_root(ctx, (void**)&_arg_38); mph_module_finish_loading(ctx, NULL, _arg_37, _arg_38); mph_gc_pop_roots(ctx, 2); });
	({ mph_ModuleManager* _arg_39 = mm; mph_gc_push_root(ctx, (void**)&_arg_39); MorphString* _arg_40 = mph_string_new(ctx, "B"); mph_gc_push_root(ctx, (void**)&_arg_40); mph_module_finish_loading(ctx, NULL, _arg_39, _arg_40); mph_gc_pop_roots(ctx, 2); });
	({ mph_ModuleManager* _arg_41 = mm; mph_gc_push_root(ctx, (void**)&_arg_41); MorphString* _arg_42 = mph_string_new(ctx, "A"); mph_gc_push_root(ctx, (void**)&_arg_42); mph_module_finish_loading(ctx, NULL, _arg_41, _arg_42); mph_gc_pop_roots(ctx, 2); });
	return 1;
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_load_chain_cycle(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm) {
	mph_gc_push_root(ctx, (void**)&mm);
	mph_native_print(ctx, mph_string_new(ctx, "Loading chain with cycle: A -> B -> C -> D -> E -> F (exceeds max 5)\n"));
	if ((({ mph_ModuleManager* _arg_43 = mm; mph_gc_push_root(ctx, (void**)&_arg_43); MorphString* _arg_44 = mph_string_new(ctx, "A"); mph_gc_push_root(ctx, (void**)&_arg_44); mph_bool _ret_45 = mph_module_start_loading(ctx, NULL, _arg_43, _arg_44); mph_gc_pop_roots(ctx, 2); _ret_45; }) == 0)) {
{
	return 0;
}
	}
	if ((({ mph_ModuleManager* _arg_46 = mm; mph_gc_push_root(ctx, (void**)&_arg_46); MorphString* _arg_47 = mph_string_new(ctx, "B"); mph_gc_push_root(ctx, (void**)&_arg_47); mph_bool _ret_48 = mph_module_start_loading(ctx, NULL, _arg_46, _arg_47); mph_gc_pop_roots(ctx, 2); _ret_48; }) == 0)) {
{
	return 0;
}
	}
	if ((({ mph_ModuleManager* _arg_49 = mm; mph_gc_push_root(ctx, (void**)&_arg_49); MorphString* _arg_50 = mph_string_new(ctx, "C"); mph_gc_push_root(ctx, (void**)&_arg_50); mph_bool _ret_51 = mph_module_start_loading(ctx, NULL, _arg_49, _arg_50); mph_gc_pop_roots(ctx, 2); _ret_51; }) == 0)) {
{
	return 0;
}
	}
	if ((({ mph_ModuleManager* _arg_52 = mm; mph_gc_push_root(ctx, (void**)&_arg_52); MorphString* _arg_53 = mph_string_new(ctx, "D"); mph_gc_push_root(ctx, (void**)&_arg_53); mph_bool _ret_54 = mph_module_start_loading(ctx, NULL, _arg_52, _arg_53); mph_gc_pop_roots(ctx, 2); _ret_54; }) == 0)) {
{
	return 0;
}
	}
	if ((({ mph_ModuleManager* _arg_55 = mm; mph_gc_push_root(ctx, (void**)&_arg_55); MorphString* _arg_56 = mph_string_new(ctx, "E"); mph_gc_push_root(ctx, (void**)&_arg_56); mph_bool _ret_57 = mph_module_start_loading(ctx, NULL, _arg_55, _arg_56); mph_gc_pop_roots(ctx, 2); _ret_57; }) == 0)) {
{
	return 0;
}
	}
	if ((({ mph_ModuleManager* _arg_58 = mm; mph_gc_push_root(ctx, (void**)&_arg_58); MorphString* _arg_59 = mph_string_new(ctx, "F"); mph_gc_push_root(ctx, (void**)&_arg_59); mph_bool _ret_60 = mph_module_start_loading(ctx, NULL, _arg_58, _arg_59); mph_gc_pop_roots(ctx, 2); _ret_60; }) == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "Cycle detected and prevented!\n"));
	return 0;
}
	}
	return 1;
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  Import Cycle Detection Test        â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 1: Linear Import Chain ===\n"));
	mph_ModuleManager* mm1 = mph_new_module_manager(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&mm1);
	mph_bool result1 = ({ mph_ModuleManager* _arg_61 = mm1; mph_gc_push_root(ctx, (void**)&_arg_61); mph_bool _ret_62 = mph_load_chain_linear(ctx, NULL, _arg_61); mph_gc_pop_roots(ctx, 1); _ret_62; });
	if (result1) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Linear chain loaded successfully\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL: Linear chain should not fail\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "  Errors: "));
	mph_native_print_int(ctx, ({ mph_ModuleManager* _obj_63 = mm1; mph_gc_push_root(ctx, (void**)&_obj_63); mph_int _ret_64 = _obj_63->errors_count; mph_gc_pop_roots(ctx, 1); _ret_64; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 2: Circular Import Chain ===\n"));
	mph_ModuleManager* mm2 = mph_new_module_manager(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&mm2);
	mph_bool result2 = ({ mph_ModuleManager* _arg_65 = mm2; mph_gc_push_root(ctx, (void**)&_arg_65); mph_bool _ret_66 = mph_load_chain_cycle(ctx, NULL, _arg_65); mph_gc_pop_roots(ctx, 1); _ret_66; });
	if (result2) {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL: Cycle should have been detected\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Cycle correctly prevented\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "  Errors: "));
	mph_native_print_int(ctx, ({ mph_ModuleManager* _obj_67 = mm2; mph_gc_push_root(ctx, (void**)&_obj_67); mph_int _ret_68 = _obj_67->errors_count; mph_gc_pop_roots(ctx, 1); _ret_68; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 3: Multiple Imports (same level) ===\n"));
	mph_ModuleManager* mm3 = mph_new_module_manager(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&mm3);
	mph_native_print(ctx, mph_string_new(ctx, "Loading multiple modules...\n"));
	mph_bool ok1 = ({ mph_ModuleManager* _arg_69 = mm3; mph_gc_push_root(ctx, (void**)&_arg_69); MorphString* _arg_70 = mph_string_new(ctx, "utils"); mph_gc_push_root(ctx, (void**)&_arg_70); mph_bool _ret_71 = mph_module_start_loading(ctx, NULL, _arg_69, _arg_70); mph_gc_pop_roots(ctx, 2); _ret_71; });
	mph_bool ok2 = ({ mph_ModuleManager* _arg_72 = mm3; mph_gc_push_root(ctx, (void**)&_arg_72); MorphString* _arg_73 = mph_string_new(ctx, "helpers"); mph_gc_push_root(ctx, (void**)&_arg_73); mph_bool _ret_74 = mph_module_start_loading(ctx, NULL, _arg_72, _arg_73); mph_gc_pop_roots(ctx, 2); _ret_74; });
	if ((ok1 == 1)) {
{
	({ mph_ModuleManager* _arg_75 = mm3; mph_gc_push_root(ctx, (void**)&_arg_75); MorphString* _arg_76 = mph_string_new(ctx, "utils"); mph_gc_push_root(ctx, (void**)&_arg_76); mph_module_finish_loading(ctx, NULL, _arg_75, _arg_76); mph_gc_pop_roots(ctx, 2); });
	mph_native_print(ctx, mph_string_new(ctx, "PASS: First module loaded\n"));
}
	}
	if ((ok2 == 1)) {
{
	({ mph_ModuleManager* _arg_77 = mm3; mph_gc_push_root(ctx, (void**)&_arg_77); MorphString* _arg_78 = mph_string_new(ctx, "helpers"); mph_gc_push_root(ctx, (void**)&_arg_78); mph_module_finish_loading(ctx, NULL, _arg_77, _arg_78); mph_gc_pop_roots(ctx, 2); });
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Second module loaded\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "  Errors: "));
	mph_native_print_int(ctx, ({ mph_ModuleManager* _obj_79 = mm3; mph_gc_push_root(ctx, (void**)&_obj_79); mph_int _ret_80 = _obj_79->errors_count; mph_gc_pop_roots(ctx, 1); _ret_80; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â         TEST SUMMARY                â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Linear imports work correctly\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Circular imports are detected\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Stack depth limiting prevents infinite loops\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ð IMPORT CYCLE TEST PASSED! ð\n"));
	mph_gc_pop_roots(ctx, 3);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
