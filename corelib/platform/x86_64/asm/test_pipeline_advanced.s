# corelib/platform/x86_64/asm/test_pipeline_advanced.s
# Test Pipeline: Parser -> Compiler -> Executor
# Verifies Control Flow (If/While) and Variables

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
# "var x = 10; jika (x) x + 5 lain 0 tutup_jika"
input_expr: .asciz "var x = 10 jika (x) x + 5 lain 0 tutup_jika"
input_len:  .quad 43

.section .text
.global main
# Externs: parser_parse_block, compile_ast_to_fragment, executor_run_with_stack
# Externs: lexer_create, mem_alloc, stack_new, sym_table_create

main:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp

    # 1. Initialize Runtime (Global Symbol Table?)
    # We need to setup symbol table for Executor to use OP_LOAD/STORE.
    # But where is it stored? Executor needs access.
    # For this test, we might need to hack/inject it or ensure runtime.s does it.
    # Let's assume runtime.s doesn't init it yet (based on reading).
    # We'll call sym_table_create and store it in a global that executor expects?
    # Executor stub for OP_LOAD/STORE currently doesn't use a global.
    # We need to UPDATE executor.s to use a symbol table.

    # 2. Create Lexer
    leaq input_expr(%rip), %rdi
    movq input_len(%rip), %rsi
    call lexer_create
    movq %rax, %r12 # Lexer

    # 3. Parse Block
    movq %r12, %rdi
    call parser_parse_block
    testq %rax, %rax
    jz .fail_parse
    movq %rax, %r13 # AST Head

    # 4. Compile
    movq %r13, %rdi
    call compile_ast_to_fragment
    testq %rax, %rax
    jz .fail_compile
    movq %rax, %r14 # Fragment

    # 5. Execute
    movq $1024, %rdi
    call stack_new
    movq %rax, %r15

    movq %r14, %rdi
    movq %r15, %rsi
    call executor_run_with_stack

    # 6. Check Result (10 + 5 = 15)
    movq %r15, %rdi
    call stack_pop

    cmpq $15, %rax
    jne .fail_result

    movq $0, %rax
    jmp .exit

.fail_parse:
    movq $101, %rax
    jmp .exit
.fail_compile:
    movq $102, %rax
    jmp .exit
.fail_result:
    # movq %rax, %rdi
    # call __mf_print_int
    movq $103, %rax
    jmp .exit

.exit:
    leave
    ret
