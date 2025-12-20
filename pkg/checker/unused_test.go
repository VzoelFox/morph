package checker

import (
	"strings"
	"testing"
)

func TestUnusedVariable(t *testing.T) {
	input := `
	fungsi test()
		var x = 1;
	akhir
	`
	_, warnings := checkWarn(input)
	if len(warnings) == 0 {
		t.Error("Expected unused variable warning")
	} else {
		if !strings.Contains(warnings[0].Message, "Unused variable") {
			t.Errorf("Expected unused variable warning, got %s", warnings[0].Message)
		}
	}
}

func TestUsedVariable(t *testing.T) {
	input := `
	fungsi test()
		var x = 1;
		var y = x;
	akhir
	`
	// y is unused, x is used.
	_, warnings := checkWarn(input)
	if len(warnings) == 0 {
		t.Error("Expected warning for y")
	}
	// Warning for y
	if !strings.Contains(warnings[0].Message, "Unused variable 'y'") {
		t.Errorf("Expected unused variable 'y', got %s", warnings[0].Message)
	}
	// Ensure no warning for x
	for _, w := range warnings {
		if strings.Contains(w.Message, "'x'") {
			t.Error("Variable 'x' should be marked as used")
		}
	}
}

func TestUsedInExpression(t *testing.T) {
	input := `
	fungsi test()
		var x = 1;
		var z = x + 1;
	akhir
	`
	// z unused, x used.
	_, warnings := checkWarn(input)
	foundX := false
	for _, w := range warnings {
		if strings.Contains(w.Message, "'x'") {
			foundX = true
		}
	}
	if foundX {
		t.Error("Variable 'x' should be used")
	}
}
