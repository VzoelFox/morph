package evaluator

var NativeRegistry = map[string]NativeFunc{}

func RegisterNative(name string, fn NativeFunc) {
	NativeRegistry[name] = fn
}
