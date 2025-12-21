package checker

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func runPointerTest(t *testing.T, input string, expectedErrors int) {
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Logf("Parser errors: %v", p.Errors())
	}

	c := New()
	c.Check(program)

	if len(c.Errors) != expectedErrors {
		t.Errorf("Input:\n%s\nExpected %d errors, got %d. Errors: %v", input, expectedErrors, len(c.Errors), c.Errors)
	}
}

func TestPointers(t *testing.T) {
	// 1. Basic Pointer Declaration
	input1 := `
	var x Int = 10
	var p *Int = &x
	var y Int = *p
	`
	runPointerTest(t, input1, 0)

	// 2. Dereference
	input2 := `
	var x Int = 10
	var p *Int = &x
	var z Int = *p
	`
	runPointerTest(t, input2, 0)

	// 3. Null assignment
	input3 := `
	var p *Int = kosong
	`
	runPointerTest(t, input3, 0)

	// 4. Struct Null assignment (Should Fail now)
	input4 := `
	struktur User
		id Int
	akhir
	var u User = kosong
	`
	runPointerTest(t, input4, 1)

	// 5. Pointer to Struct Null assignment (Should Pass)
	input5 := `
	struktur User
		id Int
	akhir
	var u *User = kosong
	`
	runPointerTest(t, input5, 0)

	// 6. Invalid AddressOf
	input6 := `
	var p *Int = &10
	`
	runPointerTest(t, input6, 1)
}
