package checker

import (
	"testing"
)

func TestEmptyArrayWithType(t *testing.T) {
	input := `var a []int = []`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors for empty array with type, got %v", errors)
	}
}

func TestEmptyMapWithType(t *testing.T) {
	input := `var m map[string]int = {}`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors for empty map with type, got %v", errors)
	}
}

func TestEmptyArrayNoType(t *testing.T) {
	input := `var a = []`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for empty array without type annotation")
	}
}

func TestEmptyMapNoType(t *testing.T) {
	input := `var m = {}`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for empty map without type annotation")
	}
}

func TestEmptyArrayFunctionParam(t *testing.T) {
	input := `
	fungsi process(items []string)
		kembalikan
	akhir

	process([])
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors for empty array in function call, got %v", errors)
	}
}

func TestNestedEmptyArrays(t *testing.T) {
	input := `var nested [][]int = [[], []]`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors for nested empty arrays, got %v", errors)
	}
}
