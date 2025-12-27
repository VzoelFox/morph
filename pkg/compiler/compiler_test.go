package compiler

import (
	"strings"
	"testing"

	"github.com/VzoelFox/morph/pkg/checker"
	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

func TestCompileHelloWorld(t *testing.T) {
	input := `
	fungsi main() void
		native_print("Hello World")
	akhir
	main()
	`

	l := lexer.New(input)
	p := parser.New(l)
	prog := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Fatalf("Parser errors: %v", p.Errors())
	}

	c := checker.New()
	c.Check(prog)

	comp := New(c)
	code, err := comp.Compile(prog)
	if err != nil {
		t.Fatalf("Compiler error: %v", err)
	}

	t.Logf("Generated C Code:\n%s", code)

	// Basic assertions
	if !strings.Contains(code, "void mph_main(MorphContext* ctx, void* _env_void)") {
		t.Errorf("Expected function definition for main")
	}
	if !strings.Contains(code, "mph_native_print(ctx, mph_string_new(ctx, \"Hello World\"))") {
		t.Errorf("Expected native_print call with string construction")
	}
	if !strings.Contains(code, "void morph_entry_point(MorphContext* ctx)") {
		t.Errorf("Expected entry point")
	}
	if !strings.Contains(code, "mph_main(ctx, NULL)") {
		t.Errorf("Expected main() call in entry point")
	}
}
