package evaluator

import (
	"fmt"

	"github.com/VzoelFox/morph/pkg/parser"
)

var (
	TRUE  = &Boolean{Value: true}
	FALSE = &Boolean{Value: false}
	NULL  = &Null{}
)

type Importer interface {
	Import(name string) (*Module, error)
}

type Evaluator struct {
	importer       Importer
	recursionDepth int // Track recursion depth to prevent stack overflow
}

// Maximum recursion depth to prevent stack overflow from deeply nested expressions
const MAX_RECURSION_DEPTH = 1000

func New(i Importer) *Evaluator {
	return &Evaluator{importer: i}
}

func (e *Evaluator) SetImporter(i Importer) {
	e.importer = i
}

func (e *Evaluator) Eval(node parser.Node, env *Environment) Object {
	// Prevent stack overflow from deeply nested expressions
	e.recursionDepth++
	defer func() { e.recursionDepth-- }()

	if e.recursionDepth > MAX_RECURSION_DEPTH {
		return newError("maximum expression nesting depth (%d) exceeded - possible stack overflow", MAX_RECURSION_DEPTH)
	}

	switch node := node.(type) {
	case *parser.Program:
		return e.evalProgram(node, env)

	case *parser.ExpressionStatement:
		return e.Eval(node.Expression, env)

	case *parser.BlockStatement:
		return e.evalBlockStatement(node, env)

	case *parser.ReturnStatement:
		if len(node.ReturnValues) == 1 {
			val := e.Eval(node.ReturnValues[0], env)
			return &ReturnValue{Value: val}
		} else if len(node.ReturnValues) > 1 {
			elements := e.evalExpressions(node.ReturnValues, env)
			if len(elements) == 1 && isError(elements[0]) {
				return elements[0]
			}
			return &ReturnValue{Value: &Tuple{Elements: elements}}
		}
		return &ReturnValue{Value: NULL}

	case *parser.VarStatement:
		return e.evalVarStatement(node, env)

	case *parser.AssignmentStatement:
		return e.evalAssignmentStatement(node, env)

	case *parser.PrefixExpression:
		right := e.Eval(node.Right, env)
		if isError(right) {
			return right
		}
		return e.evalPrefixExpression(node.Operator, right)

	case *parser.InfixExpression:
		left := e.Eval(node.Left, env)
		if isError(left) {
			return left
		}
		right := e.Eval(node.Right, env)
		if isError(right) {
			return right
		}
		return e.evalInfixExpression(node.Operator, left, right)

	case *parser.IfExpression:
		return e.evalIfExpression(node, env)

	case *parser.Identifier:
		return e.evalIdentifier(node, env)

	case *parser.IntegerLiteral:
		return &Integer{Value: node.Value}

	case *parser.StringLiteral:
		return &String{Value: node.Value}

	case *parser.BooleanLiteral:
		return nativeBoolToBooleanObject(node.Value)

	case *parser.NullLiteral:
		return NULL

	case *parser.CallExpression:
		function := e.Eval(node.Function, env)
		if isError(function) {
			return function
		}
		args := e.evalExpressions(node.Arguments, env)
		if len(args) == 1 && isError(args[0]) {
			return args[0]
		}
		return e.applyFunction(function, args)

	case *parser.FunctionLiteral:
		if node.IsNative {
			if fn, ok := GetNative(node.Name); ok {
				if node.Name != "" {
					env.Define(node.Name, fn)
				}
				return fn
			}
			return newError("native function implementation not found: %s", node.Name)
		}
		fn := &Function{Parameters: node.Parameters, Body: node.Body, Env: env}
		if node.Name != "" {
			env.Define(node.Name, fn)
		}
		return fn

	case *parser.ImportStatement:
		if e.importer != nil {
			mod, err := e.importer.Import(node.Path)
			if err != nil {
				return newError("Import error: %s", err.Error())
			}
			name := node.Path // todo: basename
			env.Define(name, mod)
			return NULL
		}
		return newError("Importer not configured")

	case *parser.MemberExpression:
		obj := e.Eval(node.Object, env)
		if isError(obj) {
			return obj
		}

		if mod, ok := obj.(*Module); ok {
			if val, ok := mod.Env.Get(node.Member.Value); ok {
				return val
			}
			return newError("Member '%s' not found in module '%s'", node.Member.Value, mod.Name)
		}
		if str, ok := obj.(*StructInstance); ok {
			if val, ok := str.Fields[node.Member.Value]; ok {
				return val
			}
			return newError("Field '%s' not found in struct", node.Member.Value)
		}
		return newError("Not a module or struct: %s", obj.Type())
	}

	return nil
}

