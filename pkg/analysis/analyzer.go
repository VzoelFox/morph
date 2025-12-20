package analysis

import (
	"crypto/sha256"
	"fmt"
	"strings"
	"time"

	"github.com/VzoelFox/morphlang/pkg/parser"
)

type Analyzer struct {
	program    *parser.Program
	filename   string
	input      string
	context    *Context
	currFunc   string
	scopeStack []map[string]bool // Stack of defined variables to simulate scope lookups
}

func GenerateContext(program *parser.Program, filename string, input string, parserErrors []parser.ParserError) (*Context, error) {
	ctx := &Context{
		Version:       "1.0.0",
		File:          filename,
		Timestamp:     time.Now(),
		Symbols:       make(map[string]*Symbol),
		GlobalVars:    make(map[string]*Variable),
		LocalScopes:   make(map[string]LocalScope),
		Errors:        []ParserError{},
		Warnings:      []Warning{},
		Imports:       []string{},
		TypeInference: make(map[string][]string),
		CallGraph:     make(map[string][]string),
	}

	// Checksum
	h := sha256.New()
	h.Write([]byte(input))
	ctx.Checksum = fmt.Sprintf("sha256:%x", h.Sum(nil))

	// Stats
	analyzeStatistics(ctx, input)

	// Parse Errors
	for _, err := range parserErrors {
		ctx.Errors = append(ctx.Errors, ParserError{
			Level:   string(err.Level),
			Message: err.Message,
			Line:    err.Line,
			Column:  err.Column,
			File:    filename,
			Context: err.Context,
		})
	}

	if program == nil {
		return ctx, nil
	}

	a := &Analyzer{
		program:    program,
		filename:   filename,
		input:      input,
		context:    ctx,
		scopeStack: []map[string]bool{},
	}

	// Push global scope
	a.scopeStack = append(a.scopeStack, make(map[string]bool))

	a.analyze()

	return ctx, nil
}

func analyzeStatistics(ctx *Context, input string) {
	lines := strings.Split(input, "\n")
	ctx.Statistics.TotalLines = len(lines)
	for _, line := range lines {
		trim := strings.TrimSpace(line)
		if trim == "" {
			ctx.Statistics.BlankLines++
		} else if strings.HasPrefix(trim, "#") {
			ctx.Statistics.CommentLines++
		} else {
			ctx.Statistics.CodeLines++
		}
	}
}

func (a *Analyzer) analyze() {
	// First pass: Global symbols and functions
	for _, stmt := range a.program.Statements {
		a.analyzeTopLevel(stmt)
	}
	// Calculate complexity summary
	a.context.Complexity.LinesOfCode = a.context.Statistics.CodeLines
}

// isDefined checks if a variable is defined in the current or outer scopes
func (a *Analyzer) isDefined(name string) bool {
	// Iterate stack backwards (from current scope to global)
	for i := len(a.scopeStack) - 1; i >= 0; i-- {
		if _, ok := a.scopeStack[i][name]; ok {
			return true
		}
	}
	return false
}

// defineInCurrentScope marks a variable as defined in the current top scope
func (a *Analyzer) defineInCurrentScope(name string) {
	if len(a.scopeStack) > 0 {
		a.scopeStack[len(a.scopeStack)-1][name] = true
	}
}

func (a *Analyzer) analyzeTopLevel(stmt parser.Statement) {
	switch s := stmt.(type) {
	case *parser.ExpressionStatement:
		// Check for function literal: `fungsi name(...) ...`
		if fn, ok := s.Expression.(*parser.FunctionLiteral); ok {
			a.analyzeFunction(fn)
		} else {
			// Walk top level expressions (scripts)
			a.walkExpression(s.Expression, func(node parser.Node) {})
		}
	case *parser.AssignmentStatement:
		// Global variable
		if ident, ok := s.Name.(*parser.Identifier); ok {
			name := ident.Value
			inferredType := a.inferType(s.Value)

			a.context.GlobalVars[name] = &Variable{
				Line: s.Token.Line,
				Type: inferredType,
			}
			a.defineInCurrentScope(name) // Mark global var
		}
		a.walkExpression(s.Value, func(node parser.Node) {})
	}
}

