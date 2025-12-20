package checker

import (
	"testing"
)

func TestArrayHomogeneity(t *testing.T) {
	tests := []struct {
		name    string
		input   string
		wantErr bool
	}{
		{
			name:    "homogeneous int array",
			input:   `var arr = [1, 2, 3]`,
			wantErr: false,
		},
		{
			name:    "mixed int and string",
			input:   `var arr = [1, "two", 3]`,
			wantErr: true,
		},
		{
			name:    "mixed int and float",
			input:   `var arr = [1, 2.5, 3]`,
			wantErr: true,
		},
		{
			name:    "all strings",
			input:   `var arr = ["a", "b", "c"]`,
			wantErr: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			errors := check(tt.input)
			if tt.wantErr && len(errors) == 0 {
				t.Error("Expected error but got none")
			}
			if !tt.wantErr && len(errors) != 0 {
				t.Errorf("Expected no errors, got %v", errors)
			}
		})
	}
}

func TestMapConsistency(t *testing.T) {
	tests := []struct {
		name    string
		input   string
		wantErr bool
	}{
		{
			name:    "consistent map",
			input:   `var m = {"a": 1, "b": 2}`,
			wantErr: false,
		},
		{
			name:    "inconsistent value types",
			input:   `var m = {"a": 1, "b": "two"}`,
			wantErr: true,
		},
		{
			name:    "inconsistent key types",
			input:   `var m = {1: "a", "2": "b"}`,
			wantErr: true,
		},
		{
			name:    "all consistent",
			input:   `var m = {1: 10, 2: 20, 3: 30}`,
			wantErr: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			errors := check(tt.input)
			if tt.wantErr && len(errors) == 0 {
				t.Error("Expected error but got none")
			}
			if !tt.wantErr && len(errors) != 0 {
				t.Errorf("Expected no errors, got %v", errors)
			}
		})
	}
}

func TestNestedLiterals(t *testing.T) {
	input := `var nested = [[1, 2], [3, 4]]`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors for nested arrays, got %v", errors)
	}
}

func TestMapOfArrays(t *testing.T) {
	input := `var m = {"a": [1, 2], "b": [3, 4]}`
	errors := check(input)
	if len(errors) != 0 {
		t.Errorf("Expected no errors for map of arrays, got %v", errors)
	}
}
