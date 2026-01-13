# corelib/platform/x86_64/asm/test_string.s
# Test Suite untuk Operasi String

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
str_hello: .ascii "hello"
str_hello_len: .quad 5
str_world: .ascii "world"
str_world_len: .quad 5
str_hello2: .ascii "hello"

# Hash "hello" FNV-1a calculation:
# Offset = 0xcbf29ce484222325
# Prime  = 0x100000001b3
# 'h' (104): (Offset ^ 104) * Prime = ...
# manual pre-calc: 0xa430d84680aabd0b
expected_hash_hello: .quad 0xa430d84680aabd0b

.section .text
.global main

main:
    pushq %rbp
    movq %rsp, %rbp

    # TEST 1: Hash Calculation
    leaq str_hello(%rip), %rdi
    movq str_hello_len(%rip), %rsi
    call __mf_string_hash

    # Check Result
    cmpq expected_hash_hello(%rip), %rax
    jne .fail

    # TEST 2: Equality (Same String)
    leaq str_hello(%rip), %rdi
    movq str_hello_len(%rip), %rsi
    leaq str_hello2(%rip), %rdx
    movq str_hello_len(%rip), %rcx
    call __mf_string_equals

    cmpq $1, %rax
    jne .fail

    # TEST 3: Inequality (Different Content, Same Length)
    leaq str_hello(%rip), %rdi
    movq str_hello_len(%rip), %rsi
    leaq str_world(%rip), %rdx
    movq str_world_len(%rip), %rcx
    call __mf_string_equals

    cmpq $0, %rax
    jne .fail

    # TEST 4: Inequality (Different Length)
    # Compare "hello" len 5 with len 4
    leaq str_hello(%rip), %rdi
    movq str_hello_len(%rip), %rsi
    leaq str_hello2(%rip), %rdx
    movq $4, %rcx
    call __mf_string_equals

    cmpq $0, %rax
    jne .fail

    # Success
    movq $0, %rax
    leave
    ret

.fail:
    movq $1, %rax
    leave
    ret
