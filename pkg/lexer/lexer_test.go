package lexer

import (
	"testing"
)

func TestNextToken(t *testing.T) {
	input := `x = 5
+ - / *
fungsi tambah(x, y) {
    kembalikan x + y
}
# This is a comment
x < y
`

	tests := []struct {
		expectedType    TokenType
		expectedLiteral string
	}{
		{IDENT, "x"},
		{ASSIGN, "="},
		{INT, "5"},
		// newline skipped
		{PLUS, "+"},
		{MINUS, "-"},
		{SLASH, "/"},
		{ASTERISK, "*"},
		{FUNGSI, "fungsi"},
		{IDENT, "tambah"},
		{LPAREN, "("},
		{IDENT, "x"},
		{COMMA, ","},
		{IDENT, "y"},
		{RPAREN, ")"},
		{LBRACE, "{"},
		{KEMBALIKAN, "kembalikan"},
		{IDENT, "x"},
		{PLUS, "+"},
		{IDENT, "y"},
		{RBRACE, "}"},
		// comment skipped
		{COMMENT, "This is a comment"},
		{IDENT, "x"},
		{LT, "<"},
		{IDENT, "y"},
		{EOF, ""},
	}

	l := New(input)

	for i, tt := range tests {
		tok := l.NextToken()

		if tok.Type != tt.expectedType {
			t.Fatalf("tests[%d] - token type wrong. expected=%q, got=%q (literal %q)",
				i, tt.expectedType, tok.Type, tok.Literal)
		}

		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("tests[%d] - token literal wrong. expected=%q, got=%q",
				i, tt.expectedLiteral, tok.Literal)
		}
	}
}

func TestTokenPosition(t *testing.T) {
	input := `a
b
  c`
	// Line 1: 'a' (col 1)
	// Line 2: 'b' (col 1)
	// Line 3: 'c' (col 3)

	tests := []struct {
		expectedLiteral string
		expectedLine    int
		expectedColumn  int
	}{
		{"a", 1, 1},
		{"b", 2, 1},
		{"c", 3, 3},
	}

	l := New(input)

	for i, tt := range tests {
		tok := l.NextToken()
		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("tests[%d] literal wrong. expected=%q, got=%q", i, tt.expectedLiteral, tok.Literal)
		}
		if tok.Line != tt.expectedLine {
			t.Fatalf("tests[%d] line wrong. expected=%d, got=%d", i, tt.expectedLine, tok.Line)
		}
		if tok.Column != tt.expectedColumn {
			t.Fatalf("tests[%d] column wrong. expected=%d, got=%d", i, tt.expectedColumn, tok.Column)
		}
	}
}

func TestNextTokenExtended(t *testing.T) {
	input := `
10 == 10
10 != 9
10 <= 20
20 >= 10
"hello"
"hello\nworld"
"foo\"bar"
`
	tests := []struct {
		expectedType    TokenType
		expectedLiteral string
	}{
		{INT, "10"},
		{EQ, "=="},
		{INT, "10"},
		{INT, "10"},
		{NOT_EQ, "!="},
		{INT, "9"},
		{INT, "10"},
		{LTE, "<="},
		{INT, "20"},
		{INT, "20"},
		{GTE, ">="},
		{INT, "10"},
		{STRING, "hello"},
		{STRING, "hello\nworld"},
		{STRING, "foo\"bar"},
		{EOF, ""},
	}

	l := New(input)

	for i, tt := range tests {
		tok := l.NextToken()

		if tok.Type != tt.expectedType {
			t.Fatalf("tests[%d] - token type wrong. expected=%q, got=%q (literal %q)",
				i, tt.expectedType, tok.Type, tok.Literal)
		}

		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("tests[%d] - token literal wrong. expected=%q, got=%q",
				i, tt.expectedLiteral, tok.Literal)
		}
	}
}
