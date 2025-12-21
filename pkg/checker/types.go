package checker

import (
	"fmt"
	"strings"
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
	KindMulti // For multiple return values
	KindUnknown
	KindError     // Internal Compiler Error
	KindNull
	KindUserError // User-facing 'Error' type
	KindModule    // Module namespace
)

type Type interface {
	Kind() TypeKind
	String() string
	Equals(other Type) bool
	AssignableTo(target Type) bool
	GetMember(name string) (Type, bool)
	Index(key Type) (Type, error)
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
	// Unknown AND Error match anything (error suppression)
	if t.K == KindUnknown || target.Kind() == KindUnknown || t.K == KindError || target.Kind() == KindError {
		return true
	}

	// Null assignment rules
	if t.K == KindNull {
		k := target.Kind()
		// Null assignable to reference types: Array, Map, Struct, Interface, Function, String, UserError
		// And Null itself
		return k == KindArray || k == KindMap || k == KindStruct || k == KindInterface || k == KindFunction || k == KindString || k == KindNull || k == KindUserError
	}

	return t.Equals(target)
}

func (t *BasicType) GetMember(name string) (Type, bool) {
	return nil, false
}

func (t *BasicType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
}

var (
	IntType       = &BasicType{K: KindInt, Name: "Int"}
	FloatType     = &BasicType{K: KindFloat, Name: "Float"}
	StringType    = &BasicType{K: KindString, Name: "String"}
	BoolType      = &BasicType{K: KindBool, Name: "Bool"}
	VoidType      = &BasicType{K: KindVoid, Name: "Void"}
	UnknownType   = &BasicType{K: KindUnknown, Name: "Unknown"}
	ErrorType     = &BasicType{K: KindError, Name: "Error"} // Internal Sentinel
	NullType      = &BasicType{K: KindNull, Name: "Null"}
	UserErrorType = &BasicType{K: KindUserError, Name: "Error"} // User Type
)

type ModuleType struct {
	Name    string
	Exports map[string]Type
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

func (t *ModuleType) GetMember(name string) (Type, bool) {
	if typ, ok := t.Exports[name]; ok {
		return typ, true
	}
	return nil, false
}

func (t *ModuleType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
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
		// Allow empty array literal (Unknown element) to match any array
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
	// Arrays are invariant (must be Equal)
	return t.Equals(target)
}

func (t *ArrayType) GetMember(name string) (Type, bool) {
	return nil, false
}

func (t *ArrayType) Index(key Type) (Type, error) {
	if key.Kind() != KindInt {
		return t.Element, fmt.Errorf("Array index must be Int")
	}
	return t.Element, nil
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
		// Allow empty map literal (Unknown key/value) to match any map
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
	// Maps are invariant
	return t.Equals(target)
}

func (t *MapType) GetMember(name string) (Type, bool) {
	return nil, false
}

func (t *MapType) Index(key Type) (Type, error) {
	if !t.Key.Equals(key) {
		return t.Value, fmt.Errorf("Map key type mismatch: expected %s, got %s", t.Key.String(), key.String())
	}
	return t.Value, nil
}

type StructType struct {
	Name    string
	Fields  map[string]Type
	Methods map[string]*FunctionType
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
	// Nominal typing
	if o, ok := other.(*StructType); ok {
		return t.Name == o.Name
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

	// Check if target is an Interface (Implementation Check)
	if iface, ok := target.(*InterfaceType); ok {
		for name, method := range iface.Methods {
			structMethod, exists := t.Methods[name]
			if !exists {
				return false
			}
			// Method signature must match exactly
			if !structMethod.Equals(method) {
				return false
			}
		}
		return true
	}

	return t.Equals(target)
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
	// Interface to Interface assignment?
	// For now, identity only.
	// Future: check if t methods are superset of target methods
	return t.Equals(target)
}

func (t *InterfaceType) GetMember(name string) (Type, bool) {
	return nil, false
}

func (t *InterfaceType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
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
	// Functions are invariant
	return t.Equals(target)
}

func (t *FunctionType) GetMember(name string) (Type, bool) {
	return nil, false
}

func (t *FunctionType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
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
	// MultiType matching
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

func (t *MultiType) GetMember(name string) (Type, bool) {
	return nil, false
}

func (t *MultiType) Index(key Type) (Type, error) {
	return nil, fmt.Errorf("Index operation not supported on type %s", t.String())
}
