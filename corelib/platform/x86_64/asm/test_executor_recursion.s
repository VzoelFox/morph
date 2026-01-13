# corelib/platform/x86_64/asm/test_executor_recursion.s
# Test Executor Recursion (Linux)
# Verifikasi: CALL, RET, Recursion
# Case: Factorial(3) = 6
# Logic:
# Func FACT:
#   // Stack: [N]
#   DUP
#   LIT 1
#   LE
#   JMP_IF base_case
#   DUP
#   LIT 1
#   SUB
#   CALL FACT
#   MUL
#   RET
# base_case:
#   POP (Drop N)
#   LIT 1
#   RET

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/rpn_gas.inc"

.section .data
fragment:
    .quad 0
    .quad code_buf
    .quad code_end - code_buf
    .quad 1024

code_buf:
    # Main: Call Fact(3)
    .byte OP_LIT
    .quad 3
    .byte OP_CALL
    .quad label_fact - label_ref_main
label_ref_main:
    .byte OP_PRINT      # Should print 6
    .byte OP_EXIT

label_fact:
    # DUP (Manual Dup: Pop A, Push A, Push A)
    # We don't have OP_DUP in SSOT?
    # RPN.FOX doesn't list DUP.
    # We must implement DUP or use local var.
    # Without DUP, we can't do recursive factorial on stack easily.
    # Hack: Add OP_DUP = 99 (using HINT slot or new).
    # Or implement it now.
    # Let's add .byte 4 (OP_STORE/LOAD?) No.
    # Let's assume I added OP_DUP (40) temporarily?
    # Or just use OP_LIT 0 + ADD (Non-destructive read? No).

    # Wait, RPN without DUP is painful.
    # SSOT RPN.FOX:
    # 00-09 Data.
    # OP_LOAD, OP_STORE.
    # If I use Store/Load with ID 0 as "Register", I can dup.
    # But recursion needs LOCAL scope. Global var will be overwritten.

    # WITHOUT LOCAL SCOPE, RECURSION IS IMPOSSIBLE FOR FACTORIAL.
    # Factorial(3) calls Factorial(2).
    # If both store N in global 'x', F(2) overwrites F(3)'s N.
    # When F(2) returns, F(3) reads modified N.

    # CONCLUSION: My plan to implement "Recursion" is BLOCKED by lack of Local Scope logic in SSOT.
    # But `executor.s` CALL stack only saves IP. It doesn't save Locals.

    # WORKAROUND: Use Stack Manipulation only.
    # But I need DUP.
    # I will Add OP_DUP (Code 5) to `executor.s` and `rpn_gas.inc` NOW.
    # It's essential for Stack Machine.

    .byte OP_DUP
    .byte OP_LIT
    .quad 1
    .byte OP_LEQ
    .byte OP_JMP_IF
    .quad label_base - label_ref_fact
label_ref_fact:

    # Recursive Step
    # Stack: [N]
    .byte OP_DUP
    .byte OP_LIT
    .quad 1
    .byte OP_SUB
    .byte OP_CALL
    .quad label_fact - label_ref_rec
label_ref_rec:
    # Stack: [N, Fact(N-1)]
    .byte OP_MUL
    .byte OP_RET

label_base:
    # Stack: [N] (which is <= 1)
    # Pop N, Push 1
    # We need OP_POP (Drop).
    # I will add OP_POP (Code 6).
    .byte OP_POP
    .byte OP_LIT
    .quad 1
    .byte OP_RET

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
