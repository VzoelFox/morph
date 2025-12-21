package checker

import "fmt"

type SymbolInfo struct {
	Type    Type
	IsConst bool
	Line    int
	Column  int
	Used    bool
}

type Scope struct {
	types     map[string]Type
	variables map[string]SymbolInfo
	outer     *Scope
}

func NewScope(outer *Scope) *Scope {
	return &Scope{
		types:     make(map[string]Type),
		variables: make(map[string]SymbolInfo),
		outer:     outer,
	}
}

func (s *Scope) DefineVariable(name string, t Type, isConst bool, line, col int) *TypeWarning {
	// Check outer scope for shadowing
	if s.outer != nil {
		if outerType, exists := s.outer.LookupVariable(name); exists {
			warning := &TypeWarning{
				Message: fmt.Sprintf("Variable '%s' shadows outer declaration (type: %s)", name, outerType.String()),
				Line:    0, // To be filled by caller
				Column:  0,
			}
			s.variables[name] = SymbolInfo{Type: t, IsConst: isConst, Line: line, Column: col, Used: false}
			return warning
		}
	}
	s.variables[name] = SymbolInfo{Type: t, IsConst: isConst, Line: line, Column: col, Used: false}
	return nil
}

func (s *Scope) MarkUsed(name string) {
	if sym, ok := s.variables[name]; ok {
		sym.Used = true
		s.variables[name] = sym
		return
	}
	if s.outer != nil {
		s.outer.MarkUsed(name)
	}
}

func (s *Scope) CheckUnused() []TypeWarning {
	var warnings []TypeWarning
	for name, sym := range s.variables {
		if !sym.Used && name != "main" && name != "parse_int" {
			warnings = append(warnings, TypeWarning{
				Message: fmt.Sprintf("Unused variable '%s'", name),
				Line:    sym.Line,
				Column:  sym.Column,
			})
		}
	}
	return warnings
}

func (s *Scope) DefineType(name string, t Type) {
	s.types[name] = t
}

func (s *Scope) LookupVariable(name string) (Type, bool) {
	sym, ok := s.variables[name]
	if ok {
		return sym.Type, true
	}
	if s.outer != nil {
		return s.outer.LookupVariable(name)
	}
	return nil, false
}

func (s *Scope) LookupSymbol(name string) (*SymbolInfo, bool) {
	sym, ok := s.variables[name]
	if ok {
		return &sym, true
	}
	if s.outer != nil {
		return s.outer.LookupSymbol(name)
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
