package compiler

import (
	"fmt"
	"sort"
	"strings"

	"github.com/VzoelFox/morph/pkg/checker"
	"github.com/VzoelFox/morph/pkg/parser"
)

type Compiler struct {
	output    strings.Builder
	typeDefs  strings.Builder
	funcDefs  strings.Builder
	entryBody strings.Builder

	checker   *checker.Checker
	hasMain   bool
	StructIDs map[string]int

	captures map[*parser.FunctionLiteral][]string
}

func New(c *checker.Checker) *Compiler {
	return &Compiler{
		checker:   c,
		StructIDs: make(map[string]int),
		captures:  make(map[*parser.FunctionLiteral][]string),
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

// getAnonFuncName generates a consistent unique name for anonymous functions
func (c *Compiler) getAnonFuncName(fn *parser.FunctionLiteral) string {
	if fn.Name != "" {
		return fn.Name
	}
	return fmt.Sprintf("anon_%p", fn)
}

func (c *Compiler) Compile(node parser.Node) (string, error) {
	c.output.Reset()
	c.typeDefs.Reset()
	c.funcDefs.Reset()
	c.entryBody.Reset()
	c.captures = make(map[*parser.FunctionLiteral][]string)

	c.output.WriteString("#include \"morph.h\"\n\n")

	c.output.WriteString("// Native bindings\n")
	c.output.WriteString("void mph_native_print(MorphContext* ctx, MorphString* s);\n")
	c.output.WriteString("void mph_native_print_int(MorphContext* ctx, mph_int n);\n\n")

	// 0. Pre-Pass: Analyze Captures
	if err := c.analyzeCaptures(node); err != nil {
		return "", err
	}

	// 1. Compile Struct Types
	if err := c.compileStructTypes(node); err != nil {
		return "", err
	}

	c.output.WriteString("// Type IDs\n")
	for name, id := range c.StructIDs {
		c.output.WriteString(fmt.Sprintf("#define MPH_TYPE_%s %d\n", name, id))
	}
	c.output.WriteString("\n")

	// 2. Compile Modules
	for path, mod := range c.checker.ModuleCache {
		if mod.Program == nil {
			continue
		}
		prefix := mangle(path)
		if err := c.compileModule(mod.Program, prefix); err != nil {
			return "", err
		}
	}

	// 3. Compile Main Program
	if prog, ok := node.(*parser.Program); ok {
		if err := c.compileModule(prog, "mph_"); err != nil {
			return "", err
		}
	} else {
		return "", fmt.Errorf("expected *parser.Program, got %T", node)
	}

	c.output.WriteString("// Struct Definitions (Env & Types)\n")
	c.output.WriteString(c.typeDefs.String())
	c.output.WriteString("\n")

	c.output.WriteString("// Function Definitions\n")
	c.output.WriteString(c.funcDefs.String())
	c.output.WriteString("\n")

	c.output.WriteString("// Entry Point\n")
	c.output.WriteString("void morph_entry_point(MorphContext* ctx) {\n")
	if c.hasMain {
		// Call main with NULL env
		c.output.WriteString("\tmph_main(ctx, NULL);\n")
	}
	c.output.WriteString(c.entryBody.String())
	c.output.WriteString("}\n")

	return c.output.String(), nil
}

// --- Analysis Phase ---

func (c *Compiler) analyzeCaptures(node parser.Node) error {
	var walker func(n parser.Node)
	walker = func(n parser.Node) {
		if n == nil { return }

		if fn, ok := n.(*parser.FunctionLiteral); ok {
			free := c.getFreeVars(fn)
			if len(free) > 0 {
				sort.Strings(free)
				c.captures[fn] = free
			}
		}

		switch t := n.(type) {
		case *parser.Program:
			for _, s := range t.Statements { walker(s) }
		case *parser.BlockStatement:
			for _, s := range t.Statements { walker(s) }
		case *parser.ExpressionStatement:
			walker(t.Expression)
		case *parser.VarStatement:
			for _, v := range t.Values { walker(v) }
		case *parser.ReturnStatement:
			for _, v := range t.ReturnValues { walker(v) }
		case *parser.AssignmentStatement:
			for _, v := range t.Values { walker(v) }
		case *parser.IfExpression:
			walker(t.Condition)
			walker(t.Consequence)
			walker(t.Alternative)
		case *parser.WhileExpression:
			walker(t.Condition)
			walker(t.Body)
		case *parser.FunctionLiteral:
			walker(t.Body)
		case *parser.CallExpression:
			walker(t.Function)
			for _, a := range t.Arguments { walker(a) }
		case *parser.InfixExpression:
			walker(t.Left); walker(t.Right)
		case *parser.PrefixExpression:
			walker(t.Right)
		case *parser.IndexExpression:
			walker(t.Left); walker(t.Index)
		case *parser.MemberExpression:
			walker(t.Object)
		case *parser.StructLiteral:
			for _, v := range t.Fields { walker(v) }
		case *parser.ArrayLiteral:
			for _, v := range t.Elements { walker(v) }
		case *parser.HashLiteral:
			for k, v := range t.Pairs { walker(k); walker(v) }
		case *parser.InterpolatedString:
			for _, p := range t.Parts { walker(p) }
		}
	}

	walker(node)
	return nil
}

func (c *Compiler) getFreeVars(fn *parser.FunctionLiteral) []string {
	used := make(map[string]bool)
	defined := make(map[string]bool)

	// Register Parameters as Defined
	for _, p := range fn.Parameters {
		defined[p.Name.Value] = true
	}

	var walkBody func(n parser.Node)
	walkBody = func(n parser.Node) {
		if n == nil { return }
		switch t := n.(type) {
		case *parser.Identifier:
			used[t.Value] = true
		case *parser.VarStatement:
			for _, name := range t.Names {
				defined[name.Value] = true
			}
			for _, val := range t.Values { walkBody(val) }
		case *parser.FunctionLiteral:
			// Recurse to find free vars in nested function
			nestedFree := c.getFreeVars(t)
			for _, v := range nestedFree {
				used[v] = true
			}
		case *parser.MemberExpression:
			walkBody(t.Object)
		case *parser.BlockStatement:
			for _, s := range t.Statements { walkBody(s) }
		case *parser.ExpressionStatement: walkBody(t.Expression)
		case *parser.ReturnStatement: for _, v := range t.ReturnValues { walkBody(v) }
		case *parser.AssignmentStatement:
			for _, name := range t.Names {
				if ident, ok := name.(*parser.Identifier); ok {
					used[ident.Value] = true // Usage (L-Value) needs definition or capture
				} else {
					walkBody(name)
				}
			}
			for _, val := range t.Values { walkBody(val) }
		case *parser.CallExpression:
			walkBody(t.Function)
			for _, a := range t.Arguments { walkBody(a) }
		case *parser.InfixExpression:
			walkBody(t.Left); walkBody(t.Right)
		case *parser.PrefixExpression:
			walkBody(t.Right)
		case *parser.IndexExpression:
			walkBody(t.Left); walkBody(t.Index)
		case *parser.StructLiteral:
			for _, v := range t.Fields { walkBody(v) }
		case *parser.ArrayLiteral:
			for _, v := range t.Elements { walkBody(v) }
		case *parser.HashLiteral:
			for k, v := range t.Pairs { walkBody(k); walkBody(v) }
		case *parser.InterpolatedString:
			for _, p := range t.Parts { walkBody(p) }
		case *parser.IfExpression:
			walkBody(t.Condition); walkBody(t.Consequence); walkBody(t.Alternative)
		case *parser.WhileExpression:
			walkBody(t.Condition); walkBody(t.Body)
		}
	}

	walkBody(fn.Body)

	var free []string
	for v := range used {
		if !defined[v] {
			free = append(free, v)
		}
	}
	return free
}

// --- Compilation ---

func (c *Compiler) compileModule(prog *parser.Program, prefix string) error {
	for _, stmt := range prog.Statements {
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

		if !isFunc && prefix == "mph_" {
			if err := c.compileStatement(stmt, &c.entryBody, prefix, nil); err != nil {
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
		if err != nil { return err }
	}

	funcName := c.getAnonFuncName(fn)
	if fn.Receiver != nil {
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

	// 1. Generate Env Struct
	captures := c.captures[fn]
	envTypeName := fmt.Sprintf("Env_%s_%s", prefix, funcName)

	if len(captures) > 0 {
		c.typeDefs.WriteString(fmt.Sprintf("typedef struct %s {\n", envTypeName))
		for _, name := range captures {
			typ := c.findTypeForName(fn.Body, name)
			cType := c.mapCheckerTypeToC(typ, prefix)
			c.typeDefs.WriteString(fmt.Sprintf("\t%s %s;\n", cType, name))
		}
		c.typeDefs.WriteString(fmt.Sprintf("} %s;\n\n", envTypeName))
	}

	// 2. Generate Function Definition
	c.funcDefs.WriteString(fmt.Sprintf("%s %s%s(MorphContext* ctx, void* _env_void", retType, prefix, funcName))

	if fn.Receiver != nil {
		cType, err := c.mapTypeToC(fn.Receiver.Type, prefix)
		if err != nil { return err }
		c.funcDefs.WriteString(fmt.Sprintf(", %s %s", cType, fn.Receiver.Name.Value))
	}

	for _, p := range fn.Parameters {
		cType, err := c.mapTypeToC(p.Type, prefix)
		if err != nil { return err }
		c.funcDefs.WriteString(fmt.Sprintf(", %s %s", cType, p.Name.Value))
	}

	c.funcDefs.WriteString(")")

	if fn.IsNative {
		c.funcDefs.WriteString(";\n\n")
		return nil
	}

	c.funcDefs.WriteString(" {\n")

	// 3. Cast Env
	if len(captures) > 0 {
		c.funcDefs.WriteString(fmt.Sprintf("\t%s* _env = (%s*)_env_void;\n", envTypeName, envTypeName))
	}

	var body strings.Builder
	if err := c.compileBlock(fn.Body, &body, prefix, fn); err != nil {
		return err
	}
	c.funcDefs.WriteString(body.String())
	c.funcDefs.WriteString("}\n\n")
	return nil
}

func (c *Compiler) findTypeForName(body parser.Statement, name string) checker.Type {
	var found checker.Type
	var finder func(n parser.Node)
	finder = func(n parser.Node) {
		if found != nil || n == nil { return }
		if ident, ok := n.(*parser.Identifier); ok {
			if ident.Value == name {
				found = c.checker.Types[ident]
			}
		}
		switch t := n.(type) {
		case *parser.BlockStatement: for _, s := range t.Statements { finder(s) }
		case *parser.ExpressionStatement: finder(t.Expression)
		case *parser.VarStatement: for _, v := range t.Values { finder(v) }
		case *parser.ReturnStatement: for _, v := range t.ReturnValues { finder(v) }
		case *parser.AssignmentStatement: for _, v := range t.Values { finder(v) }
		case *parser.IfExpression: finder(t.Condition); finder(t.Consequence); finder(t.Alternative)
		case *parser.WhileExpression: finder(t.Condition); finder(t.Body)
		case *parser.CallExpression: finder(t.Function); for _, a := range t.Arguments { finder(a) }
		case *parser.InfixExpression: finder(t.Left); finder(t.Right)
		case *parser.PrefixExpression: finder(t.Right)
		case *parser.IndexExpression: finder(t.Left); finder(t.Index)
		}
	}
	finder(body)
	if found == nil { return checker.IntType }
	return found
}

func (c *Compiler) compileBlock(block *parser.BlockStatement, buf *strings.Builder, prefix string, currentFn *parser.FunctionLiteral) error {
	for _, stmt := range block.Statements {
		if err := c.compileStatement(stmt, buf, prefix, currentFn); err != nil {
			return err
		}
	}
	return nil
}

func (c *Compiler) compileStatement(stmt parser.Statement, buf *strings.Builder, prefix string, currentFn *parser.FunctionLiteral) error {
	switch s := stmt.(type) {
	case *parser.VarStatement: return c.compileVar(s, buf, prefix, currentFn)
	case *parser.ReturnStatement: return c.compileReturn(s, buf, prefix, currentFn)
	case *parser.AssignmentStatement: return c.compileAssignment(s, buf, prefix, currentFn)
	case *parser.BlockStatement:
		buf.WriteString("{\n")
		if err := c.compileBlock(s, buf, prefix, currentFn); err != nil { return err }
		buf.WriteString("}\n")
		return nil
	case *parser.ExpressionStatement:
		if ifExpr, ok := s.Expression.(*parser.IfExpression); ok {
			return c.compileIf(ifExpr, buf, prefix, currentFn)
		}
		if whileExpr, ok := s.Expression.(*parser.WhileExpression); ok {
			return c.compileWhile(whileExpr, buf, prefix, currentFn)
		}
		exprCode, err := c.compileExpression(s.Expression, prefix, currentFn)
		if err != nil { return err }
		buf.WriteString(fmt.Sprintf("\t%s;\n", exprCode))
		return nil
	default: return nil
	}
}

func (c *Compiler) compileVar(s *parser.VarStatement, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	if len(s.Names) != 1 || len(s.Values) != 1 { return fmt.Errorf("multi-var not supported") }
	name := s.Names[0].Value
	valCode, err := c.compileExpression(s.Values[0], prefix, fn)
	if err != nil { return err }

	var destType checker.Type
	if s.Type != nil {
		destType = c.checker.Types[s.Type]
	} else {
		destType = c.checker.Types[s.Names[0]]
		if destType == nil { destType = c.checker.Types[s.Values[0]] }
	}

	// Interface Conversion check
	srcType := c.checker.Types[s.Values[0]]
	if destType != nil && destType.Kind() == checker.KindInterface && srcType != nil && srcType.Kind() == checker.KindStruct {
		valCode, err = c.compileInterfaceConversion(destType.(*checker.InterfaceType), srcType.(*checker.StructType), valCode, prefix)
		if err != nil { return err }
	}

	cType := "mph_int"
	if destType != nil {
		cType = c.mapCheckerTypeToC(destType, prefix)
	}

	buf.WriteString(fmt.Sprintf("\t%s %s = %s;\n", cType, name, valCode))
	return nil
}

func (c *Compiler) compileAssignment(s *parser.AssignmentStatement, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	valCode, err := c.compileExpression(s.Values[0], prefix, fn)
	if err != nil { return err }

	// Interface Conversion
	srcType := c.checker.Types[s.Values[0]]
	destType := c.checker.Types[s.Names[0]]
	if destType != nil && destType.Kind() == checker.KindInterface && srcType != nil && srcType.Kind() == checker.KindStruct {
		valCode, err = c.compileInterfaceConversion(destType.(*checker.InterfaceType), srcType.(*checker.StructType), valCode, prefix)
		if err != nil { return err }
	}

	if ident, ok := s.Names[0].(*parser.Identifier); ok {
		target := ident.Value
		if c.isCaptured(target, fn) {
			target = fmt.Sprintf("_env->%s", target)
		}
		buf.WriteString(fmt.Sprintf("\t%s = %s;\n", target, valCode))
		return nil
	} else if mem, ok := s.Names[0].(*parser.MemberExpression); ok {
		objCode, err := c.compileExpression(mem.Object, prefix, fn)
		if err != nil { return err }
		buf.WriteString(fmt.Sprintf("\t%s->%s = %s;\n", objCode, mem.Member.Value, valCode))
		return nil
	} else if idx, ok := s.Names[0].(*parser.IndexExpression); ok {
		// Map/Array Assignment
		objCode, err := c.compileExpression(idx.Left, prefix, fn)
		if err != nil { return err }
		idxCode, err := c.compileExpression(idx.Index, prefix, fn)
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
		} else if leftType != nil && leftType.Kind() == checker.KindArray {
			at := leftType.(*checker.ArrayType)
			elemCType := c.mapCheckerTypeToC(at.Element, prefix)
			buf.WriteString(fmt.Sprintf("\t((%s*)_a->data)[%s] = %s;\n", elemCType, idxCode, valCode))
			return nil
		}
	}
	return fmt.Errorf("complex assignment not supported")
}

func (c *Compiler) isCaptured(name string, fn *parser.FunctionLiteral) bool {
	if fn == nil { return false }
	list := c.captures[fn]
	for _, v := range list {
		if v == name { return true }
	}
	return false
}

func (c *Compiler) compileReturn(s *parser.ReturnStatement, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	if len(s.ReturnValues) == 0 {
		buf.WriteString("\treturn;\n")
		return nil
	}
	valCode, err := c.compileExpression(s.ReturnValues[0], prefix, fn)
	if err != nil { return err }
	buf.WriteString(fmt.Sprintf("\treturn %s;\n", valCode))
	return nil
}

func (c *Compiler) compileIf(ie *parser.IfExpression, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	cond, err := c.compileExpression(ie.Condition, prefix, fn)
	if err != nil { return err }
	buf.WriteString(fmt.Sprintf("\tif (%s) {\n", cond))
	c.compileStatement(ie.Consequence, buf, prefix, fn)
	buf.WriteString("\t}")
	if ie.Alternative != nil {
		buf.WriteString(" else {\n")
		c.compileStatement(ie.Alternative, buf, prefix, fn)
		buf.WriteString("\t}\n")
	} else { buf.WriteString("\n") }
	return nil
}

func (c *Compiler) compileWhile(we *parser.WhileExpression, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	cond, err := c.compileExpression(we.Condition, prefix, fn)
	if err != nil { return err }
	buf.WriteString(fmt.Sprintf("\twhile (%s) {\n", cond))
	c.compileStatement(we.Body, buf, prefix, fn)
	buf.WriteString("\t}\n")
	return nil
}

func (c *Compiler) compileExpression(expr parser.Expression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	switch e := expr.(type) {
	case *parser.Identifier:
		name := e.Value
		if c.isCaptured(name, fn) {
			return fmt.Sprintf("_env->%s", name), nil
		}
		return name, nil
	case *parser.FunctionLiteral:
		return c.compileFunctionLiteral(e, prefix, fn)
	case *parser.CallExpression:
		if ident, ok := e.Function.(*parser.Identifier); ok && ident.Value == "assert" {
			return c.compileTypeAssertion(e, prefix, fn)
		}
		return c.compileCall(e, prefix, fn)
	case *parser.StringLiteral:
		escaped := strings.ReplaceAll(e.Value, "\n", "\\n")
		escaped = strings.ReplaceAll(escaped, "\"", "\\\"")
		return fmt.Sprintf("mph_string_new(ctx, \"%s\")", escaped), nil
	case *parser.IntegerLiteral:
		return fmt.Sprintf("%d", e.Value), nil
	case *parser.BooleanLiteral:
		if e.Value { return "1", nil }
		return "0", nil
	case *parser.InfixExpression:
		return c.compileInfix(e, prefix, fn)
	case *parser.PrefixExpression:
		right, err := c.compileExpression(e.Right, prefix, fn)
		if err != nil { return "", err }
		return fmt.Sprintf("(%s%s)", e.Operator, right), nil
	case *parser.StructLiteral:
		return c.compileStructLiteral(e, prefix, fn)
	case *parser.ArrayLiteral:
		return c.compileArrayLiteral(e, prefix, fn)
	case *parser.HashLiteral:
		return c.compileHashLiteral(e, prefix, fn)
	case *parser.IndexExpression:
		return c.compileIndex(e, prefix, fn)
	case *parser.MemberExpression:
		obj, err := c.compileExpression(e.Object, prefix, fn)
		if err != nil { return "", err }
		return fmt.Sprintf("%s->%s", obj, e.Member.Value), nil
	case *parser.InterpolatedString:
		return c.compileInterpolatedString(e, prefix, fn)
	default:
		return "", fmt.Errorf("unsupported expr: %T", expr)
	}
}

func (c *Compiler) compileFunctionLiteral(fn *parser.FunctionLiteral, prefix string, parentFn *parser.FunctionLiteral) (string, error) {
	if err := c.compileFunction(fn, prefix); err != nil { return "", err }
	funcName := c.getAnonFuncName(fn)
	envTypeName := fmt.Sprintf("Env_%s_%s", prefix, funcName)
	captures := c.captures[fn]

	var sb strings.Builder
	sb.WriteString("({ ")
	if len(captures) > 0 {
		sb.WriteString(fmt.Sprintf("%s* _e = (%s*)mph_alloc(ctx, sizeof(%s)); ", envTypeName, envTypeName, envTypeName))
		for _, name := range captures {
			val := name
			if c.isCaptured(name, parentFn) {
				val = fmt.Sprintf("_env->%s", name)
			}
			sb.WriteString(fmt.Sprintf("_e->%s = %s; ", name, val))
		}
		sb.WriteString(fmt.Sprintf("mph_closure_new(ctx, (void*)%s%s, (void*)_e); ", prefix, funcName))
	} else {
		sb.WriteString(fmt.Sprintf("mph_closure_new(ctx, (void*)%s%s, NULL); ", prefix, funcName))
	}
	sb.WriteString(" })")
	return sb.String(), nil
}

func (c *Compiler) compileCall(call *parser.CallExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	var funcCode string
	isDirect := false

	if ident, ok := call.Function.(*parser.Identifier); ok {
		if ident.Value == "native_print" { return c.compileBuiltin(call, "mph_native_print", prefix, fn) }
		if ident.Value == "native_print_int" { return c.compileBuiltin(call, "mph_native_print_int", prefix, fn) }
		if ident.Value == "saluran_baru" { return c.compileBuiltin(call, "mph_channel_new", prefix, fn) }
		if ident.Value == "kirim" { return c.compileBuiltin(call, "mph_channel_send", prefix, fn) }
		if ident.Value == "terima" { return c.compileBuiltin(call, "mph_channel_recv", prefix, fn) }
		if ident.Value == "luncurkan" { return c.compileSpawn(call, prefix, fn) }
		if ident.Value == "hapus" { return c.compileDelete(call, prefix, fn) }
		if ident.Value == "panjang" { return c.compileLen(call, prefix, fn) }

		funcCode = ident.Value
		if c.isCaptured(funcCode, fn) {
			funcCode = fmt.Sprintf("_env->%s", funcCode)
		} else if !c.isLocal(funcCode, fn) && !isBuiltin(funcCode) {
			funcCode = "mph_" + ident.Value
			isDirect = true
		}
	} else if mem, ok := call.Function.(*parser.MemberExpression); ok {
		objType := c.checker.Types[mem.Object]
		if objType != nil && objType.Kind() == checker.KindModule {
			modType := objType.(*checker.ModuleType)
			funcCode = mangle(modType.Name) + mem.Member.Value
			isDirect = true
		} else if objType != nil && objType.Kind() == checker.KindInterface {
			return c.compileInterfaceCall(call, mem, objType.(*checker.InterfaceType), prefix, fn)
		} else {
			return "", fmt.Errorf("method calls not supported yet")
		}
	} else {
		var err error
		funcCode, err = c.compileExpression(call.Function, prefix, fn)
		if err != nil { return "", err }
	}

	var args []string
	args = append(args, "ctx")

	if isDirect {
		args = append(args, "NULL") // Env
	} else {
		// Closure Call: ((MorphClosureFunc)c->function)(ctx, c->env, ...)
		args = append(args, fmt.Sprintf("%s->env", funcCode))
	}

	for _, arg := range call.Arguments {
		ac, err := c.compileExpression(arg, prefix, fn)
		if err != nil { return "", err }
		args = append(args, ac)
	}

	if isDirect {
		return fmt.Sprintf("%s(%s)", funcCode, strings.Join(args, ", ")), nil
	} else {
		return fmt.Sprintf("((MorphClosureFunc)%s->function)(%s)", funcCode, strings.Join(args, ", ")), nil
	}
}

func (c *Compiler) isLocal(name string, fn *parser.FunctionLiteral) bool {
	if fn == nil { return false }
	for _, p := range fn.Parameters {
		if p.Name.Value == name { return true }
	}
	return false
}

func isBuiltin(name string) bool {
	return name == "native_print" || name == "native_print_int" || name == "len" || name == "hapus" || name == "panjang"
}

func (c *Compiler) compileBuiltin(call *parser.CallExpression, cName string, prefix string, fn *parser.FunctionLiteral) (string, error) {
	var args []string
	args = append(args, "ctx")
	for _, arg := range call.Arguments {
		ac, err := c.compileExpression(arg, prefix, fn)
		if err != nil { return "", err }
		args = append(args, ac)
	}
	return fmt.Sprintf("%s(%s)", cName, strings.Join(args, ", ")), nil
}

func (c *Compiler) compileInfix(ie *parser.InfixExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	left, err := c.compileExpression(ie.Left, prefix, fn)
	if err != nil { return "", err }
	right, err := c.compileExpression(ie.Right, prefix, fn)
	if err != nil { return "", err }

	if ie.Operator == "+" {
		if c.checker.Types[ie.Left].Kind() == checker.KindString {
			return fmt.Sprintf("mph_string_concat(ctx, %s, %s)", left, right), nil
		}
	}
	if ie.Operator == "==" {
		if c.checker.Types[ie.Left].Kind() == checker.KindString {
			return fmt.Sprintf("mph_string_eq(%s, %s)", left, right), nil
		}
	}
	return fmt.Sprintf("(%s %s %s)", left, ie.Operator, right), nil
}

func (c *Compiler) compileInterpolatedString(is *parser.InterpolatedString, prefix string, fn *parser.FunctionLiteral) (string, error) {
	if len(is.Parts) == 0 {
		return "mph_string_new(ctx, \"\")", nil
	}
	current, err := c.compileExpression(is.Parts[0], prefix, fn)
	if err != nil { return "", err }
	for _, part := range is.Parts[1:] {
		next, err := c.compileExpression(part, prefix, fn)
		if err != nil { return "", err }
		current = fmt.Sprintf("mph_string_concat(ctx, %s, %s)", current, next)
	}
	return current, nil
}

func (c *Compiler) mapCheckerTypeToC(t checker.Type, prefix string) string {
	if t == nil { return "mph_int" }
	switch t.Kind() {
	case checker.KindFunction: return "MorphClosure*"
	case checker.KindInt: return "mph_int"
	case checker.KindFloat: return "mph_float"
	case checker.KindBool: return "mph_bool"
	case checker.KindString: return "MorphString*"
	case checker.KindArray: return "MorphArray*"
	case checker.KindMap: return "MorphMap*"
	case checker.KindStruct:
		if st, ok := t.(*checker.StructType); ok {
			if st.Module != "" { return mangle(st.Module) + st.Name + "*" }
			return "mph_" + st.Name + "*"
		}
	case checker.KindInterface: return "MorphInterface"
	}
	return "mph_int"
}

func (c *Compiler) compileStructLiteral(sl *parser.StructLiteral, prefix string, fn *parser.FunctionLiteral) (string, error) {
	var cTypeName string
	if ident, ok := sl.Name.(*parser.Identifier); ok {
		cTypeName = prefix + ident.Value
	} else { return "", fmt.Errorf("complex struct literals not supported yet") }

	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("%s* _t = (%s*)mph_alloc(ctx, sizeof(%s)); ", cTypeName, cTypeName, cTypeName))
	for fieldName, valExpr := range sl.Fields {
		valCode, err := c.compileExpression(valExpr, prefix, fn)
		if err != nil { return "", err }
		sb.WriteString(fmt.Sprintf("_t->%s = %s; ", fieldName, valCode))
	}
	sb.WriteString("_t; })")
	return sb.String(), nil
}

func (c *Compiler) compileArrayLiteral(al *parser.ArrayLiteral, prefix string, fn *parser.FunctionLiteral) (string, error) {
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
		valCode, err := c.compileExpression(el, prefix, fn)
		if err != nil { return "", err }
		sb.WriteString(fmt.Sprintf("((%s*)_a->data)[%d] = %s; ", elemCType, i, valCode))
	}
	sb.WriteString("_a; })")
	return sb.String(), nil
}

func (c *Compiler) compileHashLiteral(hl *parser.HashLiteral, prefix string, fn *parser.FunctionLiteral) (string, error) {
	mapType := c.checker.Types[hl]
	kindEnum := "MPH_KEY_PTR"
	if mapType != nil {
		if mt, ok := mapType.(*checker.MapType); ok {
			if mt.Key.Kind() == checker.KindInt { kindEnum = "MPH_KEY_INT" } else if mt.Key.Kind() == checker.KindString { kindEnum = "MPH_KEY_STRING" }
		}
	}
	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("MorphMap* _m = mph_map_new(ctx, %s); ", kindEnum))
	for key, val := range hl.Pairs {
		keyCode, err := c.compileExpression(key, prefix, fn)
		if err != nil { return "", err }
		valCode, err := c.compileExpression(val, prefix, fn)
		if err != nil { return "", err }
		keyCast := fmt.Sprintf("(void*)%s", keyCode)
		if kindEnum == "MPH_KEY_INT" { keyCast = fmt.Sprintf("(void*)(int64_t)%s", keyCode) }
		sb.WriteString(fmt.Sprintf("mph_map_set(ctx, _m, %s, (void*)%s); ", keyCast, valCode))
	}
	sb.WriteString("_m; })")
	return sb.String(), nil
}

func (c *Compiler) compileIndex(ie *parser.IndexExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	leftCode, err := c.compileExpression(ie.Left, prefix, fn)
	if err != nil { return "", err }
	indexCode, err := c.compileExpression(ie.Index, prefix, fn)
	if err != nil { return "", err }

	leftType := c.checker.Types[ie.Left]
	if leftType == nil { return "", fmt.Errorf("unknown type for index expression") }

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
			if mt.Key.Kind() == checker.KindInt { keyCast = fmt.Sprintf("(void*)(int64_t)%s", indexCode) }

			if c.isPrimitive(mt.Value) {
				return fmt.Sprintf("(%s)(int64_t)mph_map_get(ctx, %s, %s)", valCType, leftCode, keyCast), nil
			} else {
				return fmt.Sprintf("(%s)mph_map_get(ctx, %s, %s)", valCType, leftCode, keyCast), nil
			}
		}
	}
	return "", fmt.Errorf("index op not supported for %s", leftType.String())
}

