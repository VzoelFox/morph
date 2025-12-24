package checker

import (
	"testing"
)

func TestCyclicStruct(t *testing.T) {
	input := `
	struktur A
		b B
	akhir
	struktur B
		a A
	akhir
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Structs are reference types, should allow cycles. Got: %v", errors)
	}
}

func TestSelfCycle(t *testing.T) {
	input := `
	struktur A
		a A
	akhir
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Structs are reference types, should allow cycles. Got: %v", errors)
	}
}

func TestNoCycleArray(t *testing.T) {
	// Arrays/Slices break cycles (references)
	input := `
	struktur Node
		children []Node
	akhir
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors for array cycle, got %v", errors)
	}
}
