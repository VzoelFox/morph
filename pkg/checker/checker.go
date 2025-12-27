package checker

import (
	"strings"

	"github.com/VzoelFox/morph/pkg/parser"
)

type Importer interface {
	Import(path string) (*parser.Program, error)
}

type Checker struct {
	Errors         []TypeError
	Warnings       []TypeWarning
	Types          map[parser.Node]Type
	scope          *Scope
	returnStack    []Type
	importer       Importer
	ModuleCache    map[string]*ModuleType
	loadingModules map[string]bool
	CurrentModule  string
	recursionDepth int // Track recursion depth to prevent stack overflow
}

// Maximum recursion depth to prevent stack overflow from deeply nested expressions
const MAX_RECURSION_DEPTH = 1000

func IsChannel(t Type) bool {
	return t.Kind() == KindChannel
}

func New() *Checker {
	c := &Checker{
		Errors:         []TypeError{},
		Warnings:       []TypeWarning{},
		Types:          make(map[parser.Node]Type),
		scope:          NewScope(nil),
		returnStack:    []Type{},
		ModuleCache:    make(map[string]*ModuleType),
		loadingModules: make(map[string]bool),
	}

	// Register primitive types
	c.scope.DefineType("int", IntType)
	c.scope.DefineType("float", FloatType)
	c.scope.DefineType("string", StringType)
	c.scope.DefineType("bool", BoolType)
	c.scope.DefineType("error", UserErrorType) // User-facing 'Error' type
	c.scope.DefineType("channel", ChannelType)

	// Register built-in functions (Stdlib Prototype)
	// parse_int(string) -> (int, string)
	parseIntType := &FunctionType{
		Parameters:  []Type{StringType},
		ReturnTypes: []Type{IntType, StringType},
	}
	c.scope.DefineVariable("parse_int", parseIntType, true, 0, 0)

	// native_print(string) -> void
	nativePrintType := &FunctionType{
		Parameters:  []Type{StringType},
		ReturnTypes: []Type{VoidType},
	}
	c.scope.DefineVariable("native_print", nativePrintType, true, 0, 0)

	// native_print_int(int) -> void
	nativePrintIntType := &FunctionType{
		Parameters:  []Type{IntType},
		ReturnTypes: []Type{VoidType},
	}
	c.scope.DefineVariable("native_print_int", nativePrintIntType, true, 0, 0)

	// native_print_error(error) -> void
	nativePrintErrorType := &FunctionType{
		Parameters:  []Type{UserErrorType},
		ReturnTypes: []Type{VoidType},
	}
	c.scope.DefineVariable("native_print_error", nativePrintErrorType, true, 0, 0)

	// index(string, string) -> int
	indexType := &FunctionType{
		Parameters:  []Type{StringType, StringType},
		ReturnTypes: []Type{IntType},
	}
	c.scope.DefineVariable("index", indexType, true, 0, 0)

	// trim(string, string) -> string
	trimType := &FunctionType{
		Parameters:  []Type{StringType, StringType},
		ReturnTypes: []Type{StringType},
	}
	c.scope.DefineVariable("trim", trimType, true, 0, 0)

	// split(string, string) -> []string
	splitType := &FunctionType{
		Parameters:  []Type{StringType, StringType},
		ReturnTypes: []Type{&ArrayType{Element: StringType}},
	}
	c.scope.DefineVariable("split", splitType, true, 0, 0)

	// substring(string, int, int) -> string
	substringType := &FunctionType{
		Parameters:  []Type{StringType, IntType, IntType},
		ReturnTypes: []Type{StringType},
	}
	c.scope.DefineVariable("substring", substringType, true, 0, 0)

	// Concurrency Primitives (int only for MVP)
	// saluran_baru() -> channel
	c.scope.DefineVariable("saluran_baru", &FunctionType{
		Parameters:  []Type{},
		ReturnTypes: []Type{ChannelType},
	}, true, 0, 0)

	// kirim(channel, int) -> void
	c.scope.DefineVariable("kirim", &FunctionType{
		Parameters:  []Type{ChannelType, IntType},
		ReturnTypes: []Type{VoidType},
	}, true, 0, 0)

	// terima(channel) -> int
	c.scope.DefineVariable("terima", &FunctionType{
		Parameters:  []Type{ChannelType},
		ReturnTypes: []Type{IntType},
	}, true, 0, 0)

	// Map Primitives
	// hapus(map[K]V, K) -> void
	// Note: Generic functions not fully supported in Types, but logic handled in Compiler/Evaluator.
	// We define it as a global variable but specific checking logic might be needed if strictly typed.
	// For now, let's allow it to pass type checking by treating as custom validation or "magic" builtins.
	// Since the Checker checks definition existence, we define it here.
	// Ideally, `hapus` should be polymorphic. For now, we rely on `checkCall` special handling or relaxed check.
	// But `Checker` is strict. Let's make `hapus` available as a known identifier, but its type
	// will be checked specially in `checkCall` if we want full generic support.
	//
	// Workaround: Define `hapus` with `UnknownType` parameters so `checkCall` passes?
	// No, `checkCall` verifies argument types against parameter types.
	//
	// Better approach for MVP: `hapus` is not a variable, it's a keyword-like builtin handled in `checkExpression`.
	// However, to keep it simple, we define it here so `Identifier` lookup succeeds,
	// and we might need to handle the "Generic" aspect.
	// Let's use `KindUnknown` for params to allow anything for now, or just handle `hapus` explicitly in `checkExpression` (CallExpression).
	// Current `checkExpression` handles `luncurkan` explicitly. We should do the same for `hapus` and `panjang`.
	// So we DO NOT define them here as variables to avoid rigid type signature.
	// Instead, we ensure `checkExpression` handles them BEFORE lookup.
	// Wait, if we don't define them, `checkExpression` -> `Identifier` lookup fails?
	// Yes.
	// So we should define them as special types or handle `CallExpression` to check name BEFORE checking function expression.

	return c
}

