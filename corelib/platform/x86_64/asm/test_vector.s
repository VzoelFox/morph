# corelib/platform/x86_64/asm/test_vector.s
# Test Suite untuk Vector (Linux)

.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global main
.extern vector_new
.extern vector_push
.extern vector_get

main:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12  # Vector Ptr
    pushq %r13  # Loop Counter

    # 1. New Vector (ItemSize = 8)
    movq $8, %rdi
    call vector_new
    testq %rax, %rax
    jz .fail
    movq %rax, %r12

    # 2. Push Loop (20 items to trigger resize 8->16->32)
    movq $0, %r13
.push_loop:
    cmpq $20, %r13
    jge .check_items

    # Item = Loop Counter
    # vector_push(vec, &val)
    # Kita butuh address dari nilai. Simpan di stack?
    pushq %r13      # Simpan nilai di stack sementara
    movq %rsp, %rsi # RSI = Address of value
    movq %r12, %rdi # RDI = Vec

    call vector_push

    addq $8, %rsp   # Restore stack (discard pushed value)

    testq %rax, %rax # Check result 0
    jnz .fail

    incq %r13
    jmp .push_loop

.check_items:
    # 3. Check Capacity/Length
    # Length should be 20
    movq 8(%r12), %rax
    cmpq $20, %rax
    jne .fail

    # Capacity should be >= 20 (likely 32 if geometric 8->16->32)
    movq 16(%r12), %rax
    cmpq $20, %rax
    jl .fail

    # 4. Check Item Value at index 10 (Expect 10)
    movq %r12, %rdi
    movq $10, %rsi
    call vector_get

    testq %rax, %rax
    jz .fail

    movq (%rax), %rcx # Value
    cmpq $10, %rcx
    jne .fail

    # Success
    movq $0, %rax
    popq %r13
    popq %r12
    leave
    ret

.fail:
    movq $1, %rax
    popq %r13
    popq %r12
    leave
    ret
