; corelib/platform/x86_64/asm_win/alloc.asm
; Implementasi Morph Allocator (Windows x86_64)
; Sesuai SSOT corelib/core/memory.fox

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    global __sys_page_size
    __sys_page_size  dq 4096 ; Fallback default for Windows
    global current_page_ptr
    current_page_ptr dq 0
    current_offset   dq 0

section .text
global mem_alloc
global mem_free
global mem_reset

; Konfigurasi Header
PAGE_HEADER_SIZE equ 48
BLOCK_HEADER_SIZE equ 8

; ------------------------------------------------------------------------------
; func mem_alloc(user_size: i64) -> ptr
; Input: RCX = user_size
; Output: RAX = pointer
; ------------------------------------------------------------------------------
mem_alloc:
    push rbp
    mov rbp, rsp
    push rbx                ; Simpan R12 (Callee-saved) -> Using RBX as stable reg
    push r12
    push r13                ; Simpan Total Aligned Size

    ; 1. Simpan User Size di RBX
    mov rbx, rcx

    ; 2. Hitung Total Size (User Size + Header 8)
    mov rax, rcx
    add rax, BLOCK_HEADER_SIZE

    ; 3. Round Up ke kelipatan 16
    add rax, 15
    and rax, -16
    mov r13, rax            ; R13 = Total Aligned Size

    ; 4. Ambil Page Info
    mov rax, [rel current_page_ptr]
    test rax, rax
    jz .new_page_needed

    ; 5. Cek Kapasitas Page Aktif
    mov rdx, [rel current_offset] ; Old Offset
    mov r8, rdx
    add r8, r13             ; New Offset

    ; Bandingkan dengan __sys_page_size
    mov r9, [rel __sys_page_size]
    cmp r8, r9
    jg .new_page_needed

    ; 6. Alokasi Berhasil
    ; RAX = Page Base
    add rax, rdx            ; RAX = Block Start (Header Ptr)

    ; Tulis Header
    mov [rax], rbx          ; User Size

    ; Zero Padding loop
    push rdi
    push rcx
    lea rdi, [rax + 8]      ; Dest
    mov rcx, r13
    sub rcx, 8              ; Count
    xor r10, r10            ; Zero
.L_zero:
    mov [rdi], r10b
    inc rdi
    dec rcx
    jnz .L_zero
    pop rcx
    pop rdi

    ; Update Offset
    mov [rel current_offset], r8

    ; Return User Ptr
    add rax, 8

    pop r13
    pop r12
    pop rbx
    leave
    ret

.new_page_needed:
    ; Butuh Page Baru
    mov rax, r13
    add rax, PAGE_HEADER_SIZE

    ; Round up ke Page Size
    mov rcx, [rel __sys_page_size]
    mov r8, rcx
    dec r8
    add rax, r8
    mov r8, rcx
    neg r8
    and rax, r8

    mov r12, rax    ; Save Alloc Size in R12 (Need to push/pop R12? Yes done in prologue)

    ; Alloc Page
    OS_ALLOC_PAGE rax
    test rax, rax
    jz .alloc_fail

    ; Init Page Header
    mov qword [rax + 0], 0   ; Next

    mov r8, [rel current_page_ptr]
    mov [rax + 8], r8        ; Prev

    test r8, r8
    jz .no_prev
    mov [r8 + 0], rax
.no_prev:

    ; [0x10] Timestamp (GetSystemTimeAsFileTime)
    push rax            ; Simpan Pointer Page
    lea rcx, [rax + 16] ; Arg 1: Pointer ke field timestamp
    sub rsp, 32         ; Shadow Space
    call GetSystemTimeAsFileTime
    add rsp, 32
    pop rax             ; Restore Pointer Page

    ; [0x18] Page Size
    mov [rax + 24], r12

    ; [0x20] Magic: "VZOELFOX"
    mov r9, 0x584F464C454F5A56
    mov [rax + 32], r9

    ; Update Global
    mov [rel current_page_ptr], rax
    mov qword [rel current_offset], PAGE_HEADER_SIZE

    ; Retry Alloc on new page
    lea rax, [rax + 48]      ; Block Start (Base + 48)

    ; Write Header
    mov [rax], rbx          ; User Size (RBX)

    ; Update Offset
    mov rdx, PAGE_HEADER_SIZE
    add rdx, r13
    mov [rel current_offset], rdx

    ; Return User Ptr
    add rax, 8

    pop r13
    pop r12
    pop rbx
    leave
    ret

.alloc_fail:
    xor rax, rax
    pop r13
    pop r12
    pop rbx
    leave
    ret

; ------------------------------------------------------------------------------
; func mem_free(ptr: ptr) -> void
; Input: RCX=ptr
; ------------------------------------------------------------------------------
mem_free:
    test rcx, rcx
    jz .free_return

    mov rax, [rel __sys_page_size]
    dec rax
    not rax
    mov rdx, rcx
    and rdx, rax    ; RDX = Page Header Ptr

    ; Check Magic at 32
    mov r8, 0x584F464C454F5A56
    cmp qword [rdx + 32], r8
    jne .free_return

    cmp qword [rcx - 8], 0
    jle .free_return

    neg qword [rcx - 8]

.free_return:
    ret

; ------------------------------------------------------------------------------
; func mem_reset() -> void
; ------------------------------------------------------------------------------
mem_reset:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13

    mov r12, [rel current_page_ptr]
    test r12, r12
    jz .rst_done

.rst_loop:
    mov r13, [r12 + 8]  ; Prev
    test r13, r13
    jz .rst_last

    ; Unmap Current (R12)
    mov rcx, r12
    xor rdx, rdx
    mov r8, 0x8000      ; MEM_RELEASE

    sub rsp, 32
    call VirtualFree
    add rsp, 32

    mov r12, r13        ; Prev becomes Current
    jmp .rst_loop

.rst_last:
    mov qword [r12 + 0], 0   ; Next = 0
    mov qword [rel current_offset], PAGE_HEADER_SIZE
    mov [rel current_page_ptr], r12

    ; Timestamp
    push r12
    lea rcx, [r12 + 16]
    sub rsp, 32
    call GetSystemTimeAsFileTime
    add rsp, 32
    pop r12

.rst_done:
    pop r13
    pop r12
    pop rbx
    leave
    ret
