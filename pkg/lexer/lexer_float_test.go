package lexer

import (
	"testing"
)

func TestFloatTokens(t *testing.T) {
	input := `
123
123.45
0.5
.5
123.
1.method
`
	tests := []struct {
		expectedType    TokenType
		expectedLiteral string
	}{
		{INT, "123"},
		{FLOAT, "123.45"},
		{FLOAT, "0.5"},
		{DOT, "."},
		{INT, "5"},
		{INT, "123"},
		{DOT, "."},
		{INT, "1"},
		{DOT, "."},
		{IDENT, "method"},
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

func TestEmptyString(t *testing.T) {
	input := `""`
	l := New(input)
	tok := l.NextToken()
	if tok.Type != STRING || tok.Literal != "" {
		t.Fatalf("expected empty string, got %q (%q)", tok.Type, tok.Literal)
	}
}