func (e *Evaluator) evalProgram(program *parser.Program, env *Environment) Object {
	var result Object
	for _, statement := range program.Statements {
		result = e.Eval(statement, env)
		switch result := result.(type) {
		case *ReturnValue:
			return result.Value
		case *Error:
			return result
		}
	}
	return result
}

func (e *Evaluator) evalBlockStatement(block *parser.BlockStatement, env *Environment) Object {
	var result Object
	for _, statement := range block.Statements {
		result = e.Eval(statement, env)
		if result != nil {
			rt := result.Type()
			if rt == RETURN_VALUE_OBJ || rt == ERROR_OBJ {
				return result
			}
		}
	}
	return result
}

func (e *Evaluator) evalIdentifier(node *parser.Identifier, env *Environment) Object {
	if val, ok := env.Get(node.Value); ok {
		return val
	}
	return newError("identifier not found: %s", node.Value)
}

func (e *Evaluator) evalExpressions(exps []parser.Expression, env *Environment) []Object {
	var result []Object
	for _, eNode := range exps {
		evaluated := e.Eval(eNode, env)
		if isError(evaluated) {
			return []Object{evaluated}
		}
		result = append(result, evaluated)
	}
	return result
}

func (e *Evaluator) applyFunction(fn Object, args []Object) Object {
	switch fn := fn.(type) {
	case *Function:
		extendedEnv := NewEnclosedEnvironment(fn.Env)
		for i, param := range fn.Parameters {
			extendedEnv.Define(param.Name.Value, args[i])
		}
		evaluated := e.Eval(fn.Body, extendedEnv)
		return unwrapReturnValue(evaluated)
	case *NativeFunction:
		return fn.Fn(args...)
	default:
		return newError("not a function: %s", fn.Type())
	}
}

func (e *Evaluator) evalVarStatement(node *parser.VarStatement, env *Environment) Object {
	var values []Object
	if len(node.Values) == 1 && len(node.Names) > 1 {
		val := e.Eval(node.Values[0], env)
		if isError(val) {
			return val
		}
		if tuple, ok := val.(*Tuple); ok {
			values = tuple.Elements
		} else {
			return newError("expected tuple for unpacking, got %s", val.Type())
		}
	} else {
		values = e.evalExpressions(node.Values, env)
		if len(values) == 1 && isError(values[0]) {
			return values[0]
		}
	}

	if len(node.Names) != len(values) {
		return newError("mismatch assignment count: %d = %d", len(node.Names), len(values))
	}

	for i, name := range node.Names {
		env.Define(name.Value, values[i])
	}
	return NULL
}

func (e *Evaluator) evalAssignmentStatement(node *parser.AssignmentStatement, env *Environment) Object {
	var values []Object
	if len(node.Values) == 1 && len(node.Names) > 1 {
		val := e.Eval(node.Values[0], env)
		if isError(val) {
			return val
		}
		if tuple, ok := val.(*Tuple); ok {
			values = tuple.Elements
		} else {
			return newError("expected tuple for unpacking, got %s", val.Type())
		}
	} else {
		values = e.evalExpressions(node.Values, env)
		if len(values) == 1 && isError(values[0]) {
			return values[0]
		}
	}

	if len(node.Names) != len(values) {
		return newError("mismatch assignment count: %d = %d", len(node.Names), len(values))
	}

	for i, nameExpr := range node.Names {
		// Assuming identifiers for now
		if ident, ok := nameExpr.(*parser.Identifier); ok {
			env.Assign(ident.Value, values[i])
		} else {
			// Todo: Handle other LHS like IndexExpression, MemberExpression
			return newError("assignment to %T not supported yet", nameExpr)
		}
	}
	return NULL
}

