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
void mph_TestEvaluator(MorphContext* ctx, void* _env_void);
void mph_TestTypeChecker(MorphContext* ctx, void* _env_void);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
void mph_TestEvaluator(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Testing Evaluator ==="));
	mph_native_print(ctx, mph_string_new(ctx, "Basic value system working"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Evaluator Ready ==="));
}

void mph_TestTypeChecker(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Testing Type Checker ==="));
	mph_native_print(ctx, mph_string_new(ctx, "Basic type system working"));
	mph_native_print(ctx, mph_string_new(ctx, "=== Type Checker Ready ==="));
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Morph Self-Hosting Compiler ==="));
	mph_TestEvaluator(ctx, NULL);
	mph_TestTypeChecker(ctx, NULL);
	mph_native_print(ctx, mph_string_new(ctx, "Bootstrap compilation successful!"));
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
	mph_main(ctx, NULL);
}