func (c *Compiler) compileSpawn(call *parser.CallExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	if len(call.Arguments) == 1 {
		if ident, ok := call.Arguments[0].(*parser.Identifier); ok {
			return fmt.Sprintf("mph_thread_spawn((MorphEntryFunction)mph_%s, NULL)", ident.Value), nil
		}
	} else if len(call.Arguments) == 2 {
		if ident, ok := call.Arguments[0].(*parser.Identifier); ok {
			arg, err := c.compileExpression(call.Arguments[1], prefix, fn)
			if err != nil { return "", err }
			return fmt.Sprintf("mph_thread_spawn((MorphEntryFunction)mph_%s, (void*)%s)", ident.Value, arg), nil
		}
	}
	return "", fmt.Errorf("luncurkan expects named function")
}

func (c *Compiler) compileDelete(call *parser.CallExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	if len(call.Arguments) != 2 { return "", fmt.Errorf("hapus expects 2 arguments") }
	mapCode, err := c.compileExpression(call.Arguments[0], prefix, fn)
	if err != nil { return "", err }
	keyCode, err := c.compileExpression(call.Arguments[1], prefix, fn)
	if err != nil { return "", err }
	mapType := c.checker.Types[call.Arguments[0]]
	if mapType == nil || mapType.Kind() != checker.KindMap { return "", fmt.Errorf("hapus arg 1 must be map") }
	mt := mapType.(*checker.MapType)
	keyCast := fmt.Sprintf("(void*)%s", keyCode)
	if mt.Key.Kind() == checker.KindInt { keyCast = fmt.Sprintf("(void*)(int64_t)%s", keyCode) }
	return fmt.Sprintf("mph_map_delete(ctx, %s, %s)", mapCode, keyCast), nil
}

