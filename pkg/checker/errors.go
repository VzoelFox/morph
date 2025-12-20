package checker

import "fmt"

type TypeError struct {
	Message string
	Line    int
	Column  int
}

func (e TypeError) Error() string {
	return fmt.Sprintf("Type Error at line %d, col %d: %s", e.Line, e.Column, e.Message)
}

func NewTypeError(line, col int, format string, args ...interface{}) TypeError {
	return TypeError{
		Message: fmt.Sprintf(format, args...),
		Line:    line,
		Column:  col,
	}
}
