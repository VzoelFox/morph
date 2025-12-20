package checker

import (
	"strings"
	"testing"
)

func TestShadowingWarning(t *testing.T) {
	input := `
	var x Int = 5
	jika true
		var x String = "hello"  # Should warn
	akhir
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
		var x Int = 1
	akhir
	jika true
		var x String = "a"  # Different scope, no shadow
	akhir
	`
	_, warnings := checkWarn(input)
	if len(warnings) != 0 {
		t.Errorf("Expected no warnings for separate scopes, got %v", warnings)
	}
}

func TestSameScopeRedeclaration(t *testing.T) {
	input := `
	var x Int = 1
	var x String = "hello"  # Same scope - ERROR
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
	var x Int = 1
	jika true
		var x String = "a"  # Shadow level 1
		jika true
			var x Bool = true  # Shadow level 2
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
    var x Int = 5;
    fungsi test(x Int)
        // x shadows global x
    akhir
    `
	_, warnings := checkWarn(input)
	if len(warnings) == 0 {
		t.Error("Expected shadowing warning for function parameter")
	}
}
