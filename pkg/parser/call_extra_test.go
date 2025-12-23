package parser

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestCallExpressionParsing_FunctionContext(t *testing.T) {
	// Case 0: 0 args
	runTest(t, "fungsi main() void foo() akhir", "Case 0")

	// Case 1: 1 arg
	runTest(t, "fungsi main() void foo(a) akhir", "Case 1")

	// Case 2: 2 args
	runTest(t, "fungsi main() void foo(a, b) akhir", "Case 2")

	// Case 3: IO Write
	runTest(t, "fungsi main() void io.Write(a, b) akhir", "Case 3")
}

func runTest(t *testing.T, input string, name string) {
	l := lexer.New(input)
	p := New(l)
	p.ParseProgram()
	if len(p.Errors()) > 0 {
		t.Errorf("%s failed: %v", name, p.Errors())
	}
}
