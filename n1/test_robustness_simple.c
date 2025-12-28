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
#define MPH_TYPE_mph_ModuleManager 2
#define MPH_TYPE_mph_ErrorRecovery 3
#define MPH_TYPE_mph_RecursionLimiter 1

// Struct Definitions (Env & Types)
typedef struct mph_RecursionLimiter mph_RecursionLimiter;
typedef struct mph_ModuleManager mph_ModuleManager;
typedef struct mph_ErrorRecovery mph_ErrorRecovery;
struct mph_RecursionLimiter {
	mph_int max_depth;
	mph_int current_depth;
	mph_int overflow_count;
};

struct mph_ModuleManager {
	mph_int loading_depth;
	mph_int max_depth;
	mph_int cycle_count;
};

struct mph_ErrorRecovery {
	mph_int errors_count;
	mph_int max_errors;
	mph_bool panic_mode;
};

MorphTypeInfo mph_ti_mph_RecursionLimiter = { "RecursionLimiter", sizeof(mph_RecursionLimiter), 0, NULL };
MorphTypeInfo mph_ti_mph_ModuleManager = { "ModuleManager", sizeof(mph_ModuleManager), 0, NULL };
MorphTypeInfo mph_ti_mph_ErrorRecovery = { "ErrorRecovery", sizeof(mph_ErrorRecovery), 0, NULL };

// Global Variables

// Function Prototypes
void mph_print_header(MorphContext* ctx, void* _env_void, MorphString* name);
void mph_print_pass(MorphContext* ctx, void* _env_void, MorphString* msg);
void mph_print_stat(MorphContext* ctx, void* _env_void, MorphString* label, mph_int value);
mph_RecursionLimiter* mph_new_recursion_limiter(MorphContext* ctx, void* _env_void, mph_int max);
mph_bool mph_recursion_enter(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl);
void mph_recursion_exit(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl);
mph_bool mph_recursive_call(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, mph_int depth);
mph_ModuleManager* mph_new_module_manager(MorphContext* ctx, void* _env_void, mph_int max);
mph_bool mph_module_load_recursive(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, mph_int depth);
mph_ErrorRecovery* mph_new_error_recovery(MorphContext* ctx, void* _env_void, mph_int max);
mph_bool mph_error_add(MorphContext* ctx, void* _env_void, mph_ErrorRecovery* er);
mph_int mph_add_multiple_errors(MorphContext* ctx, void* _env_void, mph_ErrorRecovery* er, mph_int count);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
void mph_print_header(MorphContext* ctx, void* _env_void, MorphString* name) {
	mph_gc_push_root(ctx, (void**)&name);
	mph_native_print(ctx, mph_string_new(ctx, "\n=== "));
	mph_native_print(ctx, name);
	mph_native_print(ctx, mph_string_new(ctx, " ===\n"));
	mph_gc_pop_roots(ctx, 1);
}

void mph_print_pass(MorphContext* ctx, void* _env_void, MorphString* msg) {
	mph_gc_push_root(ctx, (void**)&msg);
	mph_native_print(ctx, mph_string_new(ctx, "â PASS: "));
	mph_native_print(ctx, msg);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_gc_pop_roots(ctx, 1);
}

void mph_print_stat(MorphContext* ctx, void* _env_void, MorphString* label, mph_int value) {
	mph_gc_push_root(ctx, (void**)&label);
	mph_native_print(ctx, mph_string_new(ctx, "  "));
	mph_native_print(ctx, label);
	mph_native_print(ctx, mph_string_new(ctx, ": "));
	mph_native_print_int(ctx, value);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_gc_pop_roots(ctx, 1);
}

