; corelib/platform/x86_64/asm_win/daemon_cleaner.asm
; Native Daemon Cleaner for Windows x64
; ==============================================================================

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    ; Paths (Windows)
    monitor_dir db "morph_swap_monitor", 0
    snapshot_prefix db "snapshot_", 0
    sandbox_prefix db "sandbox_", 0
    wildcard_all db "*", 0

    ; Configuration
    global daemon_snapshot_ttl
    global daemon_sandbox_ttl
    global daemon_check_interval

    daemon_snapshot_ttl dq 300      ; 5 minutes
    daemon_sandbox_ttl dq 60        ; 1 minute
    daemon_check_interval dq 2      ; 2 seconds

    ; Messages
    msg_start db "[Daemon] MorphFox Cleaner started", 13, 10, 0
    msg_clean_snap db "[Daemon] Cleaning snapshot: ", 0
    msg_clean_sand db "[Daemon] Cleaning sandbox: ", 0
    msg_newline db 13, 10, 0

section .bss
    find_data resb 592              ; WIN32_FIND_DATA structure
    path_buffer resb 512
    time_buffer resb 64
    file_time resq 1

section .text
global daemon_start
global daemon_loop
global daemon_clean_snapshots
global daemon_clean_sandboxes

extern CreateDirectoryA
extern FindFirstFileA
extern FindNextFileA
extern FindClose
extern DeleteFileA
extern GetTickCount64
extern Sleep
extern CreateFileA
extern ReadFile
extern CloseHandle
extern GetStdHandle
extern WriteFile

%define STD_OUTPUT_HANDLE -11
%define GENERIC_READ 0x80000000
%define OPEN_EXISTING 3
%define FILE_ATTRIBUTE_NORMAL 0x80
%define INVALID_HANDLE_VALUE -1

; ------------------------------------------------------------------------------
; func daemon_start() -> PID (stub for now)
; Windows: Can't fork easily - run in current process or CreateProcess
; For now, simplified: just returns 0 (run inline)
; ------------------------------------------------------------------------------
daemon_start:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    ; Print start message
    mov rcx, STD_OUTPUT_HANDLE
    call GetStdHandle

    mov rcx, rax                    ; Handle
    lea rdx, [rel msg_start]
    call strlen_win
    mov r8, rax                     ; Length
    lea r9, [rsp + 24]              ; BytesWritten
    mov qword [rsp + 32], 0         ; lpOverlapped = NULL
    call WriteFile

    ; TODO: CreateProcess for true background daemon
    ; For now, return 0 to indicate "running in current process"
    xor rax, rax

    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func daemon_loop()
; Main loop: clean snapshots, sandboxes, monitor, sleep
; ------------------------------------------------------------------------------
daemon_loop:
    push rbp
    mov rbp, rsp

.loop:
    call daemon_clean_snapshots
    call daemon_clean_sandboxes
    ; daemon_monitor_memory() - TODO

    ; Sleep
    mov rcx, [rel daemon_check_interval]
    imul rcx, 1000                  ; Convert seconds to milliseconds
    call Sleep

    jmp .loop

    ; Never returns
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func daemon_clean_snapshots()
; Scan directory for snapshot_* files and delete old ones
; ------------------------------------------------------------------------------
daemon_clean_snapshots:
    push rbp
    mov rbp, rsp
    push rbx
    push r12                        ; Current time
    push r13                        ; Find handle
    push r14                        ; File timestamp
    sub rsp, 56                     ; 32 (Shadow) + 24 (Args 5,6,7) = 56

    ; Get current time (milliseconds since boot)
    call GetTickCount64
    mov r12, rax
    imul r12, 1000                  ; Convert to seconds (approx)

    ; Build search path: "morph_swap_monitor\*"
    lea rdi, [rel path_buffer]
    lea rsi, [rel monitor_dir]
    call strcpy_win
    lea rdi, [rel path_buffer]
    call strlen_win
    lea rdi, [rel path_buffer]
    add rdi, rax
    mov byte [rdi], '\'
    inc rdi
    lea rsi, [rel wildcard_all]
    call strcpy_win

    ; FindFirstFile
    lea rcx, [rel path_buffer]
    lea rdx, [rel find_data]
    call FindFirstFileA

    cmp rax, INVALID_HANDLE_VALUE
    je .done
    mov r13, rax                    ; Save handle

.scan_loop:
    ; Check if filename starts with "snapshot_"
    lea rdi, [rel find_data + 44]   ; cFileName offset
    lea rsi, [rel snapshot_prefix]
    mov rcx, 9
    call strncmp_win
    test rax, rax
    jnz .next_file

    ; Build full path to file
    lea rdi, [rel path_buffer]
    lea rsi, [rel monitor_dir]
    call strcpy_win
    lea rdi, [rel path_buffer]
    call strlen_win
    lea rdi, [rel path_buffer]
    add rdi, rax
    mov byte [rdi], '\'
    inc rdi
    lea rsi, [rel find_data + 44]
    call strcpy_win

    ; Read timestamp from file
    lea rcx, [rel path_buffer]
    mov rdx, GENERIC_READ
    xor r8, r8                      ; ShareMode
    xor r9, r9                      ; Security
    mov qword [rsp + 32], OPEN_EXISTING
    mov qword [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp + 48], 0         ; Template
    call CreateFileA

    cmp rax, INVALID_HANDLE_VALUE
    je .next_file
    mov rbx, rax                    ; File handle

    ; ReadFile
    mov rcx, rbx
    lea rdx, [rel time_buffer]
    mov r8, 64
    lea r9, [rsp + 24]              ; BytesRead
    mov qword [rsp + 32], 0
    call ReadFile

    ; Close file
    mov rcx, rbx
    call CloseHandle

    ; Parse timestamp
    lea rcx, [rel time_buffer]
    call atoi_win
    mov r14, rax                    ; File timestamp

    ; Calculate age
    mov rax, r12
    sub rax, r14

    ; Check TTL
    cmp rax, [rel daemon_snapshot_ttl]
    jle .next_file

    ; Delete file
    lea rcx, [rel path_buffer]
    call DeleteFileA

