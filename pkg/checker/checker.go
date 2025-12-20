package checker

import (
	"github.com/VzoelFox/morphlang/pkg/parser"
)

type Checker struct {
	Errors      []TypeError
	Warnings    []TypeWarning
	scope       *Scope
	returnStack []Type
}

func New() *Checker {
	c := &Checker{
		Errors:      []TypeError{},
		Warnings:    []TypeWarning{},
		scope:       NewScope(nil),
		returnStack: []Type{},
	}

	// Register primitive types
	c.scope.DefineType("Int", IntType)
	c.scope.DefineType("Float", FloatType)
	c.scope.DefineType("String", StringType)
	c.scope.DefineType("Bool", BoolType)

	// Register built-in functions (Stdlib Prototype)
	// parse_int(String) -> (Int, String)
	parseIntType := &FunctionType{
		Parameters:  []Type{StringType},
		ReturnTypes: []Type{IntType, StringType},
	}
	c.scope.DefineVariable("parse_int", parseIntType, 0, 0)

	return c
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
	c.checkStructCycles(program)

	// Pass 2: Check Bodies
	c.checkNodes(program.Statements)
}

func (c *Checker) collectDefinitions(program *parser.Program) {
	// 1. Structs first (types)
	for _, stmt := range program.Statements {
		if s, ok := stmt.(*parser.StructStatement); ok {
			c.defineStruct(s)
		}
	}

	// 2. Global Functions and Vars
	for _, stmt := range program.Statements {
		switch s := stmt.(type) {
		case *parser.VarStatement:
			c.defineVar(s)
		case *parser.ExpressionStatement:
			if fn, ok := s.Expression.(*parser.FunctionLiteral); ok {
				if fn.Name != "" {
					c.defineFunction(fn)
				}
			}
		}
	}
}

func (c *Checker) defineStruct(s *parser.StructStatement) {
	st := &StructType{
		Name:    s.Name.Value,
		Fields:  make(map[string]Type),
		Methods: make(map[string]*FunctionType),
	}
	c.scope.DefineType(s.Name.Value, st)
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
			c.addError(name.Token.Line, name.Token.Column, "Global variable '%s' already declared (type: %s)", name.Value, existing.Type.String())
			continue
		}
		c.scope.DefineVariable(name.Value, t, name.Token.Line, name.Token.Column)
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
		c.scope.DefineVariable(fn.Name, ft, fn.Token.Line, fn.Token.Column)
	}
}

