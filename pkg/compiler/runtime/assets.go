package runtime

import _ "embed"

// Bump to force rebuild
//go:embed morph.h.tpl
var MorphHeader string

//go:embed runtime.c.tpl
var RuntimeSource string
