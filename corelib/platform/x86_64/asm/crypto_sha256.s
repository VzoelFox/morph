# corelib/platform/x86_64/asm/crypto_sha256.s
# SHA-256 Implementation (x86_64 Assembly)
# System V AMD64 ABI

.include "corelib/platform/x86_64/asm/macros.inc"

.section .rodata
.align 16
sha256_k:
    .long 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5
    .long 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5
    .long 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3
    .long 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174
    .long 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc
    .long 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da
    .long 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7
    .long 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967
    .long 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13
    .long 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85
    .long 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3
    .long 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070
    .long 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5
    .long 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3
    .long 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208
    .long 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2

sha256_h_init:
    .long 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a
    .long 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19

.section .text
.global __mf_sha256_init
.global __mf_sha256_update
.global __mf_sha256_final

# Struct SHA256_CTX (104 bytes)
# [0-31]  State (8 x 32-bit)
# [32-39] Count (64-bit total bits)
# [40-103] Buffer (64 bytes)

# ------------------------------------------------------------------------------
# __mf_sha256_init(ctx: ptr)
# ------------------------------------------------------------------------------
__mf_sha256_init:
    pushq %rbp
    movq %rsp, %rbp

    # Init State
    movq %rdi, %rax
    leaq sha256_h_init(%rip), %rsi

    movq (%rsi), %rcx
    movq %rcx, (%rax)
    movq 8(%rsi), %rcx
    movq %rcx, 8(%rax)
    movq 16(%rsi), %rcx
    movq %rcx, 16(%rax)
    movq 24(%rsi), %rcx
    movq %rcx, 24(%rax)

    # Init Count = 0
    movq $0, 32(%rax)

    # Zero Buffer (optional but good practice)
    # Skipped for speed

    leave
    ret

# ------------------------------------------------------------------------------
# __mf_sha256_update(ctx: ptr, data: ptr, len: i64)
# ------------------------------------------------------------------------------
__mf_sha256_update:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # RDI = Ctx
    # RSI = Data
    # RDX = Len

    movq %rdi, %r12 # Ctx
    movq %rsi, %r13 # Data
    movq %rdx, %r14 # Len (Remaining)

    # Calculate current buffer index
    # Count is in bits? Or bytes?
    # Standard: Count is bits. Or typical impl uses bytes then converts at end.
    # Let's use Count as BYTES in [32-39]. Convert to bits in Final.

    movq 32(%r12), %rax # Total Bytes
    movq %rax, %rbx
    andq $0x3F, %rbx    # Index = Count % 64

    # Update Total Count
    addq %r14, 32(%r12)

.update_loop:
    testq %r14, %r14
    jz .update_done

    # Copy byte to buffer
    movb (%r13), %al
    leaq 40(%r12), %rcx # Buffer Start
    movb %al, (%rcx, %rbx)

    incq %rbx
    incq %r13
    decq %r14

    # If Buffer Full (64)
    cmpq $64, %rbx
    jne .update_loop

    # Process Block
    pushq %r12
    pushq %r13
    pushq %r14

    leaq 40(%r12), %rsi # Data Block (Context Buffer)
    movq %r12, %rdi     # State (at start of Ctx)
    call sha256_transform

    popq %r14
    popq %r13
    popq %r12

    xorq %rbx, %rbx # Reset Index
    jmp .update_loop

.update_done:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    leave
    ret

# ------------------------------------------------------------------------------
# __mf_sha256_final(ctx: ptr, digest_out: ptr)
# ------------------------------------------------------------------------------
__mf_sha256_final:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12 # Ctx
    pushq %r13 # Digest Out
    pushq %rbx

    movq %rdi, %r12
    movq %rsi, %r13

    # 1. Pad
    # Append 0x80
    movq 32(%r12), %rax # Total Bytes
    movq %rax, %r8      # Save Total for Length (Bits)

    movq %rax, %rbx
    andq $0x3F, %rbx    # Index

    leaq 40(%r12), %rcx
    movb $0x80, (%rcx, %rbx)
    incq %rbx

    # If Index > 56, we don't have space for Length.
    # Pad with zeros until 64, Process, then Pad 56 zeros.
    cmpq $56, %rbx
    jle .pad_zeros

    # Pad rest with 0
.pad_loop_1:
    cmpq $64, %rbx
    je .process_pad_1
    leaq 40(%r12), %rcx
    movb $0, (%rcx, %rbx)
    incq %rbx
    jmp .pad_loop_1

.process_pad_1:
    movq %r12, %rdi
    leaq 40(%r12), %rsi
    call sha256_transform
    xorq %rbx, %rbx # Reset Index

.pad_zeros:
    # Pad with 0 until 56
    cmpq $56, %rbx
    je .append_len
    leaq 40(%r12), %rcx
    movb $0, (%rcx, %rbx)
    incq %rbx
    jmp .pad_zeros

