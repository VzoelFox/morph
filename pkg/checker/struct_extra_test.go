package checker

import (
	"strings"
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestStructConstructor(t *testing.T) {
	input := `
	struktur User
		id int
		name string
	akhir

	var u = User(1, "Alice");
	var u2 = User(2, "Bob");

	# error cases
	var err1 = User(1); # Missing arg
	var err2 = User(1, 2); # Type mismatch
	var err3 = User("Alice", 1); # Wrong order
	`

	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	c := New()
	c.Check(program)

	// We expect errors for err1, err2, err3
	// u and u2 should be fine.

	expectedErrors := []string{
		"Struct User constructor requires 2 arguments, got 1",
		"Argument 2 (name) type mismatch: expected string, got int",
		"Argument 1 (id) type mismatch: expected int, got string",
	}

	if len(c.Errors) != len(expectedErrors) {
		t.Errorf("Expected %d errors, got %d", len(expectedErrors), len(c.Errors))
		for _, err := range c.Errors {
			t.Logf("Error: %s", err.Message)
		}
	}

	for i, expected := range expectedErrors {
		if i < len(c.Errors) {
			if !strings.Contains(c.Errors[i].Message, expected) {
				t.Errorf("Error %d mismatch. Expected '%s', got '%s'", i, expected, c.Errors[i].Message)
			}
		}
	}

	// Verify u has correct type
	sym, ok := c.scope.LookupSymbol("u")
	if !ok {
		t.Fatalf("Symbol 'u' not found")
	}
	if sym.Type.String() != "User" {
		t.Errorf("Expected 'u' to be User, got %s", sym.Type.String())
	}
}

func TestStructComparability(t *testing.T) {
	input := `
	struktur Point
		x int
		y int
	akhir

	struktur Container
		data []int
	akhir

	struktur Invalid
		m map[string]int
	akhir

	var p1 = Point(1, 2);
	var p2 = Point(1, 2);
	var valid = p1 == p2;

	var c1 = Container{data: [1]};
	var c2 = Container{data: [1]};
	# This should fail
	var invalid1 = c1 == c2;

	var i1 = Invalid{m: {"a":1}};
	var i2 = Invalid{m: {"a":1}};
	# This should fail
	var invalid2 = i1 == i2;
	`

	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	c := New()
	c.Check(program)

	expectedErrors := []string{
		"Struct Container is not comparable",
		"Struct Invalid is not comparable",
	}

	if len(c.Errors) != len(expectedErrors) {
		t.Errorf("Expected %d errors, got %d", len(expectedErrors), len(c.Errors))
		for _, err := range c.Errors {
			t.Logf("Error: %s", err.Message)
		}
	}

	for i, expected := range expectedErrors {
		if i < len(c.Errors) {
			if !strings.Contains(c.Errors[i].Message, expected) {
				t.Errorf("Error %d mismatch. Expected '%s', got '%s'", i, expected, c.Errors[i].Message)
			}
		}
	}
}
