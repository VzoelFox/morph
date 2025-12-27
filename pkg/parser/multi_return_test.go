package parser

import (
	"testing"
	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestMultiReturnParsing(t *testing.T) {
	input := `fungsi f() (int, string) kembalikan 1, "s"; akhir`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	stmt := program.Statements[0].(*ExpressionStatement)
	fn := stmt.Expression.(*FunctionLiteral)

	if len(fn.ReturnTypes) != 2 {
		t.Fatalf("Expect 2 return types, got %d", len(fn.ReturnTypes))
	}
	if fn.ReturnTypes[0].String() != "int" {
		t.Errorf("Type 0 not int")
	}
	if fn.ReturnTypes[1].String() != "string" {
		t.Errorf("Type 1 not string")
	}

	ret := fn.Body.Statements[0].(*ReturnStatement)
	if len(ret.ReturnValues) != 2 {
		t.Fatalf("Expect 2 return values, got %d", len(ret.ReturnValues))
	}
}

func TestMultiVarParsing(t *testing.T) {
	input := `var a, b = 1, 2;`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	stmt := program.Statements[0].(*VarStatement)
	if len(stmt.Names) != 2 {
		t.Fatalf("Names len %d", len(stmt.Names))
	}
	if stmt.Names[0].Value != "a" { t.Error("Name 0 != a") }
	if stmt.Names[1].Value != "b" { t.Error("Name 1 != b") }

	if stmt.Type != nil {
		t.Error("Type should be nil (inferred)")
	}

	if len(stmt.Values) != 2 {
		t.Fatalf("Values len %d", len(stmt.Values))
	}
}

func TestMultiAssignmentParsing(t *testing.T) {
	input := `a, b = 1, 2;`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	stmt := program.Statements[0].(*AssignmentStatement)
	if len(stmt.Names) != 2 {
		t.Fatalf("Names len %d", len(stmt.Names))
	}
	if len(stmt.Values) != 2 {
		t.Fatalf("Values len %d", len(stmt.Values))
	}
}
