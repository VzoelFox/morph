package parser

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestStructParsing(t *testing.T) {
	input := `
struktur Pengguna
    nama string
    umur int
akhir
`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	if len(program.Statements) != 1 {
		t.Fatalf("program.Statements does not contain 1 statement. got=%d",
			len(program.Statements))
	}

	stmt, ok := program.Statements[0].(*StructStatement)
	if !ok {
		t.Fatalf("stmt is not StructStatement. got=%T", program.Statements[0])
	}

	if stmt.Name.Value != "Pengguna" {
		t.Errorf("stmt.Name.Value not 'Pengguna'. got=%s", stmt.Name.Value)
	}

	if len(stmt.Fields) != 2 {
		t.Fatalf("len(stmt.Fields) not 2. got=%d", len(stmt.Fields))
	}

	if stmt.Fields[0].Name != "nama" {
		t.Errorf("Field 0 Name wrong. got=%s", stmt.Fields[0].Name)
	}
	if stmt.Fields[0].Type.String() != "string" {
		t.Errorf("Field 0 Type wrong. got=%s", stmt.Fields[0].Type.String())
	}

	if stmt.Fields[1].Name != "umur" {
		t.Errorf("Field 1 Name wrong. got=%s", stmt.Fields[1].Name)
	}
	if stmt.Fields[1].Type.String() != "int" {
		t.Errorf("Field 1 Type wrong. got=%s", stmt.Fields[1].Type.String())
	}
}
