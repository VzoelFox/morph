package runtime

import _ "embed"

//go:embed morph.h.tpl
var MorphHeader string

//go:embed runtime.c.tpl
var RuntimeSource string
