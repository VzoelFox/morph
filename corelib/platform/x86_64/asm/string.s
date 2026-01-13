# corelib/platform/x86_64/asm/string.s
# Implementasi Operasi String (Linux x86_64)
# Sesuai SSOT corelib/core/structures.fox

.section .text
.global __mf_string_hash
.global __mf_string_equals

# ------------------------------------------------------------------------------
# func __mf_string_hash(ptr: ptr, len: i64) -> i64
# Algoritma: FNV-1a 64-bit
# Input:  %rdi = ptr, %rsi = len
# Output: %rax = hash
# ------------------------------------------------------------------------------
__mf_string_hash:
    # FNV_OFFSET_BASIS_64 = 14695981039346656037 (0xcbf29ce484222325)
    # FNV_PRIME_64        = 1099511628211        (0x100000001b3)

    movq $0xcbf29ce484222325, %rax  # rax = hash (init offset)
    movq $0x100000001b3, %r8        # r8  = prime

    # Loop setup
    # rdi = current char ptr
    # rsi = count

    testq %rsi, %rsi
    jz .hash_done

.hash_loop:
    # XOR hash with byte
    movzbq (%rdi), %rcx     # Ambil byte, zero extend
    xorq %rcx, %rax

    # Multiply by prime
    imulq %r8, %rax

    # Next char
    incq %rdi
    decq %rsi
    jnz .hash_loop

.hash_done:
    ret

# ------------------------------------------------------------------------------
# func __mf_string_equals(ptr1: ptr, len1: i64, ptr2: ptr, len2: i64) -> bool
# Input:  %rdi = ptr1, %rsi = len1, %rdx = ptr2, %rcx = len2
# Output: %rax = 1 (True) or 0 (False)
# ------------------------------------------------------------------------------
__mf_string_equals:
    # 1. Cek Panjang
    cmpq %rsi, %rcx
    jne .not_equal

    # 2. Cek Panjang 0 (Jika sama-sama 0, dianggap equal)
    testq %rsi, %rsi
    jz .is_equal

    # 3. Loop Compare
    # Gunakan register scratch
    pushq %rdi
    pushq %rsi
    pushq %rdx

    # rdi = ptr1, rdx = ptr2, rsi = count
    # bisa gunakan rep cmpsb, tapi manual loop lebih aman kontrolnya

.cmp_loop:
    movb (%rdi), %al
    cmpb (%rdx), %al
    jne .cmp_fail

    incq %rdi
    incq %rdx
    decq %rsi
    jnz .cmp_loop

    # Equal
    popq %rdx
    popq %rsi
    popq %rdi

.is_equal:
    movq $1, %rax
    ret

.cmp_fail:
    popq %rdx
    popq %rsi
    popq %rdi

.not_equal:
    xorq %rax, %rax
    ret
