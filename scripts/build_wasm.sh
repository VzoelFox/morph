#!/bin/bash
set -e

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}[Morph WASM] Building...${NC}"

# 1. Compile WAT to WASM
echo "  -> Merging WAT files..."

# REVISED MEMORY LAYOUT (FIXED)
# 0 - 64KB: Reserved / Stack
# 65536 (64KB): Stack Base (Grows Down)
# 65536: Heap Base (Grows Up)
# Code & Source: Managed by Allocator (Heap) or placed very high.
# Let's place Heap at 1MB (1048576) to be super safe and leave room for Stack/Globals.
# Wait, Memory 10 pages = 640KB.
# So Heap starts at 65536 (64KB). Stack grows down from 65536.
# This is safe. Stack is 0-64KB. Heap is 64KB+.
# Loader places Code at 200KB (204800) and Source at 300KB (307200).
# We must ensure Heap doesn't grow into them immediately.
# Lexer allocs 32KB vector. 64KB + 32KB = 96KB. Safe.

cat > build/morph_merged.wat <<EOF
(module
  ;; Imports
  (import "env" "sys_write" (func \$sys_write (param i64 i64 i64) (result i64)))
  (import "env" "sys_dom_create" (func \$sys_dom_create (param i64) (result i64)))
  (import "env" "sys_dom_append" (func \$sys_dom_append (param i64 i64)))
  (import "env" "sys_dom_set_attr" (func \$sys_dom_set_attr (param i64 i64 i64)))
  (import "env" "sys_dom_set_text" (func \$sys_dom_set_text (param i64 i64)))
  (import "env" "sys_dom_get_by_id" (func \$sys_dom_get_by_id (param i64) (result i64)))
  (import "env" "sys_exit" (func \$sys_exit (param i64)))

  ;; Memory (10 Pages = 640KB)
  (memory \$memory (export "memory") 10)

  ;; Globals
  (global \$heap_base (mut i32) (i32.const 65536))   ;; Heap starts at 64KB
  (global \$stack_ptr (mut i32) (i32.const 65536))   ;; Stack Top at 64KB
  (global \$sp (mut i32) (i32.const 0))              ;; Morph VM SP (Internal)
  (global \$stack_base (mut i32) (i32.const 65536))  ;; Morph Stack Base
  (global \$ip (mut i32) (i32.const 0))

  ;; Constants for Lexer
  (global \$TOKEN_EOF i64 (i64.const 0))
  (global \$TOKEN_INTEGER i64 (i64.const 1))
  (global \$TOKEN_STRING i64 (i64.const 3))
  (global \$TOKEN_IDENTIFIER i64 (i64.const 4))
  (global \$TOKEN_SYMBOL i64 (i64.const 5))
  (global \$TOKEN_KEYWORD i64 (i64.const 6))
EOF

# Helper to extract body
extract_body() {
    sed '1d;$d' $1 | grep -v "(import" | grep -v "(memory" | grep -v "(global"
}

echo "  -> Extracting Runtime..."
extract_body corelib/platform/wasm/wat/runtime.wat >> build/morph_merged.wat

echo "  -> Extracting Syscalls..."
extract_body corelib/platform/wasm/wat/syscalls.wat >> build/morph_merged.wat

echo "  -> Extracting Executor..."
extract_body corelib/platform/wasm/wat/executor.wat >> build/morph_merged.wat

echo "  -> Extracting Lexer..."
extract_body corelib/platform/wasm/wat/lexer.wat >> build/morph_merged.wat

echo ")" >> build/morph_merged.wat

echo "  -> Compiling to WASM..."
wat2wasm build/morph_merged.wat -o corelib/platform/wasm/morph.wasm

echo -e "${GREEN}[Morph WASM] Build Complete: corelib/platform/wasm/morph.wasm${NC}"
