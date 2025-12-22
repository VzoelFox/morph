package parser

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestCallExpressionParsing_FunctionContext(t *testing.T) {
	// Case 3: Explicit Void (Should pass)
	runTest(t, "fungsi main() Void io.Write(a, b) akhir", "Case 3")
}

func runTest(t *testing.T, input string, name string) {
	l := lexer.New(input)
	p := New(l)
	p.ParseProgram()
	if len(p.Errors()) > 0 {
		t.Errorf("%s failed: %v", name, p.Errors())
	}
}
