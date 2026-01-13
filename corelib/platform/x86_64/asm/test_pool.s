# corelib/platform/x86_64/asm/test_pool.s
# Test Suite untuk Memory Pool

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
    msg_start: .ascii "Starting Pool Test...\n"
    len_start = . - msg_start
    msg_ok:    .ascii "[OK] "
    len_ok = . - msg_ok
    msg_fail:  .ascii "[FAIL] "
    len_fail = . - msg_fail
    msg_nl:    .ascii "\n"
    len_nl = . - msg_nl

.section .text
.global main

main:
    pushq %rbp
    movq %rsp, %rbp

    OS_WRITE $1, msg_start(%rip), $len_start

    # --------------------------------------------------------------------------
    # TEST 1: Pool Creation
    # --------------------------------------------------------------------------
    # Create Pool: ObjSize=16, Capacity=2
    movq $16, %rdi
    movq $2, %rsi
    call pool_create

    testq %rax, %rax
    jz .fail_create

    movq %rax, %r12 # R12 = Pool Ptr

    # --------------------------------------------------------------------------
    # TEST 2: Allocation (Bump)
    # --------------------------------------------------------------------------
    # Alloc A
    movq %r12, %rdi
    call pool_alloc
    testq %rax, %rax
    jz .fail_alloc
    movq %rax, %r13 # R13 = Ptr A

    # Alloc B
    movq %r12, %rdi
    call pool_alloc
    testq %rax, %rax
    jz .fail_alloc
    movq %rax, %r14 # R14 = Ptr B

    # Alloc C (Should FAIL - OOM)
    movq %r12, %rdi
    call pool_alloc
    testq %rax, %rax
    jnz .fail_oom_check # Should be 0

    # --------------------------------------------------------------------------
    # TEST 3: Free & Reuse
    # --------------------------------------------------------------------------
    # Free A
    movq %r12, %rdi
    movq %r13, %rsi # Ptr A
    call pool_free

    # Alloc D (Should reuse A)
    movq %r12, %rdi
    call pool_alloc

    testq %rax, %rax
    jz .fail_reuse

    # Check if D == A
    cmpq %r13, %rax
    jne .fail_reuse_ptr

    # --------------------------------------------------------------------------
    # TEST 4: Reuse again (Stack behavior)
    # --------------------------------------------------------------------------
    # Free B, Free A
    movq %r12, %rdi
    movq %r14, %rsi
    call pool_free

    movq %r12, %rdi
    movq %r13, %rsi
    call pool_free

    # Alloc E -> Should be A (Last Freed)
    movq %r12, %rdi
    call pool_alloc
    cmpq %r13, %rax
    jne .fail_lifo

    # Alloc F -> Should be B
    movq %r12, %rdi
    call pool_alloc
    cmpq %r14, %rax
    jne .fail_lifo

    OS_WRITE $1, msg_ok(%rip), $len_ok
    xorq %rax, %rax
    leave
    ret

.fail_create:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $1, %rax
    leave
    ret

.fail_alloc:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $2, %rax
    leave
    ret

.fail_oom_check:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $3, %rax
    leave
    ret

.fail_reuse:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $4, %rax
    leave
    ret

.fail_reuse_ptr:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $5, %rax
    leave
    ret

.fail_lifo:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $6, %rax
    leave
    ret
