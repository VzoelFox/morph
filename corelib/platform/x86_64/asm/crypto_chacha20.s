# corelib/platform/x86_64/asm/crypto_chacha20.s
# ChaCha20 Implementation (x86_64 Assembly)
# System V AMD64 ABI

.include "corelib/platform/x86_64/asm/macros.inc"

.section .rodata
# "expand 32-byte k"
sigma:
    .long 0x61707865, 0x3320646e, 0x79622d32, 0x6b206574

.section .text
.global __mf_chacha20_block
.global __mf_chacha20_xor_stream

# ------------------------------------------------------------------------------
# __mf_chacha20_block(key: ptr, nonce: ptr, counter: i64, out: ptr)
# Output: 64 bytes keystream block
# Key: 32 bytes. Nonce: 12 bytes. Counter: 32-bit (passed as i64).
# ------------------------------------------------------------------------------
__mf_chacha20_block:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # RDI = Key
    # RSI = Nonce
    # RDX = Counter
    # RCX = Out

    subq $64, %rsp # State 16 x 32-bit

    # 1. Initialize State
    # 0-3: Sigma
    leaq sigma(%rip), %r8
    movl 0(%r8), %eax
    movl %eax, 0(%rsp)
    movl 4(%r8), %eax
    movl %eax, 4(%rsp)
    movl 8(%r8), %eax
    movl %eax, 8(%rsp)
    movl 12(%r8), %eax
    movl %eax, 12(%rsp)

    # 4-11: Key
    movl 0(%rdi), %eax; movl %eax, 16(%rsp)
    movl 4(%rdi), %eax; movl %eax, 20(%rsp)
    movl 8(%rdi), %eax; movl %eax, 24(%rsp)
    movl 12(%rdi), %eax; movl %eax, 28(%rsp)
    movl 16(%rdi), %eax; movl %eax, 32(%rsp)
    movl 20(%rdi), %eax; movl %eax, 36(%rsp)
    movl 24(%rdi), %eax; movl %eax, 40(%rsp)
    movl 28(%rdi), %eax; movl %eax, 44(%rsp)

    # 12: Counter
    movl %edx, 48(%rsp)

    # 13-15: Nonce
    movl 0(%rsi), %eax; movl %eax, 52(%rsp)
    movl 4(%rsi), %eax; movl %eax, 56(%rsp)
    movl 8(%rsi), %eax; movl %eax, 60(%rsp)

    # Save Initial State (We need to add it at the end)
    # But we can't fit another 64 bytes easily on registers.
    # We will reconstruct it or copy it.
    # Let's allocate Working State on stack too.
    # Total 128 bytes.
    # RSP points to Working State.
    # RSP+64 points to Initial State.

    subq $64, %rsp
    # Copy Init to Working
    # 16 dwords
    xorq %r8, %r8
.copy_init:
    cmpq $16, %r8
    je .start_rounds
    movl 64(%rsp, %r8, 4), %eax
    movl %eax, 0(%rsp, %r8, 4)
    incq %r8
    jmp .copy_init

.start_rounds:
    # 20 Rounds (10 Loops of 2 rounds)
    movq $10, %r8 # Loop Count

.round_loop:
    testq %r8, %r8
    jz .add_state

    # Column Round
    # QR(0, 4, 8, 12)
    call .quarter_round_0_4_8_12
    # QR(1, 5, 9, 13)
    call .quarter_round_1_5_9_13
    # QR(2, 6, 10, 14)
    call .quarter_round_2_6_10_14
    # QR(3, 7, 11, 15)
    call .quarter_round_3_7_11_15

    # Diagonal Round
    # QR(0, 5, 10, 15)
    call .quarter_round_0_5_10_15
    # QR(1, 6, 11, 12)
    call .quarter_round_1_6_11_12
    # QR(2, 7, 8, 13)
    call .quarter_round_2_7_8_13
    # QR(3, 4, 9, 14)
    call .quarter_round_3_4_9_14

    decq %r8
    jmp .round_loop

.add_state:
    # Add Initial State to Working State
    # And write to Output (RCX)
    xorq %r8, %r8
.final_add:
    cmpq $16, %r8
    je .block_done

    movl 0(%rsp, %r8, 4), %eax
    addl 64(%rsp, %r8, 4), %eax # Add Init
    movl %eax, (%rcx, %r8, 4)

    incq %r8
    jmp .final_add

