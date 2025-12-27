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

func TestUnusedInLoop(t *testing.T) {
	input := `
	fungsi test()
		var i = 0;
		selama i < 2
			var s = "loop";
			i = i + 1;
		akhir
	akhir
	`
	_, warnings := checkWarn(input)
	foundS := false
	for _, w := range warnings {
		if strings.Contains(w.Message, "Unused variable 's'") {
			foundS = true
		}
	}
	if !foundS {
		t.Error("Expected unused variable warning for 's'")
	}
}

func TestUnusedTupleValue(t *testing.T) {
	input := `
	fungsi may_fail(fail bool) (int, error)
		jika fail
			kembali 0, error("gagal");
		akhir
		kembali 1, kosong;
	akhir

	fungsi test()
		var val, err = may_fail(benar);
		jika err != kosong
			native_print_error(err);
		akhir
	akhir
	`
	_, warnings := checkWarn(input)
	foundVal := false
	for _, w := range warnings {
		if strings.Contains(w.Message, "Unused variable 'val'") {
			foundVal = true
		}
	}
	if !foundVal {
		t.Error("Expected unused variable warning for 'val'")
	}
}
