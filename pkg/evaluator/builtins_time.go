package evaluator

import (
	"time"
)

func RegisterTime() {
	RegisterNative("Now", func(args ...Object) Object {
		return &Integer{Value: time.Now().UnixMilli()}
	})

	RegisterNative("Sleep", func(args ...Object) Object {
		if len(args) != 1 {
			return newError("wrong number of arguments. got=%d, want=1", len(args))
		}

		ms, ok := args[0].(*Integer)
		if !ok {
			return newError("argument must be Integer")
		}

		// TODO: Upgrade to Async/Yield when Scheduler is ready
		time.Sleep(time.Duration(ms.Value) * time.Millisecond)
		return NULL
	})
}
