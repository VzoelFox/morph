package evaluator

import (
	"os"
)

func RegisterIO() {
	RegisterNative("make_file", func(args ...Object) Object {
		if len(args) != 1 {
			return newError("wrong number of arguments. got=%d, want=1", len(args))
		}
		fd, ok := args[0].(*Integer)
		if !ok {
			return newError("argument must be Integer")
		}
		return &StructInstance{
			Name:   "File",
			Fields: map[string]Object{"fd": fd},
		}
	})

	RegisterNative("Write", func(args ...Object) Object {
		if len(args) != 2 {
			return newError("wrong number of arguments. got=%d, want=2", len(args))
		}

		fileObj, ok := args[0].(*StructInstance)
		if !ok || fileObj.Name != "File" {
			return newError("first argument must be File")
		}

		fdObj, ok := fileObj.Fields["fd"].(*Integer)
		if !ok {
			return newError("File.fd is not Integer")
		}

		strObj, ok := args[1].(*String)
		if !ok {
			return newError("second argument must be String")
		}

		var f *os.File
		fd := uintptr(fdObj.Value)

		switch fd {
		case 0:
			f = os.Stdin
		case 1:
			f = os.Stdout
		case 2:
			f = os.Stderr
		default:
			// WARNING: This assumes the FD is valid and open.
			// In a real system, we'd look up a handle map.
			f = os.NewFile(fd, "external")
		}

		n, err := f.WriteString(strObj.Value)
		if err != nil {
			return newError("Write error: %s", err.Error())
		}

		return &Integer{Value: int64(n)}
	})

	RegisterNative("Read", func(args ...Object) Object {
		if len(args) != 2 {
			return newError("wrong number of arguments. got=%d, want=2", len(args))
		}

		fileObj, ok := args[0].(*StructInstance)
		if !ok || fileObj.Name != "File" {
			return newError("first argument must be File")
		}

		fdObj, ok := fileObj.Fields["fd"].(*Integer)
		if !ok {
			return newError("File.fd is not Integer")
		}

		sizeObj, ok := args[1].(*Integer)
		if !ok {
			return newError("second argument must be Integer")
		}

		f := os.NewFile(uintptr(fdObj.Value), "external")
		buf := make([]byte, sizeObj.Value)
		n, err := f.Read(buf)
		if err != nil && err.Error() != "EOF" {
			return newError("Read error: %s", err.Error())
		}

		return &String{Value: string(buf[:n])}
	})

	RegisterNative("Open", func(args ...Object) Object {
		if len(args) != 1 {
			return newError("wrong number of arguments. got=%d, want=1", len(args))
		}
		pathObj, ok := args[0].(*String)
		if !ok {
			return newError("argument must be String")
		}

		f, err := os.Open(pathObj.Value)
		if err != nil {
			return newError("Open error: %s", err.Error())
		}

		// Note: we are leaking the file handle here if Close is not called.
		// Also relying on FD stability.
		return &StructInstance{
			Name:   "File",
			Fields: map[string]Object{"fd": &Integer{Value: int64(f.Fd())}},
		}
	})

	RegisterNative("Close", func(args ...Object) Object {
		if len(args) != 1 {
			return newError("wrong number of arguments. got=%d, want=1", len(args))
		}
		fileObj, ok := args[0].(*StructInstance)
		if !ok || fileObj.Name != "File" {
			return newError("first argument must be File")
		}

		fdObj, ok := fileObj.Fields["fd"].(*Integer)
		if !ok {
			return newError("File.fd is not Integer")
		}

		// We can't easily close an arbitrary FD via os.NewFile without risks.
		// But for now:
		f := os.NewFile(uintptr(fdObj.Value), "closed")
		if f != nil {
			f.Close()
		}
		return &Integer{Value: 0}
	})
}
