package checker

import "testing"

func TestInference(t *testing.T) {
	input := `var x = 5;`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestMultiReturnCheck(t *testing.T) {
	input := `
	fungsi f() (int, int)
		kembalikan 1, 2;
	akhir

	var a, b = f();
	`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestMultiAssignCheck(t *testing.T) {
	input := `
	var a = 1;
	var b = 2;
	a, b = 3, 4;
	`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestStdlibParseInt(t *testing.T) {
	input := `
	var val, err = parse_int("123");
	`
	errors, _ := checkWarn(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}
