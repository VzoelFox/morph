package checker

import (
	"testing"
	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestInterfaceImplementationLogic(t *testing.T) {
	// 1. Valid Implementation
	inputValid := `
antarmuka Speaker
    fungsi Speak() string
akhir

struktur Dog
    name string
akhir

fungsi (d Dog) Speak() string
    kembalikan "Woof"
akhir

var s Speaker = Dog{name: "Buddy"}
`
	runTest(t, inputValid, "Valid Interface Implementation", 0)

	// 2. Missing Method
	inputMissing := `
antarmuka Walker
    fungsi Walk() void
    fungsi Run() void
akhir

struktur Robot
    id int
akhir

fungsi (r Robot) Walk() void
    # Missing Run
akhir

var w Walker = Robot{id: 1}
`
	runTest(t, inputMissing, "Missing Method", 1) // Expected 1 error

	// 3. Wrong Signature
	inputSignature := `
antarmuka Calculator
    fungsi Add(a int, b int) int
akhir

struktur Casio
    model string
akhir

fungsi (c Casio) Add(a int, b int) float
    kembalikan 0.0 # Wrong return type
akhir

var cal Calculator = Casio{model: "FX"}
`
	runTest(t, inputSignature, "Wrong Signature", 1) // Expected 1 error
}

func runTest(t *testing.T, input, name string, expectedErrors int) {
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	c := New()
	c.Check(program)

	if len(c.Errors) != expectedErrors {
		t.Errorf("Test '%s': expected %d errors, got %d. errors: %v", name, expectedErrors, len(c.Errors), c.Errors)
	}
}
