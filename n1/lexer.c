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
#define MPH_TYPE_mph_token_Token 1
#define MPH_TYPE_mph_Lexer 2

// Struct Definitions (Env & Types)
typedef struct mph_token_Token mph_token_Token;
struct mph_token_Token {
	mph_int token_type;
	MorphString* literal;
	mph_int line;
	mph_int column;
};

typedef struct mph_Lexer mph_Lexer;
struct mph_Lexer {
	MorphString* input;
	mph_int position;
	mph_int read_position;
	mph_int ch;
	mph_int line;
	mph_int column;
};

MorphTypeInfo mph_ti_mph_token_Token = { "Token", sizeof(mph_token_Token), 1, (size_t[]){offsetof(mph_token_Token, literal)} };
MorphTypeInfo mph_ti_mph_Lexer = { "Lexer", sizeof(mph_Lexer), 1, (size_t[]){offsetof(mph_Lexer, input)} };

// Global Variables
mph_int mph_token_TOKEN_ILLEGAL;
mph_int mph_token_TOKEN_EOF;
mph_int mph_token_TOKEN_IDENT;
mph_int mph_token_TOKEN_INT;
mph_int mph_token_TOKEN_FLOAT;
mph_int mph_token_TOKEN_STRING;
mph_int mph_token_TOKEN_CHAR;
mph_int mph_token_TOKEN_ASSIGN;
mph_int mph_token_TOKEN_PLUS;
mph_int mph_token_TOKEN_MINUS;
mph_int mph_token_TOKEN_BANG;
mph_int mph_token_TOKEN_ASTERISK;
mph_int mph_token_TOKEN_SLASH;
mph_int mph_token_TOKEN_PERCENT;
mph_int mph_token_TOKEN_LT;
mph_int mph_token_TOKEN_GT;
mph_int mph_token_TOKEN_EQ;
mph_int mph_token_TOKEN_NOT_EQ;
mph_int mph_token_TOKEN_LE;
mph_int mph_token_TOKEN_GE;
mph_int mph_token_TOKEN_BIT_AND;
mph_int mph_token_TOKEN_BIT_OR;
mph_int mph_token_TOKEN_BIT_XOR;
mph_int mph_token_TOKEN_BIT_NOT;
mph_int mph_token_TOKEN_LSHIFT;
mph_int mph_token_TOKEN_RSHIFT;
mph_int mph_token_TOKEN_AND;
mph_int mph_token_TOKEN_OR;
mph_int mph_token_TOKEN_COMMA;
mph_int mph_token_TOKEN_SEMICOLON;
mph_int mph_token_TOKEN_COLON;
mph_int mph_token_TOKEN_LPAREN;
mph_int mph_token_TOKEN_RPAREN;
mph_int mph_token_TOKEN_LBRACE;
mph_int mph_token_TOKEN_RBRACE;
mph_int mph_token_TOKEN_LBRACKET;
mph_int mph_token_TOKEN_RBRACKET;
mph_int mph_token_TOKEN_FUNGSI;
mph_int mph_token_TOKEN_VAR;
mph_int mph_token_TOKEN_TETAPAN;
mph_int mph_token_TOKEN_BENAR;
mph_int mph_token_TOKEN_SALAH;
mph_int mph_token_TOKEN_JIKA;
mph_int mph_token_TOKEN_LAINNYA;
mph_int mph_token_TOKEN_KEMBALIKAN;
mph_int mph_token_TOKEN_STRUKTUR;
mph_int mph_token_TOKEN_ANTARMUKA;
mph_int mph_token_TOKEN_AMBIL;
mph_int mph_token_TOKEN_UNTUK;
mph_int mph_token_TOKEN_SELAMA;
mph_int mph_token_TOKEN_PUTUS;
mph_int mph_token_TOKEN_LANJUT;
mph_int mph_token_TOKEN_PILIH;
mph_int mph_token_TOKEN_KASUS;
mph_int mph_token_TOKEN_BAWAAN;
mph_int mph_token_TOKEN_AKHIR;
mph_int mph_token_TOKEN_ATAU_JIKA;
mph_int mph_token_TOKEN_DAN;
mph_int mph_token_TOKEN_ATAU;
mph_int mph_token_TOKEN_KOSONG;
mph_int mph_token_TOKEN_LUNCURKAN;
mph_int mph_token_TOKEN_SALURAN_BARU;
mph_int mph_token_TOKEN_KIRIM;
mph_int mph_token_TOKEN_TERIMA;
mph_int mph_token_TOKEN_DOT;
mph_int mph_token_TOKEN_ARROW;

// Function Prototypes
mph_Lexer* mph_new_lexer(MorphContext* ctx, void* _env_void, MorphString* input);
void mph_lexer_read_char(MorphContext* ctx, void* _env_void, mph_Lexer* l);
mph_int mph_char_to_ascii(MorphContext* ctx, void* _env_void, MorphString* char_str);
mph_bool mph_is_letter(MorphContext* ctx, void* _env_void, mph_int ch);
mph_bool mph_is_digit(MorphContext* ctx, void* _env_void, mph_int ch);
mph_bool mph_is_whitespace(MorphContext* ctx, void* _env_void, mph_int ch);
mph_int mph_lexer_peek_char(MorphContext* ctx, void* _env_void, mph_Lexer* l);
void mph_lexer_skip_whitespace(MorphContext* ctx, void* _env_void, mph_Lexer* l);
MorphString* mph_lexer_read_identifier(MorphContext* ctx, void* _env_void, mph_Lexer* l);
MorphString* mph_lexer_read_number(MorphContext* ctx, void* _env_void, mph_Lexer* l);
MorphString* mph_lexer_read_string(MorphContext* ctx, void* _env_void, mph_Lexer* l);
mph_token_Token* mph_lexer_next_token(MorphContext* ctx, void* _env_void, mph_Lexer* l);
mph_token_Token* mph_token_MakeToken(MorphContext* ctx, void* _env_void, mph_int t_type, MorphString* lit, mph_int ln, mph_int col);
MorphString* mph_token_TokenTypeString(MorphContext* ctx, void* _env_void, mph_int t_type);
mph_int mph_token_LookupKeyword(MorphContext* ctx, void* _env_void, MorphString* ident);

