# corelib/platform/x86_64/asm/test_lexer.s
# Test Suite untuk Lexer

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
input_str: .ascii "123 + x"
input_len: .quad 7

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

    # 2. Token 1: INTEGER 123
    movq %r12, %rdi
    call lexer_next_token
    # [0]=Type, [8]=Value
    cmpq $1, 0(%rax) # TOKEN_INTEGER
    jne .fail
    cmpq $123, 8(%rax)
    jne .fail

    # 3. Token 2: OPERATOR '+'
    movq %r12, %rdi
    call lexer_next_token
    cmpq $3, 0(%rax) # TOKEN_OPERATOR
    jne .fail
    cmpq $'+', 8(%rax)
    jne .fail

    # 4. Token 3: IDENTIFIER "x"
    movq %r12, %rdi
    call lexer_next_token
    cmpq $2, 0(%rax) # TOKEN_IDENTIFIER
    jne .fail
    # Check String Struct
    movq 8(%rax), %rcx # Ptr to String Struct
    cmpq $1, 0(%rcx)   # Len 1
    jne .fail
    movq 8(%rcx), %rdx # Ptr Data
    movzbq (%rdx), %r8
    cmpb $'x', %r8b
    jne .fail

    # 5. Token 4: EOF
    movq %r12, %rdi
    call lexer_next_token
    cmpq $0, 0(%rax) # TOKEN_EOF
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
