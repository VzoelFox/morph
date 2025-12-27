package parser

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestCommentHandling(t *testing.T) {
	tests := []struct {
		name     string
		input    string
		expected int // number of statements
	}{
		{
			name: "Comment at start",
			input: `# First comment
x = 5`,
			expected: 1,
		},
		{
			name: "Comment at end",
			input: `x = 5
# Last comment`,
			expected: 1,
		},
		{
			name: "Comment in between statements",
			input: `x = 5
# Middle comment
y = 10`,
			expected: 2,
		},
		{
			name: "Comment inside expression",
			input: `x = 5 + # comment here
5`,
			expected: 1,
		},
		{
			name: "Multiple comments",
			input: `# start
x = 5 # inline (lexer handles this as part of line? No, lexer emits COMMENT token)
# middle
y = 10
# end`,
			expected: 2,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			l := lexer.New(tt.input)
			p := New(l)
			program := p.ParseProgram()
			checkParserErrors(t, p)

			if len(program.Statements) != tt.expected {
				t.Errorf("program.Statements does not contain %d statements. got=%d",
					tt.expected, len(program.Statements))
			}
		})
	}
}
