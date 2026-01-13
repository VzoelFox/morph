#!/bin/bash
# scripts/test_net_real.sh
# Build and Run Real Networking Test (DNS + HTTP)

set -e

ASM_DIR="corelib/platform/x86_64/asm"
OUT_DIR="build/linux"

echo "[Build] Assembling Core Sources..."
# Include net.s and dns.s
SOURCES=(
    "runtime.s" "alloc.s" "stack.s" "builtins.s"
    "executor.s" "control_flow.s" "context.s"
    "scheduler.s" "string.s" "type.s" "symbol.s"
    "arena.s" "pool.s" "snapshot.s" "lexer.s" "vector.s"
    "parser.s" "compiler.s"
    "net.s" "dns.s"
)

OBJECTS=""
mkdir -p "$OUT_DIR"

for src in "${SOURCES[@]}"; do
    as -g -o "$OUT_DIR/${src%.s}.o" "$ASM_DIR/$src"
    OBJECTS="$OBJECTS $OUT_DIR/${src%.s}.o"
done

echo "[Build] Assembling Test Network..."
as -g -o "$OUT_DIR/test_net.o" "$ASM_DIR/test_net.s"

echo "[Build] Linking..."
ld -o "$OUT_DIR/test_net" $OBJECTS "$OUT_DIR/test_net.o"

echo "[Test] Running Network Test..."
"$OUT_DIR/test_net"