.block_done:
    addq $128, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    leave
    ret

# --- Helpers ---
# Quarter Round Macro? No, assembly macro?
# Macro QR a, b, c, d
# a += b; d ^= a; d <<<= 16
# c += d; b ^= c; b <<<= 12
# a += b; d ^= a; d <<<= 8
# c += d; b ^= c; b <<<= 7

# We use indices.
# Access stack: 0(%rsp) ...
# Registers are scarce. We load/store.

# Defines
.macro QR a, b, c, d
    movl \a(%rsp), %eax
    addl \b(%rsp), %eax
    movl %eax, \a(%rsp)

    movl \d(%rsp), %ebx
    xorl %eax, %ebx
    roll $16, %ebx
    movl %ebx, \d(%rsp)

    movl \c(%rsp), %eax
    addl %ebx, %eax
    movl %eax, \c(%rsp)

    movl \b(%rsp), %ebx
    xorl %eax, %ebx
    roll $12, %ebx
    movl %ebx, \b(%rsp)

    movl \a(%rsp), %eax
    addl %ebx, %eax
    movl %eax, \a(%rsp)

    movl \d(%rsp), %ebx
    xorl %eax, %ebx
    roll $8, %ebx
    movl %ebx, \d(%rsp)

    movl \c(%rsp), %eax
    addl %ebx, %eax
    movl %eax, \c(%rsp)

    movl \b(%rsp), %ebx
    xorl %eax, %ebx
    roll $7, %ebx
    movl %ebx, \b(%rsp)
.endm

.quarter_round_0_4_8_12:
    QR 0, 16, 32, 48
    ret
.quarter_round_1_5_9_13:
    QR 4, 20, 36, 52
    ret
.quarter_round_2_6_10_14:
    QR 8, 24, 40, 56
    ret
.quarter_round_3_7_11_15:
    QR 12, 28, 44, 60
    ret
.quarter_round_0_5_10_15:
    QR 0, 20, 40, 60
    ret
.quarter_round_1_6_11_12:
    QR 4, 24, 44, 48
    ret
.quarter_round_2_7_8_13:
    QR 8, 28, 32, 52
    ret
.quarter_round_3_4_9_14:
    QR 12, 16, 36, 56
    ret

# ------------------------------------------------------------------------------
# __mf_chacha20_xor_stream(key: ptr, nonce: ptr, counter: i64, input: ptr, output: ptr, len: i64)
# ------------------------------------------------------------------------------
__mf_chacha20_xor_stream:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    subq $64, %rsp # KeyStream Block Buffer

    # Args:
    # RDI: Key
    # RSI: Nonce
    # RDX: Counter
    # RCX: Input
    # R8:  Output
    # R9:  Len

    movq %rdi, %r12 # Key
    movq %rsi, %r13 # Nonce
    movq %rdx, %r14 # Counter
    movq %rcx, %r15 # Input
    movq %r8,  %rbx # Output
    movq %r9,  %r10 # Remaining Len

.stream_loop:
    testq %r10, %r10
    jz .stream_done

    # Generate Block
    movq %r12, %rdi
    movq %r13, %rsi
    movq %r14, %rdx
    movq %rsp, %rcx # Out to Stack Buffer

    # Save Regs that might be clobbered?
    # __mf_chacha20_block uses stack vars but restores callee-saved.
    # R10 is caller-saved! Must save.
    # RBX, R12-R15 are callee-saved (preserved).
    pushq %r10
    call __mf_chacha20_block
    popq %r10

    incq %r14 # Increment Counter

    # XOR Input with KeyStream
    # Process min(64, Len)
    movq $64, %rcx
    cmpq %r10, %rcx
    cmovg %r10, %rcx # RCX = min(64, Len)

    xorq %rax, %rax
.xor_loop:
    cmpq %rcx, %rax
    je .xor_done

    movb (%r15), %dl
    xorb (%rsp, %rax, 1), %dl
    movb %dl, (%rbx)

    incq %r15
    incq %rbx
    incq %rax
    jmp .xor_loop

.xor_done:
    subq %rcx, %r10
    jmp .stream_loop

.stream_done:
    addq $64, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    leave
    ret