func (c *Compiler) compileLen(call *parser.CallExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	if len(call.Arguments) != 1 { return "", fmt.Errorf("panjang expects 1 argument") }
	argCode, err := c.compileExpression(call.Arguments[0], prefix, fn)
	if err != nil { return "", err }
	argType := c.checker.Types[call.Arguments[0]]
	if argType == nil { return "", fmt.Errorf("unknown type for panjang") }
	if argType.Kind() == checker.KindMap {
		return fmt.Sprintf("mph_map_len(ctx, %s)", argCode), nil
	} else if argType.Kind() == checker.KindArray {
		return fmt.Sprintf("((MorphArray*)%s)->length", argCode), nil
	} else if argType.Kind() == checker.KindString {
		return fmt.Sprintf("((MorphString*)%s)->length", argCode), nil
	}
	return "", fmt.Errorf("panjang not supported for type %s", argType.String())
}

func (c *Compiler) compileTypeAssertion(call *parser.CallExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	if len(call.Arguments) != 2 { return "", fmt.Errorf("assert expects 2 args") }
	ifaceCode, err := c.compileExpression(call.Arguments[0], prefix, fn)
	if err != nil { return "", err }
	targetType := c.checker.Types[call.Arguments[1]]
	if targetType == nil || targetType.Kind() != checker.KindStruct { return "", fmt.Errorf("invalid assertion type") }
	st := targetType.(*checker.StructType)
	targetID := c.getStructID(st.Name)
	targetCType := c.mapCheckerTypeToC(st, prefix)
	return fmt.Sprintf("(%s)mph_assert_type(ctx, %s, %d)", targetCType, ifaceCode, targetID), nil
}

