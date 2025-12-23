package checker

import (
	"strings"
	"testing"
)

func TestInterpolation(t *testing.T) {
	input := `
	var name String = "Budi"
	var msg String = "Halo #{name}"
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestInterpolationMismatch(t *testing.T) {
	input := `
	var name String = "Budi"
	var i Int = "Halo #{name}"
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for assigning Interpolated String to Int, but got none")
	} else {
		if !strings.Contains(errors[0].Message, "mismatch") {
			t.Errorf("Expected mismatch error, got %s", errors[0].Message)
		}
	}
}

func TestInterpolationNonString(t *testing.T) {
	input := `
	var i Int = 10
	var msg String = "Number #{i}"
	`
	// Strict check: Parts must be String
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for non-string interpolation part")
	}
}
