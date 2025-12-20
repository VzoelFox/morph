package parser

import (
	"testing"
	"github.com/VzoelFox/morphlang/pkg/lexer"
)

func TestString(t *testing.T) {
	program := &Program{
		Statements: []Statement{
			&AssignmentStatement{
				Token: lexer.Token{Type: lexer.IDENT, Literal: "myVar"},
				Name: &Identifier{
					Token: lexer.Token{Type: lexer.IDENT, Literal: "myVar"},
					Value: "myVar",
				},
				Value: &Identifier{
					Token: lexer.Token{Type: lexer.IDENT, Literal: "anotherVar"},
					Value: "anotherVar",
				},
			},
		},
	}

	if program.String() != "myVar = anotherVar;" {
		t.Errorf("program.String() wrong. got=%q", program.String())
	}
}

func TestExpressionsString(t *testing.T) {
	// -a * b
	expr := &InfixExpression{
		Token: lexer.Token{Type: lexer.ASTERISK, Literal: "*"},
		Left: &PrefixExpression{
			Token: lexer.Token{Type: lexer.MINUS, Literal: "-"},
			Operator: "-",
			Right: &Identifier{
				Token: lexer.Token{Type: lexer.IDENT, Literal: "a"},
				Value: "a",
			},
		},
		Operator: "*",
		Right: &Identifier{
			Token: lexer.Token{Type: lexer.IDENT, Literal: "b"},
			Value: "b",
		},
	}

	if expr.String() != "((-a) * b)" {
		t.Errorf("expr.String() wrong. got=%q", expr.String())
	}
}

func TestIfExpressionString(t *testing.T) {
	// jika x { y }
	expr := &IfExpression{
		Token: lexer.Token{Type: lexer.JIKA, Literal: "jika"},
		Condition: &Identifier{Value: "x"},
		Consequence: &BlockStatement{
			Statements: []Statement{
				&ExpressionStatement{
					Expression: &Identifier{Value: "y"},
				},
			},
		},
	}

	// Expect "jika x y akhir" because BlockStatement.String() doesn't add braces,
	// and IfExpression.String() adds " akhir".
	expected := "jika x y akhir"
	if expr.String() != expected {
		t.Errorf("expr.String() wrong. got=%q, want=%q", expr.String(), expected)
	}
}

func TestReturnStatementString(t *testing.T) {
	stmt := &ReturnStatement{
		Token: lexer.Token{Type: lexer.KEMBALIKAN, Literal: "kembalikan"},
		ReturnValue: &Identifier{Value: "x"},
	}

	if stmt.String() != "kembalikan x;" {
		t.Errorf("stmt.String() wrong. got=%q", stmt.String())
	}
}
