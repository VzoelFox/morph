# corelib/platform/x86_64/asm/test_lexer_advanced.s
# Test Suite untuk Lexer Advanced (Multi-char ops, Comments, Strings)

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
# Input: "== != ; komentar\n \"hello\""
# Hex:
# == : 3D 3D
# Space : 20
# != : 21 3D
# Space : 20
# ; komentar\n : 3B ... 0A
# Space : 20
# "hello" : 22 68 65 6C 6C 6F 22

input_str: .ascii "== != ; komentar\n \"hello\""
input_len: .quad 23

.section .text
.global main

main:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12 # Lexer Ptr

    # 1. Create Lexer
    leaq input_str(%rip), %rdi
    movq input_len(%rip), %rsi
    call lexer_create

    testq %rax, %rax
    jz .fail
    movq %rax, %r12

    # 2. Token 1: OPERATOR '==' (0x3D3D)
    movq %r12, %rdi
    call lexer_next_token
    cmpq $3, 0(%rax)    # TOKEN_OPERATOR
    jne .fail
    cmpq $0x3D3D, 8(%rax)
    jne .fail

    # 3. Token 2: OPERATOR '!=' (0x3D21) -> Little Endian for '!=' (0x21, 0x3D) is 0x3D21
    movq %r12, %rdi
    call lexer_next_token
    cmpq $3, 0(%rax)    # TOKEN_OPERATOR
    jne .fail
    cmpq $0x3D21, 8(%rax)
    jne .fail

    # 4. Token 3: STRING "hello"
    # Note: The "; komentar\n" should be skipped automatically
    movq %r12, %rdi
    call lexer_next_token
    cmpq $5, 0(%rax)    # TOKEN_STRING
    jne .fail

    # Check String Content
    movq 8(%rax), %rcx  # String Struct
    cmpq $5, 0(%rcx)    # Len 5
    jne .fail

    movq 8(%rcx), %rdx  # Data Ptr
    movzbq (%rdx), %r8
    cmpb $'h', %r8b
    jne .fail

    # 5. Token 4: EOF
    movq %r12, %rdi
    call lexer_next_token
    cmpq $0, 0(%rax)    # TOKEN_EOF
    jne .fail

    # Success
    movq $0, %rax
    popq %r12
    leave
    ret

.fail:
    movq $1, %rax
    popq %r12
    leave
    ret
