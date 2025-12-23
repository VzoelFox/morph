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
	hasMain   bool
	StructIDs map[string]int
}

func New(c *checker.Checker) *Compiler {
	return &Compiler{
		checker:   c,
		StructIDs: make(map[string]int),
	}
}

func (c *Compiler) getStructID(name string) int {
	if id, ok := c.StructIDs[name]; ok {
		return id
	}
	id := len(c.StructIDs) + 1
	c.StructIDs[name] = id
	return id
}

func (c *Compiler) Compile(node parser.Node) (string, error) {
	c.output.Reset()
	c.entryBody.Reset()

	c.output.WriteString("#include \"morph.h\"\n\n")

	// Forward declarations for native bindings
	c.output.WriteString("// Native bindings\n")
	c.output.WriteString("void mph_native_print(MorphContext* ctx, MorphString* s);\n")
	c.output.WriteString("void mph_native_print_int(MorphContext* ctx, mph_int n);\n\n")

	// 0. Compile Types (Structs) and generate IDs
	if err := c.compileStructTypes(node); err != nil {
		return "", err
	}

	// Generate Type ID Macros
	c.output.WriteString("// Type IDs\n")
	for name, id := range c.StructIDs {
		c.output.WriteString(fmt.Sprintf("#define MPH_TYPE_%s %d\n", name, id))
	}
	c.output.WriteString("\n")

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
	if c.hasMain {
		c.output.WriteString("\tmph_main(ctx);\n")
	}
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
	if fn.Name == "main" && prefix == "mph_" {
		c.hasMain = true
	}

	retType := "void"
	if len(fn.ReturnTypes) > 0 {
		var err error
		retType, err = c.mapTypeToC(fn.ReturnTypes[0], prefix)
		if err != nil {
			return err
		}
	}

	funcName := fn.Name
	if fn.Receiver != nil {
		// Method: mph_ReceiverType_MethodName
		// Need to get receiver type name.
		// fn.Receiver.Type is TypeNode.
		recvTypeName := ""
		if st, ok := fn.Receiver.Type.(*parser.SimpleType); ok {
			recvTypeName = st.Name
		} else if qt, ok := fn.Receiver.Type.(*parser.QualifiedType); ok {
			recvTypeName = qt.Package.Value + "_" + qt.Name.Value
		}
		if recvTypeName != "" {
			funcName = recvTypeName + "_" + fn.Name
		}
	}

	c.output.WriteString(fmt.Sprintf("%s %s%s(MorphContext* ctx", retType, prefix, funcName))

	if fn.Receiver != nil {
		cType, err := c.mapTypeToC(fn.Receiver.Type, prefix)
		if err != nil {
			return err
		}
		c.output.WriteString(fmt.Sprintf(", %s %s", cType, fn.Receiver.Name.Value))
	}

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
	case *parser.InterfaceStatement:
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

	// Check Interface Assignment
	var destType checker.Type
	if s.Type != nil {
		destType = c.checker.Types[s.Type]
	} else {
		// Inference
		destType = c.checker.Types[s.Names[0]]
		if destType == nil { destType = c.checker.Types[s.Values[0]] }
	}

	valCode, err := c.compileExpression(s.Values[0], prefix)
	if err != nil {
		return err
	}

	srcType := c.checker.Types[s.Values[0]]
	if destType != nil && destType.Kind() == checker.KindInterface && srcType != nil && srcType.Kind() == checker.KindStruct {
		// Interface conversion
		valCode, err = c.compileInterfaceConversion(destType.(*checker.InterfaceType), srcType.(*checker.StructType), valCode, prefix)
		if err != nil { return err }
	}

	cType := "mph_int"
	// Prefer resolved Checker type (from Variable Name or Explicit Type Node)
	if s.Type != nil {
		// Explicit type node, look it up in checker cache
		if typeNodeType := c.checker.Types[s.Type]; typeNodeType != nil {
			cType = c.mapCheckerTypeToC(typeNodeType, prefix)
		} else {
			// Fallback to AST string mapping (unlikely if checker ran)
			var err error
			cType, err = c.mapTypeToC(s.Type, prefix)
			if err != nil { return err }
		}
	} else if destType != nil {
		// Inferred type
		cType = c.mapCheckerTypeToC(destType, prefix)
	}

	buf.WriteString(fmt.Sprintf("\t%s %s = %s;\n", cType, name, valCode))
	return nil
}

