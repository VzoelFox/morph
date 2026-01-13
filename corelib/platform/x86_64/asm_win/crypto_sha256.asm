; corelib/platform/x86_64/asm_win/crypto_sha256.asm
; SHA-256 Implementation (x86_64 NASM)
; Windows x64 ABI

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .rdata
align 16
sha256_k:
    dd 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5
    dd 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5
    dd 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3
    dd 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174
    dd 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc
    dd 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da
    dd 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7
    dd 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967
    dd 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13
    dd 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85
    dd 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3
    dd 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070
    dd 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5
    dd 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3
    dd 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208
    dd 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2

sha256_h_init:
    dd 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a
    dd 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19

section .text
global __mf_sha256_init
global __mf_sha256_update
global __mf_sha256_final

; ------------------------------------------------------------------------------
; __mf_sha256_init(ctx: rcx)
; ------------------------------------------------------------------------------
__mf_sha256_init:
    push rbp
    mov rbp, rsp

    ; Init State
    mov rax, rcx ; ctx
    lea rsi, [rel sha256_h_init]

    mov rcx, [rsi]
    mov [rax], rcx
    mov rcx, [rsi + 8]
    mov [rax + 8], rcx
    mov rcx, [rsi + 16]
    mov [rax + 16], rcx
    mov rcx, [rsi + 24]
    mov [rax + 24], rcx

    ; Init Count = 0 (Offset 32)
    mov qword [rax + 32], 0

    leave
    ret

; ------------------------------------------------------------------------------
; __mf_sha256_update(ctx: rcx, data: rdx, len: r8)
; ------------------------------------------------------------------------------
__mf_sha256_update:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 40 ; Shadow Space + Alignment

    mov r12, rcx ; Ctx
    mov r13, rdx ; Data
    mov r14, r8  ; Len

    ; Index = Count % 64
    mov rax, [r12 + 32]
    mov rbx, rax
    and rbx, 0x3F

    ; Update Count
    add [r12 + 32], r14

.update_loop:
    test r14, r14
    jz .update_done

    mov al, [r13]
    lea rcx, [r12 + 40] ; Buffer
    mov [rcx + rbx], al

    inc rbx
    inc r13
    dec r14

    cmp rbx, 64
    jne .update_loop

    ; Process Block
    ; Call sha256_transform(state: rcx, data: rdx)
    lea rdx, [r12 + 40] ; Buffer
    mov rcx, r12        ; State
    call sha256_transform

    xor rbx, rbx
    jmp .update_loop

.update_done:
    add rsp, 40
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    leave
    ret

; ------------------------------------------------------------------------------
; __mf_sha256_final(ctx: rcx, digest_out: rdx)
; ------------------------------------------------------------------------------
__mf_sha256_final:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 40

    mov r12, rcx
    mov r13, rdx

    ; Pad 0x80
    mov rax, [r12 + 32] ; Count
    mov r8, rax         ; Save for Length bits

    mov rbx, rax
    and rbx, 0x3F

    lea rcx, [r12 + 40]
    mov byte [rcx + rbx], 0x80
    inc rbx

    cmp rbx, 56
    jle .pad_zeros

.pad_loop_1:
    cmp rbx, 64
    je .process_pad_1
    lea rcx, [r12 + 40]
    mov byte [rcx + rbx], 0
    inc rbx
    jmp .pad_loop_1

.process_pad_1:
    mov rcx, r12
    lea rdx, [r12 + 40]
    call sha256_transform
    xor rbx, rbx

.pad_zeros:
    cmp rbx, 56
    je .append_len
    lea rcx, [r12 + 40]
    mov byte [rcx + rbx], 0
    inc rbx
    jmp .pad_zeros

.append_len:
    shl r8, 3 ; Bits
    bswap r8  ; Big Endian

    lea rcx, [r12 + 40]
    mov [rcx + 56], r8

    mov rcx, r12
    lea rdx, [r12 + 40]
    call sha256_transform

    ; Copy Output
    xor rbx, rbx
.copy_out:
    cmp rbx, 8
    je .final_done

    mov eax, [r12 + rbx*4]
    bswap eax
    mov [r13 + rbx*4], eax

    inc rbx
    jmp .copy_out

