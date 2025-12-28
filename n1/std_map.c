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
#define MPH_TYPE_mph_IntMap 1

// Struct Definitions (Env & Types)
typedef struct mph_IntMap mph_IntMap;
struct mph_IntMap {
	mph_int k0;
	mph_int v0;
	mph_int k1;
	mph_int v1;
	mph_int k2;
	mph_int v2;
	mph_int k3;
	mph_int v3;
	mph_int len;
};

MorphTypeInfo mph_ti_mph_IntMap = { "IntMap", sizeof(mph_IntMap), 0, NULL };

// Global Variables

// Function Prototypes
mph_IntMap* mph_imap_new(MorphContext* ctx, void* _env_void);
mph_IntMap* mph_imap_set(MorphContext* ctx, void* _env_void, mph_IntMap* m, mph_int k, mph_int v);
mph_int mph_imap_get(MorphContext* ctx, void* _env_void, mph_IntMap* m, mph_int k);
void mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_IntMap* mph_imap_new(MorphContext* ctx, void* _env_void) {
	return ({ mph_IntMap* _t = (mph_IntMap*)mph_alloc(ctx, sizeof(mph_IntMap), &mph_ti_mph_IntMap); mph_gc_push_root(ctx, (void**)&_t); _t->len = 0; _t->k0 = (-1); _t->v0 = 0; _t->v1 = 0; _t->v2 = 0; _t->k3 = (-1); _t->v3 = 0; _t->k1 = (-1); _t->k2 = (-1); mph_gc_pop_roots(ctx, 1); _t; });
}

mph_IntMap* mph_imap_set(MorphContext* ctx, void* _env_void, mph_IntMap* m, mph_int k, mph_int v) {
	mph_gc_push_root(ctx, (void**)&m);
	if ((({ mph_IntMap* _obj_1 = m; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->k0; mph_gc_pop_roots(ctx, 1); _ret_2; }) == k)) {
{
	({ mph_IntMap* _obj_3 = m; mph_gc_push_root(ctx, (void**)&_obj_3); _obj_3->v0 = v; mph_gc_pop_roots(ctx, 1); });
	return m;
}
	}
	if ((({ mph_IntMap* _obj_4 = m; mph_gc_push_root(ctx, (void**)&_obj_4); mph_int _ret_5 = _obj_4->k1; mph_gc_pop_roots(ctx, 1); _ret_5; }) == k)) {
{
	({ mph_IntMap* _obj_6 = m; mph_gc_push_root(ctx, (void**)&_obj_6); _obj_6->v1 = v; mph_gc_pop_roots(ctx, 1); });
	return m;
}
	}
	if ((({ mph_IntMap* _obj_7 = m; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->k2; mph_gc_pop_roots(ctx, 1); _ret_8; }) == k)) {
{
	({ mph_IntMap* _obj_9 = m; mph_gc_push_root(ctx, (void**)&_obj_9); _obj_9->v2 = v; mph_gc_pop_roots(ctx, 1); });
	return m;
}
	}
	if ((({ mph_IntMap* _obj_10 = m; mph_gc_push_root(ctx, (void**)&_obj_10); mph_int _ret_11 = _obj_10->k3; mph_gc_pop_roots(ctx, 1); _ret_11; }) == k)) {
{
	({ mph_IntMap* _obj_12 = m; mph_gc_push_root(ctx, (void**)&_obj_12); _obj_12->v3 = v; mph_gc_pop_roots(ctx, 1); });
	return m;
}
	}
	if ((({ mph_IntMap* _obj_13 = m; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->len; mph_gc_pop_roots(ctx, 1); _ret_14; }) == 0)) {
{
	({ mph_IntMap* _obj_15 = m; mph_gc_push_root(ctx, (void**)&_obj_15); _obj_15->k0 = k; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntMap* _obj_16 = m; mph_gc_push_root(ctx, (void**)&_obj_16); _obj_16->v0 = v; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntMap* _obj_17 = m; mph_gc_push_root(ctx, (void**)&_obj_17); _obj_17->len = 1; mph_gc_pop_roots(ctx, 1); });
	return m;
}
	}
	if ((({ mph_IntMap* _obj_18 = m; mph_gc_push_root(ctx, (void**)&_obj_18); mph_int _ret_19 = _obj_18->len; mph_gc_pop_roots(ctx, 1); _ret_19; }) == 1)) {
{
	({ mph_IntMap* _obj_20 = m; mph_gc_push_root(ctx, (void**)&_obj_20); _obj_20->k1 = k; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntMap* _obj_21 = m; mph_gc_push_root(ctx, (void**)&_obj_21); _obj_21->v1 = v; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntMap* _obj_22 = m; mph_gc_push_root(ctx, (void**)&_obj_22); _obj_22->len = 2; mph_gc_pop_roots(ctx, 1); });
	return m;
}
	}
	if ((({ mph_IntMap* _obj_23 = m; mph_gc_push_root(ctx, (void**)&_obj_23); mph_int _ret_24 = _obj_23->len; mph_gc_pop_roots(ctx, 1); _ret_24; }) == 2)) {
{
	({ mph_IntMap* _obj_25 = m; mph_gc_push_root(ctx, (void**)&_obj_25); _obj_25->k2 = k; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntMap* _obj_26 = m; mph_gc_push_root(ctx, (void**)&_obj_26); _obj_26->v2 = v; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntMap* _obj_27 = m; mph_gc_push_root(ctx, (void**)&_obj_27); _obj_27->len = 3; mph_gc_pop_roots(ctx, 1); });
	return m;
}
	}
	if ((({ mph_IntMap* _obj_28 = m; mph_gc_push_root(ctx, (void**)&_obj_28); mph_int _ret_29 = _obj_28->len; mph_gc_pop_roots(ctx, 1); _ret_29; }) == 3)) {
{
	({ mph_IntMap* _obj_30 = m; mph_gc_push_root(ctx, (void**)&_obj_30); _obj_30->k3 = k; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntMap* _obj_31 = m; mph_gc_push_root(ctx, (void**)&_obj_31); _obj_31->v3 = v; mph_gc_pop_roots(ctx, 1); });
	({ mph_IntMap* _obj_32 = m; mph_gc_push_root(ctx, (void**)&_obj_32); _obj_32->len = 4; mph_gc_pop_roots(ctx, 1); });
}
	}
	return m;
	mph_gc_pop_roots(ctx, 1);
}

