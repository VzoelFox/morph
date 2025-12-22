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
	importer Importer
}

func New(i Importer) *Evaluator {
	return &Evaluator{importer: i}
}

func (e *Evaluator) SetImporter(i Importer) {
	e.importer = i
}

func (e *Evaluator) Eval(node parser.Node, env *Environment) Object {
	switch node := node.(type) {
	case *parser.Program:
		return e.evalProgram(node, env)

	case *parser.ExpressionStatement:
		return e.Eval(node.Expression, env)

	case *parser.BlockStatement:
		return e.evalBlockStatement(node, env)

	case *parser.ReturnStatement:
		var val Object = NULL
		if len(node.ReturnValues) == 1 {
			val = e.Eval(node.ReturnValues[0], env)
		}
		// Todo: Handle multiple returns
		return &ReturnValue{Value: val}

	case *parser.VarStatement:
		val := e.Eval(node.Values[0], env)
		if isError(val) {
			return val
		}
		env.Define(node.Names[0].Value, val)
		return val

	case *parser.AssignmentStatement:
		val := e.Eval(node.Values[0], env)
		if isError(val) {
			return val
		}
		// Handle member assignment? for now simple ident
		if ident, ok := node.Names[0].(*parser.Identifier); ok {
			env.Assign(ident.Value, val)
		}
		return val

	case *parser.Identifier:
		return e.evalIdentifier(node, env)

	case *parser.IntegerLiteral:
		return &Integer{Value: node.Value}

	case *parser.StringLiteral:
		return &String{Value: node.Value}

	case *parser.BooleanLiteral:
		return nativeBoolToBooleanObject(node.Value)

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
				// Define native function in env if named
				if node.Name != "" {
					env.Define(node.Name, fn)
				}
				return fn
			}
			return newError("native function implementation not found: %s", node.Name)
		}

		fn := &Function{Parameters: node.Parameters, Body: node.Body, Env: env}
		// Define function in env if named
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
			// Define identifiers
			// If 'ambil "io"', define 'io' = mod
			// Simplification: just use base name
			name := node.Path // todo: basename
			env.Define(name, mod)
			return NULL
		}
		return newError("Importer not configured")

	case *parser.MemberExpression:
		obj := e.Eval(node.Object, env)
		if isError(obj) { return obj }

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
	// Check native registry? No, env should have it if imported/declared.
	return newError("identifier not found: " + node.Value)
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

// Placeholder for GetNative, implemented in builtins.go
func GetNative(name string) (*NativeFunction, bool) {
	if fn, ok := NativeRegistry[name]; ok {
		return &NativeFunction{Fn: fn, Name: name}, true
	}
	return nil, false
}
