; corelib/platform/x86_64/asm_win/test_infinite_registers.asm
; Test Capability: "Infinite Registers" via OP_PICK/OP_POKE
; Windows Version (NASM)
; ==============================================================================

%include "corelib/platform/x86_64/asm_win/macros.inc"
%include "corelib/platform/x86_64/asm/rpn.inc"

section .data
    msg_start   db "Test Infinite Registers Started...", 10, 0
    msg_success db "Infinite Registers Verified: Result 42", 10, 0
    msg_fail    db "Infinite Registers Failed. Expected 42.", 10, 0

section .text
global main
extern stack_new
extern stack_pop
extern executor_run_with_stack
extern mem_alloc
extern __mf_print_str
extern __mf_print_int
extern __mf_runtime_init

main:
    push rbp
    mov rbp, rsp
    sub rsp, 48

    call __mf_runtime_init

    lea rcx, [rel msg_start]
    call __mf_print_str

    ; 1. Create Fragment
    mov rcx, 32
    call mem_alloc
    mov r12, rax

    ; 2. Create Buffer
    mov rcx, 1024
    call mem_alloc
    mov r13, rax
    mov [r12 + 8], r13

    ; 3. Construct Code
    xor r15, r15

    ; Push 0..5
    xor r8, r8
.push_loop:
    mov byte [r13 + r15], OP_LIT
    mov [r13 + r15 + 8], r8
    add r15, 16
    inc r8
    cmp r8, 6
    jl .push_loop

    ; PICK 3 (R2=2)
    mov byte [r13 + r15], OP_LIT
    mov qword [r13 + r15 + 8], 3
    add r15, 16
    mov byte [r13 + r15], OP_PICK
    mov qword [r13 + r15 + 8], 0
    add r15, 16

    ; PICK 1 (R5=5, depth 1 due to R2 on top)
    mov byte [r13 + r15], OP_LIT
    mov qword [r13 + r15 + 8], 1
    add r15, 16
    mov byte [r13 + r15], OP_PICK
    mov qword [r13 + r15 + 8], 0
    add r15, 16

    ; ADD (2+5=7)
    mov byte [r13 + r15], OP_ADD
    mov qword [r13 + r15 + 8], 0
    add r15, 16

    ; LIT 35
    mov byte [r13 + r15], OP_LIT
    mov qword [r13 + r15 + 8], 35
    add r15, 16

    ; ADD (7+35=42)
    mov byte [r13 + r15], OP_ADD
    mov qword [r13 + r15 + 8], 0
    add r15, 16

    ; EXIT
    mov byte [r13 + r15], OP_EXIT
    mov qword [r13 + r15 + 8], 0
    add r15, 16

    mov [r12 + 16], r15

    ; 4. Stack
    mov rcx, 1024
    call stack_new
    mov r14, rax

    ; 5. Run
    mov rcx, r12
    mov rdx, r14
    call executor_run_with_stack

    ; 6. Verify
    mov rcx, r14
    call stack_pop

    cmp rax, 42
    jne .fail

    lea rcx, [rel msg_success]
    call __mf_print_str
    xor rax, rax
    jmp .exit

.fail:
    lea rcx, [rel msg_fail]
    call __mf_print_str
    mov rcx, 1
    call __sys_exit

.exit:
    add rsp, 48
    pop rbp
    ret
