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
mph_int mph_K_INT;
mph_int mph_K_STR;
mph_int mph_K_UNK;
mph_int mph_k0;
mph_int mph_k1;
mph_int mph_k2;
mph_int mph_k3;

// Function Prototypes
void mph_def(MorphContext* ctx, void* _env_void, mph_int id, mph_int k);
mph_int mph_get(MorphContext* ctx, void* _env_void, mph_int id);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
void mph_def(MorphContext* ctx, void* _env_void, mph_int id, mph_int k) {
	if ((id == 0)) {
{
	mph_k0 = k;
}
	}
	if ((id == 1)) {
{
	mph_k1 = k;
}
	}
	if ((id == 2)) {
{
	mph_k2 = k;
}
	}
	if ((id == 3)) {
{
	mph_k3 = k;
}
	}
}

mph_int mph_get(MorphContext* ctx, void* _env_void, mph_int id) {
	if ((id == 0)) {
{
	return mph_k0;
}
	}
	if ((id == 1)) {
{
	return mph_k1;
}
	}
	if ((id == 2)) {
{
	return mph_k2;
}
	}
	if ((id == 3)) {
{
	return mph_k3;
}
	}
	return mph_K_UNK;
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "Sym\n"));
	mph_def(ctx, NULL, 0, mph_K_INT);
	mph_def(ctx, NULL, 1, mph_K_STR);
	mph_int r = mph_get(ctx, NULL, 0);
	if ((r == mph_K_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "OK\n"));
}
	}
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_K_INT = 0;
	mph_K_STR = 2;
	mph_K_UNK = (-1);
	mph_k0 = (-1);
	mph_k1 = (-1);
	mph_k2 = (-1);
	mph_k3 = (-1);
	mph_main(ctx, NULL);
}
