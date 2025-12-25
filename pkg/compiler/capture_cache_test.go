package compiler

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/checker"
	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestGetFreeVarsCachesResult(t *testing.T) {
	input := "fungsi outer() int\n    kembali y\nakhir"
	l := lexer.New(input)
	p := parser.New(l)
	prog := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Fatalf("unexpected parser errors: %v", p.Errors())
	}

	var fn *parser.FunctionLiteral
	for _, stmt := range prog.Statements {
		if exprStmt, ok := stmt.(*parser.ExpressionStatement); ok {
			if literal, ok := exprStmt.Expression.(*parser.FunctionLiteral); ok {
				fn = literal
				break
			}
		}
	}
	if fn == nil {
		t.Fatal("expected function literal in program")
	}

	c := New(checker.New())
	c.currentGlobals = map[string]bool{}

	first := c.getFreeVars(fn)
	if len(first) != 1 || first[0] != "y" {
		t.Fatalf("expected free vars [y], got %v", first)
	}

	if _, ok := c.freeVarCache[fn]; !ok {
		t.Fatalf("expected free var cache entry after first call")
	}

	second := c.getFreeVars(fn)
	if len(second) != 1 || second[0] != "y" {
		t.Fatalf("expected cached free vars [y], got %v", second)
	}
	if &first[0] != &second[0] {
		t.Fatalf("expected cached slice reuse")
	}
}
