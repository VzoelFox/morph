package parser

import (
	"testing"

	"github.com/VzoelFox/morph/pkg/lexer"
)

func TestParserGetLineContentUsesCache(t *testing.T) {
	input := "baris-satu\nbaris-dua\nbaris-tiga"
	p := New(lexer.New(input))

	first := p.getLineContent(1)
	if first != "baris-satu" {
		t.Fatalf("expected first line, got %q", first)
	}
	if p.lineCache == nil {
		t.Fatalf("expected line cache to be initialized")
	}

	p.lineCache[0] = "mutasi-cache"
	updated := p.getLineContent(1)
	if updated != "mutasi-cache" {
		t.Fatalf("expected cached line content, got %q", updated)
	}
}
