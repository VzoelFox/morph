# corelib/platform/x86_64/asm/test_executor_flow.s
# Test Executor Control Flow (Linux) - FIXED
# Verifikasi: JMP, JMP_IF, SWITCH

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/rpn_gas.inc"

.section .data
fragment:
    .quad 0
    .quad code_buf
    .quad code_end - code_buf
    .quad 1024

code_buf:
    # 1. Test Simple Jump (Skip Print 111)
    .byte OP_JMP
    .quad label_target_1 - label_ref_1
label_ref_1:

    # Body to Skip
    .byte OP_LIT
    .quad 111
    .byte OP_PRINT

label_target_1:
    # Target
    .byte OP_LIT
    .quad 222
    .byte OP_PRINT

    # 2. Test IF-ELSE
    # IF (10 > 5) PRINT 888 ELSE PRINT 999
    .byte OP_LIT
    .quad 10
    .byte OP_LIT
    .quad 5
    .byte OP_GT
    .byte OP_JMP_FALSE
    .quad label_else - label_ref_2
label_ref_2:

    # True Block
    .byte OP_LIT
    .quad 888
    .byte OP_PRINT
    .byte OP_JMP
    .quad label_end - label_ref_3
label_ref_3:

label_else:
    # Else Block
    .byte OP_LIT
    .quad 999
    .byte OP_PRINT

label_end:
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