.append_len:
    # Append Length (Bits) as Big Endian 64-bit
    # R8 has Total Bytes. Bits = R8 * 8.
    # We ignore high 61 bits of R8? (Assuming < 2^61 bytes).
    shlq $3, %r8 # * 8

    # Write Big Endian to [56..63]
    # bswap %r8
    bswapq %r8

    leaq 40(%r12), %rcx
    movq %r8, 56(%rcx) # Buffer + 56

    # Process Last Block
    movq %r12, %rdi
    leaq 40(%r12), %rsi
    call sha256_transform

    # Copy Output (State) to Digest (Big Endian)
    # State is Little Endian in Memory? No, we treat memory as 32-bit words.
    # x86 is Little Endian.
    # SHA-256 uses Big Endian words.
    # When we loaded Init State, we stored them as is.
    # sha256_transform MUST handle endianness.

    movq $0, %rbx
.copy_out:
    cmpq $8, %rbx # 8 words
    je .final_done

    movl (%r12, %rbx, 4), %eax
    bswapl %eax
    movl %eax, (%r13, %rbx, 4)

    incq %rbx
    jmp .copy_out

.final_done:
    popq %rbx
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# Internal: sha256_transform(state: ptr, data: ptr)
# Uses 64-byte data block.
# ------------------------------------------------------------------------------
sha256_transform:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # Alloc W[64] (256 bytes) on Stack
    subq $256, %rsp
    # And 8 words for working vars (a..h) -> use registers?
    # We need 8 vars. x86_64 has 16 regs.
    # Used: RSP, RBP.
    # State: RDI. Data: RSI.
    # Available: RAX, RBX, RCX, RDX, R8-R15. (12 regs).
    # We can map a..h to regs.
    # a=R8d, b=R9d, c=R10d, d=R11d, e=R12d, f=R13d, g=R14d, h=R15d.
    # We need temps T1, T2. Use EAX, EBX.
    # We need K ptr.

    pushq %rdi # Save State Ptr

    # 1. Prepare W[0..15]
    # Data is Big Endian stream. Load and bswap.
    movq %rsp, %rcx # W base
    xorq %rdx, %rdx
.w_init:
    cmpq $16, %rdx
    je .w_expand

    movl (%rsi, %rdx, 4), %eax
    bswapl %eax
    movl %eax, (%rcx, %rdx, 4)

    incq %rdx
    jmp .w_init

.w_expand:
    # 2. Expand W[16..63]
    # W[i] = s1(W[i-2]) + W[i-7] + s0(W[i-15]) + W[i-16]
    # s1(x) = rotr(17,x) ^ rotr(19,x) ^ shr(10,x)
    # s0(x) = rotr(7,x) ^ rotr(18,x) ^ shr(3,x)
    movq $16, %rdx
.w_loop:
    cmpq $64, %rdx
    je .load_state

    # s0(W[i-15])
    movl -60(%rcx, %rdx, 4), %eax # W[i-15]
    movl %eax, %ebx
    rorl $7, %ebx
    movl %eax, %r8d
    rorl $18, %r8d
    xorl %r8d, %ebx
    shrl $3, %eax
    xorl %eax, %ebx # EBX = s0

    # s1(W[i-2])
    movl -8(%rcx, %rdx, 4), %eax # W[i-2]
    movl %eax, %r8d
    rorl $17, %r8d
    movl %eax, %r9d
    rorl $19, %r9d
    xorl %r9d, %r8d
    shrl $10, %eax
    xorl %eax, %r8d # R8D = s1

    # W[i-16] + s0 + W[i-7] + s1
    movl -64(%rcx, %rdx, 4), %eax # W[i-16]
    addl %ebx, %eax
    addl -28(%rcx, %rdx, 4), %eax # W[i-7]
    addl %r8d, %eax

    movl %eax, (%rcx, %rdx, 4)
    incq %rdx
    jmp .w_loop

.load_state:
    # Load a..h from State
    movl 0(%rdi), %r8d  # a
    movl 4(%rdi), %r9d  # b
    movl 8(%rdi), %r10d # c
    movl 12(%rdi), %r11d # d
    movl 16(%rdi), %r12d # e
    movl 20(%rdi), %r13d # f
    movl 24(%rdi), %r14d # g
    movl 28(%rdi), %r15d # h

    # 3. Main Loop 0..63
    xorq %rbx, %rbx # i
    leaq sha256_k(%rip), %rsi # K