// Function Definitions
mph_token_Token* mph_token_MakeToken(MorphContext* ctx, void* _env_void, mph_int t_type, MorphString* lit, mph_int ln, mph_int col) {
	mph_gc_push_root(ctx, (void**)&lit);
	return ({ mph_token_Token* _t = (mph_token_Token*)mph_alloc(ctx, sizeof(mph_token_Token), &mph_ti_mph_token_Token); mph_gc_push_root(ctx, (void**)&_t); _t->token_type = t_type; _t->literal = lit; _t->line = ln; _t->column = col; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

MorphString* mph_token_TokenTypeString(MorphContext* ctx, void* _env_void, mph_int t_type) {
	if ((t_type == mph_token_TOKEN_ILLEGAL)) {
{
	return mph_string_new(ctx, "ILLEGAL");
}
	}
	if ((t_type == mph_token_TOKEN_EOF)) {
{
	return mph_string_new(ctx, "EOF");
}
	}
	if ((t_type == mph_token_TOKEN_IDENT)) {
{
	return mph_string_new(ctx, "IDENT");
}
	}
	if ((t_type == mph_token_TOKEN_INT)) {
{
	return mph_string_new(ctx, "INT");
}
	}
	if ((t_type == mph_token_TOKEN_FLOAT)) {
{
	return mph_string_new(ctx, "FLOAT");
}
	}
	if ((t_type == mph_token_TOKEN_STRING)) {
{
	return mph_string_new(ctx, "STRING");
}
	}
	if ((t_type == mph_token_TOKEN_CHAR)) {
{
	return mph_string_new(ctx, "CHAR");
}
	}
	if ((t_type == mph_token_TOKEN_ASSIGN)) {
{
	return mph_string_new(ctx, "=");
}
	}
	if ((t_type == mph_token_TOKEN_PLUS)) {
{
	return mph_string_new(ctx, "+");
}
	}
	if ((t_type == mph_token_TOKEN_MINUS)) {
{
	return mph_string_new(ctx, "-");
}
	}
	if ((t_type == mph_token_TOKEN_BANG)) {
{
	return mph_string_new(ctx, "!");
}
	}
	if ((t_type == mph_token_TOKEN_ASTERISK)) {
{
	return mph_string_new(ctx, "*");
}
	}
	if ((t_type == mph_token_TOKEN_SLASH)) {
{
	return mph_string_new(ctx, "/");
}
	}
	if ((t_type == mph_token_TOKEN_PERCENT)) {
{
	return mph_string_new(ctx, "%");
}
	}
	if ((t_type == mph_token_TOKEN_LT)) {
{
	return mph_string_new(ctx, "<");
}
	}
	if ((t_type == mph_token_TOKEN_GT)) {
{
	return mph_string_new(ctx, ">");
}
	}
	if ((t_type == mph_token_TOKEN_EQ)) {
{
	return mph_string_new(ctx, "==");
}
	}
	if ((t_type == mph_token_TOKEN_NOT_EQ)) {
{
	return mph_string_new(ctx, "!=");
}
	}
	if ((t_type == mph_token_TOKEN_LE)) {
{
	return mph_string_new(ctx, "<=");
}
	}
	if ((t_type == mph_token_TOKEN_GE)) {
{
	return mph_string_new(ctx, ">=");
}
	}
	if ((t_type == mph_token_TOKEN_BIT_AND)) {
{
	return mph_string_new(ctx, "&");
}
	}
	if ((t_type == mph_token_TOKEN_BIT_OR)) {
{
	return mph_string_new(ctx, "|");
}
	}
	if ((t_type == mph_token_TOKEN_BIT_XOR)) {
{
	return mph_string_new(ctx, "^");
}
	}
	if ((t_type == mph_token_TOKEN_BIT_NOT)) {
{
	return mph_string_new(ctx, "~");
}
	}
	if ((t_type == mph_token_TOKEN_LSHIFT)) {
{
	return mph_string_new(ctx, "<<");
}
	}
	if ((t_type == mph_token_TOKEN_RSHIFT)) {
{
	return mph_string_new(ctx, ">>");
}
	}
	if ((t_type == mph_token_TOKEN_AND)) {
{
	return mph_string_new(ctx, "&&");
}
	}
	if ((t_type == mph_token_TOKEN_OR)) {
{
	return mph_string_new(ctx, "||");
}
	}
	if ((t_type == mph_token_TOKEN_COMMA)) {
{
	return mph_string_new(ctx, ",");
}
	}
	if ((t_type == mph_token_TOKEN_SEMICOLON)) {
{
	return mph_string_new(ctx, ";");
}
	}
	if ((t_type == mph_token_TOKEN_COLON)) {
{
	return mph_string_new(ctx, ":");
}
	}
	if ((t_type == mph_token_TOKEN_LPAREN)) {
{
	return mph_string_new(ctx, "(");
}
	}
	if ((t_type == mph_token_TOKEN_RPAREN)) {
{
	return mph_string_new(ctx, ")");
}
	}
	if ((t_type == mph_token_TOKEN_LBRACE)) {
{
	return mph_string_new(ctx, "{");
}
	}
	if ((t_type == mph_token_TOKEN_RBRACE)) {
{
	return mph_string_new(ctx, "}");
}
	}
	if ((t_type == mph_token_TOKEN_LBRACKET)) {
{
	return mph_string_new(ctx, "[");
}
	}
	if ((t_type == mph_token_TOKEN_RBRACKET)) {
{
	return mph_string_new(ctx, "]");
}
	}
	if ((t_type == mph_token_TOKEN_FUNGSI)) {
{
	return mph_string_new(ctx, "fungsi");
}
	}
	if ((t_type == mph_token_TOKEN_VAR)) {
{
	return mph_string_new(ctx, "var");
}
	}
	if ((t_type == mph_token_TOKEN_TETAPAN)) {
{
	return mph_string_new(ctx, "tetapan");
}
	}
	if ((t_type == mph_token_TOKEN_BENAR)) {
{
	return mph_string_new(ctx, "benar");
}
	}
	if ((t_type == mph_token_TOKEN_SALAH)) {
{
	return mph_string_new(ctx, "salah");
}
	}
	if ((t_type == mph_token_TOKEN_JIKA)) {
{
	return mph_string_new(ctx, "jika");
}
	}
	if ((t_type == mph_token_TOKEN_LAINNYA)) {
{
	return mph_string_new(ctx, "lainnya");
}
	}
	if ((t_type == mph_token_TOKEN_KEMBALIKAN)) {
{
	return mph_string_new(ctx, "kembalikan");
}
	}
	if ((t_type == mph_token_TOKEN_STRUKTUR)) {
{
	return mph_string_new(ctx, "struktur");
}
	}
	if ((t_type == mph_token_TOKEN_ANTARMUKA)) {
{
	return mph_string_new(ctx, "antarmuka");
}
	}
	if ((t_type == mph_token_TOKEN_AMBIL)) {
{
	return mph_string_new(ctx, "ambil");
}
	}
	if ((t_type == mph_token_TOKEN_UNTUK)) {
{
	return mph_string_new(ctx, "untuk");
}
	}
	if ((t_type == mph_token_TOKEN_SELAMA)) {
{
	return mph_string_new(ctx, "selama");
}
	}
	if ((t_type == mph_token_TOKEN_PUTUS)) {
{
	return mph_string_new(ctx, "putus");
}
	}
	if ((t_type == mph_token_TOKEN_LANJUT)) {
{
	return mph_string_new(ctx, "lanjut");
}
	}
	if ((t_type == mph_token_TOKEN_PILIH)) {
{
	return mph_string_new(ctx, "pilih");
}
	}
	if ((t_type == mph_token_TOKEN_KASUS)) {
{
	return mph_string_new(ctx, "kasus");
}
	}
	if ((t_type == mph_token_TOKEN_BAWAAN)) {
{
	return mph_string_new(ctx, "bawaan");
}
	}
	if ((t_type == mph_token_TOKEN_AKHIR)) {
{
	return mph_string_new(ctx, "akhir");
}
	}
	if ((t_type == mph_token_TOKEN_ATAU_JIKA)) {
{
	return mph_string_new(ctx, "atau_jika");
}
	}
	if ((t_type == mph_token_TOKEN_DAN)) {
{
	return mph_string_new(ctx, "dan");
}
	}
	if ((t_type == mph_token_TOKEN_ATAU)) {
{
	return mph_string_new(ctx, "atau");
}
	}
	if ((t_type == mph_token_TOKEN_KOSONG)) {
{
	return mph_string_new(ctx, "kosong");
}
	}
	if ((t_type == mph_token_TOKEN_LUNCURKAN)) {
{
	return mph_string_new(ctx, "luncurkan");
}
	}
	if ((t_type == mph_token_TOKEN_SALURAN_BARU)) {
{
	return mph_string_new(ctx, "saluran_baru");
}
	}
	if ((t_type == mph_token_TOKEN_KIRIM)) {
{
	return mph_string_new(ctx, "kirim");
}
	}
	if ((t_type == mph_token_TOKEN_TERIMA)) {
{
	return mph_string_new(ctx, "terima");
}
	}
	if ((t_type == mph_token_TOKEN_DOT)) {
{
	return mph_string_new(ctx, ".");
}
	}
	if ((t_type == mph_token_TOKEN_ARROW)) {
{
	return mph_string_new(ctx, "->");
}
	}
	return mph_string_new(ctx, "UNKNOWN");
}

mph_int mph_token_LookupKeyword(MorphContext* ctx, void* _env_void, MorphString* ident) {
	mph_gc_push_root(ctx, (void**)&ident);
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "fungsi"))) {
{
	return mph_token_TOKEN_FUNGSI;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "var"))) {
{
	return mph_token_TOKEN_VAR;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "tetapan"))) {
{
	return mph_token_TOKEN_TETAPAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "benar"))) {
{
	return mph_token_TOKEN_BENAR;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "salah"))) {
{
	return mph_token_TOKEN_SALAH;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "jika"))) {
{
	return mph_token_TOKEN_JIKA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "lainnya"))) {
{
	return mph_token_TOKEN_LAINNYA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "kembalikan"))) {
{
	return mph_token_TOKEN_KEMBALIKAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "struktur"))) {
{
	return mph_token_TOKEN_STRUKTUR;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "antarmuka"))) {
{
	return mph_token_TOKEN_ANTARMUKA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "ambil"))) {
{
	return mph_token_TOKEN_AMBIL;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "untuk"))) {
{
	return mph_token_TOKEN_UNTUK;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "selama"))) {
{
	return mph_token_TOKEN_SELAMA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "putus"))) {
{
	return mph_token_TOKEN_PUTUS;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "lanjut"))) {
{
	return mph_token_TOKEN_LANJUT;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "pilih"))) {
{
	return mph_token_TOKEN_PILIH;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "kasus"))) {
{
	return mph_token_TOKEN_KASUS;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "bawaan"))) {
{
	return mph_token_TOKEN_BAWAAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "akhir"))) {
{
	return mph_token_TOKEN_AKHIR;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "atau_jika"))) {
{
	return mph_token_TOKEN_ATAU_JIKA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "dan"))) {
{
	return mph_token_TOKEN_DAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "atau"))) {
{
	return mph_token_TOKEN_ATAU;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "kosong"))) {
{
	return mph_token_TOKEN_KOSONG;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "luncurkan"))) {
{
	return mph_token_TOKEN_LUNCURKAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "saluran_baru"))) {
{
	return mph_token_TOKEN_SALURAN_BARU;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "kirim"))) {
{
	return mph_token_TOKEN_KIRIM;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "terima"))) {
{
	return mph_token_TOKEN_TERIMA;
}
	}
	return mph_token_TOKEN_IDENT;
	mph_gc_pop_roots(ctx, 1);
}