func (c *Checker) SetImporter(i Importer) {
	c.importer = i
}

func (c *Checker) pushReturn(t Type) { c.returnStack = append(c.returnStack, t) }
func (c *Checker) popReturn() {
	if len(c.returnStack) > 0 {
		c.returnStack = c.returnStack[:len(c.returnStack)-1]
	}
}
func (c *Checker) currentReturn() Type {
	if len(c.returnStack) == 0 {
		return nil
	}
	return c.returnStack[len(c.returnStack)-1]
}

func (c *Checker) Check(program *parser.Program) {
	// Pass 1: Collect Definitions (Structs, Functions, Vars)
	c.collectDefinitions(program)
	c.resolveStructFields(program)
	// c.checkStructCycles(program) // Disabled: Structs are reference types, so cycles are allowed.

	// Pass 2: Check Bodies
	c.checkNodes(program.Statements)
}

func (c *Checker) collectDefinitions(program *parser.Program) {
	// 0. Process Imports first
	for _, stmt := range program.Statements {
		if imp, ok := stmt.(*parser.ImportStatement); ok {
			if imp == nil {
				continue
			}
			c.checkImport(imp)
		}
	}

	// 1. Structs first (types)
	for _, stmt := range program.Statements {
		if s, ok := stmt.(*parser.StructStatement); ok {
			if s == nil {
				continue
			}
			c.defineStruct(s)
		} else if i, ok := stmt.(*parser.InterfaceStatement); ok {
			if i == nil {
				continue
			}
			c.defineInterface(i)
		}
	}

	// 2. Global Functions and Vars
	for _, stmt := range program.Statements {
		switch s := stmt.(type) {
		case *parser.VarStatement:
			if s == nil {
				continue
			}
			c.defineVar(s)
		case *parser.ExpressionStatement:
			if s == nil {
				continue
			}
			if fn, ok := s.Expression.(*parser.FunctionLiteral); ok {
				if fn.Name != "" {
					c.defineFunction(fn)
				}
			}
		}
	}
}

func (c *Checker) checkImport(imp *parser.ImportStatement) {
	if c.importer == nil {
		return
	}

	path := imp.Path
	if c.loadingModules[path] {
		c.addError(imp.Token.Line, imp.Token.Column, "Import cycle detected: %s", path)
		return
	}

	if mod, cached := c.ModuleCache[path]; cached {
		c.registerModule(imp, mod)
		return
	}

	c.loadingModules[path] = true
	defer func() { delete(c.loadingModules, path) }()

	importedProg, err := c.importer.Import(path)
	if err != nil {
		c.addError(imp.Token.Line, imp.Token.Column, "Failed to import module '%s': %v", path, err)
		return
	}

	// Check imported module (recursively)
	subChecker := New()
	subChecker.importer = c.importer             // Share importer
	subChecker.ModuleCache = c.ModuleCache       // Share cache
	subChecker.loadingModules = c.loadingModules // Share loading state (for cycle detection)

	subChecker.CurrentModule = path

	// Collect definitions ONLY (Pass 1)
	subChecker.Check(importedProg)

	// Propagate errors from imported module
	if len(subChecker.Errors) > 0 {
		c.Errors = append(c.Errors, subChecker.Errors...)
	}

	// Merge types from sub-checker to main checker so Compiler can access them
	for node, typ := range subChecker.Types {
		c.Types[node] = typ
	}

	// Harvest Exports (Uppercase)
	exports := make(map[string]ExportInfo)

	// Collect Variables (Functions/Vars) from Scope
	for name, sym := range subChecker.scope.variables {
		if isExported(name) {
			exports[name] = ExportInfo{Type: sym.Type, IsType: false}
		}
	}
	// Collect Types (Structs, Interfaces) from Scope
	for name, typ := range subChecker.scope.types {
		if isExported(name) {
			exports[name] = ExportInfo{Type: typ, IsType: true}
		}
	}

	mod := &ModuleType{
		Name:    path,
		Exports: exports,
		Program: importedProg,
	}

	c.ModuleCache[path] = mod
	c.registerModule(imp, mod)
}

func (c *Checker) registerModule(imp *parser.ImportStatement, mod *ModuleType) {
	// 2. From Import: dari "math" ambil Sin -> var Sin = mod.Exports["Sin"]
	if len(imp.Identifiers) > 0 {
		for _, name := range imp.Identifiers {
			if info, ok := mod.Exports[name]; ok {
				if info.IsType {
					c.scope.DefineType(name, info.Type)
				} else {
					c.scope.DefineVariable(name, info.Type, true, imp.Token.Line, imp.Token.Column)
				}
			} else {
				c.addError(imp.Token.Line, imp.Token.Column, "Module '%s' does not export '%s'", imp.Path, name)
			}
		}
	} else {
		// Define module as variable
		parts := strings.Split(imp.Path, "/")
		name := parts[len(parts)-1]
		// Remove extension if any
		if idx := strings.Index(name, "."); idx != -1 {
			name = name[:idx]
		}

		c.scope.DefineVariable(name, mod, true, imp.Token.Line, imp.Token.Column)
	}
}

