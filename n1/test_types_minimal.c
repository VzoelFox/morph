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
#define MPH_TYPE_mph_Type 1

// Struct Definitions (Env & Types)
typedef struct mph_Type mph_Type;
struct mph_Type {
	mph_int kind;
	MorphString* name;
};

MorphTypeInfo mph_ti_mph_Type = { "Type", sizeof(mph_Type), 1, (size_t[]){offsetof(mph_Type, name)} };

// Global Variables
mph_int mph_KIND_INT;
mph_int mph_KIND_FLOAT;
mph_int mph_KIND_STRING;
mph_int mph_KIND_BOOL;
mph_int mph_KIND_VOID;

// Function Prototypes
mph_Type* mph_make_type(MorphContext* ctx, void* _env_void, mph_int k, MorphString* n);
mph_bool mph_type_equals(MorphContext* ctx, void* _env_void, mph_Type* t1, mph_Type* t2);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_Type* mph_make_type(MorphContext* ctx, void* _env_void, mph_int k, MorphString* n) {
	mph_gc_push_root(ctx, (void**)&n);
	return ({ mph_Type* _t = (mph_Type*)mph_alloc(ctx, sizeof(mph_Type), &mph_ti_mph_Type); mph_gc_push_root(ctx, (void**)&_t); _t->kind = k; _t->name = n; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_type_equals(MorphContext* ctx, void* _env_void, mph_Type* t1, mph_Type* t2) {
	mph_gc_push_root(ctx, (void**)&t1);
	mph_gc_push_root(ctx, (void**)&t2);
	return (({ mph_Type* _obj_1 = t1; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->kind; mph_gc_pop_roots(ctx, 1); _ret_2; }) == ({ mph_Type* _obj_3 = t2; mph_gc_push_root(ctx, (void**)&_obj_3); mph_int _ret_4 = _obj_3->kind; mph_gc_pop_roots(ctx, 1); _ret_4; }));
	mph_gc_pop_roots(ctx, 2);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_Type* int_type = ({ MorphString* _arg_5 = mph_string_new(ctx, "int"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_Type* _ret_6 = mph_make_type(ctx, NULL, mph_KIND_INT, _arg_5); mph_gc_pop_roots(ctx, 1); _ret_6; });
	mph_gc_push_root(ctx, (void**)&int_type);
	mph_Type* float_type = ({ MorphString* _arg_7 = mph_string_new(ctx, "float"); mph_gc_push_root(ctx, (void**)&_arg_7); mph_Type* _ret_8 = mph_make_type(ctx, NULL, mph_KIND_FLOAT, _arg_7); mph_gc_pop_roots(ctx, 1); _ret_8; });
	mph_gc_push_root(ctx, (void**)&float_type);
	mph_bool same = ({ mph_Type* _arg_9 = int_type; mph_gc_push_root(ctx, (void**)&_arg_9); mph_Type* _arg_10 = int_type; mph_gc_push_root(ctx, (void**)&_arg_10); mph_bool _ret_11 = mph_type_equals(ctx, NULL, _arg_9, _arg_10); mph_gc_pop_roots(ctx, 2); _ret_11; });
	mph_bool diff = ({ mph_Type* _arg_12 = int_type; mph_gc_push_root(ctx, (void**)&_arg_12); mph_Type* _arg_13 = float_type; mph_gc_push_root(ctx, (void**)&_arg_13); mph_bool _ret_14 = mph_type_equals(ctx, NULL, _arg_12, _arg_13); mph_gc_pop_roots(ctx, 2); _ret_14; });
	mph_native_print(ctx, mph_string_new(ctx, "N1 Type System - Minimal Test\n"));
	mph_native_print(ctx, mph_string_new(ctx, "================================\n"));
	mph_native_print(ctx, mph_string_new(ctx, "int == int: "));
	if (same) {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "int == float: "));
	if (diff) {
{
	mph_native_print(ctx, mph_string_new(ctx, "FAIL (should be false)\n"));
}
	} else {
{
	mph_native_print(ctx, mph_string_new(ctx, "PASS (correctly false)\n"));
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "================================\n"));
	mph_native_print(ctx, mph_string_new(ctx, "Type system foundation working!\n"));
	mph_gc_pop_roots(ctx, 2);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_KIND_INT = 0;
	mph_KIND_FLOAT = 1;
	mph_KIND_STRING = 2;
	mph_KIND_BOOL = 3;
	mph_KIND_VOID = 4;
	mph_main(ctx, NULL);
}
