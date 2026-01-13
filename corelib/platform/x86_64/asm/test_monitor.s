# corelib/platform/x86_64/asm/test_monitor.s
# Test Integration untuk Monitoring & Signal Handler

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
msg: .asciz "Test Monitor Running...\n"
dot: .asciz "."

.section .text
.global main
.extern mem_alloc

main:
    # 1. Print Start Msg
    movq $SYS_WRITE, %rax
    movq $1, %rdi
    leaq msg(%rip), %rsi
    movq $22, %rdx
    syscall

    # 2. Allocate memory (Trigger Allocator Timestamp)
    movq $128, %rdi
    call mem_alloc
    # Ignore result, we just want the side effect (timestamp in page header)

    # 3. Loop waiting for signals
.loop:
    # sys_pause (34) waits for signal
    movq $SYS_PAUSE, %rax
    syscall

    # Print dot to indicate wake up
    movq $SYS_WRITE, %rax
    movq $1, %rdi
    leaq dot(%rip), %rsi
    movq $1, %rdx
    syscall

    jmp .loop