func isExported(name string) bool {
	if len(name) == 0 {
		return false
	}
	return name[0] >= 'A' && name[0] <= 'Z'
}

func (c *Checker) defineStruct(s *parser.StructStatement) {
	st := &StructType{
		Name:       s.Name.Value,
		Module:     c.CurrentModule,
		Fields:     make(map[string]Type),
		FieldOrder: make([]string, 0, len(s.Fields)),
		Methods:    make(map[string]*FunctionType),
	}
	for _, f := range s.Fields {
		st.FieldOrder = append(st.FieldOrder, f.Name)
	}
	c.scope.DefineType(s.Name.Value, st)
}

func (c *Checker) defineInterface(i *parser.InterfaceStatement) {
	it := &InterfaceType{
		Name:    i.Name.Value,
		Methods: make(map[string]*FunctionType),
	}

	for _, methodNode := range i.Methods {
		paramTypes := make([]Type, len(methodNode.Parameters))
		for i, p := range methodNode.Parameters {
			paramTypes[i] = c.resolveType(p.Type)
		}

		returnTypes := make([]Type, len(methodNode.ReturnTypes))
		for i, rt := range methodNode.ReturnTypes {
			returnTypes[i] = c.resolveType(rt)
		}

		if len(returnTypes) == 0 {
			returnTypes = append(returnTypes, VoidType)
		}

		it.Methods[methodNode.Name] = &FunctionType{
			Parameters:  paramTypes,
			ReturnTypes: returnTypes,
		}
	}

	c.scope.DefineType(i.Name.Value, it)
}

func (c *Checker) resolveStructFields(program *parser.Program) {
	for _, stmt := range program.Statements {
		if s, ok := stmt.(*parser.StructStatement); ok {
			t, _ := c.scope.LookupType(s.Name.Value)
			st := t.(*StructType)
			for _, field := range s.Fields {
				fieldType := c.resolveType(field.Type)
				st.Fields[field.Name] = fieldType
			}
		}
	}
}

func (c *Checker) defineVar(s *parser.VarStatement) {
	var t Type = UnknownType
	if s.Type != nil {
		t = c.resolveType(s.Type)
	}
	for _, name := range s.Names {
		if existing, ok := c.scope.variables[name.Value]; ok {
			existingTypeStr := "<nil>"
			if existing.Type != nil {
				existingTypeStr = existing.Type.String()
			}
			c.addError(name.Token.Line, name.Token.Column, "Global variable '%s' already declared (type: %s)", name.Value, existingTypeStr)
			continue
		}
		c.scope.DefineVariable(name.Value, t, s.IsConst, name.Token.Line, name.Token.Column)
	}
}

func (c *Checker) defineFunction(fn *parser.FunctionLiteral) {
	paramTypes := []Type{}
	for _, p := range fn.Parameters {
		paramTypes = append(paramTypes, c.resolveType(p.Type))
	}

	returnTypes := []Type{}
	for _, rt := range fn.ReturnTypes {
		returnTypes = append(returnTypes, c.resolveType(rt))
	}
	if len(returnTypes) == 0 {
		returnTypes = append(returnTypes, VoidType)
	}

	ft := &FunctionType{Parameters: paramTypes, ReturnTypes: returnTypes}

	// If method, attach to struct
	if fn.Receiver != nil {
		recvType := c.resolveType(fn.Receiver.Type)
		if recvType.Kind() != KindStruct {
			c.addError(fn.Receiver.Token.Line, fn.Receiver.Token.Column, "Method receiver must be a struct, got %s", recvType.String())
			return
		}

		st := recvType.(*StructType)
		st.Methods[fn.Name] = ft
	} else {
		// Normal function
		c.scope.DefineVariable(fn.Name, ft, true, fn.Token.Line, fn.Token.Column)
	}
}

func (c *Checker) resolveType(n parser.TypeNode) Type {
	t := c.resolveTypeInternal(n)
	c.Types[n] = t
	return t
}

func (c *Checker) resolveTypeInternal(n parser.TypeNode) Type {
	switch t := n.(type) {
	case *parser.SimpleType:
		switch t.Name {
		case "int":
			return IntType
		case "float":
			return FloatType
		case "string":
			return StringType
		case "bool":
			return BoolType
		case "void":
			return VoidType
		default:
			if typ, ok := c.scope.LookupType(t.Name); ok {
				return typ
			}
			c.addError(t.Token.Line, t.Token.Column, "Unknown type: %s", t.Name)
			return UnknownType
		}
	case *parser.QualifiedType:
		modType, ok := c.scope.LookupVariable(t.Package.Value)
		if !ok {
			c.addError(t.Token.Line, t.Token.Column, "Unknown module: %s", t.Package.Value)
			return UnknownType
		}

		if modType.Kind() != KindModule {
			c.addError(t.Token.Line, t.Token.Column, "Expected module, got %s", modType.String())
			return UnknownType
		}

		mod := modType.(*ModuleType)
		if info, ok := mod.Exports[t.Name.Value]; ok {
			// Is it a Type?
			if info.IsType {
				return info.Type
			}
			c.addError(t.Token.Line, t.Token.Column, "'%s' is not a type", t.Name.Value)
			return UnknownType
		}

		c.addError(t.Token.Line, t.Token.Column, "Module '%s' does not export type '%s'", mod.Name, t.Name.Value)
		return UnknownType

	case *parser.ArrayType:
		elem := c.resolveType(t.Element)
		return &ArrayType{Element: elem}
	case *parser.MapType:
		key := c.resolveType(t.Key)
		val := c.resolveType(t.Value)
		return &MapType{Key: key, Value: val}
	case *parser.FunctionType:
		paramTypes := []Type{}
		for _, p := range t.Parameters {
			paramTypes = append(paramTypes, c.resolveType(p))
		}
		returnTypes := []Type{}
		for _, rt := range t.ReturnTypes {
			returnTypes = append(returnTypes, c.resolveType(rt))
		}
		if len(returnTypes) == 0 {
			returnTypes = append(returnTypes, VoidType)
		}
		return &FunctionType{Parameters: paramTypes, ReturnTypes: returnTypes}
	}
	return UnknownType
}

