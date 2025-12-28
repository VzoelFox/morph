package compiler

import (
	"fmt"
	"reflect"
	"sort"
	"strconv"
	"strings"

	"github.com/VzoelFox/morph/pkg/checker"
	"github.com/VzoelFox/morph/pkg/parser"
)

type Compiler struct {
	output     strings.Builder
	typeDefs   strings.Builder
	globalsDef strings.Builder
	prototypes strings.Builder
	funcDefs   strings.Builder
	entryBody  strings.Builder

	checker   *checker.Checker
	hasMain   bool
	StructIDs map[string]int

	captures     map[*parser.FunctionLiteral][]string
	globals      map[string]bool
	tupleTypes   map[string]string
	freeVarCache map[*parser.FunctionLiteral][]string

	moduleGlobals  map[*parser.Program]map[string]bool
	currentGlobals map[string]bool
	importOrigins  map[string]string // identifier -> module prefix
	tempIndex      int

	recursionDepth int // Track recursion depth to prevent stack overflow
}

type rootTemp struct {
	cType string
	name  string
	value string
}

func New(c *checker.Checker) *Compiler {
	return &Compiler{
		checker:        c,
		StructIDs:      make(map[string]int),
		captures:       make(map[*parser.FunctionLiteral][]string),
		globals:        make(map[string]bool),
		tupleTypes:     make(map[string]string),
		freeVarCache:   make(map[*parser.FunctionLiteral][]string),
		moduleGlobals:  make(map[*parser.Program]map[string]bool),
		currentGlobals: make(map[string]bool),
		importOrigins:  make(map[string]string),
		tempIndex:      0,
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
	c.globalsDef.Reset()
	c.prototypes.Reset()
	c.funcDefs.Reset()
	c.entryBody.Reset()
	c.captures = make(map[*parser.FunctionLiteral][]string)
	c.moduleGlobals = make(map[*parser.Program]map[string]bool)
	c.tempIndex = 0

	c.output.WriteString("#include \"morph.h\"\n\n")

	c.output.WriteString("// Native bindings\n")
	c.output.WriteString("void mph_native_print(MorphContext* ctx, MorphString* s);\n")
	c.output.WriteString("void mph_native_print_int(MorphContext* ctx, mph_int n);\n")
	c.output.WriteString("mph_int mph_string_index(MorphContext* ctx, MorphString* s, MorphString* sub);\n")
	c.output.WriteString("MorphString* mph_string_trim(MorphContext* ctx, MorphString* s, MorphString* cut);\n")
	c.output.WriteString("MorphArray* mph_string_split(MorphContext* ctx, MorphString* s, MorphString* sep);\n\n")

	// 0. Collect Globals (Pass 1)
	if err := c.collectAllGlobals(node); err != nil {
		return "", err
	}

	// 0. Pre-Pass: Analyze Captures (Pass 2)
	if err := c.analyzeAllCaptures(node); err != nil {
		return "", err
	}

	// 1. Compile Struct Types
	if err := c.compileStructTypes(node); err != nil {
		return "", err
	}

	// 1.1 Generate RTTI for Structs
	if err := c.compileStructRTTI(node); err != nil {
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

	c.output.WriteString("// Global Variables\n")
	c.output.WriteString(c.globalsDef.String())
	c.output.WriteString("\n")

	c.output.WriteString("// Function Prototypes\n")
	c.generatePrototypes(node)
	c.output.WriteString(c.prototypes.String())
	c.output.WriteString("\n")

	c.output.WriteString("// Function Definitions\n")
	c.output.WriteString(c.funcDefs.String())
	c.output.WriteString("\n")

	c.output.WriteString("// Entry Point\n")
	c.output.WriteString("void morph_entry_point(MorphContext* ctx) {\n")
	c.output.WriteString(c.entryBody.String())
	if c.hasMain {
		// Call main with NULL env
		c.output.WriteString("\tmph_main(ctx, NULL);\n")
	}
	c.output.WriteString("}\n")

	return c.output.String(), nil
}

func (c *Compiler) nextTemp(prefix string) string {
	c.tempIndex++
	return fmt.Sprintf("_%s_%d", prefix, c.tempIndex)
}

func (c *Compiler) wrapWithRoots(roots []rootTemp, expr string, retType string) string {
	if len(roots) == 0 {
		return expr
	}
	var sb strings.Builder
	sb.WriteString("({ ")
	for _, root := range roots {
		sb.WriteString(fmt.Sprintf("%s %s = %s; ", root.cType, root.name, root.value))
		sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s); ", root.name))
	}
	if retType == "void" {
		sb.WriteString(fmt.Sprintf("%s; ", expr))
		sb.WriteString(fmt.Sprintf("mph_gc_pop_roots(ctx, %d); ", len(roots)))
		sb.WriteString("})")
		return sb.String()
	}

	retTemp := c.nextTemp("ret")
	sb.WriteString(fmt.Sprintf("%s %s = %s; ", retType, retTemp, expr))
	sb.WriteString(fmt.Sprintf("mph_gc_pop_roots(ctx, %d); ", len(roots)))
	sb.WriteString(fmt.Sprintf("%s; })", retTemp))
	return sb.String()
}

// --- Analysis Phase ---

func (c *Compiler) collectAllGlobals(node parser.Node) error {
	// Main
	if prog, ok := node.(*parser.Program); ok {
		c.collectModuleGlobals(prog)
	}
	// Modules
	for _, mod := range c.checker.ModuleCache {
		if mod.Program == nil {
			continue
		}
		c.collectModuleGlobals(mod.Program)
	}
	return nil
}

func (c *Compiler) collectModuleGlobals(prog *parser.Program) {
	globals := make(map[string]bool)
	// Add builtins/natives (Always considered global)
	// We don't add them to 'globals' map to avoid mangling, but check them in isBuiltin
	// globals["native_print"] = true
	// ...

	for _, stmt := range prog.Statements {
		// Collect imports
		if imp, ok := stmt.(*parser.ImportStatement); ok {
			// Basic support: extract name from path "lib/math" -> "math"
			parts := strings.Split(imp.Path, "/")
			name := parts[len(parts)-1]
			globals[name] = true
			// Also add individual imported identifiers (dari...ambil)
			modulePrefix := mangle(name)
			for _, ident := range imp.Identifiers {
				globals[ident] = true
				c.importOrigins[ident] = modulePrefix
			}
		}
		// Collect top-level functions and vars
		if s, ok := stmt.(*parser.ExpressionStatement); ok {
			if fn, ok := s.Expression.(*parser.FunctionLiteral); ok && fn.Name != "" {
				globals[fn.Name] = true
			}
		}
		if vs, ok := stmt.(*parser.VarStatement); ok {
			for _, n := range vs.Names {
				globals[n.Value] = true
			}
		}
	}
	c.moduleGlobals[prog] = globals
}

func (c *Compiler) analyzeAllCaptures(node parser.Node) error {
	// Main
	if prog, ok := node.(*parser.Program); ok {
		c.currentGlobals = c.moduleGlobals[prog]
		if err := c.analyzeCaptures(prog); err != nil {
			return err
		}
	}
	// Modules
	for _, mod := range c.checker.ModuleCache {
		if mod.Program == nil {
			continue
		}
		c.currentGlobals = c.moduleGlobals[mod.Program]
		if err := c.analyzeCaptures(mod.Program); err != nil {
			return err
		}
	}
	return nil
}

func (c *Compiler) analyzeCaptures(node parser.Node) error {
	var walker func(n parser.Node)
	walker = func(n parser.Node) {
		if isNilNode(n) {
			return
		}

		if fn, ok := n.(*parser.FunctionLiteral); ok {
			free := c.getFreeVars(fn)
			if len(free) > 0 {
				sort.Strings(free)
				c.captures[fn] = free
			}
		}

		switch t := n.(type) {
		case *parser.Program:
			for _, s := range t.Statements {
				walker(s)
			}
		case *parser.BlockStatement:
			for _, s := range t.Statements {
				walker(s)
			}
		case *parser.ExpressionStatement:
			walker(t.Expression)
		case *parser.VarStatement:
			for _, v := range t.Values {
				walker(v)
			}
		case *parser.ReturnStatement:
			for _, v := range t.ReturnValues {
				walker(v)
			}
		case *parser.AssignmentStatement:
			for _, v := range t.Values {
				walker(v)
			}
		case *parser.IfExpression:
			walker(t.Condition)
			walker(t.Consequence)
			if t.Alternative != nil {
				walker(t.Alternative)
			}
		case *parser.SwitchStatement:
			walker(t.Condition)
			for _, cc := range t.Cases {
				for _, v := range cc.Values {
					walker(v)
				}
				walker(cc.Body)
			}
			if t.Default != nil {
				walker(t.Default)
			}
		case *parser.WhileExpression:
			walker(t.Condition)
			walker(t.Body)
		case *parser.FunctionLiteral:
			walker(t.Body)
		case *parser.CallExpression:
			walker(t.Function)
			for _, a := range t.Arguments {
				walker(a)
			}
		case *parser.InfixExpression:
			walker(t.Left)
			walker(t.Right)
		case *parser.PrefixExpression:
			walker(t.Right)
		case *parser.IndexExpression:
			walker(t.Left)
			walker(t.Index)
		case *parser.MemberExpression:
			walker(t.Object)
		case *parser.StructLiteral:
			for _, v := range t.Fields {
				walker(v)
			}
		case *parser.ArrayLiteral:
			for _, v := range t.Elements {
				walker(v)
			}
		case *parser.HashLiteral:
			for k, v := range t.Pairs {
				walker(k)
				walker(v)
			}
		case *parser.InterpolatedString:
			for _, p := range t.Parts {
				walker(p)
			}
		}
	}

	walker(node)
	return nil
}

func (c *Compiler) getFreeVars(fn *parser.FunctionLiteral) []string {
	if cached, ok := c.freeVarCache[fn]; ok {
		return cached
	}

	used := make(map[string]bool)
	defined := make(map[string]bool)

	// Register Parameters as Defined
	for _, p := range fn.Parameters {
		defined[p.Name.Value] = true
	}
	// Register Receiver as Defined
	if fn.Receiver != nil {
		defined[fn.Receiver.Name.Value] = true
	}

	var walkBody func(n parser.Node)
	walkBody = func(n parser.Node) {
		if isNilNode(n) {
			return
		}
		switch t := n.(type) {
		case *parser.Identifier:
			used[t.Value] = true
		case *parser.VarStatement:
			for _, name := range t.Names {
				defined[name.Value] = true
			}
			for _, val := range t.Values {
				walkBody(val)
			}
		case *parser.FunctionLiteral:
			// Recurse to find free vars in nested function
			nestedFree := c.getFreeVars(t)
			for _, v := range nestedFree {
				used[v] = true
			}
		case *parser.MemberExpression:
			walkBody(t.Object)
		case *parser.BlockStatement:
			for _, s := range t.Statements {
				walkBody(s)
			}
		case *parser.ExpressionStatement:
			walkBody(t.Expression)
		case *parser.ReturnStatement:
			for _, v := range t.ReturnValues {
				walkBody(v)
			}
		case *parser.AssignmentStatement:
			for _, name := range t.Names {
				if ident, ok := name.(*parser.Identifier); ok {
					used[ident.Value] = true // Usage (L-Value) needs definition or capture
				} else {
					walkBody(name)
				}
			}
			for _, val := range t.Values {
				walkBody(val)
			}
		case *parser.CallExpression:
			walkBody(t.Function)
			for _, a := range t.Arguments {
				walkBody(a)
			}
		case *parser.InfixExpression:
			walkBody(t.Left)
			walkBody(t.Right)
		case *parser.PrefixExpression:
			walkBody(t.Right)
		case *parser.IndexExpression:
			walkBody(t.Left)
			walkBody(t.Index)
		case *parser.StructLiteral:
			for _, v := range t.Fields {
				walkBody(v)
			}
		case *parser.ArrayLiteral:
			for _, v := range t.Elements {
				walkBody(v)
			}
		case *parser.HashLiteral:
			for k, v := range t.Pairs {
				walkBody(k)
				walkBody(v)
			}
		case *parser.InterpolatedString:
			for _, p := range t.Parts {
				walkBody(p)
			}
		case *parser.IfExpression:
			walkBody(t.Condition)
			walkBody(t.Consequence)
			if t.Alternative != nil {
				walkBody(t.Alternative)
			}
		case *parser.SwitchStatement:
			walkBody(t.Condition)
			for _, cc := range t.Cases {
				for _, v := range cc.Values {
					walkBody(v)
				}
				walkBody(cc.Body)
			}
			if t.Default != nil {
				walkBody(t.Default)
			}
		case *parser.WhileExpression:
			walkBody(t.Condition)
			walkBody(t.Body)
		}
	}

	walkBody(fn.Body)

	var free []string
	for v := range used {
		// Only capture if not defined locally, not global in THIS module, and not builtin
		if !defined[v] && !c.currentGlobals[v] && !isBuiltin(v) {
			free = append(free, v)
		}
	}
	c.freeVarCache[fn] = free
	return free
}

// --- Compilation ---

func (c *Compiler) generatePrototypes(node parser.Node) {
	if prog, ok := node.(*parser.Program); ok {
		c.generateModulePrototypes(prog, "mph_")
	}
	for path, mod := range c.checker.ModuleCache {
		if mod.Program == nil {
			continue
		}
		c.generateModulePrototypes(mod.Program, mangle(path))
	}
}

func (c *Compiler) generateModulePrototypes(prog *parser.Program, prefix string) {
	for _, stmt := range prog.Statements {
		if s, ok := stmt.(*parser.ExpressionStatement); ok {
			if fn, ok := s.Expression.(*parser.FunctionLiteral); ok {
				if fn.Name != "" {
					c.generateFunctionPrototype(fn, prefix)
				}
			}
		}
	}
}

func (c *Compiler) generateFunctionPrototype(fn *parser.FunctionLiteral, prefix string) {
	retType := "void"
	if len(fn.ReturnTypes) == 1 {
		retType, _ = c.mapTypeToC(fn.ReturnTypes[0], prefix)
	} else if len(fn.ReturnTypes) > 1 {
		// Resolve types
		var types []checker.Type
		for _, rt := range fn.ReturnTypes {
			t := c.resolveTypeNode(rt)
			types = append(types, t)
		}
		retType = c.getTupleCType(types, prefix)
	}

	funcName := c.getAnonFuncName(fn)
	// Handle methods
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

	c.prototypes.WriteString(fmt.Sprintf("%s %s%s(MorphContext* ctx, void* _env_void", retType, prefix, funcName))

	if fn.Receiver != nil {
		cType, _ := c.mapTypeToC(fn.Receiver.Type, prefix)
		c.prototypes.WriteString(fmt.Sprintf(", %s %s", cType, fn.Receiver.Name.Value))
	}

	for _, p := range fn.Parameters {
		cType, _ := c.mapTypeToC(p.Type, prefix)
		c.prototypes.WriteString(fmt.Sprintf(", %s %s", cType, p.Name.Value))
	}

	c.prototypes.WriteString(");\n")
}

func (c *Compiler) compileModule(prog *parser.Program, prefix string) error {
	// Set current globals context
	c.currentGlobals = c.moduleGlobals[prog]

	for _, stmt := range prog.Statements {
		if vs, ok := stmt.(*parser.VarStatement); ok {
			// Global Variable
			if err := c.compileGlobalVar(vs, prefix); err != nil {
				return err
			}
			continue
		}

		if s, ok := stmt.(*parser.ExpressionStatement); ok {
			if fn, ok := s.Expression.(*parser.FunctionLiteral); ok {
				if fn.Name != "" {
					if err := c.compileFunction(fn, prefix); err != nil {
						return err
					}
					continue
				}
			}
		}

		if prefix == "mph_" {
			// Main module top-level statements
			if err := c.compileStatement(stmt, &c.entryBody, prefix, nil); err != nil {
				return err
			}
		}
	}
	return nil
}

func (c *Compiler) compileGlobalVar(s *parser.VarStatement, prefix string) error {
	// Generate Global C Variable Definitions
	// and Init code in entryBody

	for i, nameIdent := range s.Names {
		name := nameIdent.Value
		var destType checker.Type
		if s.Type != nil {
			destType = c.checker.Types[s.Type]
		} else {
			destType = c.checker.Types[nameIdent]
			if destType == nil {
				destType = c.checker.Types[s.Values[0]]
			}
		}

		cType := "mph_int"
		if destType != nil {
			cType = c.mapCheckerTypeToC(destType, prefix)
		}

		// Definition
		c.globalsDef.WriteString(fmt.Sprintf("%s %s%s;\n", cType, prefix, name))

		// Initialization logic
		var valCode string
		if i < len(s.Values) {
			var err error
			valCode, err = c.compileExpression(s.Values[i], prefix, nil)
			if err != nil {
				return err
			}
		} else {
			// Zero value or error?
			// Checker handles "requires value". If here, valid.
			// But multi-var assign? "var x, y = 1, 2".
			// If single value and multiple names -> Tuple unpacking?
			// But global tuple unpacking not easily supported with simple loop.
			// Handled below.
		}

		// Tuple Unpacking Global: var x, y = f()
		if len(s.Names) > 1 && len(s.Values) == 1 {
			// This case handles ALL names at once.
			// Generate init code only ONCE.
			if i == 0 {
				valCode, err := c.compileExpression(s.Values[0], prefix, nil)
				if err != nil {
					return err
				}

				srcType := c.checker.Types[s.Values[0]]
				mt := srcType.(*checker.MultiType)
				tupleType := c.getTupleCType(mt.Types, prefix)

				// Generate Temp in entryBody
				tmpName := fmt.Sprintf("_t_%p", s)
				c.entryBody.WriteString(fmt.Sprintf("\t%s %s = %s;\n", tupleType, tmpName, valCode))

				// Assign all globals
				for j, n := range s.Names {
					c.entryBody.WriteString(fmt.Sprintf("\t%s%s = %s.v%d;\n", prefix, n.Value, tmpName, j))
					// Register root if pointer
					t := c.checker.Types[n]
					if c.isPointerCheckerType(t) {
						c.entryBody.WriteString(fmt.Sprintf("\tmph_gc_push_root(ctx, (void**)&%s%s);\n", prefix, n.Value))
					}
				}
			}
			return nil
		}

		// Standard Assignment
		c.entryBody.WriteString(fmt.Sprintf("\t%s%s = %s;\n", prefix, name, valCode))

		// Register Root if Pointer
		if c.isPointerCheckerType(destType) {
			c.entryBody.WriteString(fmt.Sprintf("\tmph_gc_push_root(ctx, (void**)&%s%s);\n", prefix, name))
		}
	}
	return nil
}

func (c *Compiler) compileFunction(fn *parser.FunctionLiteral, prefix string) error {
	if fn.Name == "main" && prefix == "mph_" {
		c.hasMain = true
	}

	// Determine Return Type (Tuple aware)
	retType := "void"
	if len(fn.ReturnTypes) == 1 {
		var err error
		retType, err = c.mapTypeToC(fn.ReturnTypes[0], prefix)
		if err != nil {
			return err
		}
	} else if len(fn.ReturnTypes) > 1 {
		// Resolve types from checker to handle Tuples correctly
		// We can't rely just on AST TypeNode mapping if we want to reuse getTupleCType logic which expects checker.Type
		// But we can resolve TypeNode to checker.Type using c.checker.Types map?
		// c.checker.Types maps Node -> Type.
		var types []checker.Type
		for _, rt := range fn.ReturnTypes {
			t := c.resolveTypeNode(rt)
			types = append(types, t)
		}
		retType = c.getTupleCType(types, prefix)
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
		var pointerOffsets []string
		for _, name := range captures {
			typ := c.findTypeForName(fn.Body, name)
			cType := c.mapCheckerTypeToC(typ, prefix)
			c.typeDefs.WriteString(fmt.Sprintf("\t%s %s;\n", cType, name))

			if c.isPointerCheckerType(typ) {
				pointerOffsets = append(pointerOffsets, fmt.Sprintf("offsetof(%s, %s)", envTypeName, name))
			}
		}
		c.typeDefs.WriteString(fmt.Sprintf("} %s;\n\n", envTypeName))

		// Generate RTTI for Env
		offsetsStr := "NULL"
		if len(pointerOffsets) > 0 {
			offsetsStr = fmt.Sprintf("(size_t[]){%s}", strings.Join(pointerOffsets, ", "))
		}
		c.typeDefs.WriteString(fmt.Sprintf("MorphTypeInfo mph_ti_%s = { \"%s\", sizeof(%s), %d, %s, NULL };\n", envTypeName, envTypeName, envTypeName, len(pointerOffsets), offsetsStr))
	}

	// 2. Generate Function Definition into Local Builder
	var fnBuilder strings.Builder
	fnBuilder.WriteString(fmt.Sprintf("%s %s%s(MorphContext* ctx, void* _env_void", retType, prefix, funcName))

	if fn.Receiver != nil {
		cType, err := c.mapTypeToC(fn.Receiver.Type, prefix)
		if err != nil {
			return err
		}
		fnBuilder.WriteString(fmt.Sprintf(", %s %s", cType, fn.Receiver.Name.Value))
	}

	for _, p := range fn.Parameters {
		cType, err := c.mapTypeToC(p.Type, prefix)
		if err != nil {
			return err
		}
		fnBuilder.WriteString(fmt.Sprintf(", %s %s", cType, p.Name.Value))
	}

	fnBuilder.WriteString(")")

	if fn.IsNative {
		fnBuilder.WriteString(";\n\n")
		c.funcDefs.WriteString(fnBuilder.String())
		return nil
	}

	fnBuilder.WriteString(" {\n")

	// 3. Register Pointer Parameters as Roots
	// Receiver
	if fn.Receiver != nil {
		if c.isPointerType(fn.Receiver.Type) {
			fnBuilder.WriteString(fmt.Sprintf("\tmph_gc_push_root(ctx, (void**)&%s);\n", fn.Receiver.Name.Value))
		}
	}
	// Params
	var roots int
	if fn.Receiver != nil && c.isPointerType(fn.Receiver.Type) {
		roots++
	}

	for _, p := range fn.Parameters {
		if c.isPointerType(p.Type) {
			fnBuilder.WriteString(fmt.Sprintf("\tmph_gc_push_root(ctx, (void**)&%s);\n", p.Name.Value))
			roots++
		}
	}

	// 3. Cast Env
	if len(captures) > 0 {
		fnBuilder.WriteString(fmt.Sprintf("\t%s* _env = (%s*)_env_void;\n", envTypeName, envTypeName))
	}

	var body strings.Builder
	if err := c.compileBlock(fn.Body, &body, prefix, fn); err != nil {
		return err
	}
	fnBuilder.WriteString(body.String())

	// Pop Param Roots
	if roots > 0 {
		fnBuilder.WriteString(fmt.Sprintf("\tmph_gc_pop_roots(ctx, %d);\n", roots))
	}

	fnBuilder.WriteString("}\n\n")

	// Append to global funcDefs AFTER body compilation is done (handling nested functions)
	c.funcDefs.WriteString(fnBuilder.String())
	return nil
}

func (c *Compiler) findTypeForName(body parser.Statement, name string) checker.Type {
	var found checker.Type
	var finder func(n parser.Node)
	finder = func(n parser.Node) {
		if found != nil || n == nil {
			return
		}
		if ident, ok := n.(*parser.Identifier); ok {
			if ident.Value == name {
				found = c.checker.Types[ident]
			}
		}
		switch t := n.(type) {
		case *parser.BlockStatement:
			for _, s := range t.Statements {
				finder(s)
			}
		case *parser.ExpressionStatement:
			finder(t.Expression)
		case *parser.VarStatement:
			for _, v := range t.Values {
				finder(v)
			}
		case *parser.ReturnStatement:
			for _, v := range t.ReturnValues {
				finder(v)
			}
		case *parser.AssignmentStatement:
			for _, v := range t.Values {
				finder(v)
			}
		case *parser.IfExpression:
			finder(t.Condition)
			finder(t.Consequence)
			finder(t.Alternative)
		case *parser.WhileExpression:
			finder(t.Condition)
			finder(t.Body)
		case *parser.CallExpression:
			finder(t.Function)
			for _, a := range t.Arguments {
				finder(a)
			}
		case *parser.InfixExpression:
			finder(t.Left)
			finder(t.Right)
		case *parser.PrefixExpression:
			finder(t.Right)
		case *parser.IndexExpression:
			finder(t.Left)
			finder(t.Index)
		}
	}
	finder(body)
	if found == nil {
		return checker.IntType
	}
	return found
}

func (c *Compiler) compileBlock(block *parser.BlockStatement, buf *strings.Builder, prefix string, currentFn *parser.FunctionLiteral) error {
	localRoots := 0
	for _, stmt := range block.Statements {
		if err := c.compileStatement(stmt, buf, prefix, currentFn); err != nil {
			return err
		}
		// Count roots created by this statement (if VarStatement with pointer)
		if vs, ok := stmt.(*parser.VarStatement); ok {
			var typeNode parser.TypeNode = vs.Type
			// If implicit, check checker type
			if typeNode == nil {
				// Too hard to reconstruct parser.TypeNode from checker.Type here correctly for isPointerType check
				// So we fallback to checker type check
				typ := c.checker.Types[vs.Values[0]] // Type of value
				if typ != nil && c.isPointerCheckerType(typ) {
					localRoots++
				}
			} else {
				if c.isPointerType(typeNode) {
					localRoots++
				}
			}
		}
	}
	// Pop local roots at end of block
	if localRoots > 0 {
		buf.WriteString(fmt.Sprintf("\tmph_gc_pop_roots(ctx, %d);\n", localRoots))
	}
	return nil
}

func (c *Compiler) isPointerCheckerType(t checker.Type) bool {
	if t == nil {
		return false
	}
	switch t.Kind() {
	case checker.KindString, checker.KindArray, checker.KindMap, checker.KindStruct, checker.KindFunction, checker.KindInterface, checker.KindUserError:
		return true
	}
	return false
}

func (c *Compiler) resolveTypeNode(t parser.TypeNode) checker.Type {
	if t == nil {
		return nil
	}
	// First check if checker cached it (best effort)
	if ct := c.checker.Types[t]; ct != nil {
		return ct
	}

	switch ty := t.(type) {
	case *parser.SimpleType:
		switch ty.Name {
		case "int":
			return checker.IntType
		case "float":
			return checker.FloatType
		case "bool":
			return checker.BoolType
		case "string":
			return checker.StringType
		case "void":
			return checker.VoidType
		case "error":
			return checker.UserErrorType
		// Structs? We need to look up struct definition?
		// Compiler has StructIDs but not Types map access?
		// Checker has Structs map.
		// We can look up in c.checker.Structs? No, scope logic is in checker.
		// But for getTupleCType, we mostly need the Kind and Name for the struct key.
		// We can create a dummy StructType with just name for C generation?
		// Or better: mapCheckerTypeToC handles StructType.
		default:
			return &checker.StructType{Name: ty.Name}
		}
	case *parser.QualifiedType:
		return &checker.StructType{Name: ty.Name.Value, Module: ty.Package.Value}
	case *parser.ArrayType:
		return &checker.ArrayType{Element: c.resolveTypeNode(ty.Element)}
	case *parser.MapType:
		return &checker.MapType{Key: c.resolveTypeNode(ty.Key), Value: c.resolveTypeNode(ty.Value)}
	case *parser.FunctionType:
		// Simplified function type (we don't need full signature for C tuple name usually, just "Closure")
		return &checker.FunctionType{}
	}
	return checker.UnknownType
}

func (c *Compiler) compileStatement(stmt parser.Statement, buf *strings.Builder, prefix string, currentFn *parser.FunctionLiteral) error {
	switch s := stmt.(type) {
	case *parser.VarStatement:
		return c.compileVar(s, buf, prefix, currentFn)
	case *parser.ReturnStatement:
		return c.compileReturn(s, buf, prefix, currentFn)
	case *parser.AssignmentStatement:
		return c.compileAssignment(s, buf, prefix, currentFn)
	case *parser.BlockStatement:
		buf.WriteString("{\n")
		if err := c.compileBlock(s, buf, prefix, currentFn); err != nil {
			return err
		}
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
		if err != nil {
			return err
		}
		buf.WriteString(fmt.Sprintf("\t%s;\n", exprCode))
		return nil
	case *parser.SwitchStatement:
		return c.compileSwitchStatement(s, buf, prefix, currentFn)
	default:
		return nil
	}
}

func (c *Compiler) compileSwitchStatement(s *parser.SwitchStatement, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	condType := c.checker.Types[s.Condition]

	// Evaluate Condition once
	condCode, err := c.compileExpression(s.Condition, prefix, fn)
	if err != nil {
		return err
	}

	condVar := fmt.Sprintf("_sw_%p", s)
	condCType := c.mapCheckerTypeToC(condType, prefix)
	buf.WriteString(fmt.Sprintf("\t%s %s = %s;\n", condCType, condVar, condCode))

	if c.isPrimitive(condType) {
		// Use C switch
		buf.WriteString(fmt.Sprintf("\tswitch (%s) {\n", condVar))
		for _, cc := range s.Cases {
			for _, val := range cc.Values {
				valCode, _ := c.compileExpression(val, prefix, fn)
				buf.WriteString(fmt.Sprintf("\tcase %s:\n", valCode))
			}
			// Case body with implicit break
			buf.WriteString("\t{\n")
			c.compileBlock(cc.Body, buf, prefix, fn)
			buf.WriteString("\t\tbreak;\n")
			buf.WriteString("\t}\n")
		}
		if s.Default != nil {
			buf.WriteString("\tdefault: {\n")
			c.compileBlock(s.Default, buf, prefix, fn)
			buf.WriteString("\t}\n")
		}
		buf.WriteString("\t}\n")
	} else if condType.Kind() == checker.KindString {
		// Use if-else chain
		firstCase := true
		for _, cc := range s.Cases {
			if !firstCase {
				buf.WriteString(" else ")
			}
			buf.WriteString("if (")
			for i, val := range cc.Values {
				valCode, _ := c.compileExpression(val, prefix, fn)
				if i > 0 {
					buf.WriteString(" || ")
				}
				buf.WriteString(fmt.Sprintf("mph_string_eq(ctx, %s, %s)", condVar, valCode))
			}
			buf.WriteString(") {\n")
			c.compileBlock(cc.Body, buf, prefix, fn)
			buf.WriteString("\t}")
			firstCase = false
		}
		if s.Default != nil {
			if !firstCase {
				buf.WriteString(" else {\n")
			} else {
				// No cases, just default? (Possible but weird)
				buf.WriteString("{\n")
			}
			c.compileBlock(s.Default, buf, prefix, fn)
			buf.WriteString("\t}\n")
		} else {
			if !firstCase {
				buf.WriteString("\n")
			}
		}
	} else {
		return fmt.Errorf("switch on type %s not supported", condType.String())
	}
	return nil
}

func (c *Compiler) compileVar(s *parser.VarStatement, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	// Tuple Unpacking Declaration: var x, y = f()
	if len(s.Names) > 1 && len(s.Values) == 1 {
		valCode, err := c.compileExpression(s.Values[0], prefix, fn)
		if err != nil {
			return err
		}

		srcType := c.checker.Types[s.Values[0]]
		if srcType == nil || srcType.Kind() != checker.KindMulti {
			return fmt.Errorf("expected tuple return for multi-var")
		}
		mt := srcType.(*checker.MultiType)
		tupleType := c.getTupleCType(mt.Types, prefix)

		// Emit block to scope temp variable?
		// No, var declarations need to be visible in current scope.
		// We can't use a block if we want variables to persist.
		// But we need a temp variable for the tuple.
		// Generate unique name? "_t_PTR"
		tmpName := fmt.Sprintf("_t_%p", s)
		buf.WriteString(fmt.Sprintf("\t%s %s = %s;\n", tupleType, tmpName, valCode))

		for i, nameIdent := range s.Names {
			name := nameIdent.Value

			// Determine type
			var destType checker.Type
			// If explicit type on VarStatement? currently VarStatement has single Type field?
			// "var x, y int = ..." -> Both int.
			// "var x, y = ..." -> Inferred.
			if s.Type != nil {
				destType = c.checker.Types[s.Type]
			} else {
				destType = c.checker.Types[nameIdent]
			}

			cType := c.mapCheckerTypeToC(destType, prefix)
			buf.WriteString(fmt.Sprintf("\t%s %s = %s.v%d;\n", cType, name, tmpName, i))

			// Register Root if Pointer
			isPtr := false
			if s.Type != nil {
				if c.isPointerType(s.Type) {
					isPtr = true
				}
			} else if destType != nil {
				if c.isPointerCheckerType(destType) {
					isPtr = true
				}
			}

			if isPtr {
				buf.WriteString(fmt.Sprintf("\tmph_gc_push_root(ctx, (void**)&%s);\n", name))
			}
		}
		return nil
	}

	if len(s.Names) != 1 || len(s.Values) != 1 {
		return fmt.Errorf("multi-var not supported")
	}
	name := s.Names[0].Value
	valCode, err := c.compileExpression(s.Values[0], prefix, fn)
	if err != nil {
		return err
	}

	var destType checker.Type
	if s.Type != nil {
		destType = c.checker.Types[s.Type]
	} else {
		destType = c.checker.Types[s.Names[0]]
		if destType == nil {
			destType = c.checker.Types[s.Values[0]]
		}
	}

	// Interface Conversion check
	srcType := c.checker.Types[s.Values[0]]
	if destType != nil && destType.Kind() == checker.KindInterface && srcType != nil && srcType.Kind() == checker.KindStruct {
		valCode, err = c.compileInterfaceConversion(destType.(*checker.InterfaceType), srcType.(*checker.StructType), valCode, prefix)
		if err != nil {
			return err
		}
	}

	cType := "mph_int"
	if destType != nil {
		cType = c.mapCheckerTypeToC(destType, prefix)
	}

	buf.WriteString(fmt.Sprintf("\t%s %s = %s;\n", cType, name, valCode))

	// Register Root if Pointer
	isPtr := false
	if s.Type != nil {
		if c.isPointerType(s.Type) {
			isPtr = true
		}
	} else if destType != nil {
		if c.isPointerCheckerType(destType) {
			isPtr = true
		}
	}

	if isPtr {
		buf.WriteString(fmt.Sprintf("\tmph_gc_push_root(ctx, (void**)&%s);\n", name))
	}

	return nil
}

func (c *Compiler) compileAssignment(s *parser.AssignmentStatement, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	// Tuple Unpacking: x, y = f()
	if len(s.Names) > 1 && len(s.Values) == 1 {
		valCode, err := c.compileExpression(s.Values[0], prefix, fn)
		if err != nil {
			return err
		}

		srcType := c.checker.Types[s.Values[0]]
		if srcType == nil || srcType.Kind() != checker.KindMulti {
			return fmt.Errorf("expected tuple return")
		}
		mt := srcType.(*checker.MultiType)
		tupleType := c.getTupleCType(mt.Types, prefix)

		// Use temp variable for tuple result
		// We use GCC statement expression if possible? No, assignment is statement.
		// We can just emit block.
		buf.WriteString("{\n")
		buf.WriteString(fmt.Sprintf("\t%s _t = %s;\n", tupleType, valCode))

		for i, nameExpr := range s.Names {
			if ident, ok := nameExpr.(*parser.Identifier); ok {
				target := ident.Value
				if c.isCaptured(target, fn) {
					target = fmt.Sprintf("_env->%s", target)
				}
				buf.WriteString(fmt.Sprintf("\t%s = _t.v%d;\n", target, i))
			} else {
				return fmt.Errorf("only identifiers supported in tuple unpack")
			}
		}
		buf.WriteString("}\n")
		return nil
	}

	valCode, err := c.compileExpression(s.Values[0], prefix, fn)
	if err != nil {
		return err
	}

	// Interface Conversion
	srcType := c.checker.Types[s.Values[0]]
	destType := c.checker.Types[s.Names[0]]
	if destType != nil && destType.Kind() == checker.KindInterface && srcType != nil && srcType.Kind() == checker.KindStruct {
		valCode, err = c.compileInterfaceConversion(destType.(*checker.InterfaceType), srcType.(*checker.StructType), valCode, prefix)
		if err != nil {
			return err
		}
	}

	if ident, ok := s.Names[0].(*parser.Identifier); ok {
		target := ident.Value
		if c.isCaptured(target, fn) {
			target = fmt.Sprintf("_env->%s", target)
		} else if c.currentGlobals[target] && !isBuiltin(target) {
			target = prefix + target
		}
		buf.WriteString(fmt.Sprintf("\t%s = %s;\n", target, valCode))
		return nil
	} else if mem, ok := s.Names[0].(*parser.MemberExpression); ok {
		objCode, err := c.compileExpression(mem.Object, prefix, fn)
		if err != nil {
			return err
		}
		objType := c.checker.Types[mem.Object]
		objRef := objCode
		valRef := valCode
		var roots []rootTemp
		if objType != nil && c.isPointerCheckerType(objType) {
			objTemp := c.nextTemp("obj")
			roots = append(roots, rootTemp{
				cType: c.mapCheckerTypeToC(objType, prefix),
				name:  objTemp,
				value: objCode,
			})
			objRef = objTemp
		}
		if srcType != nil && c.isPointerCheckerType(srcType) {
			valTemp := c.nextTemp("val")
			roots = append(roots, rootTemp{
				cType: c.mapCheckerTypeToC(srcType, prefix),
				name:  valTemp,
				value: valCode,
			})
			valRef = valTemp
		}
		expr := fmt.Sprintf("%s->%s = %s", objRef, mem.Member.Value, valRef)
		buf.WriteString(fmt.Sprintf("\t%s;\n", c.wrapWithRoots(roots, expr, "void")))
		return nil
	} else if idx, ok := s.Names[0].(*parser.IndexExpression); ok {
		// Map/Array Assignment
		objCode, err := c.compileExpression(idx.Left, prefix, fn)
		if err != nil {
			return err
		}
		idxCode, err := c.compileExpression(idx.Index, prefix, fn)
		if err != nil {
			return err
		}

		leftType := c.checker.Types[idx.Left]
		if leftType != nil && leftType.Kind() == checker.KindMap {
			mt := leftType.(*checker.MapType)
			objRef := objCode
			keyRef := idxCode
			valRef := valCode
			var roots []rootTemp
			if c.isPointerCheckerType(leftType) {
				objTemp := c.nextTemp("map")
				roots = append(roots, rootTemp{
					cType: c.mapCheckerTypeToC(leftType, prefix),
					name:  objTemp,
					value: objCode,
				})
				objRef = objTemp
			}
			if c.isPointerCheckerType(mt.Key) {
				keyTemp := c.nextTemp("key")
				roots = append(roots, rootTemp{
					cType: c.mapCheckerTypeToC(mt.Key, prefix),
					name:  keyTemp,
					value: idxCode,
				})
				keyRef = keyTemp
			}
			if c.isPointerCheckerType(mt.Value) {
				valTemp := c.nextTemp("val")
				roots = append(roots, rootTemp{
					cType: c.mapCheckerTypeToC(mt.Value, prefix),
					name:  valTemp,
					value: valCode,
				})
				valRef = valTemp
			}
			keyCast := fmt.Sprintf("(void*)%s", keyRef)
			if mt.Key.Kind() == checker.KindInt {
				keyCast = fmt.Sprintf("(void*)(int64_t)%s", keyRef)
			}
			valCast := fmt.Sprintf("(void*)%s", valRef)
			if c.isPrimitive(mt.Value) {
				valCast = fmt.Sprintf("(void*)(int64_t)%s", valRef)
			}
			expr := fmt.Sprintf("mph_map_set(ctx, %s, %s, %s)", objRef, keyCast, valCast)
			buf.WriteString(fmt.Sprintf("\t%s;\n", c.wrapWithRoots(roots, expr, "void")))
			return nil
		} else if leftType != nil && leftType.Kind() == checker.KindArray {
			at := leftType.(*checker.ArrayType)
			elemCType := c.mapCheckerTypeToC(at.Element, prefix)
			buf.WriteString(fmt.Sprintf("\t((%s*)((MorphArray*)%s)->data)[%s] = %s;\n", elemCType, objCode, idxCode, valCode))
			return nil
		}
	}
	return fmt.Errorf("complex assignment not supported")
}

func (c *Compiler) isCaptured(name string, fn *parser.FunctionLiteral) bool {
	if fn == nil {
		return false
	}
	list := c.captures[fn]
	for _, v := range list {
		if v == name {
			return true
		}
	}
	return false
}

func (c *Compiler) compileReturn(s *parser.ReturnStatement, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	if len(s.ReturnValues) == 0 {
		buf.WriteString("\treturn;\n")
		return nil
	}
	if len(s.ReturnValues) == 1 {
		valCode, err := c.compileExpression(s.ReturnValues[0], prefix, fn)
		if err != nil {
			return err
		}
		buf.WriteString(fmt.Sprintf("\treturn %s;\n", valCode))
		return nil
	}

	// Multiple return values -> Tuple Struct
	// We need to know the Tuple Type Name.
	// Use Target Return Types from Function Signature to ensure correct C Struct (e.g. MorphTuple_Int_Error vs MorphTuple_Int_Null)

	var valCodes []string
	var roots []rootTemp
	for _, expr := range s.ReturnValues {
		code, err := c.compileExpression(expr, prefix, fn)
		if err != nil {
			return err
		}
		valRef := code
		exprType := c.checker.Types[expr]
		if exprType != nil && c.isPointerCheckerType(exprType) {
			temp := c.nextTemp("ret")
			roots = append(roots, rootTemp{
				cType: c.mapCheckerTypeToC(exprType, prefix),
				name:  temp,
				value: code,
			})
			valRef = temp
		}
		valCodes = append(valCodes, valRef)
	}

	// Determine target types from function signature
	var targetTypes []checker.Type
	fnType := c.checker.Types[fn]
	if ft, ok := fnType.(*checker.FunctionType); ok {
		targetTypes = ft.ReturnTypes
	} else {
		// Fallback (should not happen if fn is valid)
		for _, expr := range s.ReturnValues {
			targetTypes = append(targetTypes, c.checker.Types[expr])
		}
	}

	tupleName := c.getTupleCType(targetTypes, prefix)
	expr := fmt.Sprintf("(%s){ %s }", tupleName, strings.Join(valCodes, ", "))
	if len(roots) > 0 {
		expr = c.wrapWithRoots(roots, expr, tupleName)
	}
	buf.WriteString(fmt.Sprintf("\treturn %s;\n", expr))
	return nil
}

func (c *Compiler) compileIf(ie *parser.IfExpression, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	cond, err := c.compileExpression(ie.Condition, prefix, fn)
	if err != nil {
		return err
	}
	buf.WriteString(fmt.Sprintf("\tif (%s) {\n", cond))
	c.compileStatement(ie.Consequence, buf, prefix, fn)
	buf.WriteString("\t}")

	for _, clause := range ie.ElseIfs {
		condCode, err := c.compileExpression(clause.Condition, prefix, fn)
		if err != nil {
			return err
		}
		buf.WriteString(fmt.Sprintf(" else if (%s) {\n", condCode))
		c.compileStatement(clause.Consequence, buf, prefix, fn)
		buf.WriteString("\t}")
	}

	if ie.Alternative != nil {
		buf.WriteString(" else {\n")
		c.compileStatement(ie.Alternative, buf, prefix, fn)
		buf.WriteString("\t}\n")
	} else {
		buf.WriteString("\n")
	}
	return nil
}

func (c *Compiler) compileWhile(we *parser.WhileExpression, buf *strings.Builder, prefix string, fn *parser.FunctionLiteral) error {
	cond, err := c.compileExpression(we.Condition, prefix, fn)
	if err != nil {
		return err
	}
	buf.WriteString(fmt.Sprintf("\twhile (%s) {\n", cond))
	c.compileStatement(we.Body, buf, prefix, fn)
	buf.WriteString("\t}\n")
	return nil
}

func (c *Compiler) compileExpression(expr parser.Expression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	// Guard against stack overflow from deeply nested expressions
	c.recursionDepth++
	if c.recursionDepth > 1000 {
		return "", fmt.Errorf("compilation recursion limit exceeded (max 1000 levels)")
	}
	defer func() { c.recursionDepth-- }()

	switch e := expr.(type) {
	case *parser.Identifier:
		name := e.Value
		if c.isCaptured(name, fn) {
			return fmt.Sprintf("_env->%s", name), nil
		}
		if c.isLocal(name, fn) {
			return name, nil
		}
		// Imported identifier? Use origin module prefix
		if importPrefix, ok := c.importOrigins[name]; ok {
			return importPrefix + name, nil
		}
		// Global in THIS module?
		if c.currentGlobals[name] && !isBuiltin(name) {
			return prefix + name, nil
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
		// Escape special characters for C string literals
		// IMPORTANT: Backslash must be escaped first!
		escaped := strings.ReplaceAll(e.Value, "\\", "\\\\")
		escaped = strings.ReplaceAll(escaped, "\"", "\\\"")
		escaped = strings.ReplaceAll(escaped, "\n", "\\n")
		escaped = strings.ReplaceAll(escaped, "\r", "\\r")
		escaped = strings.ReplaceAll(escaped, "\t", "\\t")
		return fmt.Sprintf("mph_string_new(ctx, \"%s\")", escaped), nil
	case *parser.IntegerLiteral:
		return fmt.Sprintf("%d", e.Value), nil
	case *parser.FloatLiteral:
		return strconv.FormatFloat(e.Value, 'g', -1, 64), nil
	case *parser.CharLiteral:
		return fmt.Sprintf("%d", e.Value), nil
	case *parser.BooleanLiteral:
		if e.Value {
			return "1", nil
		}
		return "0", nil
	case *parser.NullLiteral:
		return "NULL", nil
	case *parser.InfixExpression:
		return c.compileInfix(e, prefix, fn)
	case *parser.PrefixExpression:
		right, err := c.compileExpression(e.Right, prefix, fn)
		if err != nil {
			return "", err
		}
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
		// Check if Object is Module
		objType := c.checker.Types[e.Object]
		if objType != nil && objType.Kind() == checker.KindModule {
			modType := objType.(*checker.ModuleType)
			return mangle(modType.Name) + e.Member.Value, nil
		}
		obj, err := c.compileExpression(e.Object, prefix, fn)
		if err != nil {
			return "", err
		}
		expr := fmt.Sprintf("%s->%s", obj, e.Member.Value)
		if objType != nil && objType.Kind() != checker.KindInterface && c.isPointerCheckerType(objType) {
			temp := c.nextTemp("obj")
			roots := []rootTemp{
				{
					cType: c.mapCheckerTypeToC(objType, prefix),
					name:  temp,
					value: obj,
				},
			}
			retType := c.mapCheckerTypeToC(c.checker.Types[e], prefix)
			expr = fmt.Sprintf("%s->%s", temp, e.Member.Value)
			return c.wrapWithRoots(roots, expr, retType), nil
		}
		return expr, nil
	case *parser.InterpolatedString:
		return c.compileInterpolatedString(e, prefix, fn)
	default:
		return "", fmt.Errorf("unsupported expr: %T", expr)
	}
}

func isNilNode(n parser.Node) bool {
	if n == nil {
		return true
	}
	value := reflect.ValueOf(n)
	return value.Kind() == reflect.Ptr && value.IsNil()
}

func (c *Compiler) compileFunctionLiteral(fn *parser.FunctionLiteral, prefix string, parentFn *parser.FunctionLiteral) (string, error) {
	if err := c.compileFunction(fn, prefix); err != nil {
		return "", err
	}
	funcName := c.getAnonFuncName(fn)
	envTypeName := fmt.Sprintf("Env_%s_%s", prefix, funcName)
	captures := c.captures[fn]

	var sb strings.Builder
	sb.WriteString("({ ")
	if len(captures) > 0 {
		sb.WriteString(fmt.Sprintf("%s* _e = (%s*)mph_alloc(ctx, sizeof(%s), &mph_ti_%s); ", envTypeName, envTypeName, envTypeName, envTypeName))
		for _, name := range captures {
			val := name
			if c.isCaptured(name, parentFn) {
				val = fmt.Sprintf("_env->%s", name)
			}
			sb.WriteString(fmt.Sprintf("_e->%s = %s; ", name, val))
		}
		sb.WriteString(fmt.Sprintf("mph_closure_new(ctx, (void*)%s%s, (void*)_e, sizeof(%s)); ", prefix, funcName, envTypeName))
	} else {
		sb.WriteString(fmt.Sprintf("mph_closure_new(ctx, (void*)%s%s, NULL, 0); ", prefix, funcName))
	}
	sb.WriteString(" })")
	return sb.String(), nil
}

func (c *Compiler) compileCall(call *parser.CallExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	var funcCode string
	isDirect := false
	funcTemp := ""
	funcTempType := ""
	funcNeedsRoot := false

	if ident, ok := call.Function.(*parser.Identifier); ok {
		if st, ok := c.checker.Types[call.Function].(*checker.StructType); ok {
			return c.compileStructConstructor(call, st, prefix, fn)
		}
		switch ident.Value {
		case "native_print":
			return c.compileBuiltin(call, "mph_native_print", prefix, fn)
		case "native_print_error":
			return c.compileBuiltin(call, "mph_native_print_error", prefix, fn)
		case "native_print_int":
			return c.compileBuiltin(call, "mph_native_print_int", prefix, fn)
		case "saluran_baru":
			return c.compileBuiltin(call, "mph_channel_new", prefix, fn)
		case "kirim":
			return c.compileBuiltin(call, "mph_channel_send", prefix, fn)
		case "terima":
			return c.compileBuiltin(call, "mph_channel_recv", prefix, fn)
		case "luncurkan":
			return c.compileSpawn(call, prefix, fn)
		case "hapus":
			return c.compileDelete(call, prefix, fn)
		case "panjang":
			return c.compileLen(call, prefix, fn)
		case "error":
			return c.compileBuiltin(call, "mph_error_new", prefix, fn)
		case "index":
			return c.compileBuiltin(call, "mph_string_index", prefix, fn)
		case "trim":
			return c.compileBuiltin(call, "mph_string_trim", prefix, fn)
		case "split":
			return c.compileBuiltin(call, "mph_string_split", prefix, fn)
		case "substring":
			return c.compileBuiltin(call, "mph_string_substring", prefix, fn)
		}

		funcCode = ident.Value
		if c.isCaptured(funcCode, fn) {
			funcCode = fmt.Sprintf("_env->%s", funcCode)
		} else if !c.isLocal(funcCode, fn) && !isBuiltin(funcCode) {
			// Check if imported from another module
			if importPrefix, ok := c.importOrigins[funcCode]; ok {
				funcCode = importPrefix + funcCode
				isDirect = true
			} else if c.currentGlobals[funcCode] {
				funcCode = prefix + funcCode
				isDirect = true
			}
		}
	} else if mem, ok := call.Function.(*parser.MemberExpression); ok {
		objType := c.checker.Types[mem.Object]
		if objType != nil && objType.Kind() == checker.KindModule {
			modType := objType.(*checker.ModuleType)
			funcCode = mangle(modType.Name) + mem.Member.Value
			isDirect = true
		} else if objType != nil && objType.Kind() == checker.KindInterface {
			return c.compileInterfaceCall(call, mem, objType.(*checker.InterfaceType), prefix, fn)
		} else if objType != nil && (objType.Kind() == checker.KindStruct || (objType.Kind() == checker.KindPointer && objType.(*checker.PointerType).Element.Kind() == checker.KindStruct)) {
			var st *checker.StructType
			isPtr := false
			if objType.Kind() == checker.KindPointer {
				st = objType.(*checker.PointerType).Element.(*checker.StructType)
				isPtr = true
			} else {
				st = objType.(*checker.StructType)
			}

			funcCode := ""
			if st.Module != "" {
				funcCode = mangle(st.Module) + st.Name + "_" + mem.Member.Value
			} else {
				funcCode = "mph_" + st.Name + "_" + mem.Member.Value
			}
			isDirect = true

			objCode, err := c.compileExpression(mem.Object, prefix, fn)
			if err != nil {
				return "", err
			}

			var args []string
			args = append(args, "ctx")
			args = append(args, "NULL")

			objRef := objCode
			var roots []rootTemp
			if c.isPointerCheckerType(objType) {
				objTemp := c.nextTemp("recv")
				roots = append(roots, rootTemp{
					cType: c.mapCheckerTypeToC(objType, prefix),
					name:  objTemp,
					value: objCode,
				})
				objRef = objTemp
			}

			if isPtr {
				objRef = fmt.Sprintf("(*%s)", objRef)
			}

			args = append(args, objRef)

			for _, arg := range call.Arguments {
				ac, err := c.compileExpression(arg, prefix, fn)
				if err != nil {
					return "", err
				}
				argRef := ac
				argType := c.checker.Types[arg]
				if argType != nil && c.isPointerCheckerType(argType) {
					argTemp := c.nextTemp("arg")
					roots = append(roots, rootTemp{
						cType: c.mapCheckerTypeToC(argType, prefix),
						name:  argTemp,
						value: ac,
					})
					argRef = argTemp
				}
				args = append(args, argRef)
			}
			callExpr := fmt.Sprintf("%s(%s)", funcCode, strings.Join(args, ", "))
			if len(roots) == 0 {
				return callExpr, nil
			}
			callType := c.checker.Types[call]
			retType := "void"
			if callType != nil {
				retType = c.mapCheckerTypeToC(callType, prefix)
			}
			return c.wrapWithRoots(roots, callExpr, retType), nil
		} else {
			return "", fmt.Errorf("method calls not supported yet")
		}
	} else {
		var err error
		funcCode, err = c.compileExpression(call.Function, prefix, fn)
		if err != nil {
			return "", err
		}
	}

	if !isDirect {
		if _, ok := call.Function.(*parser.Identifier); !ok {
			if _, ok := call.Function.(*parser.MemberExpression); !ok {
				if funcType := c.checker.Types[call.Function]; funcType != nil && c.isPointerCheckerType(funcType) {
					funcTemp = c.nextTemp("fn")
					funcTempType = c.mapCheckerTypeToC(funcType, prefix)
					funcNeedsRoot = true
				}
			}
		}
	}

	type argInfo struct {
		code      string
		cType     string
		temp      string
		needsRoot bool
	}

	var argInfos []argInfo
	rootCount := 0
	for _, arg := range call.Arguments {
		ac, err := c.compileExpression(arg, prefix, fn)
		if err != nil {
			return "", err
		}
		argType := c.checker.Types[arg]
		info := argInfo{code: ac}
		if argType != nil && c.isPointerCheckerType(argType) {
			info.needsRoot = true
			info.cType = c.mapCheckerTypeToC(argType, prefix)
			info.temp = c.nextTemp("arg")
			rootCount++
		}
		argInfos = append(argInfos, info)
	}

	if funcNeedsRoot {
		rootCount++
	}

	var args []string
	args = append(args, "ctx")

	funcRef := funcCode
	if funcNeedsRoot {
		funcRef = funcTemp
	}

	if isDirect {
		args = append(args, "NULL") // Env
	} else {
		// Closure Call: ((MorphClosureFunc)c->function)(ctx, c->env, ...)
		args = append(args, fmt.Sprintf("%s->env", funcRef))
	}

	for _, info := range argInfos {
		if info.needsRoot {
			args = append(args, info.temp)
		} else {
			args = append(args, info.code)
		}
	}

	callExpr := ""
	if isDirect {
		callExpr = fmt.Sprintf("%s(%s)", funcRef, strings.Join(args, ", "))
	} else {
		// Generic cast
		cast := "MorphClosureFunc"

		// Attempt precise cast if Checker knows the type
		funcType := c.checker.Types[call.Function]
		if funcType != nil && funcType.Kind() == checker.KindFunction {
			ft := funcType.(*checker.FunctionType)

			retType := "void"
			if len(ft.ReturnTypes) > 0 {
				retType = c.mapCheckerTypeToC(ft.ReturnTypes[0], prefix)
			}

			var paramTypes []string
			paramTypes = append(paramTypes, "MorphContext*")
			paramTypes = append(paramTypes, "void*")
			for _, p := range ft.Parameters {
				paramTypes = append(paramTypes, c.mapCheckerTypeToC(p, prefix))
			}
			cast = fmt.Sprintf("%s (*)(%s)", retType, strings.Join(paramTypes, ", "))
		}

		callExpr = fmt.Sprintf("((%s)%s->function)(%s)", cast, funcRef, strings.Join(args, ", "))
	}

	if rootCount == 0 {
		return callExpr, nil
	}

	var sb strings.Builder
	sb.WriteString("({ ")
	if funcNeedsRoot {
		sb.WriteString(fmt.Sprintf("%s %s = %s; ", funcTempType, funcTemp, funcCode))
		sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s); ", funcTemp))
	}
	for _, info := range argInfos {
		if !info.needsRoot {
			continue
		}
		sb.WriteString(fmt.Sprintf("%s %s = %s; ", info.cType, info.temp, info.code))
		sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s); ", info.temp))
	}

	callType := c.checker.Types[call]
	retType := "void"
	if callType != nil {
		retType = c.mapCheckerTypeToC(callType, prefix)
	}

	if retType == "void" {
		sb.WriteString(fmt.Sprintf("%s; ", callExpr))
		sb.WriteString(fmt.Sprintf("mph_gc_pop_roots(ctx, %d); ", rootCount))
		sb.WriteString("})")
		return sb.String(), nil
	}

	resTemp := c.nextTemp("ret")
	sb.WriteString(fmt.Sprintf("%s %s = %s; ", retType, resTemp, callExpr))
	sb.WriteString(fmt.Sprintf("mph_gc_pop_roots(ctx, %d); ", rootCount))
	sb.WriteString(fmt.Sprintf("%s; })", resTemp))
	return sb.String(), nil
}

