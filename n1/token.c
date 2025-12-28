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
	MorphString* literal;
	mph_int line;
	mph_int column;
};

MorphTypeInfo mph_ti_mph_Token = { "Token", sizeof(mph_Token), 1, (size_t[]){offsetof(mph_Token, literal)} };

// Global Variables
mph_int mph_TOKEN_ILLEGAL;
mph_int mph_TOKEN_EOF;
mph_int mph_TOKEN_IDENT;
mph_int mph_TOKEN_INT;
mph_int mph_TOKEN_FLOAT;
mph_int mph_TOKEN_STRING;
mph_int mph_TOKEN_CHAR;
mph_int mph_TOKEN_ASSIGN;
mph_int mph_TOKEN_PLUS;
mph_int mph_TOKEN_MINUS;
mph_int mph_TOKEN_BANG;
mph_int mph_TOKEN_ASTERISK;
mph_int mph_TOKEN_SLASH;
mph_int mph_TOKEN_PERCENT;
mph_int mph_TOKEN_LT;
mph_int mph_TOKEN_GT;
mph_int mph_TOKEN_EQ;
mph_int mph_TOKEN_NOT_EQ;
mph_int mph_TOKEN_LE;
mph_int mph_TOKEN_GE;
mph_int mph_TOKEN_BIT_AND;
mph_int mph_TOKEN_BIT_OR;
mph_int mph_TOKEN_BIT_XOR;
mph_int mph_TOKEN_BIT_NOT;
mph_int mph_TOKEN_LSHIFT;
mph_int mph_TOKEN_RSHIFT;
mph_int mph_TOKEN_AND;
mph_int mph_TOKEN_OR;
mph_int mph_TOKEN_COMMA;
mph_int mph_TOKEN_SEMICOLON;
mph_int mph_TOKEN_COLON;
mph_int mph_TOKEN_LPAREN;
mph_int mph_TOKEN_RPAREN;
mph_int mph_TOKEN_LBRACE;
mph_int mph_TOKEN_RBRACE;
mph_int mph_TOKEN_LBRACKET;
mph_int mph_TOKEN_RBRACKET;
mph_int mph_TOKEN_FUNGSI;
mph_int mph_TOKEN_VAR;
mph_int mph_TOKEN_TETAPAN;
mph_int mph_TOKEN_BENAR;
mph_int mph_TOKEN_SALAH;
mph_int mph_TOKEN_JIKA;
mph_int mph_TOKEN_LAINNYA;
mph_int mph_TOKEN_KEMBALIKAN;
mph_int mph_TOKEN_STRUKTUR;
mph_int mph_TOKEN_ANTARMUKA;
mph_int mph_TOKEN_AMBIL;
mph_int mph_TOKEN_UNTUK;
mph_int mph_TOKEN_SELAMA;
mph_int mph_TOKEN_PUTUS;
mph_int mph_TOKEN_LANJUT;
mph_int mph_TOKEN_PILIH;
mph_int mph_TOKEN_KASUS;
mph_int mph_TOKEN_BAWAAN;
mph_int mph_TOKEN_AKHIR;
mph_int mph_TOKEN_ATAU_JIKA;
mph_int mph_TOKEN_DAN;
mph_int mph_TOKEN_ATAU;
mph_int mph_TOKEN_KOSONG;
mph_int mph_TOKEN_LUNCURKAN;
mph_int mph_TOKEN_SALURAN_BARU;
mph_int mph_TOKEN_KIRIM;
mph_int mph_TOKEN_TERIMA;
mph_int mph_TOKEN_DOT;
mph_int mph_TOKEN_ARROW;

// Function Prototypes
mph_Token* mph_MakeToken(MorphContext* ctx, void* _env_void, mph_int t_type, MorphString* lit, mph_int ln, mph_int col);
MorphString* mph_TokenTypeString(MorphContext* ctx, void* _env_void, mph_int t_type);
mph_int mph_LookupKeyword(MorphContext* ctx, void* _env_void, MorphString* ident);

// Function Definitions
mph_Token* mph_MakeToken(MorphContext* ctx, void* _env_void, mph_int t_type, MorphString* lit, mph_int ln, mph_int col) {
	mph_gc_push_root(ctx, (void**)&lit);
	return ({ mph_Token* _t = (mph_Token*)mph_alloc(ctx, sizeof(mph_Token), &mph_ti_mph_Token); mph_gc_push_root(ctx, (void**)&_t); _t->token_type = t_type; _t->literal = lit; _t->line = ln; _t->column = col; mph_gc_pop_roots(ctx, 1); _t; });
	mph_gc_pop_roots(ctx, 1);
}

