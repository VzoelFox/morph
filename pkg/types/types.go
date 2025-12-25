package types

import (
	"fmt"
	"strings"

	"github.com/VzoelFox/morph/pkg/parser"
)

type TypeKind int

const (
	KindInt TypeKind = iota
	KindFloat
	KindString
	KindBool
	KindVoid
	KindFunction
	KindStruct
	KindInterface
	KindArray
	KindMap
	KindPointer
	KindMulti // For multiple return values
	KindUnknown
	KindError // Internal Compiler Error
	KindNull
	KindUserError // User-facing 'Error' type
	KindModule    // Module namespace
	KindChannel   // Channel for concurrency
)

type Type interface {
	Kind() TypeKind
	String() string
	Equals(other Type) bool
	AssignableTo(target Type) bool
	IsComparable() bool
	GetMember(name string) (Type, bool)
	Index(key Type) (Type, error)
	BinaryOp(op string, right Type) (Type, error)
	PrefixOp(op string) (Type, error)
	Call(args []Type) (Type, string, error)
}

type BasicType struct {
	K    TypeKind
	Name string
}

func (t *BasicType) Kind() TypeKind { return t.K }
func (t *BasicType) String() string { return t.Name }
func (t *BasicType) Equals(other Type) bool {
	if other == nil {
		return false
	}
	return t.K == other.Kind()
}

func (t *BasicType) AssignableTo(target Type) bool {
	if target == nil {
		return false
	}
	if t.K == KindUnknown || target.Kind() == KindUnknown || t.K == KindError || target.Kind() == KindError {
		return true
	}
	if t.K == KindNull {
		k := target.Kind()
		return k == KindArray || k == KindMap || k == KindPointer || k == KindInterface || k == KindFunction || k == KindString || k == KindNull || k == KindUserError || k == KindStruct
	}
	return t.Equals(target)
}

func (t *BasicType) IsComparable() bool {
	switch t.K {
	case KindInt, KindFloat, KindString, KindBool, KindNull, KindUserError:
		return true
	case KindVoid, KindUnknown, KindError:
		return false
	}
	return false
}

func (t *BasicType) GetMember(name string) (Type, bool) {
	return nil, false
}

func (t *BasicType) Index(key Type) (Type, error) {
	if t.K == KindString {
		if key.Kind() == KindInt {
			// string indexing returns the byte value (Int)
			return IntType, nil
		}
		return nil, fmt.Errorf("String index must be Int")
	}
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
}

func (t *BasicType) BinaryOp(op string, right Type) (Type, error) {
	if t.K == KindUnknown || right.Kind() == KindUnknown {
		return UnknownType, nil
	}
	switch op {
	case "+", "-", "*", "/":
		if t.K == KindInt && right.Kind() == KindInt {
			return IntType, nil
		}
		if t.K == KindFloat && right.Kind() == KindFloat {
			return FloatType, nil
		}
		if t.K == KindString && right.Kind() == KindString && op == "+" {
			return StringType, nil
		}
		return nil, fmt.Errorf("Operator %s not defined for types %s and %s", op, t.String(), right.String())
	case "%":
		if t.K == KindInt && right.Kind() == KindInt {
			return IntType, nil
		}
		// Future: Float modulo?
		return nil, fmt.Errorf("Operator %% requires int operands")
	case "==", "!=":
		if t.Equals(right) {
			return BoolType, nil
		}
		// Allow comparison with Null if assignable
		if t.K == KindNull {
			if t.AssignableTo(right) {
				return BoolType, nil
			}
		}
		if right.Kind() == KindNull {
			if right.AssignableTo(t) {
				return BoolType, nil
			}
		}
		return nil, fmt.Errorf("Cannot compare different types %s and %s", t.String(), right.String())
	case "&&", "||", "dan", "atau":
		if t.K == KindBool && right.Kind() == KindBool {
			return BoolType, nil
		}
		return nil, fmt.Errorf("Operator %s requires Bool operands", op)
	case "<", ">", "<=", ">=":
		if (t.K == KindInt || t.K == KindFloat) && t.K == right.Kind() {
			return BoolType, nil
		}
		return nil, fmt.Errorf("Operator %s requires numeric operands", op)
	case "&", "|", "^", "<<", ">>":
		if t.K == KindInt && right.Kind() == KindInt {
			return IntType, nil
		}
		return nil, fmt.Errorf("Bitwise operator %s requires int operands", op)
	}
	return nil, fmt.Errorf("Unknown operator %s", op)
}