func (c *Compiler) compileStructConstructor(call *parser.CallExpression, st *checker.StructType, prefix string, fn *parser.FunctionLiteral) (string, error) {
	cTypeName := c.structCTypeName(st, prefix)

	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("%s* _t = (%s*)mph_alloc(ctx, sizeof(%s), &mph_ti_%s); ", cTypeName, cTypeName, cTypeName, cTypeName))
	sb.WriteString("mph_gc_push_root(ctx, (void**)&_t); ")
	for i, fieldName := range st.FieldOrder {
		if i >= len(call.Arguments) {
			break
		}
		valCode, err := c.compileExpression(call.Arguments[i], prefix, fn)
		if err != nil {
			return "", err
		}
		sb.WriteString(fmt.Sprintf("_t->%s = %s; ", fieldName, valCode))
	}
	sb.WriteString("mph_gc_pop_roots(ctx, 1); _t; })")
	return sb.String(), nil
}

func (c *Compiler) structCTypeName(st *checker.StructType, prefix string) string {
	if st.Module != "" {
		return mangle(st.Module) + st.Name
	}
	return prefix + st.Name
}

func (c *Compiler) isLocal(name string, fn *parser.FunctionLiteral) bool {
	if fn == nil {
		return false
	}
	for _, p := range fn.Parameters {
		if p.Name.Value == name {
			return true
		}
	}
	// Also check local variables
	if fn.Body != nil {
		for _, stmt := range fn.Body.Statements {
			if vs, ok := stmt.(*parser.VarStatement); ok {
				for _, n := range vs.Names {
					if n.Value == name {
						return true
					}
				}
			}
		}
	}
	return false
}

