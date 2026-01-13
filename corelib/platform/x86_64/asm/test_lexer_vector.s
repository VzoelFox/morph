# corelib/platform/x86_64/asm/test_lexer_vector.s
# Test Integrasi: Lexer -> Vector -> Token List

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
input_code: .ascii "123 + x"
input_len: .quad 7

.section .text
.global main
.extern lexer_create
.extern lexer_next_token
.extern vector_new
.extern vector_push
.extern vector_get

main:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12  # Lexer Ptr
    pushq %r13  # Vector Ptr
    pushq %r14  # Temp

    # 1. Create Lexer
    leaq input_code(%rip), %rdi
    movq input_len(%rip), %rsi
    call lexer_create
    testq %rax, %rax
    jz .fail
    movq %rax, %r12

    # 2. Create Vector (ItemSize = 8, menyimpan POINTER ke Token)
    movq $8, %rdi
    call vector_new
    testq %rax, %rax
    jz .fail
    movq %rax, %r13

    # 3. Loop Tokenize
.token_loop:
    movq %r12, %rdi
    call lexer_next_token
    testq %rax, %rax
    jz .fail

    # Simpan Token Ptr (RAX) ke Stack untuk addressnya
    pushq %rax      # Stack now has Token Ptr

    # Push to Vector
    movq %r13, %rdi
    movq %rsp, %rsi # RSI = Address of Token Ptr in Stack
    call vector_push

    # Restore stack & Get Token Ptr back to R14
    popq %r14

    # Check if EOF (Type 0)
    cmpq $0, 0(%r14)
    je .loop_done

    jmp .token_loop

.loop_done:
    # 4. Verify
    # Expect 4 tokens: INT, OP, ID, EOF

    # Check Length
    movq 8(%r13), %rax
    cmpq $4, %rax
    jne .fail

    # Check Token 0 (INT 123)
    movq %r13, %rdi
    movq $0, %rsi
    call vector_get
    movq (%rax), %rcx # Get Token Ptr
    cmpq $1, 0(%rcx)  # Type INT
    jne .fail
    cmpq $123, 8(%rcx)
    jne .fail

    # Check Token 2 (ID 'x')
    movq %r13, %rdi
    movq $2, %rsi
    call vector_get
    movq (%rax), %rcx # Get Token Ptr
    cmpq $2, 0(%rcx)  # Type ID
    jne .fail

    # Success
    movq $0, %rax
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

.fail:
    movq $1, %rax
    popq %r14
    popq %r13
    popq %r12
    leave
    ret
