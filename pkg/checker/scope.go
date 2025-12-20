package checker

import "fmt"

type Scope struct {
	types     map[string]Type // Struct definitions, etc.
	variables map[string]Type // Variable/Function signatures
	outer     *Scope
}

func NewScope(outer *Scope) *Scope {
	return &Scope{
		types:     make(map[string]Type),
		variables: make(map[string]Type),
		outer:     outer,
	}
}

func (s *Scope) DefineVariable(name string, t Type) *TypeWarning {
	// Check outer scope for shadowing
	if s.outer != nil {
		if _, exists := s.outer.LookupVariable(name); exists {
			warning := &TypeWarning{
				Message: fmt.Sprintf("Variable '%s' shadows outer scope", name),
				Line:    0, // To be filled by caller
				Column:  0,
			}
			s.variables[name] = t
			return warning
		}
	}
	s.variables[name] = t
	return nil
}

func (s *Scope) DefineType(name string, t Type) {
	s.types[name] = t
}

func (s *Scope) LookupVariable(name string) (Type, bool) {
	t, ok := s.variables[name]
	if ok {
		return t, true
	}
	if s.outer != nil {
		return s.outer.LookupVariable(name)
	}
	return nil, false
}

func (s *Scope) LookupType(name string) (Type, bool) {
	t, ok := s.types[name]
	if ok {
		return t, true
	}
	if s.outer != nil {
		return s.outer.LookupType(name)
	}
	return nil, false
}
