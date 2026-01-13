; corelib/platform/x86_64/asm_win/runtime.s
; Implementasi Runtime MorphFox untuk Windows x86_64
; Menggunakan Macro SSOT (macros.inc) untuk penyederhanaan logika

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .text
global _start
global __sys_write
global __sys_exit
global __mem_map
global __mem_unmap

; Import simbol eksternal (macro mungkin sudah declare, tapi aman double declare)
extern ExitProcess
extern WriteFile
extern GetStdHandle
extern GetSystemInfo
extern VirtualAlloc
extern VirtualFree
extern main
extern __sys_page_size

; ==============================================================================
; ENTRY POINT
; ==============================================================================
_start:
    ; 1. Inisialisasi Stack Alignment
    and rsp, -16

    ; Allocate Stack:
    ; 32 bytes (Shadow Space) + 48 bytes (SYSTEM_INFO struct) = 80 bytes
    sub rsp, 80

    ; 2. Deteksi Page Size (GetSystemInfo)
    ; Pointer struct di [rsp + 32]
    lea rcx, [rsp + 32]
    call GetSystemInfo

    ; Ambil dwPageSize (Offset 4 dari awal struct)
    mov eax, dword [rsp + 36]
    mov [rel __sys_page_size], rax

    ; 3. Dapatkan Handle StdOut
    ;    GetStdHandle(STD_OUTPUT_HANDLE)
    mov rcx, STD_OUTPUT_HANDLE
    call GetStdHandle
    mov [rel stdout_handle], rax

    ; 4. Panggil User Entry Point
    call main

    ; 4. Exit dengan kode return main (rax)
    OS_EXIT rax

    ; Unreachable
    hlt

; ==============================================================================
; SYSCALL WRAPPERS
; ==============================================================================

; __sys_write(fd, buffer, length)
; Input: rcx=fd, rdx=buffer, r8=length
__sys_write:
    ; Cek apakah fd == 1 (stdout simulasi)
    cmp rcx, 1
    jne .invalid_fd

    ; Gunakan Macro OS_WRITE
    ; Kita perlu passing handle, bukan fd integer 1.
    mov rax, [rel stdout_handle]

    ; OS_WRITE Handle, Buffer, Length
    OS_WRITE rax, rdx, r8

    ; Return jumlah byte yang ditulis (disimpan macro di [rsp+something]?)
    ; Macro wrapper kita simplifikasi return value.
    ; Asumsikan sukses = length requested sementara.
    mov rax, r8
    ret

.invalid_fd:
    mov rax, -9
    ret

; __sys_exit(code)
__sys_exit:
    OS_EXIT rcx
    ret

; __mem_map(size)
__mem_map:
    ; Wrapper sementara. Macro OS_ALLOC_PAGE alloc 1 page fixed.
    ; Nanti akan digantikan oleh allocator.
    OS_ALLOC_PAGE
    ret

; __mem_unmap(ptr, size)
__mem_unmap:
    sub rsp, 32
    xor rdx, rdx        ; dwSize = 0 (MEM_RELEASE)
    mov r8, 0x8000      ; MEM_RELEASE
    call VirtualFree
    add rsp, 32
    ret

section .bss
    stdout_handle resq 1
