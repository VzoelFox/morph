; corelib/platform/x86_64/asm_win/test_mem_free.asm
; Test Suite untuk Logika mem_free (Windows)

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .text
global main
extern mem_alloc
extern mem_free
extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 32 ; Shadow Space

    ; 1. Alokasi 100 bytes
    mov rcx, 100
    call mem_alloc

    ; Simpan pointer di RBX (stable)
    mov rbx, rax

    ; Cek Validitas Pointer
    test rax, rax
    jz .fail

    ; Cek Ukuran di Header (harus positif)
    ; Header ada di ptr - 8 ([rbx - 8])
    cmp qword [rbx - 8], 0
    jle .fail

    ; 2. Panggil mem_free
    mov rcx, rbx
    call mem_free

    ; 3. Verifikasi Ukuran menjadi Negatif
    cmp qword [rbx - 8], 0
    jge .fail ; Harusnya negatif

    ; 4. Tes Double Free (Harus tetap negatif / tidak berubah)
    mov rcx, rbx
    call mem_free

    cmp qword [rbx - 8], 0
    jge .fail

    ; Sukses
    xor rax, rax
    add rsp, 32
    pop rbp
    ret

.fail:
    mov rax, 1
    add rsp, 32
    pop rbp
    ret
