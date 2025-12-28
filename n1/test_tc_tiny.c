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
mph_int mph_KIND_INT;
mph_int mph_KIND_STR;

// Function Prototypes
mph_int mph_check_add(MorphContext* ctx, void* _env_void, mph_int l, mph_int r);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_int mph_check_add(MorphContext* ctx, void* _env_void, mph_int l, mph_int r) {
	if ((l == mph_KIND_INT)) {
{
	if ((r == mph_KIND_INT)) {
{
	return mph_KIND_INT;
}
	}
}
	}
	return (-1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "TC Test\n"));
	mph_int r = mph_check_add(ctx, NULL, mph_KIND_INT, mph_KIND_INT);
	if ((r == mph_KIND_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "OK\n"));
}
	}
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_STR = 2;
	mph_main(ctx, NULL);
}
