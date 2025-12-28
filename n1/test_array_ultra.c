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
#define MPH_TYPE_mph_ArrayType 1

// Struct Definitions (Env & Types)
typedef struct mph_ArrayType mph_ArrayType;
struct mph_ArrayType {
	mph_int element_kind;
};

MorphTypeInfo mph_ti_mph_ArrayType = { "ArrayType", sizeof(mph_ArrayType), 0, NULL };

// Global Variables
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;

// Function Prototypes
mph_ArrayType* mph_make_array(MorphContext* ctx, void* _env_void, mph_int k);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_ArrayType* mph_make_array(MorphContext* ctx, void* _env_void, mph_int k) {
	return ({ mph_ArrayType* _t = (mph_ArrayType*)mph_alloc(ctx, sizeof(mph_ArrayType), &mph_ti_mph_ArrayType); mph_gc_push_root(ctx, (void**)&_t); _t->element_kind = k; mph_gc_pop_roots(ctx, 1); _t; });
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_ArrayType* arr = mph_make_array(ctx, NULL, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&arr);
	mph_native_print(ctx, mph_string_new(ctx, "ArrayType element_kind: "));
	if ((({ mph_ArrayType* _obj_1 = arr; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->element_kind; mph_gc_pop_roots(ctx, 1); _ret_2; }) == mph_KIND_INT)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "INT - OK\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL\n"));
}
	}
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_STRING = 2;
	mph_main(ctx, NULL);
}