func (a *Analyzer) analyzeFunction(fn *parser.FunctionLiteral) {
	name := fn.Name
	if name == "" {
		name = "<anonymous>"
	}

	sym := &Symbol{
		Type:            "function",
		Line:            fn.Token.Line,
		Column:          fn.Token.Column,
		Parameters:      []Parameter{},
		LocalVars:       []string{},
		Calls:           []string{},
		ErrorConditions: []ErrorCond{},
		Doc:             fn.Doc,
	}

	for _, p := range fn.Parameters {
		sym.Parameters = append(sym.Parameters, Parameter{
			Name:         p.Value,
			InferredType: "any",
			Line:         p.Token.Line,
			Column:       p.Token.Column,
		})
	}

	a.context.Symbols[name] = sym
	a.context.CallGraph[name] = []string{}
	a.context.LocalScopes[name] = make(LocalScope)

	prevFunc := a.currFunc
	a.currFunc = name

	// Push new scope for function
	a.scopeStack = append(a.scopeStack, make(map[string]bool))

	// Define parameters in this scope
	for _, p := range fn.Parameters {
		a.defineInCurrentScope(p.Value)
	}

	canError := false
	complexity := 1 // Base complexity

	a.walkBlock(fn.Body, func(node parser.Node) {
		// Check for calls
		if call, ok := node.(*parser.CallExpression); ok {
			if ident, ok := call.Function.(*parser.Identifier); ok {
				sym.Calls = append(sym.Calls, ident.Value)
				a.context.CallGraph[name] = append(a.context.CallGraph[name], ident.Value)
			}
		}
		// Check for error conditions (if ... return galat)
		if ifExpr, ok := node.(*parser.IfExpression); ok {
			if containsReturnError(ifExpr.Consequence) {
				condStr := ifExpr.Condition.String()
				msg := extractErrorMessage(ifExpr.Consequence)
				sym.ErrorConditions = append(sym.ErrorConditions, ErrorCond{
					Condition: condStr,
					Message:   msg,
					Line:      ifExpr.Token.Line,
				})
				canError = true
			}
		}
		// Check for returns galat (direct)
		if ret, ok := node.(*parser.ReturnStatement); ok {
			if ret.ReturnValue != nil {
				if call, ok := ret.ReturnValue.(*parser.CallExpression); ok {
					if ident, ok := call.Function.(*parser.Identifier); ok {
						if ident.Value == "galat" {
							canError = true
						}
					}
				}
			}
		}
		// Local vars logic (Closure Aware)
		if assign, ok := node.(*parser.AssignmentStatement); ok {
			if ident, ok := assign.Name.(*parser.Identifier); ok {
				varName := ident.Value

				// Infer Type
				inferred := a.inferType(assign.Value)

				// Check if variable is defined in any scope up the chain
				if a.isDefined(varName) {
					// It's an UPDATE. Check if it's local in CURRENT function scope to update type
					if v, ok := a.context.LocalScopes[name][varName]; ok {
						if v.Type == "inferred" || v.Type == "unknown" {
							v.Type = inferred
						}
					}
				} else {
					// It's a NEW declaration in this scope
					a.defineInCurrentScope(varName)

					// Register in symbol table as Local Var
					found := false
					for _, v := range sym.LocalVars {
						if v == varName {
							found = true
							break
						}
					}
					if !found {
						sym.LocalVars = append(sym.LocalVars, varName)
						a.context.LocalScopes[name][varName] = &Variable{
							Line: assign.Token.Line,
							Type: inferred,
						}
					}
				}
			}
		}
		// Complexity
		if _, ok := node.(*parser.IfExpression); ok {
			complexity++
		}
		if _, ok := node.(*parser.WhileExpression); ok {
			complexity++
		}
	})

	// Pop scope
	a.scopeStack = a.scopeStack[:len(a.scopeStack)-1]

	sym.CanError = canError
	if canError {
		sym.Returns = &TypeInfo{Type: "union", Types: []string{"any", "error"}}
	}

	if complexity > a.context.Complexity.Cyclomatic {
		a.context.Complexity.Cyclomatic += complexity
	}

	a.currFunc = prevFunc
	a.context.Complexity.Functions++
}

func (a *Analyzer) walkBlock(block *parser.BlockStatement, visitor func(parser.Node)) {
	if block == nil {
		return
	}
	for _, stmt := range block.Statements {
		visitor(stmt)
		a.walkStatement(stmt, visitor)
	}
}

