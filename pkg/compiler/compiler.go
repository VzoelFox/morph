package compiler

import (
	"fmt"
	"strings"

	"github.com/VzoelFox/morph/pkg/checker"
	"github.com/VzoelFox/morph/pkg/parser"
)

type Compiler struct {
	output    strings.Builder
	entryBody strings.Builder
	checker   *checker.Checker
}

func New(c *checker.Checker) *Compiler {
	return &Compiler{checker: c}
}

func (c *Compiler) Compile(node parser.Node) (string, error) {
	c.output.Reset()
	c.entryBody.Reset()

	c.output.WriteString("#include \"morph.h\"\n\n")

	// Forward declarations for native bindings
	c.output.WriteString("// Native bindings\n")
	c.output.WriteString("void mph_native_print(MorphContext* ctx, MorphString* s);\n")
	c.output.WriteString("void mph_native_print_int(MorphContext* ctx, mph_int n);\n\n")

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
	if s, ok := stmt.(*parser.ExpressionStatement); ok {
		if fn, ok := s.Expression.(*parser.FunctionLiteral); ok {
			if fn.Name != "" {
				return c.compileFunction(fn)
			}
		}
	}
	return c.compileStatement(stmt, &c.entryBody)
}

func (c *Compiler) compileFunction(fn *parser.FunctionLiteral) error {
	retType := "void"
	if len(fn.ReturnTypes) > 0 {
		if sim, ok := fn.ReturnTypes[0].(*parser.SimpleType); ok {
			if sim.Name == "Int" {
				retType = "mph_int"
			}
			if sim.Name == "Bool" {
				retType = "mph_bool"
			}
		}
	}

	c.output.WriteString(fmt.Sprintf("%s mph_%s(MorphContext* ctx", retType, fn.Name))

	for _, p := range fn.Parameters {
		cType := "mph_int"
		if sim, ok := p.Type.(*parser.SimpleType); ok {
			if sim.Name == "String" {
				cType = "MorphString*"
			}
			if sim.Name == "Bool" {
				cType = "mph_bool"
			}
		}
		c.output.WriteString(fmt.Sprintf(", %s %s", cType, p.Name.Value))
	}

	c.output.WriteString(") {\n")

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
	case *parser.VarStatement:
		return c.compileVar(s, buf)
	case *parser.ReturnStatement:
		return c.compileReturn(s, buf)
	case *parser.AssignmentStatement:
		return c.compileAssignment(s, buf)
	case *parser.BlockStatement:
		buf.WriteString("{\n")
		if err := c.compileBlock(s, buf); err != nil {
			return err
		}
		buf.WriteString("}\n")
		return nil
	case *parser.ExpressionStatement:
		// Handle Control Flow Expressions as Statements in C
		if ifExpr, ok := s.Expression.(*parser.IfExpression); ok {
			return c.compileIf(ifExpr, buf)
		}
		if whileExpr, ok := s.Expression.(*parser.WhileExpression); ok {
			return c.compileWhile(whileExpr, buf)
		}

		exprCode, err := c.compileExpression(s.Expression)
		if err != nil {
			return err
		}
		buf.WriteString(fmt.Sprintf("\t%s;\n", exprCode))
		return nil
	default:
		return fmt.Errorf("unsupported statement: %T", stmt)
	}
}

func (c *Compiler) compileVar(s *parser.VarStatement, buf *strings.Builder) error {
	if len(s.Names) != 1 || len(s.Values) != 1 {
		return fmt.Errorf("multi-var not supported in compiler yet")
	}
	name := s.Names[0].Value
	valCode, err := c.compileExpression(s.Values[0])
	if err != nil {
		return err
	}

	typ := c.checker.Types[s.Names[0]]
	if typ == nil {
		typ = c.checker.Types[s.Values[0]]
	}

	cType := "mph_int"
	if typ != nil && typ.Kind() == checker.KindString {
		cType = "MorphString*"
	} else if typ != nil && typ.Kind() == checker.KindBool {
		cType = "mph_bool"
	}

	buf.WriteString(fmt.Sprintf("\t%s %s = %s;\n", cType, name, valCode))
	return nil
}