func (c *Checker) resolveType(n parser.TypeNode) Type {
	switch t := n.(type) {
	case *parser.SimpleType:
		switch t.Name {
		case "Int":
			return IntType
		case "Float":
			return FloatType
		case "String":
			return StringType
		case "Bool":
			return BoolType
		case "Void":
			return VoidType
		default:
			if typ, ok := c.scope.LookupType(t.Name); ok {
				return typ
			}
			c.addError(t.Token.Line, t.Token.Column, "Unknown type: %s", t.Name)
			return UnknownType
		}
	case *parser.ArrayType:
		elem := c.resolveType(t.Element)
		return &ArrayType{Element: elem}
	case *parser.MapType:
		key := c.resolveType(t.Key)
		val := c.resolveType(t.Value)
		return &MapType{Key: key, Value: val}
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
		return c.allPathsReturn(n.Consequence) &&
			c.allPathsReturn(n.Alternative)

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
			if !expected.Equals(actual) && actual.Kind() != KindUnknown {
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
	}
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

		lhsType := c.checkExpression(nameExpr)
		rhsType := valueTypes[i]

		if lhsType.Kind() != KindUnknown && rhsType.Kind() != KindUnknown {
			if !lhsType.Equals(rhsType) {
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
				if !finalType.Equals(actual) {
					c.addError(s.Token.Line, s.Token.Column, "Type mismatch for '%s': expected %s, got %s", name.Value, finalType.String(), actual.String())
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
		if warning := c.scope.DefineVariable(name.Value, finalType, name.Token.Line, name.Token.Column); warning != nil {
			warning.Line = name.Token.Line
			warning.Column = name.Token.Column
			c.Warnings = append(c.Warnings, *warning)
		}
	}
}

func (c *Checker) checkExpression(e parser.Expression) Type {
	switch exp := e.(type) {
	case *parser.IntegerLiteral:
		return IntType
	case *parser.FloatLiteral:
		return FloatType
	case *parser.StringLiteral:
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
			if !expectedType.Equals(valType) {
				c.addError(exp.Token.Line, exp.Token.Column, "Field '%s' type mismatch: expected %s, got %s", key, expectedType.String(), valType.String())
			}
		}
		if len(exp.Fields) < len(st.Fields) {
			c.addError(exp.Token.Line, exp.Token.Column, "Missing fields in struct literal")
		}
		return st

	case *parser.MemberExpression:
		objType := c.checkExpression(exp.Object)

		if objType.Kind() != KindStruct {
			c.addError(exp.Token.Line, exp.Token.Column, "Cannot access member on non-struct type %s", objType.String())
			return ErrorType
		}

		st := objType.(*StructType)
		if fieldType, exists := st.Fields[exp.Member.Value]; exists {
			return fieldType
		}

		if methodType, exists := st.Methods[exp.Member.Value]; exists {
			return methodType
		}

		c.addError(exp.Token.Line, exp.Token.Column, "Field or method '%s' not found in struct '%s'", exp.Member.Value, st.Name)
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
			if !firstType.Equals(elemType) {
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

			if !keyType.Equals(kt) {
				c.addError(exp.Token.Line, exp.Token.Column, "Map key type mismatch: expected %s, got %s", keyType.String(), kt.String())
			}

			if !valType.Equals(vt) {
				c.addError(exp.Token.Line, exp.Token.Column, "Map value type mismatch: expected %s, got %s", valType.String(), vt.String())
			}
		}

		return &MapType{Key: keyType, Value: valType}

	case *parser.IndexExpression:
		leftType := c.checkExpression(exp.Left)
		idxType := c.checkExpression(exp.Index)

		if leftType.Kind() == KindArray {
			if idxType.Kind() != KindInt {
				c.addError(exp.Token.Line, exp.Token.Column, "Array index must be Int")
			}
			return leftType.(*ArrayType).Element
		}

		if leftType.Kind() == KindMap {
			mt := leftType.(*MapType)
			if !mt.Key.Equals(idxType) {
				c.addError(exp.Token.Line, exp.Token.Column, "Map key type mismatch: expected %s, got %s", mt.Key.String(), idxType.String())
			}
			return mt.Value
		}

		if leftType.Kind() != KindUnknown {
			c.addError(exp.Token.Line, exp.Token.Column, "Index operation not supported on type %s", leftType.String())
		}
		return UnknownType

	case *parser.PrefixExpression:
		right := c.checkExpression(exp.Right)
		if exp.Operator == "!" {
			if right.Kind() != KindBool && right.Kind() != KindUnknown {
				c.addError(exp.Token.Line, exp.Token.Column, "Operator ! not defined for type %s", right.String())
			}
			return BoolType
		}
		if exp.Operator == "-" {
			if right.Kind() != KindInt && right.Kind() != KindFloat && right.Kind() != KindUnknown {
				c.addError(exp.Token.Line, exp.Token.Column, "Operator - not defined for type %s", right.String())
			}
			return right
		}
		return UnknownType

	case *parser.InfixExpression:
		left := c.checkExpression(exp.Left)
		right := c.checkExpression(exp.Right)

		if left.Kind() == KindUnknown || right.Kind() == KindUnknown {
			return UnknownType
		}

		switch exp.Operator {
		case "+", "-", "*", "/":
			if left.Kind() == KindInt && right.Kind() == KindInt {
				return IntType
			}
			if left.Kind() == KindFloat && right.Kind() == KindFloat {
				return FloatType
			}
			c.addError(exp.Token.Line, exp.Token.Column, "Operator %s not defined for types %s and %s", exp.Operator, left.String(), right.String())
			return ErrorType
		case "==", "!=":
			if left.Equals(right) {
				return BoolType
			}
			c.addError(exp.Token.Line, exp.Token.Column, "Cannot compare different types %s and %s", left.String(), right.String())
			return ErrorType
		}

	case *parser.FunctionLiteral:
		c.enterScope()

		// Define Receiver in scope if present
		if exp.Receiver != nil {
			recvType := c.resolveType(exp.Receiver.Type)
			if warning := c.scope.DefineVariable(exp.Receiver.Name.Value, recvType, exp.Receiver.Name.Token.Line, exp.Receiver.Name.Token.Column); warning != nil {
				warning.Line = exp.Receiver.Name.Token.Line
				warning.Column = exp.Receiver.Name.Token.Column
				c.Warnings = append(c.Warnings, *warning)
			}
		}

		for _, p := range exp.Parameters {
			t := c.resolveType(p.Type)
			if warning := c.scope.DefineVariable(p.Name.Value, t, p.Name.Token.Line, p.Name.Token.Column); warning != nil {
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

		c.pushReturn(retStack)
		c.checkNodes(exp.Body.Statements)

		// Check if all paths return (if function expects return)
		if len(returnTypes) > 0 && returnTypes[0].Kind() != KindVoid {
			if !c.allPathsReturn(exp.Body) {
				c.addError(exp.Token.Line, exp.Token.Column, "Not all code paths return a value")
			}
		}

		c.popReturn()

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
		funcType := c.checkExpression(exp.Function)
		if funcType.Kind() == KindUnknown {
			return UnknownType
		}
		if f, ok := funcType.(*FunctionType); ok {
			if len(exp.Arguments) != len(f.Parameters) {
				c.addError(exp.Token.Line, exp.Token.Column, "Wrong number of arguments: expected %d, got %d", len(f.Parameters), len(exp.Arguments))
				return ErrorType
			}
			for i, arg := range exp.Arguments {
				argType := c.checkExpression(arg)
				if !argType.Equals(f.Parameters[i]) {
					c.addError(exp.Token.Line, exp.Token.Column, "Argument %d type mismatch: expected %s, got %s", i+1, f.Parameters[i].String(), argType.String())
				}
			}
			if len(f.ReturnTypes) == 1 {
				return f.ReturnTypes[0]
			}
			return &MultiType{Types: f.ReturnTypes}
		}

		// Check for Type Casting (e.g. Int(5.5))
		if id, ok := exp.Function.(*parser.Identifier); ok {
			// If it resolves to a Type and NOT a Variable, it's a Cast
			if targetType, isType := c.scope.LookupType(id.Value); isType {
				if _, isVar := c.scope.LookupVariable(id.Value); !isVar {
					// It is a cast
					if len(exp.Arguments) != 1 {
						c.addError(exp.Token.Line, exp.Token.Column, "Type conversion requires exactly 1 argument")
						return ErrorType
					}
					argType := c.checkExpression(exp.Arguments[0])

					// Tier 1: Int -> Float (Lossless-ish)
					if targetType.Kind() == KindFloat && argType.Kind() == KindInt {
						return FloatType
					}

					// Tier 2: Float -> Int (Lossy - Warning)
					if targetType.Kind() == KindInt && argType.Kind() == KindFloat {
						c.addWarning(exp.Token.Line, exp.Token.Column, "Lossy conversion from Float to Int")
						return IntType
					}

					// Identity
					if targetType.Equals(argType) {
						return targetType
					}

					// Tier 3: Forbidden (String -> Int, etc)
					c.addError(exp.Token.Line, exp.Token.Column, "Cannot convert type %s to %s", argType.String(), targetType.String())
					return ErrorType
				}
			}
		}

		c.addError(exp.Token.Line, exp.Token.Column, "Not a function: %s", funcType.String())
		return ErrorType
	}

	return UnknownType
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
	c.Errors = append(c.Errors, NewTypeError(line, col, format, args...))
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
