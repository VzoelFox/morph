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
	KindArray
	KindMap
	KindMulti // For multiple return values
	KindUnknown
	KindError
	KindNull
)

type Type interface {
	Kind() TypeKind
	String() string
	Equals(other Type) bool
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

var (
	IntType    = &BasicType{K: KindInt, Name: "Int"}
	FloatType  = &BasicType{K: KindFloat, Name: "Float"}
	StringType = &BasicType{K: KindString, Name: "String"}
	BoolType   = &BasicType{K: KindBool, Name: "Bool"}
	VoidType   = &BasicType{K: KindVoid, Name: "Void"}
	UnknownType = &BasicType{K: KindUnknown, Name: "Unknown"}
	ErrorType   = &BasicType{K: KindError, Name: "Error"}
	NullType    = &BasicType{K: KindNull, Name: "Null"}
)

type ArrayType struct {
	Element Type
}

func (t *ArrayType) Kind() TypeKind { return KindArray }
func (t *ArrayType) String() string { return "[]" + t.Element.String() }
func (t *ArrayType) Equals(other Type) bool {
	if other.Kind() == KindNull {
		return true
	}
	if o, ok := other.(*ArrayType); ok {
		return t.Element.Equals(o.Element)
	}
	return false
}

type MapType struct {
	Key   Type
	Value Type
}

func (t *MapType) Kind() TypeKind { return KindMap }
func (t *MapType) String() string { return fmt.Sprintf("map[%s]%s", t.Key.String(), t.Value.String()) }
func (t *MapType) Equals(other Type) bool {
	if other.Kind() == KindNull {
		return true
	}
	if o, ok := other.(*MapType); ok {
		return t.Key.Equals(o.Key) && t.Value.Equals(o.Value)
	}
	return false
}

type StructType struct {
	Name   string
	Fields map[string]Type
}

func (t *StructType) Kind() TypeKind { return KindStruct }
func (t *StructType) String() string { return t.Name }
func (t *StructType) Equals(other Type) bool {
	if other.Kind() == KindNull {
		return true
	}
	// Nominal typing for structs? Or Structural?
	// Usually Nominal (by name).
	if o, ok := other.(*StructType); ok {
		return t.Name == o.Name
	}
	return false
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