func (c *Compiler) compileAssignment(s *parser.AssignmentStatement, buf *strings.Builder) error {
	var name string
	if ident, ok := s.Names[0].(*parser.Identifier); ok {
		name = ident.Value
	} else {
		return fmt.Errorf("complex assignment not supported")
	}

	valCode, err := c.compileExpression(s.Values[0])
	if err != nil {
		return err
	}
	buf.WriteString(fmt.Sprintf("\t%s = %s;\n", name, valCode))
	return nil
}

func (c *Compiler) compileReturn(s *parser.ReturnStatement, buf *strings.Builder) error {
	if len(s.ReturnValues) == 0 {
		buf.WriteString("\treturn;\n")
		return nil
	}
	valCode, err := c.compileExpression(s.ReturnValues[0])
	if err != nil {
		return err
	}
	buf.WriteString(fmt.Sprintf("\treturn %s;\n", valCode))
	return nil
}

func (c *Compiler) compileIf(ie *parser.IfExpression, buf *strings.Builder) error {
	cond, err := c.compileExpression(ie.Condition)
	if err != nil {
		return err
	}

	buf.WriteString(fmt.Sprintf("\tif (%s) {\n", cond))
	if err := c.compileStatement(ie.Consequence, buf); err != nil {
		return err
	}
	buf.WriteString("\t}")

	if ie.Alternative != nil {
		buf.WriteString(" else {\n")
		if err := c.compileStatement(ie.Alternative, buf); err != nil {
			return err
		}
		buf.WriteString("\t}\n")
	} else {
		buf.WriteString("\n")
	}
	return nil
}

func (c *Compiler) compileWhile(we *parser.WhileExpression, buf *strings.Builder) error {
	cond, err := c.compileExpression(we.Condition)
	if err != nil {
		return err
	}

	buf.WriteString(fmt.Sprintf("\twhile (%s) {\n", cond))
	if err := c.compileStatement(we.Body, buf); err != nil {
		return err
	}
	buf.WriteString("\t}\n")
	return nil
}

func (c *Compiler) compileExpression(expr parser.Expression) (string, error) {
	switch e := expr.(type) {
	case *parser.CallExpression:
		return c.compileCall(e)
	case *parser.StringLiteral:
		return fmt.Sprintf("mph_string_new(ctx, \"%s\")", e.Value), nil
	case *parser.IntegerLiteral:
		return fmt.Sprintf("%d", e.Value), nil
	case *parser.Identifier:
		return e.Value, nil
	case *parser.InfixExpression:
		return c.compileInfix(e)
	case *parser.PrefixExpression:
		right, err := c.compileExpression(e.Right)
		if err != nil {
			return "", err
		}
		return fmt.Sprintf("(%s%s)", e.Operator, right), nil
	default:
		return "", fmt.Errorf("unsupported expression: %T", expr)
	}
}

func (c *Compiler) compileCall(call *parser.CallExpression) (string, error) {
	var funcName string
	if ident, ok := call.Function.(*parser.Identifier); ok {
		funcName = ident.Value
	} else {
		return "", fmt.Errorf("only direct function calls supported in MVP")
	}

	var args []string
	args = append(args, "ctx")

	for _, arg := range call.Arguments {
		code, err := c.compileExpression(arg)
		if err != nil {
			return "", err
		}
		args = append(args, code)
	}

	if funcName == "native_print" {
		return fmt.Sprintf("mph_native_print(%s)", strings.Join(args, ", ")), nil
	}
	if funcName == "native_print_int" {
		return fmt.Sprintf("mph_native_print_int(%s)", strings.Join(args, ", ")), nil
	}

	return fmt.Sprintf("mph_%s(%s)", funcName, strings.Join(args, ", ")), nil
}

func (c *Compiler) compileInfix(ie *parser.InfixExpression) (string, error) {
	left, err := c.compileExpression(ie.Left)
	if err != nil {
		return "", err
	}
	right, err := c.compileExpression(ie.Right)
	if err != nil {
		return "", err
	}

	lType := c.checker.Types[ie.Left]

	if lType != nil && lType.Kind() == checker.KindString {
		if ie.Operator == "+" {
			return fmt.Sprintf("mph_string_concat(ctx, %s, %s)", left, right), nil
		}
		if ie.Operator == "==" {
			return fmt.Sprintf("mph_string_eq(%s, %s)", left, right), nil
		}
	}

	return fmt.Sprintf("(%s %s %s)", left, ie.Operator, right), nil
}
