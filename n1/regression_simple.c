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
mph_int mph_test_arithmetic(MorphContext* ctx, void* _env_void);
mph_bool mph_test_conditional(MorphContext* ctx, void* _env_void, mph_int x);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_int mph_test_arithmetic(MorphContext* ctx, void* _env_void) {
	mph_int a = 10;
	mph_int b = 20;
	mph_int sum = (a + b);
	return sum;
}

mph_bool mph_test_conditional(MorphContext* ctx, void* _env_void, mph_int x) {
	if ((x == 5)) {
{
	return 1;
}
	} else {
{
	return 0;
}
	}
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== N1 Regression Test - Simple ===\n\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Test 1: Arithmetic (10 + 20)\n"));
	mph_int result = mph_test_arithmetic(ctx, NULL);
	mph_native_print(ctx, mph_string_new(ctx, "Result: "));
	mph_native_print_int(ctx, result);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Expected: 30\n"));
	if ((result == 30)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS\n\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL\n\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Test 2: Conditional (x == 5)\n"));
	mph_bool cond1 = mph_test_conditional(ctx, NULL, 5);
	mph_native_print(ctx, mph_string_new(ctx, "Result: "));
	if (cond1) {
{
	mph_native_print(ctx, mph_string_new(ctx, "true\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "false\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Expected: true\n"));
	if (cond1) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS\n\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL\n\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Test 3: Conditional (x == 10 when x=5)\n"));
	mph_bool cond2 = mph_test_conditional(ctx, NULL, 10);
	mph_native_print(ctx, mph_string_new(ctx, "Result: "));
	if (cond2) {
{
	mph_native_print(ctx, mph_string_new(ctx, "true\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "false\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Expected: false\n"));
	if ((cond2 == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS\n\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL\n\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "=== Summary ===\n"));
	mph_native_print(ctx, mph_string_new(ctx, "All basic tests completed.\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Compare with N0 compiler output.\n"));
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
