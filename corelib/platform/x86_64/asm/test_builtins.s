# corelib/platform/x86_64/asm/test_builtins.s
# Program Test untuk Builtins Library

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
msg_start: .ascii "Testing Builtins...\n"
.set len_start, . - msg_start

.section .text
.global _start

_start:
    # 1. Print Start Message
    movq $msg_start, %rdi
    movq $len_start, %rsi
    call __mf_print_str

    # 2. Print Positive Integer (12345)
    movq $12345, %rdi
    call __mf_print_int

    # 3. Print Negative Integer (-9876)
    movq $-9876, %rdi
    call __mf_print_int

    # 4. Print Zero (0)
    movq $0, %rdi
    call __mf_print_int

    # 5. Print Max Int (Small test)
    movq $999, %rdi
    call __mf_print_int

    # Return 0
    OS_EXIT $0
