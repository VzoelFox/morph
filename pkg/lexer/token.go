package lexer

import "strings"

type TokenType string

type Token struct {
	Type            TokenType
	Literal         string
	Line            int
	Column          int
	HasLeadingSpace bool
}

const (
	ILLEGAL = "ILLEGAL"
	EOF     = "EOF"

	// Identifiers + Literals
	IDENT  = "IDENT"  // add, foobar, x, y
	INT    = "INT"    // 1343456
	FLOAT  = "FLOAT"  // 123.45
	STRING = "STRING" // "foobar"

	// Operators
	ASSIGN   = "="
	PLUS     = "+"
	MINUS    = "-"
	BANG     = "!"
	ASTERISK = "*"
	SLASH    = "/"

	LT     = "<"
	GT     = ">"
	EQ     = "=="
	NOT_EQ = "!="
	LTE    = "<="
	GTE    = ">="

	// Bitwise Operators
	AND    = "&"
	OR     = "|"
	XOR    = "^"
	TILDE  = "~" // Bitwise NOT
	LSHIFT = "<<"
	RSHIFT = ">>"

	// Delimiters
	COMMA     = ","
	SEMICOLON = ";"
	COLON     = ":"
	DOT       = "."

	LPAREN   = "("
	RPAREN   = ")"
	LBRACE   = "{"
	RBRACE   = "}"
	LBRACKET = "["
	RBRACKET = "]"

	// Interpolation
	INTERP_START = "#{"

	// Keywords
	FUNGSI     = "FUNGSI"
	JIKA       = "JIKA"
	ATAU_JIKA  = "ATAU_JIKA"
	LAINNYA    = "LAINNYA"
	KEMBALIKAN = "KEMBALIKAN"
	BENAR      = "BENAR"
	SALAH      = "SALAH"
	KOSONG     = "KOSONG"
	AKHIR      = "AKHIR"
	SELAMA     = "SELAMA"
	DAN        = "DAN"
	ATAU       = "ATAU"
	AMBIL      = "AMBIL"
	DARI       = "DARI"
	BERHENTI   = "BERHENTI"
	LANJUT     = "LANJUT"
	STRUKTUR   = "STRUKTUR"
	COMMENT    = "COMMENT"
)

var keywords = map[string]TokenType{
	"struktur":   STRUKTUR,
	"fungsi":     FUNGSI,
	"jika":       JIKA,
	"atau_jika":  ATAU_JIKA,
	"lainnya":    LAINNYA,
	"kembalikan": KEMBALIKAN,
	"benar":      BENAR,
	"salah":      SALAH,
	"kosong":     KOSONG,
	"akhir":      AKHIR,
	"selama":     SELAMA,
	"dan":        DAN,
	"atau":       ATAU,
	"ambil":      AMBIL,
	"dari":       DARI,
	"berhenti":   BERHENTI,
	"lanjut":     LANJUT,
}

// LookupIdent checks if an identifier is a keyword (case-insensitive)
func LookupIdent(ident string) TokenType {
	lowerIdent := strings.ToLower(ident)
	if tok, ok := keywords[lowerIdent]; ok {
		return tok
	}
	return IDENT
}
