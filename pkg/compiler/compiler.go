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

	// 0. Compile Types (Structs)
	if err := c.compileStructTypes(node); err != nil {
		return "", err
	}

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
			if err := c.compileStatement(stmt, &c.entryBody, prefix); err != nil {
				return err
			}
		}
	}
	return nil
}

func (c *Compiler) compileFunction(fn *parser.FunctionLiteral, prefix string) error {
	retType := "void"
	if len(fn.ReturnTypes) > 0 {
		var err error
		retType, err = c.mapTypeToC(fn.ReturnTypes[0], prefix)
		if err != nil {
			return err
		}
	}

	c.output.WriteString(fmt.Sprintf("%s %s%s(MorphContext* ctx", retType, prefix, fn.Name))

	for _, p := range fn.Parameters {
		cType, err := c.mapTypeToC(p.Type, prefix)
		if err != nil {
			return err
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
	if err := c.compileBlock(fn.Body, &body, prefix); err != nil {
		return err
	}
	c.output.WriteString(body.String())
	c.output.WriteString("}\n\n")
	return nil
}

func (c *Compiler) compileBlock(block *parser.BlockStatement, buf *strings.Builder, prefix string) error {
	for _, stmt := range block.Statements {
		if err := c.compileStatement(stmt, buf, prefix); err != nil {
			return err
		}
	}
	return nil
}

func (c *Compiler) compileStatement(stmt parser.Statement, buf *strings.Builder, prefix string) error {
	switch s := stmt.(type) {
	case *parser.VarStatement:
		return c.compileVar(s, buf, prefix)
	case *parser.ReturnStatement:
		return c.compileReturn(s, buf, prefix)
	case *parser.AssignmentStatement:
		return c.compileAssignment(s, buf, prefix)
	case *parser.ImportStatement:
		return nil
	case *parser.StructStatement:
		return nil
	case *parser.BlockStatement:
		buf.WriteString("{\n")
		if err := c.compileBlock(s, buf, prefix); err != nil {
			return err
		}
		buf.WriteString("}\n")
		return nil
	case *parser.ExpressionStatement:
		if ifExpr, ok := s.Expression.(*parser.IfExpression); ok {
			return c.compileIf(ifExpr, buf, prefix)
		}
		if whileExpr, ok := s.Expression.(*parser.WhileExpression); ok {
			return c.compileWhile(whileExpr, buf, prefix)
		}

		exprCode, err := c.compileExpression(s.Expression, prefix)
		if err != nil {
			return err
		}
		buf.WriteString(fmt.Sprintf("\t%s;\n", exprCode))
		return nil
	default:
		return fmt.Errorf("unsupported statement: %T", stmt)
	}
}

func (c *Compiler) compileVar(s *parser.VarStatement, buf *strings.Builder, prefix string) error {
	if len(s.Names) != 1 || len(s.Values) != 1 {
		return fmt.Errorf("multi-var not supported in compiler yet")
	}
	name := s.Names[0].Value
	valCode, err := c.compileExpression(s.Values[0], prefix)
	if err != nil {
		return err
	}

	cType := "mph_int"
	if s.Type != nil {
		var err error
		cType, err = c.mapTypeToC(s.Type, prefix)
		if err != nil {
			return err
		}
	} else {
		typ := c.checker.Types[s.Names[0]]
		if typ == nil {
			typ = c.checker.Types[s.Values[0]]
		}
		cType = c.mapCheckerTypeToC(typ, prefix)
	}

	buf.WriteString(fmt.Sprintf("\t%s %s = %s;\n", cType, name, valCode))
	return nil
}

func (c *Compiler) compileAssignment(s *parser.AssignmentStatement, buf *strings.Builder, prefix string) error {
	var name string
	if ident, ok := s.Names[0].(*parser.Identifier); ok {
		name = ident.Value
	} else if mem, ok := s.Names[0].(*parser.MemberExpression); ok {
		objCode, err := c.compileExpression(mem.Object, prefix)
		if err != nil {
			return err
		}
		name = fmt.Sprintf("%s->%s", objCode, mem.Member.Value)
	} else {
		return fmt.Errorf("complex assignment not supported")
	}

	valCode, err := c.compileExpression(s.Values[0], prefix)
	if err != nil {
		return err
	}
	buf.WriteString(fmt.Sprintf("\t%s = %s;\n", name, valCode))
	return nil
}

func (c *Compiler) compileReturn(s *parser.ReturnStatement, buf *strings.Builder, prefix string) error {
	if len(s.ReturnValues) == 0 {
		buf.WriteString("\treturn;\n")
		return nil
	}
	valCode, err := c.compileExpression(s.ReturnValues[0], prefix)
	if err != nil {
		return err
	}
	buf.WriteString(fmt.Sprintf("\treturn %s;\n", valCode))
	return nil
}

func (c *Compiler) compileIf(ie *parser.IfExpression, buf *strings.Builder, prefix string) error {
	cond, err := c.compileExpression(ie.Condition, prefix)
	if err != nil {
		return err
	}

	buf.WriteString(fmt.Sprintf("\tif (%s) {\n", cond))
	if err := c.compileStatement(ie.Consequence, buf, prefix); err != nil {
		return err
	}
	buf.WriteString("\t}")

	if ie.Alternative != nil {
		buf.WriteString(" else {\n")
		if err := c.compileStatement(ie.Alternative, buf, prefix); err != nil {
			return err
		}
		buf.WriteString("\t}\n")
	} else {
		buf.WriteString("\n")
	}
	return nil
}

func (c *Compiler) compileWhile(we *parser.WhileExpression, buf *strings.Builder, prefix string) error {
	cond, err := c.compileExpression(we.Condition, prefix)
	if err != nil {
		return err
	}

	buf.WriteString(fmt.Sprintf("\twhile (%s) {\n", cond))
	if err := c.compileStatement(we.Body, buf, prefix); err != nil {
		return err
	}
	buf.WriteString("\t}\n")
	return nil
}

func (c *Compiler) compileExpression(expr parser.Expression, prefix string) (string, error) {
	switch e := expr.(type) {
	case *parser.CallExpression:
		return c.compileCall(e, prefix)
	case *parser.StringLiteral:
		return fmt.Sprintf("mph_string_new(ctx, \"%s\")", e.Value), nil
	case *parser.InterpolatedString:
		return c.compileInterpolatedString(e, prefix)
	case *parser.IntegerLiteral:
		return fmt.Sprintf("%d", e.Value), nil
	case *parser.BooleanLiteral:
		if e.Value {
			return "1", nil
		}
		return "0", nil
	case *parser.Identifier:
		return e.Value, nil
	case *parser.InfixExpression:
		return c.compileInfix(e, prefix)
	case *parser.PrefixExpression:
		right, err := c.compileExpression(e.Right, prefix)
		if err != nil {
			return "", err
		}
		return fmt.Sprintf("(%s%s)", e.Operator, right), nil
	case *parser.MemberExpression:
		obj, err := c.compileExpression(e.Object, prefix)
		if err != nil {
			return "", err
		}
		return fmt.Sprintf("%s->%s", obj, e.Member.Value), nil
	case *parser.StructLiteral:
		return c.compileStructLiteral(e, prefix)
	case *parser.ArrayLiteral:
		return c.compileArrayLiteral(e, prefix)
	case *parser.IndexExpression:
		return c.compileIndex(e, prefix)
	default:
		return "", fmt.Errorf("unsupported expression: %T", expr)
	}
}

func (c *Compiler) compileArrayLiteral(al *parser.ArrayLiteral, prefix string) (string, error) {
	arrType := c.checker.Types[al]
	elemCType := "mph_int"
	if arrType != nil {
		if at, ok := arrType.(*checker.ArrayType); ok {
			elemCType = c.mapCheckerTypeToC(at.Element, prefix)
		}
	}

	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("MorphArray* _a = mph_array_new(ctx, %d, sizeof(%s)); ", len(al.Elements), elemCType))

	for i, el := range al.Elements {
		valCode, err := c.compileExpression(el, prefix)
		if err != nil {
			return "", err
		}
		sb.WriteString(fmt.Sprintf("((%s*)_a->data)[%d] = %s; ", elemCType, i, valCode))
	}
	sb.WriteString("_a; })")
	return sb.String(), nil
}

func (c *Compiler) compileIndex(ie *parser.IndexExpression, prefix string) (string, error) {
	leftCode, err := c.compileExpression(ie.Left, prefix)
	if err != nil {
		return "", err
	}
	indexCode, err := c.compileExpression(ie.Index, prefix)
	if err != nil {
		return "", err
	}

	leftType := c.checker.Types[ie.Left]
	if leftType == nil {
		return "", fmt.Errorf("unknown type for index expression")
	}

	if leftType.Kind() == checker.KindString {
		return fmt.Sprintf("((MorphString*)%s)->data[%s]", leftCode, indexCode), nil
	} else if leftType.Kind() == checker.KindArray {
		if at, ok := leftType.(*checker.ArrayType); ok {
			elemCType := c.mapCheckerTypeToC(at.Element, prefix)
			return fmt.Sprintf("*(%s*)mph_array_at(ctx, %s, %s)", elemCType, leftCode, indexCode), nil
		}
	}

	return "", fmt.Errorf("index operation not supported for type %s", leftType.String())
}

func (c *Compiler) compileStructLiteral(sl *parser.StructLiteral, prefix string) (string, error) {
	var cTypeName string
	if ident, ok := sl.Name.(*parser.Identifier); ok {
		cTypeName = prefix + ident.Value
	} else {
		return "", fmt.Errorf("complex struct literals not supported yet")
	}

	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("%s* _t = (%s*)mph_alloc(ctx, sizeof(%s)); ", cTypeName, cTypeName, cTypeName))

	for fieldName, valExpr := range sl.Fields {
		valCode, err := c.compileExpression(valExpr, prefix)
		if err != nil {
			return "", err
		}
		sb.WriteString(fmt.Sprintf("_t->%s = %s; ", fieldName, valCode))
	}

	sb.WriteString("_t; })")
	return sb.String(), nil
}

