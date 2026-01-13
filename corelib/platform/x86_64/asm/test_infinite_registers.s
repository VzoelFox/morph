# corelib/platform/x86_64/asm/test_infinite_registers.s
# Test Capability: "Infinite Registers" via OP_PICK/OP_POKE
# This test manually constructs a Fragment that uses the Stack as a Register File.
# It pushes 10 values (Reg 0-9), then performs random access arithmetic.
# Expected Result: 42
# ==============================================================================

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/rpn_gas.inc"

.section .data
msg_start:   .asciz "Test Infinite Registers Started...\n"
msg_success: .asciz "Infinite Registers Verified: Result 42\n"
msg_fail:    .asciz "Infinite Registers Failed: Expected 42, Got "
msg_newline: .asciz "\n"

.section .text
.global main
.extern stack_new
.extern executor_run_with_stack
.extern mem_alloc
.extern __mf_print_int

main:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp

    # Print Start
    leaq msg_start(%rip), %rdi
    call print_cstr

    # 1. Create Fragment
    movq $32, %rdi
    call mem_alloc
    movq %rax, %r12 # Fragment Ptr

    # 2. Create Code Buffer
    movq $1024, %rdi
    call mem_alloc
    movq %rax, %r13 # Code Buffer

    movq %r13, 8(%r12) # Set Code Ptr

    # 3. Construct Code
    xorq %r15, %r15

    # Push 0..5
    movq $0, %rcx
.push_loop:
    movq $OP_LIT, 0(%r13, %r15)
    movq %rcx, 8(%r13, %r15)
    addq $16, %r15
    incq %rcx
    cmpq $6, %rcx
    jl .push_loop

    # PICK 3 (R2=2)
    movq $OP_LIT, 0(%r13, %r15)
    movq $3, 8(%r13, %r15)
    addq $16, %r15

    movq $OP_PICK, 0(%r13, %r15)
    movq $0, 8(%r13, %r15)
    addq $16, %r15

    # PICK 1 (R5=5, but depth changed!)
    movq $OP_LIT, 0(%r13, %r15)
    movq $1, 8(%r13, %r15)
    addq $16, %r15

    movq $OP_PICK, 0(%r13, %r15)
    movq $0, 8(%r13, %r15)
    addq $16, %r15

    # ADD (2 + 5 = 7)
    movq $OP_ADD, 0(%r13, %r15)
    movq $0, 8(%r13, %r15)
    addq $16, %r15

    # LIT 35
    movq $OP_LIT, 0(%r13, %r15)
    movq $35, 8(%r13, %r15)
    addq $16, %r15

    # ADD (7 + 35 = 42)
    movq $OP_ADD, 0(%r13, %r15)
    movq $0, 8(%r13, %r15)
    addq $16, %r15

    # EXIT
    movq $OP_EXIT, 0(%r13, %r15)
    movq $0, 8(%r13, %r15)
    addq $16, %r15

    movq %r15, 16(%r12) # Set Size

    # 4. Create Stack
    movq $1024, %rdi
    call stack_new
    movq %rax, %r14 # Stack

    # 5. Run
    movq %r12, %rdi
    movq %r14, %rsi
    call executor_run_with_stack

    # 6. Verify Top
    movq %r14, %rdi
    call stack_pop # Should be 42

    cmpq $42, %rax
    jne .fail

    # Success
    leaq msg_success(%rip), %rdi
    call print_cstr
    xorq %rax, %rax
    leave
    ret

.fail:
    pushq %rax
    leaq msg_fail(%rip), %rdi
    call print_cstr
    popq %rdi
    call __mf_print_int
    leaq msg_newline(%rip), %rdi
    call print_cstr
    movq $1, %rax
    leave
    ret

print_cstr:
    pushq %rsi
    pushq %rdx
    pushq %rcx
    movq %rdi, %rsi
    xorq %rdx, %rdx
.pc_len:
    cmpb $0, (%rsi, %rdx)
    je .pc_print
    incq %rdx
    jmp .pc_len
.pc_print:
    movq $1, %rax
    movq $1, %rdi
    syscall
    popq %rcx
    popq %rdx
    popq %rsi
    ret
