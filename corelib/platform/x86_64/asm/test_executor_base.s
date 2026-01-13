# corelib/platform/x86_64/asm/test_executor_base.s
# Test Executor Base Logic (Linux)
# Verifikasi: LIT, ADD, PRINT

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/rpn_gas.inc"

.section .data
# Pseudo-fragment structure
fragment:
    .quad 0          # Type
    .quad code_buf   # Code Ptr
    .quad code_end - code_buf # Size
    .quad 1024       # Capacity

code_buf:
    # 10 + 20
    .byte OP_LIT
    .quad 10
    .byte OP_LIT
    .quad 20
    .byte OP_ADD
    .byte OP_PRINT
    .byte OP_EXIT
code_end:

.section .text
.global main
.extern stack_new
.extern executor_run_with_stack
.extern __sys_exit

main:
    pushq %rbp
    movq %rsp, %rbp

    # 1. Create Stack
    movq $1024, %rdi    # Capacity
    call stack_new
    movq %rax, %rbx     # Save Stack Ptr

    # 2. Run Executor
    leaq fragment(%rip), %rdi
    movq %rbx, %rsi
    call executor_run_with_stack

    # 3. Exit
    movq $0, %rdi
    call __sys_exit
