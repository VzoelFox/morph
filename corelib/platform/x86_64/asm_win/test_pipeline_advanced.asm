; corelib/platform/x86_64/asm_win/test_pipeline_advanced.asm
; Test Pipeline: Parser -> Compiler -> Executor (Windows Port)
; Verifies Control Flow (If/While) and Variables
; ==============================================================================

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    ; "var x = 10; jika (x) x + 5 lain 0 tutup_jika"
    input_expr db "var x = 10 jika (x) x + 5 lain 0 tutup_jika", 0
    input_len dq 43

section .text
global main
extern parser_parse_block
extern compile_ast_to_fragment
extern executor_run_with_stack
extern lexer_create
extern mem_alloc
extern stack_new
extern stack_pop
extern sym_table_create
extern __mf_print_int
extern global_sym_table
extern global_sym_cap

main:
    push rbp
    mov rbp, rsp
    sub rsp, 48             ; Shadow space + Locals

    ; 1. Runtime Symbol Table is now initialized in runtime.asm:_start

    ; 2. Create Lexer
    lea rcx, [rel input_expr]
    mov rdx, [rel input_len]
    call lexer_create
    mov r12, rax ; Lexer

    ; 3. Parse Block
    mov rcx, r12
    call parser_parse_block
    test rax, rax
    jz .fail_parse
    mov r13, rax ; AST Head

    ; 4. Compile
    mov rcx, r13
    call compile_ast_to_fragment
    test rax, rax
    jz .fail_compile
    mov r14, rax ; Fragment

    ; 5. Execute
    mov rcx, 1024
    call stack_new
    mov r15, rax ; Stack

    mov rcx, r14
    mov rdx, r15
    call executor_run_with_stack

    ; 6. Check Result (10 + 5 = 15)
    mov rcx, r15
    call stack_pop

    cmp rax, 15
    jne .fail_result

    mov rax, 0
    jmp .exit

.fail_parse:
    mov rax, 101
    jmp .exit
.fail_compile:
    mov rax, 102
    jmp .exit
.fail_result:
    ; mov rcx, rax
    ; call __mf_print_int
    mov rax, 103
    jmp .exit

.exit:
    add rsp, 48
    pop rbp
    ret
