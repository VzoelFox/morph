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

// Struct Definitions (Env & Types)

// Global Variables

// Function Prototypes
void mph_print_test_header(MorphContext* ctx, void* _env_void, MorphString* name);
void mph_print_result(MorphContext* ctx, void* _env_void, MorphString* expected, MorphString* got);
mph_int mph_test_missing_return_bad(MorphContext* ctx, void* _env_void, mph_int x);
mph_int mph_test_all_paths_return(MorphContext* ctx, void* _env_void, mph_int x);
mph_int mph_test_nested_return(MorphContext* ctx, void* _env_void, mph_int x);
void mph_test_void_function(MorphContext* ctx, void* _env_void, mph_int x);
mph_int mph_test_block_return(MorphContext* ctx, void* _env_void, mph_int x);
mph_int mph_test_multiple_returns(MorphContext* ctx, void* _env_void, mph_int x);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
void mph_print_test_header(MorphContext* ctx, void* _env_void, MorphString* name) {
	mph_gc_push_root(ctx, (void**)&name);
	mph_native_print(ctx, mph_string_new(ctx, "\n=== "));
	mph_native_print(ctx, name);
	mph_native_print(ctx, mph_string_new(ctx, " ===\n"));
	mph_gc_pop_roots(ctx, 1);
}

void mph_print_result(MorphContext* ctx, void* _env_void, MorphString* expected, MorphString* got) {
	mph_gc_push_root(ctx, (void**)&expected);
	mph_gc_push_root(ctx, (void**)&got);
	mph_native_print(ctx, mph_string_new(ctx, "Expected: "));
	mph_native_print(ctx, expected);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Got: "));
	mph_native_print(ctx, got);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_gc_pop_roots(ctx, 2);
}

mph_int mph_test_missing_return_bad(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x > 0)) {
{
	return 1;
}
	}
	return (-999);
}

mph_int mph_test_all_paths_return(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x > 0)) {
{
	return 1;
}
	} else {
{
	return 0;
}
	}
}

mph_int mph_test_nested_return(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x > 0)) {
{
	if ((x > 10)) {
{
	return 2;
}
	} else {
{
	return 1;
}
	}
}
	} else {
{
	return 0;
}
	}
}

void mph_test_void_function(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x > 0)) {
{
	mph_int y = 1;
	mph_native_print(ctx, mph_string_new(ctx, "void function executed\n"));
}
	}
}

mph_int mph_test_block_return(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x > 0)) {
{
	mph_int y = (x + 1);
	return y;
}
	} else {
{
	return 0;
}
	}
}