func (c *Checker) checkNodes(stmts []parser.Statement) {
	for i, s := range stmts {
		c.checkStatement(s)
		if c.isReturn(s) && i < len(stmts)-1 {
			next := stmts[i+1]
			line, col := c.getStatementLineCol(next)
			c.addWarning(line, col, "Unreachable code detected")
		}
	}
}

func (c *Checker) allPathsReturn(node interface{}) bool {
	switch n := node.(type) {
	case *parser.ReturnStatement:
		return true

	case *parser.BlockStatement:
		for _, stmt := range n.Statements {
			if c.allPathsReturn(stmt) {
				return true
			}
		}
		return false

	case *parser.ExpressionStatement:
		return c.allPathsReturn(n.Expression)

	case *parser.IfExpression:
		if n.Alternative == nil {
			return false
		}
		if !c.allPathsReturn(n.Consequence) {
			return false
		}
		for _, clause := range n.ElseIfs {
			if !c.allPathsReturn(clause.Consequence) {
				return false
			}
		}
		return c.allPathsReturn(n.Alternative)

	case *parser.WhileExpression:
		return false

	default:
		return false
	}
}

func (c *Checker) checkStatement(s parser.Statement) {
	switch stmt := s.(type) {
	case *parser.VarStatement:
		c.checkVarStatement(stmt)
	case *parser.ExpressionStatement:
		c.checkExpression(stmt.Expression)
	case *parser.ReturnStatement:
		var actual Type = VoidType
		if len(stmt.ReturnValues) == 1 {
			actual = c.checkExpression(stmt.ReturnValues[0])
		} else if len(stmt.ReturnValues) > 1 {
			types := []Type{}
			for _, v := range stmt.ReturnValues {
				types = append(types, c.checkExpression(v))
			}
			actual = &MultiType{Types: types}
		}

		expected := c.currentReturn()
		if expected != nil {
			if !actual.AssignableTo(expected) && actual.Kind() != KindUnknown {
				c.addError(stmt.Token.Line, stmt.Token.Column, "Return type mismatch: expected %s, got %s", expected.String(), actual.String())
			}
		}
	case *parser.AssignmentStatement:
		c.checkAssignment(stmt)
	case *parser.StructStatement:
		// Fields resolved in pass 1.5
	case *parser.BlockStatement:
		c.enterScope()
		defer c.leaveScope()
		c.checkNodes(stmt.Statements)
	case *parser.SwitchStatement:
		c.checkSwitchStatement(stmt)
	}
}

func (c *Checker) checkSwitchStatement(s *parser.SwitchStatement) {
	condType := c.checkExpression(s.Condition)

	// Validate Condition Type
	if condType.Kind() != KindUnknown {
		switch condType.Kind() {
		case KindInt, KindBool, KindString:
			// Allowed
		default:
			c.addError(s.Token.Line, s.Token.Column, "Switch condition must be Int, Bool, or String, got %s", condType.String())
		}
	}

	for _, cc := range s.Cases {
		for _, val := range cc.Values {
			valType := c.checkExpression(val)
			if condType.Kind() != KindUnknown && valType.Kind() != KindUnknown {
				if !valType.AssignableTo(condType) {
					c.addError(cc.Token.Line, cc.Token.Column, "Case value type mismatch: expected %s, got %s", condType.String(), valType.String())
				}
			}
		}
		c.checkBlockStatement(cc.Body)
	}

	if s.Default != nil {
		c.checkBlockStatement(s.Default)
	}
}

func (c *Checker) checkBlockStatement(b *parser.BlockStatement) {
	c.enterScope()
	defer c.leaveScope()
	c.checkNodes(b.Statements)
}

func (c *Checker) checkAssignment(s *parser.AssignmentStatement) {
	// 1. Analyze RHS Types
	var valueTypes []Type
	if len(s.Values) == 1 {
		// Single expression could be MultiType (unpacking)
		t := c.checkExpression(s.Values[0])
		if mt, ok := t.(*MultiType); ok {
			valueTypes = mt.Types
		} else {
			valueTypes = []Type{t}
		}
	} else {
		// Multiple expressions: 1-to-1
		for _, v := range s.Values {
			valueTypes = append(valueTypes, c.checkExpression(v))
		}
	}

	// 2. Count mismatch
	if len(s.Names) != len(valueTypes) {
		c.addError(s.Token.Line, s.Token.Column, "Assignment count mismatch: %d = %d", len(s.Names), len(valueTypes))
	}

	// 3. Validate LHS
	for i, nameExpr := range s.Names {
		if i >= len(valueTypes) {
			break
		}

		// Check mutability
		if ident, ok := nameExpr.(*parser.Identifier); ok {
			if sym, ok := c.scope.LookupSymbol(ident.Value); ok {
				if sym.IsConst {
					c.addError(s.Token.Line, s.Token.Column, "Cannot reassign constant '%s'", ident.Value)
				}
			}
		}

		// Check string assignment via index
		if idxExpr, ok := nameExpr.(*parser.IndexExpression); ok {
			leftObjType := c.checkExpression(idxExpr.Left)
			if leftObjType.Kind() == KindString {
				c.addError(s.Token.Line, s.Token.Column, "Cannot assign to string index (immutable)")
			}
		}

		lhsType := c.checkExpression(nameExpr)
		rhsType := valueTypes[i]

		if lhsType.Kind() != KindUnknown && rhsType.Kind() != KindUnknown {
			if !rhsType.AssignableTo(lhsType) {
				c.addError(s.Token.Line, s.Token.Column, "Type mismatch in assignment: expected %s, got %s", lhsType.String(), rhsType.String())
			}
		}
	}
}

