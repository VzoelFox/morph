package checker

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func runConstTest(t *testing.T, input string, expectedErrors int) {
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Logf("Parser errors: %v", p.Errors())
	}

	c := New()
	c.Check(program)

	if len(c.Errors) != expectedErrors {
		t.Errorf("Input:\n%s\nExpected %d errors, got %d. errors: %v", input, expectedErrors, len(c.Errors), c.Errors)
	}
}

func TestConstImmutability(t *testing.T) {
	// 1. Valid Const
	input1 := `
	tetapan PI = 3.14
	var r = 10.0
	var area = PI * r * r
	`
	runConstTest(t, input1, 0)

	// 2. Reassignment error
	input2 := `
	tetapan PI = 3.14
	PI = 3.14159
	`
	runConstTest(t, input2, 1)

	// 3. Shadowing (Allowed)
	input3 := `
	tetapan X = 10
	fungsi test()
		var X = 20
	akhir
	`
	runConstTest(t, input3, 0)
}

func TestConstWithoutValue(t *testing.T) {
	// Parser should reject this
	input := `
	tetapan PI int;
	`
	l := lexer.New(input)
	p := parser.New(l)
	p.ParseProgram()

	if len(p.Errors()) == 0 {
		t.Error("Expected parser error for const without value")
	}
}
