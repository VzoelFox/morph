; corelib/platform/x86_64/asm_win/parser.asm
; Intent Parser: Mengubah Token Stream menjadi IntentTree (Unit -> Shard -> Fragment)
; ==============================================================================

%include "corelib/platform/x86_64/asm_win/macros.inc"
%include "corelib/platform/x86_64/asm_win/intent.inc"

section .data
    ; Global recursion depth counter for parser safety
    __parser_recursion_depth: dq 0

    ; ROBUSTNESS: Parser error reporting
    __parser_error_code: dq 0
    __parser_error_line: dq 0
    __parser_error_col: dq 0

section .bss
    __parser_error_msg: resb 256  ; Error message buffer (Moved to BSS)

section .rodata
    ; Error messages
    err_msg_unexpected_token: db "Unexpected token", 0
    err_msg_expected_string: db "Expected string literal", 0
    err_msg_expected_identifier: db "Expected identifier", 0
    err_msg_expected_keyword: db "Expected keyword", 0
    err_msg_import_failed: db "Import statement malformed", 0
    err_msg_function_failed: db "Function definition malformed", 0
    err_msg_unexpected_eof: db "Unexpected end of file", 0

section .text
global parser_parse_unit
global parser_parse_block
global parser_parse_expression
extern mem_alloc
extern lexer_next_token
extern lexer_peek_token
extern __sys_exit

; Token Types
TOKEN_EOF        equ 0
TOKEN_INTEGER    equ 1
TOKEN_IDENTIFIER equ 2
TOKEN_OPERATOR   equ 3
TOKEN_KEYWORD    equ 4
TOKEN_STRING     equ 5
TOKEN_DELIMITER  equ 6
TOKEN_SYSCALL    equ 20

; SAFETY: Maximum recursion depth limit (prevents stack overflow)
MAX_PARSER_DEPTH equ 256

; ==============================================================================
; ENTRY POINT: parser_parse_unit
; Output: RAX = Pointer to UNIT Node
; ==============================================================================
parser_parse_unit:
    push rbp
    mov rbp, rsp
    push rbx
    push r12 ; Lexer
    push r13 ; Unit Node
    push r14 ; Last Child (Tail)
    push r15
    sub rsp, 32

    mov r12, rcx

    ; 1. Alloc Unit Node
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov r13, rax

    mov qword [r13 + INTENT_OFFSET_TYPE], INTENT_UNIT_MODULE
    mov qword [r13 + INTENT_OFFSET_NEXT], 0
    mov qword [r13 + INTENT_OFFSET_CHILD], 0

    xor r14, r14

.unit_loop:
    mov rcx, r12
    call lexer_peek_token
    mov rbx, rax

    cmp qword [rbx + 0], TOKEN_EOF
    je .unit_done

    cmp qword [rbx + 0], TOKEN_KEYWORD
    je .unit_keyword

    ; Statement Global
    mov rcx, r12
    call parse_statement
    test rax, rax
    jz .unit_skip

    jmp .unit_append

.unit_keyword:
    mov rcx, [rbx + 8]
    call get_keyword_id
    cmp rax, 4 ; fungsi
    je .unit_func
    cmp rax, 9 ; ambil
    je .unit_import

    mov rcx, r12
    call parse_statement
    test rax, rax
    jz .unit_skip
    jmp .unit_append

.unit_func:
    mov rcx, r12
    call parse_function
    test rax, rax
    jz .unit_skip
    jmp .unit_append

.unit_import:
    mov rcx, r12
    call parse_import
    test rax, rax
    jz .unit_skip
    jmp .unit_append

.unit_append:
    test r14, r14
    jnz .unit_link_tail
    mov [r13 + INTENT_OFFSET_CHILD], rax
    mov r14, rax
    jmp .unit_loop

.unit_link_tail:
    mov [r14 + INTENT_OFFSET_NEXT], rax
    mov r14, rax
    jmp .unit_loop

.unit_skip:
    mov rcx, r12
    call lexer_next_token
    jmp .unit_loop

