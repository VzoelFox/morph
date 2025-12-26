package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/VzoelFox/morph/pkg_legacy/checker"
	"github.com/VzoelFox/morph/pkg_legacy/compiler"
	cruntime "github.com/VzoelFox/morph/pkg_legacy/compiler/runtime"
	"github.com/VzoelFox/morph/pkg_legacy/evaluator"
	"github.com/VzoelFox/morph/pkg_legacy/lexer"
	"github.com/VzoelFox/morph/pkg_legacy/parser"
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
		// Add extension if missing
		if filepath.Ext(path) == "" {
			path += ".fox"
		}
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
		fmt.Printf("Morph Compiler\n")
		fmt.Printf("Usage:\n")
		fmt.Printf("  morph <filename.fox>        (Run Interpreter)\n")
		fmt.Printf("  morph build <filename.fox>  (Compile to C)\n")
		fmt.Printf("  morph n3 <filename.fox>     (Compile to Pure Morph - N3)\n")
		os.Exit(1)
	}

	arg1 := os.Args[1]
	if arg1 == "build" {
		if len(os.Args) < 3 {
			fmt.Println("Usage: morph build <filename.fox>")
			os.Exit(1)
		}
		runBuild(os.Args[2])
	} else if arg1 == "n3" {
		if len(os.Args) < 3 {
			fmt.Println("Usage: morph n3 <filename.fox>")
			os.Exit(1)
		}
		runN3Build(os.Args[2])
	} else {
		runInterpreter(arg1)
	}
}

func runBuild(filename string) {
	// 1. Parse
	content, err := os.ReadFile(filename)
	if err != nil {
		fmt.Printf("Error reading file: %v\n", err)
		os.Exit(1)
	}

	l := lexer.New(string(content))
	p := parser.New(l)
	prog := p.ParseProgram()

	if len(p.Errors()) > 0 {
		fmt.Println("❌ Parser Errors:")
		for _, err := range p.Errors() {
			fmt.Println(err)
		}
		os.Exit(1)
	}

	// 2. Type Checking
	c := checker.New()

	absPath, _ := filepath.Abs(filename)
	rootDir := filepath.Dir(absPath)
	currentDir, _ := os.Getwd()
	searchPaths := []string{
		rootDir, 
		currentDir, 
		filepath.Join(currentDir, "stdlib"), 
		filepath.Join(currentDir, "morphfox"),
		filepath.Join(currentDir, "morphfox", "stdlib"),
		".",
	}

	c.SetImporter(&FileImporter{SearchPaths: searchPaths})
	c.Check(prog)

	if len(c.Errors) > 0 {
		fmt.Println("❌ Type Errors:")
		for _, err := range c.Errors {
			fmt.Printf("  [%d:%d] %s\n", err.Line, err.Column, err.Message)
		}
		os.Exit(1)
	}

	// 3. Compile
	fmt.Println("🛠️  Transpiling to C...")
	comp := compiler.New(c)
	var cCode string
	cCode, err = comp.Compile(prog)
	if err != nil {
		fmt.Printf("❌ Compilation Error: %v\n", err)
		os.Exit(1)
	}

	// 3. Write Assets
	workDir := rootDir
	baseName := strings.TrimSuffix(filepath.Base(filename), filepath.Ext(filename))
	outC := filepath.Join(workDir, baseName+".c")
	outExe := filepath.Join(workDir, baseName)

	if err := os.WriteFile(filepath.Join(workDir, "morph.h"), []byte(cruntime.MorphHeader), 0644); err != nil {
		fmt.Printf("Error writing header: %v\n", err)
		os.Exit(1)
	}
	if err := os.WriteFile(filepath.Join(workDir, "runtime.c"), []byte(cruntime.RuntimeSource), 0644); err != nil {
		fmt.Printf("Error writing runtime: %v\n", err)
		os.Exit(1)
	}
	if err := os.WriteFile(outC, []byte(cCode), 0644); err != nil {
		fmt.Printf("Error writing C source: %v\n", err)
		os.Exit(1)
	}

	// 4. GCC
	fmt.Println("🔨 Compiling with GCC...")
	cmd := exec.Command("gcc", "-o", outExe, outC, filepath.Join(workDir, "runtime.c"), "-lpthread")
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		fmt.Printf("❌ GCC Error: %v\n", err)
		os.Exit(1)
	}

	fmt.Printf("✅ Build Success! Output: %s\n", outExe)
}

func runN3Build(filename string) {
	// 1. Parse
	content, err := os.ReadFile(filename)
	if err != nil {
		fmt.Printf("Error reading file: %v\n", err)
		os.Exit(1)
	}

	l := lexer.New(string(content))
	p := parser.New(l)
	prog := p.ParseProgram()

	if len(p.Errors()) > 0 {
		fmt.Println("❌ Parser Errors:")
		for _, err := range p.Errors() {
			fmt.Printf("  %s\n", err)
		}
		os.Exit(1)
	}

	// 2. Type Check
	rootDir := filepath.Dir(filename)
	importer := &FileImporter{
		SearchPaths: []string{
			rootDir, 
			".", 
			"stdlib",
			"morphfox",
			"morphfox/stdlib",
		},
	}
	c := checker.New()
	c.SetImporter(importer)
	c.Check(prog)

	// 3. N3 Compile to Pure Morph
	fmt.Println("🚀 Compiling to Pure Morph (N3)...")
	n3comp := compiler.NewN3MorphCompiler(c)
	morphCode, err := n3comp.Compile(prog)
	if err != nil {
		fmt.Printf("❌ N3 Compilation Error: %v\n", err)
		os.Exit(1)
	}

	// 4. Write Pure Morph Output
	baseName := strings.TrimSuffix(filepath.Base(filename), filepath.Ext(filename))
	outMorph := filepath.Join(rootDir, baseName+"_n3.fox")
	
	if err := os.WriteFile(outMorph, []byte(morphCode), 0644); err != nil {
		fmt.Printf("Error writing Morph output: %v\n", err)
		os.Exit(1)
	}

	fmt.Printf("✅ N3 Build Success! Pure Morph Output: %s\n", outMorph)
	fmt.Println("🎯 Ready for binary release compilation!")
}

func runInterpreter(filename string) {
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

	absPath, _ := filepath.Abs(filename)
	rootDir := filepath.Dir(absPath)
	currentDir, _ := os.Getwd()
	searchPaths := []string{
		rootDir, 
		currentDir, 
		filepath.Join(currentDir, "stdlib"), 
		filepath.Join(currentDir, "morphfox"),
		filepath.Join(currentDir, "morphfox", "stdlib"),
		".",
	}

	c.SetImporter(&FileImporter{SearchPaths: searchPaths})

	c.Check(program)

	if len(c.Warnings) > 0 {
		fmt.Println("⚠️  Warnings:")
		for _, warn := range c.Warnings {
			fmt.Printf("  [%d:%d] %s\n", warn.Line, warn.Column, warn.Message)
		}
	}

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
