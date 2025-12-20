package parser

import (
	"strings"
	"testing"

	"github.com/VzoelFox/morphlang/pkg/lexer"
)

func TestInterpolatedString(t *testing.T) {
	input := `"Hello #{name}!"`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	if len(program.Statements) != 1 {
		t.Fatalf("program.Statements does not contain 1 statements. got=%d",
			len(program.Statements))
	}
	stmt, ok := program.Statements[0].(*ExpressionStatement)
	if !ok {
		t.Fatalf("stmt not ExpressionStatement")
	}

	is, ok := stmt.Expression.(*InterpolatedString)
	if !ok {
		t.Fatalf("exp not InterpolatedString. got=%T", stmt.Expression)
	}

	if len(is.Parts) != 3 {
		t.Fatalf("InterpolatedString parts wrong. want 3, got %d", len(is.Parts))
	}

	// Part 0: String "Hello "
	if sl, ok := is.Parts[0].(*StringLiteral); !ok || sl.Value != "Hello " {
		t.Errorf("Part 0 wrong. got %T %v", is.Parts[0], is.Parts[0])
	}
	// Part 1: Expr (Identifier "name")
	if ident, ok := is.Parts[1].(*Identifier); !ok || ident.Value != "name" {
		t.Errorf("Part 1 wrong. got %T %v", is.Parts[1], is.Parts[1])
	}
	// Part 2: String "!"
	if sl, ok := is.Parts[2].(*StringLiteral); !ok || sl.Value != "!" {
		t.Errorf("Part 2 wrong. got %T %v", is.Parts[2], is.Parts[2])
	}
}

func TestInterpolatedStringStarts(t *testing.T) {
	input := `"#{x}"`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	if len(program.Statements) != 1 {
		t.Fatalf("program.Statements does not contain 1 statements. got=%d",
			len(program.Statements))
	}
	stmt, ok := program.Statements[0].(*ExpressionStatement)
	if !ok {
		t.Fatalf("stmt not ExpressionStatement")
	}

	is, ok := stmt.Expression.(*InterpolatedString)
	if !ok {
		t.Fatalf("exp not InterpolatedString. got=%T", stmt.Expression)
	}

	if len(is.Parts) != 1 {
		t.Fatalf("InterpolatedString parts wrong. want 1, got %d", len(is.Parts))
	}

	// Part 0: Expr (Identifier "x")
	if ident, ok := is.Parts[0].(*Identifier); !ok || ident.Value != "x" {
		t.Errorf("Part 0 wrong. got %T %v", is.Parts[0], is.Parts[0])
	}
}

func TestStrictWhitespaceErrors(t *testing.T) {
	tests := []struct {
		input        string
		errorMsgPart string
	}{
		{"10+5", "requires space before"},
		{"10 +5", "requires space after"},
		{"10+ 5", "requires space before"},
		{"a==b", "requires space before"},
	}

	for _, tt := range tests {
		l := lexer.New(tt.input)
		p := New(l)
		p.ParseProgram()

		if len(p.Errors()) == 0 {
			t.Errorf("Expected syntax error for input %q, but got none", tt.input)
			continue
		}

		found := false
		for _, err := range p.Errors() {
			if strings.Contains(err.Message, tt.errorMsgPart) {
				found = true
				break
			}
		}

		if !found {
			t.Errorf("Expected error containing %q for input %q, got errors: %v",
				tt.errorMsgPart, tt.input, p.Errors())
		}
	}
}

func TestDetailedErrorFormatting(t *testing.T) {
	input := `x = 10 +` // Syntax error at end
	l := lexer.New(input)
	p := New(l)
	p.ParseProgram()

	if len(p.Errors()) == 0 {
		t.Fatalf("Expected errors")
	}

	err := p.Errors()[0]
	// Check format
	if !strings.Contains(err.String(), "ERROR [") {
		t.Errorf("Error format wrong: %s", err)
	}
}