func (c *Compiler) compileCall(call *parser.CallExpression, prefix string) (string, error) {
	var funcName string

	if ident, ok := call.Function.(*parser.Identifier); ok {
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
			return c.compileSpawn(call, prefix)
		}
	} else if mem, ok := call.Function.(*parser.MemberExpression); ok {
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
		code, err := c.compileExpression(arg, prefix)
		if err != nil {
			return "", err
		}
		args = append(args, code)
	}

	return fmt.Sprintf("%s(%s)", funcName, strings.Join(args, ", ")), nil
}

func (c *Compiler) compileSpawn(call *parser.CallExpression, prefix string) (string, error) {
	if len(call.Arguments) == 1 {
		if ident, ok := call.Arguments[0].(*parser.Identifier); ok {
			return fmt.Sprintf("mph_thread_spawn((MorphEntryFunction)mph_%s, NULL)", ident.Value), nil
		}
		return "", fmt.Errorf("luncurkan arg 1 must be named function")
	} else if len(call.Arguments) == 2 {
		if ident, ok := call.Arguments[0].(*parser.Identifier); ok {
			arg, err := c.compileExpression(call.Arguments[1], prefix)
			if err != nil {
				return "", err
			}
			return fmt.Sprintf("mph_thread_spawn((MorphEntryFunction)mph_%s, (void*)%s)", ident.Value, arg), nil
		}
		return "", fmt.Errorf("luncurkan arg 1 must be named function")
	}
	return "", fmt.Errorf("luncurkan expects 1 or 2 arguments")
}

