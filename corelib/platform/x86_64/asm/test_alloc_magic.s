# corelib/platform/x86_64/asm/test_alloc_magic.s
# Test Suite untuk Memverifikasi Custom Magic Header "V Z O E L F O XS"

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
    # Page Size dummy (akan diisi runtime)
    # Tapi kita perlu akses, jadi kita asumsikan __sys_page_size global

.section .text
.global main

main:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12

    # 1. Alokasi Memori (Cukup besar untuk force page baru jika perlu, tapi page pertama pasti baru)
    movq $64, %rdi
    call mem_alloc

    testq %rax, %rax
    jz .fail
    movq %rax, %r12     # User Ptr

    # 2. Hitung Page Header Address
    # Page Header = UserPtr & ~(PageSize - 1)
    movq __sys_page_size(%rip), %rcx
    decq %rcx
    notq %rcx

    movq %r12, %rdx
    andq %rcx, %rdx     # RDX = Page Header Ptr

    # 3. Verifikasi Magic (Offset 24)
    # Expected: "VZOELFOX" -> 0x584F464C454F5A56
    movabsq $0x584F464C454F5A56, %rcx
    cmpq %rcx, 24(%rdx)
    jne .fail

    # 5. Free Memori (untuk memastikan mem_free tidak crash dengan magic baru)
    movq %r12, %rdi
    call mem_free

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
