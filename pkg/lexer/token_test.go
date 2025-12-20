package lexer

import (
	"testing"
)

func TestLookupIdent(t *testing.T) {
	tests := []struct {
		input    string
		expected TokenType
	}{
		{"fungsi", FUNGSI},
		{"jika", JIKA},
		{"atau_jika", ATAU_JIKA},
		{"lainnya", LAINNYA},
		{"kembalikan", KEMBALIKAN},
		{"benar", BENAR},
		{"salah", SALAH},
		{"kosong", KOSONG},
		{"akhir", AKHIR},
		{"selama", SELAMA},
		{"variabel", IDENT},
		{"x", IDENT},
		{"foobar", IDENT},
		{"fungs", IDENT},       // Typo
		{"FUNGSI", FUNGSI},     // Case insensitive check
		{"Fungsi", FUNGSI},     // Case insensitive check
		{"JiKa", JIKA},         // Mixed case
	}

	for i, tt := range tests {
		tok := LookupIdent(tt.input)
		if tok != tt.expected {
			t.Errorf("tests[%d] - token type wrong. expected=%q, got=%q",
				i, tt.expected, tok)
		}
	}
}

func TestTokenTypes(t *testing.T) {
	// Verify some critical token values
	tests := []struct {
		tokenType TokenType
		expected  string
	}{
		{ILLEGAL, "ILLEGAL"},
		{EOF, "EOF"},
		{IDENT, "IDENT"},
		{INT, "INT"},
		{ASSIGN, "="},
		{PLUS, "+"},
		{COMMA, ","},
		{FUNGSI, "FUNGSI"},
	}

	for i, tt := range tests {
		if string(tt.tokenType) != tt.expected {
			t.Errorf("tests[%d] - token literal wrong. expected=%q, got=%q",
				i, tt.expected, string(tt.tokenType))
		}
	}
}