func (c *Compiler) compileInterpolatedString(is *parser.InterpolatedString, prefix string) (string, error) {
	if len(is.Parts) == 0 {
		return "mph_string_new(ctx, \"\")", nil
	}

	current, err := c.compileExpression(is.Parts[0], prefix)
	if err != nil {
		return "", err
	}

	for _, part := range is.Parts[1:] {
		next, err := c.compileExpression(part, prefix)
		if err != nil {
			return "", err
		}
		current = fmt.Sprintf("mph_string_concat(ctx, %s, %s)", current, next)
	}

	return current, nil
}

func (c *Compiler) compileInfix(ie *parser.InfixExpression, prefix string) (string, error) {
	left, err := c.compileExpression(ie.Left, prefix)
	if err != nil {
		return "", err
	}
	right, err := c.compileExpression(ie.Right, prefix)
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

// --- Struct & Type Compilation Logic ---

func (c *Compiler) compileStructTypes(node parser.Node) error {
	c.output.WriteString("// Struct Definitions\n")

	for path, mod := range c.checker.ModuleCache {
		if mod.Program == nil {
			continue
		}
		prefix := mangle(path)
		if err := c.compileModuleStructs(mod.Program, prefix); err != nil {
			return err
		}
	}

	if prog, ok := node.(*parser.Program); ok {
		if err := c.compileModuleStructs(prog, "mph_"); err != nil {
			return err
		}
	}

	c.output.WriteString("\n")
	return nil
}

func (c *Compiler) compileModuleStructs(prog *parser.Program, prefix string) error {
	for _, stmt := range prog.Statements {
		if s, ok := stmt.(*parser.StructStatement); ok {
			name := prefix + s.Name.Value
			c.output.WriteString(fmt.Sprintf("typedef struct %s %s;\n", name, name))
		}
	}

	for _, stmt := range prog.Statements {
		if s, ok := stmt.(*parser.StructStatement); ok {
			if err := c.compileStructDef(s, prefix); err != nil {
				return err
			}
		}
	}
	return nil
}

func (c *Compiler) compileStructDef(s *parser.StructStatement, prefix string) error {
	name := prefix + s.Name.Value
	c.output.WriteString(fmt.Sprintf("struct %s {\n", name))

	for _, field := range s.Fields {
		cType, err := c.mapTypeToC(field.Type, prefix)
		if err != nil {
			return err
		}
		c.output.WriteString(fmt.Sprintf("\t%s %s;\n", cType, field.Name))
	}

	c.output.WriteString("};\n\n")
	return nil
}

func (c *Compiler) mapTypeToC(t parser.TypeNode, prefix string) (string, error) {
	switch ty := t.(type) {
	case *parser.SimpleType:
		switch ty.Name {
		case "int":
			return "mph_int", nil
		case "float":
			return "mph_float", nil
		case "bool":
			return "mph_bool", nil
		case "string":
			return "MorphString*", nil
		case "channel":
			return "MorphChannel*", nil
		case "void":
			return "void", nil
		default:
			return prefix + ty.Name + "*", nil
		}
	case *parser.QualifiedType:
		return "mph_" + ty.Package.Value + "_" + ty.Name.Value + "*", nil
	case *parser.ArrayType:
		return "MorphArray*", nil
	case *parser.MapType:
		return "", fmt.Errorf("maps not supported yet")
	}
	return "", fmt.Errorf("unknown type node: %T", t)
}

func (c *Compiler) mapCheckerTypeToC(t checker.Type, prefix string) string {
	if t == nil {
		return "mph_int"
	}
	switch t.Kind() {
	case checker.KindInt:
		return "mph_int"
	case checker.KindFloat:
		return "mph_float"
	case checker.KindBool:
		return "mph_bool"
	case checker.KindString:
		return "MorphString*"
	case checker.KindChannel:
		return "MorphChannel*"
	case checker.KindStruct:
		if st, ok := t.(*checker.StructType); ok {
			return "mph_" + st.Name + "*"
		}
	case checker.KindArray:
		return "MorphArray*"
	}
	return "mph_int" // Fallback
}