MorphString* mph_TokenTypeString(MorphContext* ctx, void* _env_void, mph_int t_type) {
	if ((t_type == mph_TOKEN_ILLEGAL)) {
{
	return mph_string_new(ctx, "ILLEGAL");
}
	}
	if ((t_type == mph_TOKEN_EOF)) {
{
	return mph_string_new(ctx, "EOF");
}
	}
	if ((t_type == mph_TOKEN_IDENT)) {
{
	return mph_string_new(ctx, "IDENT");
}
	}
	if ((t_type == mph_TOKEN_INT)) {
{
	return mph_string_new(ctx, "INT");
}
	}
	if ((t_type == mph_TOKEN_FLOAT)) {
{
	return mph_string_new(ctx, "FLOAT");
}
	}
	if ((t_type == mph_TOKEN_STRING)) {
{
	return mph_string_new(ctx, "STRING");
}
	}
	if ((t_type == mph_TOKEN_CHAR)) {
{
	return mph_string_new(ctx, "CHAR");
}
	}
	if ((t_type == mph_TOKEN_ASSIGN)) {
{
	return mph_string_new(ctx, "=");
}
	}
	if ((t_type == mph_TOKEN_PLUS)) {
{
	return mph_string_new(ctx, "+");
}
	}
	if ((t_type == mph_TOKEN_MINUS)) {
{
	return mph_string_new(ctx, "-");
}
	}
	if ((t_type == mph_TOKEN_BANG)) {
{
	return mph_string_new(ctx, "!");
}
	}
	if ((t_type == mph_TOKEN_ASTERISK)) {
{
	return mph_string_new(ctx, "*");
}
	}
	if ((t_type == mph_TOKEN_SLASH)) {
{
	return mph_string_new(ctx, "/");
}
	}
	if ((t_type == mph_TOKEN_PERCENT)) {
{
	return mph_string_new(ctx, "%");
}
	}
	if ((t_type == mph_TOKEN_LT)) {
{
	return mph_string_new(ctx, "<");
}
	}
	if ((t_type == mph_TOKEN_GT)) {
{
	return mph_string_new(ctx, ">");
}
	}
	if ((t_type == mph_TOKEN_EQ)) {
{
	return mph_string_new(ctx, "==");
}
	}
	if ((t_type == mph_TOKEN_NOT_EQ)) {
{
	return mph_string_new(ctx, "!=");
}
	}
	if ((t_type == mph_TOKEN_LE)) {
{
	return mph_string_new(ctx, "<=");
}
	}
	if ((t_type == mph_TOKEN_GE)) {
{
	return mph_string_new(ctx, ">=");
}
	}
	if ((t_type == mph_TOKEN_BIT_AND)) {
{
	return mph_string_new(ctx, "&");
}
	}
	if ((t_type == mph_TOKEN_BIT_OR)) {
{
	return mph_string_new(ctx, "|");
}
	}
	if ((t_type == mph_TOKEN_BIT_XOR)) {
{
	return mph_string_new(ctx, "^");
}
	}
	if ((t_type == mph_TOKEN_BIT_NOT)) {
{
	return mph_string_new(ctx, "~");
}
	}
	if ((t_type == mph_TOKEN_LSHIFT)) {
{
	return mph_string_new(ctx, "<<");
}
	}
	if ((t_type == mph_TOKEN_RSHIFT)) {
{
	return mph_string_new(ctx, ">>");
}
	}
	if ((t_type == mph_TOKEN_AND)) {
{
	return mph_string_new(ctx, "&&");
}
	}
	if ((t_type == mph_TOKEN_OR)) {
{
	return mph_string_new(ctx, "||");
}
	}
	if ((t_type == mph_TOKEN_COMMA)) {
{
	return mph_string_new(ctx, ",");
}
	}
	if ((t_type == mph_TOKEN_SEMICOLON)) {
{
	return mph_string_new(ctx, ";");
}
	}
	if ((t_type == mph_TOKEN_COLON)) {
{
	return mph_string_new(ctx, ":");
}
	}
	if ((t_type == mph_TOKEN_LPAREN)) {
{
	return mph_string_new(ctx, "(");
}
	}
	if ((t_type == mph_TOKEN_RPAREN)) {
{
	return mph_string_new(ctx, ")");
}
	}
	if ((t_type == mph_TOKEN_LBRACE)) {
{
	return mph_string_new(ctx, "{");
}
	}
	if ((t_type == mph_TOKEN_RBRACE)) {
{
	return mph_string_new(ctx, "}");
}
	}
	if ((t_type == mph_TOKEN_LBRACKET)) {
{
	return mph_string_new(ctx, "[");
}
	}
	if ((t_type == mph_TOKEN_RBRACKET)) {
{
	return mph_string_new(ctx, "]");
}
	}
	if ((t_type == mph_TOKEN_FUNGSI)) {
{
	return mph_string_new(ctx, "fungsi");
}
	}
	if ((t_type == mph_TOKEN_VAR)) {
{
	return mph_string_new(ctx, "var");
}
	}
	if ((t_type == mph_TOKEN_TETAPAN)) {
{
	return mph_string_new(ctx, "tetapan");
}
	}
	if ((t_type == mph_TOKEN_BENAR)) {
{
	return mph_string_new(ctx, "benar");
}
	}
	if ((t_type == mph_TOKEN_SALAH)) {
{
	return mph_string_new(ctx, "salah");
}
	}
	if ((t_type == mph_TOKEN_JIKA)) {
{
	return mph_string_new(ctx, "jika");
}
	}
	if ((t_type == mph_TOKEN_LAINNYA)) {
{
	return mph_string_new(ctx, "lainnya");
}
	}
	if ((t_type == mph_TOKEN_KEMBALIKAN)) {
{
	return mph_string_new(ctx, "kembalikan");
}
	}
	if ((t_type == mph_TOKEN_STRUKTUR)) {
{
	return mph_string_new(ctx, "struktur");
}
	}
	if ((t_type == mph_TOKEN_ANTARMUKA)) {
{
	return mph_string_new(ctx, "antarmuka");
}
	}
	if ((t_type == mph_TOKEN_AMBIL)) {
{
	return mph_string_new(ctx, "ambil");
}
	}
	if ((t_type == mph_TOKEN_UNTUK)) {
{
	return mph_string_new(ctx, "untuk");
}
	}
	if ((t_type == mph_TOKEN_SELAMA)) {
{
	return mph_string_new(ctx, "selama");
}
	}
	if ((t_type == mph_TOKEN_PUTUS)) {
{
	return mph_string_new(ctx, "putus");
}
	}
	if ((t_type == mph_TOKEN_LANJUT)) {
{
	return mph_string_new(ctx, "lanjut");
}
	}
	if ((t_type == mph_TOKEN_PILIH)) {
{
	return mph_string_new(ctx, "pilih");
}
	}
	if ((t_type == mph_TOKEN_KASUS)) {
{
	return mph_string_new(ctx, "kasus");
}
	}
	if ((t_type == mph_TOKEN_BAWAAN)) {
{
	return mph_string_new(ctx, "bawaan");
}
	}
	if ((t_type == mph_TOKEN_AKHIR)) {
{
	return mph_string_new(ctx, "akhir");
}
	}
	if ((t_type == mph_TOKEN_ATAU_JIKA)) {
{
	return mph_string_new(ctx, "atau_jika");
}
	}
	if ((t_type == mph_TOKEN_DAN)) {
{
	return mph_string_new(ctx, "dan");
}
	}
	if ((t_type == mph_TOKEN_ATAU)) {
{
	return mph_string_new(ctx, "atau");
}
	}
	if ((t_type == mph_TOKEN_KOSONG)) {
{
	return mph_string_new(ctx, "kosong");
}
	}
	if ((t_type == mph_TOKEN_LUNCURKAN)) {
{
	return mph_string_new(ctx, "luncurkan");
}
	}
	if ((t_type == mph_TOKEN_SALURAN_BARU)) {
{
	return mph_string_new(ctx, "saluran_baru");
}
	}
	if ((t_type == mph_TOKEN_KIRIM)) {
{
	return mph_string_new(ctx, "kirim");
}
	}
	if ((t_type == mph_TOKEN_TERIMA)) {
{
	return mph_string_new(ctx, "terima");
}
	}
	if ((t_type == mph_TOKEN_DOT)) {
{
	return mph_string_new(ctx, ".");
}
	}
	if ((t_type == mph_TOKEN_ARROW)) {
{
	return mph_string_new(ctx, "->");
}
	}
	return mph_string_new(ctx, "UNKNOWN");
}