func (c *Checker) checkVarStatement(s *parser.VarStatement) {
	// 1. Analyze RHS Types
	var valueTypes []Type
	if len(s.Values) == 1 {
		// Single expression could be MultiType (unpacking)
		t := c.checkExpression(s.Values[0])
		if mt, ok := t.(*MultiType); ok {
			valueTypes = mt.Types
		} else {
			valueTypes = []Type{t}
		}
	} else {
		// Multiple expressions: 1-to-1
		for _, v := range s.Values {
			valueTypes = append(valueTypes, c.checkExpression(v))
		}
	}

	// 2. Resolve LHS Type (if explicit)
	var expected Type = nil
	if s.Type != nil {
		expected = c.resolveType(s.Type)
	}

	// 3. Validation & Definition
	if len(s.Names) != len(valueTypes) && len(s.Values) > 0 {
		c.addError(s.Token.Line, s.Token.Column, "Assignment count mismatch: %d names but %d values", len(s.Names), len(valueTypes))
	}

	for i, name := range s.Names {
		var finalType Type = UnknownType

		var actual Type = UnknownType
		if i < len(valueTypes) {
			actual = valueTypes[i]
		}

		if expected != nil {
			// Explicit Type
			finalType = expected
			if actual.Kind() != KindUnknown {
				if !actual.AssignableTo(finalType) {
					c.addError(s.Token.Line, s.Token.Column, "Type mismatch for '%s': expected %s, got %s", name.Value, finalType.String(), actual.String())
				}
				// Refine Array Literal type if needed (e.g. var x []Piece = [])
				if at, ok := actual.(*ArrayType); ok && at.Element.Kind() == KindUnknown {
					if et, ok := expected.(*ArrayType); ok {
						c.Types[s.Values[i]] = et // Update literal type to expected type
					}
				}
			}
		} else {
			// Inference
			if actual.Kind() != KindUnknown {
				if c.isUnresolved(actual) {
					c.addError(s.Token.Line, s.Token.Column, "Cannot infer type from empty literal for '%s'", name.Value)
				}
				finalType = actual
			} else {
				if len(s.Values) == 0 {
					c.addError(s.Token.Line, s.Token.Column, "Variable '%s' requires type or value", name.Value)
				}
			}
		}

		// Check for same-scope redeclaration (Local only)
		if c.scope.outer != nil {
			if existingSym, exists := c.scope.variables[name.Value]; exists {
				c.addError(name.Token.Line, name.Token.Column, "Variable '%s' already declared in this scope (type: %s)", name.Value, existingSym.Type.String())
				continue
			}
		}

		// Update scope with refined type
		if warning := c.scope.DefineVariable(name.Value, finalType, s.IsConst, name.Token.Line, name.Token.Column); warning != nil {
			warning.Line = name.Token.Line
			warning.Column = name.Token.Column
			c.Warnings = append(c.Warnings, *warning)
		}
		// Register type for Compiler
		c.Types[name] = finalType
	}
}

func (c *Checker) checkExpression(e parser.Expression) Type {
	// Prevent stack overflow from deeply nested expressions
	c.recursionDepth++
	defer func() { c.recursionDepth-- }()

	if c.recursionDepth > MAX_RECURSION_DEPTH {
		// Get token info if available
		var line, column int
		if e != nil && e.TokenLiteral() != "" {
			// Try to extract position info - fallback to 0,0 if not available
			line = 0
			column = 0
		}
		c.addError(line, column, "maximum expression nesting depth (%d) exceeded - possible stack overflow", MAX_RECURSION_DEPTH)
		return UnknownType
	}

	t := c.checkExpressionInternal(e)
	c.Types[e] = t
	return t
}