.unit_done:
    mov rax, r13
    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ==============================================================================
; parse_import
; "ambil 'path' [: symbol]"
; ==============================================================================
parse_import:
    push rbp
    mov rbp, rsp
    push r12
    push r13 ; Path
    push r14 ; Symbol (Optional)
    sub rsp, 32
    mov r12, rcx

    ; SAFETY: Check recursion depth
    lea rax, [rel __parser_recursion_depth]
    mov rcx, [rax]
    cmp rcx, MAX_PARSER_DEPTH
    jge .pi_depth_exceeded
    inc qword [rax]  ; Increment depth

    ; Consume "ambil"
    mov rcx, r12
    call lexer_next_token

    ; Expect String (Path)
    mov rcx, r12
    call lexer_next_token
    cmp qword [rax + 0], TOKEN_STRING
    jne .pi_fail
    mov r13, [rax + 8]

    ; Check for Colon (Granular) - in Windows, ':' is TOKEN_DELIMITER
    mov rcx, r12
    call lexer_peek_token
    cmp qword [rax + 0], TOKEN_DELIMITER
    jne .pi_no_symbol
    cmp qword [rax + 8], ':' ; Check if delimiter is ':'
    je .pi_granular

.pi_no_symbol:

    xor r14, r14 ; No Symbol (Full Import)
    jmp .pi_create

.pi_granular:
    ; Consume Colon
    mov rcx, r12
    call lexer_next_token

    ; Expect Identifier (Symbol Name)
    mov rcx, r12
    call lexer_next_token
    cmp qword [rax + 0], TOKEN_IDENTIFIER
    jne .pi_fail
    mov r14, [rax + 8]

.pi_create:
    ; Create INTENT_FRAG_IMPORT Node
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov qword [rax + INTENT_OFFSET_TYPE], 0x1002
    mov [rax + INTENT_OFFSET_DATA_A], r13
    mov [rax + INTENT_OFFSET_DATA_B], r14

    jmp .pi_ret

.pi_fail:
    xor rax, rax
    jmp .pi_ret

.pi_depth_exceeded:
    ; Recursion depth exceeded - exit with error code
    mov rcx, 112
    call __sys_exit

.pi_ret:
    ; SAFETY: Decrement recursion depth before returning
    lea rcx, [rel __parser_recursion_depth]
    dec qword [rcx]

    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

; ==============================================================================
; parse_function
; "fungsi nama() block tutup_fungsi"
; Output: SHARD_FUNC Node
; ==============================================================================
parse_function:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 32

    mov r12, rcx

    ; Consume "fungsi"
    mov rcx, r12
    call lexer_next_token

    ; Expect Identifier
    mov rcx, r12
    call lexer_next_token
    cmp qword [rax + 0], TOKEN_IDENTIFIER
    jne .pf_fail
    mov r13, [rax + 8] ; Name

    ; Expect "("
    mov rcx, r12
    call lexer_next_token

    ; Expect ")"
    mov rcx, r12
    call lexer_next_token

    ; Parse Body
    mov rcx, r12
    call parser_parse_block
    mov r14, rax

    ; Expect "tutup_fungsi"
    mov rcx, r12
    call lexer_next_token

    ; Create SHARD Node
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc

    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_SHARD_FUNC
    mov [rax + INTENT_OFFSET_CHILD], r14 ; Body
    mov [rax + INTENT_OFFSET_DATA_A], r13 ; Name
    mov qword [rax + INTENT_OFFSET_NEXT], 0

    jmp .pf_ret

.pf_fail:
    xor rax, rax
.pf_ret:
    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ==============================================================================
; parser_parse_block
; ==============================================================================
parser_parse_block:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 32

    mov r12, rcx
    xor r13, r13
    xor r14, r14

.blk_loop:
    mov rcx, r12
    call lexer_peek_token
    mov rbx, rax

    cmp qword [rbx + 0], TOKEN_EOF
    je .blk_done

    cmp qword [rbx + 0], TOKEN_KEYWORD
    jne .blk_stmt

    mov rcx, [rbx + 8]
    call is_closing_keyword
    test rax, rax
    jnz .blk_done

