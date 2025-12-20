package checker

import (
	"github.com/VzoelFox/morphlang/pkg/parser"
)

type Checker struct {
	Errors             []TypeError
	scope              *Scope
	expectedReturnType Type
}

func New() *Checker {
	c := &Checker{
		Errors:             []TypeError{},
		scope:              NewScope(nil),
		expectedReturnType: nil,
	}
	// Register built-ins if any
	return c
}

func (c *Checker) Check(program *parser.Program) {
	// Pass 1: Collect Definitions (Structs, Functions, Vars)
	c.collectDefinitions(program)
	c.resolveStructFields(program)

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
	st := &StructType{Name: s.Name.Value, Fields: make(map[string]Type)}
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
	t := c.resolveType(s.Type)
	c.scope.DefineVariable(s.Name.Value, t)
}

func (c *Checker) defineFunction(fn *parser.FunctionLiteral) {
	paramTypes := []Type{}
	for _, p := range fn.Parameters {
		paramTypes = append(paramTypes, c.resolveType(p.Type))
	}

	var returnType Type = VoidType
	if fn.ReturnType != nil {
		returnType = c.resolveType(fn.ReturnType)
	}

	ft := &FunctionType{Parameters: paramTypes, ReturnType: returnType}
	c.scope.DefineVariable(fn.Name, ft)
}

func (c *Checker) resolveType(n parser.TypeNode) Type {
	switch t := n.(type) {
	case *parser.SimpleType:
		switch t.Name {
		case "Int":
			return IntType
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
	for _, s := range stmts {
		c.checkStatement(s)
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
		if stmt.ReturnValue != nil {
			actual = c.checkExpression(stmt.ReturnValue)
		}
		if c.expectedReturnType != nil {
			if !c.expectedReturnType.Equals(actual) && actual.Kind() != KindUnknown {
				c.addError(stmt.Token.Line, stmt.Token.Column, "Return type mismatch: expected %s, got %s", c.expectedReturnType.String(), actual.String())
			}
		}
	case *parser.StructStatement:
		// Fields resolved in pass 1.5
	case *parser.BlockStatement:
		c.enterScope()
		defer c.leaveScope()
		c.checkNodes(stmt.Statements)
	}
}

func (c *Checker) checkVarStatement(s *parser.VarStatement) {
	expected := c.resolveType(s.Type)
	if s.Value != nil {
		actual := c.checkExpression(s.Value)
		if !expected.Equals(actual) && actual.Kind() != KindUnknown && expected.Kind() != KindUnknown {
			c.addError(s.Token.Line, s.Token.Column, "Type mismatch in variable declaration '%s': expected %s, got %s", s.Name.Value, expected.String(), actual.String())
		}
	}
}

func (c *Checker) checkExpression(e parser.Expression) Type {
	switch exp := e.(type) {
	case *parser.IntegerLiteral:
		return IntType
	case *parser.StringLiteral:
		return StringType
	case *parser.BooleanLiteral:
		return BoolType
	case *parser.NullLiteral:
		return NullType
	case *parser.Identifier:
		if t, ok := c.scope.LookupVariable(exp.Value); ok {
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

	case *parser.IndexExpression:
		leftType := c.checkExpression(exp.Left)

		if leftType.Kind() == KindStruct {
			if strLit, ok := exp.Index.(*parser.StringLiteral); ok {
				fieldName := strLit.Value
				if fieldType, ok := leftType.(*StructType).Fields[fieldName]; ok {
					return fieldType
				}
				c.addError(exp.Token.Line, exp.Token.Column, "Field '%s' not found in struct '%s'", fieldName, leftType.String())
				return ErrorType
			}
		}

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

		if leftType.Kind() != KindStruct && leftType.Kind() != KindUnknown {
			c.addError(exp.Token.Line, exp.Token.Column, "Index operation not supported on type %s", leftType.String())
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

		for _, p := range exp.Parameters {
			t := c.resolveType(p.Type)
			c.scope.DefineVariable(p.Name.Value, t)
		}

		var retType Type = VoidType
		if exp.ReturnType != nil {
			retType = c.resolveType(exp.ReturnType)
		}

		prevRet := c.expectedReturnType
		c.expectedReturnType = retType

		c.checkNodes(exp.Body.Statements)

		c.expectedReturnType = prevRet
		c.leaveScope()

		paramTypes := []Type{}
		for _, p := range exp.Parameters {
			paramTypes = append(paramTypes, c.resolveType(p.Type))
		}
		return &FunctionType{Parameters: paramTypes, ReturnType: retType}

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
			return f.ReturnType
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
	if c.scope.outer != nil {
		c.scope = c.scope.outer
	}
}

func (c *Checker) addError(line, col int, format string, args ...interface{}) {
	c.Errors = append(c.Errors, NewTypeError(line, col, format, args...))
}