mph_Lexer* mph_new_lexer(MorphContext* ctx, void* _env_void, MorphString* input) {
	mph_gc_push_root(ctx, (void**)&input);
	mph_Lexer* l = ({ mph_Lexer* _t = (mph_Lexer*)mph_alloc(ctx, sizeof(mph_Lexer), &mph_ti_mph_Lexer); mph_gc_push_root(ctx, (void**)&_t); _t->input = input; _t->position = 0; _t->read_position = 0; _t->ch = 0; _t->line = 1; _t->column = 0; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_push_root(ctx, (void**)&l);
	({ mph_Lexer* _arg_1 = l; mph_gc_push_root(ctx, (void**)&_arg_1); mph_lexer_read_char(ctx, NULL, _arg_1); mph_gc_pop_roots(ctx, 1); });
	return l;
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}

void mph_lexer_read_char(MorphContext* ctx, void* _env_void, mph_Lexer* l) {
	mph_gc_push_root(ctx, (void**)&l);
	if ((({ mph_Lexer* _obj_2 = l; mph_gc_push_root(ctx, (void**)&_obj_2); mph_int _ret_3 = _obj_2->read_position; mph_gc_pop_roots(ctx, 1); _ret_3; }) >= ({ MorphString* _len_arg_6 = ({ mph_Lexer* _obj_4 = l; mph_gc_push_root(ctx, (void**)&_obj_4); MorphString* _ret_5 = _obj_4->input; mph_gc_pop_roots(ctx, 1); _ret_5; }); mph_gc_push_root(ctx, (void**)&_len_arg_6); mph_int _ret_7 = ((MorphString*)_len_arg_6)->length; mph_gc_pop_roots(ctx, 1); _ret_7; }))) {
{
	({ mph_Lexer* _obj_8 = l; mph_gc_push_root(ctx, (void**)&_obj_8); _obj_8->ch = 0; mph_gc_pop_roots(ctx, 1); });
}
	} else {
{
	MorphString* char_str = mph_string_substring(ctx, ({ mph_Lexer* _obj_9 = l; mph_gc_push_root(ctx, (void**)&_obj_9); MorphString* _ret_10 = _obj_9->input; mph_gc_pop_roots(ctx, 1); _ret_10; }), ({ mph_Lexer* _obj_11 = l; mph_gc_push_root(ctx, (void**)&_obj_11); mph_int _ret_12 = _obj_11->read_position; mph_gc_pop_roots(ctx, 1); _ret_12; }), (({ mph_Lexer* _obj_13 = l; mph_gc_push_root(ctx, (void**)&_obj_13); mph_int _ret_14 = _obj_13->read_position; mph_gc_pop_roots(ctx, 1); _ret_14; }) + 1));
	mph_gc_push_root(ctx, (void**)&char_str);
	({ mph_Lexer* _obj_17 = l; mph_gc_push_root(ctx, (void**)&_obj_17); _obj_17->ch = ({ MorphString* _arg_15 = char_str; mph_gc_push_root(ctx, (void**)&_arg_15); mph_int _ret_16 = mph_char_to_ascii(ctx, NULL, _arg_15); mph_gc_pop_roots(ctx, 1); _ret_16; }); mph_gc_pop_roots(ctx, 1); });
	mph_gc_pop_roots(ctx, 1);
}
	}
	({ mph_Lexer* _obj_20 = l; mph_gc_push_root(ctx, (void**)&_obj_20); _obj_20->position = ({ mph_Lexer* _obj_18 = l; mph_gc_push_root(ctx, (void**)&_obj_18); mph_int _ret_19 = _obj_18->read_position; mph_gc_pop_roots(ctx, 1); _ret_19; }); mph_gc_pop_roots(ctx, 1); });
	({ mph_Lexer* _obj_23 = l; mph_gc_push_root(ctx, (void**)&_obj_23); _obj_23->read_position = (({ mph_Lexer* _obj_21 = l; mph_gc_push_root(ctx, (void**)&_obj_21); mph_int _ret_22 = _obj_21->read_position; mph_gc_pop_roots(ctx, 1); _ret_22; }) + 1); mph_gc_pop_roots(ctx, 1); });
	if ((({ mph_Lexer* _obj_24 = l; mph_gc_push_root(ctx, (void**)&_obj_24); mph_int _ret_25 = _obj_24->ch; mph_gc_pop_roots(ctx, 1); _ret_25; }) == 10)) {
{
	({ mph_Lexer* _obj_28 = l; mph_gc_push_root(ctx, (void**)&_obj_28); _obj_28->line = (({ mph_Lexer* _obj_26 = l; mph_gc_push_root(ctx, (void**)&_obj_26); mph_int _ret_27 = _obj_26->line; mph_gc_pop_roots(ctx, 1); _ret_27; }) + 1); mph_gc_pop_roots(ctx, 1); });
	({ mph_Lexer* _obj_29 = l; mph_gc_push_root(ctx, (void**)&_obj_29); _obj_29->column = 0; mph_gc_pop_roots(ctx, 1); });
}
	} else {
{
	({ mph_Lexer* _obj_32 = l; mph_gc_push_root(ctx, (void**)&_obj_32); _obj_32->column = (({ mph_Lexer* _obj_30 = l; mph_gc_push_root(ctx, (void**)&_obj_30); mph_int _ret_31 = _obj_30->column; mph_gc_pop_roots(ctx, 1); _ret_31; }) + 1); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_gc_pop_roots(ctx, 1);
}

mph_int mph_char_to_ascii(MorphContext* ctx, void* _env_void, MorphString* char_str) {
	mph_gc_push_root(ctx, (void**)&char_str);
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, " "))) {
{
	return 32;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "!"))) {
{
	return 33;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "\""))) {
{
	return 34;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "#"))) {
{
	return 35;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "$"))) {
{
	return 36;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "%"))) {
{
	return 37;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "&"))) {
{
	return 38;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "'"))) {
{
	return 39;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "("))) {
{
	return 40;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, ")"))) {
{
	return 41;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "*"))) {
{
	return 42;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "+"))) {
{
	return 43;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, ","))) {
{
	return 44;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "-"))) {
{
	return 45;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "."))) {
{
	return 46;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "/"))) {
{
	return 47;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "0"))) {
{
	return 48;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "1"))) {
{
	return 49;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "2"))) {
{
	return 50;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "3"))) {
{
	return 51;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "4"))) {
{
	return 52;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "5"))) {
{
	return 53;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "6"))) {
{
	return 54;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "7"))) {
{
	return 55;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "8"))) {
{
	return 56;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "9"))) {
{
	return 57;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, ":"))) {
{
	return 58;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, ";"))) {
{
	return 59;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "<"))) {
{
	return 60;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "="))) {
{
	return 61;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, ">"))) {
{
	return 62;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "?"))) {
{
	return 63;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "@"))) {
{
	return 64;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "A"))) {
{
	return 65;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "B"))) {
{
	return 66;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "C"))) {
{
	return 67;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "D"))) {
{
	return 68;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "E"))) {
{
	return 69;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "F"))) {
{
	return 70;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "G"))) {
{
	return 71;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "H"))) {
{
	return 72;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "I"))) {
{
	return 73;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "J"))) {
{
	return 74;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "K"))) {
{
	return 75;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "L"))) {
{
	return 76;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "M"))) {
{
	return 77;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "N"))) {
{
	return 78;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "O"))) {
{
	return 79;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "P"))) {
{
	return 80;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "Q"))) {
{
	return 81;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "R"))) {
{
	return 82;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "S"))) {
{
	return 83;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "T"))) {
{
	return 84;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "U"))) {
{
	return 85;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "V"))) {
{
	return 86;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "W"))) {
{
	return 87;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "X"))) {
{
	return 88;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "Y"))) {
{
	return 89;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "Z"))) {
{
	return 90;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "["))) {
{
	return 91;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "\\"))) {
{
	return 92;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "]"))) {
{
	return 93;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "^"))) {
{
	return 94;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "_"))) {
{
	return 95;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "`"))) {
{
	return 96;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "a"))) {
{
	return 97;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "b"))) {
{
	return 98;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "c"))) {
{
	return 99;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "d"))) {
{
	return 100;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "e"))) {
{
	return 101;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "f"))) {
{
	return 102;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "g"))) {
{
	return 103;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "h"))) {
{
	return 104;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "i"))) {
{
	return 105;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "j"))) {
{
	return 106;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "k"))) {
{
	return 107;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "l"))) {
{
	return 108;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "m"))) {
{
	return 109;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "n"))) {
{
	return 110;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "o"))) {
{
	return 111;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "p"))) {
{
	return 112;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "q"))) {
{
	return 113;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "r"))) {
{
	return 114;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "s"))) {
{
	return 115;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "t"))) {
{
	return 116;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "u"))) {
{
	return 117;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "v"))) {
{
	return 118;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "w"))) {
{
	return 119;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "x"))) {
{
	return 120;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "y"))) {
{
	return 121;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "z"))) {
{
	return 122;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "{"))) {
{
	return 123;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "|"))) {
{
	return 124;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "}"))) {
{
	return 125;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "~"))) {
{
	return 126;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "\n"))) {
{
	return 10;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "\t"))) {
{
	return 9;
}
	}
	if (mph_string_eq(ctx, char_str, mph_string_new(ctx, "\r"))) {
{
	return 13;
}
	}
	return 0;
	mph_gc_pop_roots(ctx, 1);
}

mph_bool mph_is_letter(MorphContext* ctx, void* _env_void, mph_int ch) {
	if ((ch >= 97)) {
{
	if ((ch <= 122)) {
{
	return 1;
}
	}
}
	}
	if ((ch >= 65)) {
{
	if ((ch <= 90)) {
{
	return 1;
}
	}
}
	}
	if ((ch == 95)) {
{
	return 1;
}
	}
	return 0;
}

mph_bool mph_is_digit(MorphContext* ctx, void* _env_void, mph_int ch) {
	if ((ch >= 48)) {
{
	if ((ch <= 57)) {
{
	return 1;
}
	}
}
	}
	return 0;
}

mph_bool mph_is_whitespace(MorphContext* ctx, void* _env_void, mph_int ch) {
	if ((ch == 32)) {
{
	return 1;
}
	}
	if ((ch == 9)) {
{
	return 1;
}
	}
	if ((ch == 10)) {
{
	return 1;
}
	}
	if ((ch == 13)) {
{
	return 1;
}
	}
	return 0;
}

mph_int mph_lexer_peek_char(MorphContext* ctx, void* _env_void, mph_Lexer* l) {
	mph_gc_push_root(ctx, (void**)&l);
	if ((({ mph_Lexer* _obj_33 = l; mph_gc_push_root(ctx, (void**)&_obj_33); mph_int _ret_34 = _obj_33->read_position; mph_gc_pop_roots(ctx, 1); _ret_34; }) >= ({ MorphString* _len_arg_37 = ({ mph_Lexer* _obj_35 = l; mph_gc_push_root(ctx, (void**)&_obj_35); MorphString* _ret_36 = _obj_35->input; mph_gc_pop_roots(ctx, 1); _ret_36; }); mph_gc_push_root(ctx, (void**)&_len_arg_37); mph_int _ret_38 = ((MorphString*)_len_arg_37)->length; mph_gc_pop_roots(ctx, 1); _ret_38; }))) {
{
	return 0;
}
	} else {
{
	MorphString* char_str = mph_string_substring(ctx, ({ mph_Lexer* _obj_39 = l; mph_gc_push_root(ctx, (void**)&_obj_39); MorphString* _ret_40 = _obj_39->input; mph_gc_pop_roots(ctx, 1); _ret_40; }), ({ mph_Lexer* _obj_41 = l; mph_gc_push_root(ctx, (void**)&_obj_41); mph_int _ret_42 = _obj_41->read_position; mph_gc_pop_roots(ctx, 1); _ret_42; }), (({ mph_Lexer* _obj_43 = l; mph_gc_push_root(ctx, (void**)&_obj_43); mph_int _ret_44 = _obj_43->read_position; mph_gc_pop_roots(ctx, 1); _ret_44; }) + 1));
	mph_gc_push_root(ctx, (void**)&char_str);
	return ({ MorphString* _arg_45 = char_str; mph_gc_push_root(ctx, (void**)&_arg_45); mph_int _ret_46 = mph_char_to_ascii(ctx, NULL, _arg_45); mph_gc_pop_roots(ctx, 1); _ret_46; });
	mph_gc_pop_roots(ctx, 1);
}
	}
	mph_gc_pop_roots(ctx, 1);
}

void mph_lexer_skip_whitespace(MorphContext* ctx, void* _env_void, mph_Lexer* l) {
	mph_gc_push_root(ctx, (void**)&l);
	while (mph_is_whitespace(ctx, NULL, ({ mph_Lexer* _obj_47 = l; mph_gc_push_root(ctx, (void**)&_obj_47); mph_int _ret_48 = _obj_47->ch; mph_gc_pop_roots(ctx, 1); _ret_48; }))) {
{
	({ mph_Lexer* _arg_49 = l; mph_gc_push_root(ctx, (void**)&_arg_49); mph_lexer_read_char(ctx, NULL, _arg_49); mph_gc_pop_roots(ctx, 1); });
}
	}
	mph_gc_pop_roots(ctx, 1);
}