func isBuiltin(name string) bool {
	return name == "native_print" || name == "native_print_int" || name == "len" || name == "hapus" || name == "panjang" || name == "error" || name == "native_print_error" || name == "index" || name == "trim" || name == "split" || name == "substring"
}

func (c *Compiler) compileBuiltin(call *parser.CallExpression, cName string, prefix string, fn *parser.FunctionLiteral) (string, error) {
	var args []string
	args = append(args, "ctx")
	for _, arg := range call.Arguments {
		ac, err := c.compileExpression(arg, prefix, fn)
		if err != nil {
			return "", err
		}
		args = append(args, ac)
	}
	return fmt.Sprintf("%s(%s)", cName, strings.Join(args, ", ")), nil
}

func (c *Compiler) compileInfix(ie *parser.InfixExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	left, err := c.compileExpression(ie.Left, prefix, fn)
	if err != nil {
		return "", err
	}
	right, err := c.compileExpression(ie.Right, prefix, fn)
	if err != nil {
		return "", err
	}

	if ie.Operator == "+" {
		if t := c.checker.Types[ie.Left]; t != nil {
			if t.Kind() == checker.KindString {
				leftType := c.mapCheckerTypeToC(t, prefix)
				rightType := leftType
				if rt := c.checker.Types[ie.Right]; rt != nil {
					rightType = c.mapCheckerTypeToC(rt, prefix)
				}
				leftTemp := c.nextTemp("lhs")
				rightTemp := c.nextTemp("rhs")
				var sb strings.Builder
				sb.WriteString("({ ")
				sb.WriteString(fmt.Sprintf("%s %s = %s; ", leftType, leftTemp, left))
				sb.WriteString(fmt.Sprintf("%s %s = %s; ", rightType, rightTemp, right))
				sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s); ", leftTemp))
				sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s); ", rightTemp))
				resTemp := c.nextTemp("ret")
				sb.WriteString(fmt.Sprintf("%s %s = mph_string_concat(ctx, %s, %s); ", leftType, resTemp, leftTemp, rightTemp))
				sb.WriteString("mph_gc_pop_roots(ctx, 2); ")
				sb.WriteString(fmt.Sprintf("%s; })", resTemp))
				return sb.String(), nil
			}
			if t.Kind() == checker.KindArray {
				leftType := c.mapCheckerTypeToC(t, prefix)
				rightType := leftType
				if rt := c.checker.Types[ie.Right]; rt != nil {
					rightType = c.mapCheckerTypeToC(rt, prefix)
				}
				leftTemp := c.nextTemp("lhs")
				rightTemp := c.nextTemp("rhs")
				var sb strings.Builder
				sb.WriteString("({ ")
				sb.WriteString(fmt.Sprintf("%s %s = %s; ", leftType, leftTemp, left))
				sb.WriteString(fmt.Sprintf("%s %s = %s; ", rightType, rightTemp, right))
				sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s); ", leftTemp))
				sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s); ", rightTemp))
				resTemp := c.nextTemp("ret")
				sb.WriteString(fmt.Sprintf("%s %s = mph_array_concat(ctx, %s, %s); ", leftType, resTemp, leftTemp, rightTemp))
				sb.WriteString("mph_gc_pop_roots(ctx, 2); ")
				sb.WriteString(fmt.Sprintf("%s; })", resTemp))
				return sb.String(), nil
			}
		}
	}
	if ie.Operator == "==" {
		if t := c.checker.Types[ie.Left]; t != nil && t.Kind() == checker.KindString {
			return fmt.Sprintf("mph_string_eq(ctx, %s, %s)", left, right), nil
		}
	}

	// Map Fox logical operators to C operators
	operator := ie.Operator
	switch operator {
	case "dan":
		operator = "&&"
	case "atau":
		operator = "||"
	case "tidak":
		operator = "!"
	}

	return fmt.Sprintf("(%s %s %s)", left, operator, right), nil
}

