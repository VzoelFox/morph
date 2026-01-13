# corelib/platform/x86_64/asm/test_pipeline.s
# Test Pipeline: Parser -> Compiler -> Executor
# Verifies standard "10 + 20" logic.

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
input_expr: .asciz "10 + 20"
input_len:  .quad 7

.section .text
.global main
# Externs: parser_parse_expression, compile_ast_to_fragment, executor_run_with_stack
# Externs: lexer_create, mem_alloc, stack_new

main:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp

    # 1. Initialize Runtime (Called by _start usually, but we need manual if not linked with runtime?)
    # Assuming linked with runtime.s which calls main.

    # 2. Create Lexer
    leaq input_expr(%rip), %rdi
    movq input_len(%rip), %rsi
    call lexer_create
    movq %rax, %r12 # R12 = Lexer Ptr

    # 3. Parse Expression
    movq %r12, %rdi
    call parser_parse_expression
    testq %rax, %rax
    jz .fail_parse
    movq %rax, %r13 # R13 = AST Node Ptr

    # 4. Compile to Fragment
    movq %r13, %rdi
    call compile_ast_to_fragment
    testq %rax, %rax
    jz .fail_compile
    movq %rax, %r14 # R14 = Fragment Ptr

    # 5. Create Stack for Executor
    movq $1024, %rdi
    call stack_new
    movq %rax, %r15 # R15 = MorphStack Ptr

    # 6. Execute Fragment
    movq %r14, %rdi
    movq %r15, %rsi
    call executor_run_with_stack

    # 7. Check Result
    # Pop from stack, should be 30
    movq %r15, %rdi
    call stack_pop

    cmpq $30, %rax
    jne .fail_result

    # Success
    movq $0, %rax # Exit 0
    jmp .exit

.fail_parse:
    movq $101, %rax
    jmp .exit
.fail_compile:
    movq $102, %rax
    jmp .exit
.fail_result:
    # Print result for debug
    movq %rax, %rdi
    call __mf_print_int
    movq $103, %rax
    jmp .exit

.exit:
    leave
    ret
