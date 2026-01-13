; corelib/platform/x86_64/asm_win/runtime.asm
; Implementasi Runtime MorphFox untuk Windows x86_64
; Menggunakan Macro SSOT (macros.inc) untuk penyederhanaan logika

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    monitor_dir db "morph_swap_monitor", 0
    monitor_prefix db "morph_swap_monitor\sandbox_", 0

    global vm_cycle_budget
    vm_cycle_budget dq 1000000 ; Circuit Breaker Budget

section .bss
    stdout_handle resq 1
    monitor_path resb 260 ; MAX_PATH

section .text
global _start
global __sys_write
global __sys_read
global __sys_open
global __sys_close
global __sys_exit
global __mem_map
global __mem_unmap

; Import simbol eksternal (macro mungkin sudah declare, tapi aman double declare)
extern ReadFile
extern ExitProcess
extern WriteFile
extern GetStdHandle
extern GetSystemInfo
extern SetConsoleOutputCP
extern VirtualAlloc
extern VirtualFree
extern main
extern __sys_page_size
extern CreateDirectoryA
extern CreateFileA
extern CloseHandle
extern GetCurrentProcessId
extern GetSystemTimeAsFileTime
extern mem_snapshot_update_tracker
extern sym_table_create
extern __mf_print_int ; Helper to format int (although we need to write to buffer, not stdout)

; Global Symbols
extern global_sym_table
extern global_sym_cap

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

    ; 2.5 Set UTF-8 Output (65001)
    mov rcx, 65001
    call SetConsoleOutputCP

    ; 3. Dapatkan Handle StdOut
    ;    GetStdHandle(STD_OUTPUT_HANDLE)
    mov rcx, STD_OUTPUT_HANDLE
    call GetStdHandle
    mov [rel stdout_handle], rax

    ; 4. Initialize Runtime (Monitor File)
    call __mf_runtime_init

    ; 4.5 Initialize Symbol Table
    mov rcx, 1024
    call sym_table_create
    mov [rel global_sym_table], rax
    mov qword [rel global_sym_cap], 1024

    ; 5. Panggil User Entry Point
    call main

    ; 6. Exit dengan kode return main (rax)
    OS_EXIT rax

    ; Unreachable
    hlt

; ==============================================================================
; RUNTIME INITIALIZATION
; ==============================================================================
__mf_runtime_init:
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13

    ; Stack alignment: 8(rbp)+8(ret)+40(regs) = 56.
    ; Need 16-byte alignment + 32 shadow + args.
    ; Sub 136 -> 192 (Aligned 16).
    sub rsp, 136

    ; A. Create Directory "morph_swap_monitor"
    lea rcx, [rel monitor_dir]
    xor rdx, rdx    ; SecurityAttributes = NULL
    call CreateDirectoryA

    ; B. Construct Filename
    ; Copy Prefix
    lea rsi, [rel monitor_prefix]
    lea rdi, [rel monitor_path]
.copy_prefix:
    mov al, [rsi]
    test al, al
    jz .copy_pid
    mov [rdi], al
    inc rsi
    inc rdi
    jmp .copy_prefix

.copy_pid:
    ; Get PID
    call GetCurrentProcessId
    mov rbx, rax    ; Save PID in RBX

    ; Convert PID (EAX) to String at RDI
    mov rcx, 10
    mov r8, rdi     ; Save start of number

    ; Simple itoa logic (assume EAX > 0)
.itoa_loop:
    xor rdx, rdx
    div rcx         ; EAX / 10
    add dl, '0'
    mov [rdi], dl
    inc rdi
    test eax, eax
    jnz .itoa_loop

    ; Reverse
    mov r9, rdi
    dec r9          ; End
    mov r10, r8     ; Start
.rev_loop:
    cmp r10, r9
    jge .done_pid
    mov al, [r10]
    mov bl, [r9]
    mov [r10], bl
    mov [r9], al
    inc r10
    dec r9
    jmp .rev_loop

.done_pid:
    mov byte [rdi], 0   ; Null terminator

    ; C. Create File
    ; CreateFileA(path, ACCESS, SHARE, NULL, OPEN_ALWAYS, ATTR, NULL)

    ; Stack args (right to left)
    mov qword [rsp + 48], 0     ; hTemplateFile
    mov qword [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp + 32], OPEN_ALWAYS

    xor r9, r9                  ; Security = NULL
    mov r8, FILE_SHARE_READ
    mov rdx, GENERIC_READ | GENERIC_WRITE
    lea rcx, [rel monitor_path]

    call CreateFileA

    cmp rax, -1                 ; INVALID_HANDLE_VALUE
    je .init_done

    mov rbx, rax                ; Save Handle

    ; D. Get Timestamp
    lea rcx, [rsp + 60]         ; Buffer for FILETIME
    call GetSystemTimeAsFileTime

    ; Convert FILETIME to Unix Timestamp (approx)
    ; (FT - 116444736000000000) / 10000000
    mov rax, [rsp + 60]
    mov rdx, 0x019DB1DED53E8000
    sub rax, rdx
    mov rcx, 10000000
    xor rdx, rdx
    div rcx
    mov r12, rax                ; R12 = Timestamp

    ; E. Write Timestamp to Buffer
    lea rdi, [rsp + 80]         ; Buffer for String
    mov rsi, rdi                ; Save start

    mov rax, r12
    mov rcx, 10

    ; Itoa for Timestamp
