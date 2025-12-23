package compiler

import (
	"strings"
	"testing"

	"github.com/VzoelFox/morph/pkg/checker"
	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestCompileInterpolation(t *testing.T) {
	input := `
	var name string = "Budi"
	var msg string = "Halo #{name}"
	`

	l := lexer.New(input)
	p := parser.New(l)
	prog := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Fatalf("Parser errors: %v", p.Errors())
	}

	c := checker.New()
	c.Check(prog)
	if len(c.Errors) > 0 {
		t.Fatalf("Checker errors: %v", c.Errors)
	}

	comp := New(c)
	code, err := comp.Compile(prog)
	if err != nil {
		t.Fatalf("Compiler error: %v", err)
	}

	if !strings.Contains(code, "mph_string_concat") {
		t.Errorf("Expected mph_string_concat call, got:\n%s", code)
	}
}
