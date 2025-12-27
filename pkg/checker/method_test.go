package checker

import (
	"testing"
)

func TestMethodDefinitionAndCall(t *testing.T) {
	input := `
	struktur User
		id int
	akhir

	fungsi (u User) getId() int
		kembalikan u.id;
	akhir

	var u = User{id: 5};
	var x = u.getId();
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestMethodReceiverAccess(t *testing.T) {
	input := `
	struktur User
		id int
	akhir

	fungsi (u User) weird() int
		var x = u.id;
		kembalikan x + 1;
	akhir
	`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors, got %v", errors)
	}
}

func TestMethodNotFound(t *testing.T) {
	input := `
	struktur User
		id int
	akhir

	var u = User{id: 5};
	var x = u.unknownMethod();
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for unknown method")
	} else {
		expected := "Field or method 'unknownMethod' not found in struct 'User'"
		if errors[0].Message != expected {
			t.Errorf("Expected '%s', got '%s'", expected, errors[0].Message)
		}
	}
}

func TestMethodGlobalScopeIsolation(t *testing.T) {
	input := `
	struktur User
		id int
	akhir

	fungsi (u User) getId() int
		kembalikan u.id;
	akhir

	// getId should NOT be in global scope
	var x = getId();
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for calling method as global function")
	}
}

func TestMethodOnNonStruct(t *testing.T) {
	input := `
	fungsi (i int) test() {}
	`
	errors := check(input)
	if len(errors) == 0 {
		t.Error("Expected error for method on non-struct")
	}
}
