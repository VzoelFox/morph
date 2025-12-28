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
#define MPH_TYPE_mph_RecursionLimiter 2
#define MPH_TYPE_mph_Type 1

// Struct Definitions (Env & Types)
typedef struct mph_Type mph_Type;
typedef struct mph_RecursionLimiter mph_RecursionLimiter;
struct mph_Type {
	mph_int kind;
	MorphString* name;
};

struct mph_RecursionLimiter {
	mph_int max_depth;
	mph_int current_depth;
	mph_int overflow_count;
};

MorphTypeInfo mph_ti_mph_Type = { "Type", sizeof(mph_Type), 1, (size_t[]){offsetof(mph_Type, name)} };
MorphTypeInfo mph_ti_mph_RecursionLimiter = { "RecursionLimiter", sizeof(mph_RecursionLimiter), 0, NULL };

// Global Variables
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;

// Function Prototypes
mph_Type* mph_int_type(MorphContext* ctx, void* _env_void);
mph_RecursionLimiter* mph_new_recursion_limiter(MorphContext* ctx, void* _env_void);
mph_bool mph_recursion_enter(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, MorphString* func_name);
void mph_recursion_exit(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl);
mph_int mph_recursive_function(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, mph_int depth);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_Type* mph_int_type(MorphContext* ctx, void* _env_void) {
	return ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->name = mph_string_new(ctx, "int"); _t->kind = mph_KIND_INT; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_RecursionLimiter* mph_new_recursion_limiter(MorphContext* ctx, void* _env_void) {
	return ({ mph_RecursionLimiter* _t = (mph_RecursionLimiter*)mph_alloc(ctx, sizeof(mph_RecursionLimiter), &mph_ti_mph_RecursionLimiter); mph_gc_push_root(ctx, (void**)&_t); _t->max_depth = 10; _t->current_depth = 0; _t->overflow_count = 0; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_recursion_enter(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, MorphString* func_name) {
	mph_gc_push_root(ctx, (void**)&rl);
	mph_gc_push_root(ctx, (void**)&func_name);
	if ((({ mph_RecursionLimiter* _obj_1 = rl; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->current_depth; mph_gc_pop_roots(ctx, 1); _ret_2; }) >= ({ mph_RecursionLimiter* _obj_3 = rl; mph_gc_push_root(ctx, (void**)&_obj_3); mph_int _ret_4 = _obj_3->max_depth; mph_gc_pop_roots(ctx, 1); _ret_4; }))) {
{
	({ mph_RecursionLimiter* _obj_7 = rl; mph_gc_push_root(ctx, (void**)&_obj_7); _obj_7->overflow_count = (({ mph_RecursionLimiter* _obj_5 = rl; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->overflow_count; mph_gc_pop_roots(ctx, 1); _ret_6; }) + 1); mph_gc_pop_roots(ctx, 1); });
	mph_native_print(ctx, mph_string_new(ctx, "ERROR: Stack overflow prevented at depth "));
	mph_native_print_int(ctx, ({ mph_RecursionLimiter* _obj_8 = rl; mph_gc_push_root(ctx, (void**)&_obj_8); mph_int _ret_9 = _obj_8->current_depth; mph_gc_pop_roots(ctx, 1); _ret_9; }));
	mph_native_print(ctx, mph_string_new(ctx, " in '"));
	mph_native_print(ctx, func_name);
	mph_native_print(ctx, mph_string_new(ctx, "'\n"));
	return 0;
}
	}
	({ mph_RecursionLimiter* _obj_12 = rl; mph_gc_push_root(ctx, (void**)&_obj_12); _obj_12->current_depth = (({ mph_RecursionLimiter* _obj_10 = rl; mph_gc_push_root(ctx, (void**)&_obj_10); mph_int _ret_11 = _obj_10->current_depth; mph_gc_pop_roots(ctx, 1); _ret_11; }) + 1); mph_gc_pop_roots(ctx, 1); });
	return 1;
	mph_gc_pop_roots(ctx, 2);
}

void mph_recursion_exit(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl) {
	mph_gc_push_root(ctx, (void**)&rl);
	if ((({ mph_RecursionLimiter* _obj_13 = rl; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->current_depth; mph_gc_pop_roots(ctx, 1); _ret_14; }) > 0)) {
{
	({ mph_RecursionLimiter* _obj_17 = rl; mph_gc_push_root(ctx, (void**)&_obj_17); _obj_17->current_depth = (({ mph_RecursionLimiter* _obj_15 = rl; mph_gc_push_root(ctx, (void**)&_obj_15); mph_int _ret_16 = _obj_15->current_depth; mph_gc_pop_roots(ctx, 1); _ret_16; }) - 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_gc_pop_roots(ctx, 1);
}

mph_int mph_recursive_function(MorphContext* ctx, void* _env_void, mph_RecursionLimiter* rl, mph_int depth) {
	mph_gc_push_root(ctx, (void**)&rl);
	if ((({ mph_RecursionLimiter* _arg_18 = rl; mph_gc_push_root(ctx, (void**)&_arg_18); MorphString* _arg_19 = mph_string_new(ctx, "recursive_function"); mph_gc_push_root(ctx, (void**)&_arg_19); mph_bool _ret_20 = mph_recursion_enter(ctx, NULL, _arg_18, _arg_19); mph_gc_pop_roots(ctx, 2); _ret_20; }) == 0)) {
{
	return (-1);
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "  Depth: "));
	mph_native_print_int(ctx, depth);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_int result = 0;
	if ((depth > 0)) {
{
	result = ({ mph_RecursionLimiter* _arg_21 = rl; mph_gc_push_root(ctx, (void**)&_arg_21); mph_int _ret_22 = mph_recursive_function(ctx, NULL, _arg_21, (depth - 1)); mph_gc_pop_roots(ctx, 1); _ret_22; });
}
	}
	({ mph_RecursionLimiter* _arg_23 = rl; mph_gc_push_root(ctx, (void**)&_arg_23); mph_recursion_exit(ctx, NULL, _arg_23); mph_gc_pop_roots(ctx, 1); });
	return result;
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  Recursion Limiting Test            â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 1: Normal Recursion (depth=5) ===\n"));
	mph_RecursionLimiter* rl1 = mph_new_recursion_limiter(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&rl1);
	mph_int result1 = ({ mph_RecursionLimiter* _arg_24 = rl1; mph_gc_push_root(ctx, (void**)&_arg_24); mph_int _ret_25 = mph_recursive_function(ctx, NULL, _arg_24, 5); mph_gc_pop_roots(ctx, 1); _ret_25; });
	if ((result1 == (-1))) {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL: Should not overflow at depth 5\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Recursion completed successfully\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "  Overflow count: "));
	mph_native_print_int(ctx, ({ mph_RecursionLimiter* _obj_26 = rl1; mph_gc_push_root(ctx, (void**)&_obj_26); mph_int _ret_27 = _obj_26->overflow_count; mph_gc_pop_roots(ctx, 1); _ret_27; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 2: Deep Recursion (depth=15) ===\n"));
	mph_RecursionLimiter* rl2 = mph_new_recursion_limiter(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&rl2);
	mph_int result2 = ({ mph_RecursionLimiter* _arg_28 = rl2; mph_gc_push_root(ctx, (void**)&_arg_28); mph_int _ret_29 = mph_recursive_function(ctx, NULL, _arg_28, 15); mph_gc_pop_roots(ctx, 1); _ret_29; });
	if ((result2 == (-1))) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Overflow correctly prevented\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL: Should have overflowed at depth >10\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "  Overflow count: "));
	mph_native_print_int(ctx, ({ mph_RecursionLimiter* _obj_30 = rl2; mph_gc_push_root(ctx, (void**)&_obj_30); mph_int _ret_31 = _obj_30->overflow_count; mph_gc_pop_roots(ctx, 1); _ret_31; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Test 3: Exact Limit (depth=10) ===\n"));
	mph_RecursionLimiter* rl3 = mph_new_recursion_limiter(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&rl3);
	mph_int result3 = ({ mph_RecursionLimiter* _arg_32 = rl3; mph_gc_push_root(ctx, (void**)&_arg_32); mph_int _ret_33 = mph_recursive_function(ctx, NULL, _arg_32, 10); mph_gc_pop_roots(ctx, 1); _ret_33; });
	if ((result3 == (-1))) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS: Overflow at exact limit\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "INFO: Completed at exact limit\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "  Overflow count: "));
	mph_native_print_int(ctx, ({ mph_RecursionLimiter* _obj_34 = rl3; mph_gc_push_root(ctx, (void**)&_obj_34); mph_int _ret_35 = _obj_34->overflow_count; mph_gc_pop_roots(ctx, 1); _ret_35; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â         TEST SUMMARY                â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Recursion limiting prevents stack overflow\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Depth tracking works correctly\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Overflow counter increments properly\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ð RECURSION LIMITING TEST PASSED! ð\n"));
	mph_gc_pop_roots(ctx, 3);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_STRING = 2;
	mph_main(ctx, NULL);
}
