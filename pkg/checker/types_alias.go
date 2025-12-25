package checker

import "github.com/VzoelFox/morph/pkg/types"

type Type = types.Type
type TypeKind = types.TypeKind

type BasicType = types.BasicType
type ExportInfo = types.ExportInfo
type ModuleType = types.ModuleType
type ArrayType = types.ArrayType
type MapType = types.MapType
type StructType = types.StructType
type InterfaceType = types.InterfaceType
type FunctionType = types.FunctionType
type MultiType = types.MultiType
type PointerType = types.PointerType

const (
	KindInt       = types.KindInt
	KindFloat     = types.KindFloat
	KindString    = types.KindString
	KindBool      = types.KindBool
	KindVoid      = types.KindVoid
	KindFunction  = types.KindFunction
	KindStruct    = types.KindStruct
	KindInterface = types.KindInterface
	KindArray     = types.KindArray
	KindMap       = types.KindMap
	KindPointer   = types.KindPointer
	KindMulti     = types.KindMulti
	KindUnknown   = types.KindUnknown
	KindError     = types.KindError
	KindNull      = types.KindNull
	KindUserError = types.KindUserError
	KindModule    = types.KindModule
	KindChannel   = types.KindChannel
)

var (
	IntType       = types.IntType
	FloatType     = types.FloatType
	StringType    = types.StringType
	BoolType      = types.BoolType
	VoidType      = types.VoidType
	UnknownType   = types.UnknownType
	ErrorType     = types.ErrorType
	NullType      = types.NullType
	UserErrorType = types.UserErrorType
	ChannelType   = types.ChannelType
)
