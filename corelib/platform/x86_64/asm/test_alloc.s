# corelib/platform/x86_64/asm/test_alloc.s
# Program Test untuk Verifikasi Alokator

.include "corelib/platform/x86_64/asm/macros.inc"

.section .rodata
    msg_success: .ascii "Alokasi Sukses! Data: "
    msg_len_success = . - msg_success
    newline: .ascii "\n"

    # String yang akan disalin ke heap
    heap_data_src: .ascii "Hello Heap"
    heap_data_len = . - heap_data_src

.section .text
.global main
.global mem_alloc

# Entry point main dipanggil oleh runtime.s
main:
    pushq %rbp
    movq %rsp, %rbp

    # 1. Alokasi 16 bytes di Heap
    movq $16, %rdi
    call mem_alloc

    # Cek apakah NULL
    testq %rax, %rax
    jz .fail

    # Simpan pointer heap di r12
    movq %rax, %r12

    # 2. Tulis data ke memory heap (manual copy "Hello Heap")
    # "Hello He" (8 bytes)
    movq $0x6548206f6c6c6548, %rbx # Little Endian "Hello He" (reveresed)
    # Ah, manual byte move lebih aman biar tidak pusing endianness
    # "H"
    movb $0x48, 0(%r12)
    movb $0x65, 1(%r12) # e
    movb $0x6c, 2(%r12) # l
    movb $0x6c, 3(%r12) # l
    movb $0x6f, 4(%r12) # o

    # 3. Print pesan sukses
    OS_WRITE $1, msg_success, $msg_len_success

    # 4. Print isi heap
    OS_WRITE $1, (%r12), $5

    # 5. Print newline
    OS_WRITE $1, newline, $1

    # Exit Success
    movq $0, %rax
    leave
    ret

.fail:
    # Exit Fail (1)
    movq $1, %rax
    leave
    ret