func (c *Compiler) compileInterfaceConversion(iface *checker.InterfaceType, st *checker.StructType, srcCode string, prefix string) (string, error) {
	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString("static void* _vt[] = { ")
	ifaceMethods := make([]string, 0, len(iface.Methods))
	for k := range iface.Methods { ifaceMethods = append(ifaceMethods, k) }
	sort.Strings(ifaceMethods)
	for i, mName := range ifaceMethods {
		if i > 0 { sb.WriteString(", ") }
		structMethodName := fmt.Sprintf("%s%s_%s", prefix, st.Name, mName)
		if st.Module != "" { structMethodName = mangle(st.Module) + st.Name + "_" + mName }
		sb.WriteString("(void*)" + structMethodName)
	}
	sb.WriteString(" }; ");
	structID := c.getStructID(st.Name)
	sb.WriteString(fmt.Sprintf("(MorphInterface){ .instance = (void*)%s, .vtable = _vt, .type_id = %d };", srcCode, structID))
	sb.WriteString(" })")
	return sb.String(), nil
}

func (c *Compiler) compileInterfaceCall(call *parser.CallExpression, mem *parser.MemberExpression, iface *checker.InterfaceType, prefix string, fn *parser.FunctionLiteral) (string, error) {
	ifaceMethods := make([]string, 0, len(iface.Methods))
	for k := range iface.Methods { ifaceMethods = append(ifaceMethods, k) }
	sort.Strings(ifaceMethods)
	methodIndex := -1
	for i, name := range ifaceMethods {
		if name == mem.Member.Value { methodIndex = i; break }
	}
	if methodIndex == -1 { return "", fmt.Errorf("method not found") }

	objCode, err := c.compileExpression(mem.Object, prefix, fn)
	if err != nil { return "", err }

	var args []string
	args = append(args, "ctx")
	args = append(args, fmt.Sprintf("(%s).instance", objCode))
	for _, arg := range call.Arguments {
		code, err := c.compileExpression(arg, prefix, fn)
		if err != nil { return "", err }
		args = append(args, code)
	}

	methodType := iface.Methods[mem.Member.Value]
	retType, _ := c.mapTypeToC(nil, prefix), ""
	if len(methodType.ReturnTypes) > 0 { retType, _ = c.mapCheckerTypeToC(methodType.ReturnTypes[0], prefix), "" }
	var paramTypes []string
	paramTypes = append(paramTypes, "MorphContext*")
	paramTypes = append(paramTypes, "void*")
	for _, p := range methodType.Parameters { paramTypes = append(paramTypes, c.mapCheckerTypeToC(p, prefix)) }
	fnPtrType := fmt.Sprintf("%s (*)(%s)", retType, strings.Join(paramTypes, ", "))

	return fmt.Sprintf("((%s)(%s).vtable[%d])(%s)", fnPtrType, objCode, methodIndex, strings.Join(args, ", ")), nil
}

func (c *Compiler) isPrimitive(t checker.Type) bool {
	if t == nil { return true }
	switch t.Kind() {
	case checker.KindInt, checker.KindFloat, checker.KindBool: return true
	}
	return false
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
			c.typeDefs.WriteString(fmt.Sprintf("typedef struct %s %s;\n", name, name))
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

	c.typeDefs.WriteString(fmt.Sprintf("struct %s {\n", name))

	for _, field := range s.Fields {
		cType, err := c.mapTypeToC(field.Type, prefix)
		if err != nil {
			return err
		}
		c.typeDefs.WriteString(fmt.Sprintf("\t%s %s;\n", cType, field.Name))
	}

	c.typeDefs.WriteString("};\n\n")
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
	case *parser.FunctionType:
		// Function types mapped to Closure*
		return "MorphClosure*", nil
	}
	return "", fmt.Errorf("unknown type node: %T", t)
}
