package checker

import (
	"strings"
	"testing"

	"github.com/VzoelFox/morphlang/pkg/lexer"
	"github.com/VzoelFox/morphlang/pkg/parser"
)

func checkWarn(input string) ([]TypeError, []TypeWarning) {
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	c := New()
	c.Check(program)
	return c.Errors, c.Warnings
}

func TestFloatLiteral(t *testing.T) {
	input := `var f Float = 5.5;`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestStrictMixing(t *testing.T) {
	// Int + Float should fail
	input := `var x Float = 5 + 5.5;`
	errors, _ := checkWarn(input)
	if len(errors) == 0 {
		t.Error("Expected error for mixed types Int + Float")
	} else {
		if !strings.Contains(errors[0].Message, "Operator + not defined") {
			t.Errorf("Expected operator error, got %s", errors[0].Message)
		}
	}
}

func TestFloatMath(t *testing.T) {
	// Float + Float should succeed
	input := `var x Float = 5.5 + 1.1;`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestCastingIntToFloat(t *testing.T) {
	// Float(5) -> Float
	input := `var x Float = Float(5);`
	errors, warnings := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
	if len(warnings) != 0 {
		t.Errorf("Expected no warnings for safe cast, got %v", warnings)
	}
}

func TestCastingFloatToInt(t *testing.T) {
	// Int(5.5) -> Int (Warning)
	input := `var x Int = Int(5.5);`
	errors, warnings := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
	if len(warnings) == 0 {
		t.Error("Expected warning for lossy cast Float->Int")
	} else {
		if !strings.Contains(warnings[0].Message, "Lossy conversion") {
			t.Errorf("Expected Lossy conversion warning, got %s", warnings[0].Message)
		}
	}
}

func TestCastingForbidden(t *testing.T) {
	// Int("123") -> Error
	input := `var x Int = Int("123");`
	errors, _ := checkWarn(input)
	if len(errors) == 0 {
		t.Error("Expected error for String->Int cast")
	} else {
		if !strings.Contains(errors[0].Message, "Cannot convert type String to Int") {
			t.Errorf("Expected Cannot convert error, got %s", errors[0].Message)
		}
	}
}

func TestCastingIdentity(t *testing.T) {
	// Int(5) -> Int
	input := `var x Int = Int(5);`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}
