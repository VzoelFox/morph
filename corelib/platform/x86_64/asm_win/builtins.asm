; corelib/platform/x86_64/asm_win/builtins.asm
; Implementasi Builtins untuk Windows x86_64
; Menyediakan fungsi-fungsi dasar yang didefinisikan di core/builtins.fox

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .text
global __mf_print_str
global __mf_print_asciz
global __mf_print_int
global __mf_print_int_raw
global __mf_memcpy

extern GetStdHandle

; ------------------------------------------------------------------------------
; func __mf_memcpy(dest: ptr, src: ptr, size: i64) -> ptr
; Input: RCX = dest, RDX = src, R8 = size
; Output: RAX = dest
; ------------------------------------------------------------------------------
__mf_memcpy:
    push rdi
    push rsi

    mov rax, rcx        ; Save dest to RAX

    ; Setup for rep movsb
    mov rdi, rcx        ; RDI = dest
    mov rsi, rdx        ; RSI = src
    mov rcx, r8         ; RCX = size (Counter)

    cld
    rep movsb

    pop rsi
    pop rdi
    ret

; ------------------------------------------------------------------------------
; func __mf_print_str(ptr: ptr, len: i64)
; Input: RCX = ptr, RDX = len
; Output: None
; ------------------------------------------------------------------------------
__mf_print_str:
    push rbp
    mov rbp, rsp

    ; Stack alignment (16 byte boundary)
    ; Masuk: +8 (RetAddr) +8 (Push RBP) = 16 (Aligned)

    ; Save volatile regs (Arguments)
    push rcx ; Buffer
    push rdx ; Length
    sub rsp, 32 ; Shadow Space for GetStdHandle

    ; 1. Get StdOut Handle
    mov rcx, STD_OUTPUT_HANDLE ; -11
    call GetStdHandle
    ; RAX = Handle

    add rsp, 32
    pop r8  ; Restore Length -> R8 (Arg 3 for OS_WRITE logic)
    pop rdx ; Restore Buffer -> RDX (Arg 2)
    mov rcx, rax ; Handle -> RCX (Arg 1)

    ; 2. Call OS_WRITE
    ; OS_WRITE handle, buffer, length
    OS_WRITE rcx, rdx, r8

    leave
    ret

; ------------------------------------------------------------------------------
; func __mf_print_asciz(ptr: ptr)
; Input: RCX = ptr (null-terminated)
; Output: None
; ------------------------------------------------------------------------------
__mf_print_asciz:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 32

    mov rbx, rcx ; Save Ptr

    ; Calculate length
    xor rdx, rdx ; Len
.asciz_len:
    mov al, byte [rbx + rdx]
    test al, al
    jz .asciz_print
    inc rdx
    jmp .asciz_len

.asciz_print:
    ; RCX is already ptr (if we restore it, or use RBX)
    mov rcx, rbx
    ; RDX is len
    call __mf_print_str

    add rsp, 32
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; MATH SAFETY (CHECKED)
; ------------------------------------------------------------------------------

; func __mf_add_checked(a: i64, b: i64) -> (val: i64, err: i64)
global __mf_add_checked
__mf_add_checked:
    mov rax, rcx
    add rax, rdx
    jo .math_err
    xor rdx, rdx
    ret

; func __mf_sub_checked(a: i64, b: i64) -> (val: i64, err: i64)
global __mf_sub_checked
__mf_sub_checked:
    mov rax, rcx
    sub rax, rdx
    jo .math_err
    xor rdx, rdx
    ret

; func __mf_mul_checked(a: i64, b: i64) -> (val: i64, err: i64)
global __mf_mul_checked
__mf_mul_checked:
    mov rax, rcx
    imul rax, rdx
    jo .math_err
    xor rdx, rdx
    ret

; func __mf_div_checked(a: i64, b: i64) -> (val: i64, err: i64)
global __mf_div_checked
__mf_div_checked:
    test rdx, rdx
    jz .math_err

    mov r8, rdx         ; Save divisor
    mov rax, rcx        ; Dividend

    ; Check Overflow (INT_MIN / -1)
    mov r9, 0x8000000000000000
    cmp rax, r9
    jne .do_div
    cmp r8, -1
    je .math_err

.do_div:
    cqo
    idiv r8
    xor rdx, rdx
    ret

.math_err:
    xor rax, rax
    mov rdx, 1
    ret

; ------------------------------------------------------------------------------
; COMPLEX MATH
; ------------------------------------------------------------------------------

