package parser

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestStructLiteralParsing(t *testing.T) {
	input := `
	var u User = User{id: 1, name: "Budi"};
	`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	if len(program.Statements) != 1 {
		t.Fatalf("Expected 1 statement, got %d", len(program.Statements))
	}

	stmt, ok := program.Statements[0].(*VarStatement)
	if !ok {
		t.Fatalf("Expected VarStatement, got %T", program.Statements[0])
	}

	lit, ok := stmt.Values[0].(*StructLiteral)
	if !ok {
		t.Fatalf("Expected StructLiteral, got %T", stmt.Values[0])
	}

	if lit.Name.String() != "User" {
		t.Errorf("Expected struct name User, got %s", lit.Name.String())
	}

	if len(lit.Fields) != 2 {
		t.Errorf("Expected 2 fields, got %d", len(lit.Fields))
	}

	// Use testLiteralExpression helper if available (it is in parser_test.go, but not exported).
	// Since we are in same package 'parser', we can access it if 'parser_test.go' is compiled.
	// Yes, 'go test ./pkg/parser' compiles all *_test.go files.
	if !testLiteralExpression(t, lit.Fields["id"], 1) {
		t.Error("Field 'id' incorrect")
	}
	if !testLiteralExpression(t, lit.Fields["name"], "Budi") {
		t.Error("Field 'name' incorrect")
	}
}
