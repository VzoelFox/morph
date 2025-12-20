package analysis

import (
	"time"
)

type Context struct {
	Version       string                 `json:"version"`
	File          string                 `json:"file"`
	Timestamp     time.Time              `json:"timestamp"`
	Checksum      string                 `json:"checksum"`
	Symbols       map[string]*Symbol     `json:"symbols"`
	GlobalVars    map[string]*Variable   `json:"global_variables"`
	LocalScopes   map[string]LocalScope  `json:"local_scopes"`
	Errors        []ParserError          `json:"errors"`
	Warnings      []Warning              `json:"warnings"`
	Imports       []string               `json:"imports"`
	TypeInference map[string][]string    `json:"type_inference"`
	CallGraph     map[string][]string    `json:"call_graph"`
	Complexity    ComplexityMetrics      `json:"complexity"`
	Statistics    CodeStatistics         `json:"statistics"`
}

type Symbol struct {
	Type            string      `json:"type"` // "function", "variable"
	Line            int         `json:"line"`
	Column          int         `json:"column"`
	Parameters      []Parameter `json:"parameters,omitempty"`
	Returns         *TypeInfo   `json:"returns,omitempty"`
	CanError        bool        `json:"can_error,omitempty"`
	ErrorConditions []ErrorCond `json:"error_conditions,omitempty"`
	Doc             string      `json:"doc,omitempty"`
	Calls           []string    `json:"calls,omitempty"`
	LocalVars       []string    `json:"local_variables,omitempty"`
}

type Variable struct {
	Line         int    `json:"line"`
	Type         string `json:"type"` // inferred type
	InitialValue string `json:"initial_value,omitempty"`
}

type LocalScope map[string]*Variable

type Parameter struct {
	Name         string `json:"name"`
	InferredType string `json:"inferred_type"`
	Line         int    `json:"line"`
	Column       int    `json:"column"`
}

type TypeInfo struct {
	Type  string   `json:"type"` // "union", "integer", etc
	Types []string `json:"types,omitempty"`
}

type ErrorCond struct {
	Condition string `json:"condition"`
	Message   string `json:"message"`
	Line      int    `json:"line"`
}

type ParserError struct {
	Level   string `json:"level"`
	Line    int    `json:"line"`
	Column  int    `json:"column"`
	Message string `json:"message"`
	File    string `json:"file"`
	Context string `json:"context"`
}

type Warning struct {
	Code     string `json:"code"`
	Type     string `json:"type"`
	Line     int    `json:"line"`
	Column   int    `json:"column"`
	Message  string `json:"message"`
	Severity string `json:"severity"`
	Function string `json:"function,omitempty"`
	Variable string `json:"variable,omitempty"`
}

type ComplexityMetrics struct {
	Cyclomatic  int `json:"cyclomatic"`
	LinesOfCode int `json:"lines_of_code"`
	Functions   int `json:"functions"`
	MaxNesting  int `json:"max_nesting"`
}

type CodeStatistics struct {
	TotalLines   int `json:"total_lines"`
	CodeLines    int `json:"code_lines"`
	CommentLines int `json:"comment_lines"`
	BlankLines   int `json:"blank_lines"`
}