func (c *Compiler) compileAssignment(s *parser.AssignmentStatement, buf *strings.Builder, prefix string) error {
	valCode, err := c.compileExpression(s.Values[0], prefix)
	if err != nil {
		return err
	}

	// Check Interface Conversion
	srcType := c.checker.Types[s.Values[0]]
	destType := c.checker.Types[s.Names[0]]
	if destType != nil && destType.Kind() == checker.KindInterface && srcType.Kind() == checker.KindStruct {
		valCode, err = c.compileInterfaceConversion(destType.(*checker.InterfaceType), srcType.(*checker.StructType), valCode, prefix)
		if err != nil { return err }
	}

	if ident, ok := s.Names[0].(*parser.Identifier); ok {
		buf.WriteString(fmt.Sprintf("\t%s = %s;\n", ident.Value, valCode))
		return nil
	} else if mem, ok := s.Names[0].(*parser.MemberExpression); ok {
		objCode, err := c.compileExpression(mem.Object, prefix)
		if err != nil {
			return err
		}
		buf.WriteString(fmt.Sprintf("\t%s->%s = %s;\n", objCode, mem.Member.Value, valCode))
		return nil
	} else if idx, ok := s.Names[0].(*parser.IndexExpression); ok {
		// Map or Array Assignment
		objCode, err := c.compileExpression(idx.Left, prefix)
		if err != nil { return err }

		idxCode, err := c.compileExpression(idx.Index, prefix)
		if err != nil { return err }

		leftType := c.checker.Types[idx.Left]
		if leftType != nil && leftType.Kind() == checker.KindMap {
			mt := leftType.(*checker.MapType)

			keyCast := fmt.Sprintf("(void*)%s", idxCode)
			if mt.Key.Kind() == checker.KindInt {
				keyCast = fmt.Sprintf("(void*)(int64_t)%s", idxCode)
			}

			valCast := fmt.Sprintf("(void*)%s", valCode)
			if c.isPrimitive(mt.Value) {
				valCast = fmt.Sprintf("(void*)(int64_t)%s", valCode)
			}

			buf.WriteString(fmt.Sprintf("\tmph_map_set(ctx, %s, %s, %s);\n", objCode, keyCast, valCast))
			return nil
		}

		return fmt.Errorf("array/complex index assignment not supported yet")
	}

	return fmt.Errorf("complex assignment not supported")
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
		// Check for assertion: assert(iface, Type)
		if ident, ok := e.Function.(*parser.Identifier); ok && ident.Value == "assert" {
			return c.compileTypeAssertion(e, prefix)
		}
		return c.compileCall(e, prefix)
	case *parser.StringLiteral:
		escaped := strings.ReplaceAll(e.Value, "\n", "\\n")
		escaped = strings.ReplaceAll(escaped, "\"", "\\\"")
		return fmt.Sprintf("mph_string_new(ctx, \"%s\")", escaped), nil
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
	case *parser.HashLiteral:
		return c.compileHashLiteral(e, prefix)
	case *parser.IndexExpression:
		return c.compileIndex(e, prefix)
	default:
		return "", fmt.Errorf("unsupported expression: %T", expr)
	}
}

