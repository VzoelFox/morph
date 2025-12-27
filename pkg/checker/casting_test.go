package checker

import (
	"strings"
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
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
	input := `var f float = 5.5;`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestStrictMixing(t *testing.T) {
	// int + float should fail
	input := `var x float = 5 + 5.5;`
	errors, _ := checkWarn(input)
	if len(errors) == 0 {
		t.Error("Expected error for mixed types int + float")
	} else {
		if !strings.Contains(errors[0].Message, "Operator + not defined") {
			t.Errorf("Expected operator error, got %s", errors[0].Message)
		}
	}
}

func TestFloatMath(t *testing.T) {
	// float + float should succeed
	input := `var x float = 5.5 + 1.1;`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestCastingIntToFloat(t *testing.T) {
	// float(5) -> float
	input := `var x float = float(5);`
	errors, warnings := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
	if len(warnings) != 0 {
		t.Errorf("Expected no warnings for safe cast, got %v", warnings)
	}
}

func TestCastingFloatToInt(t *testing.T) {
	// int(5.5) -> int (Warning)
	input := `var x int = int(5.5);`
	errors, warnings := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
	if len(warnings) == 0 {
		t.Error("Expected warning for lossy cast float->int")
	} else {
		if !strings.Contains(warnings[0].Message, "Lossy conversion") {
			t.Errorf("Expected Lossy conversion warning, got %s", warnings[0].Message)
		}
	}
}

func TestCastingForbidden(t *testing.T) {
	// int("123") -> error
	input := `var x int = int("123");`
	errors, _ := checkWarn(input)
	if len(errors) == 0 {
		t.Error("Expected error for string->int cast")
	} else {
		if !strings.Contains(errors[0].Message, "Cannot convert type string to int") {
			t.Errorf("Expected Cannot convert error, got %s", errors[0].Message)
		}
	}
}

func TestCastingIdentity(t *testing.T) {
	// int(5) -> int
	input := `var x int = int(5);`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}
