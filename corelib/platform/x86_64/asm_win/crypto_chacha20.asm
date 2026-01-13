; corelib/platform/x86_64/asm_win/crypto_chacha20.asm
; ChaCha20 (x86_64 NASM)
; Windows x64 ABI

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .rdata
sigma:
    dd 0x61707865, 0x3320646e, 0x79622d32, 0x6b206574

section .text
global __mf_chacha20_block
global __mf_chacha20_xor_stream

; ------------------------------------------------------------------------------
; __mf_chacha20_block(key: rcx, nonce: rdx, counter: r8, out: r9)
; ------------------------------------------------------------------------------
__mf_chacha20_block:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15

    sub rsp, 128 ; 64 Working + 64 Init

    ; Init State at [rsp + 64]
    lea r10, [rel sigma]
    mov eax, [r10]
    mov [rsp + 64], eax
    mov eax, [r10 + 4]
    mov [rsp + 68], eax
    mov eax, [r10 + 8]
    mov [rsp + 72], eax
    mov eax, [r10 + 12]
    mov [rsp + 76], eax

    ; Key (RCX)
    mov r10, rcx
    xor rax, rax
.copy_key:
    cmp rax, 8
    je .init_cnt
    mov r11d, [r10 + rax*4]
    mov [rsp + 80 + rax*4], r11d
    inc rax
    jmp .copy_key

.init_cnt:
    mov [rsp + 112], r8d

    ; Nonce (RDX)
    mov r10, rdx
    mov eax, [r10]
    mov [rsp + 116], eax
    mov eax, [r10 + 4]
    mov [rsp + 120], eax
    mov eax, [r10 + 8]
    mov [rsp + 124], eax

    ; Copy to Working
    xor rax, rax
.copy_init:
    cmp rax, 16
    je .start_rounds
    mov r10d, [rsp + 64 + rax*4]
    mov [rsp + rax*4], r10d
    inc rax
    jmp .copy_init

.start_rounds:
    mov r8, 10

.round_loop:
    test r8, r8
    jz .add_state

    call .qr_0_4_8_12
    call .qr_1_5_9_13
    call .qr_2_6_10_14
    call .qr_3_7_11_15

    call .qr_0_5_10_15
    call .qr_1_6_11_12
    call .qr_2_7_8_13
    call .qr_3_4_9_14

    dec r8
    jmp .round_loop

.add_state:
    xor rax, rax
    ; R9 is Out
.final_add:
    cmp rax, 16
    je .block_done

    mov r10d, [rsp + rax*4]
    add r10d, [rsp + 64 + rax*4]
    mov [r9 + rax*4], r10d

    inc rax
    jmp .final_add

.block_done:
    add rsp, 128
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    leave
    ret

; Macros for QR
; Use r10d, r11d temps
%macro QR 4
    ; a=%1, b=%2, c=%3, d=%4
    mov r10d, [rsp + %1*4]
    add r10d, [rsp + %2*4]
    mov [rsp + %1*4], r10d

    mov r11d, [rsp + %4*4]
    xor r11d, r10d
    rol r11d, 16
    mov [rsp + %4*4], r11d

    mov r10d, [rsp + %3*4]
    add r10d, r11d
    mov [rsp + %3*4], r10d

    mov r11d, [rsp + %2*4]
    xor r11d, r10d
    rol r11d, 12
    mov [rsp + %2*4], r11d

    mov r10d, [rsp + %1*4]
    add r10d, r11d
    mov [rsp + %1*4], r10d

    mov r11d, [rsp + %4*4]
    xor r11d, r10d
    rol r11d, 8
    mov [rsp + %4*4], r11d

    mov r10d, [rsp + %3*4]
    add r10d, r11d
    mov [rsp + %3*4], r10d

    mov r11d, [rsp + %2*4]
    xor r11d, r10d
    rol r11d, 7
    mov [rsp + %2*4], r11d
%endmacro

.qr_0_4_8_12: QR 0, 4, 8, 12
    ret
.qr_1_5_9_13: QR 1, 5, 9, 13
    ret
.qr_2_6_10_14: QR 2, 6, 10, 14
    ret
.qr_3_7_11_15: QR 3, 7, 11, 15
    ret
.qr_0_5_10_15: QR 0, 5, 10, 15
    ret
.qr_1_6_11_12: QR 1, 6, 11, 12
    ret
.qr_2_7_8_13: QR 2, 7, 8, 13
    ret
.qr_3_4_9_14: QR 3, 4, 9, 14
    ret

; ------------------------------------------------------------------------------
; __mf_chacha20_xor_stream
; Args:
; RCX: Key
; RDX: Nonce
; R8:  Counter
; R9:  In
; Stack: Out, Len
; ------------------------------------------------------------------------------
__mf_chacha20_xor_stream:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15

    sub rsp, 64 ; Block Buf

    ; Load Args
    mov r12, rcx ; Key
    mov r13, rdx ; Nonce
    mov r14, r8  ; Counter
    mov r15, r9  ; In

    ; Out is at [rbp + 48] (Shadow 32 + Ret 8 + Push 5*8 = 80? No)
    ; Caller Pushed: Len, Out.
    ; Shadow Space (32) allocated by caller? Yes.
    ; So [rbp + 16 + 32]? No.
    ; Win64: Shadow (32) at [RSP+8].
    ; Args 5+ at [RSP + 32 + 8].
    ; Return Address at [RSP].
    ; Previous Frame RBP at [RSP-8].
    ; So Arg5 (Out) is at [rbp + 48].
    ; Arg6 (Len) is at [rbp + 56].

    mov rbx, [rbp + 48] ; Out
    mov r10, [rbp + 56] ; Len

.stream_loop:
    test r10, r10
    jz .stream_done

    ; Setup Call
    mov rcx, r12
    mov rdx, r13
    mov r8, r14
    lea r9, [rsp] ; Out Buffer

    ; Save Volatiles? R10, R11.
    push r10
    sub rsp, 32 ; Shadow Space
    call __mf_chacha20_block
    add rsp, 32
    pop r10

    inc r14

    mov rcx, 64
    cmp r10, rcx
    cmovg rcx, r10 ; Min(64, Len)

    xor rax, rax
.xor_loop:
    cmp rax, rcx
    je .xor_done

    mov dl, [r15]
    xor dl, [rsp + rax]
    mov [rbx], dl

    inc r15
    inc rbx
    inc rax
    jmp .xor_loop

.xor_done:
    sub r10, rcx
    jmp .stream_loop

.stream_done:
    add rsp, 64
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    leave
    ret
