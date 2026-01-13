# corelib/platform/x86_64/asm/builtins.s
# Implementasi Builtins untuk Linux x86_64
# Menyediakan fungsi-fungsi dasar yang didefinisikan di core/builtins.fox

.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global __mf_print_str
.global __mf_print_asciz
.global __mf_print_int
.global __mf_print_int_raw
.global __mf_memcpy

# ------------------------------------------------------------------------------
# func __mf_memcpy(dest: ptr, src: ptr, size: i64) -> ptr
# Input: %rdi = dest, %rsi = src, %rdx = size
# Output: %rax = dest
# ------------------------------------------------------------------------------
__mf_memcpy:
    movq %rdi, %rax     # Save return value (dest)
    movq %rdx, %rcx     # Counter = size

    # Rep movsb (Repeat Move String Byte)
    # Mencopy RCX bytes dari RSI (src) ke RDI (dest)
    cld                 # Clear Direction Flag (Forward)
    rep movsb

    ret

# Crypto
.global __mf_sha256_init
.global __mf_sha256_update
.global __mf_sha256_final
.global __mf_chacha20_block
.global __mf_chacha20_xor_stream

# ------------------------------------------------------------------------------
# func __mf_print_str(ptr: ptr, len: i64)
# Input: %rdi = ptr, %rsi = len
# Output: None
# ------------------------------------------------------------------------------
__mf_print_str:
    # OS_WRITE macro: fd, buffer, length
    movq %rsi, %rdx     # len ke rdx
    movq %rdi, %rsi     # buf ke rsi
    movq $1, %rdi       # fd = 1 ke rdi

    OS_WRITE %rdi, (%rsi), %rdx
    ret

# ------------------------------------------------------------------------------
# func __mf_print_asciz(ptr: ptr)
# Input: %rdi = ptr (null-terminated)
# Output: None
# ------------------------------------------------------------------------------
__mf_print_asciz:
    pushq %rbp
    movq %rsp, %rbp

    # Calculate length
    movq %rdi, %rax
    xorq %rcx, %rcx
.asciz_len:
    movb (%rax), %dl
    testb %dl, %dl
    jz .asciz_print
    incq %rax
    incq %rcx
    jmp .asciz_len

.asciz_print:
    movq %rcx, %rsi # len
    # rdi is already ptr
    call __mf_print_str

    leave
    ret

# ------------------------------------------------------------------------------
# func __mf_print_int(val: i64)
# Input: %rdi = val
# Output: None
# Deskripsi: Mengubah integer menjadi string desimal dan mencetaknya ke stdout
#            menggunakan buffer di stack. Menangani nilai negatif.
# ------------------------------------------------------------------------------
__mf_print_int:
    pushq %rbp
    movq %rsp, %rbp
    movq $1, %r10       # Flag: Newline
    jmp .L_print_int_common

__mf_print_int_raw:
    pushq %rbp
    movq %rsp, %rbp
    xorq %r10, %r10     # Flag: No Newline
    jmp .L_print_int_common

.L_print_int_common:
    # Alokasi buffer di stack
    subq $32, %rsp

    movq %rdi, %rax     # val

    # Pointer akhir
    leaq 31(%rsp), %rsi

    xorq %rcx, %rcx

    # If newline needed
    testq %r10, %r10
    jz .L_no_nl
    movb $0x0A, (%rsi)
    decq %rsi
    incq %rcx
.L_no_nl:

    # Handle 0
    testq %rax, %rax
    jnz .L_chk_neg
    movb $'0', (%rsi)
    decq %rsi
    incq %rcx
    jmp .L_print_done

.L_chk_neg:
    cmpq $0, %rax
    jge .L_conv_loop_start
    negq %rax

.L_conv_loop_start:
    xorq %rdx, %rdx
    movq $10, %r8
    divq %r8
    addb $'0', %dl
    movb %dl, (%rsi)
    decq %rsi
    incq %rcx
    testq %rax, %rax
    jnz .L_conv_loop_start

    cmpq $0, %rdi
    jge .L_print_done
    movb $'-', (%rsi)
    decq %rsi
    incq %rcx

.L_print_done:
    incq %rsi
    movq %rsi, %rdi
    movq %rcx, %rsi
    call __mf_print_str
    leave
    ret

# ------------------------------------------------------------------------------
# MATH SAFETY (CHECKED ARITHMETIC)
# ------------------------------------------------------------------------------

