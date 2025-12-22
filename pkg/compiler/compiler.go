package compiler

import (
	"fmt"
	"strings"

	"github.com/VzoelFox/morph/pkg/parser"
)

type Compiler struct {
	output    strings.Builder
	entryBody strings.Builder
}

func New() *Compiler {
	return &Compiler{}
}

func (c *Compiler) Compile(node parser.Node) (string, error) {
	c.output.Reset()
	c.entryBody.Reset()

	c.output.WriteString("#include \"morph.h\"\n\n")

	// Forward declarations for native bindings (Temporary)
	c.output.WriteString("// Native bindings\n")
	c.output.WriteString("void mph_native_print(MorphContext* ctx, const char* s);\n\n")

	if prog, ok := node.(*parser.Program); ok {
		for _, stmt := range prog.Statements {
			if err := c.compileTopLevelStatement(stmt); err != nil {
				return "", err
			}
		}
	} else {
		return "", fmt.Errorf("expected *parser.Program, got %T", node)
	}

	c.output.WriteString("\n// Entry Point\n")
	c.output.WriteString("void morph_entry_point(MorphContext* ctx) {\n")
	c.output.WriteString(c.entryBody.String())
	c.output.WriteString("}\n")

	return c.output.String(), nil
}

func (c *Compiler) compileTopLevelStatement(stmt parser.Statement) error {
	// Check for Function Definition
	if s, ok := stmt.(*parser.ExpressionStatement); ok {
		if fn, ok := s.Expression.(*parser.FunctionLiteral); ok {
			if fn.Name != "" {
				return c.compileFunction(fn)
			}
		}
	}

	// Otherwise, treat as executable statement for entry point
	return c.compileStatement(stmt, &c.entryBody)
}

func (c *Compiler) compileFunction(fn *parser.FunctionLiteral) error {
	c.output.WriteString(fmt.Sprintf("void mph_%s(MorphContext* ctx) {\n", fn.Name))

	var body strings.Builder
	if err := c.compileBlock(fn.Body, &body); err != nil {
		return err
	}
	c.output.WriteString(body.String())
	c.output.WriteString("}\n\n")
	return nil
}

func (c *Compiler) compileBlock(block *parser.BlockStatement, buf *strings.Builder) error {
	for _, stmt := range block.Statements {
		if err := c.compileStatement(stmt, buf); err != nil {
			return err
		}
	}
	return nil
}

func (c *Compiler) compileStatement(stmt parser.Statement, buf *strings.Builder) error {
	switch s := stmt.(type) {
	case *parser.ExpressionStatement:
		exprCode, err := c.compileExpression(s.Expression)
		if err != nil {
			return err
		}
		buf.WriteString(fmt.Sprintf("\t%s;\n", exprCode))
		return nil
	default:
		return fmt.Errorf("unsupported statement in body: %T", stmt)
	}
}

func (c *Compiler) compileExpression(expr parser.Expression) (string, error) {
	switch e := expr.(type) {
	case *parser.CallExpression:
		return c.compileCall(e)
	case *parser.StringLiteral:
		return fmt.Sprintf("\"%s\"", e.Value), nil
	case *parser.IntegerLiteral:
		return fmt.Sprintf("%d", e.Value), nil
	default:
		return "", fmt.Errorf("unsupported expression: %T", expr)
	}
}

func (c *Compiler) compileCall(call *parser.CallExpression) (string, error) {
	// Resolve function name
	var funcName string
	if ident, ok := call.Function.(*parser.Identifier); ok {
		funcName = ident.Value
	} else {
		return "", fmt.Errorf("only direct function calls supported in MVP")
	}

	// Args
	var args []string
	args = append(args, "ctx") // Always pass context first

	for _, arg := range call.Arguments {
		code, err := c.compileExpression(arg)
		if err != nil {
			return "", err
		}
		args = append(args, code)
	}

	// Native mapping
	if funcName == "native_print" {
		return fmt.Sprintf("mph_native_print(%s)", strings.Join(args, ", ")), nil
	}

	// User function mapping
	return fmt.Sprintf("mph_%s(%s)", funcName, strings.Join(args, ", ")), nil
}
