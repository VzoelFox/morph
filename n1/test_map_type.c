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
#define MPH_TYPE_mph_MapType 1

// Struct Definitions (Env & Types)
typedef struct mph_MapType mph_MapType;
struct mph_MapType {
	mph_int key_kind;
	mph_int value_kind;
};

MorphTypeInfo mph_ti_mph_MapType = { "MapType", sizeof(mph_MapType), 0, NULL };

// Global Variables
mph_int mph_KIND_INT;
mph_int mph_KIND_STRING;

// Function Prototypes
mph_MapType* mph_make_map(MorphContext* ctx, void* _env_void, mph_int k, mph_int v);
mph_bool mph_map_check_key(MorphContext* ctx, void* _env_void, mph_MapType* m, mph_int k);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_MapType* mph_make_map(MorphContext* ctx, void* _env_void, mph_int k, mph_int v) {
	return ({ mph_MapType* _t = (mph_MapType*)mph_alloc(ctx, sizeof(mph_MapType), &mph_ti_mph_MapType); mph_gc_push_root(ctx, (void**)&_t); _t->key_kind = k; _t->value_kind = v; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_bool mph_map_check_key(MorphContext* ctx, void* _env_void, mph_MapType* m, mph_int k) {
	mph_gc_push_root(ctx, (void**)&m);
	return (({ mph_MapType* _obj_1 = m; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->key_kind; mph_gc_pop_roots(ctx, 1); _ret_2; }) == k);
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "MapType Test\n"));
	mph_native_print(ctx, mph_string_new(ctx, "============\n"));
	mph_MapType* m = mph_make_map(ctx, NULL, mph_KIND_STRING, mph_KIND_INT);
	mph_gc_push_root(ctx, (void**)&m);
	mph_native_print(ctx, mph_string_new(ctx, "map[string]int created\n"));
	if (({ mph_MapType* _arg_3 = m; mph_gc_push_root(ctx, (void**)&_arg_3); mph_bool _ret_4 = mph_map_check_key(ctx, NULL, _arg_3, mph_KIND_STRING); mph_gc_pop_roots(ctx, 1); _ret_4; })) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â key check string OK\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â key check FAIL\n"));
}
	}
	if ((({ mph_MapType* _arg_5 = m; mph_gc_push_root(ctx, (void**)&_arg_5); mph_bool _ret_6 = mph_map_check_key(ctx, NULL, _arg_5, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_6; }) == 0)) {
{
	mph_native_print(ctx, mph_string_new(ctx, "â key check int rejected OK\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "â should reject int key\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "MapType OK!\n"));
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_STRING = 2;
	mph_main(ctx, NULL);
}
