# corelib/platform/x86_64/asm/runtime.s
# Implementasi Fisik Kontrak ABI MorphFox untuk x86_64 Linux
# Menggunakan Abstraksi Macro (macros.inc) untuk SSOT Compliance

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
.global __sys_page_size
.global vm_cycle_budget
.global global_sym_table
.global global_sym_cap

__sys_page_size: .quad 4096 # Default safe fallback
vm_cycle_budget: .quad 1000000 # Circuit Breaker Budget
global_sym_table: .quad 0
global_sym_cap: .quad 1024

# Runtime Monitoring Data
monitor_dir:     .asciz "/tmp/morph_swap_monitor"
monitor_prefix:  .asciz "/tmp/morph_swap_monitor/sandbox_"
msg_snap:        .asciz "[Runtime] Snapshot Triggered (Signal USR1)\n"
msg_reset:       .asciz "[Runtime] System Reset Triggered (Signal USR2)\n"

.section .bss
monitor_path:    .skip 256  # Buffer untuk path lengkap
sigaction_act:   .skip 152  # Struct sigaction (sizeof 152 cukup aman)

.section .text
.global _start
.global __sys_write
.global __sys_exit
.global __mem_map
.global __mem_unmap
.global __mf_runtime_init

# ------------------------------------------------------------------------------
# ENTRY POINT
# ------------------------------------------------------------------------------
_start:
    # 1. Detect Page Size from AUXV
    # Stack: [argc] [argv...] 0 [envp...] 0 [auxv...]
    movq (%rsp), %rcx       # argc
    leaq 8(%rsp), %rsi      # argv start

    # Skip argv
    leaq (%rsi, %rcx, 8), %rsi # rsi = after last argv
    addq $8, %rsi           # Skip NULL terminator of argv

    # Skip envp
.skip_envp:
    movq (%rsi), %rax
    addq $8, %rsi
    testq %rax, %rax
    jnz .skip_envp

    # Now at AUXV. Format: [Type, Value]
    # AT_PAGESZ = 6
.scan_auxv:
    movq (%rsi), %rax       # Type
    movq 8(%rsi), %rbx      # Value
    addq $16, %rsi          # Next pair

    testq %rax, %rax        # Type 0 = End
    jz .auxv_done

    cmpq $6, %rax           # AT_PAGESZ?
    jne .scan_auxv

    # Found it!
    movq %rbx, __sys_page_size(%rip)

.auxv_done:

    # 2. Initialize Runtime (Monitoring & Signals)
    call __mf_runtime_init

    # 2.5 Initialize Symbol Table
    movq $1024, %rdi
    call sym_table_create
    movq %rax, global_sym_table(%rip)

    # 3. Call Main
    # Setup standard C ABI arguments: RDI=argc, RSI=argv
    # Stack at entry: [argc] [argv ptrs...] [NULL] [envp...]
    # Note: stack pointer %rsp has not changed since entry (we only read from it)
    movq (%rsp), %rdi       # argc
    leaq 8(%rsp), %rsi      # argv
    call main

    # 4. Exit
    OS_EXIT %rax

# ------------------------------------------------------------------------------
# RUNTIME INITIALIZATION (INTERNAL)
# ------------------------------------------------------------------------------
__mf_runtime_init:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13

    # A. Setup Signal Handlers
    # -------------------------
    # sigaction(signum, act, oldact)
    # act struct: handler(8), flags(8), restorer(8), mask(8)

    # Prepare struct sigaction
    leaq sigaction_act(%rip), %r12

    # 1. Handler
    leaq __mf_sig_handler(%rip), %rax
    movq %rax, 0(%r12)

    # 2. Flags: SA_RESTORER (0x04000000)
    # Linux requires SA_RESTORER set and a restorer function provided
    movq $0x04000000, 8(%r12)

    # 3. Restorer Function
    leaq __mf_sig_restorer(%rip), %rax
    movq %rax, 16(%r12)

    # 4. Mask (Empty/Zero)
    movq $0, 24(%r12)

    # Register SIGUSR1
    movq $SYS_RT_SIGACTION, %rax
    movq $SIGUSR1, %rdi
    movq %r12, %rsi
    xorq %rdx, %rdx     # oldact = NULL
    movq $8, %r10       # sigsetsize (8 bytes)
    syscall

    # Register SIGUSR2
    movq $SYS_RT_SIGACTION, %rax
    movq $SIGUSR2, %rdi
    movq %r12, %rsi
    xorq %rdx, %rdx
    movq $8, %r10
    syscall

    # B. Create Monitor Directory
    # ---------------------------
    # mkdir("/tmp/morph_swap_monitor", 0700)
    movq $SYS_MKDIR, %rax
    leaq monitor_dir(%rip), %rdi
    movq $0700, %rsi
    syscall
    # Ignore error (EEXIST is fine)

    # C. Create Sandbox File
    # ----------------------
    # Format: Prefix + PID

    # 1. Copy Prefix to Buffer
    leaq monitor_prefix(%rip), %rsi
    leaq monitor_path(%rip), %rdi

.L_copy_prefix:
    movb (%rsi), %al
    testb %al, %al
    jz .L_copy_prefix_done
    movb %al, (%rdi)
    incq %rsi
    incq %rdi
    jmp .L_copy_prefix
.L_copy_prefix_done:

    # 2. Get PID
    movq $SYS_GETPID, %rax
    syscall
    movq %rax, %r12     # R12 = PID

    # 3. Convert PID to String (Simple Itoa)
    # We write directly to %rdi (end of prefix)

    # Check 0
    testq %r12, %r12
    jnz .L_itoa_start
    movb $'0', (%rdi)
    incq %rdi
    jmp .L_itoa_done

