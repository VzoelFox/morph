package checker

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestNativeFunctionCheck(t *testing.T) {
	input := `
	fungsi Open(path string) int native
	`
	l := lexer.New(input)
	p := parser.New(l)
	prog := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Fatalf("Parser errors: %v", p.Errors())
	}

	c := New()
	c.Check(prog)

	if len(c.Errors) > 0 {
		t.Errorf("Checker reported errors for native function: %v", c.Errors)
	}
}