.blk_stmt:
    mov rcx, r12
    call parse_statement
    test rax, rax
    jz .blk_done

    test r13, r13
    jnz .blk_append
    mov r13, rax
    mov r14, rax
    jmp .blk_loop

.blk_append:
    mov [r14 + INTENT_OFFSET_NEXT], rax
    mov r14, rax
    jmp .blk_loop

.blk_done:
    mov rax, r13
    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ==============================================================================
; parse_statement
; ==============================================================================
parse_statement:
    add rsp, 40 ; Align (Optional safe)
    pop r12
    pop rbx

    push rbp
    mov rbp, rsp
    push rbx
    push r12
    sub rsp, 32

    mov r12, rcx

    mov rcx, r12
    call lexer_peek_token
    mov rbx, rax

    cmp qword [rbx + 0], TOKEN_KEYWORD
    je .stmt_kw

    mov rcx, r12
    call parser_parse_expression
    jmp .stmt_ret

.stmt_kw:
    mov rcx, [rbx + 8]
    call get_keyword_id

    cmp rax, 1 ; var
    je .do_var
    cmp rax, 2 ; jika
    je .do_if
    cmp rax, 3 ; selama
    je .do_while
    cmp rax, 5 ; kembali
    je .do_return
    cmp rax, 6 ; pilih
    je .do_switch

    xor rax, rax
    jmp .stmt_ret

.do_var:
    mov rcx, r12
    call parse_var_decl
    jmp .stmt_ret
.do_if:
    mov rcx, r12
    call parse_if
    jmp .stmt_ret
.do_while:
    mov rcx, r12
    call parse_while
    jmp .stmt_ret
.do_return:
    mov rcx, r12
    call parse_return
    jmp .stmt_ret
.do_switch:
    mov rcx, r12
    call parse_switch
    jmp .stmt_ret

.stmt_ret:
    add rsp, 32
    pop r12
    pop rbx
    pop rbp
    ret

; ==============================================================================
; parse_syscall
; "sistem intent, arg1, arg2..."
; ==============================================================================
parse_syscall:
    push rbp
    mov rbp, rsp
    push r12
    push r13 ; Intent
    push r14 ; Arg Head
    push r15 ; Arg Tail
    sub rsp, 32
    mov r12, rcx

    mov rcx, r12
    call parser_parse_expression
    mov r13, rax

    xor r14, r14
    xor r15, r15

.psys_loop:
    mov rcx, r12
    call lexer_peek_token
    mov rax, [rax + 8]
    cmp rax, 44 ; ','
    jne .psys_done

    mov rcx, r12
    call lexer_next_token

    mov rcx, r12
    call parser_parse_expression
    test rax, rax
    jz .psys_done

    test r14, r14
    jnz .psys_append
    mov r14, rax
    mov r15, rax
    jmp .psys_loop

.psys_append:
    mov [r15 + INTENT_OFFSET_NEXT], rax
    mov r15, rax
    jmp .psys_loop

.psys_done:
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov qword [rax + INTENT_OFFSET_TYPE], 0x4003 ; INTENT_FRAG_SYSCALL
    mov [rax + INTENT_OFFSET_CHILD], r13
    mov [rax + INTENT_OFFSET_DATA_A], r14

    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

; ==============================================================================
; parse_switch (pilih)
; ==============================================================================
parse_switch:
    push rbp
    mov rbp, rsp
    push r12
    push r13 ; Cond Expr
    push r14 ; Case List Head
    push r15 ; Case List Tail
    sub rsp, 32
    mov r12, rcx

    mov rcx, r12
    call lexer_next_token ; pilih
    mov rcx, r12
    call lexer_next_token ; (
    mov rcx, r12
    call parser_parse_expression
    mov r13, rax
    mov rcx, r12
    call lexer_next_token ; )

    xor r14, r14
    xor r15, r15