.L_itoa_start:
    # Save current buffer ptr
    movq %rdi, %r13     # R13 = Start of Number

    # Loop Div 10
    movq %r12, %rax
    movq $10, %rbx
.L_itoa_loop:
    xorq %rdx, %rdx
    divq %rbx           # RAX / 10, RDX % 10
    addb $'0', %dl
    movb %dl, (%rdi)
    incq %rdi
    testq %rax, %rax
    jnz .L_itoa_loop

    # Reverse the digits
    # R13 = Start, RDI = End (Exclusive)
    movq %rdi, %rcx     # End
    decq %rcx           # Last Char
    movq %r13, %rsi     # First Char

.L_reverse:
    cmpq %rcx, %rsi
    jge .L_itoa_done
    movb (%rsi), %al
    movb (%rcx), %bl
    movb %bl, (%rsi)
    movb %al, (%rcx)
    incq %rsi
    decq %rcx
    jmp .L_reverse

.L_itoa_done:
    # Null Terminate
    movb $0, (%rdi)

    # 4. Open File
    # open(path, O_CREAT | O_RDWR, 0600)
    movq $SYS_OPEN, %rax
    leaq monitor_path(%rip), %rdi
    movq $O_CREAT, %rdx
    orq $O_RDWR, %rdx
    movq %rdx, %rsi     # flags
    movq $0600, %rdx    # mode
    syscall

    cmpq $0, %rax
    js .init_done       # If error, skip write

    movq %rax, %r12     # R12 = FD

    # 5. Get Timestamp (Seconds)
    # time(NULL) -> RAX
    movq $SYS_TIME, %rax
    xorq %rdi, %rdi
    syscall
    movq %rax, %r13     # Timestamp

    # 6. Write Timestamp (String) to File
    # For now, just write "STARTED" or similar.
    # The bash script reads the content.
    # Logic: `ts=$(cat "$snapshot_file")`. It expects a NUMBER.
    # We must convert Timestamp R13 to String and write it.

    # Reuse Itoa Buffer in Stack? No, use monitor_path buffer end?
    # Let's verify buffer size. 256 bytes is plenty.
    # Write to a temp buffer on stack.
    subq $32, %rsp
    movq %rsp, %rdi

    # Itoa R13 (Timestamp)
    movq %r13, %rax
    # ... Same itoa logic ...
    # Simplified: Assume > 0
    movq %rdi, %rsi     # Save start
    movq $10, %rbx
.L_ts_loop:
    xorq %rdx, %rdx
    divq %rbx
    addb $'0', %dl
    movb %dl, (%rdi)
    incq %rdi
    testq %rax, %rax
    jnz .L_ts_loop

    # Reverse
    movq %rdi, %rcx
    decq %rcx
    movq %rsi, %r8
.L_ts_rev:
    cmpq %rcx, %r8
    jge .L_ts_done
    movb (%r8), %al
    movb (%rcx), %bl
    movb %bl, (%r8)
    movb %al, (%rcx)
    incq %r8
    decq %rcx
    jmp .L_ts_rev
.L_ts_done:
    # Calculate Length
    subq %rsp, %rdi     # Len = Ptr - Start

    # Write(fd, buf, len)
    movq %rdi, %rdx     # Length -> RDX
    movq $SYS_WRITE, %rax
    movq %r12, %rdi     # FD -> RDI
    movq %rsp, %rsi     # Buffer -> RSI
    syscall

    addq $32, %rsp      # Restore Stack

    # 7. Close File
    movq $SYS_CLOSE, %rax
    movq %r12, %rdi
    syscall

    # 8. Create Snapshot Tracker
    call mem_snapshot_update_tracker

.init_done:
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# SIGNAL HANDLING
# ------------------------------------------------------------------------------
__mf_sig_restorer:
    movq $SYS_RT_SIGRETURN, %rax
    syscall

__mf_sig_handler:
    # Input: RDI = Signum
    pushq %rax
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11

    cmpq $SIGUSR1, %rdi
    je .handle_snapshot
    cmpq $SIGUSR2, %rdi
    je .handle_reset
    jmp .sig_exit

.handle_snapshot:
    # Print Msg
    movq $SYS_WRITE, %rax
    movq $1, %rdi
    leaq msg_snap(%rip), %rsi
    movq $43, %rdx
    syscall

    # Save Snapshot
    call mem_snapshot_save

    # Update Tracker
    call mem_snapshot_update_tracker
    jmp .sig_exit

.handle_reset:
    # Print Msg
    movq $SYS_WRITE, %rax
    movq $1, %rdi
    leaq msg_reset(%rip), %rsi
    movq $47, %rdx
    syscall

    # Perform System Reset
    call mem_reset
    jmp .sig_exit

.sig_exit:
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi
    popq %rax
    ret

# ------------------------------------------------------------------------------
# SYSCALL WRAPPERS (Menggunakan Macro)
# ------------------------------------------------------------------------------

# Fungsi: __sys_write(fd, buffer, length)
__sys_write:
    OS_WRITE %rdi, (%rsi), %rdx
    ret

# Fungsi: __sys_exit(code)
__sys_exit:
    OS_EXIT %rdi
    # Unreachable

# Fungsi: __mem_map(size) -> wrapper legacy, akan diganti allocator baru
# Input: rdi = size
__mem_map:
    # Peringatan: Macro OS_ALLOC_PAGE saat ini hardcode 4096 bytes.
    # Nanti allocator logic yang akan handle looping jika butuh > 4KB.
    OS_ALLOC_PAGE %rdi
    ret

# Fungsi: __mem_unmap(ptr, size)
__mem_unmap:
    mov $11, %rax       # Syscall ID 11 = sys_munmap
    syscall
    ret
