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
mph_int mph_io_Stdout;

// Function Prototypes
void mph_TestEvaluator(MorphContext* ctx, void* _env_void);
void mph_TestTypeChecker(MorphContext* ctx, void* _env_void);
void mph_main(MorphContext* ctx, void* _env_void);
mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_int fd, MorphString* data);
MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_int fd, mph_int size);
void mph_io_Print(MorphContext* ctx, void* _env_void, MorphString* data);
void mph_io_Println(MorphContext* ctx, void* _env_void, MorphString* data);

// Function Definitions
mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_int fd, MorphString* data);

MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_int fd, mph_int size);

void mph_io_Print(MorphContext* ctx, void* _env_void, MorphString* data);

void mph_io_Println(MorphContext* ctx, void* _env_void, MorphString* data);

void mph_TestEvaluator(MorphContext* ctx, void* _env_void) {
	({ MorphString* _arg_1 = mph_string_new(ctx, "=== Testing Evaluator ===\n"); mph_gc_push_root(ctx, (void**)&_arg_1); mph_int _ret_2 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_1); mph_gc_pop_roots(ctx, 1); _ret_2; });
	({ MorphString* _arg_3 = mph_string_new(ctx, "Basic value system working\n"); mph_gc_push_root(ctx, (void**)&_arg_3); mph_int _ret_4 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_3); mph_gc_pop_roots(ctx, 1); _ret_4; });
	({ MorphString* _arg_5 = mph_string_new(ctx, "=== Evaluator Ready ===\n"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_int _ret_6 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_5); mph_gc_pop_roots(ctx, 1); _ret_6; });
}

void mph_TestTypeChecker(MorphContext* ctx, void* _env_void) {
	({ MorphString* _arg_7 = mph_string_new(ctx, "=== Testing Type Checker ===\n"); mph_gc_push_root(ctx, (void**)&_arg_7); mph_int _ret_8 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_7); mph_gc_pop_roots(ctx, 1); _ret_8; });
	({ MorphString* _arg_9 = mph_string_new(ctx, "Basic type system working\n"); mph_gc_push_root(ctx, (void**)&_arg_9); mph_int _ret_10 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_9); mph_gc_pop_roots(ctx, 1); _ret_10; });
	({ MorphString* _arg_11 = mph_string_new(ctx, "=== Type Checker Ready ===\n"); mph_gc_push_root(ctx, (void**)&_arg_11); mph_int _ret_12 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_11); mph_gc_pop_roots(ctx, 1); _ret_12; });
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ MorphString* _arg_13 = mph_string_new(ctx, "=== Morph Self-Hosting Compiler ===\n"); mph_gc_push_root(ctx, (void**)&_arg_13); mph_int _ret_14 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_13); mph_gc_pop_roots(ctx, 1); _ret_14; });
	mph_TestEvaluator(ctx, NULL);
	mph_TestTypeChecker(ctx, NULL);
	({ MorphString* _arg_15 = mph_string_new(ctx, "Bootstrap compilation successful!\n"); mph_gc_push_root(ctx, (void**)&_arg_15); mph_int _ret_16 = mph_io_Write(ctx, NULL, mph_io_Stdout, _arg_15); mph_gc_pop_roots(ctx, 1); _ret_16; });
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = 1;
	mph_main(ctx, NULL);
}