global __mf_abs
__mf_abs:
    mov rax, rcx
    cqo
    xor rax, rdx
    sub rax, rdx
    ret

global __mf_sqrt
__mf_sqrt:
    test rcx, rcx
    js .math_err

    mov r8, 0                   ; Low
    mov r9, 3037000500          ; High

    cmp rcx, r9
    cmovl r9, rcx

    xor rax, rax

.sqrt_loop:
    cmp r8, r9
    jg .sqrt_done

    mov r10, r8
    add r10, r9
    shr r10, 1          ; Mid

    mov r11, r10
    imul r11, r10       ; Mid*Mid

    cmp r11, rcx
    je .sqrt_exact
    jg .sqrt_less

    mov rax, r10
    inc r10
    mov r8, r10
    jmp .sqrt_loop

.sqrt_less:
    dec r10
    mov r9, r10
    jmp .sqrt_loop

.sqrt_exact:
    mov rax, r10
    jmp .sqrt_done

.sqrt_done:
    xor rdx, rdx
    ret

global __mf_pow
__mf_pow:
    cmp rdx, 0          ; Exp
    jl .math_err
    je .pow_one

    mov r8, rcx         ; Base
    mov r9, rdx         ; Exp
    mov rax, 1

.pow_loop:
    test r9, r9
    jz .pow_done

    test r9, 1
    jz .pow_square

    imul rax, r8
    jo .math_err

.pow_square:
    shr r9, 1
    test r9, r9
    jz .pow_done

    imul r8, r8
    jo .math_err

    jmp .pow_loop

.pow_one:
    mov rax, 1
    xor rdx, rdx
    ret

.pow_done:
    xor rdx, rdx
    ret

__mf_print_int:
    push rbp
    mov rbp, rsp
    mov r11, 1 ; Flag Newline
    jmp .L_print_common

__mf_print_int_raw:
    push rbp
    mov rbp, rsp
    xor r11, r11 ; Flag No Newline
    jmp .L_print_common

.L_print_common:
    sub rsp, 64
    mov rax, rcx
    mov r10, rcx ; original val

    lea r9, [rsp + 63] ; End of buffer
    xor r8, r8 ; Count

    test r11, r11
    jz .L_no_nl
    mov byte [r9], 10
    dec r9
    inc r8
.L_no_nl:

    ; Handle 0
    test rax, rax
    jnz .L_chk_neg
    mov byte [r9], '0'
    dec r9
    inc r8
    jmp .L_print

.L_chk_neg:
    cmp rax, 0
    jge .L_conv_loop_start
    neg rax

.L_conv_loop_start:
    xor rdx, rdx
    mov rcx, 10
    div rcx
    add dl, '0'
    mov byte [r9], dl
    dec r9
    inc r8
    test rax, rax
    jnz .L_conv_loop_start

    cmp r10, 0
    jge .L_print
    mov byte [r9], '-'
    dec r9
    inc r8

.L_print:
    inc r9
    mov rcx, r9
    mov rdx, r8
    call __mf_print_str
    leave
    ret

; Old logic
# .L_convert_loop:
    xor rdx, rdx
    mov rcx, 10
    div rcx             ; rax / 10

    add dl, '0'         ; Convert remainder to ASCII
    mov byte [r9], dl   ; Store digit
    dec r9
    inc r8

    test rax, rax
    jnz .L_convert_loop

    ; Cek tanda minus
    cmp r10, 0
    jge .L_print
    mov byte [r9], '-'
    dec r9
    inc r8

.L_print:
    inc r9              ; Adjust pointer (karena dec terakhir)

    ; Call __mf_print_str(ptr, len)
    ; Input: RCX = ptr, RDX = len

    mov rcx, r9
    mov rdx, r8
    call __mf_print_str

    leave
    ret

; ==============================================================================
; MEMORY BUILTINS - Intent Tree Support (Windows)
; ==============================================================================

extern mem_alloc
extern mem_free

global __mf_mem_alloc
__mf_mem_alloc:
    jmp mem_alloc

global __mf_mem_free
__mf_mem_free:
    jmp mem_free

global __mf_load_i64
__mf_load_i64:
    mov rax, [rcx]
    ret

global __mf_poke_i64
__mf_poke_i64:
    mov [rcx], rdx
    ret

global __mf_load_byte
__mf_load_byte:
    xor rax, rax
    mov al, [rcx]
    ret

global __mf_poke_byte
__mf_poke_byte:
    mov [rcx], dl
    ret