func (c *Compiler) compileInterpolatedString(is *parser.InterpolatedString, prefix string, fn *parser.FunctionLiteral) (string, error) {
	if len(is.Parts) == 0 {
		return "mph_string_new(ctx, \"\")", nil
	}
	current, err := c.compileExpression(is.Parts[0], prefix, fn)
	if err != nil {
		return "", err
	}
	for _, part := range is.Parts[1:] {
		next, err := c.compileExpression(part, prefix, fn)
		if err != nil {
			return "", err
		}
		current = fmt.Sprintf("mph_string_concat(ctx, %s, %s)", current, next)
	}
	return current, nil
}

func (c *Compiler) mapCheckerTypeToC(t checker.Type, prefix string) string {
	if t == nil {
		return "mph_int"
	}
	switch t.Kind() {
	case checker.KindFunction:
		return "MorphClosure*"
	case checker.KindInt:
		return "mph_int"
	case checker.KindFloat:
		return "mph_float"
	case checker.KindBool:
		return "mph_bool"
	case checker.KindString:
		return "MorphString*"
	case checker.KindArray:
		return "MorphArray*"
	case checker.KindMap:
		return "MorphMap*"
	case checker.KindStruct:
		if st, ok := t.(*checker.StructType); ok {
			if st.Module != "" {
				return mangle(st.Module) + st.Name + "*"
			}
			return "mph_" + st.Name + "*"
		}
	case checker.KindInterface:
		return "MorphInterface"
	case checker.KindVoid:
		return "void"
	case checker.KindMulti:
		if mt, ok := t.(*checker.MultiType); ok {
			return c.getTupleCType(mt.Types, prefix)
		}
	case checker.KindUserError:
		// error type maps to MorphError*
		return "MorphError*"
	}
	return "mph_int"
}

