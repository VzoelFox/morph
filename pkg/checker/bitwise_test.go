package checker

import (
	"strings"
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestBitwiseOps(t *testing.T) {
	input := `
	var a Int = 10 & 2;
	var b Int = 10 | 2;
	var c Int = 10 ^ 2;
	var d Int = 10 << 2;
	var e Int = 10 >> 2;
	var f Int = ~10;
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
	var a Int = 10 & "2";
	var b Int = 3.14 | 2;
	var c Int = 3.14 ^ 3.14;
	var d Int = "s" << 2;
	var e Int = 10 >> "s";
	var f Int = ~3.14;
	`
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	c := New()
	c.Check(program)

	expectedErrors := []string{
		"Bitwise operator & requires Int operands",
		"Bitwise operator | requires Int operands",
		"Bitwise operator ^ requires Int operands",
		"Bitwise operator << requires Int operands",
		"Bitwise operator >> requires Int operands",
		"Operator ~ requires Int",
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
