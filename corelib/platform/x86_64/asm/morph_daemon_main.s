# corelib/platform/x86_64/asm/morph_daemon_main.s
# Standalone Daemon Launcher - Entry Point for MorphFox Cleaner Daemon
# ==============================================================================
# Usage: ./morph_daemon {start|stop|status}
# Replaces morph_cleaner.sh dengan native implementation

.include "corelib/platform/x86_64/asm/macros.inc"

# Syscall numbers (Linux x86_64)
.equ SYS_READ, 0
.equ SYS_WRITE, 1
.equ SYS_OPEN, 2
.equ SYS_CLOSE, 3
.equ SYS_UNLINK, 87
.equ SYS_KILL, 62

# Open flags
.equ O_RDONLY, 0
.equ O_WRONLY, 1
.equ O_CREAT, 64
.equ O_TRUNC, 512

.section .data
    # PID file path
    pid_file: .asciz "/tmp/morph_cleaner.pid"

    # Usage messages
    msg_usage: .asciz "Usage: morph_daemon {start|stop|status}\n"
    msg_starting: .asciz "[MorphDaemon] Starting...\n"
    msg_started: .asciz "[MorphDaemon] Started with PID: "
    msg_stopping: .asciz "[MorphDaemon] Stopping...\n"
    msg_stopped: .asciz "[MorphDaemon] Stopped\n"
    msg_running: .asciz "[MorphDaemon] Running (PID: "
    msg_not_running: .asciz "[MorphDaemon] Not running\n"
    msg_already_running: .asciz "[MorphDaemon] Already running\n"
    msg_newline: .asciz "\n"
    msg_bracket: .asciz ")\n"

    # Command strings
    cmd_start: .asciz "start"
    cmd_stop: .asciz "stop"
    cmd_status: .asciz "status"

    .lcomm pid_buffer, 32

.section .text
.global _start
.extern daemon_start
.extern daemon_loop

# ------------------------------------------------------------------------------
# Entry Point
# ------------------------------------------------------------------------------
_start:
    # Check argc (at rsp)
    movq (%rsp), %rax
    cmpq $2, %rax
    jne .usage

    # Get argv[1] (command)
    movq 16(%rsp), %rdi     # argv[1]

    # Compare with "start"
    leaq cmd_start(%rip), %rsi
    call strcmp_cmd
    testq %rax, %rax
    jz .cmd_start

    # Compare with "stop"
    movq 16(%rsp), %rdi
    leaq cmd_stop(%rip), %rsi
    call strcmp_cmd
    testq %rax, %rax
    jz .cmd_stop

    # Compare with "status"
    movq 16(%rsp), %rdi
    leaq cmd_status(%rip), %rsi
    call strcmp_cmd
    testq %rax, %rax
    jz .cmd_status

    # Unknown command
    jmp .usage

# ------------------------------------------------------------------------------
# START Command
# ------------------------------------------------------------------------------
.cmd_start:
    # Check if already running
    call check_pid_file
    testq %rax, %rax
    jnz .already_running

    # Print starting message
    OS_WRITE $1, msg_starting, $27

    # Start daemon (forks and returns child PID)
    call daemon_start

    testq %rax, %rax
    jl .start_failed

    # Parent: Write PID to file
    pushq %rax              # Save PID
    call write_pid_file
    popq %rdi               # Restore PID

    # Print started message with PID
    OS_WRITE $1, msg_started, $33
    call print_number
    OS_WRITE $1, msg_newline, $1

    OS_EXIT $0

.already_running:
    OS_WRITE $1, msg_already_running, $31
    OS_EXIT $1

.start_failed:
    OS_EXIT $1

# ------------------------------------------------------------------------------
# STOP Command
# ------------------------------------------------------------------------------
.cmd_stop:
    OS_WRITE $1, msg_stopping, $27

    # Read PID from file
    call read_pid_file
    testq %rax, %rax
    jle .not_running

    # Send SIGTERM to daemon
    movq %rax, %rdi         # PID
    movq $15, %rsi          # SIGTERM
    movq $SYS_KILL, %rax
    syscall

    # Delete PID file
    movq $SYS_UNLINK, %rax
    leaq pid_file(%rip), %rdi
    syscall

    OS_WRITE $1, msg_stopped, $26
    OS_EXIT $0

# ------------------------------------------------------------------------------
# STATUS Command
# ------------------------------------------------------------------------------
.cmd_status:
    # Read PID from file
    call read_pid_file
    testq %rax, %rax
    jle .not_running

    movq %rax, %r12         # Save PID

    # Check if process is still alive (kill -0)
    movq %r12, %rdi
    xorq %rsi, %rsi         # Signal 0
    movq $SYS_KILL, %rax
    syscall

    testq %rax, %rax
    jnz .not_running

    # Process is running
    OS_WRITE $1, msg_running, $29
    movq %r12, %rdi
    call print_number
    OS_WRITE $1, msg_bracket, $2

    OS_EXIT $0

.not_running:
    OS_WRITE $1, msg_not_running, $28
    OS_EXIT $1

# ------------------------------------------------------------------------------
# USAGE
# ------------------------------------------------------------------------------
.usage:
    OS_WRITE $1, msg_usage, $43
    OS_EXIT $1

