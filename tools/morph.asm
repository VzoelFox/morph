; tools/morph.asm
; The Native Morph Runner (Windows x86_64)
; Usage: morph.exe <file.fox|.morph>

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    msg_usage   db "Penggunaan: morph <file.fox|.morph>", 10, 0
    msg_start   db "Menjalankan ", 0
    msg_dot     db "...", 10, 0
    msg_err     db "Gagal.", 10, 0
    msg_inv_ext db "Ekstensi file tidak dikenali (.fox atau .morph)", 10, 0
    msg_bad_hdr db "Header .morph tidak valid (Magic/Versi salah)", 10, 0
    newline     db 10, 0

    ext_fox     db ".fox", 0
    ext_morph   db ".morph", 0

    header_magic db 0x56, 0x5A, 0x4F, 0x45, 0x4C, 0x46, 0x4F, 0x58
    header_ver   dq 1

section .bss
    file_buffer resb 1048576 ; 1MB
    manual_fragment resb 32
    intent_root resq 1

section .text
global main
extern lexer_create
extern parser_parse_unit
extern compile_ast_to_fragment
extern executor_run_with_stack
extern stack_new
extern stack_pop
extern __mf_print_str
extern __mf_print_int
extern __sys_open
extern __sys_read
extern __sys_close

; ------------------------------------------------------------------------------
; main(argc: rcx, argv: rdx)
; ------------------------------------------------------------------------------
main:
    push rbp
    mov rbp, rsp
    sub rsp, 48 ; Shadow (32) + Local (16)

    cmp rcx, 2
    jl .usage

    ; Save Filename (argv[1]) -> R12
    mov r12, [rdx + 8]
    mov r15, r12

    ; Print Status
    lea rcx, [rel msg_start]
    call print_str_asciz
    mov rcx, r12
    call print_str_asciz
    lea rcx, [rel msg_dot]
    call print_str_asciz

    ; Detect Extension
    mov rcx, r12
    call get_extension
    test rax, rax
    jz .err_ext
    mov rbx, rax ; Ext Ptr

    ; Check .fox
    mov rcx, rbx
    lea rdx, [rel ext_fox]
    call str_equals
    test rax, rax
    jnz .run_fox

    ; Check .morph
    mov rcx, rbx
    lea rdx, [rel ext_morph]
    call str_equals
    test rax, rax
    jnz .run_morph

    jmp .err_ext

.run_fox:
    call read_file_r12
    cmp rax, -1
    je .error

    ; RAX = Len
    mov rdx, rax
    lea rcx, [rel file_buffer]
    call lexer_create
    mov rcx, rax
    call parser_parse_unit
    mov [rel intent_root], rax

    mov rcx, [rel intent_root]
    call compile_ast_to_fragment
    mov r14, rax
    jmp .execute

.run_morph:
    call read_file_r12
    cmp rax, -1
    je .error
    mov r13, rax ; Len

    ; Validate Header
    cmp r13, 16
    jl .err_header

    ; Check Magic
    lea rcx, [rel file_buffer]
    lea rdx, [rel header_magic]
    mov r8, 8
    call mem_cmp
    test rax, rax
    jnz .err_header

    ; Check Version
    lea rcx, [rel file_buffer + 8]
    lea rdx, [rel header_ver]
    mov r8, 8
    call mem_cmp
    test rax, rax
    jnz .err_header

    ; Create Manual Fragment
    lea r14, [rel manual_fragment]
    lea rax, [rel file_buffer + 16]
    mov [r14 + 8], rax
    mov rax, r13
    sub rax, 16
    mov [r14 + 16], rax
    jmp .execute

.execute:
    mov rcx, 8192
    call stack_new
    mov rsi, rax ; Stack Ptr

    mov rcx, r14 ; Fragment
    mov rdx, rsi ; Stack
    push rsi     ; Save Stack
    sub rsp, 32
    call executor_run_with_stack
    add rsp, 32
    pop rsi

    ; Print Result (Top of Stack)
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    call __mf_print_int

    lea rcx, [rel newline]
    call print_str_asciz

    xor rax, rax
    jmp .exit

.error:
    lea rcx, [rel msg_err]
    call print_str_asciz
    mov rax, 1
    jmp .exit

.err_ext:
    lea rcx, [rel msg_inv_ext]
    call print_str_asciz
    mov rax, 1
    jmp .exit

.err_header:
    lea rcx, [rel msg_bad_hdr]
    call print_str_asciz
    mov rax, 1
    jmp .exit

.usage:
    lea rcx, [rel msg_usage]
    call print_str_asciz
    mov rax, 1
    jmp .exit

.exit:
    add rsp, 48
    pop rbp
    ret

; Helpers

read_file_r12:
    push rbp
    mov rbp, rsp
    push r15
    sub rsp, 32

    ; Open
    mov rcx, r12
    xor rdx, rdx ; Flags
    xor r8, r8   ; Mode
    call __sys_open
    cmp rax, -1
    je .rf_fail
    mov r15, rax ; FD

    ; Read
    mov rcx, r15
    lea rdx, [rel file_buffer]
    mov r8, 1048576
    call __sys_read
    push rax ; Save Len

    ; Close
    mov rcx, r15
    call __sys_close

    pop rax ; Len
    add rsp, 32
    pop r15
    pop rbp
    ret
.rf_fail:
    mov rax, -1
    add rsp, 32
    pop r15
    pop rbp
    ret

get_extension:
    xor rax, rax
    mov rdx, rcx
.ext_loop:
    mov r8b, [rdx]
    test r8b, r8b
    jz .ext_done
    cmp r8b, '.'
    cmove rax, rdx
    inc rdx
    jmp .ext_loop
.ext_done:
    ret

str_equals:
    xor rax, rax
.seq_loop:
    mov r8b, [rcx]
    mov r9b, [rdx]
    cmp r8b, r9b
    jne .seq_fail
    test r8b, r8b
    jz .seq_match
    inc rcx
    inc rdx
    jmp .seq_loop
.seq_match:
    mov rax, 1
    ret
.seq_fail:
    xor rax, rax
    ret

mem_cmp:
    xor rax, rax
.mc_loop:
    mov r9b, [rcx]
    mov r10b, [rdx]
    sub r9b, r10b
    jnz .mc_fail
    inc rcx
    inc rdx
    dec r8
    jnz .mc_loop
    xor rax, rax
    ret
.mc_fail:
    mov rax, 1
    ret

print_str_asciz:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    call __mf_print_asciz
    add rsp, 32
    pop rbp
    ret
