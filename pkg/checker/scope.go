package checker

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

func (s *Scope) DefineVariable(name string, t Type) {
	s.variables[name] = t
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
