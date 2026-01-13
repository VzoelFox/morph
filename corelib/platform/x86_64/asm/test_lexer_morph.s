# corelib/platform/x86_64/asm/test_lexer_morph.s
# Test Suite untuk Lexer dengan Sintaks Morph (Keywords & Delimiters)

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
# "fungsi init() var x = 10 tutup_fungsi"
# Keywords: fungsi, var, tutup_fungsi
# Delimiter: (, )
# Op: =
# Int: 10
input_str: .ascii "fungsi init() var x = 10 tutup_fungsi"
input_len: .quad 37

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

    # 2. Token 1: KEYWORD 'fungsi'
    movq %r12, %rdi
    call lexer_next_token
    cmpq $4, 0(%rax)    # TOKEN_KEYWORD
    jne .fail

    # 3. Token 2: IDENTIFIER 'init'
    movq %r12, %rdi
    call lexer_next_token
    cmpq $2, 0(%rax)    # TOKEN_IDENTIFIER
    jne .fail

    # 4. Token 3: DELIMITER '('
    movq %r12, %rdi
    call lexer_next_token
    cmpq $6, 0(%rax)    # TOKEN_DELIMITER
    jne .fail
    cmpq $'(', 8(%rax)
    jne .fail

    # 5. Token 4: DELIMITER ')'
    movq %r12, %rdi
    call lexer_next_token
    cmpq $6, 0(%rax)    # TOKEN_DELIMITER
    jne .fail
    cmpq $')', 8(%rax)
    jne .fail

    # 6. Token 5: KEYWORD 'var'
    movq %r12, %rdi
    call lexer_next_token
    cmpq $4, 0(%rax)    # TOKEN_KEYWORD
    jne .fail

    # 7. Token 6: IDENTIFIER 'x'
    movq %r12, %rdi
    call lexer_next_token
    cmpq $2, 0(%rax)    # TOKEN_IDENTIFIER
    jne .fail

    # 8. Token 7: OPERATOR '='
    movq %r12, %rdi
    call lexer_next_token
    cmpq $3, 0(%rax)    # TOKEN_OPERATOR
    jne .fail

    # 9. Token 8: INTEGER 10
    movq %r12, %rdi
    call lexer_next_token
    cmpq $1, 0(%rax)    # TOKEN_INTEGER
    jne .fail
    cmpq $10, 8(%rax)
    jne .fail

    # 10. Token 9: KEYWORD 'tutup_fungsi'
    movq %r12, %rdi
    call lexer_next_token
    cmpq $4, 0(%rax)    # TOKEN_KEYWORD
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
