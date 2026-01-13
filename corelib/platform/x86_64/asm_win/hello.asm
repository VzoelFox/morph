; corelib/platform/x86_64/asm_win/hello.asm
; Program Demo "Hello World" menggunakan Runtime MorphFox (Windows)

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    msg db "MorphFox Hidup! (Windows)", 13, 10, 0
    len equ $ - msg

section .text
global main
extern __sys_write

main:
    ; Arg 1: FD = 1 (stdout) - but __sys_write expects Handle or FD logic?
    ; Our __sys_write wrapper handles GetStdHandle(-11) if FD=1.
    mov rcx, 1

    ; Arg 2: Buffer
    lea rdx, [rel msg]

    ; Arg 3: Length
    mov r8, len

    sub rsp, 32
    call __sys_write
    add rsp, 32

    mov rax, 0
    ret
