package checker

import (
	"testing"
)

func TestShadowingWarning(t *testing.T) {
	input := `
    var x Int = 5;
    jika true
        var x String = "hello";
    akhir
    `
	_, warnings := checkWarn(input)
	if len(warnings) == 0 {
		t.Error("Expected shadowing warning")
	} else {
		expected := "Variable 'x' shadows outer scope"
		if warnings[0].Message != expected {
			t.Errorf("Expected warning '%s', got '%s'", expected, warnings[0].Message)
		}
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

func TestNoShadowing(t *testing.T) {
	input := `
    var x Int = 5;
    jika true
        var y String = "hello";
    akhir
    `
	_, warnings := checkWarn(input)
	if len(warnings) != 0 {
		t.Errorf("Expected no warnings, got %v", warnings)
	}
}