func (t *BasicType) PrefixOp(op string) (Type, error) {
	if t.K == KindUnknown {
		return UnknownType, nil
	}
	switch op {
	case "!":
		if t.K == KindBool {
			return BoolType, nil
		}
		return nil, fmt.Errorf("Operator ! not defined for type %s", t.String())
	case "-":
		if t.K == KindInt || t.K == KindFloat {
			return t, nil
		}
		return nil, fmt.Errorf("Operator - not defined for type %s", t.String())
	case "~":
		if t.K == KindInt {
			return IntType, nil
		}
		return nil, fmt.Errorf("Operator ~ requires int")
	}
	return nil, fmt.Errorf("Unknown prefix operator %s", op)
}

func (t *BasicType) Call(args []Type) (Type, string, error) {
	// Implements Type Conversion/Casting (e.g. Int(5.5))
	if len(args) != 1 {
		return nil, "", fmt.Errorf("Type conversion requires exactly 1 argument")
	}
	arg := args[0]
	if arg.Kind() == KindUnknown {
		return t, "", nil
	}

	if t.K == KindFloat && arg.Kind() == KindInt {
		return FloatType, "", nil
	}
	if t.K == KindInt && arg.Kind() == KindFloat {
		return IntType, "Lossy conversion from Float to Int", nil
	}
	if t.K == KindUserError && arg.Kind() == KindString {
		return t, "", nil
	}
	if arg.AssignableTo(t) {
		return t, "", nil
	}
	return nil, "", fmt.Errorf("Cannot convert type %s to %s", arg.String(), t.String())
}

var (
	IntType       = &BasicType{K: KindInt, Name: "int"}
	FloatType     = &BasicType{K: KindFloat, Name: "float"}
	StringType    = &BasicType{K: KindString, Name: "string"}
	BoolType      = &BasicType{K: KindBool, Name: "bool"}
	VoidType      = &BasicType{K: KindVoid, Name: "void"}
	UnknownType   = &BasicType{K: KindUnknown, Name: "Unknown"}
	ErrorType     = &BasicType{K: KindError, Name: "Error"}
	NullType      = &BasicType{K: KindNull, Name: "Null"}
	UserErrorType = &BasicType{K: KindUserError, Name: "error"}
	ChannelType   = &BasicType{K: KindChannel, Name: "channel"}
)

type ExportInfo struct {
	Type   Type
	IsType bool // True if it is a Type Definition, False if it is a Value/Variable
}

type ModuleType struct {
	Name    string
	Exports map[string]ExportInfo
	Program *parser.Program
}

func (t *ModuleType) Kind() TypeKind { return KindModule }
func (t *ModuleType) String() string { return "Module<" + t.Name + ">" }
func (t *ModuleType) Equals(other Type) bool {
	if other == nil {
		return false
	}
	if o, ok := other.(*ModuleType); ok {
		return t.Name == o.Name
	}
	return false
}
func (t *ModuleType) AssignableTo(target Type) bool {
	if target == nil {
		return false
	}
	if target.Kind() == KindUnknown {
		return true
	}
	return t.Equals(target)
}
func (t *ModuleType) IsComparable() bool { return false }
func (t *ModuleType) GetMember(name string) (Type, bool) {
	if info, ok := t.Exports[name]; ok {
		return info.Type, true
	}
	return nil, false
}
func (t *ModuleType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
}
func (t *ModuleType) BinaryOp(op string, right Type) (Type, error) {
	return nil, fmt.Errorf("Operator %s not supported on module", op)
}
func (t *ModuleType) PrefixOp(op string) (Type, error) {
	return nil, fmt.Errorf("Prefix operator %s not supported on module", op)
}
func (t *ModuleType) Call(args []Type) (Type, string, error) {
	return nil, "", fmt.Errorf("Cannot call module type")
}

type ArrayType struct {
	Element Type
}

