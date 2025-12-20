package lexer

import (
	"testing"
)

func TestStringInterpolation(t *testing.T) {
	input := `"Hello #{name}!"`

	tests := []struct {
		expectedType    TokenType
		expectedLiteral string
	}{
		{STRING, "Hello "},
		{INTERP_START, "#{"},
		{IDENT, "name"},
		{RBRACE, "}"},
		{STRING, "!"},
		{EOF, ""},
	}

	l := New(input)

	for i, tt := range tests {
		tok := l.NextToken()

		if tok.Type != tt.expectedType {
			t.Fatalf("tests[%d] - token type wrong. expected=%q, got=%q",
				i, tt.expectedType, tok.Type)
		}

		if tok.Literal != tt.expectedLiteral {
			t.Fatalf("tests[%d] - token literal wrong. expected=%q, got=%q",
				i, tt.expectedLiteral, tok.Literal)
		}
	}
}

func TestNestedInterpolation(t *testing.T) {
	input := `"A #{ "B #{c}" } D"`

	tests := []struct {
		expectedType    TokenType
		expectedLiteral string
	}{
		{STRING, "A "},
		{INTERP_START, "#{"},
		// Inner string starts
		{STRING, "B "},
		{INTERP_START, "#{"},
		{IDENT, "c"},
		{RBRACE, "}"},
		// Inner string ends, Lexer recurses on closing quote, next token is outer RBRACE
		{RBRACE, "}"},
		{STRING, " D"},
		// Outer string ends, Lexer recurses, next is EOF
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

func TestHasLeadingSpace(t *testing.T) {
	input := `a + b
a+b
-5
- 5`

	tests := []struct {
		literal  string
		hasSpace bool
	}{
		{"a", false}, // Start of file
		{"+", true},
		{"b", true},
		{"a", true},  // Newline is whitespace
		{"+", false}, // No space
		{"b", false}, // No space
		{"-", true},  // Newline
		{"5", false}, // No space
		{"-", true},  // Newline
		{"5", true},  // Space
	}

	l := New(input)

	for i, tt := range tests {
		tok := l.NextToken()

		if tok.Literal != tt.literal {
			t.Fatalf("tests[%d] - literal wrong. expected=%q, got=%q",
				i, tt.literal, tok.Literal)
		}

		if tok.HasLeadingSpace != tt.hasSpace {
			t.Fatalf("tests[%d] - hasSpace wrong for %q. expected=%v, got=%v",
				i, tt.literal, tt.hasSpace, tok.HasLeadingSpace)
		}
	}
}