mph_int mph_test_multiple_returns(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x > 10)) {
{
	return 2;
}
	} else {
{
	if ((x > 5)) {
{
	return 1;
}
	} else {
{
	return 0;
}
	}
}
	}
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  N1 Control Flow Analysis Test                    â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  Port from N0 control_flow_test.go                â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_int passed = 0;
	mph_int total = 6;
	({ MorphString* _arg_1 = mph_string_new(ctx, "Test 1: Missing Return Path"); mph_gc_push_root(ctx, (void**)&_arg_1); mph_print_test_header(ctx, NULL, _arg_1); mph_gc_pop_roots(ctx, 1); });
	mph_int result1 = mph_test_missing_return_bad(ctx, NULL, 5);
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=5: "));
	mph_native_print_int(ctx, result1);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Note: N0 would reject this at compile time\n"));
	mph_native_print(ctx, mph_string_new(ctx, "N1: Compiles but returns marker value\n"));
	mph_native_print(ctx, mph_string_new(ctx, "INFO: N1 doesn't enforce all-paths-return yet\n"));
	({ MorphString* _arg_2 = mph_string_new(ctx, "Test 2: All Paths Return"); mph_gc_push_root(ctx, (void**)&_arg_2); mph_print_test_header(ctx, NULL, _arg_2); mph_gc_pop_roots(ctx, 1); });
	mph_int result2a = mph_test_all_paths_return(ctx, NULL, 5);
	mph_int result2b = mph_test_all_paths_return(ctx, NULL, (-1));
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=5: "));
	mph_native_print_int(ctx, result2a);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=-1: "));
	mph_native_print_int(ctx, result2b);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if ((result2a == 1)) {
{
	if ((result2b == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS\n"));
	passed = (passed + 1);
}
	}
}
	}
	({ MorphString* _arg_3 = mph_string_new(ctx, "Test 3: Nested Control Flow"); mph_gc_push_root(ctx, (void**)&_arg_3); mph_print_test_header(ctx, NULL, _arg_3); mph_gc_pop_roots(ctx, 1); });
	mph_int result3a = mph_test_nested_return(ctx, NULL, 15);
	mph_int result3b = mph_test_nested_return(ctx, NULL, 5);
	mph_int result3c = mph_test_nested_return(ctx, NULL, (-1));
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=15: "));
	mph_native_print_int(ctx, result3a);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=5: "));
	mph_native_print_int(ctx, result3b);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=-1: "));
	mph_native_print_int(ctx, result3c);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if ((result3a == 2)) {
{
	if ((result3b == 1)) {
{
	if ((result3c == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS\n"));
	passed = (passed + 1);
}
	}
}
	}
}
	}
	({ MorphString* _arg_4 = mph_string_new(ctx, "Test 4: Void Function (no return needed)"); mph_gc_push_root(ctx, (void**)&_arg_4); mph_print_test_header(ctx, NULL, _arg_4); mph_gc_pop_roots(ctx, 1); });
	mph_test_void_function(ctx, NULL, 5);
	mph_native_print(ctx, mph_string_new(ctx, "PASS (void function executed)\n"));
	passed = (passed + 1);
	({ MorphString* _arg_5 = mph_string_new(ctx, "Test 5: Block with Return"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_print_test_header(ctx, NULL, _arg_5); mph_gc_pop_roots(ctx, 1); });
	mph_int result5a = mph_test_block_return(ctx, NULL, 10);
	mph_int result5b = mph_test_block_return(ctx, NULL, (-5));
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=10: "));
	mph_native_print_int(ctx, result5a);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=-5: "));
	mph_native_print_int(ctx, result5b);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if ((result5a == 11)) {
{
	if ((result5b == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS\n"));
	passed = (passed + 1);
}
	}
}
	}
	({ MorphString* _arg_6 = mph_string_new(ctx, "Test 6: Multiple Returns in Branches"); mph_gc_push_root(ctx, (void**)&_arg_6); mph_print_test_header(ctx, NULL, _arg_6); mph_gc_pop_roots(ctx, 1); });
	mph_int result6a = mph_test_multiple_returns(ctx, NULL, 15);
	mph_int result6b = mph_test_multiple_returns(ctx, NULL, 7);
	mph_int result6c = mph_test_multiple_returns(ctx, NULL, 2);
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=15: "));
	mph_native_print_int(ctx, result6a);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=7: "));
	mph_native_print_int(ctx, result6b);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Result when x=2: "));
	mph_native_print_int(ctx, result6c);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	if ((result6a == 2)) {
{
	if ((result6b == 1)) {
{
	if ((result6c == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS\n"));
	passed = (passed + 1);
}
	}
}
	}
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "\nââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â  TEST SUMMARY                                      â\n"));
	mph_native_print(ctx, mph_string_new(ctx, "ââââââââââââââââââââââââââââââââââââââââââââââââââââââ\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Tests passed: "));
	mph_native_print_int(ctx, passed);
	mph_native_print(ctx, mph_string_new(ctx, " / "));
	mph_native_print_int(ctx, total);
	mph_native_print(ctx, mph_string_new(ctx, "\n\n"));
	if ((passed == total)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "ð ALL CONTROL FLOW TESTS PASSED! ð\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â ï¸  Some tests failed\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "\nN0 vs N1 Comparison:\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â All-paths-return logic tested\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Nested conditionals working\n"));
	mph_native_print(ctx, mph_string_new(ctx, "â Void functions handled correctly\n"));
	mph_native_print(ctx, mph_string_new(ctx, "! N1 doesn't enforce all-paths-return at compile time yet\n"));
	mph_native_print(ctx, mph_string_new(ctx, "  (This is a known limitation - to be implemented)\n"));
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
