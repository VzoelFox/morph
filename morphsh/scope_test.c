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
#define MPH_TYPE_mph_Scope 2

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

typedef struct mph_Scope mph_Scope;
struct mph_Scope {
	mph_int level;
	mph_int count;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };
MorphTypeInfo mph_ti_mph_Scope = { "Scope", sizeof(mph_Scope), 0, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
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

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_1 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_1); MorphString* _arg_2 = mph_string_new(ctx, "=== SCOPE TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_2); mph_int _ret_3 = mph_io_Write(ctx, NULL, _arg_1, _arg_2); mph_gc_pop_roots(ctx, 2); _ret_3; });
	mph_Scope* s = ({ mph_Scope* _t = (mph_Scope*)mph_alloc(ctx, sizeof(mph_Scope), &mph_ti_mph_Scope); mph_gc_push_root(ctx, (void**)&_t); _t->count = 1; _t->level = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&s);
	({ mph_io_File* _arg_4 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_4); MorphString* _arg_5 = mph_string_new(ctx, "â Scope created\n"); mph_gc_push_root(ctx, (void**)&_arg_5); mph_int _ret_6 = mph_io_Write(ctx, NULL, _arg_4, _arg_5); mph_gc_pop_roots(ctx, 2); _ret_6; });
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}