mph_RecursionLimiter* mph_new_recursion_limiter(MorphContext* ctx, void* _env_void, mph_int max) {
	return ({ mph_RecursionLimiter* _t = (mph_RecursionLimiter*)mph_alloc(ctx, sizeof(mph_RecursionLimiter), &mph_ti_mph_RecursionLimiter); mph_gc_push_root(ctx, (void**)&_t); _t->overflow_count = 0; _t->max_depth = max; _t->current_depth = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_recursion_enter(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl) {
	mph_gc_push_root(ctx, (void**)&rl);
	if ((({ mph_RecursionLimiter* _obj_1 = rl; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->current_depth; mph_gc_pop_roots(ctx, 1); _ret_2; }) >= ({ mph_RecursionLimiter* _obj_3 = rl; mph_gc_push_root(ctx, (void**)&_obj_3); mph_int _ret_4 = _obj_3->max_depth; mph_gc_pop_roots(ctx, 1); _ret_4; }))) {
{
	({ mph_RecursionLimiter* _obj_7 = rl; mph_gc_push_root(ctx, (void**)&_obj_7); _obj_7->overflow_count = (({ mph_RecursionLimiter* _obj_5 = rl; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->overflow_count; mph_gc_pop_roots(ctx, 1); _ret_6; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return 0;
}
	}
	({ mph_RecursionLimiter* _obj_10 = rl; mph_gc_push_root(ctx, (void**)&_obj_10); _obj_10->current_depth = (({ mph_RecursionLimiter* _obj_8 = rl; mph_gc_push_root(ctx, (void**)&_obj_8); mph_int _ret_9 = _obj_8->current_depth; mph_gc_pop_roots(ctx, 1); _ret_9; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return 1;
	mph_gc_pop_roots(ctx, 1);
}

void mph_recursion_exit(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl) {
	mph_gc_push_root(ctx, (void**)&rl);
	if ((({ mph_RecursionLimiter* _obj_11 = rl; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->current_depth; mph_gc_pop_roots(ctx, 1); _ret_12; }) > 0)) {
{
	({ mph_RecursionLimiter* _obj_15 = rl; mph_gc_push_root(ctx, (void**)&_obj_15); _obj_15->current_depth = (({ mph_RecursionLimiter* _obj_13 = rl; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->current_depth; mph_gc_pop_roots(ctx, 1); _ret_14; }) - 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_recursive_call(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, mph_int depth) {
	mph_gc_push_root(ctx, (void**)&rl);
	if ((depth <= 0)) {
{
	return 1;
}
	}
	if ((({ mph_RecursionLimiter* _arg_16 = rl; mph_gc_push_root(ctx, (void**)&_arg_16); mph_bool _ret_17 = mph_recursion_enter(ctx, NULL, _arg_16); mph_gc_pop_roots(ctx, 1); _ret_17; }) == 0)) {
{
	return 0;
}
	}
	mph_bool result = ({ mph_RecursionLimiter* _arg_18 = rl; mph_gc_push_root(ctx, (void**)&_arg_18); mph_bool _ret_19 = mph_recursive_call(ctx, NULL, _arg_18, (depth - 1)); mph_gc_pop_roots(ctx, 1); _ret_19; });
	({ mph_RecursionLimiter* _arg_20 = rl; mph_gc_push_root(ctx, (void**)&_arg_20); mph_recursion_exit(ctx, NULL, _arg_20); mph_gc_pop_roots(ctx, 1); });
	return result;
	mph_gc_pop_roots(ctx, 1);
}

mph_ModuleManager* mph_new_module_manager(MorphContext* ctx, void* _env_void, mph_int max) {
	return ({ mph_ModuleManager* _t = (mph_ModuleManager*)mph_alloc(ctx, sizeof(mph_ModuleManager), &mph_ti_mph_ModuleManager); mph_gc_push_root(ctx, (void**)&_t); _t->loading_depth = 0; _t->max_depth = max; _t->cycle_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_module_load_recursive(MorphContext* ctx, void* _env_void, mph_ModuleManager* mm, mph_int depth) {
	mph_gc_push_root(ctx, (void**)&mm);
	if ((depth <= 0)) {
{
	return 1;
}
	}
	if ((({ mph_ModuleManager* _obj_21 = mm; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_22; }) >= ({ mph_ModuleManager* _obj_23 = mm; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->max_depth; mph_gc_pop_roots(ctx, 1); _ret_24; }))) {
{
	({ mph_ModuleManager* _obj_27 = mm; mph_gc_push_root(ctx, (void**)&_obj_27); _obj_27->cycle_count = (({ mph_ModuleManager* _obj_25 = mm; mph_gc_push_root(ctx, (void**)&_obj_25); mph_int _ret_26 = _obj_25->cycle_count; mph_gc_pop_roots(ctx, 1); _ret_26; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return 0;
}
	}
	({ mph_ModuleManager* _obj_30 = mm; mph_gc_push_root(ctx, (void**)&_obj_30); _obj_30->loading_depth = (({ mph_ModuleManager* _obj_28 = mm; mph_gc_push_root(ctx, (void**)&_obj_28); mph_int _ret_29 = _obj_28->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_29; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_bool result = ({ mph_ModuleManager* _arg_31 = mm; mph_gc_push_root(ctx, (void**)&_arg_31); mph_bool _ret_32 = mph_module_load_recursive(ctx, NULL, _arg_31, (depth - 1)); mph_gc_pop_roots(ctx, 1); _ret_32; });
	if ((({ mph_ModuleManager* _obj_33 = mm; mph_gc_push_root(ctx, (void**)&_obj_33); mph_int _ret_34 = _obj_33->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_34; }) > 0)) {
{
	({ mph_ModuleManager* _obj_37 = mm; mph_gc_push_root(ctx, (void**)&_obj_37); _obj_37->loading_depth = (({ mph_ModuleManager* _obj_35 = mm; mph_gc_push_root(ctx, (void**)&_obj_35); mph_int _ret_36 = _obj_35->loading_depth; mph_gc_pop_roots(ctx, 1); _ret_36; }) - 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	return result;
	mph_gc_pop_roots(ctx, 1);
}

mph_ErrorRecovery* mph_new_error_recovery(MorphContext* ctx, void* _env_void, mph_int max) {
	return ({ mph_ErrorRecovery* _t = (mph_ErrorRecovery*)mph_alloc(ctx, sizeof(mph_ErrorRecovery), &mph_ti_mph_ErrorRecovery); mph_gc_push_root(ctx, (void**)&_t); _t->errors_count = 0; _t->max_errors = max; _t->panic_mode = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_error_add(MorphContext* ctx, void* _env_void, mph_ErrorRecovery* er) {
	mph_gc_push_root(ctx, (void**)&er);
	({ mph_ErrorRecovery* _obj_40 = er; mph_gc_push_root(ctx, (void**)&_obj_40); _obj_40->errors_count = (({ mph_ErrorRecovery* _obj_38 = er; mph_gc_push_root(ctx, (void**)&_obj_38); mph_int _ret_39 = _obj_38->errors_count; mph_gc_pop_roots(ctx, 1); _ret_39; }) + 1); mph_gc_pop_roots(ctx, 1); });
	if ((({ mph_ErrorRecovery* _obj_41 = er; mph_gc_push_root(ctx, (void**)&_obj_41); mph_int _ret_42 = _obj_41->errors_count; mph_gc_pop_roots(ctx, 1); _ret_42; }) >= ({ mph_ErrorRecovery* _obj_43 = er; mph_gc_push_root(ctx, (void**)&_obj_43); mph_int _ret_44 = _obj_43->max_errors; mph_gc_pop_roots(ctx, 1); _ret_44; }))) {
{
	({ mph_ErrorRecovery* _obj_45 = er; mph_gc_push_root(ctx, (void**)&_obj_45); _obj_45->panic_mode = 1; mph_gc_pop_roots(ctx, 1); });
	return 0;
}
	}
	return 1;
	mph_gc_pop_roots(ctx, 1);
}

mph_int mph_add_multiple_errors(MorphContext* ctx, void* _env_void, mph_ErrorRecovery* er, mph_int count) {
	mph_gc_push_root(ctx, (void**)&er);
	if ((count <= 0)) {
{
	return ({ mph_ErrorRecovery* _obj_46 = er; mph_gc_push_root(ctx, (void**)&_obj_46); mph_int _ret_47 = _obj_46->errors_count; mph_gc_pop_roots(ctx, 1); _ret_47; });
}
	}
	({ mph_ErrorRecovery* _arg_48 = er; mph_gc_push_root(ctx, (void**)&_arg_48); mph_bool _ret_49 = mph_error_add(ctx, NULL, _arg_48); mph_gc_pop_roots(ctx, 1); _ret_49; });
	return ({ mph_ErrorRecovery* _arg_50 = er; mph_gc_push_root(ctx, (void**)&_arg_50); mph_int _ret_51 = mph_add_multiple_errors(ctx, NULL, _arg_50, (count - 1)); mph_gc_pop_roots(ctx, 1); _ret_51; });
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  N1 ROBUSTNESS TEST SUITE                         â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_int passed = 0;
	({ MorphString* _arg_52 = mph_string_new(ctx, "Test 1: Recursion Limiting"); mph_gc_push_root(ctx, (void**)&_arg_52); mph_print_header(ctx, NULL, _arg_52); mph_gc_pop_roots(ctx, 1); });
	mph_RecursionLimiter* rl1 = mph_new_recursion_limiter(ctx, NULL, 100);
	mph_gc_push_root(ctx, (void**)&rl1);
	mph_bool ok1 = ({ mph_RecursionLimiter* _arg_53 = rl1; mph_gc_push_root(ctx, (void**)&_arg_53); mph_bool _ret_54 = mph_recursive_call(ctx, NULL, _arg_53, 50); mph_gc_pop_roots(ctx, 1); _ret_54; });
	if (ok1) {
{
	({ MorphString* _arg_55 = mph_string_new(ctx, "Normal recursion (depth 50)"); mph_gc_push_root(ctx, (void**)&_arg_55); mph_print_pass(ctx, NULL, _arg_55); mph_gc_pop_roots(ctx, 1); });
	passed = (passed + 1);
}
	}
	mph_RecursionLimiter* rl2 = mph_new_recursion_limiter(ctx, NULL, 20);
	mph_gc_push_root(ctx, (void**)&rl2);
	mph_bool ok2 = ({ mph_RecursionLimiter* _arg_56 = rl2; mph_gc_push_root(ctx, (void**)&_arg_56); mph_bool _ret_57 = mph_recursive_call(ctx, NULL, _arg_56, 50); mph_gc_pop_roots(ctx, 1); _ret_57; });
	if ((ok2 == 0)) {
{
	({ MorphString* _arg_58 = mph_string_new(ctx, "Overflow prevented at limit"); mph_gc_push_root(ctx, (void**)&_arg_58); mph_print_pass(ctx, NULL, _arg_58); mph_gc_pop_roots(ctx, 1); });
	passed = (passed + 1);
}
	}
	({ MorphString* _arg_59 = mph_string_new(ctx, "Overflow count"); mph_gc_push_root(ctx, (void**)&_arg_59); mph_print_stat(ctx, NULL, _arg_59, ({ mph_RecursionLimiter* _obj_60 = rl2; mph_gc_push_root(ctx, (void**)&_obj_60); mph_int _ret_61 = _obj_60->overflow_count; mph_gc_pop_roots(ctx, 1); _ret_61; })); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_62 = mph_string_new(ctx, "Test 2: Import Cycle Detection"); mph_gc_push_root(ctx, (void**)&_arg_62); mph_print_header(ctx, NULL, _arg_62); mph_gc_pop_roots(ctx, 1); });
	mph_ModuleManager* mm1 = mph_new_module_manager(ctx, NULL, 10);
	mph_gc_push_root(ctx, (void**)&mm1);
	mph_bool ok3 = ({ mph_ModuleManager* _arg_63 = mm1; mph_gc_push_root(ctx, (void**)&_arg_63); mph_bool _ret_64 = mph_module_load_recursive(ctx, NULL, _arg_63, 5); mph_gc_pop_roots(ctx, 1); _ret_64; });
	if (ok3) {
{
	({ MorphString* _arg_65 = mph_string_new(ctx, "Normal import chain (depth 5)"); mph_gc_push_root(ctx, (void**)&_arg_65); mph_print_pass(ctx, NULL, _arg_65); mph_gc_pop_roots(ctx, 1); });
	passed = (passed + 1);
}
	}
	mph_ModuleManager* mm2 = mph_new_module_manager(ctx, NULL, 5);
	mph_gc_push_root(ctx, (void**)&mm2);
	mph_bool ok4 = ({ mph_ModuleManager* _arg_66 = mm2; mph_gc_push_root(ctx, (void**)&_arg_66); mph_bool _ret_67 = mph_module_load_recursive(ctx, NULL, _arg_66, 10); mph_gc_pop_roots(ctx, 1); _ret_67; });
	if ((ok4 == 0)) {
{
	({ MorphString* _arg_68 = mph_string_new(ctx, "Cycle detected and prevented"); mph_gc_push_root(ctx, (void**)&_arg_68); mph_print_pass(ctx, NULL, _arg_68); mph_gc_pop_roots(ctx, 1); });
	passed = (passed + 1);
}
	}
	({ MorphString* _arg_69 = mph_string_new(ctx, "Cycles detected"); mph_gc_push_root(ctx, (void**)&_arg_69); mph_print_stat(ctx, NULL, _arg_69, ({ mph_ModuleManager* _obj_70 = mm2; mph_gc_push_root(ctx, (void**)&_obj_70); mph_int _ret_71 = _obj_70->cycle_count; mph_gc_pop_roots(ctx, 1); _ret_71; })); mph_gc_pop_roots(ctx, 1); });
	({ MorphString* _arg_72 = mph_string_new(ctx, "Test 3: Error Recovery"); mph_gc_push_root(ctx, (void**)&_arg_72); mph_print_header(ctx, NULL, _arg_72); mph_gc_pop_roots(ctx, 1); });
	mph_ErrorRecovery* er1 = mph_new_error_recovery(ctx, NULL, 10);
	mph_gc_push_root(ctx, (void**)&er1);
	mph_int total1 = ({ mph_ErrorRecovery* _arg_73 = er1; mph_gc_push_root(ctx, (void**)&_arg_73); mph_int _ret_74 = mph_add_multiple_errors(ctx, NULL, _arg_73, 5); mph_gc_pop_roots(ctx, 1); _ret_74; });
	if ((({ mph_ErrorRecovery* _obj_75 = er1; mph_gc_push_root(ctx, (void**)&_obj_75); mph_bool _ret_76 = _obj_75->panic_mode; mph_gc_pop_roots(ctx, 1); _ret_76; }) == 0)) {
{
	({ MorphString* _arg_77 = mph_string_new(ctx, "Can continue after 5 errors"); mph_gc_push_root(ctx, (void**)&_arg_77); mph_print_pass(ctx, NULL, _arg_77); mph_gc_pop_roots(ctx, 1); });
	passed = (passed + 1);
}
	}
	mph_ErrorRecovery* er2 = mph_new_error_recovery(ctx, NULL, 10);
	mph_gc_push_root(ctx, (void**)&er2);
	mph_int total2 = ({ mph_ErrorRecovery* _arg_78 = er2; mph_gc_push_root(ctx, (void**)&_arg_78); mph_int _ret_79 = mph_add_multiple_errors(ctx, NULL, _arg_78, 15); mph_gc_pop_roots(ctx, 1); _ret_79; });
	if (({ mph_ErrorRecovery* _obj_80 = er2; mph_gc_push_root(ctx, (void**)&_obj_80); mph_bool _ret_81 = _obj_80->panic_mode; mph_gc_pop_roots(ctx, 1); _ret_81; })) {
{
	({ MorphString* _arg_82 = mph_string_new(ctx, "Panic mode triggered at limit"); mph_gc_push_root(ctx, (void**)&_arg_82); mph_print_pass(ctx, NULL, _arg_82); mph_gc_pop_roots(ctx, 1); });
	passed = (passed + 1);
}
	}
	({ MorphString* _arg_83 = mph_string_new(ctx, "Total errors"); mph_gc_push_root(ctx, (void**)&_arg_83); mph_print_stat(ctx, NULL, _arg_83, ({ mph_ErrorRecovery* _obj_84 = er2; mph_gc_push_root(ctx, (void**)&_obj_84); mph_int _ret_85 = _obj_84->errors_count; mph_gc_pop_roots(ctx, 1); _ret_85; })); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "\nââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  SUMMARY                                           â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Tests passed: "));
	mph_native_print_int(ctx, passed);
	mph_native_print(ctx, mph_string_new(ctx, " / 6\n\n"));
	if ((passed == 6)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "ð ALL TESTS PASSED! ð\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â ï¸  Some tests failed\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "\nâ Stack overflow prevention\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Import cycle detection\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Error recovery system\n"));
	mph_gc_pop_roots(ctx, 6);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