func (c *Checker) checkExpressionInternal(e parser.Expression) Type {
	switch exp := e.(type) {
	case *parser.IntegerLiteral:
		return IntType
	case *parser.CharLiteral:
		return IntType
	case *parser.FloatLiteral:
		return FloatType
	case *parser.StringLiteral:
		return StringType
	case *parser.InterpolatedString:
		for _, part := range exp.Parts {
			t := c.checkExpression(part)
			if t.Kind() != KindString && t.Kind() != KindUnknown {
				c.addError(exp.Token.Line, exp.Token.Column, "Interpolation requires String type, got %s", t.String())
			}
		}
		return StringType
	case *parser.BooleanLiteral:
		return BoolType
	case *parser.NullLiteral:
		return NullType
	case *parser.Identifier:
		if t, ok := c.scope.LookupVariable(exp.Value); ok {
			c.scope.MarkUsed(exp.Value)
			return t
		}
		// Also lookup types (e.g. Struct Name for literal)
		if t, ok := c.scope.LookupType(exp.Value); ok {
			return t
		}
		c.addError(exp.Token.Line, exp.Token.Column, "Undefined variable or type: %s", exp.Value)
		return UnknownType
	case *parser.StructLiteral:
		nameType := c.checkExpression(exp.Name)
		if nameType.Kind() != KindStruct {
			c.addError(exp.Token.Line, exp.Token.Column, "Expected struct type, got %s", nameType.String())
			return ErrorType
		}
		st := nameType.(*StructType)

		for key, valExpr := range exp.Fields {
			expectedType, ok := st.Fields[key]
			if !ok {
				c.addError(exp.Token.Line, exp.Token.Column, "Field '%s' not defined in struct '%s'", key, st.Name)
				continue
			}
			valType := c.checkExpression(valExpr)
			if !valType.AssignableTo(expectedType) {
				c.addError(exp.Token.Line, exp.Token.Column, "Field '%s' type mismatch: expected %s, got %s", key, expectedType.String(), valType.String())
			}
		}
		if len(exp.Fields) < len(st.Fields) {
			c.addError(exp.Token.Line, exp.Token.Column, "Missing fields in struct literal")
		}
		return st

	case *parser.MemberExpression:
		objType := c.checkExpression(exp.Object)

		if objType.Kind() == KindUnknown {
			return UnknownType
		}

		if memberType, ok := objType.GetMember(exp.Member.Value); ok {
			return memberType
		}

		if objType.Kind() == KindModule {
			mod := objType.(*ModuleType)
			c.addError(exp.Token.Line, exp.Token.Column, "Module '%s' does not export '%s'", mod.Name, exp.Member.Value)
		} else if objType.Kind() == KindStruct {
			st := objType.(*StructType)
			c.addError(exp.Token.Line, exp.Token.Column, "Field or method '%s' not found in struct '%s'", exp.Member.Value, st.Name)
		} else {
			c.addError(exp.Token.Line, exp.Token.Column, "Cannot access member on non-struct/module type %s", objType.String())
		}
		return ErrorType

	case *parser.ArrayLiteral:
		if len(exp.Elements) == 0 {
			// Empty array - need context or explicit type
			return &ArrayType{Element: UnknownType}
		}

		// Check first element
		firstType := c.checkExpression(exp.Elements[0])

		// Verify all elements match
		for i, elem := range exp.Elements[1:] {
			elemType := c.checkExpression(elem)
			if !elemType.AssignableTo(firstType) {
				c.addError(exp.Token.Line, exp.Token.Column, "Array element %d type mismatch: expected %s, got %s", i+1, firstType.String(), elemType.String())
			}
		}

		return &ArrayType{Element: firstType}

	case *parser.HashLiteral:
		if len(exp.Pairs) == 0 {
			return &MapType{Key: UnknownType, Value: UnknownType}
		}

		// Infer from first pair
		var keyType, valType Type
		for k, v := range exp.Pairs {
			kt := c.checkExpression(k)
			vt := c.checkExpression(v)

			if keyType == nil {
				keyType = kt
				valType = vt
				continue
			}

			if !kt.AssignableTo(keyType) {
				c.addError(exp.Token.Line, exp.Token.Column, "Map key type mismatch: expected %s, got %s", keyType.String(), kt.String())
			}

			if !vt.AssignableTo(valType) {
				c.addError(exp.Token.Line, exp.Token.Column, "Map value type mismatch: expected %s, got %s", valType.String(), vt.String())
			}
		}

		return &MapType{Key: keyType, Value: valType}

	case *parser.IndexExpression:
		leftType := c.checkExpression(exp.Left)
		idxType := c.checkExpression(exp.Index)

		if leftType.Kind() == KindUnknown {
			return UnknownType
		}

		resType, err := leftType.Index(idxType)
		if err != nil {
			c.addError(exp.Token.Line, exp.Token.Column, "%s", err.Error())
			if resType != nil {
				return resType
			}
			return UnknownType
		}
		return resType

	case *parser.PrefixExpression:
		// Address Of Operation (&)
		if exp.Operator == "&" {
			// Validate operand is addressable
			// Addressable: Identifier, Index, Member.
			// NOT Addressable: Literal, Call, Prefix, Infix.
			isAddressable := false
			switch exp.Right.(type) {
			case *parser.Identifier, *parser.IndexExpression, *parser.MemberExpression:
				isAddressable = true
			}

			if !isAddressable {
				c.addError(exp.Token.Line, exp.Token.Column, "Cannot take address of non-variable expression")
				return ErrorType
			}

			rightType := c.checkExpression(exp.Right)
			if rightType.Kind() == KindUnknown {
				return UnknownType
			}
			return &PointerType{Element: rightType}
		}

		right := c.checkExpression(exp.Right)
		if right.Kind() == KindUnknown {
			return UnknownType
		}
		resType, err := right.PrefixOp(exp.Operator)
		if err != nil {
			c.addError(exp.Token.Line, exp.Token.Column, "%s", err.Error())
			return UnknownType
		}
		return resType

	case *parser.InfixExpression:
		left := c.checkExpression(exp.Left)
		right := c.checkExpression(exp.Right)

		if left.Kind() == KindUnknown || right.Kind() == KindUnknown {
			return UnknownType
		}

		resType, err := left.BinaryOp(exp.Operator, right)
		if err != nil {
			c.addError(exp.Token.Line, exp.Token.Column, "%s", err.Error())
			return ErrorType
		}
		return resType

	case *parser.FunctionLiteral:
		c.enterScope()

		// Define Receiver in scope if present
		if exp.Receiver != nil {
			recvType := c.resolveType(exp.Receiver.Type)
			if warning := c.scope.DefineVariable(exp.Receiver.Name.Value, recvType, false, exp.Receiver.Name.Token.Line, exp.Receiver.Name.Token.Column); warning != nil {
				warning.Line = exp.Receiver.Name.Token.Line
				warning.Column = exp.Receiver.Name.Token.Column
				c.Warnings = append(c.Warnings, *warning)
			}
		}

		for _, p := range exp.Parameters {
			t := c.resolveType(p.Type)
			if warning := c.scope.DefineVariable(p.Name.Value, t, false, p.Name.Token.Line, p.Name.Token.Column); warning != nil {
				warning.Line = p.Name.Token.Line
				warning.Column = p.Name.Token.Column
				c.Warnings = append(c.Warnings, *warning)
			}
		}

		returnTypes := []Type{}
		for _, rt := range exp.ReturnTypes {
			returnTypes = append(returnTypes, c.resolveType(rt))
		}
		if len(returnTypes) == 0 {
			returnTypes = append(returnTypes, VoidType)
		}

		var retStack Type
		if len(returnTypes) == 1 {
			retStack = returnTypes[0]
		} else {
			retStack = &MultiType{Types: returnTypes}
		}

		if !exp.IsNative {
			c.pushReturn(retStack)
			c.checkNodes(exp.Body.Statements)

			// Check if all paths return (if function expects return)
			if len(returnTypes) > 0 && returnTypes[0].Kind() != KindVoid {
				if !c.allPathsReturn(exp.Body) {
					c.addError(exp.Token.Line, exp.Token.Column, "Not all code paths return a value")
				}
			}

			c.popReturn()
		}

		c.leaveScope()

		paramTypes := []Type{}
		for _, p := range exp.Parameters {
			paramTypes = append(paramTypes, c.resolveType(p.Type))
		}
		return &FunctionType{Parameters: paramTypes, ReturnTypes: returnTypes}

	case *parser.IfExpression:
		condType := c.checkExpression(exp.Condition)
		if condType.Kind() != KindBool && condType.Kind() != KindUnknown {
			c.addError(exp.Token.Line, exp.Token.Column, "If condition must be Bool, got %s", condType.String())
		}
		c.checkStatement(exp.Consequence)
		for _, clause := range exp.ElseIfs {
			clauseType := c.checkExpression(clause.Condition)
			if clauseType.Kind() != KindBool && clauseType.Kind() != KindUnknown {
				c.addError(clause.Token.Line, clause.Token.Column, "If condition must be Bool, got %s", clauseType.String())
			}
			c.checkStatement(clause.Consequence)
		}
		if exp.Alternative != nil {
			c.checkStatement(exp.Alternative)
		}
		return VoidType

	case *parser.WhileExpression:
		condType := c.checkExpression(exp.Condition)
		if condType.Kind() != KindBool && condType.Kind() != KindUnknown {
			c.addError(exp.Token.Line, exp.Token.Column, "While condition must be Bool, got %s", condType.String())
		}
		c.checkStatement(exp.Body)
		return VoidType

	case *parser.CallExpression:
		if ident, ok := exp.Function.(*parser.Identifier); ok {
			if ident.Value == "luncurkan" {
				return c.checkSpawn(exp)
			}
			if ident.Value == "hapus" {
				return c.checkDelete(exp)
			}
			if ident.Value == "panjang" {
				return c.checkLen(exp)
			}
			if ident.Value == "assert" {
				return c.checkAssert(exp)
			}
		}

		funcType := c.checkExpression(exp.Function)
		if funcType.Kind() == KindUnknown {
			return UnknownType
		}

		argTypes := []Type{}
		for _, arg := range exp.Arguments {
			argTypes = append(argTypes, c.checkExpression(arg))
		}

		resType, warning, err := funcType.Call(argTypes)
		if warning != "" {
			c.addWarning(exp.Token.Line, exp.Token.Column, "%s", warning)
		}
		if err != nil {
			c.addError(exp.Token.Line, exp.Token.Column, "%s", err.Error())
			return ErrorType
		}
		return resType
	}

	return UnknownType
}