.final_done:
    add rsp, 40
    pop r13
    pop r12
    pop rbx
    leave
    ret

; ------------------------------------------------------------------------------
; Internal: sha256_transform(state: rcx, data: rdx)
; ------------------------------------------------------------------------------
sha256_transform:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    push rdi
    push rsi

    ; Save State Ptr (RCX) - Windows ABI uses RCX for Arg1
    ; But we use RCX inside logic?
    ; In Linux logic, I used R8-R15 for vars.
    ; I used Stack for W[64].
    ; RCX was used as W base in initialization.

    push rcx ; Save State Ptr

    sub rsp, 256 ; W[64]

    mov rsi, rdx ; Data Ptr

    ; 1. W Init
    mov rcx, rsp ; W Base
    xor rdx, rdx
.w_init:
    cmp rdx, 16
    je .w_expand

    mov eax, [rsi + rdx*4]
    bswap eax
    mov [rcx + rdx*4], eax
    inc rdx
    jmp .w_init

.w_expand:
    mov rdx, 16
.w_loop:
    cmp rdx, 64
    je .load_state

    ; s0
    mov eax, [rcx + rdx*4 - 60]
    mov ebx, eax
    ror ebx, 7
    mov r8d, eax
    ror r8d, 18
    xor ebx, r8d
    shr eax, 3
    xor ebx, eax ; s0

    ; s1
    mov eax, [rcx + rdx*4 - 8]
    mov r8d, eax
    ror r8d, 17
    mov r9d, eax
    ror r9d, 19
    xor r8d, r9d
    shr eax, 10
    xor r8d, eax ; s1

    ; Sum
    mov eax, [rcx + rdx*4 - 64]
    add eax, ebx
    add eax, [rcx + rdx*4 - 28]
    add eax, r8d

    mov [rcx + rdx*4], eax
    inc rdx
    jmp .w_loop

.load_state:
    ; Restore State Ptr from Stack to access it
    mov rdi, [rsp + 256] ; Pushed before W alloc

    mov r8d, [rdi]
    mov r9d, [rdi + 4]
    mov r10d, [rdi + 8]
    mov r11d, [rdi + 12]
    mov r12d, [rdi + 16]
    mov r13d, [rdi + 20]
    mov r14d, [rdi + 24]
    mov r15d, [rdi + 28]

    xor rbx, rbx ; i
    lea rsi, [rel sha256_k]

.main_loop:
    cmp rbx, 64
    je .update_state

    ; Sigma1(e)
    mov eax, r12d
    ror eax, 6
    mov edx, r12d
    ror edx, 11
    xor eax, edx
    mov edx, r12d
    ror edx, 25
    xor eax, edx ; Sigma1

    add eax, r15d ; + h

    ; Ch
    mov edx, r12d
    and edx, r13d
    mov ecx, r12d
    not ecx
    and ecx, r14d
    xor edx, ecx

    add eax, edx
    add eax, [rsi + rbx*4]
    add eax, [rsp + rbx*4] ; W[i]

    ; T2
    mov edx, r8d
    ror edx, 2
    mov ecx, r8d
    ror ecx, 13
    xor edx, ecx
    mov ecx, r8d
    ror ecx, 22
    xor edx, ecx ; Sigma0

    mov ecx, r8d
    and ecx, r9d
    mov edi, r8d
    and edi, r10d
    xor ecx, edi
    mov edi, r9d
    and edi, r10d
    xor ecx, edi ; Maj

    add edx, ecx ; T2

    ; Shift
    mov r15d, r14d
    mov r14d, r13d
    mov r13d, r12d

    mov r12d, r11d
    add r12d, eax ; e = d + T1

    mov r11d, r10d
    mov r10d, r9d
    mov r9d, r8d

    add eax, edx
    mov r8d, eax ; a

    inc rbx
    jmp .main_loop

.update_state:
    mov rdi, [rsp + 256] ; State Ptr

    add [rdi], r8d
    add [rdi + 4], r9d
    add [rdi + 8], r10d
    add [rdi + 12], r11d
    add [rdi + 16], r12d
    add [rdi + 20], r13d
    add [rdi + 24], r14d
    add [rdi + 28], r15d

    add rsp, 256
    pop rcx ; Pop State Ptr

    pop rsi
    pop rdi
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    leave
    ret