func (a *Analyzer) walkStatement(stmt parser.Statement, visitor func(parser.Node)) {
	switch s := stmt.(type) {
	case *parser.ExpressionStatement:
		a.walkExpression(s.Expression, visitor)
	case *parser.ReturnStatement:
		if s.ReturnValue != nil {
			a.walkExpression(s.ReturnValue, visitor)
		}
	case *parser.AssignmentStatement:
		a.walkExpression(s.Name, visitor)
		a.walkExpression(s.Value, visitor)
	}
}

func (a *Analyzer) walkExpression(expr parser.Expression, visitor func(parser.Node)) {
	if expr == nil {
		return
	}
	visitor(expr)

	switch e := expr.(type) {
	case *parser.InfixExpression:
		a.walkExpression(e.Left, visitor)
		a.walkExpression(e.Right, visitor)
	case *parser.PrefixExpression:
		a.walkExpression(e.Right, visitor)
	case *parser.IfExpression:
		a.walkExpression(e.Condition, visitor)
		a.walkBlock(e.Consequence, visitor)
		a.walkBlock(e.Alternative, visitor)
	case *parser.WhileExpression:
		a.walkExpression(e.Condition, visitor)
		a.walkBlock(e.Body, visitor)
	case *parser.CallExpression:
		a.walkExpression(e.Function, visitor)
		for _, arg := range e.Arguments {
			a.walkExpression(arg, visitor)
		}
	case *parser.InterpolatedString:
		for _, part := range e.Parts {
			a.walkExpression(part, visitor)
		}
	case *parser.FunctionLiteral:
		a.analyzeFunction(e)
	}
}

func (a *Analyzer) inferType(expr parser.Expression) string {
	switch e := expr.(type) {
	case *parser.IntegerLiteral:
		return "integer"
	case *parser.FloatLiteral:
		return "float"
	case *parser.StringLiteral:
		return "string"
	case *parser.BooleanLiteral:
		return "boolean"
	case *parser.Identifier:
		// Check local scope
		if a.currFunc != "" {
			if scope, ok := a.context.LocalScopes[a.currFunc]; ok {
				if v, ok := scope[e.Value]; ok {
					return v.Type
				}
			}
			// Check params
			if sym, ok := a.context.Symbols[a.currFunc]; ok {
				for _, p := range sym.Parameters {
					if p.Name == e.Value {
						return p.InferredType
					}
				}
			}
		}
		// Check global
		if v, ok := a.context.GlobalVars[e.Value]; ok {
			return v.Type
		}
		return "unknown"
	case *parser.CallExpression:
		if ident, ok := e.Function.(*parser.Identifier); ok {
			if ident.Value == "galat" {
				return "error"
			}
			if sym, ok := a.context.Symbols[ident.Value]; ok {
				if sym.Returns != nil && len(sym.Returns.Types) > 0 {
					if len(sym.Returns.Types) == 1 {
						return sym.Returns.Types[0]
					}
					return "union"
				}
			}
		}
		return "any"
	case *parser.InfixExpression:
		switch e.Operator {
		case "==", "!=", "<", ">", "<=", ">=":
			return "boolean"
		default:
			return a.inferType(e.Left)
		}
	}
	return "unknown"
}

func containsReturnError(block *parser.BlockStatement) bool {
	if block == nil {
		return false
	}
	for _, stmt := range block.Statements {
		if ret, ok := stmt.(*parser.ReturnStatement); ok {
			if ret.ReturnValue != nil {
				if call, ok := ret.ReturnValue.(*parser.CallExpression); ok {
					if ident, ok := call.Function.(*parser.Identifier); ok {
						if ident.Value == "galat" {
							return true
						}
					}
				}
			}
		}
	}
	return false
}

func extractErrorMessage(block *parser.BlockStatement) string {
	if block == nil {
		return ""
	}
	for _, stmt := range block.Statements {
		if ret, ok := stmt.(*parser.ReturnStatement); ok {
			if ret.ReturnValue != nil {
				if call, ok := ret.ReturnValue.(*parser.CallExpression); ok {
					if len(call.Arguments) > 0 {
						if str, ok := call.Arguments[0].(*parser.StringLiteral); ok {
							return str.Value
						}
					}
				}
			}
		}
	}
	return ""
}