# func __mf_add_checked(a: i64, b: i64) -> (val: i64, err: i64)
.global __mf_add_checked
__mf_add_checked:
    movq %rdi, %rax
    addq %rsi, %rax
    jo .math_err
    xorq %rdx, %rdx
    ret

# func __mf_sub_checked(a: i64, b: i64) -> (val: i64, err: i64)
.global __mf_sub_checked
__mf_sub_checked:
    movq %rdi, %rax
    subq %rsi, %rax
    jo .math_err
    xorq %rdx, %rdx
    ret

# func __mf_mul_checked(a: i64, b: i64) -> (val: i64, err: i64)
.global __mf_mul_checked
__mf_mul_checked:
    movq %rdi, %rax
    imulq %rsi
    jo .math_err
    xorq %rdx, %rdx
    ret

# func __mf_div_checked(a: i64, b: i64) -> (val: i64, err: i64)
.global __mf_div_checked
__mf_div_checked:
    testq %rsi, %rsi
    jz .math_err        # Div by zero

    movq %rdi, %rax
    cqo                 # Sign extend RAX -> RDX:RAX

    # Check Overflow (INT_MIN / -1)
    movabsq $0x8000000000000000, %rcx
    cmpq %rcx, %rdi
    jne .do_div
    cmpq $-1, %rsi
    je .math_err

.do_div:
    idivq %rsi
    xorq %rdx, %rdx     # Status OK
    ret

.math_err:
    xorq %rax, %rax
    movq $1, %rdx
    ret

# ------------------------------------------------------------------------------
# COMPLEX MATH
# ------------------------------------------------------------------------------

# func __mf_abs(val: i64) -> i64
.global __mf_abs
__mf_abs:
    movq %rdi, %rax
    cqo
    xorq %rdx, %rax
    subq %rdx, %rax
    ret

# func __mf_sqrt(val: i64) -> (res: i64, err: i64)
.global __mf_sqrt
__mf_sqrt:
    testq %rdi, %rdi
    js .math_err        # Negative

    movq $0, %r8        # Low
    movq $3037000500, %r9 # High

    cmpq %r9, %rdi
    cmovlq %rdi, %r9

    movq $0, %rax       # Ans

.sqrt_loop:
    cmpq %r9, %r8
    jg .sqrt_done

    movq %r8, %rcx
    addq %r9, %rcx
    shrq $1, %rcx       # Mid

    movq %rcx, %r10
    imulq %rcx, %r10    # Mid*Mid

    cmpq %rdi, %r10
    je .sqrt_exact
    jg .sqrt_less

    movq %rcx, %rax
    incq %rcx
    movq %rcx, %r8
    jmp .sqrt_loop

.sqrt_less:
    decq %rcx
    movq %rcx, %r9
    jmp .sqrt_loop

.sqrt_exact:
    movq %rcx, %rax
    jmp .sqrt_done

.sqrt_done:
    xorq %rdx, %rdx
    ret

# func __mf_pow(base: i64, exp: i64) -> (res: i64, err: i64)
.global __mf_pow
__mf_pow:
    cmpq $0, %rsi
    jl .math_err
    je .pow_one

    movq %rdi, %r8      # Base
    movq %rsi, %r9      # Exp
    movq $1, %rax       # Result

.pow_loop:
    testq %r9, %r9
    jz .pow_done

    testq $1, %r9
    jz .pow_square

    imulq %r8, %rax
    jo .math_err

.pow_square:
    shrq $1, %r9
    testq %r9, %r9
    jz .pow_done

    imulq %r8, %r8
    jo .math_err

    jmp .pow_loop

.pow_one:
    movq $1, %rax
    xorq %rdx, %rdx
    ret

.pow_done:
    xorq %rdx, %rdx
    ret

# ==============================================================================
# MEMORY BUILTINS - Intent Tree Support
# ==============================================================================

.extern mem_alloc
.extern mem_free

.global __mf_mem_alloc
__mf_mem_alloc:
    jmp mem_alloc

.global __mf_mem_free
__mf_mem_free:
    jmp mem_free

.global __mf_load_i64
__mf_load_i64:
    movq (%rdi), %rax
    ret

.global __mf_poke_i64
__mf_poke_i64:
    movq %rsi, (%rdi)
    ret

.global __mf_load_byte
__mf_load_byte:
    xorq %rax, %rax
    movb (%rdi), %al
    ret

.global __mf_poke_byte
__mf_poke_byte:
    movb %sil, (%rdi)
    ret
