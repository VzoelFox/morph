package main

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/VzoelFox/morph/pkg/checker"
	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

// FileImporter implements checker.Importer to load files from disk
type FileImporter struct {
	Root string
}

func (fi *FileImporter) Import(path string) (*parser.Program, error) {
	// Simple resolution: relative to Root or absolute
	fullPath := path
	if !filepath.IsAbs(path) {
		fullPath = filepath.Join(fi.Root, path)
		// Add extension if missing
		if filepath.Ext(fullPath) == "" {
			fullPath += ".fox"
		}
	}

	content, err := os.ReadFile(fullPath)
	if err != nil {
		return nil, err
	}

	l := lexer.New(string(content))
	p := parser.New(l)
	prog := p.ParseProgram()

	if len(p.Errors()) > 0 {
		// Convert parser errors to single error string for now
		msg := "Parser errors in module " + path + ":\n"
		for _, err := range p.Errors() {
			msg += fmt.Sprintf("  %s\n", err.String())
		}
		return nil, fmt.Errorf(msg)
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
	c.SetImporter(&FileImporter{Root: rootDir})

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
}
