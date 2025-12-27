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
#define MPH_TYPE_mph_Variable 2

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_Variable mph_Variable;
struct mph_Variable {
	MorphString* name;
	mph_int var_type;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_Variable = { "Variable", sizeof(mph_Variable), 1, (size_t[]){offsetof(mph_Variable, name)} };

// Global Variables
mph_io_File* mph_io_Stdout;
mph_int mph_KIND_INT;
mph_int mph_KIND_ERROR;

// Function Prototypes
mph_Variable* mph_make_var(MorphContext* ctx, void* _env_void, MorphString* name, mph_int var_type);
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

mph_Variable* mph_make_var(MorphContext* ctx, void* _env_void, MorphString* name, mph_int var_type) {
	mph_gc_push_root(ctx, (void**)&name);
	return ({ mph_Variable* _ret_1 = ({ mph_Variable* _t = (mph_Variable*)mph_alloc(ctx, sizeof(mph_Variable), &mph_ti_mph_Variable); mph_gc_push_root(ctx, (void**)&_t); _t->name = name; _t->var_type = var_type; mph_gc_pop_roots(ctx, 1); _t; }); mph_gc_push_root(ctx, (void**)&_ret_1); mph_Variable* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
	mph_gc_pop_roots(ctx, 1);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_3 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_3); MorphString* _arg_4 = mph_string_new(ctx, "=== SCOPE MINIMAL TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_4); mph_int _ret_5 = mph_io_Write(ctx, NULL, _arg_3, _arg_4); mph_gc_pop_roots(ctx, 2); _ret_5; });
	mph_Variable* x = ({ MorphString* _arg_6 = mph_string_new(ctx, "x"); mph_gc_push_root(ctx, (void**)&_arg_6); mph_Variable* _ret_7 = mph_make_var(ctx, NULL, _arg_6, mph_KIND_INT); mph_gc_pop_roots(ctx, 1); _ret_7; });
	mph_gc_push_root(ctx, (void**)&x);
	({ mph_io_File* _arg_8 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_8); MorphString* _arg_9 = mph_string_new(ctx, "â Variable created\n"); mph_gc_push_root(ctx, (void**)&_arg_9); mph_int _ret_10 = mph_io_Write(ctx, NULL, _arg_8, _arg_9); mph_gc_pop_roots(ctx, 2); _ret_10; });
	if ((({ mph_Variable* _obj_11 = x; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->var_type; mph_gc_pop_roots(ctx, 1); _ret_12; }) == mph_KIND_INT)) {
{
	({ mph_io_File* _arg_13 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_13); MorphString* _arg_14 = mph_string_new(ctx, "â Variable type correct\n"); mph_gc_push_root(ctx, (void**)&_arg_14); mph_int _ret_15 = mph_io_Write(ctx, NULL, _arg_13, _arg_14); mph_gc_pop_roots(ctx, 2); _ret_15; });
}
	} else {
{
	({ mph_io_File* _arg_16 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_16); MorphString* _arg_17 = mph_string_new(ctx, "â Variable type incorrect\n"); mph_gc_push_root(ctx, (void**)&_arg_17); mph_int _ret_18 = mph_io_Write(ctx, NULL, _arg_16, _arg_17); mph_gc_pop_roots(ctx, 2); _ret_18; });
}
	}
	({ mph_io_File* _arg_19 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_19); MorphString* _arg_20 = mph_string_new(ctx, "\nð¯ SCOPE MINIMAL COMPLETE!\n"); mph_gc_push_root(ctx, (void**)&_arg_20); mph_int _ret_21 = mph_io_Write(ctx, NULL, _arg_19, _arg_20); mph_gc_pop_roots(ctx, 2); _ret_21; });
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_KIND_INT = 0;
	mph_KIND_ERROR = (-1);
	mph_main(ctx, NULL);
}
