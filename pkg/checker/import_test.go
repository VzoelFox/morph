package checker

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
	"github.com/VzoelFox/morph/pkg/parser"
)

// MockImporter implements Importer interface
type MockImporter struct {
	modules map[string]string
}

func (m *MockImporter) Import(path string) (*parser.Program, error) {
	content, ok := m.modules[path]
	if !ok {
		// return nil, fmt.Errorf("module not found: %s", path)
		// Return empty program or simple error?
		// parser error?
		return nil, nil // error handled in checkImport by checking err!=nil. Wait, if err is nil?
	}
	l := lexer.New(content)
	p := parser.New(l)
	return p.ParseProgram(), nil
}

func TestImportStatement(t *testing.T) {
	// Setup mock modules
	mock := &MockImporter{
		modules: map[string]string{
			"math": `
				fungsi Add(a int, b int) int
					kembalikan a + b
				akhir

				var PI float = 3.14

				# Private (lowercase)
				var secret int = 42
			`,
			"cycle/a": `
				ambil "cycle/b"
			`,
			"cycle/b": `
				ambil "cycle/a"
			`,
		},
	}

	// 1. Basic Import and Member Access
	input1 := `
	ambil "math"

	var res int = math.Add(1, 2)
	var pi float = math.PI
	`
	runImportTest(t, mock, input1, 0)

	// 2. Private Member Access (Should Fail)
	input2 := `
	ambil "math"
	var s int = math.secret
	`
	runImportTest(t, mock, input2, 1) // Expected error: Module does not export 'secret'

	// 3. From Import
	input3 := `
	dari "math" ambil Add, PI
	var x = Add(1, 2)
	var y = PI
	`
	runImportTest(t, mock, input3, 0)

	// 4. From Import Missing Symbol
	input4 := `
	dari "math" ambil Missing
	`
	runImportTest(t, mock, input4, 1)

	// 5. Import Cycle
	input5 := `
	ambil "cycle/a"
	`
	// Cycle detection logic:
	// main -> a -> b -> a (Cycle!)
	// When checking "cycle/b", it imports "cycle/a".
	// "cycle/a" is in loadingModules stack?
	// main -> imports "cycle/a". loading["cycle/a"] = true.
	// check("cycle/a") -> imports "cycle/b". loading["cycle/b"] = true.
	// check("cycle/b") -> imports "cycle/a". loading["cycle/a"] is true. -> error.
	// So we expect error.
	runImportTest(t, mock, input5, 1)
}

func runImportTest(t *testing.T, importer Importer, input string, expectedErrors int) {
	l := lexer.New(input)
	p := parser.New(l)
	program := p.ParseProgram()

	c := New()
	c.SetImporter(importer)
	c.Check(program)

	if len(c.Errors) != expectedErrors {
		t.Errorf("Input:\n%s\nExpected %d errors, got %d. errors: %v", input, expectedErrors, len(c.Errors), c.Errors)
	}
}
