#!/bin/bash
# script/build_linux.sh
# Build and Run Linux Assembly Tests (Executor & Pipeline)

set -e

ASM_DIR="corelib/platform/x86_64/asm"
OUT_DIR="build/linux"

echo "[Build] Setting up directories..."
mkdir -p "$OUT_DIR"

# List of Source Files (Core)
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
    "crypto_sha256.s"
    "crypto_chacha20.s"
)

OBJECTS=""

echo "[Build] Assembling Core Sources..."
for src in "${SOURCES[@]}"; do
    echo "  -> $src"
    as -g -o "$OUT_DIR/${src%.s}.o" "$ASM_DIR/$src"
    OBJECTS="$OBJECTS $OUT_DIR/${src%.s}.o"
done

echo "[Build] Assembling Tests..."
as -g -o "$OUT_DIR/test_pipeline_advanced.o" "$ASM_DIR/test_pipeline_advanced.s"

echo "[Build] Linking test_pipeline_advanced..."
ld -o "$OUT_DIR/test_pipeline_advanced" $OBJECTS "$OUT_DIR/test_pipeline_advanced.o"

echo "[Build] Assembling Graphics Test..."
as -g -o "$OUT_DIR/test_graphics.o" "$ASM_DIR/test_graphics.s"

echo "[Build] Linking Graphics Test..."
ld -o "$OUT_DIR/test_graphics" $OBJECTS "$OUT_DIR/test_graphics.o"

echo "[Build] Assembling Test DOM..."
as -g -o "$OUT_DIR/test_dom.o" "$ASM_DIR/test_dom.s"

echo "[Build] Linking Test DOM..."
ld -o "$OUT_DIR/test_dom" $OBJECTS "$OUT_DIR/test_dom.o"

echo "[Build] Assembling Morph CLI..."
as -g -o "tools/morph.o" "tools/morph.s"

echo "[Build] Linking Morph CLI..."
ld -o "tools/morph" tools/morph.o $OBJECTS

echo "--------------------------------------------------------"
echo "[Test 1] Running test_pipeline_advanced (Vars & Flow)..."
"$OUT_DIR/test_pipeline_advanced"
if [ $? -eq 0 ]; then
    echo "[Success] Advanced Pipeline passed."
else
    echo "[Failure] Advanced Pipeline failed."
    exit 1
fi

echo "--------------------------------------------------------"
echo "[Test 2] Running test_dom (Manual DOM Construction)..."
"$OUT_DIR/test_dom"
if [ $? -eq 0 ]; then
    echo "[Success] DOM Structure Verified."
else
    echo "[Failure] DOM Structure Invalid."
    exit 1
fi

echo "--------------------------------------------------------"
echo "[All Tests Passed]"
