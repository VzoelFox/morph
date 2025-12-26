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
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "=== Morph Self-Hosting Compiler ==="));
	MorphString* sourceFile = mph_string_new(ctx, "test.fox");
	mph_gc_push_root(ctx, (void**)&sourceFile);
	MorphString* outputFile = mph_string_new(ctx, "test.c");
	mph_gc_push_root(ctx, (void**)&outputFile);
	mph_native_print(ctx, mph_string_new(ctx, "Compiling:"));
	mph_native_print(ctx, sourceFile);
	mph_native_print(ctx, mph_string_new(ctx, "[1/4] Reading source..."));
	MorphString* sourceCode = mph_string_new(ctx, "fungsi hello() void native_print(\"Hello from Morph!\") akhir");
	mph_gc_push_root(ctx, (void**)&sourceCode);
	mph_native_print(ctx, mph_string_new(ctx, "[2/4] Lexical analysis..."));
	mph_native_print(ctx, mph_string_new(ctx, "[3/4] Parsing..."));
	mph_native_print(ctx, mph_string_new(ctx, "[4/4] Code generation..."));
	mph_native_print(ctx, mph_string_new(ctx, "Output:"));
	mph_native_print(ctx, outputFile);
	mph_native_print(ctx, mph_string_new(ctx, "=== Compilation successful! ==="));
	mph_native_print(ctx, mph_string_new(ctx, "Self-hosting compiler ready for implementation"));
	mph_gc_pop_roots(ctx, 3);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
	mph_main(ctx, NULL);
}
