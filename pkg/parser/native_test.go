package parser

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestNativeFunction(t *testing.T) {
	input := `
	fungsi Open(path string) File native
	fungsi Write(f File, s string) int native;
	`

	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	if len(program.Statements) != 2 {
		t.Fatalf("program.Statements does not contain 2 statements. got=%d",
			len(program.Statements))
	}

	tests := []struct {
		expectedName string
		expectedArgs int
	}{
		{"Open", 1},
		{"Write", 2},
	}

	for i, tt := range tests {
		stmt, ok := program.Statements[i].(*ExpressionStatement)
		if !ok {
			t.Fatalf("stmt not *ExpressionStatement. got=%T", program.Statements[i])
		}

		fn, ok := stmt.Expression.(*FunctionLiteral)
		if !ok {
			t.Fatalf("stmt.Expression not *FunctionLiteral. got=%T", stmt.Expression)
		}

		if fn.Name != tt.expectedName {
			t.Errorf("function name not '%s'. got=%s", tt.expectedName, fn.Name)
		}

		if len(fn.Parameters) != tt.expectedArgs {
			t.Errorf("function parameters wrong. want=%d, got=%d",
				tt.expectedArgs, len(fn.Parameters))
		}

		if !fn.IsNative {
			t.Errorf("function '%s' IsNative is false", fn.Name)
		}

		if fn.Body != nil {
			t.Errorf("function '%s' Body is not nil", fn.Name)
		}
	}
}
