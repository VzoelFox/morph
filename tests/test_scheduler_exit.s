# tests/test_scheduler_exit.s
# Test Case: Verifikasi scheduler_exit_current
# Skenario:
# 1. Main Routine berjalan.
# 2. Spawn Routine B.
# 3. Routine B mencetak pesan dan exit.
# 4. Main Routine harus kembali berjalan.
# 5. Main Routine exit -> Process Exit.

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
msg_main_start: .asciz "[Main] Start\n"
msg_routine_b:  .asciz "[Routine B] Exit\n"
msg_main_back:  .asciz "[Main] Back\n"
msg_main_exit:  .asciz "[Main] Process Exit\n"

.section .text
.global main
.extern scheduler_init
.extern scheduler_spawn
.extern scheduler_yield
.extern scheduler_exit_current
.extern __mf_print_asciz
.extern __mf_runtime_init

main:
    call __mf_runtime_init
    call scheduler_init

    leaq msg_main_start(%rip), %rdi
    call __mf_print_asciz

    leaq routine_b_entry(%rip), %rdi
    movq $0, %rsi
    call scheduler_spawn

    call scheduler_yield

    # Back in Main
    leaq msg_main_back(%rip), %rdi
    call __mf_print_asciz

    leaq msg_main_exit(%rip), %rdi
    call __mf_print_asciz

    call scheduler_exit_current

    # Should not reach here
    movq $60, %rax
    movq $1, %rdi
    syscall

routine_b_entry:
    leaq msg_routine_b(%rip), %rdi
    call __mf_print_asciz
    call scheduler_exit_current
    hlt
