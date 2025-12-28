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
mph_int mph_std_types_KIND_INT;
mph_int mph_std_types_KIND_FLOAT;
mph_int mph_std_types_KIND_STR;
mph_int mph_std_types_KIND_BOOL;
mph_int mph_std_types_KIND_VOID;
mph_int mph_std_types_KIND_ERR;
mph_int mph_std_types_KIND_UNK;

// Function Prototypes
void mph_main(MorphContext* ctx, void* _env_void);
mph_int mph_std_types_GetKindInt(MorphContext* ctx, void* _env_void);
mph_int mph_std_types_GetKindStr(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_int mph_std_types_GetKindInt(MorphContext* ctx, void* _env_void) {
	return 0;
}

mph_int mph_std_types_GetKindStr(MorphContext* ctx, void* _env_void) {
	return 2;
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "Import Vars Test\n"));
	mph_native_print_int(ctx, mph_std_types_KIND_INT);
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print_int(ctx, mph_std_types_KIND_STR);
	mph_native_print(ctx, mph_string_new(ctx, "\nOK\n"));
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_std_types_KIND_INT = 0;
	mph_std_types_KIND_FLOAT = 1;
	mph_std_types_KIND_STR = 2;
	mph_std_types_KIND_BOOL = 3;
	mph_std_types_KIND_VOID = 4;
	mph_std_types_KIND_ERR = (-1);
	mph_std_types_KIND_UNK = (-2);
	mph_main(ctx, NULL);
}