MorphString* mph_lexer_read_identifier(MorphContext* ctx, void* _env_void, mph_Lexer* l) {
	mph_gc_push_root(ctx, (void**)&l);
	mph_int position = ({ mph_Lexer* _obj_50 = l; mph_gc_push_root(ctx, (void**)&_obj_50); mph_int _ret_51 = _obj_50->position; mph_gc_pop_roots(ctx, 1); _ret_51; });
	mph_bool done = 0;
	while ((done == 0)) {
{
	if (mph_is_letter(ctx, NULL, ({ mph_Lexer* _obj_52 = l; mph_gc_push_root(ctx, (void**)&_obj_52); mph_int _ret_53 = _obj_52->ch; mph_gc_pop_roots(ctx, 1); _ret_53; }))) {
{
	({ mph_Lexer* _arg_54 = l; mph_gc_push_root(ctx, (void**)&_arg_54); mph_lexer_read_char(ctx, NULL, _arg_54); mph_gc_pop_roots(ctx, 1); });
}
	} else {
{
	if (mph_is_digit(ctx, NULL, ({ mph_Lexer* _obj_55 = l; mph_gc_push_root(ctx, (void**)&_obj_55); mph_int _ret_56 = _obj_55->ch; mph_gc_pop_roots(ctx, 1); _ret_56; }))) {
{
	({ mph_Lexer* _arg_57 = l; mph_gc_push_root(ctx, (void**)&_arg_57); mph_lexer_read_char(ctx, NULL, _arg_57); mph_gc_pop_roots(ctx, 1); });
}
	} else {
{
	done = 1;
}
	}
}
	}
}
	}
	return mph_string_substring(ctx, ({ mph_Lexer* _obj_58 = l; mph_gc_push_root(ctx, (void**)&_obj_58); MorphString* _ret_59 = _obj_58->input; mph_gc_pop_roots(ctx, 1); _ret_59; }), position, ({ mph_Lexer* _obj_60 = l; mph_gc_push_root(ctx, (void**)&_obj_60); mph_int _ret_61 = _obj_60->position; mph_gc_pop_roots(ctx, 1); _ret_61; }));
	mph_gc_pop_roots(ctx, 1);
}

MorphString* mph_lexer_read_number(MorphContext* ctx, void* _env_void, mph_Lexer* l) {
	mph_gc_push_root(ctx, (void**)&l);
	mph_int position = ({ mph_Lexer* _obj_62 = l; mph_gc_push_root(ctx, (void**)&_obj_62); mph_int _ret_63 = _obj_62->position; mph_gc_pop_roots(ctx, 1); _ret_63; });
	while (mph_is_digit(ctx, NULL, ({ mph_Lexer* _obj_64 = l; mph_gc_push_root(ctx, (void**)&_obj_64); mph_int _ret_65 = _obj_64->ch; mph_gc_pop_roots(ctx, 1); _ret_65; }))) {
{
	({ mph_Lexer* _arg_66 = l; mph_gc_push_root(ctx, (void**)&_arg_66); mph_lexer_read_char(ctx, NULL, _arg_66); mph_gc_pop_roots(ctx, 1); });
}
	}
	return mph_string_substring(ctx, ({ mph_Lexer* _obj_67 = l; mph_gc_push_root(ctx, (void**)&_obj_67); MorphString* _ret_68 = _obj_67->input; mph_gc_pop_roots(ctx, 1); _ret_68; }), position, ({ mph_Lexer* _obj_69 = l; mph_gc_push_root(ctx, (void**)&_obj_69); mph_int _ret_70 = _obj_69->position; mph_gc_pop_roots(ctx, 1); _ret_70; }));
	mph_gc_pop_roots(ctx, 1);
}

MorphString* mph_lexer_read_string(MorphContext* ctx, void* _env_void, mph_Lexer* l) {
	mph_gc_push_root(ctx, (void**)&l);
	mph_int position = (({ mph_Lexer* _obj_71 = l; mph_gc_push_root(ctx, (void**)&_obj_71); mph_int _ret_72 = _obj_71->position; mph_gc_pop_roots(ctx, 1); _ret_72; }) + 1);
	({ mph_Lexer* _arg_73 = l; mph_gc_push_root(ctx, (void**)&_arg_73); mph_lexer_read_char(ctx, NULL, _arg_73); mph_gc_pop_roots(ctx, 1); });
	mph_bool should_continue = 1;
	while (should_continue) {
{
	if ((({ mph_Lexer* _obj_74 = l; mph_gc_push_root(ctx, (void**)&_obj_74); mph_int _ret_75 = _obj_74->ch; mph_gc_pop_roots(ctx, 1); _ret_75; }) == 34)) {
{
	should_continue = 0;
}
	} else {
{
	if ((({ mph_Lexer* _obj_76 = l; mph_gc_push_root(ctx, (void**)&_obj_76); mph_int _ret_77 = _obj_76->ch; mph_gc_pop_roots(ctx, 1); _ret_77; }) == 0)) {
{
	should_continue = 0;
}
	} else {
{
	({ mph_Lexer* _arg_78 = l; mph_gc_push_root(ctx, (void**)&_arg_78); mph_lexer_read_char(ctx, NULL, _arg_78); mph_gc_pop_roots(ctx, 1); });
}
	}
}
	}
}
	}
	return mph_string_substring(ctx, ({ mph_Lexer* _obj_79 = l; mph_gc_push_root(ctx, (void**)&_obj_79); MorphString* _ret_80 = _obj_79->input; mph_gc_pop_roots(ctx, 1); _ret_80; }), position, ({ mph_Lexer* _obj_81 = l; mph_gc_push_root(ctx, (void**)&_obj_81); mph_int _ret_82 = _obj_81->position; mph_gc_pop_roots(ctx, 1); _ret_82; }));
	mph_gc_pop_roots(ctx, 1);
}

