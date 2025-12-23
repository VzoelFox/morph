package checker

import (
	"strings"
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestBitwiseOps(t *testing.T) {
	input := `
	var a int = 10 & 2;
	var b int = 10 | 2;
	var c int = 10 ^ 2;
	var d int = 10 << 2;
	var e int = 10 >> 2;
	var f int = ~10;
	`
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	c := New()
	c.Check(program)

	if len(c.Errors) != 0 {
		t.Errorf("Expected no errors, got %d", len(c.Errors))
		for _, err := range c.Errors {
			t.Logf("Error: %s", err.Message)
		}
	}
}

func TestBitwiseErrors(t *testing.T) {
	input := `
	var a int = 10 & "2";
	var b int = 3.14 | 2;
	var c int = 3.14 ^ 3.14;
	var d int = "s" << 2;
	var e int = 10 >> "s";
	var f int = ~3.14;
	`
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	c := New()
	c.Check(program)

	expectedErrors := []string{
		"Bitwise operator & requires int operands",
		"Bitwise operator | requires int operands",
		"Bitwise operator ^ requires int operands",
		"Bitwise operator << requires int operands",
		"Bitwise operator >> requires int operands",
		"Operator ~ requires int",
	}

	if len(c.Errors) != len(expectedErrors) {
		t.Errorf("Expected %d errors, got %d", len(expectedErrors), len(c.Errors))
		for _, err := range c.Errors {
			t.Logf("Error: %s", err.Message)
		}
	}

	for i, expected := range expectedErrors {
		if i < len(c.Errors) {
			if !strings.Contains(c.Errors[i].Message, expected) {
				t.Errorf("Error %d mismatch. Expected '%s', got '%s'", i, expected, c.Errors[i].Message)
			}
		}
	}
}
