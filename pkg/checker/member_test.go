package checker

import (
	"testing"
)

func TestMemberExpression(t *testing.T) {
	input := `
    struktur User
        id int
        name string
    akhir

    var u = User{id: 1, name: "Test"};
    var x int = u.id;
    var y string = u.name;
    `
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestMemberExpressionError(t *testing.T) {
	input := `
    struktur User
        id int
    akhir

    var u = User{id: 1};
    var x = u.unknown_field;
    `
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for unknown field")
	} else {
		expected := "Field or method 'unknown_field' not found in struct 'User'"
		if errors[0].Message != expected {
			t.Errorf("Expected error '%s', got '%s'", expected, errors[0].Message)
		}
	}
}

func TestMemberExpressionOnNonStruct(t *testing.T) {
	input := `
	var i = 10;
	var x = i.something;
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for member access on non-struct")
	}
}

func TestIndexStillWorks(t *testing.T) {
	input := `
    var arr = [1, 2, 3];
    var x = arr[0];

    var m = {"key": 42};
    var y = m["key"];
    `
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}