mph_token_Token* mph_lexer_next_token(MorphContext* ctx, void* _env_void, mph_Lexer* l) {
	mph_gc_push_root(ctx, (void**)&l);
	({ mph_Lexer* _arg_83 = l; mph_gc_push_root(ctx, (void**)&_arg_83); mph_lexer_skip_whitespace(ctx, NULL, _arg_83); mph_gc_pop_roots(ctx, 1); });
	mph_token_Token* tok = ({ MorphString* _arg_84 = mph_string_new(ctx, "-"); mph_gc_push_root(ctx, (void**)&_arg_84); mph_token_Token* _ret_89 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_ILLEGAL, _arg_84, ({ mph_Lexer* _obj_85 = l; mph_gc_push_root(ctx, (void**)&_obj_85); mph_int _ret_86 = _obj_85->line; mph_gc_pop_roots(ctx, 1); _ret_86; }), ({ mph_Lexer* _obj_87 = l; mph_gc_push_root(ctx, (void**)&_obj_87); mph_int _ret_88 = _obj_87->column; mph_gc_pop_roots(ctx, 1); _ret_88; })); mph_gc_pop_roots(ctx, 1); _ret_89; });
	mph_gc_push_root(ctx, (void**)&tok);
	if ((({ mph_Lexer* _obj_90 = l; mph_gc_push_root(ctx, (void**)&_obj_90); mph_int _ret_91 = _obj_90->ch; mph_gc_pop_roots(ctx, 1); _ret_91; }) == 61)) {
{
	if ((({ mph_Lexer* _arg_92 = l; mph_gc_push_root(ctx, (void**)&_arg_92); mph_int _ret_93 = mph_lexer_peek_char(ctx, NULL, _arg_92); mph_gc_pop_roots(ctx, 1); _ret_93; }) == 61)) {
{
	mph_int ch = ({ mph_Lexer* _obj_94 = l; mph_gc_push_root(ctx, (void**)&_obj_94); mph_int _ret_95 = _obj_94->ch; mph_gc_pop_roots(ctx, 1); _ret_95; });
	({ mph_Lexer* _arg_96 = l; mph_gc_push_root(ctx, (void**)&_arg_96); mph_lexer_read_char(ctx, NULL, _arg_96); mph_gc_pop_roots(ctx, 1); });
	tok = ({ MorphString* _arg_97 = mph_string_new(ctx, "=="); mph_gc_push_root(ctx, (void**)&_arg_97); mph_token_Token* _ret_102 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_EQ, _arg_97, ({ mph_Lexer* _obj_98 = l; mph_gc_push_root(ctx, (void**)&_obj_98); mph_int _ret_99 = _obj_98->line; mph_gc_pop_roots(ctx, 1); _ret_99; }), ({ mph_Lexer* _obj_100 = l; mph_gc_push_root(ctx, (void**)&_obj_100); mph_int _ret_101 = _obj_100->column; mph_gc_pop_roots(ctx, 1); _ret_101; })); mph_gc_pop_roots(ctx, 1); _ret_102; });
}
	} else {
{
	tok = ({ MorphString* _arg_103 = mph_string_new(ctx, "="); mph_gc_push_root(ctx, (void**)&_arg_103); mph_token_Token* _ret_108 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_ASSIGN, _arg_103, ({ mph_Lexer* _obj_104 = l; mph_gc_push_root(ctx, (void**)&_obj_104); mph_int _ret_105 = _obj_104->line; mph_gc_pop_roots(ctx, 1); _ret_105; }), ({ mph_Lexer* _obj_106 = l; mph_gc_push_root(ctx, (void**)&_obj_106); mph_int _ret_107 = _obj_106->column; mph_gc_pop_roots(ctx, 1); _ret_107; })); mph_gc_pop_roots(ctx, 1); _ret_108; });
}
	}
}
	} else if ((({ mph_Lexer* _obj_109 = l; mph_gc_push_root(ctx, (void**)&_obj_109); mph_int _ret_110 = _obj_109->ch; mph_gc_pop_roots(ctx, 1); _ret_110; }) == 43)) {
{
	tok = ({ MorphString* _arg_111 = mph_string_new(ctx, "+"); mph_gc_push_root(ctx, (void**)&_arg_111); mph_token_Token* _ret_116 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_PLUS, _arg_111, ({ mph_Lexer* _obj_112 = l; mph_gc_push_root(ctx, (void**)&_obj_112); mph_int _ret_113 = _obj_112->line; mph_gc_pop_roots(ctx, 1); _ret_113; }), ({ mph_Lexer* _obj_114 = l; mph_gc_push_root(ctx, (void**)&_obj_114); mph_int _ret_115 = _obj_114->column; mph_gc_pop_roots(ctx, 1); _ret_115; })); mph_gc_pop_roots(ctx, 1); _ret_116; });
}
	} else if ((({ mph_Lexer* _obj_117 = l; mph_gc_push_root(ctx, (void**)&_obj_117); mph_int _ret_118 = _obj_117->ch; mph_gc_pop_roots(ctx, 1); _ret_118; }) == 45)) {
{
	if ((({ mph_Lexer* _arg_119 = l; mph_gc_push_root(ctx, (void**)&_arg_119); mph_int _ret_120 = mph_lexer_peek_char(ctx, NULL, _arg_119); mph_gc_pop_roots(ctx, 1); _ret_120; }) == 62)) {
{
	({ mph_Lexer* _arg_121 = l; mph_gc_push_root(ctx, (void**)&_arg_121); mph_lexer_read_char(ctx, NULL, _arg_121); mph_gc_pop_roots(ctx, 1); });
	tok = ({ MorphString* _arg_122 = mph_string_new(ctx, "->"); mph_gc_push_root(ctx, (void**)&_arg_122); mph_token_Token* _ret_127 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_ARROW, _arg_122, ({ mph_Lexer* _obj_123 = l; mph_gc_push_root(ctx, (void**)&_obj_123); mph_int _ret_124 = _obj_123->line; mph_gc_pop_roots(ctx, 1); _ret_124; }), ({ mph_Lexer* _obj_125 = l; mph_gc_push_root(ctx, (void**)&_obj_125); mph_int _ret_126 = _obj_125->column; mph_gc_pop_roots(ctx, 1); _ret_126; })); mph_gc_pop_roots(ctx, 1); _ret_127; });
}
	} else {
{
	tok = ({ MorphString* _arg_128 = mph_string_new(ctx, "-"); mph_gc_push_root(ctx, (void**)&_arg_128); mph_token_Token* _ret_133 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_MINUS, _arg_128, ({ mph_Lexer* _obj_129 = l; mph_gc_push_root(ctx, (void**)&_obj_129); mph_int _ret_130 = _obj_129->line; mph_gc_pop_roots(ctx, 1); _ret_130; }), ({ mph_Lexer* _obj_131 = l; mph_gc_push_root(ctx, (void**)&_obj_131); mph_int _ret_132 = _obj_131->column; mph_gc_pop_roots(ctx, 1); _ret_132; })); mph_gc_pop_roots(ctx, 1); _ret_133; });
}
	}
}
	} else if ((({ mph_Lexer* _obj_134 = l; mph_gc_push_root(ctx, (void**)&_obj_134); mph_int _ret_135 = _obj_134->ch; mph_gc_pop_roots(ctx, 1); _ret_135; }) == 33)) {
{
	if ((({ mph_Lexer* _arg_136 = l; mph_gc_push_root(ctx, (void**)&_arg_136); mph_int _ret_137 = mph_lexer_peek_char(ctx, NULL, _arg_136); mph_gc_pop_roots(ctx, 1); _ret_137; }) == 61)) {
{
	({ mph_Lexer* _arg_138 = l; mph_gc_push_root(ctx, (void**)&_arg_138); mph_lexer_read_char(ctx, NULL, _arg_138); mph_gc_pop_roots(ctx, 1); });
	tok = ({ MorphString* _arg_139 = mph_string_new(ctx, "!="); mph_gc_push_root(ctx, (void**)&_arg_139); mph_token_Token* _ret_144 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_NOT_EQ, _arg_139, ({ mph_Lexer* _obj_140 = l; mph_gc_push_root(ctx, (void**)&_obj_140); mph_int _ret_141 = _obj_140->line; mph_gc_pop_roots(ctx, 1); _ret_141; }), ({ mph_Lexer* _obj_142 = l; mph_gc_push_root(ctx, (void**)&_obj_142); mph_int _ret_143 = _obj_142->column; mph_gc_pop_roots(ctx, 1); _ret_143; })); mph_gc_pop_roots(ctx, 1); _ret_144; });
}
	} else {
{
	tok = ({ MorphString* _arg_145 = mph_string_new(ctx, "!"); mph_gc_push_root(ctx, (void**)&_arg_145); mph_token_Token* _ret_150 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_BANG, _arg_145, ({ mph_Lexer* _obj_146 = l; mph_gc_push_root(ctx, (void**)&_obj_146); mph_int _ret_147 = _obj_146->line; mph_gc_pop_roots(ctx, 1); _ret_147; }), ({ mph_Lexer* _obj_148 = l; mph_gc_push_root(ctx, (void**)&_obj_148); mph_int _ret_149 = _obj_148->column; mph_gc_pop_roots(ctx, 1); _ret_149; })); mph_gc_pop_roots(ctx, 1); _ret_150; });
}
	}
}
	} else if ((({ mph_Lexer* _obj_151 = l; mph_gc_push_root(ctx, (void**)&_obj_151); mph_int _ret_152 = _obj_151->ch; mph_gc_pop_roots(ctx, 1); _ret_152; }) == 42)) {
{
	tok = ({ MorphString* _arg_153 = mph_string_new(ctx, "*"); mph_gc_push_root(ctx, (void**)&_arg_153); mph_token_Token* _ret_158 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_ASTERISK, _arg_153, ({ mph_Lexer* _obj_154 = l; mph_gc_push_root(ctx, (void**)&_obj_154); mph_int _ret_155 = _obj_154->line; mph_gc_pop_roots(ctx, 1); _ret_155; }), ({ mph_Lexer* _obj_156 = l; mph_gc_push_root(ctx, (void**)&_obj_156); mph_int _ret_157 = _obj_156->column; mph_gc_pop_roots(ctx, 1); _ret_157; })); mph_gc_pop_roots(ctx, 1); _ret_158; });
}
	} else if ((({ mph_Lexer* _obj_159 = l; mph_gc_push_root(ctx, (void**)&_obj_159); mph_int _ret_160 = _obj_159->ch; mph_gc_pop_roots(ctx, 1); _ret_160; }) == 47)) {
{
	tok = ({ MorphString* _arg_161 = mph_string_new(ctx, "/"); mph_gc_push_root(ctx, (void**)&_arg_161); mph_token_Token* _ret_166 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_SLASH, _arg_161, ({ mph_Lexer* _obj_162 = l; mph_gc_push_root(ctx, (void**)&_obj_162); mph_int _ret_163 = _obj_162->line; mph_gc_pop_roots(ctx, 1); _ret_163; }), ({ mph_Lexer* _obj_164 = l; mph_gc_push_root(ctx, (void**)&_obj_164); mph_int _ret_165 = _obj_164->column; mph_gc_pop_roots(ctx, 1); _ret_165; })); mph_gc_pop_roots(ctx, 1); _ret_166; });
}
	} else if ((({ mph_Lexer* _obj_167 = l; mph_gc_push_root(ctx, (void**)&_obj_167); mph_int _ret_168 = _obj_167->ch; mph_gc_pop_roots(ctx, 1); _ret_168; }) == 37)) {
{
	tok = ({ MorphString* _arg_169 = mph_string_new(ctx, "%"); mph_gc_push_root(ctx, (void**)&_arg_169); mph_token_Token* _ret_174 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_PERCENT, _arg_169, ({ mph_Lexer* _obj_170 = l; mph_gc_push_root(ctx, (void**)&_obj_170); mph_int _ret_171 = _obj_170->line; mph_gc_pop_roots(ctx, 1); _ret_171; }), ({ mph_Lexer* _obj_172 = l; mph_gc_push_root(ctx, (void**)&_obj_172); mph_int _ret_173 = _obj_172->column; mph_gc_pop_roots(ctx, 1); _ret_173; })); mph_gc_pop_roots(ctx, 1); _ret_174; });
}
	} else if ((({ mph_Lexer* _obj_175 = l; mph_gc_push_root(ctx, (void**)&_obj_175); mph_int _ret_176 = _obj_175->ch; mph_gc_pop_roots(ctx, 1); _ret_176; }) == 60)) {
{
	if ((({ mph_Lexer* _arg_177 = l; mph_gc_push_root(ctx, (void**)&_arg_177); mph_int _ret_178 = mph_lexer_peek_char(ctx, NULL, _arg_177); mph_gc_pop_roots(ctx, 1); _ret_178; }) == 61)) {
{
	({ mph_Lexer* _arg_179 = l; mph_gc_push_root(ctx, (void**)&_arg_179); mph_lexer_read_char(ctx, NULL, _arg_179); mph_gc_pop_roots(ctx, 1); });
	tok = ({ MorphString* _arg_180 = mph_string_new(ctx, "<="); mph_gc_push_root(ctx, (void**)&_arg_180); mph_token_Token* _ret_185 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_LE, _arg_180, ({ mph_Lexer* _obj_181 = l; mph_gc_push_root(ctx, (void**)&_obj_181); mph_int _ret_182 = _obj_181->line; mph_gc_pop_roots(ctx, 1); _ret_182; }), ({ mph_Lexer* _obj_183 = l; mph_gc_push_root(ctx, (void**)&_obj_183); mph_int _ret_184 = _obj_183->column; mph_gc_pop_roots(ctx, 1); _ret_184; })); mph_gc_pop_roots(ctx, 1); _ret_185; });
}
	} else {
{
	if ((({ mph_Lexer* _arg_186 = l; mph_gc_push_root(ctx, (void**)&_arg_186); mph_int _ret_187 = mph_lexer_peek_char(ctx, NULL, _arg_186); mph_gc_pop_roots(ctx, 1); _ret_187; }) == 60)) {
{
	({ mph_Lexer* _arg_188 = l; mph_gc_push_root(ctx, (void**)&_arg_188); mph_lexer_read_char(ctx, NULL, _arg_188); mph_gc_pop_roots(ctx, 1); });
	tok = ({ MorphString* _arg_189 = mph_string_new(ctx, "<<"); mph_gc_push_root(ctx, (void**)&_arg_189); mph_token_Token* _ret_194 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_LSHIFT, _arg_189, ({ mph_Lexer* _obj_190 = l; mph_gc_push_root(ctx, (void**)&_obj_190); mph_int _ret_191 = _obj_190->line; mph_gc_pop_roots(ctx, 1); _ret_191; }), ({ mph_Lexer* _obj_192 = l; mph_gc_push_root(ctx, (void**)&_obj_192); mph_int _ret_193 = _obj_192->column; mph_gc_pop_roots(ctx, 1); _ret_193; })); mph_gc_pop_roots(ctx, 1); _ret_194; });
}
	} else {
{
	tok = ({ MorphString* _arg_195 = mph_string_new(ctx, "<"); mph_gc_push_root(ctx, (void**)&_arg_195); mph_token_Token* _ret_200 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_LT, _arg_195, ({ mph_Lexer* _obj_196 = l; mph_gc_push_root(ctx, (void**)&_obj_196); mph_int _ret_197 = _obj_196->line; mph_gc_pop_roots(ctx, 1); _ret_197; }), ({ mph_Lexer* _obj_198 = l; mph_gc_push_root(ctx, (void**)&_obj_198); mph_int _ret_199 = _obj_198->column; mph_gc_pop_roots(ctx, 1); _ret_199; })); mph_gc_pop_roots(ctx, 1); _ret_200; });
}
	}
}
	}
}
	} else if ((({ mph_Lexer* _obj_201 = l; mph_gc_push_root(ctx, (void**)&_obj_201); mph_int _ret_202 = _obj_201->ch; mph_gc_pop_roots(ctx, 1); _ret_202; }) == 62)) {
{
	if ((({ mph_Lexer* _arg_203 = l; mph_gc_push_root(ctx, (void**)&_arg_203); mph_int _ret_204 = mph_lexer_peek_char(ctx, NULL, _arg_203); mph_gc_pop_roots(ctx, 1); _ret_204; }) == 61)) {
{
	({ mph_Lexer* _arg_205 = l; mph_gc_push_root(ctx, (void**)&_arg_205); mph_lexer_read_char(ctx, NULL, _arg_205); mph_gc_pop_roots(ctx, 1); });
	tok = ({ MorphString* _arg_206 = mph_string_new(ctx, ">="); mph_gc_push_root(ctx, (void**)&_arg_206); mph_token_Token* _ret_211 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_GE, _arg_206, ({ mph_Lexer* _obj_207 = l; mph_gc_push_root(ctx, (void**)&_obj_207); mph_int _ret_208 = _obj_207->line; mph_gc_pop_roots(ctx, 1); _ret_208; }), ({ mph_Lexer* _obj_209 = l; mph_gc_push_root(ctx, (void**)&_obj_209); mph_int _ret_210 = _obj_209->column; mph_gc_pop_roots(ctx, 1); _ret_210; })); mph_gc_pop_roots(ctx, 1); _ret_211; });
}
	} else {
{
	if ((({ mph_Lexer* _arg_212 = l; mph_gc_push_root(ctx, (void**)&_arg_212); mph_int _ret_213 = mph_lexer_peek_char(ctx, NULL, _arg_212); mph_gc_pop_roots(ctx, 1); _ret_213; }) == 62)) {
{
	({ mph_Lexer* _arg_214 = l; mph_gc_push_root(ctx, (void**)&_arg_214); mph_lexer_read_char(ctx, NULL, _arg_214); mph_gc_pop_roots(ctx, 1); });
	tok = ({ MorphString* _arg_215 = mph_string_new(ctx, ">>"); mph_gc_push_root(ctx, (void**)&_arg_215); mph_token_Token* _ret_220 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_RSHIFT, _arg_215, ({ mph_Lexer* _obj_216 = l; mph_gc_push_root(ctx, (void**)&_obj_216); mph_int _ret_217 = _obj_216->line; mph_gc_pop_roots(ctx, 1); _ret_217; }), ({ mph_Lexer* _obj_218 = l; mph_gc_push_root(ctx, (void**)&_obj_218); mph_int _ret_219 = _obj_218->column; mph_gc_pop_roots(ctx, 1); _ret_219; })); mph_gc_pop_roots(ctx, 1); _ret_220; });
}
	} else {
{
	tok = ({ MorphString* _arg_221 = mph_string_new(ctx, ">"); mph_gc_push_root(ctx, (void**)&_arg_221); mph_token_Token* _ret_226 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_GT, _arg_221, ({ mph_Lexer* _obj_222 = l; mph_gc_push_root(ctx, (void**)&_obj_222); mph_int _ret_223 = _obj_222->line; mph_gc_pop_roots(ctx, 1); _ret_223; }), ({ mph_Lexer* _obj_224 = l; mph_gc_push_root(ctx, (void**)&_obj_224); mph_int _ret_225 = _obj_224->column; mph_gc_pop_roots(ctx, 1); _ret_225; })); mph_gc_pop_roots(ctx, 1); _ret_226; });
}
	}
}
	}
}
	} else if ((({ mph_Lexer* _obj_227 = l; mph_gc_push_root(ctx, (void**)&_obj_227); mph_int _ret_228 = _obj_227->ch; mph_gc_pop_roots(ctx, 1); _ret_228; }) == 38)) {
{
	if ((({ mph_Lexer* _arg_229 = l; mph_gc_push_root(ctx, (void**)&_arg_229); mph_int _ret_230 = mph_lexer_peek_char(ctx, NULL, _arg_229); mph_gc_pop_roots(ctx, 1); _ret_230; }) == 38)) {
{
	({ mph_Lexer* _arg_231 = l; mph_gc_push_root(ctx, (void**)&_arg_231); mph_lexer_read_char(ctx, NULL, _arg_231); mph_gc_pop_roots(ctx, 1); });
	tok = ({ MorphString* _arg_232 = mph_string_new(ctx, "&&"); mph_gc_push_root(ctx, (void**)&_arg_232); mph_token_Token* _ret_237 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_AND, _arg_232, ({ mph_Lexer* _obj_233 = l; mph_gc_push_root(ctx, (void**)&_obj_233); mph_int _ret_234 = _obj_233->line; mph_gc_pop_roots(ctx, 1); _ret_234; }), ({ mph_Lexer* _obj_235 = l; mph_gc_push_root(ctx, (void**)&_obj_235); mph_int _ret_236 = _obj_235->column; mph_gc_pop_roots(ctx, 1); _ret_236; })); mph_gc_pop_roots(ctx, 1); _ret_237; });
}
	} else {
{
	tok = ({ MorphString* _arg_238 = mph_string_new(ctx, "&"); mph_gc_push_root(ctx, (void**)&_arg_238); mph_token_Token* _ret_243 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_BIT_AND, _arg_238, ({ mph_Lexer* _obj_239 = l; mph_gc_push_root(ctx, (void**)&_obj_239); mph_int _ret_240 = _obj_239->line; mph_gc_pop_roots(ctx, 1); _ret_240; }), ({ mph_Lexer* _obj_241 = l; mph_gc_push_root(ctx, (void**)&_obj_241); mph_int _ret_242 = _obj_241->column; mph_gc_pop_roots(ctx, 1); _ret_242; })); mph_gc_pop_roots(ctx, 1); _ret_243; });
}
	}
}
	} else if ((({ mph_Lexer* _obj_244 = l; mph_gc_push_root(ctx, (void**)&_obj_244); mph_int _ret_245 = _obj_244->ch; mph_gc_pop_roots(ctx, 1); _ret_245; }) == 124)) {
{
	if ((({ mph_Lexer* _arg_246 = l; mph_gc_push_root(ctx, (void**)&_arg_246); mph_int _ret_247 = mph_lexer_peek_char(ctx, NULL, _arg_246); mph_gc_pop_roots(ctx, 1); _ret_247; }) == 124)) {
{
	({ mph_Lexer* _arg_248 = l; mph_gc_push_root(ctx, (void**)&_arg_248); mph_lexer_read_char(ctx, NULL, _arg_248); mph_gc_pop_roots(ctx, 1); });
	tok = ({ MorphString* _arg_249 = mph_string_new(ctx, "||"); mph_gc_push_root(ctx, (void**)&_arg_249); mph_token_Token* _ret_254 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_OR, _arg_249, ({ mph_Lexer* _obj_250 = l; mph_gc_push_root(ctx, (void**)&_obj_250); mph_int _ret_251 = _obj_250->line; mph_gc_pop_roots(ctx, 1); _ret_251; }), ({ mph_Lexer* _obj_252 = l; mph_gc_push_root(ctx, (void**)&_obj_252); mph_int _ret_253 = _obj_252->column; mph_gc_pop_roots(ctx, 1); _ret_253; })); mph_gc_pop_roots(ctx, 1); _ret_254; });
}
	} else {
{
	tok = ({ MorphString* _arg_255 = mph_string_new(ctx, "|"); mph_gc_push_root(ctx, (void**)&_arg_255); mph_token_Token* _ret_260 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_BIT_OR, _arg_255, ({ mph_Lexer* _obj_256 = l; mph_gc_push_root(ctx, (void**)&_obj_256); mph_int _ret_257 = _obj_256->line; mph_gc_pop_roots(ctx, 1); _ret_257; }), ({ mph_Lexer* _obj_258 = l; mph_gc_push_root(ctx, (void**)&_obj_258); mph_int _ret_259 = _obj_258->column; mph_gc_pop_roots(ctx, 1); _ret_259; })); mph_gc_pop_roots(ctx, 1); _ret_260; });
}
	}
}
	} else if ((({ mph_Lexer* _obj_261 = l; mph_gc_push_root(ctx, (void**)&_obj_261); mph_int _ret_262 = _obj_261->ch; mph_gc_pop_roots(ctx, 1); _ret_262; }) == 94)) {
{
	tok = ({ MorphString* _arg_263 = mph_string_new(ctx, "^"); mph_gc_push_root(ctx, (void**)&_arg_263); mph_token_Token* _ret_268 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_BIT_XOR, _arg_263, ({ mph_Lexer* _obj_264 = l; mph_gc_push_root(ctx, (void**)&_obj_264); mph_int _ret_265 = _obj_264->line; mph_gc_pop_roots(ctx, 1); _ret_265; }), ({ mph_Lexer* _obj_266 = l; mph_gc_push_root(ctx, (void**)&_obj_266); mph_int _ret_267 = _obj_266->column; mph_gc_pop_roots(ctx, 1); _ret_267; })); mph_gc_pop_roots(ctx, 1); _ret_268; });
}
	} else if ((({ mph_Lexer* _obj_269 = l; mph_gc_push_root(ctx, (void**)&_obj_269); mph_int _ret_270 = _obj_269->ch; mph_gc_pop_roots(ctx, 1); _ret_270; }) == 126)) {
{
	tok = ({ MorphString* _arg_271 = mph_string_new(ctx, "~"); mph_gc_push_root(ctx, (void**)&_arg_271); mph_token_Token* _ret_276 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_BIT_NOT, _arg_271, ({ mph_Lexer* _obj_272 = l; mph_gc_push_root(ctx, (void**)&_obj_272); mph_int _ret_273 = _obj_272->line; mph_gc_pop_roots(ctx, 1); _ret_273; }), ({ mph_Lexer* _obj_274 = l; mph_gc_push_root(ctx, (void**)&_obj_274); mph_int _ret_275 = _obj_274->column; mph_gc_pop_roots(ctx, 1); _ret_275; })); mph_gc_pop_roots(ctx, 1); _ret_276; });
}
	} else if ((({ mph_Lexer* _obj_277 = l; mph_gc_push_root(ctx, (void**)&_obj_277); mph_int _ret_278 = _obj_277->ch; mph_gc_pop_roots(ctx, 1); _ret_278; }) == 44)) {
{
	tok = ({ MorphString* _arg_279 = mph_string_new(ctx, ","); mph_gc_push_root(ctx, (void**)&_arg_279); mph_token_Token* _ret_284 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_COMMA, _arg_279, ({ mph_Lexer* _obj_280 = l; mph_gc_push_root(ctx, (void**)&_obj_280); mph_int _ret_281 = _obj_280->line; mph_gc_pop_roots(ctx, 1); _ret_281; }), ({ mph_Lexer* _obj_282 = l; mph_gc_push_root(ctx, (void**)&_obj_282); mph_int _ret_283 = _obj_282->column; mph_gc_pop_roots(ctx, 1); _ret_283; })); mph_gc_pop_roots(ctx, 1); _ret_284; });
}
	} else if ((({ mph_Lexer* _obj_285 = l; mph_gc_push_root(ctx, (void**)&_obj_285); mph_int _ret_286 = _obj_285->ch; mph_gc_pop_roots(ctx, 1); _ret_286; }) == 59)) {
{
	tok = ({ MorphString* _arg_287 = mph_string_new(ctx, ";"); mph_gc_push_root(ctx, (void**)&_arg_287); mph_token_Token* _ret_292 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_SEMICOLON, _arg_287, ({ mph_Lexer* _obj_288 = l; mph_gc_push_root(ctx, (void**)&_obj_288); mph_int _ret_289 = _obj_288->line; mph_gc_pop_roots(ctx, 1); _ret_289; }), ({ mph_Lexer* _obj_290 = l; mph_gc_push_root(ctx, (void**)&_obj_290); mph_int _ret_291 = _obj_290->column; mph_gc_pop_roots(ctx, 1); _ret_291; })); mph_gc_pop_roots(ctx, 1); _ret_292; });
}
	} else if ((({ mph_Lexer* _obj_293 = l; mph_gc_push_root(ctx, (void**)&_obj_293); mph_int _ret_294 = _obj_293->ch; mph_gc_pop_roots(ctx, 1); _ret_294; }) == 58)) {
{
	tok = ({ MorphString* _arg_295 = mph_string_new(ctx, ":"); mph_gc_push_root(ctx, (void**)&_arg_295); mph_token_Token* _ret_300 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_COLON, _arg_295, ({ mph_Lexer* _obj_296 = l; mph_gc_push_root(ctx, (void**)&_obj_296); mph_int _ret_297 = _obj_296->line; mph_gc_pop_roots(ctx, 1); _ret_297; }), ({ mph_Lexer* _obj_298 = l; mph_gc_push_root(ctx, (void**)&_obj_298); mph_int _ret_299 = _obj_298->column; mph_gc_pop_roots(ctx, 1); _ret_299; })); mph_gc_pop_roots(ctx, 1); _ret_300; });
}
	} else if ((({ mph_Lexer* _obj_301 = l; mph_gc_push_root(ctx, (void**)&_obj_301); mph_int _ret_302 = _obj_301->ch; mph_gc_pop_roots(ctx, 1); _ret_302; }) == 40)) {
{
	tok = ({ MorphString* _arg_303 = mph_string_new(ctx, "("); mph_gc_push_root(ctx, (void**)&_arg_303); mph_token_Token* _ret_308 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_LPAREN, _arg_303, ({ mph_Lexer* _obj_304 = l; mph_gc_push_root(ctx, (void**)&_obj_304); mph_int _ret_305 = _obj_304->line; mph_gc_pop_roots(ctx, 1); _ret_305; }), ({ mph_Lexer* _obj_306 = l; mph_gc_push_root(ctx, (void**)&_obj_306); mph_int _ret_307 = _obj_306->column; mph_gc_pop_roots(ctx, 1); _ret_307; })); mph_gc_pop_roots(ctx, 1); _ret_308; });
}
	} else if ((({ mph_Lexer* _obj_309 = l; mph_gc_push_root(ctx, (void**)&_obj_309); mph_int _ret_310 = _obj_309->ch; mph_gc_pop_roots(ctx, 1); _ret_310; }) == 41)) {
{
	tok = ({ MorphString* _arg_311 = mph_string_new(ctx, ")"); mph_gc_push_root(ctx, (void**)&_arg_311); mph_token_Token* _ret_316 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_RPAREN, _arg_311, ({ mph_Lexer* _obj_312 = l; mph_gc_push_root(ctx, (void**)&_obj_312); mph_int _ret_313 = _obj_312->line; mph_gc_pop_roots(ctx, 1); _ret_313; }), ({ mph_Lexer* _obj_314 = l; mph_gc_push_root(ctx, (void**)&_obj_314); mph_int _ret_315 = _obj_314->column; mph_gc_pop_roots(ctx, 1); _ret_315; })); mph_gc_pop_roots(ctx, 1); _ret_316; });
}
	} else if ((({ mph_Lexer* _obj_317 = l; mph_gc_push_root(ctx, (void**)&_obj_317); mph_int _ret_318 = _obj_317->ch; mph_gc_pop_roots(ctx, 1); _ret_318; }) == 123)) {
{
	tok = ({ MorphString* _arg_319 = mph_string_new(ctx, "{"); mph_gc_push_root(ctx, (void**)&_arg_319); mph_token_Token* _ret_324 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_LBRACE, _arg_319, ({ mph_Lexer* _obj_320 = l; mph_gc_push_root(ctx, (void**)&_obj_320); mph_int _ret_321 = _obj_320->line; mph_gc_pop_roots(ctx, 1); _ret_321; }), ({ mph_Lexer* _obj_322 = l; mph_gc_push_root(ctx, (void**)&_obj_322); mph_int _ret_323 = _obj_322->column; mph_gc_pop_roots(ctx, 1); _ret_323; })); mph_gc_pop_roots(ctx, 1); _ret_324; });
}
	} else if ((({ mph_Lexer* _obj_325 = l; mph_gc_push_root(ctx, (void**)&_obj_325); mph_int _ret_326 = _obj_325->ch; mph_gc_pop_roots(ctx, 1); _ret_326; }) == 125)) {
{
	tok = ({ MorphString* _arg_327 = mph_string_new(ctx, "}"); mph_gc_push_root(ctx, (void**)&_arg_327); mph_token_Token* _ret_332 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_RBRACE, _arg_327, ({ mph_Lexer* _obj_328 = l; mph_gc_push_root(ctx, (void**)&_obj_328); mph_int _ret_329 = _obj_328->line; mph_gc_pop_roots(ctx, 1); _ret_329; }), ({ mph_Lexer* _obj_330 = l; mph_gc_push_root(ctx, (void**)&_obj_330); mph_int _ret_331 = _obj_330->column; mph_gc_pop_roots(ctx, 1); _ret_331; })); mph_gc_pop_roots(ctx, 1); _ret_332; });
}
	} else if ((({ mph_Lexer* _obj_333 = l; mph_gc_push_root(ctx, (void**)&_obj_333); mph_int _ret_334 = _obj_333->ch; mph_gc_pop_roots(ctx, 1); _ret_334; }) == 91)) {
{
	tok = ({ MorphString* _arg_335 = mph_string_new(ctx, "["); mph_gc_push_root(ctx, (void**)&_arg_335); mph_token_Token* _ret_340 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_LBRACKET, _arg_335, ({ mph_Lexer* _obj_336 = l; mph_gc_push_root(ctx, (void**)&_obj_336); mph_int _ret_337 = _obj_336->line; mph_gc_pop_roots(ctx, 1); _ret_337; }), ({ mph_Lexer* _obj_338 = l; mph_gc_push_root(ctx, (void**)&_obj_338); mph_int _ret_339 = _obj_338->column; mph_gc_pop_roots(ctx, 1); _ret_339; })); mph_gc_pop_roots(ctx, 1); _ret_340; });
}
	} else if ((({ mph_Lexer* _obj_341 = l; mph_gc_push_root(ctx, (void**)&_obj_341); mph_int _ret_342 = _obj_341->ch; mph_gc_pop_roots(ctx, 1); _ret_342; }) == 93)) {
{
	tok = ({ MorphString* _arg_343 = mph_string_new(ctx, "]"); mph_gc_push_root(ctx, (void**)&_arg_343); mph_token_Token* _ret_348 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_RBRACKET, _arg_343, ({ mph_Lexer* _obj_344 = l; mph_gc_push_root(ctx, (void**)&_obj_344); mph_int _ret_345 = _obj_344->line; mph_gc_pop_roots(ctx, 1); _ret_345; }), ({ mph_Lexer* _obj_346 = l; mph_gc_push_root(ctx, (void**)&_obj_346); mph_int _ret_347 = _obj_346->column; mph_gc_pop_roots(ctx, 1); _ret_347; })); mph_gc_pop_roots(ctx, 1); _ret_348; });
}
	} else if ((({ mph_Lexer* _obj_349 = l; mph_gc_push_root(ctx, (void**)&_obj_349); mph_int _ret_350 = _obj_349->ch; mph_gc_pop_roots(ctx, 1); _ret_350; }) == 46)) {
{
	tok = ({ MorphString* _arg_351 = mph_string_new(ctx, "."); mph_gc_push_root(ctx, (void**)&_arg_351); mph_token_Token* _ret_356 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_DOT, _arg_351, ({ mph_Lexer* _obj_352 = l; mph_gc_push_root(ctx, (void**)&_obj_352); mph_int _ret_353 = _obj_352->line; mph_gc_pop_roots(ctx, 1); _ret_353; }), ({ mph_Lexer* _obj_354 = l; mph_gc_push_root(ctx, (void**)&_obj_354); mph_int _ret_355 = _obj_354->column; mph_gc_pop_roots(ctx, 1); _ret_355; })); mph_gc_pop_roots(ctx, 1); _ret_356; });
}
	} else if ((({ mph_Lexer* _obj_357 = l; mph_gc_push_root(ctx, (void**)&_obj_357); mph_int _ret_358 = _obj_357->ch; mph_gc_pop_roots(ctx, 1); _ret_358; }) == 34)) {
{
	MorphString* str_literal = ({ mph_Lexer* _arg_359 = l; mph_gc_push_root(ctx, (void**)&_arg_359); MorphString* _ret_360 = mph_lexer_read_string(ctx, NULL, _arg_359); mph_gc_pop_roots(ctx, 1); _ret_360; });
	mph_gc_push_root(ctx, (void**)&str_literal);
	tok = ({ MorphString* _arg_361 = str_literal; mph_gc_push_root(ctx, (void**)&_arg_361); mph_token_Token* _ret_366 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_STRING, _arg_361, ({ mph_Lexer* _obj_362 = l; mph_gc_push_root(ctx, (void**)&_obj_362); mph_int _ret_363 = _obj_362->line; mph_gc_pop_roots(ctx, 1); _ret_363; }), ({ mph_Lexer* _obj_364 = l; mph_gc_push_root(ctx, (void**)&_obj_364); mph_int _ret_365 = _obj_364->column; mph_gc_pop_roots(ctx, 1); _ret_365; })); mph_gc_pop_roots(ctx, 1); _ret_366; });
	mph_gc_pop_roots(ctx, 1);
}
	} else if ((({ mph_Lexer* _obj_367 = l; mph_gc_push_root(ctx, (void**)&_obj_367); mph_int _ret_368 = _obj_367->ch; mph_gc_pop_roots(ctx, 1); _ret_368; }) == 0)) {
{
	tok = ({ MorphString* _arg_369 = mph_string_new(ctx, "-"); mph_gc_push_root(ctx, (void**)&_arg_369); mph_token_Token* _ret_374 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_EOF, _arg_369, ({ mph_Lexer* _obj_370 = l; mph_gc_push_root(ctx, (void**)&_obj_370); mph_int _ret_371 = _obj_370->line; mph_gc_pop_roots(ctx, 1); _ret_371; }), ({ mph_Lexer* _obj_372 = l; mph_gc_push_root(ctx, (void**)&_obj_372); mph_int _ret_373 = _obj_372->column; mph_gc_pop_roots(ctx, 1); _ret_373; })); mph_gc_pop_roots(ctx, 1); _ret_374; });
}
	} else {
{
	if (mph_is_letter(ctx, NULL, ({ mph_Lexer* _obj_375 = l; mph_gc_push_root(ctx, (void**)&_obj_375); mph_int _ret_376 = _obj_375->ch; mph_gc_pop_roots(ctx, 1); _ret_376; }))) {
{
	MorphString* literal = ({ mph_Lexer* _arg_377 = l; mph_gc_push_root(ctx, (void**)&_arg_377); MorphString* _ret_378 = mph_lexer_read_identifier(ctx, NULL, _arg_377); mph_gc_pop_roots(ctx, 1); _ret_378; });
	mph_gc_push_root(ctx, (void**)&literal);
	mph_int token_type = ({ MorphString* _arg_379 = literal; mph_gc_push_root(ctx, (void**)&_arg_379); mph_int _ret_380 = mph_token_LookupKeyword(ctx, NULL, _arg_379); mph_gc_pop_roots(ctx, 1); _ret_380; });
	tok = ({ MorphString* _arg_381 = literal; mph_gc_push_root(ctx, (void**)&_arg_381); mph_token_Token* _ret_386 = mph_token_MakeToken(ctx, NULL, token_type, _arg_381, ({ mph_Lexer* _obj_382 = l; mph_gc_push_root(ctx, (void**)&_obj_382); mph_int _ret_383 = _obj_382->line; mph_gc_pop_roots(ctx, 1); _ret_383; }), ({ mph_Lexer* _obj_384 = l; mph_gc_push_root(ctx, (void**)&_obj_384); mph_int _ret_385 = _obj_384->column; mph_gc_pop_roots(ctx, 1); _ret_385; })); mph_gc_pop_roots(ctx, 1); _ret_386; });
	return tok;
	mph_gc_pop_roots(ctx, 1);
}
	} else {
{
	if (mph_is_digit(ctx, NULL, ({ mph_Lexer* _obj_387 = l; mph_gc_push_root(ctx, (void**)&_obj_387); mph_int _ret_388 = _obj_387->ch; mph_gc_pop_roots(ctx, 1); _ret_388; }))) {
{
	MorphString* literal = ({ mph_Lexer* _arg_389 = l; mph_gc_push_root(ctx, (void**)&_arg_389); MorphString* _ret_390 = mph_lexer_read_number(ctx, NULL, _arg_389); mph_gc_pop_roots(ctx, 1); _ret_390; });
	mph_gc_push_root(ctx, (void**)&literal);
	tok = ({ MorphString* _arg_391 = literal; mph_gc_push_root(ctx, (void**)&_arg_391); mph_token_Token* _ret_396 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_INT, _arg_391, ({ mph_Lexer* _obj_392 = l; mph_gc_push_root(ctx, (void**)&_obj_392); mph_int _ret_393 = _obj_392->line; mph_gc_pop_roots(ctx, 1); _ret_393; }), ({ mph_Lexer* _obj_394 = l; mph_gc_push_root(ctx, (void**)&_obj_394); mph_int _ret_395 = _obj_394->column; mph_gc_pop_roots(ctx, 1); _ret_395; })); mph_gc_pop_roots(ctx, 1); _ret_396; });
	return tok;
	mph_gc_pop_roots(ctx, 1);
}
	} else {
{
	tok = ({ MorphString* _arg_397 = mph_string_new(ctx, "-"); mph_gc_push_root(ctx, (void**)&_arg_397); mph_token_Token* _ret_402 = mph_token_MakeToken(ctx, NULL, mph_token_TOKEN_ILLEGAL, _arg_397, ({ mph_Lexer* _obj_398 = l; mph_gc_push_root(ctx, (void**)&_obj_398); mph_int _ret_399 = _obj_398->line; mph_gc_pop_roots(ctx, 1); _ret_399; }), ({ mph_Lexer* _obj_400 = l; mph_gc_push_root(ctx, (void**)&_obj_400); mph_int _ret_401 = _obj_400->column; mph_gc_pop_roots(ctx, 1); _ret_401; })); mph_gc_pop_roots(ctx, 1); _ret_402; });
}
	}
}
	}
}
	}
	({ mph_Lexer* _arg_403 = l; mph_gc_push_root(ctx, (void**)&_arg_403); mph_lexer_read_char(ctx, NULL, _arg_403); mph_gc_pop_roots(ctx, 1); });
	return tok;
	mph_gc_pop_roots(ctx, 1);
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_token_TOKEN_ILLEGAL = 0;
	mph_token_TOKEN_EOF = 1;
	mph_token_TOKEN_IDENT = 2;
	mph_token_TOKEN_INT = 3;
	mph_token_TOKEN_FLOAT = 4;
	mph_token_TOKEN_STRING = 5;
	mph_token_TOKEN_CHAR = 6;
	mph_token_TOKEN_ASSIGN = 7;
	mph_token_TOKEN_PLUS = 8;
	mph_token_TOKEN_MINUS = 9;
	mph_token_TOKEN_BANG = 10;
	mph_token_TOKEN_ASTERISK = 11;
	mph_token_TOKEN_SLASH = 12;
	mph_token_TOKEN_PERCENT = 13;
	mph_token_TOKEN_LT = 14;
	mph_token_TOKEN_GT = 15;
	mph_token_TOKEN_EQ = 16;
	mph_token_TOKEN_NOT_EQ = 17;
	mph_token_TOKEN_LE = 18;
	mph_token_TOKEN_GE = 19;
	mph_token_TOKEN_BIT_AND = 20;
	mph_token_TOKEN_BIT_OR = 21;
	mph_token_TOKEN_BIT_XOR = 22;
	mph_token_TOKEN_BIT_NOT = 23;
	mph_token_TOKEN_LSHIFT = 24;
	mph_token_TOKEN_RSHIFT = 25;
	mph_token_TOKEN_AND = 26;
	mph_token_TOKEN_OR = 27;
	mph_token_TOKEN_COMMA = 28;
	mph_token_TOKEN_SEMICOLON = 29;
	mph_token_TOKEN_COLON = 30;
	mph_token_TOKEN_LPAREN = 31;
	mph_token_TOKEN_RPAREN = 32;
	mph_token_TOKEN_LBRACE = 33;
	mph_token_TOKEN_RBRACE = 34;
	mph_token_TOKEN_LBRACKET = 35;
	mph_token_TOKEN_RBRACKET = 36;
	mph_token_TOKEN_FUNGSI = 37;
	mph_token_TOKEN_VAR = 38;
	mph_token_TOKEN_TETAPAN = 39;
	mph_token_TOKEN_BENAR = 40;
	mph_token_TOKEN_SALAH = 41;
	mph_token_TOKEN_JIKA = 42;
	mph_token_TOKEN_LAINNYA = 43;
	mph_token_TOKEN_KEMBALIKAN = 44;
	mph_token_TOKEN_STRUKTUR = 45;
	mph_token_TOKEN_ANTARMUKA = 46;
	mph_token_TOKEN_AMBIL = 47;
	mph_token_TOKEN_UNTUK = 48;
	mph_token_TOKEN_SELAMA = 49;
	mph_token_TOKEN_PUTUS = 50;
	mph_token_TOKEN_LANJUT = 51;
	mph_token_TOKEN_PILIH = 52;
	mph_token_TOKEN_KASUS = 53;
	mph_token_TOKEN_BAWAAN = 54;
	mph_token_TOKEN_AKHIR = 55;
	mph_token_TOKEN_ATAU_JIKA = 56;
	mph_token_TOKEN_DAN = 57;
	mph_token_TOKEN_ATAU = 58;
	mph_token_TOKEN_KOSONG = 59;
	mph_token_TOKEN_LUNCURKAN = 60;
	mph_token_TOKEN_SALURAN_BARU = 61;
	mph_token_TOKEN_KIRIM = 62;
	mph_token_TOKEN_TERIMA = 63;
	mph_token_TOKEN_DOT = 64;
	mph_token_TOKEN_ARROW = 65;
}
