# corelib/platform/x86_64/asm/test_alloc_features.s
# Test Suite untuk Fitur Baru Allocator (Header, Alignment, Padding)

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
    msg_start: .ascii "Starting Alloc Features Test...\n"
    len_start = . - msg_start
    msg_ok:    .ascii "[OK] "
    len_ok = . - msg_ok
    msg_fail:  .ascii "[FAIL] "
    len_fail = . - msg_fail
    msg_nl:    .ascii "\n"
    len_nl = . - msg_nl

.section .text
.global main
.global mem_alloc

main:
    pushq %rbp
    movq %rsp, %rbp

    OS_WRITE $1, msg_start(%rip), $len_start

    # --------------------------------------------------------------------------
    # TEST 1: Header & Alignment
    # Request 10 bytes.
    # Expect:
    #   Header = 8 bytes.
    #   Total Need = 10 + 8 = 18.
    #   Aligned = 32.
    #   Padding = 32 - 8 - 10 = 14 bytes (Zeroed).
    # --------------------------------------------------------------------------
    movq $10, %rdi
    call mem_alloc
    testq %rax, %rax
    jz .fail_alloc

    movq %rax, %r12 # Ptr1

    # Verify Header (Ptr1 - 8) == 10
    movq -8(%r12), %rcx
    cmpq $10, %rcx
    jne .fail_header

    # Verify Padding Zeroing
    # Data is at 0..9. Padding at 10..21 (Wait. Total 32. Header 8. Data 10. Padding 14.)
    # Bytes 10 to 23 relative to Ptr1 should be 0?
    # Total Block 32.
    # [H:8][D:10][P:14]
    # Ptr points to D[0].
    # Padding starts at Ptr[10]. Ends at Ptr[23].

    # Check byte at Ptr[10]
    movb 10(%r12), %al
    testb %al, %al
    jnz .fail_padding

    # Check byte at Ptr[23] (Last byte of block)
    movb 23(%r12), %al
    testb %al, %al
    jnz .fail_padding

    # --------------------------------------------------------------------------
    # TEST 2: Block Spacing (Alignment)
    # Alloc another 10 bytes.
    # Expect Ptr2 = Ptr1 + 32.
    # --------------------------------------------------------------------------
    movq $10, %rdi
    call mem_alloc
    testq %rax, %rax
    jz .fail_alloc

    movq %rax, %r13 # Ptr2

    # Calculate Diff
    subq %r12, %rax # Ptr2 - Ptr1
    cmpq $32, %rax
    jne .fail_spacing

    OS_WRITE $1, msg_ok(%rip), $len_ok
    xorq %rax, %rax
    leave
    ret

.fail_alloc:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $1, %rax
    leave
    ret

.fail_header:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $2, %rax
    leave
    ret

.fail_padding:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $3, %rax
    leave
    ret

.fail_spacing:
    OS_WRITE $1, msg_fail(%rip), $len_fail
    movq $4, %rax
    leave
    ret