func (t *ArrayType) Kind() TypeKind { return KindArray }
func (t *ArrayType) String() string { return "[]" + t.Element.String() }
func (t *ArrayType) Equals(other Type) bool {
	if other == nil {
		return false
	}
	if other.Kind() == KindNull {
		return true
	}
	if o, ok := other.(*ArrayType); ok {
		if t.Element.Kind() == KindUnknown || o.Element.Kind() == KindUnknown {
			return true
		}
		return t.Element.Equals(o.Element)
	}
	return false
}
func (t *ArrayType) AssignableTo(target Type) bool {
	if target == nil {
		return false
	}
	if target.Kind() == KindUnknown {
		return true
	}
	return t.Equals(target)
}
func (t *ArrayType) IsComparable() bool { return false }
func (t *ArrayType) GetMember(name string) (Type, bool) {
	return nil, false
}
func (t *ArrayType) Index(key Type) (Type, error) {
	if key.Kind() != KindInt {
		return t.Element, fmt.Errorf("Array index must be Int")
	}
	return t.Element, nil
}
func (t *ArrayType) BinaryOp(op string, right Type) (Type, error) {
	if op == "==" || op == "!=" {
		// Arrays not comparable by default in Go, but maybe logic equality?
		// For now, identity/type check.
		if t.Equals(right) {
			return BoolType, nil
		}
		return nil, fmt.Errorf("Type mismatch")
	}
	if op == "+" {
		if t.Equals(right) {
			return t, nil
		}
		return nil, fmt.Errorf("Cannot concatenate arrays of different types %s and %s", t.String(), right.String())
	}
	return nil, fmt.Errorf("Operator %s not supported on array", op)
}
func (t *ArrayType) PrefixOp(op string) (Type, error) {
	return nil, fmt.Errorf("Prefix operator %s not supported on array", op)
}
func (t *ArrayType) Call(args []Type) (Type, string, error) {
	return nil, "", fmt.Errorf("Cannot call array type")
}

type MapType struct {
	Key   Type
	Value Type
}

func (t *MapType) Kind() TypeKind { return KindMap }
func (t *MapType) String() string { return fmt.Sprintf("map[%s]%s", t.Key.String(), t.Value.String()) }
func (t *MapType) Equals(other Type) bool {
	if other == nil {
		return false
	}
	if other.Kind() == KindNull {
		return true
	}
	if o, ok := other.(*MapType); ok {
		keyUnknown := t.Key.Kind() == KindUnknown || o.Key.Kind() == KindUnknown
		valUnknown := t.Value.Kind() == KindUnknown || o.Value.Kind() == KindUnknown
		if keyUnknown && valUnknown {
			return true
		}
		return t.Key.Equals(o.Key) && t.Value.Equals(o.Value)
	}
	return false
}
func (t *MapType) AssignableTo(target Type) bool {
	if target == nil {
		return false
	}
	if target.Kind() == KindUnknown {
		return true
	}
	return t.Equals(target)
}
func (t *MapType) IsComparable() bool { return false }
func (t *MapType) GetMember(name string) (Type, bool) {
	return nil, false
}
func (t *MapType) Index(key Type) (Type, error) {
	if !t.Key.Equals(key) {
		return t.Value, fmt.Errorf("Map key type mismatch: expected %s, got %s", t.Key.String(), key.String())
	}
	return t.Value, nil
}
func (t *MapType) BinaryOp(op string, right Type) (Type, error) {
	if op == "==" || op == "!=" {
		if t.Equals(right) {
			return BoolType, nil
		}
		return nil, fmt.Errorf("Type mismatch")
	}
	return nil, fmt.Errorf("Operator %s not supported on map", op)
}
func (t *MapType) PrefixOp(op string) (Type, error) {
	return nil, fmt.Errorf("Prefix operator %s not supported on map", op)
}
func (t *MapType) Call(args []Type) (Type, string, error) {
	return nil, "", fmt.Errorf("Cannot call map type")
}

type StructType struct {
	Name       string
	Module     string
	Fields     map[string]Type
	FieldOrder []string
	Methods    map[string]*FunctionType
}

func (t *StructType) Kind() TypeKind { return KindStruct }
func (t *StructType) String() string { return t.Name }
func (t *StructType) Equals(other Type) bool {
	if other == nil {
		return false
	}
	if other.Kind() == KindNull {
		return true
	}
	if o, ok := other.(*StructType); ok {
		// Strict equality checks Module too
		return t.Name == o.Name && t.Module == o.Module
	}
	return false
}
func (t *StructType) AssignableTo(target Type) bool {
	if target == nil {
		return false
	}
	if target.Kind() == KindUnknown {
		return true
	}
	if iface, ok := target.(*InterfaceType); ok {
		for name, method := range iface.Methods {
			structMethod, exists := t.Methods[name]
			if !exists {
				return false
			}
			if !structMethod.Equals(method) {
				return false
			}
		}
		return true
	}
	return t.Equals(target)
}

func (t *StructType) IsComparable() bool {
	for _, fieldType := range t.Fields {
		if !fieldType.IsComparable() {
			return false
		}
	}
	return true
}