mph_int mph_LookupKeyword(MorphContext* ctx, void* _env_void, MorphString* ident) {
	mph_gc_push_root(ctx, (void**)&ident);
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "fungsi"))) {
{
	return mph_TOKEN_FUNGSI;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "var"))) {
{
	return mph_TOKEN_VAR;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "tetapan"))) {
{
	return mph_TOKEN_TETAPAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "benar"))) {
{
	return mph_TOKEN_BENAR;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "salah"))) {
{
	return mph_TOKEN_SALAH;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "jika"))) {
{
	return mph_TOKEN_JIKA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "lainnya"))) {
{
	return mph_TOKEN_LAINNYA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "kembalikan"))) {
{
	return mph_TOKEN_KEMBALIKAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "struktur"))) {
{
	return mph_TOKEN_STRUKTUR;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "antarmuka"))) {
{
	return mph_TOKEN_ANTARMUKA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "ambil"))) {
{
	return mph_TOKEN_AMBIL;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "untuk"))) {
{
	return mph_TOKEN_UNTUK;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "selama"))) {
{
	return mph_TOKEN_SELAMA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "putus"))) {
{
	return mph_TOKEN_PUTUS;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "lanjut"))) {
{
	return mph_TOKEN_LANJUT;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "pilih"))) {
{
	return mph_TOKEN_PILIH;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "kasus"))) {
{
	return mph_TOKEN_KASUS;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "bawaan"))) {
{
	return mph_TOKEN_BAWAAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "akhir"))) {
{
	return mph_TOKEN_AKHIR;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "atau_jika"))) {
{
	return mph_TOKEN_ATAU_JIKA;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "dan"))) {
{
	return mph_TOKEN_DAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "atau"))) {
{
	return mph_TOKEN_ATAU;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "kosong"))) {
{
	return mph_TOKEN_KOSONG;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "luncurkan"))) {
{
	return mph_TOKEN_LUNCURKAN;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "saluran_baru"))) {
{
	return mph_TOKEN_SALURAN_BARU;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "kirim"))) {
{
	return mph_TOKEN_KIRIM;
}
	}
	if (mph_string_eq(ctx, ident, mph_string_new(ctx, "terima"))) {
{
	return mph_TOKEN_TERIMA;
}
	}
	return mph_TOKEN_IDENT;
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_TOKEN_ILLEGAL = 0;
	mph_TOKEN_EOF = 1;
	mph_TOKEN_IDENT = 2;
	mph_TOKEN_INT = 3;
	mph_TOKEN_FLOAT = 4;
	mph_TOKEN_STRING = 5;
	mph_TOKEN_CHAR = 6;
	mph_TOKEN_ASSIGN = 7;
	mph_TOKEN_PLUS = 8;
	mph_TOKEN_MINUS = 9;
	mph_TOKEN_BANG = 10;
	mph_TOKEN_ASTERISK = 11;
	mph_TOKEN_SLASH = 12;
	mph_TOKEN_PERCENT = 13;
	mph_TOKEN_LT = 14;
	mph_TOKEN_GT = 15;
	mph_TOKEN_EQ = 16;
	mph_TOKEN_NOT_EQ = 17;
	mph_TOKEN_LE = 18;
	mph_TOKEN_GE = 19;
	mph_TOKEN_BIT_AND = 20;
	mph_TOKEN_BIT_OR = 21;
	mph_TOKEN_BIT_XOR = 22;
	mph_TOKEN_BIT_NOT = 23;
	mph_TOKEN_LSHIFT = 24;
	mph_TOKEN_RSHIFT = 25;
	mph_TOKEN_AND = 26;
	mph_TOKEN_OR = 27;
	mph_TOKEN_COMMA = 28;
	mph_TOKEN_SEMICOLON = 29;
	mph_TOKEN_COLON = 30;
	mph_TOKEN_LPAREN = 31;
	mph_TOKEN_RPAREN = 32;
	mph_TOKEN_LBRACE = 33;
	mph_TOKEN_RBRACE = 34;
	mph_TOKEN_LBRACKET = 35;
	mph_TOKEN_RBRACKET = 36;
	mph_TOKEN_FUNGSI = 37;
	mph_TOKEN_VAR = 38;
	mph_TOKEN_TETAPAN = 39;
	mph_TOKEN_BENAR = 40;
	mph_TOKEN_SALAH = 41;
	mph_TOKEN_JIKA = 42;
	mph_TOKEN_LAINNYA = 43;
	mph_TOKEN_KEMBALIKAN = 44;
	mph_TOKEN_STRUKTUR = 45;
	mph_TOKEN_ANTARMUKA = 46;
	mph_TOKEN_AMBIL = 47;
	mph_TOKEN_UNTUK = 48;
	mph_TOKEN_SELAMA = 49;
	mph_TOKEN_PUTUS = 50;
	mph_TOKEN_LANJUT = 51;
	mph_TOKEN_PILIH = 52;
	mph_TOKEN_KASUS = 53;
	mph_TOKEN_BAWAAN = 54;
	mph_TOKEN_AKHIR = 55;
	mph_TOKEN_ATAU_JIKA = 56;
	mph_TOKEN_DAN = 57;
	mph_TOKEN_ATAU = 58;
	mph_TOKEN_KOSONG = 59;
	mph_TOKEN_LUNCURKAN = 60;
	mph_TOKEN_SALURAN_BARU = 61;
	mph_TOKEN_KIRIM = 62;
	mph_TOKEN_TERIMA = 63;
	mph_TOKEN_DOT = 64;
	mph_TOKEN_ARROW = 65;
}
