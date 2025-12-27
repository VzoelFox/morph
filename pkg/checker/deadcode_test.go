package checker

import (
	"strings"
	"testing"
)

func TestDeadCode(t *testing.T) {
	input := `
	fungsi test()
		kembalikan;
		var x = 1;
	akhir
	`
	_, warnings := checkWarn(input)
	if len(warnings) == 0 {
		t.Error("Expected dead code warning")
	} else {
		if !strings.Contains(warnings[0].Message, "Unreachable code") {
			t.Errorf("Expected unreachable code warning, got %s", warnings[0].Message)
		}
	}
}

func TestNoDeadCode(t *testing.T) {
	input := `
	fungsi test()
		var x = 1;
		x + 1;
		kembalikan;
	akhir
	`
	_, warnings := checkWarn(input)
	if len(warnings) != 0 {
		t.Errorf("Expected no warnings, got %v", warnings)
	}
}
