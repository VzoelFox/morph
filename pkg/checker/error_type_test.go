package checker

import (
	"testing"
	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestErrorTypeUsage(t *testing.T) {
	// Test: Declaration, Null Assignment, Function Return
	input := `
    # 1. Declaration & Init with Null (kosong)
    var e Error = kosong

    # 3. Function returning Error
    fungsi mightFail(x Int) (Int, Error)
        jika x < 0
            # Return variable of type Error
            kembalikan 0, e
        akhir
        # Return literal null (kosong)
        kembalikan x, kosong
    akhir

    var val, err = mightFail(10)
    `

	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

    if len(p.Errors()) > 0 {
        t.Fatalf("Parser errors: %v", p.Errors())
    }

	c := New()
	c.Check(program)

    if len(c.Errors) > 0 {
        t.Fatalf("Expected no errors, got: %v", c.Errors)
    }
}
