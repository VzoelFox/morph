# corelib/platform/x86_64/asm/test_alloc_big.s
# Test Case untuk Multi-Page Allocation
# Mengetes alokasi > 4096 bytes

.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global _start

_start:
    # 1. Alokasi kecil pertama (Warming up)
    movq $100, %rdi
    call mem_alloc
    # Simpan pointer ke r12
    movq %rax, %r12

    # Check if NULL
    testq %rax, %rax
    jz .fail

    # 2. Alokasi BESAR (10000 bytes) -> Butuh ~3 pages
    movq $10000, %rdi
    call mem_alloc
    # Simpan pointer ke r13
    movq %rax, %r13

    # Check if NULL
    testq %rax, %rax
    jz .fail

    # 3. Alokasi kecil lagi (harus sukses di page baru / sisa big page?)
    # Karena implementasi kita saat ini menset offset = size+header,
    # dan size 10000 > PAGE_SIZE (4096 constant check),
    # maka alokasi berikutnya AKAN memicu page baru standar.
    movq $50, %rdi
    call mem_alloc
    movq %rax, %r14

    # Check if NULL
    testq %rax, %rax
    jz .fail

    # Validasi Pointer Berbeda
    cmpq %r12, %r13
    je .fail
    cmpq %r13, %r14
    je .fail

    # Print Success Message
    OS_WRITE $1, msg_ok, $3
    OS_EXIT $0

.fail:
    OS_WRITE $1, msg_fail, $5
    OS_EXIT $1

.section .data
msg_ok:   .ascii "OK\n"
msg_fail: .ascii "FAIL\n"
