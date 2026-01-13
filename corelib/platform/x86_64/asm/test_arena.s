# corelib/platform/x86_64/asm/test_arena.s
# Test Suite untuk Memory Arena

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
    msg_start: .ascii "Starting Arena Test...\n"
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

    # Print Start
    OS_WRITE $1, msg_start(%rip), $len_start

    # --------------------------------------------------------------------------
    # TEST 1: Arena Creation
    # --------------------------------------------------------------------------
    # Create Arena with size 1024
    movq $1024, %rdi
    call arena_create

    # Check result (should not be 0)
    testq %rax, %rax
    jz .fail_create

    movq %rax, %r12 # Save Arena Ptr to R12

    # Print Arena Address (Debug)
    movq %r12, %rdi
    call __mf_print_int
    OS_WRITE $1, msg_nl(%rip), $len_nl

    # Verify Capacity (Should be >= 1024 - 32 = 992)
    movq %r12, %rdi
    call arena_get_capacity
    movq %rax, %r15     # Save capacity in R15

    # Print Capacity
    movq %rax, %rdi
    call __mf_print_int
    OS_WRITE $1, msg_nl(%rip), $len_nl

    cmpq $992, %r15
    jl .fail_cap

    # --------------------------------------------------------------------------
    # TEST 2: Allocation
    # --------------------------------------------------------------------------
    # Alloc 100 bytes
    movq %r12, %rdi
    movq $100, %rsi
    call arena_alloc

    testq %rax, %rax
    jz .fail_alloc

    movq %rax, %r13 # Save Ptr1

    # Check Usage (Should be 100)
    movq %r12, %rdi
    call arena_get_usage
    cmpq $100, %rax
    jne .fail_usage

    # Alloc 200 bytes
    movq %r12, %rdi
    movq $200, %rsi
    call arena_alloc

    testq %rax, %rax
    jz .fail_alloc

    movq %rax, %r14 # Save Ptr2

    # Check Usage (Should be 300)
    movq %r12, %rdi
    call arena_get_usage
    cmpq $300, %rax
    jne .fail_usage

    # --------------------------------------------------------------------------
    # TEST 3: Reset & Reuse
    # --------------------------------------------------------------------------
    # Reset Arena
    movq %r12, %rdi
    call arena_reset

    # Check Usage (Should be 0)
    movq %r12, %rdi
    call arena_get_usage
    cmpq $0, %rax
    jne .fail_reset

    # Alloc 100 bytes again
    movq %r12, %rdi
    movq $100, %rsi
    call arena_alloc

    # Ptr should be equal to Ptr1 (Reuse)
    cmpq %r13, %rax
    jne .fail_reuse

    # Success!
    OS_WRITE $1, msg_ok(%rip), $len_ok
    xorq %rax, %rax
    leave
    ret

.fail_create:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $1, %rax
    leave
    ret

.fail_cap:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $2, %rax
    leave
    ret

.fail_alloc:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $3, %rax
    leave
    ret

.fail_usage:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $4, %rax
    leave
    ret

.fail_reset:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $5, %rax
    leave
    ret

.fail_reuse:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $6, %rax
    leave
    ret
