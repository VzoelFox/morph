package checker

import (
	"strings"
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func check(input string) []TypeError {
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	// If parser has errors, they might cascade. But we ignore them here
	// assuming the test input is syntactically valid.
	// If syntax is invalid, Check might crash or behave weirdly, so we print them.
	if len(p.Errors()) > 0 {
		// Log parser errors?
	}

	c := New()
	c.Check(program)
	return c.Errors
}

func TestVarDeclaration(t *testing.T) {
	input := `
	var x int = 5;
	var y string = "hello";
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestVarTypeMismatch(t *testing.T) {
	input := `
	var x int = "hello";
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected type mismatch error")
	} else {
		if !strings.Contains(errors[0].Message, "Type mismatch") {
			t.Errorf("Expected Type mismatch error, got %s", errors[0].Message)
		}
	}
}

func TestBinaryOp(t *testing.T) {
	input := `
	var x int = 5 + 10;
	var y bool = 5 == 5;
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestBinaryOpMismatch(t *testing.T) {
	input := `
	var x int = 5 + "s";
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error")
	}
}

func TestFunctionCall(t *testing.T) {
	input := `
	fungsi add(a int, b int) int
		kembalikan a + b;
	akhir

	var res int = add(1, 2);
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestFunctionCallMismatch(t *testing.T) {
	input := `
	fungsi add(a int, b int) int
		kembalikan a + b;
	akhir

	add(1, "s");
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error")
	} else {
		if !strings.Contains(errors[0].Message, "type mismatch") {
			t.Errorf("Expected type mismatch error, got %s", errors[0].Message)
		}
	}
}

func TestStructType(t *testing.T) {
	input := `
	struktur User
		id int
	akhir
	var x User = 5;
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected mismatch User vs int")
	}
}

func TestReturnTypeMismatch(t *testing.T) {
	input := `
	fungsi add(a int, b int) int
		kembalikan "hello";
	akhir
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected return type mismatch")
	} else {
		if !strings.Contains(errors[0].Message, "Return type mismatch") {
			t.Errorf("Expected Return type mismatch error, got %s", errors[0].Message)
		}
	}
}

func TestStructLiteral(t *testing.T) {
	input := `
	struktur User
		id int
		name string
	akhir

	var u User = User{id: 1, name: "Budi"};
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestStructLiteralError(t *testing.T) {
	input := `
	struktur User
		id int
	akhir

	var u User = User{id: "s"};
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error")
	}
}

func TestFieldAccess(t *testing.T) {
	input := `
	struktur User
		id int
	akhir

	var u User = User{id: 1};
	var i int = u.id;
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestNullAssignment(t *testing.T) {
	input := `
	struktur User
		id int
	akhir

	var u User = kosong;
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Structs are reference types, should be nullable. Got: %v", errors)
	}
}

func TestControlFlow(t *testing.T) {
	input := `
	var x int = 10;
	jika x == 10
		x = 20;
	akhir
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestControlFlowError(t *testing.T) {
	input := `
	var x int = 10;
	jika x
		x = 20;
	akhir
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for non-bool condition")
	}
}

func TestNestedReturn(t *testing.T) {
	input := `
	fungsi outer() int
		fungsi inner() string
			kembalikan "hello";
		akhir
		kembalikan 1;
	akhir
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestNestedReturnError(t *testing.T) {
	input := `
	fungsi outer() int
		fungsi inner() string
			kembalikan 1;
		akhir
		kembalikan 1;
	akhir
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error in inner function return")
	}
}
