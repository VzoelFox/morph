package compiler

import (
	"fmt"
	"strings"

	"github.com/VzoelFox/morph/pkg/checker"
	"github.com/VzoelFox/morph/pkg/parser"
)

// N3MorphCompiler - Pure Morph output compiler
type N3MorphCompiler struct {
	output   strings.Builder
	checker  *checker.Checker
	indent   int
}

func NewN3MorphCompiler(c *checker.Checker) *N3MorphCompiler {
	return &N3MorphCompiler{
		checker: c,
		indent:  0,
	}
}

func (c *N3MorphCompiler) Compile(program *parser.Program) (string, error) {
	c.output.Reset()
	c.indent = 0
	
	// Generate pure Morph code
	for _, stmt := range program.Statements {
		if err := c.compileStatement(stmt); err != nil {
			return "", err
		}
	}
	
	return c.output.String(), nil
}

func (c *N3MorphCompiler) compileStatement(stmt parser.Statement) error {
	switch s := stmt.(type) {
	case *parser.VarStatement:
		return c.compileVarStatement(s)
	case *parser.ReturnStatement:
		return c.compileReturnStatement(s)
	case *parser.ExpressionStatement:
		return c.compileExpressionStatement(s)
	default:
		return fmt.Errorf("unsupported statement: %T", stmt)
	}
}

func (c *N3MorphCompiler) compileVarStatement(stmt *parser.VarStatement) error {
	c.writeIndent()
	c.output.WriteString("var ")
	
	for i, name := range stmt.Names {
		if i > 0 {
			c.output.WriteString(", ")
		}
		c.output.WriteString(name.Value)
	}
	
	if len(stmt.Values) > 0 {
		c.output.WriteString(" = ")
		for i, val := range stmt.Values {
			if i > 0 {
				c.output.WriteString(", ")
			}
			if err := c.compileExpression(val); err != nil {
				return err
			}
		}
	}
	
	c.output.WriteString("\n")
	return nil
}

func (c *N3MorphCompiler) compileFunctionStatement(stmt *parser.ExpressionStatement) error {
	// Simplified function compilation
	c.writeIndent()
	c.output.WriteString("# Function compilation not implemented in N3 MVP\n")
	return nil
}

func (c *N3MorphCompiler) compileReturnStatement(stmt *parser.ReturnStatement) error {
	c.writeIndent()
	c.output.WriteString("kembali")
	
	if len(stmt.ReturnValues) > 0 {
		c.output.WriteString(" ")
		for i, val := range stmt.ReturnValues {
			if i > 0 {
				c.output.WriteString(", ")
			}
			if err := c.compileExpression(val); err != nil {
				return err
			}
		}
	}
	
	c.output.WriteString("\n")
	return nil
}

func (c *N3MorphCompiler) compileExpressionStatement(stmt *parser.ExpressionStatement) error {
	c.writeIndent()
	if err := c.compileExpression(stmt.Expression); err != nil {
		return err
	}
	c.output.WriteString("\n")
	return nil
}

func (c *N3MorphCompiler) compileIfStatement(stmt *parser.ExpressionStatement) error {
	c.writeIndent()
	c.output.WriteString("# If statement compilation not implemented in N3 MVP\n")
	return nil
}

func (c *N3MorphCompiler) compileWhileStatement(stmt *parser.ExpressionStatement) error {
	c.writeIndent()
	c.output.WriteString("# While statement compilation not implemented in N3 MVP\n")
	return nil
}

func (c *N3MorphCompiler) compileBlockStatement(stmt *parser.BlockStatement) error {
	c.indent++
	for _, s := range stmt.Statements {
		if err := c.compileStatement(s); err != nil {
			return err
		}
	}
	c.indent--
	return nil
}

func (c *N3MorphCompiler) compileExpression(expr parser.Expression) error {
	switch e := expr.(type) {
	case *parser.Identifier:
		c.output.WriteString(e.Value)
	case *parser.IntegerLiteral:
		c.output.WriteString(fmt.Sprintf("%d", e.Value))
	case *parser.StringLiteral:
		c.output.WriteString(fmt.Sprintf("\"%s\"", e.Value))
	case *parser.BooleanLiteral:
		if e.Value {
			c.output.WriteString("benar")
		} else {
			c.output.WriteString("salah")
		}
	case *parser.InfixExpression:
		if err := c.compileExpression(e.Left); err != nil {
			return err
		}
		c.output.WriteString(" ")
		c.output.WriteString(e.Operator)
		c.output.WriteString(" ")
		if err := c.compileExpression(e.Right); err != nil {
			return err
		}
	case *parser.CallExpression:
		if err := c.compileExpression(e.Function); err != nil {
			return err
		}
		c.output.WriteString("(")
		for i, arg := range e.Arguments {
			if i > 0 {
				c.output.WriteString(", ")
			}
			if err := c.compileExpression(arg); err != nil {
				return err
			}
		}
		c.output.WriteString(")")
	default:
		return fmt.Errorf("unsupported expression: %T", expr)
	}
	return nil
}

func (c *N3MorphCompiler) compileType(typeNode parser.Node) error {
	switch t := typeNode.(type) {
	case *parser.Identifier:
		c.output.WriteString(t.Value)
	default:
		return fmt.Errorf("unsupported type: %T", typeNode)
	}
	return nil
}

func (c *N3MorphCompiler) writeIndent() {
	for i := 0; i < c.indent; i++ {
		c.output.WriteString("    ")
	}
}