func (c *Compiler) getTupleCType(types []checker.Type, prefix string) string {
	// Generate signature key
	var sigBuilder strings.Builder
	for _, t := range types {
		sigBuilder.WriteString(t.String())
		sigBuilder.WriteString("_")
	}
	key := sigBuilder.String()

	if name, ok := c.tupleTypes[key]; ok {
		return name
	}

	// Generate new struct
	// Naming convention: MorphTuple_<Hash> or similar.
	// Simplified: MorphTuple_<Index>
	name := fmt.Sprintf("MorphTuple_%d", len(c.tupleTypes))

	// BUT we want stable names for Stdlib.
	// If types are Int, UserError -> MorphTuple_Int_Error
	// Let's try to make readable names if possible.
	var nameBuilder strings.Builder
	nameBuilder.WriteString("MorphTuple")
	for _, t := range types {
		nameBuilder.WriteString("_")
		// Clean type name
		clean := strings.ReplaceAll(t.String(), "[]", "Array")
		clean = strings.ReplaceAll(clean, "*", "Ptr")
		clean = strings.ReplaceAll(clean, "map[", "Map")
		clean = strings.ReplaceAll(clean, "]", "")
		clean = strings.ReplaceAll(clean, ".", "")
		clean = strings.ReplaceAll(clean, " ", "")
		// Handle error -> Error
		if t.Kind() == checker.KindUserError {
			clean = "Error"
		}
		if t.Kind() == checker.KindInt {
			clean = "Int"
		}
		if t.Kind() == checker.KindString {
			clean = "String"
		}
		nameBuilder.WriteString(clean)
	}
	name = nameBuilder.String()

	// Check if name collision? (Unlikely with full types)
	if _, exists := c.tupleTypes[name]; exists {
		// Already registered? Return it.
		// But key was signature. If name matches but sig doesn't? Collision.
		// Assume types.String() is unique enough.
		c.tupleTypes[key] = name // Map sig to existing name
		return name
	}

	c.tupleTypes[key] = name
	c.tupleTypes[name] = name // Also map name to itself for consistency?

	// Generate definition
	c.typeDefs.WriteString(fmt.Sprintf("typedef struct %s {\n", name))

	// Collect pointer offsets for RTTI
	var offsets []string

	for i, t := range types {
		cType := c.mapCheckerTypeToC(t, prefix)
		c.typeDefs.WriteString(fmt.Sprintf("\t%s v%d;\n", cType, i))

		if c.isPointerCheckerType(t) {
			offsets = append(offsets, fmt.Sprintf("offsetof(%s, v%d)", name, i))
		}
	}
	c.typeDefs.WriteString(fmt.Sprintf("} %s;\n\n", name))

	// Generate RTTI
	numPtrs := len(offsets)
	offsetsStr := "NULL"
	if numPtrs > 0 {
		offsetsStr = fmt.Sprintf("(size_t[]){%s}", strings.Join(offsets, ", "))
	}
	c.typeDefs.WriteString(fmt.Sprintf("MorphTypeInfo mph_ti_%s = { \"%s\", sizeof(%s), %d, %s };\n", name, name, name, numPtrs, offsetsStr))

	return name
}

