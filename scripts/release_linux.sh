#!/bin/bash
# scripts/release_linux.sh
# Release Build Script for Linux x86_64
# Compiles all core libraries and links them into a single 'morph' executable.
# Uses 'hello.s' as the entry point for now (Proof of Concept).

set -e

ASM_DIR="corelib/platform/x86_64/asm"
OUT_DIR="release/linux"
BIN_NAME="morph_v1.1"

echo "[Release] Setting up directories..."
mkdir -p "$OUT_DIR"

# Core Sources
SOURCES=(
    "runtime.s"
    "alloc.s"
    "stack.s"
    "scheduler.s"
    "builtins.s"
    "executor.s"
    "control_flow.s"
    "context.s"
    "string.s"
    "type.s"
    "symbol.s"
    "arena.s"
    "pool.s"
    "snapshot.s"
    "lexer.s"
    "vector.s"
    "parser.s"
    "compiler.s"
    "net.s"
    "dns.s"
    "graphics.s"
    "font.s"
    "daemon_cleaner.s"
)

OBJECTS=""

echo "[Release] Assembling Core Sources..."
for src in "${SOURCES[@]}"; do
    echo "  -> $src"
    as -o "$OUT_DIR/${src%.s}.o" "$ASM_DIR/$src"
    OBJECTS="$OBJECTS $OUT_DIR/${src%.s}.o"
done

# Entry Point (Main)
# Using 'hello.s' for v1.1 frozen binary demonstration
echo "[Release] Assembling Entry Point..."
as -o "$OUT_DIR/main.o" "$ASM_DIR/hello.s"

echo "[Release] Linking $BIN_NAME..."
ld -o "$OUT_DIR/$BIN_NAME" $OBJECTS "$OUT_DIR/main.o"

echo "[Release] Stripping symbols..."
strip "$OUT_DIR/$BIN_NAME"

echo "[Success] Binary created at $OUT_DIR/$BIN_NAME"
ls -lh "$OUT_DIR/$BIN_NAME"
