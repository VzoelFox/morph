package main

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/VzoelFox/morph/pkg/checker"
	"github.com/VzoelFox/morph/pkg/evaluator"
	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

// EvalImporter implements evaluator.Importer
type EvalImporter struct {
	base *FileImporter
	eval *evaluator.Evaluator
}

func (ei *EvalImporter) Import(path string) (*evaluator.Module, error) {
	// Reuse base parsing logic
	prog, err := ei.base.Import(path)
	if err != nil {
		return nil, err
	}

	modEnv := evaluator.NewEnvironment()
	// Eval module body
	ei.eval.Eval(prog, modEnv)

	return &evaluator.Module{Name: path, Env: modEnv}, nil
}

// FileImporter implements checker.Importer to load files from disk
type FileImporter struct {
	SearchPaths []string
}

func (fi *FileImporter) Import(path string) (*parser.Program, error) {
	// 1. Try Absolute Path
	if filepath.IsAbs(path) {
		return fi.parseFile(path)
	}

	// 2. Try Search Paths
	for _, root := range fi.SearchPaths {
		fullPath := filepath.Join(root, path)
		// Add extension if missing
		if filepath.Ext(fullPath) == "" {
			fullPath += ".fox"
		}

		if _, err := os.Stat(fullPath); err == nil {
			return fi.parseFile(fullPath)
		}
	}

	return nil, fmt.Errorf("module '%s' not found", path)
}

func (fi *FileImporter) parseFile(path string) (*parser.Program, error) {
	content, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}

	l := lexer.New(string(content))
	p := parser.New(l)
	prog := p.ParseProgram()

	if len(p.Errors()) > 0 {
		msg := "Parser errors in module " + path + ":\n"
		for _, err := range p.Errors() {
			msg += fmt.Sprintf("  %s\n", err.String())
		}
		return nil, fmt.Errorf("%s", msg)
	}

	return prog, nil
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf("Morph Compiler (Frontend)\n")
		fmt.Printf("Usage: morph <filename.fox>\n")
		os.Exit(1)
	}

	filename := os.Args[1]
	content, err := os.ReadFile(filename)
	if err != nil {
		fmt.Printf("Error reading file: %v\n", err)
		os.Exit(1)
	}

	fmt.Printf("Compiling %s...\n", filename)

	// 1. Lexing & Parsing
	l := lexer.New(string(content))
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) > 0 {
		fmt.Println("❌ Parser Errors:")
		for _, err := range p.Errors() {
			fmt.Println(err)
		}
		os.Exit(1)
	}

	// 2. Type Checking
	c := checker.New()

	// Setup Importer
	absPath, _ := filepath.Abs(filename)
	rootDir := filepath.Dir(absPath)

	// Default Search Paths:
	// 1. Directory of the input file
	// 2. ./stdlib (Standard Library)
	searchPaths := []string{rootDir, "stdlib"}

	c.SetImporter(&FileImporter{SearchPaths: searchPaths})

	c.Check(program)

	// Print Warnings
	if len(c.Warnings) > 0 {
		fmt.Println("⚠️  Warnings:")
		for _, warn := range c.Warnings {
			fmt.Printf("  [%d:%d] %s\n", warn.Line, warn.Column, warn.Message)
		}
	}

	// Print Errors
	if len(c.Errors) > 0 {
		fmt.Println("❌ Type Errors:")
		for _, err := range c.Errors {
			fmt.Printf("  [%d:%d] %s\n", err.Line, err.Column, err.Message)
		}
		os.Exit(1)
	}

	fmt.Println("✅ Validasi Sukses! (Syntax & Types OK)")

	// 3. Execution (Interpreter)
	fmt.Println("🚀 Executing...")

	evaluator.RegisterIO()
	evaluator.RegisterTime()
	evaluator.RegisterConv()

	env := evaluator.NewEnvironment()
	eval := evaluator.New(nil)

	fileImporter := &FileImporter{SearchPaths: searchPaths}
	evalImporter := &EvalImporter{
		base: fileImporter,
		eval: eval,
	}
	eval.SetImporter(evalImporter)

	result := eval.Eval(program, env)
	if result != nil && result.Type() == evaluator.ERROR_OBJ {
		fmt.Printf("❌ Runtime Error: %s\n", result.Inspect())
		os.Exit(1)
	}
}