func (c *Compiler) compileStructLiteral(sl *parser.StructLiteral, prefix string, fn *parser.FunctionLiteral) (string, error) {
	var cTypeName string
	if ident, ok := sl.Name.(*parser.Identifier); ok {
		cTypeName = prefix + ident.Value
	} else {
		return "", fmt.Errorf("complex struct literals not supported yet")
	}

	var sb strings.Builder
	sb.WriteString("({ ")
	// Pass pointer to global RTTI variable: &mph_ti_TypeName
	sb.WriteString(fmt.Sprintf("%s* _t = (%s*)mph_alloc(ctx, sizeof(%s), &mph_ti_%s); ", cTypeName, cTypeName, cTypeName, cTypeName))
	sb.WriteString("mph_gc_push_root(ctx, (void**)&_t); ")
	for fieldName, valExpr := range sl.Fields {
		valCode, err := c.compileExpression(valExpr, prefix, fn)
		if err != nil {
			return "", err
		}
		sb.WriteString(fmt.Sprintf("_t->%s = %s; ", fieldName, valCode))
	}
	sb.WriteString("mph_gc_pop_roots(ctx, 1); _t; })")
	return sb.String(), nil
}

func (c *Compiler) compileArrayLiteral(al *parser.ArrayLiteral, prefix string, fn *parser.FunctionLiteral) (string, error) {
	arrType := c.checker.Types[al]
	elemCType := "mph_int"
	isPtr := 0
	if arrType != nil {
		if at, ok := arrType.(*checker.ArrayType); ok {
			elemCType = c.mapCheckerTypeToC(at.Element, prefix)
			if c.isPointerCheckerType(at.Element) {
				isPtr = 1
			}
		}
	}
	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("MorphArray* _a = mph_array_new(ctx, %d, sizeof(%s), %d); ", len(al.Elements), elemCType, isPtr))
	sb.WriteString("mph_gc_push_root(ctx, (void**)&_a); ")
	for i, el := range al.Elements {
		valCode, err := c.compileExpression(el, prefix, fn)
		if err != nil {
			return "", err
		}
		sb.WriteString(fmt.Sprintf("((%s*)_a->data)[%d] = %s; ", elemCType, i, valCode))
	}
	sb.WriteString("mph_gc_pop_roots(ctx, 1); _a; })")
	return sb.String(), nil
}

