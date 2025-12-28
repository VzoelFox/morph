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
#define MPH_TYPE_mph_Token 1

// Struct Definitions (Env & Types)
typedef struct mph_Token mph_Token;
struct mph_Token {
	mph_int token_type;
	mph_int value;
};

MorphTypeInfo mph_ti_mph_Token = { "Token", sizeof(mph_Token), 0, NULL };

// Global Variables
mph_int mph_TOKEN_INT;
mph_int mph_TOKEN_PLUS;
mph_int mph_TOKEN_EOF;

// Function Prototypes
mph_Token* mph_make_token(MorphContext* ctx, void* _env_void, mph_int t_type, mph_int val);
mph_int mph_tokenize(MorphContext* ctx, void* _env_void);
mph_int mph_main(MorphContext* ctx, void* _env_void);

// Function Definitions
mph_Token* mph_make_token(MorphContext* ctx, void* _env_void, mph_int t_type, mph_int val) {
	return ({ mph_Token* _t = (mph_Token*)mph_alloc(ctx, sizeof(mph_Token), &mph_ti_mph_Token); mph_gc_push_root(ctx, (void**)&_t); _t->value = val; _t->token_type = t_type; mph_gc_pop_roots(ctx, 1); _t; });
}

mph_int mph_tokenize(MorphContext* ctx, void* _env_void) {
	mph_Token* tok1 = mph_make_token(ctx, NULL, mph_TOKEN_INT, 42);
	mph_gc_push_root(ctx, (void**)&tok1);
	mph_Token* tok2 = mph_make_token(ctx, NULL, mph_TOKEN_PLUS, 0);
	mph_gc_push_root(ctx, (void**)&tok2);
	mph_Token* tok3 = mph_make_token(ctx, NULL, mph_TOKEN_INT, 10);
	mph_gc_push_root(ctx, (void**)&tok3);
	mph_Token* tok4 = mph_make_token(ctx, NULL, mph_TOKEN_EOF, 0);
	mph_gc_push_root(ctx, (void**)&tok4);
	if ((({ mph_Token* _obj_1 = tok1; mph_gc_push_root(ctx, (void**)&_obj_1); mph_int _ret_2 = _obj_1->token_type; mph_gc_pop_roots(ctx, 1); _ret_2; }) == mph_TOKEN_INT)) {
{
	if ((({ mph_Token* _obj_3 = tok1; mph_gc_push_root(ctx, (void**)&_obj_3); mph_int _ret_4 = _obj_3->value; mph_gc_pop_roots(ctx, 1); _ret_4; }) == 42)) {
{
	return 1;
}
	}
}
	}
	return 0;
	mph_gc_pop_roots(ctx, 4);
}

mph_int mph_main(MorphContext* ctx, void* _env_void) {
	mph_int result = mph_tokenize(ctx, NULL);
	if ((result == 1)) {
{
	return 42;
}
	}
	return 1;
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_TOKEN_INT = 1;
	mph_TOKEN_PLUS = 2;
	mph_TOKEN_EOF = 3;
	mph_main(ctx, NULL);
}
