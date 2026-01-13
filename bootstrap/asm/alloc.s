# bootstrap/asm/alloc.s
# Implementasi Morph Allocator (Linux x86_64)
# Sesuai SSOT corelib/core/memory.fox

.include "bootstrap/asm/macros.inc"

.section .data
    # State global
    .global current_page_ptr
    .global current_offset
    current_page_ptr: .quad 0
    current_offset:   .quad 0

.section .text
.global mem_alloc
.global mem_free
.global mem_reset

# Konfigurasi Header (Sesuai SSOT)
.equ PAGE_HEADER_SIZE, 48
.equ BLOCK_HEADER_SIZE, 8

# ------------------------------------------------------------------------------
# func mem_alloc(user_size: i64) -> ptr
# Input: %rdi = user_size
# Output: %rax = pointer ke user data
# ------------------------------------------------------------------------------
mem_alloc:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12          # Simpan R12 (Callee-saved)
    pushq %r13          # Simpan R13 (Total Size)

    # 1. Simpan User Size di R12 (untuk Header)
    movq %rdi, %r12

    # 2. Hitung Total Size (User Size + Header 8)
    movq %rdi, %rax
    addq $BLOCK_HEADER_SIZE, %rax

    # 3. Round Up ke kelipatan 16
    addq $15, %rax
    andq $-16, %rax
    movq %rax, %r13     # R13 = Total Aligned Size

    # 4. Ambil Page Info
    movq current_page_ptr(%rip), %rax
    testq %rax, %rax
    jz .new_page_needed

    # 5. Cek Kapasitas Page Aktif
    movq current_offset(%rip), %rcx # Old Offset
    movq %rcx, %rdx
    addq %r13, %rdx     # New Offset

    # Bandingkan dengan __sys_page_size
    cmpq __sys_page_size(%rip), %rdx
    jg .new_page_needed

    # 6. Alokasi Berhasil di Page Ini
    # %rax = Page Base
    addq %rcx, %rax     # %rax = Block Start (Header Ptr)

    # Tulis Header (User Size)
    movq %r12, (%rax)

    # Zero Out Padding loop
    pushq %rdi
    pushq %rcx
    leaq 8(%rax), %rdi  # Dest = User Ptr
    movq %r13, %rcx
    subq $8, %rcx       # Count = Total - 8
    xorq %r8, %r8
.L_zero:
    movb %r8b, (%rdi)
    incq %rdi
    decq %rcx
    jnz .L_zero

    popq %rcx
    popq %rdi

    # Update Offset
    movq %rdx, current_offset(%rip)

    # Return User Ptr (Block Start + 8)
    addq $8, %rax

    popq %r13
    popq %r12
    leave
    ret

.new_page_needed:
    # Butuh Page Baru
    # Hitung size yang diminta ke OS
    # Total = Total Aligned Block Size + PAGE HEADER
    movq %r13, %rax
    addq $PAGE_HEADER_SIZE, %rax

    # Round up ke kelipatan Page Size
    movq __sys_page_size(%rip), %rcx
    movq %rcx, %r8
    decq %r8            # PageSize - 1
    addq %r8, %rax      # Total + (PageSize - 1)
    movq %rcx, %r8
    negq %r8            # -PageSize
    andq %r8, %rax      # Rounded Size (Actual Page Size to Alloc)

    movq %rax, %r9      # Save Actual Alloc Size in R9

    # Alloc Page
    pushq %r9           # Save R9 (Clobbered by Macro)
    OS_ALLOC_PAGE %rax
    popq %r9            # Restore R9

    testq %rax, %rax
    js .alloc_fail

    # Setup Page Header
    # [0x00] Next = 0
    movq $0, 0(%rax)

    # [0x08] Prev = Current Page
    movq current_page_ptr(%rip), %r8
    movq %r8, 8(%rax)

    # Linking
    testq %r8, %r8
    jz .no_prev
    movq %rax, 0(%r8)   # Old->Next = New
.no_prev:

    # [0x10] Timestamp
    pushq %rax      # Save Page Ptr
    pushq %r12      # Save User Size
    pushq %rcx
    pushq %r11
    pushq %r9       # Save Alloc Size
    movq $SYS_TIME, %rax
    xorq %rdi, %rdi
    syscall
    movq %rax, %r8  # Timestamp
    popq %r9
    popq %r11
    popq %rcx
    popq %r12       # Restore User Size
    popq %rax       # Restore Page Ptr
    movq %r8, 16(%rax)

    # [0x18] Page Size
    movq %r9, 24(%rax)

    # [0x20] Magic: "VZOELFOX"
    movabsq $0x584F464C454F5A56, %rcx
    movq %rcx, 32(%rax)

    # Update Global State
    movq %rax, current_page_ptr(%rip)
    movq $PAGE_HEADER_SIZE, current_offset(%rip)

    # Retry alloc (Calculate pointers)
    leaq 48(%rax), %rax # Base + 48

    # Finalize
    movq %r12, (%rax) # User Size Header

    # Update Offset
    movq $PAGE_HEADER_SIZE, %rdx
    addq %r13, %rdx
    movq %rdx, current_offset(%rip)

    addq $8, %rax

    popq %r13
    popq %r12
    leave
    ret

.alloc_fail:
    xorq %rax, %rax
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func mem_free(ptr: ptr) -> void
# ------------------------------------------------------------------------------
mem_free:
    testq %rdi, %rdi
    jz .free_return

    movq __sys_page_size(%rip), %rcx
    decq %rcx
    notq %rcx
    movq %rdi, %rdx
    andq %rcx, %rdx

    # Check Magic at 32
    movabsq $0x584F464C454F5A56, %rax
    cmpq %rax, 32(%rdx)
    jne .free_return

    cmpq $0, -8(%rdi)
    jle .free_return
    negq -8(%rdi)

.free_return:
    ret

# ------------------------------------------------------------------------------
# func mem_reset() -> void
# ------------------------------------------------------------------------------
mem_reset:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13

    movq current_page_ptr(%rip), %r12
    testq %r12, %r12
    jz .rst_done

.rst_loop:
    movq 8(%r12), %r13  # Prev
    testq %r13, %r13
    jz .rst_last

    # Unmap Current (%r12)
    movq 24(%r12), %rsi # Size
    movq %r12, %rdi     # Ptr

    pushq %r13          # Save Prev
    movq $11, %rax      # SYS_MUNMAP
    syscall
    popq %r12           # Prev becomes Current
    jmp .rst_loop

.rst_last:
    movq $0, 0(%r12)    # Next = 0
    movq $PAGE_HEADER_SIZE, current_offset(%rip)
    movq %r12, current_page_ptr(%rip)

    # Update Timestamp
    movq $SYS_TIME, %rax
    xorq %rdi, %rdi
    syscall
    movq %rax, 16(%r12)

.rst_done:
    popq %r13
    popq %r12
    leave
    ret
