package checker

import (
	"testing"
)

func TestArrayLiteralTypeCheck(t *testing.T) {
	input := `var arr []int = [1, 2, 3];`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestArrayLiteralMismatch(t *testing.T) {
	input := `var arr []int = [1, "wrong", 3];`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected type mismatch error")
	} else {
		expected := "Array element 1 type mismatch: expected int, got string"
		if errors[0].Message != expected {
			t.Errorf("Expected '%s', got '%s'", expected, errors[0].Message)
		}
	}
}

func TestArrayLiteralInference(t *testing.T) {
	// Should infer []int
	input := `var arr = [1, 2, 3];`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestMapLiteralCheck(t *testing.T) {
	input := `var m map[string]int = {"a": 1, "b": 2};`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestMapLiteralKeyMismatch(t *testing.T) {
	input := `var m = {"a": 1, 2: 2};`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected key mismatch error")
	} else {
		expected := "Map key type mismatch: expected string, got int"
		// Since map iteration order is random, we might catch either "a":1 first or 2:2 first.
		// But in this case, "a":1 is string Key, 2:2 is int Key.
		// If "a":1 is processed first -> expected string, got int.
		// If 2:2 is processed first -> expected int, got string.
		// Wait, Go map iteration order is random.
		// So the error message might vary.
		// We should accept either direction of mismatch.
		if errors[0].Message != expected && errors[0].Message != "Map key type mismatch: expected int, got string" {
			t.Errorf("Unexpected error message: %s", errors[0].Message)
		}
	}
}

func TestMapLiteralValueMismatch(t *testing.T) {
	input := `var m = {"a": 1, "b": "string"};`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected value mismatch error")
	}
}
