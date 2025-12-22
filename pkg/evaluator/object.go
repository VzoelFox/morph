package evaluator

import (
	"bytes"
	"fmt"
	"strings"

	"github.com/VzoelFox/morph/pkg/parser"
)

type ObjectType string

const (
	INTEGER_OBJ      = "INTEGER"
	FLOAT_OBJ        = "FLOAT"
	STRING_OBJ       = "STRING"
	BOOLEAN_OBJ      = "BOOLEAN"
	NULL_OBJ         = "NULL"
	RETURN_VALUE_OBJ = "RETURN_VALUE"
	ERROR_OBJ        = "ERROR"
	FUNCTION_OBJ     = "FUNCTION"
	NATIVE_OBJ       = "NATIVE"
	STRUCT_OBJ       = "STRUCT"
	MODULE_OBJ       = "MODULE"
	TUPLE_OBJ        = "TUPLE"
)

type Object interface {
	Type() ObjectType
	Inspect() string
}

type Integer struct {
	Value int64
}

func (i *Integer) Type() ObjectType { return INTEGER_OBJ }
func (i *Integer) Inspect() string  { return fmt.Sprintf("%d", i.Value) }

type Float struct {
	Value float64
}

func (f *Float) Type() ObjectType { return FLOAT_OBJ }
func (f *Float) Inspect() string  { return fmt.Sprintf("%f", f.Value) }

type String struct {
	Value string
}

func (s *String) Type() ObjectType { return STRING_OBJ }
func (s *String) Inspect() string  { return s.Value }

type Boolean struct {
	Value bool
}

func (b *Boolean) Type() ObjectType { return BOOLEAN_OBJ }
func (b *Boolean) Inspect() string  { return fmt.Sprintf("%t", b.Value) }

type Null struct{}

func (n *Null) Type() ObjectType { return NULL_OBJ }
func (n *Null) Inspect() string  { return "null" }

type ReturnValue struct {
	Value Object
}

func (rv *ReturnValue) Type() ObjectType { return RETURN_VALUE_OBJ }
func (rv *ReturnValue) Inspect() string  { return rv.Value.Inspect() }

type Error struct {
	Message string
}

func (e *Error) Type() ObjectType { return ERROR_OBJ }
func (e *Error) Inspect() string  { return "ERROR: " + e.Message }

type Function struct {
	Parameters []*parser.Parameter
	Body       *parser.BlockStatement
	Env        *Environment
}

func (f *Function) Type() ObjectType { return FUNCTION_OBJ }
func (f *Function) Inspect() string {
	var out bytes.Buffer

	params := []string{}
	for _, p := range f.Parameters {
		params = append(params, p.String())
	}

	out.WriteString("fungsi")
	out.WriteString("(")
	out.WriteString(strings.Join(params, ", "))
	out.WriteString(") {\n")
	out.WriteString(f.Body.String())
	out.WriteString("\n}")

	return out.String()
}

type NativeFunc func(args ...Object) Object

type NativeFunction struct {
	Fn NativeFunc
    Name string
}

func (n *NativeFunction) Type() ObjectType { return NATIVE_OBJ }
func (n *NativeFunction) Inspect() string  { return fmt.Sprintf("fungsi native %s", n.Name) }

type StructInstance struct {
    Name   string
    Fields map[string]Object
}

func (s *StructInstance) Type() ObjectType { return STRUCT_OBJ }
func (s *StructInstance) Inspect() string {
    var out bytes.Buffer
    out.WriteString(s.Name)
    out.WriteString("{")
    fields := []string{}
    for k, v := range s.Fields {
        fields = append(fields, k+": "+v.Inspect())
    }
    out.WriteString(strings.Join(fields, ", "))
    out.WriteString("}")
    return out.String()
}

type Module struct {
    Name string
    Env  *Environment
}

func (m *Module) Type() ObjectType { return MODULE_OBJ }
func (m *Module) Inspect() string  { return "module(" + m.Name + ")" }

type Tuple struct {
	Elements []Object
}

func (t *Tuple) Type() ObjectType { return TUPLE_OBJ }
func (t *Tuple) Inspect() string {
	var out bytes.Buffer
	elements := []string{}
	for _, el := range t.Elements {
		elements = append(elements, el.Inspect())
	}
	out.WriteString("(")
	out.WriteString(strings.Join(elements, ", "))
	out.WriteString(")")
	return out.String()
}
