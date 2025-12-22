package evaluator

import (
	"strconv"
)

func RegisterConv() {
	RegisterNative("Atoi", func(args ...Object) Object {
		if len(args) != 1 {
			return newError("wrong number of arguments. got=%d, want=1", len(args))
		}
		s, ok := args[0].(*String)
		if !ok {
			return newError("argument must be String")
		}

		val, err := strconv.Atoi(s.Value)
		if err != nil {
			return &Tuple{Elements: []Object{
				&Integer{Value: 0},
				&Error{Message: err.Error()},
			}}
		}

		return &Tuple{Elements: []Object{
			&Integer{Value: int64(val)},
			NULL,
		}}
	})

	RegisterNative("Itoa", func(args ...Object) Object {
		if len(args) != 1 {
			return newError("wrong number of arguments. got=%d, want=1", len(args))
		}
		i, ok := args[0].(*Integer)
		if !ok {
			return newError("argument must be Integer")
		}

		return &String{Value: strconv.Itoa(int(i.Value))}
	})
}
