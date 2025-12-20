package checker

import (
	"strings"
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
	if len(errors) == 0 {
		t.Error("Expected error for cyclic dependency")
	} else {
		if !strings.Contains(errors[0].Message, "Cyclic struct dependency") {
			t.Errorf("Expected cyclic dependency error, got %s", errors[0].Message)
		}
	}
}

func TestSelfCycle(t *testing.T) {
	input := `
	struktur A
		a A
	akhir
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for self cycle")
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
