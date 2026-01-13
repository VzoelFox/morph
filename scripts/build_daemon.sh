#!/bin/bash
# scripts/build_daemon.sh
# Build MorphFox Native Daemon Cleaner
# ==============================================================================

set -e

ASM_DIR="corelib/platform/x86_64/asm"
OUT_DIR="build/linux"

echo "[Build] Building MorphFox Daemon Cleaner..."
mkdir -p "$OUT_DIR"

# Assemble daemon components
echo "[Build] Assembling daemon_cleaner.s..."
as -g -o "$OUT_DIR/daemon_cleaner.o" "$ASM_DIR/daemon_cleaner.s"

echo "[Build] Assembling morph_daemon_main.s..."
as -g -o "$OUT_DIR/morph_daemon_main.o" "$ASM_DIR/morph_daemon_main.s"

# Link
echo "[Build] Linking morph_daemon..."
ld -o "$OUT_DIR/morph_daemon" "$OUT_DIR/morph_daemon_main.o" "$OUT_DIR/daemon_cleaner.o"

echo "[Build] Success! Binary: $OUT_DIR/morph_daemon"
echo ""
echo "Usage:"
echo "  $OUT_DIR/morph_daemon start   - Start daemon"
echo "  $OUT_DIR/morph_daemon stop    - Stop daemon"
echo "  $OUT_DIR/morph_daemon status  - Check status"
