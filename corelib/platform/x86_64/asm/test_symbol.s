# corelib/platform/x86_64/asm/test_symbol.s
# Test Suite untuk Symbol Table

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
key_x: .ascii "x"
len_x: .quad 1
key_y: .ascii "y"
len_y: .quad 1
key_z: .ascii "z"
len_z: .quad 1

val_x: .quad 10
val_y: .quad 20

.section .text
.global main

main:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12  # Table Ptr
    pushq %r13  # Capacity

    # 1. Create Table (Cap = 16)
    movq $16, %rdi
    call sym_table_create
    movq %rax, %r12
    movq $16, %r13

    # Check not null
    testq %r12, %r12
    jz .fail

    # 2. Put "x" = 10
    movq %r12, %rdi
    leaq key_x(%rip), %rsi
    movq len_x(%rip), %rdx
    movq val_x(%rip), %rcx
    movq %r13, %r8
    call sym_table_put

    # 3. Put "y" = 20
    movq %r12, %rdi
    leaq key_y(%rip), %rsi
    movq len_y(%rip), %rdx
    movq val_y(%rip), %rcx
    movq %r13, %r8
    call sym_table_put

    # 4. Get "x" -> Expect 10
    movq %r12, %rdi
    leaq key_x(%rip), %rsi
    movq len_x(%rip), %rdx
    movq %r13, %rcx
    call sym_table_get

    cmpq $10, %rax
    jne .fail

    # 5. Get "y" -> Expect 20
    movq %r12, %rdi
    leaq key_y(%rip), %rsi
    movq len_y(%rip), %rdx
    movq %r13, %rcx
    call sym_table_get

    cmpq $20, %rax
    jne .fail

    # 6. Get "z" -> Expect -1 (Not Found)
    movq %r12, %rdi
    leaq key_z(%rip), %rsi
    movq len_z(%rip), %rdx
    movq %r13, %rcx
    call sym_table_get

    cmpq $-1, %rax
    jne .fail

    # 7. Update "x" = 99
    movq %r12, %rdi
    leaq key_x(%rip), %rsi
    movq len_x(%rip), %rdx
    movq $99, %rcx
    movq %r13, %r8
    call sym_table_put

    # Get "x" -> Expect 99
    movq %r12, %rdi
    leaq key_x(%rip), %rsi
    movq len_x(%rip), %rdx
    movq %r13, %rcx
    call sym_table_get

    cmpq $99, %rax
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