func (c *Checker) checkSpawn(call *parser.CallExpression) Type {
	if len(call.Arguments) < 1 || len(call.Arguments) > 2 {
		c.addError(call.Token.Line, call.Token.Column, "luncurkan expects 1 or 2 arguments")
		return ErrorType
	}

	// Arg 1: Function
	fnType := c.checkExpression(call.Arguments[0])
	if fnType.Kind() != KindFunction {
		c.addError(call.Token.Line, call.Token.Column, "Argument 1 must be a function, got %s", fnType.String())
		return ErrorType
	}
	ft := fnType.(*FunctionType)

	// Arg 2: Argument (Optional)
	if len(call.Arguments) == 2 {
		argType := c.checkExpression(call.Arguments[1])

		// Match with function param
		if len(ft.Parameters) != 1 {
			c.addError(call.Token.Line, call.Token.Column, "Function passed to luncurkan must accept exactly 1 argument")
			return ErrorType
		}
		if !argType.AssignableTo(ft.Parameters[0]) {
			c.addError(call.Token.Line, call.Token.Column, "Argument type mismatch: expected %s, got %s", ft.Parameters[0].String(), argType.String())
		}
	} else {
		// 0 args passed
		if len(ft.Parameters) != 0 {
			c.addError(call.Token.Line, call.Token.Column, "Function requires arguments")
		}
	}

	return VoidType
}

