package analysis

import (
	"testing"

	"github.com/VzoelFox/morphlang/pkg/lexer"
	"github.com/VzoelFox/morphlang/pkg/parser"
)

func TestAnalysisClosures(t *testing.T) {
	// Use 'var' to avoid ambiguity with return type parsing
	input := `
	fungsi luar()
		var x Int = 10
		fungsi dalam() Void
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
var x Int = 10

fungsi tambah(a Int, b Int) Int
  kembalikan a + b
akhir

fungsi main()
  var y Int = tambah(x, 5)
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

	if ctx.File != "test.morph" {
		t.Errorf("Expected filename 'test.morph', got %s", ctx.File)
	}
	if ctx.Checksum == "" {
		t.Error("Checksum is empty")
	}

	if ctx.Statistics.TotalLines != 12 {
		t.Errorf("Expected 12 total lines, got %d", ctx.Statistics.TotalLines)
	}
	// Code lines count should remain similar, 7.
	if ctx.Statistics.CodeLines != 7 {
		t.Errorf("Expected 7 code lines, got %d", ctx.Statistics.CodeLines)
	}

	if _, ok := ctx.GlobalVars["x"]; !ok {
		t.Error("Global variable 'x' not found in context")
	}

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

	mainScope := ctx.LocalScopes["main"]
	if _, ok := mainScope["y"]; !ok {
		t.Error("Local variable 'y' not found in 'main' scope")
	}
}

func TestAnalyzeErrorFunction(t *testing.T) {
	input := `
fungsi cek(x Int)
  jika x < 0
    kembalikan galat("negatif")
  akhir
  kembalikan x
akhir
`
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Fatalf("Parser errors: %v", p.Errors())
	}

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
var x Int = 10
var y String = "halo"
var z Bool = benar
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

	if v, ok := ctx.GlobalVars["x"]; !ok {
		t.Error("Global variable 'x' not found")
	} else if v.Type != "Int" {
		t.Errorf("Expected x to be Int, got %s", v.Type)
	}

	if v, ok := ctx.GlobalVars["y"]; !ok {
		t.Error("Global variable 'y' not found")
	} else if v.Type != "String" {
		t.Errorf("Expected y to be String, got %s", v.Type)
	}

	if v, ok := ctx.GlobalVars["z"]; !ok {
		t.Error("Global variable 'z' not found")
	} else if v.Type != "Bool" {
		t.Errorf("Expected z to be Bool, got %s", v.Type)
	}
}

func TestAnalysisStructAndImport(t *testing.T) {
	input := `
ambil "std/math"

# Data pengguna
struktur Pengguna
    nama string
    umur int
akhir
`
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) > 0 {
		t.Fatalf("Parser errors: %v", p.Errors())
	}

	ctx, err := GenerateContext(program, "struct.fox", input, []parser.ParserError{})
	if err != nil {
		t.Fatalf("GenerateContext failed: %v", err)
	}

	if len(ctx.Imports) != 1 {
		t.Fatalf("Expected 1 import, got %d", len(ctx.Imports))
	}
	if ctx.Imports[0] != "std/math" {
		t.Errorf("Expected import 'std/math', got '%s'", ctx.Imports[0])
	}

	if len(ctx.Structs) != 1 {
		t.Fatalf("Expected 1 struct, got %d", len(ctx.Structs))
	}
	st, ok := ctx.Structs["Pengguna"]
	if !ok {
		t.Fatalf("Struct 'Pengguna' not found")
	}

	if st.Doc == "" {
		t.Errorf("Struct doc is empty")
	}

	if len(st.Fields) != 2 {
		t.Fatalf("Expected 2 fields, got %d", len(st.Fields))
	}
	if st.Fields[0].Name != "nama" || st.Fields[0].Type != "string" {
		t.Errorf("Field 0 mismatch: %v", st.Fields[0])
	}
}
