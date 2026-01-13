# test_cb.s
.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global main
.extern vm_cycle_budget
.extern ctx_loop_new
.extern ctx_stack_push
.extern ctx_stack_pop

main:
    # 1. Test Budget Global
    movq vm_cycle_budget(%rip), %rax
    subq $10, %rax
    movq %rax, vm_cycle_budget(%rip)

    # 2. Test Loop Ctx
    movq $10, %rdi # Start
    movq $20, %rsi # End
    call ctx_loop_new
    testq %rax, %rax
    jz .fail
    movq %rax, %r12 # Ctx Ptr

    # 3. Test Stack Push
    movq $0, %rdi   # Top = 0
    movq %r12, %rsi # Ctx
    movq $2, %rdx   # Type Loop
    call ctx_stack_push
    testq %rax, %rax
    jz .fail
    movq %rax, %r13 # New Top

    # Verify Content
    # Node: [0]Prev, [8]Ctx, [16]Type
    movq 8(%r13), %rbx
    cmpq %r12, %rbx
    jne .fail

    # 4. Test Stack Pop
    movq %r13, %rdi
    call ctx_stack_pop
    # Should be 0 (Prev)
    testq %rax, %rax
    jnz .fail

    # Success
    movq $60, %rax
    movq $0, %rdi
    syscall

.fail:
    movq $60, %rax
    movq $1, %rdi
    syscall