func (c *Checker) checkDelete(call *parser.CallExpression) Type {
	if len(call.Arguments) != 2 {
		c.addError(call.Token.Line, call.Token.Column, "hapus expects 2 arguments: map and key")
		return VoidType
	}

	mapType := c.checkExpression(call.Arguments[0])
	keyType := c.checkExpression(call.Arguments[1])

	if mapType.Kind() != KindMap {
		c.addError(call.Token.Line, call.Token.Column, "Argument 1 must be a map, got %s", mapType.String())
		return VoidType
	}

	mt := mapType.(*MapType)
	if !keyType.AssignableTo(mt.Key) {
		c.addError(call.Token.Line, call.Token.Column, "Key type mismatch: expected %s, got %s", mt.Key.String(), keyType.String())
	}

	return VoidType
}

func (c *Checker) checkLen(call *parser.CallExpression) Type {
	if len(call.Arguments) != 1 {
		c.addError(call.Token.Line, call.Token.Column, "panjang expects 1 argument")
		return IntType
	}

	argType := c.checkExpression(call.Arguments[0])
	if argType.Kind() != KindArray && argType.Kind() != KindMap && argType.Kind() != KindString {
		// Allow checking length of unknown type (runtime check?)
		// No, strict type checking.
		c.addError(call.Token.Line, call.Token.Column, "Argument must be Array, Map, or String, got %s", argType.String())
	}

	return IntType
}

func (c *Checker) checkAssert(call *parser.CallExpression) Type {
	if len(call.Arguments) != 2 {
		c.addError(call.Token.Line, call.Token.Column, "assert expects 2 arguments: interface and type")
		return ErrorType
	}

	ifaceType := c.checkExpression(call.Arguments[0])
	if ifaceType.Kind() != KindInterface {
		c.addError(call.Token.Line, call.Token.Column, "Argument 1 must be an interface, got %s", ifaceType.String())
	}

	// Argument 2 must be a Type
	// checkExpression returns Type if Identifier is a Type
	targetType := c.checkExpression(call.Arguments[1])
	if targetType.Kind() != KindStruct {
		c.addError(call.Token.Line, call.Token.Column, "Argument 2 must be a struct type for assertion")
	}

	return targetType
}

func (c *Checker) enterScope() {
	c.scope = NewScope(c.scope)
}

func (c *Checker) leaveScope() {
	if warnings := c.scope.CheckUnused(); len(warnings) > 0 {
		c.Warnings = append(c.Warnings, warnings...)
	}
	if c.scope.outer != nil {
		c.scope = c.scope.outer
	}
}

func (c *Checker) addError(line, col int, format string, args ...interface{}) {
	err := NewTypeError(line, col, format, args...)
	// Deduplicate
	if len(c.Errors) > 0 {
		last := c.Errors[len(c.Errors)-1]
		if last.Line == err.Line && last.Column == err.Column && last.Message == err.Message {
			return
		}
	}
	c.Errors = append(c.Errors, err)
}

func (c *Checker) addWarning(line, col int, format string, args ...interface{}) {
	c.Warnings = append(c.Warnings, NewTypeWarning(line, col, format, args...))
}

func (c *Checker) checkStructCycles(program *parser.Program) {
	visited := make(map[string]bool)
	visiting := make(map[string]bool)

	var check func(name string) bool
	check = func(name string) bool {
		if visiting[name] {
			return true
		}
		if visited[name] {
			return false
		}

		visiting[name] = true
		defer func() { visiting[name] = false }()

		t, ok := c.scope.LookupType(name)
		if !ok {
			return false
		}
		st, ok := t.(*StructType)
		if !ok {
			return false
		}

		for _, fieldType := range st.Fields {
			if depName := c.getStructName(fieldType); depName != "" {
				if check(depName) {
					return true
				}
			}
		}

		visited[name] = true
		return false
	}

	for _, stmt := range program.Statements {
		if s, ok := stmt.(*parser.StructStatement); ok {
			if check(s.Name.Value) {
				c.addError(s.Name.Token.Line, s.Name.Token.Column, "Cyclic struct dependency detected involving '%s'", s.Name.Value)
			}
		}
	}
}

func (c *Checker) getStructName(t Type) string {
	if st, ok := t.(*StructType); ok {
		return st.Name
	}
	return ""
}

func (c *Checker) isReturn(s parser.Statement) bool {
	_, ok := s.(*parser.ReturnStatement)
	return ok
}

func (c *Checker) getStatementLineCol(s parser.Statement) (int, int) {
	switch stmt := s.(type) {
	case *parser.VarStatement:
		return stmt.Token.Line, stmt.Token.Column
	case *parser.ReturnStatement:
		return stmt.Token.Line, stmt.Token.Column
	case *parser.ExpressionStatement:
		return stmt.Token.Line, stmt.Token.Column
	case *parser.AssignmentStatement:
		return stmt.Token.Line, stmt.Token.Column
	case *parser.BlockStatement:
		return stmt.Token.Line, stmt.Token.Column
	case *parser.StructStatement:
		return stmt.Token.Line, stmt.Token.Column
	}
	return 0, 0
}

func (c *Checker) isUnresolved(t Type) bool {
	switch T := t.(type) {
	case *ArrayType:
		return T.Element.Kind() == KindUnknown || c.isUnresolved(T.Element)
	case *MapType:
		return T.Key.Kind() == KindUnknown || T.Value.Kind() == KindUnknown || c.isUnresolved(T.Key) || c.isUnresolved(T.Value)
	}
	return false
}
