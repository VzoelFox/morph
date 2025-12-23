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

	// 1. Compile Modules
	for path, mod := range c.checker.ModuleCache {
		if mod.Program == nil {
			continue
		}
		prefix := mangle(path)
		if err := c.compileModule(mod.Program, prefix); err != nil {
			return "", err
		}
	}

	// 2. Compile Main Program
	if prog, ok := node.(*parser.Program); ok {
		// Main program functions use "mph_" prefix
		if err := c.compileModule(prog, "mph_"); err != nil {
			return "", err
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

func (c *Compiler) compileModule(prog *parser.Program, prefix string) error {
	for _, stmt := range prog.Statements {
		// Check for Function Definitions
		isFunc := false
		if s, ok := stmt.(*parser.ExpressionStatement); ok {
			if fn, ok := s.Expression.(*parser.FunctionLiteral); ok {
				if fn.Name != "" {
					if err := c.compileFunction(fn, prefix); err != nil {
						return err
					}
					isFunc = true
				}
			}
		}

		// If not a function, and we are in Main (prefix "mph_"), generate entry code
		if !isFunc && prefix == "mph_" {
			if err := c.compileStatement(stmt, &c.entryBody); err != nil {
				return err
			}
		}
	}
	return nil
}

func (c *Compiler) compileFunction(fn *parser.FunctionLiteral, prefix string) error {
	retType := "void"
	if len(fn.ReturnTypes) > 0 {
		if sim, ok := fn.ReturnTypes[0].(*parser.SimpleType); ok {
			if sim.Name == "Int" {
				retType = "mph_int"
			}
			if sim.Name == "Float" {
				retType = "mph_float"
			}
			if sim.Name == "Bool" {
				retType = "mph_bool"
			}
		}
	}

	c.output.WriteString(fmt.Sprintf("%s %s%s(MorphContext* ctx", retType, prefix, fn.Name))

	for _, p := range fn.Parameters {
		cType := "mph_int"
		if sim, ok := p.Type.(*parser.SimpleType); ok {
			if sim.Name == "String" {
				cType = "MorphString*"
			}
			if sim.Name == "Float" {
				cType = "mph_float"
			}
			if sim.Name == "Bool" {
				cType = "mph_bool"
			}
			if sim.Name == "Channel" {
				cType = "MorphChannel*"
			}
		}
		c.output.WriteString(fmt.Sprintf(", %s %s", cType, p.Name.Value))
	}

	c.output.WriteString(")")

	if fn.IsNative {
		c.output.WriteString(";\n\n")
		return nil
	}

	c.output.WriteString(" {\n")

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
	case *parser.ImportStatement:
		return nil
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
	if typ != nil {
		if typ.Kind() == checker.KindString {
			cType = "MorphString*"
		} else if typ.Kind() == checker.KindBool {
			cType = "mph_bool"
		} else if typ.Kind() == checker.KindChannel {
			cType = "MorphChannel*"
		}
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
	case *parser.InterpolatedString:
		return c.compileInterpolatedString(e)
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
	case *parser.MemberExpression:
		return "", fmt.Errorf("property access not supported yet")
	default:
		return "", fmt.Errorf("unsupported expression: %T", expr)
	}
}

func (c *Compiler) compileCall(call *parser.CallExpression) (string, error) {
	var funcName string

	if ident, ok := call.Function.(*parser.Identifier); ok {
		// Local Function or Builtin
		funcName = "mph_" + ident.Value
		if ident.Value == "native_print" {
			funcName = "mph_native_print"
		} else if ident.Value == "native_print_int" {
			funcName = "mph_native_print_int"
		} else if ident.Value == "saluran_baru" {
			funcName = "mph_channel_new"
		} else if ident.Value == "kirim" {
			funcName = "mph_channel_send"
		} else if ident.Value == "terima" {
			funcName = "mph_channel_recv"
		} else if ident.Value == "luncurkan" {
			return c.compileSpawn(call)
		}
	} else if mem, ok := call.Function.(*parser.MemberExpression); ok {
		// Module Call: math.Add -> mph_stdlib_math_Add
		objType := c.checker.Types[mem.Object]
		if objType != nil && objType.Kind() == checker.KindModule {
			modType := objType.(*checker.ModuleType)
			funcName = mangle(modType.Name) + mem.Member.Value
		} else {
			return "", fmt.Errorf("method calls not supported yet")
		}
	} else {
		return "", fmt.Errorf("complex function calls not supported")
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

	return fmt.Sprintf("%s(%s)", funcName, strings.Join(args, ", ")), nil
}

func (c *Compiler) compileSpawn(call *parser.CallExpression) (string, error) {
	if len(call.Arguments) == 1 {
		if ident, ok := call.Arguments[0].(*parser.Identifier); ok {
			return fmt.Sprintf("mph_thread_spawn((MorphEntryFunction)mph_%s, NULL)", ident.Value), nil
		}
		return "", fmt.Errorf("luncurkan arg 1 must be named function")
	} else if len(call.Arguments) == 2 {
		if ident, ok := call.Arguments[0].(*parser.Identifier); ok {
			arg, err := c.compileExpression(call.Arguments[1])
			if err != nil {
				return "", err
			}
			return fmt.Sprintf("mph_thread_spawn((MorphEntryFunction)mph_%s, (void*)%s)", ident.Value, arg), nil
		}
		return "", fmt.Errorf("luncurkan arg 1 must be named function")
	}
	return "", fmt.Errorf("luncurkan expects 1 or 2 arguments")
}

func (c *Compiler) compileInterpolatedString(is *parser.InterpolatedString) (string, error) {
	if len(is.Parts) == 0 {
		return "mph_string_new(ctx, \"\")", nil
	}

	current, err := c.compileExpression(is.Parts[0])
	if err != nil {
		return "", err
	}

	for _, part := range is.Parts[1:] {
		next, err := c.compileExpression(part)
		if err != nil {
			return "", err
		}
		current = fmt.Sprintf("mph_string_concat(ctx, %s, %s)", current, next)
	}

	return current, nil
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

func mangle(path string) string {
	s := strings.ReplaceAll(path, "/", "_")
	s = strings.ReplaceAll(s, ".", "_")
	return "mph_" + s + "_"
}