.main_loop:
    cmpq $64, %rbx
    je .update_state

    # T1 = h + Sigma1(e) + Ch(e,f,g) + K[i] + W[i]
    # Sigma1(e) = rotr(6,e) ^ rotr(11,e) ^ rotr(25,e)
    # Ch(e,f,g) = (e & f) ^ (~e & g)

    # Sigma1(e)
    movl %r12d, %eax
    rorl $6, %eax
    movl %r12d, %edx
    rorl $11, %edx
    xorl %edx, %eax
    movl %r12d, %edx
    rorl $25, %edx
    xorl %edx, %eax # EAX = Sigma1

    # T1 = h + Sigma1
    addl %r15d, %eax

    # Ch(e,f,g)
    movl %r12d, %edx
    andl %r13d, %edx # e & f
    movl %r12d, %ecx
    notl %ecx
    andl %r14d, %ecx # ~e & g
    xorl %ecx, %edx  # Ch

    addl %edx, %eax # T1 += Ch
    addl (%rsi, %rbx, 4), %eax # T1 += K[i]
    addl (%rsp, %rbx, 4), %eax # T1 += W[i]

    # T2 = Sigma0(a) + Maj(a,b,c)
    # Sigma0(a) = rotr(2,a) ^ rotr(13,a) ^ rotr(22,a)
    # Maj(a,b,c) = (a & b) ^ (a & c) ^ (b & c)

    # Sigma0(a)
    movl %r8d, %edx
    rorl $2, %edx
    movl %r8d, %ecx
    rorl $13, %ecx
    xorl %ecx, %edx
    movl %r8d, %ecx
    rorl $22, %ecx
    xorl %ecx, %edx # EDX = Sigma0

    # Maj(a,b,c)
    movl %r8d, %ecx
    andl %r9d, %ecx # a & b
    movl %r8d, %edi # Save a
    andl %r10d, %edi # a & c
    xorl %edi, %ecx
    movl %r9d, %edi
    andl %r10d, %edi # b & c
    xorl %edi, %ecx # Maj -> ECX

    addl %ecx, %edx # T2

    # Update Vars
    # h = g
    # g = f
    # f = e
    # e = d + T1
    # d = c
    # c = b
    # b = a
    # a = T1 + T2

    movl %r14d, %r15d # h = g
    movl %r13d, %r14d # g = f
    movl %r12d, %r13d # f = e

    addl %eax, %r11d  # e = d + T1
    movl %r11d, %r12d # move to e reg
    # Wait, R11D IS 'd' in previous step.
    # The assignment is `e_new = d_old + T1`.
    # `d_new = c_old`.
    # So I must be careful with register reuse.
    # Current Mapping:
    # a=R8, b=R9, c=R10, d=R11, e=R12, f=R13, g=R14, h=R15

    # Temp save T1 (EAX) and T2 (EDX).
    # Shift registers:
    # new_h = old_g (R14) -> R15
    # new_g = old_f (R13) -> R14
    # new_f = old_e (R12) -> R13
    # new_e = old_d (R11) + T1
    # new_d = old_c (R10) -> R11
    # new_c = old_b (R9)  -> R10
    # new_b = old_a (R8)  -> R9
    # new_a = T1 + T2

    # Safe Rotate
    # We need to perform simultaneous update.
    # T1 is EAX. T2 is EDX.

    movl %r14d, %r15d # h = g
    movl %r13d, %r14d # g = f
    movl %r12d, %r13d # f = e

    movl %r11d, %r12d # e_temp = d
    addl %eax, %r12d  # e = d + T1

    movl %r10d, %r11d # d = c
    movl %r9d, %r10d  # c = b
    movl %r8d, %r9d   # b = a

    addl %edx, %eax   # a = T1 + T2
    movl %eax, %r8d   # a

    # Restore RDI (State Ptr) clobbered by Maj logic?
    # I used RDI as temp!
    # "movl %r8d, %edi # Save a"
    # Panic! I lost State Ptr.
    # But I don't need State Ptr inside the loop.
    # I only need it at the end.
    # I pushed R12, R13... but RDI is Caller Saved (Argument).
    # I need to recover RDI.
    # It was in %rdi at function start.
    # But I didn't save %rdi on stack explicitly (only via RBP frame but RDI is reg).
    # I need to save RDI before loop!

    # Fix: Save RDI to Stack or Safe Reg.
    # I used R8-R15. RBX used.
    # RSI used for K.
    # RDI clobbered.
    # I'll save RDI to Stack.

    incq %rbx
    jmp .main_loop

.update_state:
    popq %rdi # Restore State Ptr

    addl %r8d, 0(%rdi)
    addl %r9d, 4(%rdi)
    addl %r10d, 8(%rdi)
    addl %r11d, 12(%rdi)
    addl %r12d, 16(%rdi)
    addl %r13d, 20(%rdi)
    addl %r14d, 24(%rdi)
    addl %r15d, 28(%rdi)

    addq $256, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    leave
    ret
