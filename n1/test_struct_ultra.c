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
#define MPH_TYPE_mph_StructType 1

// Struct Definitions (Env & Types)
typedef struct mph_StructType mph_StructType;
struct mph_StructType {
	MorphString* name;
	MorphString* f0_name;
	mph_int f0_kind;
};

MorphTypeInfo mph_ti_mph_StructType = { "StructType", sizeof(mph_StructType), 2, (size_t[]){offsetof(mph_StructType, name), offsetof(mph_StructType, f0_name)} };

// Global Variables

// Function Prototypes
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
void mph_main(MorphContext* ctx, void* _env_void) {
	mph_StructType* s = ({ mph_StructType* _t = (mph_StructType*)mph_alloc(ctx, sizeof(mph_StructType), &mph_ti_mph_StructType); mph_gc_push_root(ctx, (void**)&_t); _t->name = mph_string_new(ctx, "Person"); _t->f0_name = mph_string_new(ctx, "age"); _t->f0_kind = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&s);
	mph_native_print(ctx, mph_string_new(ctx, "Struct name: "));
	mph_native_print(ctx, ({ mph_StructType* _obj_1 = s; mph_gc_push_root(ctx, (void**)&_obj_1); MorphString* _ret_2 = _obj_1->name; mph_gc_pop_roots(ctx, 1); _ret_2; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Field name: "));
	mph_native_print(ctx, ({ mph_StructType* _obj_3 = s; mph_gc_push_root(ctx, (void**)&_obj_3); MorphString* _ret_4 = _obj_3->f0_name; mph_gc_pop_roots(ctx, 1); _ret_4; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print(ctx, mph_string_new(ctx, "OK!\n"));
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
