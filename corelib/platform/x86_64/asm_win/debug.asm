; corelib/platform/x86_64/asm_win/debug.asm
; Utilitas Debug untuk Windows x86_64
; Menyediakan fungsi untuk visualisasi struktur data

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    str_token_type      db "Type: ", 0
    str_token_val       db ", Val: ", 0
    str_token_int       db "(Int) ", 0
    str_token_str       db "(Str) ", 0
    str_token_kw        db "(Kw) ", 0
    str_token_id        db "(Id) ", 0
    str_token_op        db "(Op) ", 0
    str_token_delim     db "(Delim) ", 0
    str_token_marker    db "(Marker) ", 0
    str_newline         db 10, 0
    str_quote           db 34, 0 ; Double quote

section .text
global __mf_print_token
extern __mf_print_str
extern __mf_print_int

; Konstanta Tipe Token (Harus sama dengan lexer.asm)
TOKEN_EOF        equ 0
TOKEN_INTEGER    equ 1
TOKEN_IDENTIFIER equ 2
TOKEN_OPERATOR   equ 3
TOKEN_KEYWORD    equ 4
TOKEN_STRING     equ 5
TOKEN_DELIMITER  equ 6
TOKEN_MARKER     equ 7

; ------------------------------------------------------------------------------
; func __mf_print_token(token_ptr: ptr)
; Input: RCX = token_ptr
; ------------------------------------------------------------------------------
__mf_print_token:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 32

    mov r12, rcx ; Token Ptr

    test rcx, rcx
    jz .null_ptr

    ; 1. Print "Type: "
    lea rcx, [rel str_token_type]
    mov rdx, 6
    call __mf_print_str

    ; 2. Print Type ID (Integer)
    mov rcx, [r12 + 0] ; Type
    call __mf_print_int

    ; 3. Print ", Val: "
    lea rcx, [rel str_token_val]
    mov rdx, 7
    call __mf_print_str

    ; Switch on Type
    mov rax, [r12 + 0]

    cmp rax, TOKEN_INTEGER
    je .print_int_val
    cmp rax, TOKEN_STRING
    je .print_str_val
    cmp rax, TOKEN_IDENTIFIER
    je .print_id_val
    cmp rax, TOKEN_KEYWORD
    je .print_kw_val
    cmp rax, TOKEN_OPERATOR
    je .print_op_val
    cmp rax, TOKEN_DELIMITER
    je .print_delim_val
    cmp rax, TOKEN_MARKER
    je .print_marker_val

    jmp .done_val

.print_int_val:
    lea rcx, [rel str_token_int]
    mov rdx, 6
    call __mf_print_str

    mov rcx, [r12 + 8] ; Val (i64)
    call __mf_print_int
    jmp .done_val

.print_str_val:
    lea rcx, [rel str_token_str]
    mov rdx, 6
    call __mf_print_str
    jmp .print_str_content

.print_id_val:
    lea rcx, [rel str_token_id]
    mov rdx, 5
    call __mf_print_str
    jmp .print_str_content

.print_kw_val:
    lea rcx, [rel str_token_kw]
    mov rdx, 5
    call __mf_print_str
    jmp .print_str_content

.print_op_val:
    lea rcx, [rel str_token_op]
    mov rdx, 5
    call __mf_print_str

    mov rcx, [r12 + 8] ; Val (ASCII)
    call __mf_print_int
    jmp .done_val

.print_delim_val:
    lea rcx, [rel str_token_delim]
    mov rdx, 8
    call __mf_print_str

    mov rcx, [r12 + 8] ; Val (ASCII)
    call __mf_print_int
    jmp .done_val

.print_marker_val:
    lea rcx, [rel str_token_marker]
    mov rdx, 9
    call __mf_print_str

    mov rcx, [r12 + 8] ; Val (Packed)
    call __mf_print_int
    jmp .done_val

.print_str_content:
    ; Value is Ptr to String Struct [Len][Data]
    mov r13, [r12 + 8]

    ; Print Quote
    lea rcx, [rel str_quote]
    mov rdx, 1
    call __mf_print_str

    ; Print Content
    mov rdx, [r13 + 0] ; Len
    mov rcx, [r13 + 8] ; Data Ptr
    call __mf_print_str

    ; Print Quote
    lea rcx, [rel str_quote]
    mov rdx, 1
    call __mf_print_str
    jmp .done_val

.done_val:
    ; Newline is handled by __mf_print_int but we might want one explicitly if not ending with int
    ; But __mf_print_int adds newline.
    ; If we printed string, we need newline.

    mov rax, [r12 + 0]
    cmp rax, TOKEN_INTEGER
    je .ret_fn
    cmp rax, TOKEN_OPERATOR
    je .ret_fn
    cmp rax, TOKEN_DELIMITER
    je .ret_fn
    cmp rax, TOKEN_MARKER
    je .ret_fn

    ; For strings/ids, add newline
    lea rcx, [rel str_newline]
    mov rdx, 1
    call __mf_print_str

.ret_fn:
    add rsp, 32
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

.null_ptr:
    add rsp, 32
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
