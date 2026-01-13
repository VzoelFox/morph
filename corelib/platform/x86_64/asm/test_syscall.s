# corelib/platform/x86_64/asm/test_syscall.s
# Test OP_SYSCALL (Glue Layer)
# Intent: WRITE "Glue\n" to Stdout

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/rpn_gas.inc"
.include "corelib/platform/x86_64/asm/syscalls.inc"

.section .data
msg_glue: .ascii "Glue\n"

fragment:
    .quad 0
    .quad code_buf
    .quad code_end - code_buf
    .quad 1024

code_buf:
    # 1. Push FD (1)
    .byte OP_LIT
    .quad 1

    # 2. Push Ptr (Address of msg_glue)
    # RPN biasanya pake OP_LOAD_LIT atau sejenisnya.
    # Kita pake OP_LIT dengan value alamat (Linker akan resolve).
    .byte OP_LIT
    .quad msg_glue

    # 3. Push Len (5)
    .byte OP_LIT
    .quad 5

    # 4. Push Intent (WRITE)
    .byte OP_LIT
    .quad SYS_INTENT_WRITE

    # 5. Syscall
    .byte OP_SYSCALL

    # 6. Result is on stack. Pop and ignore? Or Print?
    # Print result (count bytes written)
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

    movq $1024, %rdi
    call stack_new
    movq %rax, %rbx

    leaq fragment(%rip), %rdi
    movq %rbx, %rsi
    call executor_run_with_stack

    movq $0, %rdi
    call __sys_exit
