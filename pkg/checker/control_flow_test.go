package checker

import (
	"testing"
)

func TestControlFlowMissingReturn(t *testing.T) {
	input := `
    fungsi test(x int) int
        jika x > 0
            kembalikan 1;
        akhir
        # Missing return here!
    akhir
    `
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected missing return error")
	} else {
		if errors[0].Message != "Not all code paths return a value" {
			t.Errorf("Expected 'Not all code paths return a value', got '%s'", errors[0].Message)
		}
	}
}

func TestControlFlowAllPathsReturn(t *testing.T) {
	input := `
    fungsi test(x int) int
        jika x > 0
            kembalikan 1;
        lainnya
            kembalikan 0;
        akhir
    akhir
    `
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestControlFlowNested(t *testing.T) {
	input := `
    fungsi test(x int) int
        jika x > 0
            jika x > 10
                kembalikan 2;
            lainnya
                kembalikan 1;
            akhir
        lainnya
            kembalikan 0;
        akhir
    akhir
    `
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestControlFlowVoidFunction(t *testing.T) {
	input := `
    fungsi test(x int)
        jika x > 0
            var y = 1;
        akhir
    akhir
    `
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors for void function, got %v", errors)
	}
}

func TestControlFlowWhileDoesntGuarantee(t *testing.T) {
	input := `
    fungsi test() int
        selama true
            kembalikan 1;
        akhir
    akhir
    `
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected missing return error - while doesn't guarantee execution")
	}
}

func TestControlFlowBlockReturn(t *testing.T) {
	input := `
    fungsi test(x int) int
        jika x > 0
            var y = x + 1;
            kembalikan y;
        lainnya
            kembalikan 0;
        akhir
    akhir
    `
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}
