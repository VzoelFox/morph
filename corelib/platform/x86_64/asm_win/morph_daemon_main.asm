; corelib/platform/x86_64/asm_win/morph_daemon_main.asm
; Daemon Launcher for Windows x64
; ==============================================================================

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    ; PID file path (Windows temp directory)
    pid_file db "morph_cleaner.pid", 0

    ; Messages
    msg_usage db "Usage: morph_daemon {start|stop|status}", 13, 10, 0
    msg_starting db "[MorphDaemon] Starting...", 13, 10, 0
    msg_started db "[MorphDaemon] Started (inline mode)", 13, 10, 0
    msg_stopping db "[MorphDaemon] Stopping...", 13, 10, 0
    msg_stopped db "[MorphDaemon] Stopped", 13, 10, 0
    msg_running db "[MorphDaemon] Running", 13, 10, 0
    msg_not_running db "[MorphDaemon] Not running", 13, 10, 0
    msg_already_running db "[MorphDaemon] Already running", 13, 10, 0

    ; Commands
    cmd_start db "start", 0
    cmd_stop db "stop", 0
    cmd_status db "status", 0
    cmd_run db "run", 0             ; Internal: daemon loop

section .bss
    argc resq 1
    argv resq 1
    pid_buffer resb 32

section .text
global _start
extern daemon_start
extern daemon_loop
extern ExitProcess
extern GetCommandLineA
extern CommandLineToArgvW
extern GetStdHandle
extern WriteFile
extern CreateFileA
extern ReadFile
extern CloseHandle
extern DeleteFileA
extern LocalFree

%define STD_OUTPUT_HANDLE -11
%define GENERIC_READ 0x80000000
%define GENERIC_WRITE 0x40000000
%define CREATE_ALWAYS 2
%define OPEN_EXISTING 3
%define FILE_ATTRIBUTE_NORMAL 0x80

; ------------------------------------------------------------------------------
; Entry Point
; ------------------------------------------------------------------------------
_start:
    sub rsp, 40                     ; Align stack + shadow space

    ; Get command line
    call GetCommandLineA
    mov rcx, rax
    lea rdx, [rel argc]
    call CommandLineToArgvW

    mov [rel argv], rax

    ; Check argc
    mov rax, [rel argc]
    cmp rax, 2
    jl .usage

    ; Get argv[1]
    mov rax, [rel argv]
    mov rcx, [rax + 8]              ; argv[1]

    ; Compare with "start"
    lea rdx, [rel cmd_start]
    call strcmp_cmd
    test rax, rax
    jz .cmd_start

    ; Compare with "run" (internal)
    mov rax, [rel argv]
    mov rcx, [rax + 8]
    lea rdx, [rel cmd_run]
    call strcmp_cmd
    test rax, rax
    jz .cmd_run

    ; Compare with "status"
    mov rax, [rel argv]
    mov rcx, [rax + 8]
    lea rdx, [rel cmd_status]
    call strcmp_cmd
    test rax, rax
    jz .cmd_status

    jmp .usage

; ------------------------------------------------------------------------------
; START Command
; ------------------------------------------------------------------------------
.cmd_start:
    ; Check if already running
    call check_pid_file
    test rax, rax
    jnz .already_running

    ; Print starting
    call print_msg
    dq msg_starting

    ; For Windows simplified: Run daemon inline (no background process yet)
    ; TODO: Use CreateProcess to spawn background process
    call daemon_start

    ; Print started
    call print_msg
    dq msg_started

    ; Write PID file (current process)
    call write_current_pid

    ; Enter daemon loop (blocking)
    call daemon_loop

    ; Never reaches here
    xor rcx, rcx
    call ExitProcess

.already_running:
    call print_msg
    dq msg_already_running
    mov rcx, 1
    call ExitProcess

; ------------------------------------------------------------------------------
; RUN Command (internal - enters daemon loop)
; ------------------------------------------------------------------------------
.cmd_run:
    call daemon_loop
    xor rcx, rcx
    call ExitProcess

; ------------------------------------------------------------------------------
; STATUS Command
; ------------------------------------------------------------------------------
.cmd_status:
    call check_pid_file
    test rax, rax
    jz .not_running

    call print_msg
    dq msg_running
    xor rcx, rcx
    call ExitProcess

.not_running:
    call print_msg
    dq msg_not_running
    mov rcx, 1
    call ExitProcess

; ------------------------------------------------------------------------------
; USAGE
; ------------------------------------------------------------------------------
.usage:
    call print_msg
    dq msg_usage
    mov rcx, 1
    call ExitProcess

; ------------------------------------------------------------------------------
; Helper Functions
; ------------------------------------------------------------------------------

; check_pid_file() -> 1 if exists and valid, 0 otherwise
check_pid_file:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    lea rcx, [rel pid_file]
    mov rdx, GENERIC_READ
    xor r8, r8
    xor r9, r9
    mov qword [rsp + 32], OPEN_EXISTING
    mov qword [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp + 48], 0
    call CreateFileA

    cmp rax, -1
    je .not_exists

    ; File exists - for now, assume valid
    mov rcx, rax
    call CloseHandle

    mov rax, 1
    add rsp, 32
    pop rbp
    ret

.not_exists:
    xor rax, rax
    add rsp, 32
    pop rbp
    ret

; write_current_pid() - Write current PID to file
write_current_pid:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    ; Create PID file
    lea rcx, [rel pid_file]
    mov rdx, GENERIC_WRITE
    xor r8, r8
    xor r9, r9
    mov qword [rsp + 32], CREATE_ALWAYS
    mov qword [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp + 48], 0
    call CreateFileA

    cmp rax, -1
    je .fail

    mov rbx, rax                    ; Handle

    ; Write dummy PID (for now, just "1")
    mov rcx, rbx
    lea rdx, [rel dummy_pid]
    mov r8, 1
    lea r9, [rsp + 24]
    mov qword [rsp + 32], 0
    call WriteFile

    mov rcx, rbx
    call CloseHandle

.fail:
    add rsp, 32
    pop rbp
    ret

; print_msg(msg_ptr)
print_msg:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 32

    pop rbx                         ; Return address
    pop rax                         ; Message address
    push rbx                        ; Restore return

    mov rbx, rax

    mov rcx, STD_OUTPUT_HANDLE
    call GetStdHandle

    mov rcx, rax
    mov rdx, rbx
    push rdx
    call strlen_cmd
    pop rdx
    mov r8, rax
    lea r9, [rsp + 24]
    mov qword [rsp + 32], 0
    call WriteFile

    add rsp, 32
    pop rbx
    pop rbp
    ret

; strcmp_cmd(s1, s2) -> 0 if equal, 1 if not
strcmp_cmd:
.loop:
    movzx rax, byte [rcx]
    movzx rdx, byte [rdx]
    cmp al, dl
    jne .not_equal
    test al, al
    jz .equal
    inc rcx
    inc rdx
    jmp .loop
.equal:
    xor rax, rax
    ret
.not_equal:
    mov rax, 1
    ret

; strlen_cmd(str) -> length
strlen_cmd:
    xor rax, rax
.loop:
    cmp byte [rdx], 0
    je .done
    inc rax
    inc rdx
    jmp .loop
.done:
    ret

section .data
dummy_pid db "1", 0
