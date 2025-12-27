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
#define MPH_TYPE_mph_Worker 2
#define MPH_TYPE_mph_Unit 3
#define MPH_TYPE_mph_io_File 1

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_Worker mph_Worker;
typedef struct mph_Unit mph_Unit;
struct mph_Worker {
	mph_int id;
	mph_int clearance;
	MorphString* zone;
};

struct mph_Unit {
	mph_int id;
	mph_int secLevel;
	MorphString* memZone;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_Worker = { "Worker", sizeof(mph_Worker), 1, (size_t[]){offsetof(mph_Worker, zone)} };
MorphTypeInfo mph_ti_mph_Unit = { "Unit", sizeof(mph_Unit), 1, (size_t[]){offsetof(mph_Unit, memZone)} };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
mph_Worker* mph_NewWorker(MorphContext* ctx, void* _env_void, mph_int id, mph_int clearance, MorphString* zone);
mph_Unit* mph_NewUnit(MorphContext* ctx, void* _env_void, mph_int id, mph_int secLevel, MorphString* memZone);
mph_bool mph_CanMigrate(MorphContext* ctx, void* _env_void, mph_Worker* worker, mph_Unit* unit);
void mph_TestMorphroutines(MorphContext* ctx, void* _env_void);
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

mph_Worker* mph_NewWorker(MorphContext* ctx, void* _env_void, mph_int id, mph_int clearance, MorphString* zone) {
	mph_gc_push_root(ctx, (void**)&zone);
	return ({ mph_Worker* _ret_1 = ({ mph_Worker* _t = (mph_Worker*)mph_alloc(ctx, sizeof(mph_Worker), &mph_ti_mph_Worker); mph_gc_push_root(ctx, (void**)&_t); _t->id = id; _t->clearance = clearance; _t->zone = zone; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_Worker* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
	mph_gc_pop_roots(ctx, 1);
}

mph_Unit* mph_NewUnit(MorphContext* ctx, void* _env_void, mph_int id, mph_int secLevel, MorphString* memZone) {
	mph_gc_push_root(ctx, (void**)&memZone);
	return ({ mph_Unit* _ret_3 = ({ mph_Unit* _t = (mph_Unit*)mph_alloc(ctx, sizeof(mph_Unit), &mph_ti_mph_Unit); mph_gc_push_root(ctx, (void**)&_t); _t->id = id; _t->secLevel = secLevel; _t->memZone = memZone; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_3); mph_Unit* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_CanMigrate(MorphContext* ctx, void* _env_void, mph_Worker* worker, mph_Unit* unit) {
	mph_gc_push_root(ctx, (void**)&worker);
	mph_gc_push_root(ctx, (void**)&unit);
	if ((({ mph_Worker* _obj_5 = worker; mph_gc_push_root(ctx, (void**)&_obj_5); mph_int _ret_6 = _obj_5->clearance; mph_gc_pop_roots(ctx, 1); _ret_6; }) >= ({ mph_Unit* _obj_7 = unit; mph_gc_push_root(ctx, (void**)&_obj_7); mph_int _ret_8 = _obj_7->secLevel; mph_gc_pop_roots(ctx, 1); _ret_8; }))) {
{
	if (mph_string_eq(ctx, ({ mph_Worker* _obj_9 = worker; mph_gc_push_root(ctx, (void**)&_obj_9); MorphString* _ret_10 = _obj_9->zone; mph_gc_pop_roots(ctx, 1); _ret_10; }), ({ mph_Unit* _obj_11 = unit; mph_gc_push_root(ctx, (void**)&_obj_11); MorphString* _ret_12 = _obj_11->memZone; mph_gc_pop_roots(ctx, 1); _ret_12; }))) {
{
	return 1;
}
	}
}
	}
	return 0;
	mph_gc_pop_roots(ctx, 2);
}

void mph_TestMorphroutines(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_13 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_13); MorphString* _arg_14 = mph_string_new(ctx, "=== Morphroutines Security Test ==="); mph_gc_push_root(ctx, (void**)&_arg_14); mph_int _ret_15 = mph_io_Write(ctx, NULL, _arg_13, _arg_14); mph_gc_pop_roots(ctx, 2); _ret_15; });
	mph_Worker* worker1 = ({ MorphString* _arg_16 = mph_string_new(ctx, "ZONE_A"); mph_gc_push_root(ctx, (void**)&_arg_16); mph_Worker* _ret_17 = mph_NewWorker(ctx, NULL, 1, 1, _arg_16); mph_gc_pop_roots(ctx, 1); _ret_17; });
	mph_gc_push_root(ctx, (void**)&worker1);
	mph_Worker* worker2 = ({ MorphString* _arg_18 = mph_string_new(ctx, "ZONE_B"); mph_gc_push_root(ctx, (void**)&_arg_18); mph_Worker* _ret_19 = mph_NewWorker(ctx, NULL, 2, 2, _arg_18); mph_gc_pop_roots(ctx, 1); _ret_19; });
	mph_gc_push_root(ctx, (void**)&worker2);
	mph_Unit* unit1 = ({ MorphString* _arg_20 = mph_string_new(ctx, "ZONE_A"); mph_gc_push_root(ctx, (void**)&_arg_20); mph_Unit* _ret_21 = mph_NewUnit(ctx, NULL, 1, 1, _arg_20); mph_gc_pop_roots(ctx, 1); _ret_21; });
	mph_gc_push_root(ctx, (void**)&unit1);
	mph_Unit* unit2 = ({ MorphString* _arg_22 = mph_string_new(ctx, "ZONE_B"); mph_gc_push_root(ctx, (void**)&_arg_22); mph_Unit* _ret_23 = mph_NewUnit(ctx, NULL, 2, 2, _arg_22); mph_gc_pop_roots(ctx, 1); _ret_23; });
	mph_gc_push_root(ctx, (void**)&unit2);
	if (({ mph_Worker* _arg_24 = worker1; mph_gc_push_root(ctx, (void**)&_arg_24); mph_Unit* _arg_25 = unit1; mph_gc_push_root(ctx, (void**)&_arg_25); mph_bool _ret_26 = mph_CanMigrate(ctx, NULL, _arg_24, _arg_25); mph_gc_pop_roots(ctx, 2); _ret_26; })) {
{
	({ mph_io_File* _arg_27 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_27); MorphString* _arg_28 = mph_string_new(ctx, "Worker 1 â Unit 1: ALLOWED"); mph_gc_push_root(ctx, (void**)&_arg_28); mph_int _ret_29 = mph_io_Write(ctx, NULL, _arg_27, _arg_28); mph_gc_pop_roots(ctx, 2); _ret_29; });
}
	} else {
{
	({ mph_io_File* _arg_30 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_30); MorphString* _arg_31 = mph_string_new(ctx, "Worker 1 â Unit 1: DENIED"); mph_gc_push_root(ctx, (void**)&_arg_31); mph_int _ret_32 = mph_io_Write(ctx, NULL, _arg_30, _arg_31); mph_gc_pop_roots(ctx, 2); _ret_32; });
}
	}
	if (({ mph_Worker* _arg_33 = worker1; mph_gc_push_root(ctx, (void**)&_arg_33); mph_Unit* _arg_34 = unit2; mph_gc_push_root(ctx, (void**)&_arg_34); mph_bool _ret_35 = mph_CanMigrate(ctx, NULL, _arg_33, _arg_34); mph_gc_pop_roots(ctx, 2); _ret_35; })) {
{
	({ mph_io_File* _arg_36 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_36); MorphString* _arg_37 = mph_string_new(ctx, "Worker 1 â Unit 2: ALLOWED"); mph_gc_push_root(ctx, (void**)&_arg_37); mph_int _ret_38 = mph_io_Write(ctx, NULL, _arg_36, _arg_37); mph_gc_pop_roots(ctx, 2); _ret_38; });
}
	} else {
{
	({ mph_io_File* _arg_39 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_39); MorphString* _arg_40 = mph_string_new(ctx, "Worker 1 â Unit 2: DENIED (Security)\n"); mph_gc_push_root(ctx, (void**)&_arg_40); mph_int _ret_41 = mph_io_Write(ctx, NULL, _arg_39, _arg_40); mph_gc_pop_roots(ctx, 2); _ret_41; });
}
	}
	if (({ mph_Worker* _arg_42 = worker2; mph_gc_push_root(ctx, (void**)&_arg_42); mph_Unit* _arg_43 = unit2; mph_gc_push_root(ctx, (void**)&_arg_43); mph_bool _ret_44 = mph_CanMigrate(ctx, NULL, _arg_42, _arg_43); mph_gc_pop_roots(ctx, 2); _ret_44; })) {
{
	({ mph_io_File* _arg_45 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_45); MorphString* _arg_46 = mph_string_new(ctx, "Worker 2 â Unit 2: ALLOWED"); mph_gc_push_root(ctx, (void**)&_arg_46); mph_int _ret_47 = mph_io_Write(ctx, NULL, _arg_45, _arg_46); mph_gc_pop_roots(ctx, 2); _ret_47; });
}
	} else {
{
	({ mph_io_File* _arg_48 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_48); MorphString* _arg_49 = mph_string_new(ctx, "Worker 2 â Unit 2: DENIED"); mph_gc_push_root(ctx, (void**)&_arg_49); mph_int _ret_50 = mph_io_Write(ctx, NULL, _arg_48, _arg_49); mph_gc_pop_roots(ctx, 2); _ret_50; });
}
	}
	({ mph_io_File* _arg_51 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_51); MorphString* _arg_52 = mph_string_new(ctx, "=== Security System Works ==="); mph_gc_push_root(ctx, (void**)&_arg_52); mph_int _ret_53 = mph_io_Write(ctx, NULL, _arg_51, _arg_52); mph_gc_pop_roots(ctx, 2); _ret_53; });
	mph_gc_pop_roots(ctx, 4);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_54 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_54); MorphString* _arg_55 = mph_string_new(ctx, "=== Morphroutines Security ==="); mph_gc_push_root(ctx, (void**)&_arg_55); mph_int _ret_56 = mph_io_Write(ctx, NULL, _arg_54, _arg_55); mph_gc_pop_roots(ctx, 2); _ret_56; });
	mph_TestMorphroutines(ctx, NULL);
	({ mph_io_File* _arg_57 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_57); MorphString* _arg_58 = mph_string_new(ctx, "Security clearance ready!"); mph_gc_push_root(ctx, (void**)&_arg_58); mph_int _ret_59 = mph_io_Write(ctx, NULL, _arg_57, _arg_58); mph_gc_pop_roots(ctx, 2); _ret_59; });
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
