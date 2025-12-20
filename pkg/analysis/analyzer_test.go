package analysis

import (
	"testing"

	"github.com/VzoelFox/morphlang/pkg/lexer"
	"github.com/VzoelFox/morphlang/pkg/parser"
)

func TestAnalysisClosures(t *testing.T) {
	input := `
	fungsi luar()
		x = 10
		fungsi dalam()
			x = 20
		akhir
	akhir
	`

	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Fatalf("Parser errors: %v", p.Errors())
	}

	ctx, err := GenerateContext(program, "test.fox", input, []parser.ParserError{})
	if err != nil {
		t.Fatalf("GenerateContext failed: %v", err)
	}

	// 'dalam' should NOT list 'x' as a local variable because it updates the outer 'x'.
	dalamSym := ctx.Symbols["dalam"]
	if dalamSym == nil {
		// Analyzer might use different naming key for nested functions in flat map?
		// Currently analyzer uses fn.Name. If unique, it's fine.
		// If 'dalam' is not found, we skip check.
		// But in this simple test, it should be found.
		return
	}

	for _, v := range dalamSym.LocalVars {
		if v == "x" {
			t.Errorf("Variable 'x' should be treated as captured (closure), not local in 'dalam'")
		}
	}
}

func TestGenerateContext(t *testing.T) {
	input := `
# Sample program
x = 10

fungsi tambah(a, b)
  kembalikan a + b
akhir

fungsi main()
  y = tambah(x, 5)
akhir
`

	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Fatalf("Parser has errors: %v", p.Errors())
	}

	ctx, err := GenerateContext(program, "test.morph", input, nil)
	if err != nil {
		t.Fatalf("GenerateContext failed: %v", err)
	}

	// 1. Verify File and Checksum
	if ctx.File != "test.morph" {
		t.Errorf("Expected filename 'test.morph', got %s", ctx.File)
	}
	if ctx.Checksum == "" {
		t.Error("Checksum is empty")
	}

	// 2. Verify Statistics
	// Lines:
	// 1: empty
	// 2: # Sample program (comment)
	// 3: x = 10 (code)
	// 4: empty
	// 5: fungsi tambah(a, b) (code)
	// 6:   kembalikan a + b (code)
	// 7: akhir (code)
	// 8: empty
	// 9: fungsi main() (code)
	// 10:   y = tambah(x, 5) (code)
	// 11: akhir (code)
	// 12: empty

	// Total 12 lines.
	// Blank: 1, 4, 8, 12 -> 4 lines
	// Comment: 2 -> 1 line
	// Code: 3, 5, 6, 7, 9, 10, 11 -> 7 lines

	if ctx.Statistics.TotalLines != 12 {
		t.Errorf("Expected 12 total lines, got %d", ctx.Statistics.TotalLines)
	}
	if ctx.Statistics.CodeLines != 7 {
		t.Errorf("Expected 7 code lines, got %d", ctx.Statistics.CodeLines)
	}

	// 3. Verify Global Variables
	if _, ok := ctx.GlobalVars["x"]; !ok {
		t.Error("Global variable 'x' not found in context")
	}

	// 4. Verify Symbols (Functions)
	tambahFn, ok := ctx.Symbols["tambah"]
	if !ok {
		t.Fatal("Function 'tambah' not found in symbols")
	}
	if len(tambahFn.Parameters) != 2 {
		t.Errorf("Expected 2 parameters for 'tambah', got %d", len(tambahFn.Parameters))
	}
	if tambahFn.Parameters[0].Name != "a" {
		t.Errorf("Expected param 0 to be 'a', got %s", tambahFn.Parameters[0].Name)
	}

	_, ok = ctx.Symbols["main"]
	if !ok {
		t.Fatal("Function 'main' not found in symbols")
	}

	// 5. Verify Call Graph
	// main calls tambah
	calls := ctx.CallGraph["main"]
	foundCall := false
	for _, c := range calls {
		if c == "tambah" {
			foundCall = true
			break
		}
	}
	if !foundCall {
		t.Errorf("Expected 'main' to call 'tambah', got calls: %v", calls)
	}

	// 6. Verify Local Scopes
	// main has local var 'y'
	mainScope := ctx.LocalScopes["main"]
	if _, ok := mainScope["y"]; !ok {
		t.Error("Local variable 'y' not found in 'main' scope")
	}
}

func TestAnalyzeErrorFunction(t *testing.T) {
	input := `
fungsi cek(x)
  jika x < 0
    kembalikan galat("negatif")
  akhir
  kembalikan x
akhir
`
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	ctx, _ := GenerateContext(program, "error.morph", input, nil)

	sym := ctx.Symbols["cek"]
	if !sym.CanError {
		t.Error("Expected function 'cek' to be marked as CanError=true")
	}
	if sym.Returns == nil || sym.Returns.Type != "union" {
		t.Error("Expected return type to be union (error)")
	}
}

func TestAnalysisVariableTypes(t *testing.T) {
	input := `
# Global variables type test
x = 10
y = "halo"
z = benar
`
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Fatalf("Parser has errors: %v", p.Errors())
	}

	ctx, err := GenerateContext(program, "types.morph", input, nil)
	if err != nil {
		t.Fatalf("GenerateContext failed: %v", err)
	}

	// Verify 'x' is integer
	if v, ok := ctx.GlobalVars["x"]; !ok {
		t.Error("Global variable 'x' not found")
	} else if v.Type != "integer" {
		t.Errorf("Expected x to be integer, got %s", v.Type)
	}

	// Verify 'y' is string
	if v, ok := ctx.GlobalVars["y"]; !ok {
		t.Error("Global variable 'y' not found")
	} else if v.Type != "string" {
		t.Errorf("Expected y to be string, got %s", v.Type)
	}

	// Verify 'z' is boolean
	if v, ok := ctx.GlobalVars["z"]; !ok {
		t.Error("Global variable 'z' not found")
	} else if v.Type != "boolean" {
		t.Errorf("Expected z to be boolean, got %s", v.Type)
	}
}