.ps_loop:
    mov rcx, r12
    call lexer_peek_token
    mov rcx, [rax + 8]
    call get_keyword_id
    cmp rax, 8 ; kasus
    jne .ps_done

    mov rcx, r12
    call parse_case
    test rax, rax
    jz .ps_done

    test r14, r14
    jnz .ps_append
    mov r14, rax
    mov r15, rax
    jmp .ps_loop
.ps_append:
    mov [r15 + INTENT_OFFSET_NEXT], rax
    mov r15, rax
    jmp .ps_loop

.ps_done:
    mov rcx, r12
    call lexer_next_token ; tutup_pilih

    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_FRAG_SWITCH
    mov [rax + INTENT_OFFSET_CHILD], r13
    mov [rax + INTENT_OFFSET_DATA_A], r14

    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

; ==============================================================================
; parse_case
; ==============================================================================
parse_case:
    push rbp
    mov rbp, rsp
    push r12
    push r13
    push r14
    sub rsp, 32
    mov r12, rcx

    mov rcx, r12
    call lexer_next_token ; kasus

    mov rcx, r12
    call parser_parse_expression
    mov r13, rax

    mov rcx, r12
    call lexer_next_token ; :

    mov rcx, r12
    call parser_parse_block
    mov rbx, rax

    push rbx
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    pop rbx
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_SHARD_BLOCK
    mov [rax + INTENT_OFFSET_CHILD], rbx
    mov r14, rax

    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_FRAG_CASE
    mov [rax + INTENT_OFFSET_CHILD], r13
    mov [rax + INTENT_OFFSET_DATA_A], r14

    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

; ==============================================================================
; parse_if
; ==============================================================================
parse_if:
    push rbp
    mov rbp, rsp
    push r12
    push r13
    push r14
    push r15
    sub rsp, 32
    mov r12, rcx

    mov rcx, r12
    call lexer_next_token
    mov rcx, r12
    call lexer_next_token

    mov rcx, r12
    call parser_parse_expression
    mov r13, rax

    mov rcx, r12
    call lexer_next_token

    mov rcx, r12
    call parser_parse_block
    mov rbx, rax
    push rbx
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    pop rbx
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_SHARD_BLOCK
    mov [rax + INTENT_OFFSET_CHILD], rbx
    mov r14, rax

    xor r15, r15
    mov rcx, r12
    call lexer_peek_token
    mov rcx, [rax + 8]
    call get_keyword_id
    cmp rax, 7 ; lain
    jne .pif_close

    mov rcx, r12
    call lexer_next_token

    mov rcx, r12
    call parser_parse_block
    mov rbx, rax
    push rbx
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    pop rbx
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_SHARD_BLOCK
    mov [rax + INTENT_OFFSET_CHILD], rbx
    mov r15, rax

.pif_close:
    mov rcx, r12
    call lexer_next_token

    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_FRAG_IF
    mov [rax + INTENT_OFFSET_CHILD], r13

    test r13, r13
    jz .pif_done
    mov [r13 + INTENT_OFFSET_NEXT], r14
    test r14, r14
    jz .pif_done
    mov [r14 + INTENT_OFFSET_NEXT], r15

.pif_done:
    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

; ==============================================================================
; parse_while
; ==============================================================================
parse_while:
    push rbp
    mov rbp, rsp
    push r12
    push r13
    push r14
    sub rsp, 32
    mov r12, rcx

    mov rcx, r12
    call lexer_next_token
    mov rcx, r12
    call lexer_next_token

    mov rcx, r12
    call parser_parse_expression
    mov r13, rax

    mov rcx, r12
    call lexer_next_token

    mov rcx, r12
    call parser_parse_block
    mov rbx, rax
    push rbx
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    pop rbx
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_SHARD_BLOCK
    mov [rax + INTENT_OFFSET_CHILD], rbx
    mov r14, rax

    mov rcx, r12
    call lexer_next_token

    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_FRAG_WHILE
    mov [rax + INTENT_OFFSET_CHILD], r13

    test r13, r13
    jz .pw_done
    mov [r13 + INTENT_OFFSET_NEXT], r14

.pw_done:
    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

