package parser

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestVarStatement(t *testing.T) {
	tests := []struct {
		input        string
		expectedName string
		expectedType string
		expectedVal  interface{}
	}{
		{"var x int = 5;", "x", "int", 5},
		{"var y string = \"hello\";", "y", "string", "hello"},
		{"var z bool = benar;", "z", "bool", true},
		{"var arr []int = [1, 2];", "arr", "[]int", nil}, // Value check skipped for array literal for brevity
	}

	for _, tt := range tests {
		l := lexer.New(tt.input)
		p := New(l)
		program := p.ParseProgram()
		checkParserErrors(t, p)

		if len(program.Statements) != 1 {
			t.Fatalf("program.Statements does not contain 1 statements. got=%d",
				len(program.Statements))
		}

		stmt, ok := program.Statements[0].(*VarStatement)
		if !ok {
			t.Errorf("stmt not *VarStatement. got=%T", program.Statements[0])
			continue
		}

		if len(stmt.Names) != 1 {
			t.Errorf("stmt.Names length %d", len(stmt.Names))
			continue
		}

		if stmt.Names[0].Value != tt.expectedName {
			t.Errorf("stmt.Names[0].Value not %s. got=%s", tt.expectedName, stmt.Names[0].Value)
		}

		if stmt.Type.String() != tt.expectedType {
			t.Errorf("stmt.Type.String not %s. got=%s", tt.expectedType, stmt.Type.String())
		}

		if tt.expectedVal != nil {
			if len(stmt.Values) != 1 {
				t.Errorf("stmt.Values length %d", len(stmt.Values))
				continue
			}
			if !testLiteralExpression(t, stmt.Values[0], tt.expectedVal) {
				return
			}
		}
	}
}

func TestComplexTypes(t *testing.T) {
	input := `
var a []int = [];
var b map[string]int = {};
var c [][]String = [];
var d map[string]map[int]bool = {};
`
	l := lexer.New(input)
	p := New(l)
	program := p.ParseProgram()
	checkParserErrors(t, p)

	tests := []struct {
		expectedType string
	}{
		{"[]int"},
		{"map[string]int"},
		{"[][]String"},
		{"map[string]map[int]bool"},
	}

	for i, tt := range tests {
		stmt, ok := program.Statements[i].(*VarStatement)
		if !ok {
			t.Errorf("stmt %d not VarStatement", i)
			continue
		}
		if stmt.Type.String() != tt.expectedType {
			t.Errorf("type wrong. expected=%q, got=%q", tt.expectedType, stmt.Type.String())
		}
	}
}