func (c *Compiler) compileHashLiteral(hl *parser.HashLiteral, prefix string, fn *parser.FunctionLiteral) (string, error) {
	mapType := c.checker.Types[hl]
	kindEnum := "MPH_KEY_PTR"
	valIsPtr := 0
	if mapType != nil {
		if mt, ok := mapType.(*checker.MapType); ok {
			if mt.Key.Kind() == checker.KindInt {
				kindEnum = "MPH_KEY_INT"
			} else if mt.Key.Kind() == checker.KindString {
				kindEnum = "MPH_KEY_STRING"
			}
			if c.isPointerCheckerType(mt.Value) {
				valIsPtr = 1
			}
		}
	}
	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("MorphMap* _m = mph_map_new(ctx, %s, %d); ", kindEnum, valIsPtr))
	sb.WriteString("mph_gc_push_root(ctx, (void**)&_m); ")
	for key, val := range hl.Pairs {
		keyCode, err := c.compileExpression(key, prefix, fn)
		if err != nil {
			return "", err
		}
		valCode, err := c.compileExpression(val, prefix, fn)
		if err != nil {
			return "", err
		}
		keyCast := fmt.Sprintf("(void*)%s", keyCode)
		if kindEnum == "MPH_KEY_INT" {
			keyCast = fmt.Sprintf("(void*)(int64_t)%s", keyCode)
		}
		sb.WriteString(fmt.Sprintf("mph_map_set(ctx, _m, %s, (void*)%s); ", keyCast, valCode))
	}
	sb.WriteString("mph_gc_pop_roots(ctx, 1); _m; })")
	return sb.String(), nil
}

func (c *Compiler) compileIndex(ie *parser.IndexExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	leftCode, err := c.compileExpression(ie.Left, prefix, fn)
	if err != nil {
		return "", err
	}
	indexCode, err := c.compileExpression(ie.Index, prefix, fn)
	if err != nil {
		return "", err
	}

	leftType := c.checker.Types[ie.Left]
	if leftType == nil {
		return "", fmt.Errorf("unknown type for index expression")
	}

	if leftType.Kind() == checker.KindString {
		leftRef := leftCode
		var roots []rootTemp
		if c.isPointerCheckerType(leftType) {
			leftTemp := c.nextTemp("str")
			roots = append(roots, rootTemp{
				cType: c.mapCheckerTypeToC(leftType, prefix),
				name:  leftTemp,
				value: leftCode,
			})
			leftRef = leftTemp
		}
		expr := fmt.Sprintf("((MorphString*)%s)->data[%s]", leftRef, indexCode)
		return c.wrapWithRoots(roots, expr, "mph_int"), nil
	} else if leftType.Kind() == checker.KindArray {
		if at, ok := leftType.(*checker.ArrayType); ok {
			elemCType := c.mapCheckerTypeToC(at.Element, prefix)
			leftRef := leftCode
			var roots []rootTemp
			if c.isPointerCheckerType(leftType) {
				leftTemp := c.nextTemp("lhs")
				roots = append(roots, rootTemp{
					cType: c.mapCheckerTypeToC(leftType, prefix),
					name:  leftTemp,
					value: leftCode,
				})
				leftRef = leftTemp
			}
			expr := fmt.Sprintf("(*(%s*)mph_array_at(ctx, %s, %s))", elemCType, leftRef, indexCode)
			retType := elemCType
			return c.wrapWithRoots(roots, expr, retType), nil
		}
	} else if leftType.Kind() == checker.KindMap {
		if mt, ok := leftType.(*checker.MapType); ok {
			valCType := c.mapCheckerTypeToC(mt.Value, prefix)
			leftRef := leftCode
			indexRef := indexCode
			var roots []rootTemp
			if c.isPointerCheckerType(leftType) {
				leftTemp := c.nextTemp("map")
				roots = append(roots, rootTemp{
					cType: c.mapCheckerTypeToC(leftType, prefix),
					name:  leftTemp,
					value: leftCode,
				})
				leftRef = leftTemp
			}
			if c.isPointerCheckerType(mt.Key) {
				keyTemp := c.nextTemp("key")
				roots = append(roots, rootTemp{
					cType: c.mapCheckerTypeToC(mt.Key, prefix),
					name:  keyTemp,
					value: indexCode,
				})
				indexRef = keyTemp
			}
			keyCast := fmt.Sprintf("(void*)%s", indexRef)
			if mt.Key.Kind() == checker.KindInt {
				keyCast = fmt.Sprintf("(void*)(int64_t)%s", indexRef)
			}

			if c.isPrimitive(mt.Value) {
				expr := fmt.Sprintf("(%s)(int64_t)mph_map_get(ctx, %s, %s)", valCType, leftRef, keyCast)
				return c.wrapWithRoots(roots, expr, valCType), nil
			}
			expr := fmt.Sprintf("(%s)mph_map_get(ctx, %s, %s)", valCType, leftRef, keyCast)
			return c.wrapWithRoots(roots, expr, valCType), nil
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
			if err != nil {
				return "", err
			}
			var roots []rootTemp
			argRef := arg
			if argType := c.checker.Types[call.Arguments[1]]; argType != nil && c.isPointerCheckerType(argType) {
				temp := c.nextTemp("spawn_arg")
				roots = append(roots, rootTemp{
					cType: c.mapCheckerTypeToC(argType, prefix),
					name:  temp,
					value: arg,
				})
				argRef = temp
			}
			expr := fmt.Sprintf("mph_thread_spawn((MorphEntryFunction)mph_%s, (void*)%s)", ident.Value, argRef)
			return c.wrapWithRoots(roots, expr, "void"), nil
		}
	}
	return "", fmt.Errorf("luncurkan expects named function")
}

func (c *Compiler) compileDelete(call *parser.CallExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	if len(call.Arguments) != 2 {
		return "", fmt.Errorf("hapus expects 2 arguments")
	}
	mapCode, err := c.compileExpression(call.Arguments[0], prefix, fn)
	if err != nil {
		return "", err
	}
	keyCode, err := c.compileExpression(call.Arguments[1], prefix, fn)
	if err != nil {
		return "", err
	}
	mapType := c.checker.Types[call.Arguments[0]]
	if mapType == nil || mapType.Kind() != checker.KindMap {
		return "", fmt.Errorf("hapus arg 1 must be map")
	}
	mt := mapType.(*checker.MapType)
	mapRef := mapCode
	keyRef := keyCode
	var roots []rootTemp
	if c.isPointerCheckerType(mapType) {
		temp := c.nextTemp("map")
		roots = append(roots, rootTemp{
			cType: c.mapCheckerTypeToC(mapType, prefix),
			name:  temp,
			value: mapCode,
		})
		mapRef = temp
	}
	if c.isPointerCheckerType(mt.Key) {
		temp := c.nextTemp("key")
		roots = append(roots, rootTemp{
			cType: c.mapCheckerTypeToC(mt.Key, prefix),
			name:  temp,
			value: keyCode,
		})
		keyRef = temp
	}
	keyCast := fmt.Sprintf("(void*)%s", keyRef)
	if mt.Key.Kind() == checker.KindInt {
		keyCast = fmt.Sprintf("(void*)(int64_t)%s", keyRef)
	}
	expr := fmt.Sprintf("mph_map_delete(ctx, %s, %s)", mapRef, keyCast)
	return c.wrapWithRoots(roots, expr, "void"), nil
}

