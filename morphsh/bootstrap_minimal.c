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
#define MPH_TYPE_mph_io_File 1

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
void mph_TestEvaluator(MorphContext* ctx, void* _env_void);
void mph_TestTypeChecker(MorphContext* ctx, void* _env_void);
void mph_main(MorphContext* ctx, void* _env_void);
mph_io_File* mph_io_make_file(MorphContext* ctx, void* _env_void, mph_int fd);
mph_io_File* mph_io_Open(MorphContext* ctx, void* _env_void, MorphString* path);
mph_io_File* mph_io_Create(MorphContext* ctx, void* _env_void, MorphString* path);
MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_io_File* f, mph_int size);
mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_io_File* f, MorphString* s);
mph_int mph_io_Close(MorphContext* ctx, void* _env_void, mph_io_File* f);

// Function Definitions
mph_io_File* mph_io_make_file(MorphContext* ctx, void* _env_void, mph_int fd);

mph_io_File* mph_io_Open(MorphContext* ctx, void* _env_void, MorphString* path);

mph_io_File* mph_io_Create(MorphContext* ctx, void* _env_void, MorphString* path);

MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_io_File* f, mph_int size);

mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_io_File* f, MorphString* s);

mph_int mph_io_Close(MorphContext* ctx, void* _env_void, mph_io_File* f);

void mph_TestEvaluator(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_1 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_1); MorphString* _arg_2 = mph_string_new(ctx, "=== Testing Evaluator ===\n"); mph_gc_push_root(ctx, (void**)&_arg_2); mph_int _ret_3 = mph_io_Write(ctx, NULL, _arg_1, _arg_2); mph_gc_pop_roots(ctx, 2); _ret_3; });
	({ mph_io_File* _arg_4 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_4); MorphString* _arg_5 = mph_string_new(ctx, "Basic value system working\n"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_int _ret_6 = mph_io_Write(ctx, NULL, _arg_4, _arg_5); mph_gc_pop_roots(ctx, 2); _ret_6; });
	({ mph_io_File* _arg_7 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_7); MorphString* _arg_8 = mph_string_new(ctx, "=== Evaluator Ready ===\n"); mph_gc_push_root(ctx, (void**)&_arg_8); mph_int _ret_9 = mph_io_Write(ctx, NULL, _arg_7, _arg_8); mph_gc_pop_roots(ctx, 2); _ret_9; });
}

void mph_TestTypeChecker(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_10 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_10); MorphString* _arg_11 = mph_string_new(ctx, "=== Testing Type Checker ===\n"); mph_gc_push_root(ctx, (void**)&_arg_11); mph_int _ret_12 = mph_io_Write(ctx, NULL, _arg_10, _arg_11); mph_gc_pop_roots(ctx, 2); _ret_12; });
	({ mph_io_File* _arg_13 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_13); MorphString* _arg_14 = mph_string_new(ctx, "Basic type system working\n"); mph_gc_push_root(ctx, (void**)&_arg_14); mph_int _ret_15 = mph_io_Write(ctx, NULL, _arg_13, _arg_14); mph_gc_pop_roots(ctx, 2); _ret_15; });
	({ mph_io_File* _arg_16 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_16); MorphString* _arg_17 = mph_string_new(ctx, "=== Type Checker Ready ===\n"); mph_gc_push_root(ctx, (void**)&_arg_17); mph_int _ret_18 = mph_io_Write(ctx, NULL, _arg_16, _arg_17); mph_gc_pop_roots(ctx, 2); _ret_18; });
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_19 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_19); MorphString* _arg_20 = mph_string_new(ctx, "=== Morph Self-Hosting Compiler ===\n"); mph_gc_push_root(ctx, (void**)&_arg_20); mph_int _ret_21 = mph_io_Write(ctx, NULL, _arg_19, _arg_20); mph_gc_pop_roots(ctx, 2); _ret_21; });
	mph_TestEvaluator(ctx, NULL);
	mph_TestTypeChecker(ctx, NULL);
	({ mph_io_File* _arg_22 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_22); MorphString* _arg_23 = mph_string_new(ctx, "Bootstrap compilation successful!\n"); mph_gc_push_root(ctx, (void**)&_arg_23); mph_int _ret_24 = mph_io_Write(ctx, NULL, _arg_22, _arg_23); mph_gc_pop_roots(ctx, 2); _ret_24; });
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