.next_file:
    mov rcx, r13
    lea rdx, [rel find_data]
    call FindNextFileA
    test rax, rax
    jnz .scan_loop

    ; Close find handle
    mov rcx, r13
    call FindClose

.done:
    add rsp, 56
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func daemon_clean_sandboxes()
; Similar to snapshots but for sandbox_* files
; ------------------------------------------------------------------------------
daemon_clean_sandboxes:
    push rbp
    mov rbp, rsp
    push rbx
    push r12                        ; Current time
    push r13                        ; Find handle
    push r14                        ; File timestamp
    sub rsp, 56                     ; 32 (Shadow) + 24 (Args 5,6,7) = 56

    ; Get current time (milliseconds since boot)
    call GetTickCount64
    mov r12, rax
    imul r12, 1000                  ; Convert to seconds (approx)

    ; Build search path: "morph_swap_monitor\*"
    lea rdi, [rel path_buffer]
    lea rsi, [rel monitor_dir]
    call strcpy_win
    lea rdi, [rel path_buffer]
    call strlen_win
    lea rdi, [rel path_buffer]
    add rdi, rax
    mov byte [rdi], '\'
    inc rdi
    lea rsi, [rel wildcard_all]
    call strcpy_win

    ; FindFirstFile
    lea rcx, [rel path_buffer]
    lea rdx, [rel find_data]
    call FindFirstFileA

    cmp rax, INVALID_HANDLE_VALUE
    je .sb_done
    mov r13, rax                    ; Save handle

.sb_scan_loop:
    ; Check if filename starts with "sandbox_"
    lea rdi, [rel find_data + 44]   ; cFileName offset
    lea rsi, [rel sandbox_prefix]
    mov rcx, 8                      ; "sandbox_" is 8 chars
    call strncmp_win
    test rax, rax
    jnz .sb_next_file

    ; Build full path to file
    lea rdi, [rel path_buffer]
    lea rsi, [rel monitor_dir]
    call strcpy_win
    lea rdi, [rel path_buffer]
    call strlen_win
    lea rdi, [rel path_buffer]
    add rdi, rax
    mov byte [rdi], '\'
    inc rdi
    lea rsi, [rel find_data + 44]
    call strcpy_win

    ; Read timestamp from file
    lea rcx, [rel path_buffer]
    mov rdx, GENERIC_READ
    xor r8, r8                      ; ShareMode
    xor r9, r9                      ; Security
    mov qword [rsp + 32], OPEN_EXISTING
    mov qword [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp + 48], 0         ; Template
    call CreateFileA

    cmp rax, INVALID_HANDLE_VALUE
    je .sb_next_file
    mov rbx, rax                    ; File handle

    ; ReadFile
    mov rcx, rbx
    lea rdx, [rel time_buffer]
    mov r8, 64
    lea r9, [rsp + 24]              ; BytesRead
    mov qword [rsp + 32], 0
    call ReadFile

    ; Close file
    mov rcx, rbx
    call CloseHandle

    ; Parse timestamp
    lea rcx, [rel time_buffer]
    call atoi_win
    mov r14, rax                    ; File timestamp

    ; Calculate age
    mov rax, r12
    sub rax, r14

    ; Check TTL
    cmp rax, [rel daemon_sandbox_ttl]
    jle .sb_next_file

    ; Delete file
    lea rcx, [rel path_buffer]
    call DeleteFileA

.sb_next_file:
    mov rcx, r13
    lea rdx, [rel find_data]
    call FindNextFileA
    test rax, rax
    jnz .sb_scan_loop

    ; Close find handle
    mov rcx, r13
    call FindClose

.sb_done:
    add rsp, 56
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; Helper Functions
; ------------------------------------------------------------------------------

strlen_win:
    xor rax, rax
.loop:
    cmp byte [rcx], 0
    je .done
    inc rax
    inc rcx
    jmp .loop
.done:
    ret

strcpy_win:
.loop:
    mov al, [rsi]
    mov [rdi], al
    test al, al
    jz .done
    inc rsi
    inc rdi
    jmp .loop
.done:
    ret

strncmp_win:
    xor rax, rax
.loop:
    test rcx, rcx
    jz .equal
    mov al, [rdi]
    cmp al, [rsi]
    jne .not_equal
    inc rdi
    inc rsi
    dec rcx
    jmp .loop
.equal:
    xor rax, rax
    ret
.not_equal:
    mov rax, 1
    ret

atoi_win:
    xor rax, rax
    xor rdx, rdx
.loop:
    movzx rdx, byte [rcx]
    cmp dl, '0'
    jb .done
    cmp dl, '9'
    ja .done
    sub dl, '0'
    imul rax, 10
    add rax, rdx
    inc rcx
    jmp .loop
.done:
    ret
