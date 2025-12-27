package parser

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestMethodParsing(t *testing.T) {
	input := `
	fungsi (u User) getId() int
		kembalikan u.id;
	akhir
	`
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
		t.Fatalf("program.Statements[0] is not ExpressionStatement. got=%T",
			program.Statements[0])
	}

	fn, ok := stmt.Expression.(*FunctionLiteral)
	if !ok {
		t.Fatalf("stmt.Expression is not FunctionLiteral. got=%T", stmt.Expression)
	}

	if fn.Name != "getId" {
		t.Errorf("function name wrong. expected='getId', got='%s'", fn.Name)
	}

	if fn.Receiver == nil {
		t.Fatalf("function receiver is nil")
	}

	if fn.Receiver.Name.Value != "u" {
		t.Errorf("receiver name wrong. expected='u', got='%s'", fn.Receiver.Name.Value)
	}

	if fn.Receiver.Type.String() != "User" {
		t.Errorf("receiver type wrong. expected='User', got='%s'", fn.Receiver.Type.String())
	}
}

func TestAnonymousFunctionParsing(t *testing.T) {
	// Verify regression
	input := `
	var f = fungsi(x int) int
		kembalikan x;
	akhir;
	`
	l := lexer.New(input)
	p := New(l)
	p.ParseProgram() // program unused, just checking for errors
	checkParserErrors(t, p)
}