mph_int mph_imap_get(MorphContext* ctx, void* _env_void, mph_IntMap* m, mph_int k) {
	mph_gc_push_root(ctx, (void**)&m);
	if ((({ mph_IntMap* _obj_33 = m; mph_gc_push_root(ctx, (void**)&_obj_33); mph_int _ret_34 = _obj_33->k0; mph_gc_pop_roots(ctx, 1); _ret_34; }) == k)) {
{
	return ({ mph_IntMap* _obj_35 = m; mph_gc_push_root(ctx, (void**)&_obj_35); mph_int _ret_36 = _obj_35->v0; mph_gc_pop_roots(ctx, 1); _ret_36; });
}
	}
	if ((({ mph_IntMap* _obj_37 = m; mph_gc_push_root(ctx, (void**)&_obj_37); mph_int _ret_38 = _obj_37->k1; mph_gc_pop_roots(ctx, 1); _ret_38; }) == k)) {
{
	return ({ mph_IntMap* _obj_39 = m; mph_gc_push_root(ctx, (void**)&_obj_39); mph_int _ret_40 = _obj_39->v1; mph_gc_pop_roots(ctx, 1); _ret_40; });
}
	}
	if ((({ mph_IntMap* _obj_41 = m; mph_gc_push_root(ctx, (void**)&_obj_41); mph_int _ret_42 = _obj_41->k2; mph_gc_pop_roots(ctx, 1); _ret_42; }) == k)) {
{
	return ({ mph_IntMap* _obj_43 = m; mph_gc_push_root(ctx, (void**)&_obj_43); mph_int _ret_44 = _obj_43->v2; mph_gc_pop_roots(ctx, 1); _ret_44; });
}
	}
	if ((({ mph_IntMap* _obj_45 = m; mph_gc_push_root(ctx, (void**)&_obj_45); mph_int _ret_46 = _obj_45->k3; mph_gc_pop_roots(ctx, 1); _ret_46; }) == k)) {
{
	return ({ mph_IntMap* _obj_47 = m; mph_gc_push_root(ctx, (void**)&_obj_47); mph_int _ret_48 = _obj_47->v3; mph_gc_pop_roots(ctx, 1); _ret_48; });
}
	}
	return (-1);
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	mph_native_print(ctx, mph_string_new(ctx, "Map\n"));
	mph_IntMap* m = mph_imap_new(ctx, NULL);
	mph_gc_push_root(ctx, (void**)&m);
	m = ({ mph_IntMap* _arg_49 = m; mph_gc_push_root(ctx, (void**)&_arg_49); mph_IntMap* _ret_50 = mph_imap_set(ctx, NULL, _arg_49, 1, 100); mph_gc_pop_roots(ctx, 1); _ret_50; });
	m = ({ mph_IntMap* _arg_51 = m; mph_gc_push_root(ctx, (void**)&_arg_51); mph_IntMap* _ret_52 = mph_imap_set(ctx, NULL, _arg_51, 2, 200); mph_gc_pop_roots(ctx, 1); _ret_52; });
	mph_native_print_int(ctx, ({ mph_IntMap* _arg_53 = m; mph_gc_push_root(ctx, (void**)&_arg_53); mph_int _ret_54 = mph_imap_get(ctx, NULL, _arg_53, 1); mph_gc_pop_roots(ctx, 1); _ret_54; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	mph_native_print_int(ctx, ({ mph_IntMap* _arg_55 = m; mph_gc_push_root(ctx, (void**)&_arg_55); mph_int _ret_56 = mph_imap_get(ctx, NULL, _arg_55, 2); mph_gc_pop_roots(ctx, 1); _ret_56; }));
	mph_native_print(ctx, mph_string_new(ctx, "\n"));
	m = ({ mph_IntMap* _arg_57 = m; mph_gc_push_root(ctx, (void**)&_arg_57); mph_IntMap* _ret_58 = mph_imap_set(ctx, NULL, _arg_57, 1, 111); mph_gc_pop_roots(ctx, 1); _ret_58; });
	mph_native_print_int(ctx, ({ mph_IntMap* _arg_59 = m; mph_gc_push_root(ctx, (void**)&_arg_59); mph_int _ret_60 = mph_imap_get(ctx, NULL, _arg_59, 1); mph_gc_pop_roots(ctx, 1); _ret_60; }));
	mph_native_print(ctx, mph_string_new(ctx, "\nOK\n"));
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx, NULL);
}
