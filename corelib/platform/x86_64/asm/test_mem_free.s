# corelib/platform/x86_64/asm/test_mem_free.s
# Test Suite untuk Logika mem_free (Linux)

.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global main

main:
    pushq %rbp
    movq %rsp, %rbp

    # 1. Alokasi 100 bytes
    movq $100, %rdi
    call mem_alloc

    # Simpan pointer di R12
    movq %rax, %r12

    # Cek Validitas Pointer
    testq %rax, %rax
    jz .fail

    # Cek Ukuran di Header (harus positif)
    # Header ada di ptr - 8
    movq -8(%r12), %r13
    cmpq $0, %r13
    jle .fail

    # 2. Panggil mem_free
    movq %r12, %rdi
    call mem_free

    # 3. Verifikasi Ukuran menjadi Negatif
    movq -8(%r12), %r13
    cmpq $0, %r13
    jge .fail # Harusnya negatif

    # 4. Tes Double Free (Harus tetap negatif / tidak berubah)
    movq %r12, %rdi
    call mem_free

    movq -8(%r12), %r13
    cmpq $0, %r13
    jge .fail

    # Sukses
    movq $0, %rax
    leave
    ret

.fail:
    movq $1, %rax
    leave
    ret