func (t *StructType) GetMember(name string) (Type, bool) {
	if fieldType, exists := t.Fields[name]; exists {
		return fieldType, true
	}
	if methodType, exists := t.Methods[name]; exists {
		return methodType, true
	}
	return nil, false
}
func (t *StructType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
}
func (t *StructType) BinaryOp(op string, right Type) (Type, error) {
	if op == "==" || op == "!=" {
		if !t.IsComparable() {
			return nil, fmt.Errorf("Struct %s is not comparable (contains map, array, or function)", t.Name)
		}
		if t.Equals(right) {
			return BoolType, nil
		}
		return nil, fmt.Errorf("Type mismatch")
	}
	return nil, fmt.Errorf("Operator %s not supported on struct", op)
}
func (t *StructType) PrefixOp(op string) (Type, error) {
	return nil, fmt.Errorf("Prefix operator %s not supported on struct", op)
}
func (t *StructType) Call(args []Type) (Type, string, error) {
	// Struct Constructor logic
	if len(args) != len(t.FieldOrder) {
		return nil, "", fmt.Errorf("Struct %s constructor requires %d arguments, got %d", t.Name, len(t.FieldOrder), len(args))
	}

	for i, arg := range args {
		fieldName := t.FieldOrder[i]
		expectedType := t.Fields[fieldName]

		if !arg.AssignableTo(expectedType) {
			return nil, "", fmt.Errorf("Argument %d (%s) type mismatch: expected %s, got %s", i+1, fieldName, expectedType.String(), arg.String())
		}
	}

	return t, "", nil
}

type InterfaceType struct {
	Name    string
	Methods map[string]*FunctionType
}

func (t *InterfaceType) Kind() TypeKind { return KindInterface }
func (t *InterfaceType) String() string { return t.Name }
func (t *InterfaceType) Equals(other Type) bool {
	if other == nil {
		return false
	}
	if other.Kind() == KindNull {
		return true
	}
	if o, ok := other.(*InterfaceType); ok {
		return t.Name == o.Name
	}
	return false
}
func (t *InterfaceType) AssignableTo(target Type) bool {
	if target == nil {
		return false
	}
	if target.Kind() == KindUnknown {
		return true
	}
	return t.Equals(target)
}
func (t *InterfaceType) IsComparable() bool { return true }
func (t *InterfaceType) GetMember(name string) (Type, bool) {
	if method, ok := t.Methods[name]; ok {
		return method, true
	}
	return nil, false
}
func (t *InterfaceType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
}
func (t *InterfaceType) BinaryOp(op string, right Type) (Type, error) {
	if op == "==" || op == "!=" {
		if t.Equals(right) {
			return BoolType, nil
		}
		return nil, fmt.Errorf("Type mismatch")
	}
	return nil, fmt.Errorf("Operator %s not supported on interface", op)
}
func (t *InterfaceType) PrefixOp(op string) (Type, error) {
	return nil, fmt.Errorf("Prefix operator %s not supported on interface", op)
}
func (t *InterfaceType) Call(args []Type) (Type, string, error) {
	return nil, "", fmt.Errorf("Cannot call interface type")
}

type FunctionType struct {
	Parameters  []Type
	ReturnTypes []Type
}

func (t *FunctionType) Kind() TypeKind { return KindFunction }
func (t *FunctionType) String() string {
	params := ""
	for i, p := range t.Parameters {
		if i > 0 {
			params += ", "
		}
		params += p.String()
	}
	rets := ""
	if len(t.ReturnTypes) == 1 {
		rets = t.ReturnTypes[0].String()
	} else if len(t.ReturnTypes) > 1 {
		rets = "("
		for i, r := range t.ReturnTypes {
			if i > 0 {
				rets += ", "
			}
			rets += r.String()
		}
		rets += ")"
	} else {
		rets = "Void"
	}
	return fmt.Sprintf("fungsi(%s) %s", params, rets)
}
func (t *FunctionType) Equals(other Type) bool {
	if other == nil {
		return false
	}
	if other.Kind() == KindNull {
		return true
	}
	if o, ok := other.(*FunctionType); ok {
		if len(t.Parameters) != len(o.Parameters) {
			return false
		}
		for i, p := range t.Parameters {
			if !p.Equals(o.Parameters[i]) {
				return false
			}
		}
		if len(t.ReturnTypes) != len(o.ReturnTypes) {
			return false
		}
		for i, r := range t.ReturnTypes {
			if !r.Equals(o.ReturnTypes[i]) {
				return false
			}
		}
		return true
	}
	return false
}
func (t *FunctionType) AssignableTo(target Type) bool {
	if target == nil {
		return false
	}
	if target.Kind() == KindUnknown {
		return true
	}
	return t.Equals(target)
}
func (t *FunctionType) IsComparable() bool { return false }
func (t *FunctionType) GetMember(name string) (Type, bool) {
	return nil, false
}
func (t *FunctionType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
}
func (t *FunctionType) BinaryOp(op string, right Type) (Type, error) {
	if op == "==" || op == "!=" {
		if t.Equals(right) {
			return BoolType, nil
		}
		return nil, fmt.Errorf("Type mismatch")
	}
	return nil, fmt.Errorf("Operator %s not supported on function", op)
}
func (t *FunctionType) PrefixOp(op string) (Type, error) {
	return nil, fmt.Errorf("Prefix operator %s not supported on function", op)
}
func (t *FunctionType) Call(args []Type) (Type, string, error) {
	if len(args) != len(t.Parameters) {
		return nil, "", fmt.Errorf("Wrong number of arguments: expected %d, got %d", len(t.Parameters), len(args))
	}
	for i, arg := range args {
		if !arg.AssignableTo(t.Parameters[i]) {
			return nil, "", fmt.Errorf("Argument %d type mismatch: expected %s, got %s", i+1, t.Parameters[i].String(), arg.String())
		}
	}
	if len(t.ReturnTypes) == 1 {
		return t.ReturnTypes[0], "", nil
	}
	return &MultiType{Types: t.ReturnTypes}, "", nil
}