; ==============================================================================
; parse_var_decl
; ==============================================================================
parse_var_decl:
    push rbp
    mov rbp, rsp
    push r12
    push r13
    push r14
    sub rsp, 32
    mov r12, rcx

    mov rcx, r12
    call lexer_next_token
    mov rcx, r12
    call lexer_next_token
    mov r13, [rax + 8]

    mov rcx, r12
    call lexer_next_token

    mov rcx, r12
    call parser_parse_expression
    mov r14, rax

    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_FRAG_ASSIGN
    mov [rax + INTENT_OFFSET_DATA_A], r13
    mov [rax + INTENT_OFFSET_CHILD], r14

    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

; ==============================================================================
; parse_return
; ==============================================================================
parse_return:
    push rbp
    mov rbp, rsp
    push r12
    push rbx
    sub rsp, 32
    mov r12, rcx

    mov rcx, r12
    call lexer_next_token
    mov rcx, r12
    call parser_parse_expression
    mov rbx, rax

    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_FRAG_RETURN
    mov [rax + INTENT_OFFSET_CHILD], rbx

    add rsp, 32
    pop rbx
    pop r12
    pop rbp
    ret

; ==============================================================================
; parser_parse_expression
; ==============================================================================
parser_parse_expression:
    push rbp
    mov rbp, rsp
    push r12
    push r13
    push r14
    push r15
    sub rsp, 32
    mov r12, rcx

    mov rcx, r12
    call parse_primary
    test rax, rax
    jz .expr_fail
    mov r13, rax

    mov rcx, r12
    call lexer_peek_token
    mov rbx, rax
    cmp qword [rbx + 0], TOKEN_OPERATOR
    jne .expr_ret_left

    mov rsi, [rbx + 8]
    cmp rsi, 58 ; :
    je .expr_ret_left

    mov rcx, r12
    call lexer_next_token
    mov r14, [rax + 8]

    mov rcx, r12
    call parse_primary
    mov r15, rax

    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_FRAG_BINARY
    mov [rax + INTENT_OFFSET_DATA_A], r14
    mov [rax + INTENT_OFFSET_CHILD], r13
    mov [r13 + INTENT_OFFSET_NEXT], r15
    jmp .expr_done

.expr_ret_left:
    mov rax, r13
    jmp .expr_done

.expr_fail:
    xor rax, rax
.expr_done:
    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

parse_primary:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    sub rsp, 32
    mov r12, rcx

    mov rcx, r12
    call lexer_next_token
    mov rbx, [rax + 0]
    mov rcx, [rax + 8]

    cmp rbx, TOKEN_INTEGER
    je .pp_int
    cmp rbx, TOKEN_IDENTIFIER
    je .pp_var
    cmp rbx, TOKEN_STRING
    je .pp_str
    cmp rbx, TOKEN_KEYWORD
    je .pp_kw

    xor rax, rax
    jmp .pp_ret

.pp_int:
    push rcx
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    pop rcx
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_FRAG_LITERAL
    mov [rax + INTENT_OFFSET_DATA_A], rcx
    jmp .pp_ret

.pp_var:
    push rcx
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    pop rcx
    mov qword [rax + INTENT_OFFSET_TYPE], INTENT_FRAG_VAR
    mov [rax + INTENT_OFFSET_DATA_A], rcx
    jmp .pp_ret

.pp_str:
    push rcx
    mov rcx, INTENT_NODE_SIZE
    call mem_alloc
    pop rcx
    mov qword [rax + INTENT_OFFSET_TYPE], 0x4002 ; INTENT_FRAG_STRING
    mov [rax + INTENT_OFFSET_DATA_A], rcx
    jmp .pp_ret

.pp_kw:
    push rcx
    mov rcx, rcx ; No-op, just save val in rcx for get_keyword_id
    call get_keyword_id
    cmp rax, 20 ; sistem
    je .pp_sys
    pop rcx
    xor rax, rax
    jmp .pp_ret

.pp_sys:
    pop rcx
    mov rcx, r12
    call parse_syscall
    jmp .pp_ret