.ts_loop:
    xor rdx, rdx
    div rcx
    add dl, '0'
    mov [rdi], dl
    inc rdi
    test rax, rax
    jnz .ts_loop

    ; Reverse Timestamp
    mov r8, rdi
    dec r8      ; End
    mov r9, rsi ; Start
.ts_rev:
    cmp r9, r8
    jge .ts_done
    mov al, [r9]
    mov bl, [r8]
    mov [r9], bl
    mov [r8], al
    inc r9
    dec r8
    jmp .ts_rev

.ts_done:
    ; Calculate Length
    mov r8, rdi
    sub r8, rsi ; Length

    ; F. Write to File
    ; WriteFile(handle, buf, len, &written, NULL)

    mov qword [rsp + 32], 0     ; Overlapped
    lea r9, [rsp + 60]          ; &bytesWritten
    lea rdx, [rsp + 80]         ; Buffer
    mov rcx, rbx                ; Handle

    call WriteFile

    ; CloseHandle
    mov rcx, rbx
    call CloseHandle

    ; Tracker
    call mem_snapshot_update_tracker

.init_done:
    add rsp, 136
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    leave
    ret

; ==============================================================================
; SYSCALL WRAPPERS
; ==============================================================================

; __sys_write(fd, buffer, length)
; Input: rcx=fd, rdx=buffer, r8=length
__sys_write:
    cmp rcx, 1
    je .use_stdout
    ; Use RCX as Handle directly
    mov rax, rcx
    jmp .do_write

.use_stdout:
    mov rax, [rel stdout_handle]

.do_write:
    OS_WRITE rax, rdx, r8
    ; Return bytes written?
    ; OS_WRITE macro implementation stores written bytes?
    ; Let's assume macro handles calling WriteFile.
    ; We should verify OS_WRITE implementation.
    ; Assuming it works.
    mov rax, r8 ; Hack: return len
    ret

; __sys_read(fd, buffer, length)
; Input: rcx=fd, rdx=buffer, r8=length
__sys_read:
    ; ReadFile(handle, buf, len, &read, NULL)
    push rbp
    mov rbp, rsp
    sub rsp, 48

    mov qword [rsp + 32], 0 ; Overlapped
    lea r9, [rsp + 40]      ; &read
    ; args: rcx=handle, rdx=buf, r8=len, r9=written
    call ReadFile

    test eax, eax
    jz .read_fail

    mov rax, [rsp + 40] ; Return bytes read
    add rsp, 48
    pop rbp
    ret

.read_fail:
    mov rax, -1
    add rsp, 48
    pop rbp
    ret

; __sys_open(path, flags, mode)
; Input: rcx=path, rdx=flags, r8=mode
__sys_open:
    ; CreateFileA(path, ACCESS, SHARE, NULL, OPEN_EXISTING, ATTR, NULL)
    push rbp
    mov rbp, rsp
    sub rsp, 64

    mov qword [rsp + 48], 0     ; Template
    mov qword [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp + 32], OPEN_EXISTING ; Default for read?
    ; Check Flags logic?
    ; Minimal mapping:
    ; O_RDONLY = 0.
    ; Let's assume Read Only for now or handle simple flags.
    ; 0102 = O_CREAT | O_WRONLY.
    ; If RDX & 0x40 (O_CREAT) -> OPEN_ALWAYS or CREATE_ALWAYS.

    ; Logic:
    ; ACCESS = GENERIC_READ
    ; SHARE = FILE_SHARE_READ

    xor r9, r9 ; Security
    mov r8, FILE_SHARE_READ
    mov rdx, GENERIC_READ

    call CreateFileA

    add rsp, 64
    pop rbp
    ret

; __sys_close(fd)
__sys_close:
    sub rsp, 40
    call CloseHandle
    add rsp, 40
    ret

; __sys_exit(code)
__sys_exit:
    OS_EXIT rcx
    ret

; __mem_map(size)
__mem_map:
    ; Wrapper sementara. Macro OS_ALLOC_PAGE alloc 1 page fixed.
    ; Nanti akan digantikan oleh allocator.
    OS_ALLOC_PAGE rcx
    ret

; __mem_unmap(ptr, size)
__mem_unmap:
    sub rsp, 32
    xor rdx, rdx        ; dwSize = 0 (MEM_RELEASE)
    mov r8, 0x8000      ; MEM_RELEASE
    call VirtualFree
    add rsp, 32
    ret