type MultiType struct {
	Types []Type
}

func (t *MultiType) Kind() TypeKind { return KindMulti }
func (t *MultiType) String() string {
	parts := []string{}
	for _, sub := range t.Types {
		parts = append(parts, sub.String())
	}
	return "(" + strings.Join(parts, ", ") + ")"
}
func (t *MultiType) Equals(other Type) bool {
	if o, ok := other.(*MultiType); ok {
		if len(t.Types) != len(o.Types) {
			return false
		}
		for i, sub := range t.Types {
			if !sub.Equals(o.Types[i]) {
				return false
			}
		}
		return true
	}
	return false
}
func (t *MultiType) AssignableTo(target Type) bool {
	if target == nil {
		return false
	}
	if target.Kind() == KindUnknown {
		return true
	}
	if o, ok := target.(*MultiType); ok {
		if len(t.Types) != len(o.Types) {
			return false
		}
		for i, sub := range t.Types {
			if !sub.AssignableTo(o.Types[i]) {
				return false
			}
		}
		return true
	}
	return false
}
func (t *MultiType) IsComparable() bool { return false }
func (t *MultiType) GetMember(name string) (Type, bool) {
	return nil, false
}
func (t *MultiType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
}
func (t *MultiType) BinaryOp(op string, right Type) (Type, error) {
	return nil, fmt.Errorf("Operator %s not supported on multi-type", op)
}
func (t *MultiType) PrefixOp(op string) (Type, error) {
	return nil, fmt.Errorf("Prefix operator %s not supported on multi-type", op)
}
func (t *MultiType) Call(args []Type) (Type, string, error) {
	return nil, "", fmt.Errorf("Cannot call multi-type")
}

type PointerType struct {
	Element Type
}

func (t *PointerType) Kind() TypeKind { return KindPointer }
func (t *PointerType) String() string { return "*" + t.Element.String() }
func (t *PointerType) Equals(other Type) bool {
	if other == nil {
		return false
	}
	if other.Kind() == KindNull {
		return true
	}
	if o, ok := other.(*PointerType); ok {
		return t.Element.Equals(o.Element)
	}
	return false
}
func (t *PointerType) AssignableTo(target Type) bool {
	if target == nil {
		return false
	}
	if target.Kind() == KindUnknown {
		return true
	}
	return t.Equals(target)
}
func (t *PointerType) IsComparable() bool { return true }
func (t *PointerType) GetMember(name string) (Type, bool) {
	// Auto-dereference support for convenience (p.Name vs (*p).Name)
	return t.Element.GetMember(name)
}
func (t *PointerType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Cannot index pointer type")
}
func (t *PointerType) BinaryOp(op string, right Type) (Type, error) {
	if op == "==" || op == "!=" {
		if t.Equals(right) || right.Kind() == KindNull {
			return BoolType, nil
		}
		return nil, fmt.Errorf("Type mismatch")
	}
	return nil, fmt.Errorf("Operator %s not supported on pointer", op)
}
func (t *PointerType) PrefixOp(op string) (Type, error) {
	if op == "*" {
		return t.Element, nil
	}
	return nil, fmt.Errorf("Prefix operator %s not supported on pointer", op)
}
func (t *PointerType) Call(args []Type) (Type, string, error) {
	return nil, "", fmt.Errorf("Cannot call pointer type")
}
