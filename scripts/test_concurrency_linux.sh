#!/bin/bash
# script/test_concurrency_linux.sh
# Build and Run Concurrency Test

set -e

ASM_DIR="corelib/platform/x86_64/asm"
OUT_DIR="build/linux"

echo "[Build] Assembling Core Sources..."
# Note: Added scheduler.s
SOURCES=(
    "runtime.s" "alloc.s" "stack.s" "builtins.s"
    "executor.s" "control_flow.s" "context.s"
    "scheduler.s"
    "string.s" "type.s" "symbol.s" "arena.s"
    "pool.s" "snapshot.s" "lexer.s" "vector.s"
    "parser.s" "compiler.s"
)

OBJECTS=""
mkdir -p "$OUT_DIR"

for src in "${SOURCES[@]}"; do
    as -g -o "$OUT_DIR/${src%.s}.o" "$ASM_DIR/$src"
    OBJECTS="$OBJECTS $OUT_DIR/${src%.s}.o"
done

echo "[Build] Assembling Test Concurrency..."
as -g -o "$OUT_DIR/test_concurrency.o" "$ASM_DIR/test_concurrency.s"

echo "[Build] Linking..."
ld -o "$OUT_DIR/test_concurrency" $OBJECTS "$OUT_DIR/test_concurrency.o"

echo "[Test] Running..."
"$OUT_DIR/test_concurrency"
