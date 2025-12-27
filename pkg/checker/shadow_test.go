package checker

import (
	"strings"
	"testing"
)

func TestShadowingWarning(t *testing.T) {
	input := `
	var x int = 5
	jika true
		var x string = "hello"  # Should warn
		x + "world"
	akhir
	x + 1
	`
	_, warnings := checkWarn(input)
	if len(warnings) == 0 {
		t.Error("Expected shadowing warning")
	} else {
		if !strings.Contains(warnings[0].Message, "shadows outer declaration") {
			t.Errorf("Expected shadow message, got %s", warnings[0].Message)
		}
	}
}

func TestNoShadowingNewScope(t *testing.T) {
	input := `
	jika true
		var x int = 1
		x + 1
	akhir
	jika true
		var x string = "a"  # Different scope, no shadow
		x + "b"
	akhir
	`
	_, warnings := checkWarn(input)
	if len(warnings) != 0 {
		t.Errorf("Expected no warnings for separate scopes, got %v", warnings)
	}
}

func TestSameScopeRedeclaration(t *testing.T) {
	input := `
	var x int = 1
	var x string = "hello"  # Same scope - ERROR
	`
	errors, _ := checkWarn(input)
	if len(errors) == 0 {
		t.Error("Expected error for same-scope redeclaration")
	} else {
		if !strings.Contains(errors[0].Message, "already declared") {
			t.Errorf("Expected 'already declared' error, got %s", errors[0].Message)
		}
	}
}

func TestNestedShadowing(t *testing.T) {
	input := `
	var x int = 1
	x + 1
	jika true
		var x string = "a"  # Shadow level 1
		x + "b"
		jika true
			var x bool = true  # Shadow level 2
			!x
		akhir
	akhir
	`
	_, warnings := checkWarn(input)
	if len(warnings) != 2 {
		t.Errorf("Expected 2 shadowing warnings, got %d", len(warnings))
	}
}

func TestShadowingInFunctionParam(t *testing.T) {
	input := `
    var x int = 5;
    fungsi test(x int)
        x + 1
    akhir
	x + 1
    `
	_, warnings := checkWarn(input)
	if len(warnings) == 0 {
		t.Error("Expected shadowing warning for function parameter")
	}
}
