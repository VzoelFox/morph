package parser

import (
	"testing"
	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestInterfaceStatement(t *testing.T) {
	input := `
antarmuka Penulis
    fungsi Tulis(data []byte) (int, error)
    fungsi Tutup() error
akhir
`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	if len(program.Statements) != 1 {
		t.Fatalf("program.Statements does not contain 1 statement. got=%d", len(program.Statements))
	}

	stmt, ok := program.Statements[0].(*InterfaceStatement)
	if !ok {
		t.Fatalf("program.Statements[0] is not *InterfaceStatement. got=%T", program.Statements[0])
	}

	if stmt.Name.Value != "Penulis" {
		t.Errorf("stmt.Name.Value not 'Penulis'. got=%s", stmt.Name.Value)
	}

	if len(stmt.Methods) != 2 {
		t.Fatalf("interface should have 2 methods. got=%d", len(stmt.Methods))
	}

	// Test method 1: Tulis
	method1 := stmt.Methods[0]
	if method1.Name != "Tulis" {
		t.Errorf("method 1 name not 'Tulis'. got=%s", method1.Name)
	}
	if len(method1.Parameters) != 1 {
		t.Fatalf("method 1 should have 1 parameter. got=%d", len(method1.Parameters))
	}
	testParameter(t, method1.Parameters[0], "data", "[]byte")

	if len(method1.ReturnTypes) != 2 {
		t.Fatalf("method 1 should have 2 return types. got=%d", len(method1.ReturnTypes))
	}
	testTypeNode(t, method1.ReturnTypes[0], "int")
	testTypeNode(t, method1.ReturnTypes[1], "error")

	if method1.Body != nil {
		t.Errorf("method 1 body should be nil")
	}

	// Test method 2: Tutup
	method2 := stmt.Methods[1]
	if method2.Name != "Tutup" {
		t.Errorf("method 2 name not 'Tutup'. got=%s", method2.Name)
	}
	if len(method2.Parameters) != 0 {
		t.Fatalf("method 2 should have 0 parameters. got=%d", len(method2.Parameters))
	}
	if len(method2.ReturnTypes) != 1 {
		t.Fatalf("method 2 should have 1 return type. got=%d", len(method2.ReturnTypes))
	}
	testTypeNode(t, method2.ReturnTypes[0], "error")

	if method2.Body != nil {
		t.Errorf("method 2 body should be nil")
	}
}

func testParameter(t *testing.T, param *Parameter, expectedName, expectedType string) {
	if param.Name.Value != expectedName {
		t.Errorf("parameter name not '%s'. got=%s", expectedName, param.Name.Value)
	}
	if param.Type.String() != expectedType {
		t.Errorf("parameter type not '%s'. got=%s", expectedType, param.Type.String())
	}
}

func testTypeNode(t *testing.T, tn TypeNode, expected string) {
	if tn.String() != expected {
		t.Errorf("type string not '%s'. got=%s", expected, tn.String())
	}
}