func (c *Compiler) compileHashLiteral(hl *parser.HashLiteral, prefix string) (string, error) {
	mapType := c.checker.Types[hl]
	kindEnum := "MPH_KEY_PTR"

	if mapType != nil {
		if mt, ok := mapType.(*checker.MapType); ok {
			if mt.Key.Kind() == checker.KindInt {
				kindEnum = "MPH_KEY_INT"
			} else if mt.Key.Kind() == checker.KindString {
				kindEnum = "MPH_KEY_STRING"
			}
		}
	} else if len(hl.Pairs) > 0 {
		// Try to infer from first key if checker didn't catch it (unlikely)
		for k := range hl.Pairs {
			if _, ok := k.(*parser.IntegerLiteral); ok {
				kindEnum = "MPH_KEY_INT"
			} else if _, ok := k.(*parser.StringLiteral); ok {
				kindEnum = "MPH_KEY_STRING"
			}
			break
		}
	}

	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("MorphMap* _m = mph_map_new(ctx, %s); ", kindEnum))

	for key, val := range hl.Pairs {
		keyCode, err := c.compileExpression(key, prefix)
		if err != nil {
			return "", err
		}
		valCode, err := c.compileExpression(val, prefix)
		if err != nil {
			return "", err
		}
		// Cast keys to void* for generic API. Integers are cast directly.
		keyCast := fmt.Sprintf("(void*)%s", keyCode)
		if kindEnum == "MPH_KEY_INT" {
			keyCast = fmt.Sprintf("(void*)(int64_t)%s", keyCode)
		}

		sb.WriteString(fmt.Sprintf("mph_map_set(ctx, _m, %s, (void*)%s); ", keyCast, valCode))
	}

	sb.WriteString("_m; })")
	return sb.String(), nil
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
	} else if leftType.Kind() == checker.KindMap {
		if mt, ok := leftType.(*checker.MapType); ok {
			valCType := c.mapCheckerTypeToC(mt.Value, prefix)

			keyCast := fmt.Sprintf("(void*)%s", indexCode)
			if mt.Key.Kind() == checker.KindInt {
				keyCast = fmt.Sprintf("(void*)(int64_t)%s", indexCode)
			}

			// Warning: Returns raw pointer, we cast it back to value type pointer then dereference?
			// But map stores (void*)value.
			// If value is pointer (String*, Struct*), we get (void*)ptr.
			// If value is primitive (int), we stored (void*)int_val.

			if c.isPrimitive(mt.Value) {
				// Primitive stored as void* directly
				return fmt.Sprintf("(%s)(int64_t)mph_map_get(ctx, %s, %s)", valCType, leftCode, keyCast), nil
			} else {
				// Pointer type stored as void*
				return fmt.Sprintf("(%s)mph_map_get(ctx, %s, %s)", valCType, leftCode, keyCast), nil
			}
		}
	}

	return "", fmt.Errorf("index operation not supported for type %s", leftType.String())
}