# ------------------------------------------------------------------------------
# Helper Functions
# ------------------------------------------------------------------------------

# check_pid_file() -> PID or 0
check_pid_file:
    pushq %rbp
    movq %rsp, %rbp

    # Open PID file
    movq $SYS_OPEN, %rax
    leaq pid_file(%rip), %rdi
    movq $O_RDONLY, %rsi
    xorq %rdx, %rdx
    syscall

    testq %rax, %rax
    js .cpf_not_exists

    movq %rax, %r12         # FD

    # Read PID
    movq $SYS_READ, %rax
    movq %r12, %rdi
    leaq pid_buffer(%rip), %rsi
    movq $32, %rdx
    syscall

    # Close
    pushq %rax
    movq $SYS_CLOSE, %rax
    movq %r12, %rdi
    syscall
    popq %rcx

    testq %rcx, %rcx
    jle .cpf_not_exists

    # Parse PID
    leaq pid_buffer(%rip), %rdi
    call atoi_simple

    # Check if process alive
    movq %rax, %rdi
    xorq %rsi, %rsi
    pushq %rdi
    movq $SYS_KILL, %rax
    syscall
    popq %rdi

    testq %rax, %rax
    jnz .cpf_not_exists     # Process doesn't exist

    movq %rdi, %rax
    leave
    ret

.cpf_not_exists:
    xorq %rax, %rax
    leave
    ret

# read_pid_file() -> PID or 0
read_pid_file:
    pushq %rbp
    movq %rsp, %rbp

    movq $SYS_OPEN, %rax
    leaq pid_file(%rip), %rdi
    movq $O_RDONLY, %rsi
    xorq %rdx, %rdx
    syscall

    testq %rax, %rax
    js .rpf_fail
    movq %rax, %r12

    movq $SYS_READ, %rax
    movq %r12, %rdi
    leaq pid_buffer(%rip), %rsi
    movq $32, %rdx
    syscall

    pushq %rax
    movq $SYS_CLOSE, %rax
    movq %r12, %rdi
    syscall
    popq %rcx

    testq %rcx, %rcx
    jle .rpf_fail

    leaq pid_buffer(%rip), %rdi
    call atoi_simple
    leave
    ret

.rpf_fail:
    xorq %rax, %rax
    leave
    ret

# write_pid_file(%rdi = PID)
write_pid_file:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12

    movq %rdi, %r12         # Save PID

    # Open file
    movq $SYS_OPEN, %rax
    leaq pid_file(%rip), %rdi
    movq $O_CREAT, %rsi
    orq $O_WRONLY, %rsi
    orq $O_TRUNC, %rsi
    movq $0644, %rdx
    syscall

    testq %rax, %rax
    js .wpf_fail
    movq %rax, %r13

    # Convert PID to string
    leaq pid_buffer(%rip), %rdi
    movq %r12, %rsi
    call itoa_simple

    # Write
    movq $SYS_WRITE, %rax
    movq %r13, %rdi
    leaq pid_buffer(%rip), %rsi
    movq %rdx, %rdx         # Length from itoa_simple
    syscall

    # Close
    movq $SYS_CLOSE, %rax
    movq %r13, %rdi
    syscall

.wpf_fail:
    popq %r12
    leave
    ret

# strcmp_cmd(s1, s2) -> 0 if equal, 1 if not
strcmp_cmd:
.strcmp_loop:
    movb (%rdi), %al
    movb (%rsi), %bl
    cmpb %al, %bl
    jne .strcmp_neq
    testb %al, %al
    jz .strcmp_eq
    incq %rdi
    incq %rsi
    jmp .strcmp_loop
.strcmp_eq:
    xorq %rax, %rax
    ret
.strcmp_neq:
    movq $1, %rax
    ret

# atoi_simple(str) -> number
atoi_simple:
    xorq %rax, %rax
    xorq %rcx, %rcx
.atoi_loop:
    movb (%rdi), %cl
    cmpb $'0', %cl
    jb .atoi_done
    cmpb $'9', %cl
    ja .atoi_done
    subb $'0', %cl
    imulq $10, %rax
    addq %rcx, %rax
    incq %rdi
    jmp .atoi_loop
.atoi_done:
    ret

# itoa_simple(dest, number) -> length in RDX
itoa_simple:
    pushq %rbx
    movq %rdi, %rbx         # Save dest
    movq %rsi, %rax         # Number
    movq $10, %rcx
    xorq %r8, %r8           # Counter

.itoa_loop:
    xorq %rdx, %rdx
    divq %rcx
    addb $'0', %dl
    movb %dl, (%rdi)
    incq %rdi
    incq %r8
    testq %rax, %rax
    jnz .itoa_loop

    # Reverse
    movq %rbx, %rsi
    decq %rdi
.itoa_rev:
    cmpq %rsi, %rdi
    jle .itoa_done
    movb (%rsi), %al
    movb (%rdi), %cl
    movb %cl, (%rsi)
    movb %al, (%rdi)
    incq %rsi
    decq %rdi
    jmp .itoa_rev

.itoa_done:
    movq %r8, %rdx
    popq %rbx
    ret

# print_number(%rdi = number)
print_number:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp

    leaq -32(%rbp), %rsi
    call itoa_simple

    OS_WRITE $1, -32(%rbp), %rdx

    leave
    ret