func (c *Compiler) compileLen(call *parser.CallExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	if len(call.Arguments) != 1 {
		return "", fmt.Errorf("panjang expects 1 argument")
	}
	argCode, err := c.compileExpression(call.Arguments[0], prefix, fn)
	if err != nil {
		return "", err
	}
	argType := c.checker.Types[call.Arguments[0]]
	if argType == nil {
		return "", fmt.Errorf("unknown type for panjang")
	}
	if argType.Kind() == checker.KindMap {
		var roots []rootTemp
		argRef := argCode
		if c.isPointerCheckerType(argType) {
			temp := c.nextTemp("len_arg")
			roots = append(roots, rootTemp{
				cType: c.mapCheckerTypeToC(argType, prefix),
				name:  temp,
				value: argCode,
			})
			argRef = temp
		}
		expr := fmt.Sprintf("mph_map_len(ctx, %s)", argRef)
		return c.wrapWithRoots(roots, expr, "mph_int"), nil
	} else if argType.Kind() == checker.KindArray {
		var roots []rootTemp
		argRef := argCode
		if c.isPointerCheckerType(argType) {
			temp := c.nextTemp("len_arg")
			roots = append(roots, rootTemp{
				cType: c.mapCheckerTypeToC(argType, prefix),
				name:  temp,
				value: argCode,
			})
			argRef = temp
		}
		expr := fmt.Sprintf("((MorphArray*)%s)->length", argRef)
		return c.wrapWithRoots(roots, expr, "mph_int"), nil
	} else if argType.Kind() == checker.KindString {
		var roots []rootTemp
		argRef := argCode
		if c.isPointerCheckerType(argType) {
			temp := c.nextTemp("len_arg")
			roots = append(roots, rootTemp{
				cType: c.mapCheckerTypeToC(argType, prefix),
				name:  temp,
				value: argCode,
			})
			argRef = temp
		}
		expr := fmt.Sprintf("((MorphString*)%s)->length", argRef)
		return c.wrapWithRoots(roots, expr, "mph_int"), nil
	}
	return "", fmt.Errorf("panjang not supported for type %s", argType.String())
}

func (c *Compiler) compileTypeAssertion(call *parser.CallExpression, prefix string, fn *parser.FunctionLiteral) (string, error) {
	if len(call.Arguments) != 2 {
		return "", fmt.Errorf("assert expects 2 args")
	}
	ifaceCode, err := c.compileExpression(call.Arguments[0], prefix, fn)
	if err != nil {
		return "", err
	}
	targetType := c.checker.Types[call.Arguments[1]]
	if targetType == nil || targetType.Kind() != checker.KindStruct {
		return "", fmt.Errorf("invalid assertion type")
	}
	st := targetType.(*checker.StructType)
	targetID := c.getStructID(st.Name)
	targetCType := c.mapCheckerTypeToC(st, prefix)
	ifaceTemp := c.nextTemp("iface")
	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("MorphInterface %s = %s; ", ifaceTemp, ifaceCode))
	sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s.instance); ", ifaceTemp))
	sb.WriteString(fmt.Sprintf("%s _ret = (%s)mph_assert_type(ctx, %s, %d); ", targetCType, targetCType, ifaceTemp, targetID))
	sb.WriteString("mph_gc_pop_roots(ctx, 1); ")
	sb.WriteString("_ret; })")
	return sb.String(), nil
}

func (c *Compiler) compileInterfaceConversion(iface *checker.InterfaceType, st *checker.StructType, srcCode string, prefix string) (string, error) {
	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString("static void* _vt[] = { ")
	ifaceMethods := make([]string, 0, len(iface.Methods))
	for k := range iface.Methods {
		ifaceMethods = append(ifaceMethods, k)
	}
	sort.Strings(ifaceMethods)
	for i, mName := range ifaceMethods {
		if i > 0 {
			sb.WriteString(", ")
		}
		structMethodName := fmt.Sprintf("%s%s_%s", prefix, st.Name, mName)
		if st.Module != "" {
			structMethodName = mangle(st.Module) + st.Name + "_" + mName
		}
		sb.WriteString("(void*)" + structMethodName)
	}
	sb.WriteString(" }; ")
	structID := c.getStructID(st.Name)
	sb.WriteString(fmt.Sprintf("(MorphInterface){ .instance = (void*)%s, .vtable = _vt, .type_id = %d };", srcCode, structID))
	sb.WriteString(" })")
	return sb.String(), nil
}

func (c *Compiler) compileInterfaceCall(call *parser.CallExpression, mem *parser.MemberExpression, iface *checker.InterfaceType, prefix string, fn *parser.FunctionLiteral) (string, error) {
	ifaceMethods := make([]string, 0, len(iface.Methods))
	for k := range iface.Methods {
		ifaceMethods = append(ifaceMethods, k)
	}
	sort.Strings(ifaceMethods)
	methodIndex := -1
	for i, name := range ifaceMethods {
		if name == mem.Member.Value {
			methodIndex = i
			break
		}
	}
	if methodIndex == -1 {
		return "", fmt.Errorf("method not found")
	}

	objCode, err := c.compileExpression(mem.Object, prefix, fn)
	if err != nil {
		return "", err
	}

	objTemp := c.nextTemp("iface")
	type argInfo struct {
		code      string
		cType     string
		temp      string
		needsRoot bool
	}

	var args []string
	var argInfos []argInfo
	args = append(args, "ctx")
	args = append(args, fmt.Sprintf("%s.instance", objTemp))
	for _, arg := range call.Arguments {
		code, err := c.compileExpression(arg, prefix, fn)
		if err != nil {
			return "", err
		}
		info := argInfo{code: code}
		argType := c.checker.Types[arg]
		if argType != nil && c.isPointerCheckerType(argType) {
			info.needsRoot = true
			info.cType = c.mapCheckerTypeToC(argType, prefix)
			info.temp = c.nextTemp("arg")
		}
		argInfos = append(argInfos, info)
	}
	for _, info := range argInfos {
		if info.needsRoot {
			args = append(args, info.temp)
		} else {
			args = append(args, info.code)
		}
	}

	methodType := iface.Methods[mem.Member.Value]
	retType := "void"
	if len(methodType.ReturnTypes) > 0 {
		retType = c.mapCheckerTypeToC(methodType.ReturnTypes[0], prefix)
	}
	var paramTypes []string
	paramTypes = append(paramTypes, "MorphContext*")
	paramTypes = append(paramTypes, "void*")
	for _, p := range methodType.Parameters {
		paramTypes = append(paramTypes, c.mapCheckerTypeToC(p, prefix))
	}
	fnPtrType := fmt.Sprintf("%s (*)(%s)", retType, strings.Join(paramTypes, ", "))
	callExpr := fmt.Sprintf("((%s)(%s.vtable[%d]))(%s)", fnPtrType, objTemp, methodIndex, strings.Join(args, ", "))
	var sb strings.Builder
	sb.WriteString("({ ")
	sb.WriteString(fmt.Sprintf("MorphInterface %s = %s; ", objTemp, objCode))
	sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s.instance); ", objTemp))
	rootCount := 1
	for _, info := range argInfos {
		if !info.needsRoot {
			continue
		}
		sb.WriteString(fmt.Sprintf("%s %s = %s; ", info.cType, info.temp, info.code))
		sb.WriteString(fmt.Sprintf("mph_gc_push_root(ctx, (void**)&%s); ", info.temp))
		rootCount++
	}
	if retType == "void" {
		sb.WriteString(fmt.Sprintf("%s; ", callExpr))
		sb.WriteString(fmt.Sprintf("mph_gc_pop_roots(ctx, %d); ", rootCount))
		sb.WriteString("})")
		return sb.String(), nil
	}
	sb.WriteString(fmt.Sprintf("%s _ret = %s; ", retType, callExpr))
	sb.WriteString(fmt.Sprintf("mph_gc_pop_roots(ctx, %d); ", rootCount))
	sb.WriteString("_ret; })")
	return sb.String(), nil
}

func (c *Compiler) isPrimitive(t checker.Type) bool {
	if t == nil {
		return true
	}
	switch t.Kind() {
	case checker.KindInt, checker.KindFloat, checker.KindBool:
		return true
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
	// Resolve to Checker Type to handle imports/mangling correctly
	ct := c.resolveTypeNode(t)
	if ct != nil && ct.Kind() != checker.KindUnknown {
		return c.mapCheckerTypeToC(ct, prefix), nil
	}

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
		// Fallback (should be handled by resolveTypeNode if imports are correct)
		return "mph_" + ty.Package.Value + "_" + ty.Name.Value + "*", nil
	case *parser.ArrayType:
		return "MorphArray*", nil
	case *parser.MapType:
		return "MorphMap*", nil
	case *parser.FunctionType:
		return "MorphClosure*", nil
	}
	return "", fmt.Errorf("unknown type node: %T", t)
}

// New method to generate RTTI
func (c *Compiler) compileStructRTTI(node parser.Node) error {
	c.output.WriteString("// RTTI Definitions\n")

	var generateRTTI func(prog *parser.Program, prefix string) error
	generateRTTI = func(prog *parser.Program, prefix string) error {
		for _, stmt := range prog.Statements {
			if s, ok := stmt.(*parser.StructStatement); ok {
				name := prefix + s.Name.Value
				// Collect pointer offsets
				var offsets []string
				for _, f := range s.Fields {
					if c.isPointerType(f.Type) {
						offsets = append(offsets, fmt.Sprintf("offsetof(%s, %s)", name, f.Name))
					}
				}

				numPtrs := len(offsets)
				offsetsStr := "NULL"
				if numPtrs > 0 {
					offsetsStr = fmt.Sprintf("(size_t[]){%s}", strings.Join(offsets, ", "))
				}

				c.typeDefs.WriteString(fmt.Sprintf("MorphTypeInfo mph_ti_%s = { \"%s\", sizeof(%s), %d, %s };\n", name, s.Name.Value, name, numPtrs, offsetsStr))
			}
		}
		return nil
	}

	// Modules
	for path, mod := range c.checker.ModuleCache {
		if mod.Program == nil {
			continue
		}
		if err := generateRTTI(mod.Program, mangle(path)); err != nil {
			return err
		}
	}

	// Main
	if prog, ok := node.(*parser.Program); ok {
		if err := generateRTTI(prog, "mph_"); err != nil {
			return err
		}
	}
	c.output.WriteString("\n")
	return nil
}

func (c *Compiler) isPointerType(t parser.TypeNode) bool {
	// Strings, Arrays, Maps, Closures, Structs are pointers
	switch ty := t.(type) {
	case *parser.SimpleType:
		switch ty.Name {
		case "int", "float", "bool", "void":
			return false
		default:
			return true // Structs are pointers
		}
	case *parser.QualifiedType:
		return true
	case *parser.ArrayType:
		return true
	case *parser.MapType:
		return true
	case *parser.FunctionType:
		return true
	}
	return false
}