func (c *Compiler) isPrimitive(t checker.Type) bool {
	if t == nil { return true }
	switch t.Kind() {
	case checker.KindInt, checker.KindFloat, checker.KindBool:
		return true
	}
	return false
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
		} else if ident.Value == "hapus" {
			return c.compileDelete(call, prefix)
		} else if ident.Value == "panjang" {
			return c.compileLen(call, prefix)
		}
	} else if mem, ok := call.Function.(*parser.MemberExpression); ok {
		objType := c.checker.Types[mem.Object]
		if objType != nil && objType.Kind() == checker.KindModule {
			modType := objType.(*checker.ModuleType)
			funcName = mangle(modType.Name) + mem.Member.Value
		} else if objType != nil && objType.Kind() == checker.KindInterface {
			return c.compileInterfaceCall(call, mem, objType.(*checker.InterfaceType), prefix)
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

func (c *Compiler) compileInterfaceCall(call *parser.CallExpression, mem *parser.MemberExpression, iface *checker.InterfaceType, prefix string) (string, error) {
	// 1. Find Method Index (Alphabetical Order)
	// We must sort methods same as VTable generation!
	ifaceMethods := make([]string, 0, len(iface.Methods))
	for k := range iface.Methods {
		ifaceMethods = append(ifaceMethods, k)
	}
	for i := 0; i < len(ifaceMethods); i++ {
		for j := i + 1; j < len(ifaceMethods); j++ {
			if ifaceMethods[i] > ifaceMethods[j] {
				ifaceMethods[i], ifaceMethods[j] = ifaceMethods[j], ifaceMethods[i]
			}
		}
	}

	methodIndex := -1
	for i, name := range ifaceMethods {
		if name == mem.Member.Value {
			methodIndex = i
			break
		}
	}
	if methodIndex == -1 {
		return "", fmt.Errorf("method %s not found in interface %s", mem.Member.Value, iface.Name)
	}

	// 2. Compile Object (Interface)
	objCode, err := c.compileExpression(mem.Object, prefix)
	if err != nil { return "", err }

	// 3. Compile Arguments
	var args []string
	args = append(args, "ctx") // Context

	// Add instance as 'this'
	// objCode is MorphInterface (value type). We need .instance
	// Wait, compileExpression returns string of expression.
	// If objCode is a variable name, we use `var.instance`.
	// But `objCode` could be an expression returning MorphInterface.
	// Safe to use `(objCode).instance`?
	// Note: MorphInterface is passed by value.
	// So `((MorphInterface)objCode).instance`

	instanceCode := fmt.Sprintf("(%s).instance", objCode)
	args = append(args, instanceCode)

	for _, arg := range call.Arguments {
		code, err := c.compileExpression(arg, prefix)
		if err != nil { return "", err }
		args = append(args, code)
	}

	// 4. Generate Call
	// ((MethodSig) (objCode).vtable[idx])(...)
	// We need to cast function pointer to correct signature.
	// For now, let's assume `void (*)(MorphContext*, void*, ...)` generic sig?
	// No, arguments have types. C is strict.
	// We should generate `typedef` for function signatures?
	// Or cast to specific signature matching method params.

	methodType := iface.Methods[mem.Member.Value]

	// Build function pointer type string
	// retType (*)(MorphContext*, void*, arg1Type, arg2Type)
	retType, _ := c.mapTypeToC(nil, prefix) // void default?
	if len(methodType.ReturnTypes) > 0 {
		retType, _ = c.mapCheckerTypeToC(methodType.ReturnTypes[0], prefix), ""
	}

	var paramTypes []string
	paramTypes = append(paramTypes, "MorphContext*")
	paramTypes = append(paramTypes, "void*") // 'this'

	for _, p := range methodType.Parameters {
		pt := c.mapCheckerTypeToC(p, prefix)
		paramTypes = append(paramTypes, pt)
	}

	fnPtrType := fmt.Sprintf("%s (*)(%s)", retType, strings.Join(paramTypes, ", "))

	callCode := fmt.Sprintf("((%s)(%s).vtable[%d])(%s)", fnPtrType, objCode, methodIndex, strings.Join(args, ", "))
	return callCode, nil
}

func (c *Compiler) compileTypeAssertion(call *parser.CallExpression, prefix string) (string, error) {
	if len(call.Arguments) != 2 {
		return "", fmt.Errorf("assert expects 2 arguments: interface and type")
	}

	ifaceCode, err := c.compileExpression(call.Arguments[0], prefix)
	if err != nil { return "", err }

	// Arg 2 must be type name (Identifier) representing a struct?
	// `pkg/checker` resolves types. But here `call.Arguments[1]` is an Expression.
	// Ideally `assert(i, User)` -> `User` is parsed as Type?
	// No, `User` is identifier. Checker resolves it to Type?
	// If `User` is a type name, `Identifier` lookup returns Type?
	// `checker.checkExpression` handles Identifier: if it's a type, it returns Type.
	// So `c.checker.Types[arg1]` should be the type we want.

	targetType := c.checker.Types[call.Arguments[1]]
	if targetType == nil {
		return "", fmt.Errorf("unknown type for assertion")
	}

	// Must be struct
	if targetType.Kind() != checker.KindStruct {
		return "", fmt.Errorf("can only assert to struct type")
	}
	st := targetType.(*checker.StructType)

	targetID := c.getStructID(st.Name)
	targetCType := c.mapCheckerTypeToC(st, prefix)

	return fmt.Sprintf("(%s)mph_assert_type(ctx, %s, %d)", targetCType, ifaceCode, targetID), nil
}

func (c *Compiler) compileDelete(call *parser.CallExpression, prefix string) (string, error) {
	if len(call.Arguments) != 2 {
		return "", fmt.Errorf("hapus expects 2 arguments")
	}

	mapCode, err := c.compileExpression(call.Arguments[0], prefix)
	if err != nil { return "", err }

	keyCode, err := c.compileExpression(call.Arguments[1], prefix)
	if err != nil { return "", err }

	mapType := c.checker.Types[call.Arguments[0]]
	if mapType == nil || mapType.Kind() != checker.KindMap {
		return "", fmt.Errorf("hapus arg 1 must be map")
	}
	mt := mapType.(*checker.MapType)

	keyCast := fmt.Sprintf("(void*)%s", keyCode)
	if mt.Key.Kind() == checker.KindInt {
		keyCast = fmt.Sprintf("(void*)(int64_t)%s", keyCode)
	}

	return fmt.Sprintf("mph_map_delete(ctx, %s, %s)", mapCode, keyCast), nil
}

func (c *Compiler) compileLen(call *parser.CallExpression, prefix string) (string, error) {
	if len(call.Arguments) != 1 {
		return "", fmt.Errorf("panjang expects 1 argument")
	}

	argCode, err := c.compileExpression(call.Arguments[0], prefix)
	if err != nil { return "", err }

	argType := c.checker.Types[call.Arguments[0]]
	if argType == nil {
		return "", fmt.Errorf("unknown type for panjang")
	}

	if argType.Kind() == checker.KindMap {
		return fmt.Sprintf("mph_map_len(ctx, %s)", argCode), nil
	} else if argType.Kind() == checker.KindArray {
		// Array len access
		return fmt.Sprintf("((MorphArray*)%s)->length", argCode), nil
	} else if argType.Kind() == checker.KindString {
		return fmt.Sprintf("((MorphString*)%s)->length", argCode), nil
	}

	return "", fmt.Errorf("panjang not supported for type %s", argType.String())
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
	c.getStructID(name) // Register ID

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
		return "MorphMap*", nil
	}
	return "", fmt.Errorf("unknown type node: %T", t)
}

func (c *Compiler) compileInterfaceConversion(iface *checker.InterfaceType, st *checker.StructType, srcCode string, prefix string) (string, error) {
	// 2. Generate Static VTable (if needed - currently we generate it locally or globally?)
	// Global is better to avoid dupes, but for MVP local/expression is fine if we use Statement Expression?
	// No, VTable array must be static/global.
	// We can inject it into `output` before `entry_point`.
	// But `output` is currently being written to.
	// We should buffer definitions or write them at top.
	// HACK: For MVP, let's use a Statement Expression with static array inside?
	// `({ static void* vt[] = { ... }; vt; })` - Valid in GCC.

	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString("static void* _vt[] = { ")

	// 3. Populate VTable
	// Order matches Interface Methods
	// We need to sort methods by name to ensure consistent order?
	// `pkg/checker` stores `Methods map[string]*FunctionType`. Map order is random!
	// We need deterministic order.
	// Does `InterfaceType` have ordered keys? No.
	// We must sort keys.
	// `Checker` doesn't expose sorted keys. We should sort them here.

	// Wait, `InterfaceType` in `checker` doesn't have order.
	// We need to define canonical order. Alphabetical?
	// Yes, assume alphabetical for vtable slots.
	// We also need to mangle struct method names correctly.

	// Sort Interface Methods
	ifaceMethods := make([]string, 0, len(iface.Methods))
	for k := range iface.Methods {
		ifaceMethods = append(ifaceMethods, k)
	}
	// Sort ifaceMethods ... need "sort" package
	// Assuming simple sort.
	for i := 0; i < len(ifaceMethods); i++ {
		for j := i + 1; j < len(ifaceMethods); j++ {
			if ifaceMethods[i] > ifaceMethods[j] {
				ifaceMethods[i], ifaceMethods[j] = ifaceMethods[j], ifaceMethods[i]
			}
		}
	}

	for i, mName := range ifaceMethods {
		if i > 0 { sb.WriteString(", ") }
		// Find struct method
		// Struct methods are mangled as `mph_StructName_MethodName`.
		// If struct is imported, it's `mph_Pkg_StructName_MethodName`.
		// We need logic to resolve struct method name.
		structMethodName := fmt.Sprintf("%s%s_%s", prefix, st.Name, mName)
		if st.Module != "" {
			structMethodName = mangle(st.Module) + st.Name + "_" + mName
		}
		sb.WriteString("(void*)" + structMethodName)
	}

	sb.WriteString(" }; ");

	// 4. Create Interface Struct
	// (MorphInterface){ .instance = (void*)src, .vtable = _vt, .type_id = ID }
	structID := c.getStructID(st.Name)
	sb.WriteString(fmt.Sprintf("(MorphInterface){ .instance = (void*)%s, .vtable = _vt, .type_id = %d };", srcCode, structID))
	sb.WriteString(" })")

	return sb.String(), nil
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
			if st.Module != "" {
				return mangle(st.Module) + st.Name + "*"
			}
			return "mph_" + st.Name + "*"
		}
	case checker.KindArray:
		return "MorphArray*"
	case checker.KindMap:
		return "MorphMap*"
	case checker.KindInterface:
		return "MorphInterface" // Value type
	}
	return "mph_int" // Fallback
}
