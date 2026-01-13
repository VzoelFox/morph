# corelib/platform/x86_64/asm/context.s
# Implementasi Context Switching untuk MorphRoutine (Linux x64)
# ==============================================================================

.global morph_switch_context
.text

# ------------------------------------------------------------------------------
# func morph_switch_context(old_rsp_ptr: ptr, new_rsp: ptr)
# Input:
#   RDI = old_rsp_ptr (Pointer ke lokasi memori untuk menyimpan RSP lama)
#   RSI = new_rsp     (Nilai RSP baru yang akan dimuat)
#
# Tugas:
# 1. Push semua Callee-Saved Register ke Stack LAMA.
# 2. Simpan nilai RSP (setelah push) ke [RDI].
# 3. Ganti RSP dengan RSI (Stack BARU).
# 4. Pop semua Callee-Saved Register dari Stack BARU.
# 5. Return (ke alamat yang ada di Stack BARU).
# ------------------------------------------------------------------------------
morph_switch_context:
    # 1. Save Callee-Saved Registers (System V AMD64 ABI)
    # Non-Volatile: RBX, RBP, R12, R13, R14, R15
    # (RDI, RSI, RDX, RCX, R8, R9 are volatile/arguments)

    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # 2. Store Old RSP
    movq %rsp, (%rdi)

    # 3. Load New RSP
    movq %rsi, %rsp

    # 4. Restore Callee-Saved Registers (Order: Reverse of Push)
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx

    # 5. Return (Pop RIP from new stack)
    ret