.pp_ret:
    add rsp, 32
    pop r12
    pop rbx
    pop rbp
    ret

; Helpers
is_closing_keyword:
    mov rsi, rcx
    cmp byte [rsi], 't'
    jne .chk_lain
    cmp byte [rsi+1], 'u'
    jne .chk_lain
    mov rax, 1
    ret
.chk_lain:
    cmp byte [rsi], 'l'
    jne .not_close
    cmp byte [rsi+1], 'a'
    jne .not_close
    mov rax, 1
    ret
.not_close:
    xor rax, rax
    ret

get_keyword_id:
    mov rsi, rcx
    movzx rax, byte [rsi]
    cmp al, 'v'
    je .k_var
    cmp al, 'j'
    je .k_jika
    cmp al, 's'
    je .k_check_s
    cmp al, 'f'
    je .k_fun
    cmp al, 'p'
    je .k_pil
    cmp al, 'k'
    je .k_check_k
    cmp al, 'l'
    je .k_lain
    cmp al, 'a'
    je .k_ambil
    xor rax, rax
    ret
.k_var: mov rax, 1; ret
.k_jika: mov rax, 2; ret
.k_sel: mov rax, 3; ret
.k_sys: mov rax, 20; ret
.k_fun: mov rax, 4; ret
.k_pil: mov rax, 6; ret
.k_lain: mov rax, 7; ret
.k_ambil: mov rax, 9; ret
.k_check_k:
    movzx rcx, byte [rsi+1]
    cmp cl, 'e'
    je .k_ret
    cmp cl, 'a'
    je .k_kas
    xor rax, rax
    ret
.k_ret: mov rax, 5; ret
.k_kas: mov rax, 8; ret
.k_check_s:
    movzx rcx, byte [rsi+1]
    cmp cl, 'e'
    je .k_sel
    cmp cl, 'i'
    je .k_sys
    xor rax, rax
    ret
.k_ret_sistem: mov rax, 20; ret

; ------------------------------------------------------------------------------
; Parser Error Reporting Helper Functions
; ------------------------------------------------------------------------------

; set_parser_error(error_code: rcx, message: rdx, token: r8)
; Records an error with context information
global set_parser_error
set_parser_error:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13

    mov r12, rcx  ; error code
    mov r13, rdx  ; message

    ; Store error code
    lea rax, [rel __parser_error_code]
    mov [rax], r12

    ; Copy message to buffer (max 255 bytes)
    lea rdi, [rel __parser_error_msg]
    mov rsi, r13
    xor rcx, rcx
.copy_msg:
    mov al, [rsi + rcx]
    mov [rdi + rcx], al
    test al, al
    jz .copy_done
    inc rcx
    cmp rcx, 255
    jl .copy_msg
.copy_done:
    mov byte [rdi + rcx], 0  ; Null terminate

    ; Extract line/col from token if provided
    test r8, r8
    jz .skip_token
    ; Assuming token structure has line at offset 8, col at offset 16
    mov rax, [r8 + 8]
    lea rbx, [rel __parser_error_line]
    mov [rbx], rax
    mov rax, [r8 + 16]
    lea rbx, [rel __parser_error_col]
    mov [rbx], rax
.skip_token:

    pop r13
    pop r12
    pop rbx
    leave
    ret

; parser_get_last_error() -> message pointer or NULL
; Returns pointer to error message if an error exists, NULL otherwise
global parser_get_last_error
parser_get_last_error:
    lea rax, [rel __parser_error_code]
    mov rcx, [rax]
    test rcx, rcx
    jz .no_error
    lea rax, [rel __parser_error_msg]
    ret
.no_error:
    xor rax, rax
    ret

; parser_clear_error()
; Clears the error state
global parser_clear_error
parser_clear_error:
    lea rax, [rel __parser_error_code]
    mov qword [rax], 0
    lea rax, [rel __parser_error_line]
    mov qword [rax], 0
    lea rax, [rel __parser_error_col]
    mov qword [rax], 0
    ret