func (e *Evaluator) evalPrefixExpression(operator string, right Object) Object {
	switch operator {
	case "!":
		return evalBangOperatorExpression(right)
	case "-":
		return evalMinusPrefixOperatorExpression(right)
	default:
		return newError("unknown operator: %s%s", operator, right.Type())
	}
}

func (e *Evaluator) evalInfixExpression(operator string, left, right Object) Object {
	switch {
	case left.Type() == INTEGER_OBJ && right.Type() == INTEGER_OBJ:
		return evalIntegerInfixExpression(operator, left, right)
	case left.Type() == STRING_OBJ && right.Type() == STRING_OBJ:
		return evalStringInfixExpression(operator, left, right)
	case operator == "==":
		return nativeBoolToBooleanObject(left == right) // Pointer comparison works for Booleans/Null
	case operator == "!=":
		return nativeBoolToBooleanObject(left != right)
	case left.Type() != right.Type():
		return newError("type mismatch: %s %s %s", left.Type(), operator, right.Type())
	default:
		return newError("unknown operator: %s %s %s", left.Type(), operator, right.Type())
	}
}

func evalIntegerInfixExpression(operator string, left, right Object) Object {
	leftVal := left.(*Integer).Value
	rightVal := right.(*Integer).Value

	switch operator {
	case "+":
		return &Integer{Value: leftVal + rightVal}
	case "-":
		return &Integer{Value: leftVal - rightVal}
	case "*":
		return &Integer{Value: leftVal * rightVal}
	case "/":
		return &Integer{Value: leftVal / rightVal}
	case "<":
		return nativeBoolToBooleanObject(leftVal < rightVal)
	case ">":
		return nativeBoolToBooleanObject(leftVal > rightVal)
	case "==":
		return nativeBoolToBooleanObject(leftVal == rightVal)
	case "!=":
		return nativeBoolToBooleanObject(leftVal != rightVal)
	default:
		return newError("unknown operator: %s %s %s", left.Type(), operator, right.Type())
	}
}

func evalStringInfixExpression(operator string, left, right Object) Object {
	if operator != "+" {
		return newError("unknown operator: %s %s %s", left.Type(), operator, right.Type())
	}
	leftVal := left.(*String).Value
	rightVal := right.(*String).Value
	return &String{Value: leftVal + rightVal}
}

func evalBangOperatorExpression(right Object) Object {
	switch right {
	case TRUE:
		return FALSE
	case FALSE:
		return TRUE
	case NULL:
		return TRUE
	default:
		return FALSE
	}
}

func evalMinusPrefixOperatorExpression(right Object) Object {
	if right.Type() != INTEGER_OBJ {
		return newError("unknown operator: -%s", right.Type())
	}
	value := right.(*Integer).Value
	return &Integer{Value: -value}
}

func (e *Evaluator) evalIfExpression(ie *parser.IfExpression, env *Environment) Object {
	condition := e.Eval(ie.Condition, env)
	if isError(condition) {
		return condition
	}

	if isTruthy(condition) {
		return e.Eval(ie.Consequence, env)
	}
	for _, clause := range ie.ElseIfs {
		cond := e.Eval(clause.Condition, env)
		if isError(cond) {
			return cond
		}
		if isTruthy(cond) {
			return e.Eval(clause.Consequence, env)
		}
	}
	if ie.Alternative != nil {
		return e.Eval(ie.Alternative, env)
	}
	return NULL
}

func isTruthy(obj Object) bool {
	switch obj {
	case NULL:
		return false
	case TRUE:
		return true
	case FALSE:
		return false
	default:
		return true
	}
}

func unwrapReturnValue(obj Object) Object {
	if returnValue, ok := obj.(*ReturnValue); ok {
		return returnValue.Value
	}
	return obj
}

func nativeBoolToBooleanObject(input bool) *Boolean {
	if input {
		return TRUE
	}
	return FALSE
}

func newError(format string, a ...interface{}) *Error {
	return &Error{Message: fmt.Sprintf(format, a...)}
}

func isError(obj Object) bool {
	if obj != nil {
		return obj.Type() == ERROR_OBJ
	}
	return false
}

func GetNative(name string) (*NativeFunction, bool) {
	if fn, ok := NativeRegistry[name]; ok {
		return &NativeFunction{Fn: fn, Name: name}, true
	}
	return nil, false
}
