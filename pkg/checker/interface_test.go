package checker

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestInterfaceDefinition(t *testing.T) {
	input := `
# Definisikan tipe error kustom untuk pengujian
struktur MyError
    msg string
akhir

antarmuka Pembaca
    fungsi Baca(p []int) (int, MyError)
    fungsi Tutup() MyError
akhir
`
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()
	if len(p.Errors()) != 0 {
		t.Fatalf("Parser errors: %v", p.Errors())
	}

	c := New()
	c.Check(program)

	if len(c.Errors) != 0 {
		t.Fatalf("Checker errors: %v", c.Errors)
	}

	// Verifikasi bahwa tipe 'Pembaca' ada di scope
	typ, ok := c.scope.LookupType("Pembaca")
	if !ok {
		t.Fatalf("type 'Pembaca' not found in scope")
	}

	ifaceType, ok := typ.(*InterfaceType)
	if !ok {
		t.Fatalf("type 'Pembaca' is not an InterfaceType, got %T", typ)
	}

	if ifaceType.Name != "Pembaca" {
		t.Errorf("interface name is not 'Pembaca', got %s", ifaceType.Name)
	}

	if len(ifaceType.Methods) != 2 {
		t.Fatalf("expected 2 methods, got %d", len(ifaceType.Methods))
	}

	// Periksa metode 'Baca'
	bacaMethod, ok := ifaceType.Methods["Baca"]
	if !ok {
		t.Fatalf("method 'Baca' not found in interface")
	}

	if len(bacaMethod.Parameters) != 1 {
		t.Errorf("expected 1 parameter for 'Baca', got %d", len(bacaMethod.Parameters))
	} else {
		if bacaMethod.Parameters[0].String() != "[]int" {
			t.Errorf("expected param type '[]int', got %s", bacaMethod.Parameters[0].String())
		}
	}

	if len(bacaMethod.ReturnTypes) != 2 {
		t.Errorf("expected 2 return types for 'Baca', got %d", len(bacaMethod.ReturnTypes))
	} else {
		if !bacaMethod.ReturnTypes[0].Equals(IntType) {
			t.Errorf("expected return type 1 to be 'int', got %s", bacaMethod.ReturnTypes[0].String())
		}
		if bacaMethod.ReturnTypes[1].String() != "MyError" {
			t.Errorf("expected return type 2 to be 'MyError', got %s", bacaMethod.ReturnTypes[1].String())
		}
	}

	// Periksa metode 'Tutup'
	tutupMethod, ok := ifaceType.Methods["Tutup"]
	if !ok {
		t.Fatalf("method 'Tutup' not found in interface")
	}

	if len(tutupMethod.Parameters) != 0 {
		t.Errorf("expected 0 parameters for 'Tutup', got %d", len(tutupMethod.Parameters))
	}

	if len(tutupMethod.ReturnTypes) != 1 {
		t.Errorf("expected 1 return type for 'Tutup', got %d", len(tutupMethod.ReturnTypes))
	} else {
		if tutupMethod.ReturnTypes[0].String() != "MyError" {
			t.Errorf("expected return type to be 'MyError', got %s", tutupMethod.ReturnTypes[0].String())
		}
	}
}
