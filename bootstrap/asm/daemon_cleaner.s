# bootstrap/asm/daemon_cleaner.s
# Native Daemon Cleaner Implementation - Replaces morph_cleaner.sh
# ==============================================================================
# Fungsi:
# - Monitor /tmp/morph_swap_monitor/ untuk snapshot dan sandbox files
# - Clean files yang sudah expired (TTL check)
# - Monitor memory usage dan trigger cleanup jika high
# - Berjalan sebagai background daemon process

.include "bootstrap/asm/macros.inc"

# Syscall numbers (Linux x86_64)
.equ SYS_READ, 0
.equ SYS_WRITE, 1
.equ SYS_OPEN, 2
.equ SYS_CLOSE, 3
.equ SYS_TIME, 201
.equ SYS_NANOSLEEP, 35
.equ SYS_GETDENTS64, 217
.equ SYS_UNLINK, 87
.equ SYS_KILL, 62
.equ SYS_FORK, 57
.equ SYS_SETSID, 112
.equ SYS_CHDIR, 80

# Open flags
.equ O_RDONLY, 0
.equ O_WRONLY, 1
.equ O_CREAT, 64
.equ O_TRUNC, 512
.equ O_DIRECTORY, 65536

# Signals
.equ SIGUSR2, 12

.section .data
    # Paths
    monitor_dir: .asciz "/tmp/morph_swap_monitor"
    snapshot_prefix: .asciz "snapshot_"
    sandbox_prefix: .asciz "sandbox_"
    dump_prefix: .asciz "dump_"

    # Configuration (matching bash script)
    .global daemon_snapshot_ttl
    .global daemon_sandbox_ttl
    .global daemon_check_interval

    daemon_snapshot_ttl: .quad 300      # 5 minutes
    daemon_sandbox_ttl: .quad 60        # 1 minute
    daemon_check_interval: .quad 2      # 2 seconds

    # Log messages
    msg_daemon_start: .asciz "[Daemon] MorphFox Cleaner started\n"
    msg_cleaning_snap: .asciz "[Daemon] Cleaning old snapshot: "
    msg_cleaning_sand: .asciz "[Daemon] Cleaning old sandbox: "
    msg_high_memory: .asciz "[Daemon] High memory usage detected\n"
    msg_newline: .asciz "\n"

    # Buffer for directory scanning
    .lcomm dirent_buffer, 280       # struct linux_dirent64
    .lcomm path_buffer, 512
    .lcomm time_buffer, 64

.section .text
.global daemon_start
.global daemon_loop
.global daemon_clean_snapshots
.global daemon_clean_sandboxes
.global daemon_monitor_memory

# ------------------------------------------------------------------------------
# func daemon_start() -> pid
# Fork daemon process, return PID in parent, 0 in child
# ------------------------------------------------------------------------------
daemon_start:
    pushq %rbp
    movq %rsp, %rbp

    # Fork
    movq $SYS_FORK, %rax
    syscall

    testq %rax, %rax
    jl .fork_error      # Error
    jnz .parent_return  # Parent

    # Child: Setup daemon environment
    # 1. Create session leader (setsid)
    movq $SYS_SETSID, %rax
    syscall

    # 2. Change working directory to /
    movq $SYS_CHDIR, %rax
    leaq root_dir(%rip), %rdi
    syscall

    # 3. Print start message
    OS_WRITE $1, msg_daemon_start, $35

    # 4. Enter main loop
    call daemon_loop

    # Should never reach here, but exit cleanly
    OS_EXIT $0

.parent_return:
    # Parent: Return child PID
    leave
    ret

.fork_error:
    # Fork failed, return -1
    movq $-1, %rax
    leave
    ret

# ------------------------------------------------------------------------------
# func daemon_loop()
# Main daemon loop: clean snapshots, sandboxes, monitor memory
# ------------------------------------------------------------------------------
daemon_loop:
    pushq %rbp
    movq %rsp, %rbp

.daemon_main_loop:
    # 1. Clean snapshots
    call daemon_clean_snapshots

    # 2. Clean sandboxes
    call daemon_clean_sandboxes

    # 3. Monitor memory
    call daemon_monitor_memory

    # 4. Sleep for check interval
    movq daemon_check_interval(%rip), %rdi
    call daemon_sleep

    jmp .daemon_main_loop

    # Never returns
    leave
    ret

# ------------------------------------------------------------------------------
# func daemon_clean_snapshots()
# Scan /tmp/morph_swap_monitor/ for snapshot_* files and clean old ones
# ------------------------------------------------------------------------------
daemon_clean_snapshots:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12      # Current time
    pushq %r13      # File descriptor
    pushq %r14      # Bytes read
    pushq %r15      # File timestamp

    # Get current time
    movq $SYS_TIME, %rax
    xorq %rdi, %rdi
    syscall
    movq %rax, %r12     # R12 = Now

    # Open directory
    movq $SYS_OPEN, %rax
    leaq monitor_dir(%rip), %rdi
    movq $O_RDONLY, %rsi
    orq $O_DIRECTORY, %rsi
    xorq %rdx, %rdx
    syscall

    testq %rax, %rax
    js .clean_snap_done     # Directory doesn't exist or error
    movq %rax, %r13         # R13 = Directory FD

.scan_snap_loop:
    # getdents64(fd, dirp, count)
    movq $SYS_GETDENTS64, %rax
    movq %r13, %rdi
    leaq dirent_buffer(%rip), %rsi
    movq $280, %rdx
    syscall

    testq %rax, %rax
    jle .scan_snap_done     # No more entries
    movq %rax, %r14         # R14 = Bytes read

    # Parse dirent (simplified - just check prefix)
    # struct linux_dirent64: ino(8), off(8), reclen(2), type(1), name(...)
    leaq dirent_buffer(%rip), %rsi
    addq $19, %rsi          # Skip to name field

    # Check if filename starts with "snapshot_"
    leaq snapshot_prefix(%rip), %rdi
    movq $9, %rcx           # Length of "snapshot_"
    call strncmp_check
    testq %rax, %rax
    jnz .scan_snap_loop     # Not a snapshot file

    # It's a snapshot file - construct full path
    leaq path_buffer(%rip), %rdi
    leaq monitor_dir(%rip), %rsi
    call strcpy_simple

    leaq path_buffer(%rip), %rdi
    call strlen_simple
    leaq path_buffer(%rip), %rdi
    addq %rax, %rdi
    movb $'/', (%rdi)
    incq %rdi

    leaq dirent_buffer(%rip), %rsi
    addq $19, %rsi
    call strcpy_simple

    # Read timestamp from file
    movq $SYS_OPEN, %rax
    leaq path_buffer(%rip), %rdi
    movq $O_RDONLY, %rsi
    xorq %rdx, %rdx
    syscall

    testq %rax, %rax
    js .scan_snap_loop      # Can't open, skip
    movq %rax, %r15         # R15 = File FD

    # Read timestamp (as text)
    movq $SYS_READ, %rax
    movq %r15, %rdi
    leaq time_buffer(%rip), %rsi
    movq $64, %rdx
    syscall

    # Close file
    pushq %rax
    movq $SYS_CLOSE, %rax
    movq %r15, %rdi
    syscall
    popq %rcx               # RCX = bytes read

    testq %rcx, %rcx
    jle .scan_snap_loop     # Empty file

    # Parse timestamp (atoi)
    leaq time_buffer(%rip), %rdi
    call atoi_simple
    movq %rax, %r15         # R15 = File timestamp

    # Calculate age
    movq %r12, %rax
    subq %r15, %rax         # Age = Now - File_timestamp

    # Check if expired
    cmpq daemon_snapshot_ttl(%rip), %rax
    jle .scan_snap_loop     # Not expired yet

    # Expired - delete file
    # Print message
    OS_WRITE $1, msg_cleaning_snap, $33
    leaq path_buffer(%rip), %rdi
    call strlen_simple
    OS_WRITE $1, path_buffer, %rax
    OS_WRITE $1, msg_newline, $1

    # Unlink file
    movq $SYS_UNLINK, %rax
    leaq path_buffer(%rip), %rdi
    syscall

    jmp .scan_snap_loop

.scan_snap_done:
    # Close directory
    movq $SYS_CLOSE, %rax
    movq %r13, %rdi
    syscall

.clean_snap_done:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func daemon_clean_sandboxes()
# Scan for sandbox_* files and send SIGUSR2 to old processes
# ------------------------------------------------------------------------------
daemon_clean_sandboxes:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12      # Current time
    pushq %r13      # Directory FD
    pushq %r14      # File timestamp
    pushq %r15      # PID

    # Get current time
    movq $SYS_TIME, %rax
    xorq %rdi, %rdi
    syscall
    movq %rax, %r12

    # Open directory
    movq $SYS_OPEN, %rax
    leaq monitor_dir(%rip), %rdi
    movq $O_RDONLY, %rsi
    orq $O_DIRECTORY, %rsi
    xorq %rdx, %rdx
    syscall

    testq %rax, %rax
    js .clean_sand_done
    movq %rax, %r13

.scan_sand_loop:
    # getdents64
    movq $SYS_GETDENTS64, %rax
    movq %r13, %rdi
    leaq dirent_buffer(%rip), %rsi
    movq $280, %rdx
    syscall

    testq %rax, %rax
    jle .scan_sand_done

    # Check prefix "sandbox_"
    leaq dirent_buffer(%rip), %rsi
    addq $19, %rsi
    leaq sandbox_prefix(%rip), %rdi
    movq $8, %rcx
    call strncmp_check
    testq %rax, %rax
    jnz .scan_sand_loop

    # Construct full path
    leaq path_buffer(%rip), %rdi
    leaq monitor_dir(%rip), %rsi
    call strcpy_simple
    leaq path_buffer(%rip), %rdi
    call strlen_simple
    leaq path_buffer(%rip), %rdi
    addq %rax, %rdi
    movb $'/', (%rdi)
    incq %rdi
    leaq dirent_buffer(%rip), %rsi
    addq $19, %rsi
    call strcpy_simple

    # Read timestamp
    movq $SYS_OPEN, %rax
    leaq path_buffer(%rip), %rdi
    movq $O_RDONLY, %rsi
    xorq %rdx, %rdx
    syscall
    testq %rax, %rax
    js .scan_sand_loop
    movq %rax, %r15

    movq $SYS_READ, %rax
    movq %r15, %rdi
    leaq time_buffer(%rip), %rsi
    movq $64, %rdx
    syscall

    pushq %rax
    movq $SYS_CLOSE, %rax
    movq %r15, %rdi
    syscall
    popq %rcx

    testq %rcx, %rcx
    jle .scan_sand_loop

    # Parse timestamp
    leaq time_buffer(%rip), %rdi
    call atoi_simple
    movq %rax, %r14

    # Calculate age
    movq %r12, %rax
    subq %r14, %rax

    # Check TTL
    cmpq daemon_sandbox_ttl(%rip), %rax
    jle .scan_sand_loop

    # Extract PID from filename "sandbox_<PID>"
    leaq dirent_buffer(%rip), %rdi
    addq $27, %rdi      # Skip "sandbox_"
    call atoi_simple
    movq %rax, %r15     # PID

    # Send SIGUSR2 to process
    movq $SYS_KILL, %rax
    movq %r15, %rdi     # PID
    movq $SIGUSR2, %rsi
    syscall

    # Print message
    OS_WRITE $1, msg_cleaning_sand, $33
    leaq path_buffer(%rip), %rdi
    call strlen_simple
    OS_WRITE $1, path_buffer, %rax
    OS_WRITE $1, msg_newline, $1

    # Delete file
    movq $SYS_UNLINK, %rax
    leaq path_buffer(%rip), %rdi
    syscall

    jmp .scan_sand_loop

.scan_sand_done:
    movq $SYS_CLOSE, %rax
    movq %r13, %rdi
    syscall

.clean_sand_done:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func daemon_monitor_memory()
# Check memory usage via /proc/meminfo, trigger cleanup if >80%
# ------------------------------------------------------------------------------
daemon_monitor_memory:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12      # MemTotal
    pushq %r13      # MemAvailable

    # Open /proc/meminfo
    movq $SYS_OPEN, %rax
    leaq meminfo_path(%rip), %rdi
    movq $O_RDONLY, %rsi
    xorq %rdx, %rdx
    syscall

    testq %rax, %rax
    js .mem_done
    movq %rax, %r14     # FD

    # Read into buffer
    movq $SYS_READ, %rax
    movq %r14, %rdi
    leaq path_buffer(%rip), %rsi    # Reuse path_buffer
    movq $512, %rdx
    syscall

    # Close
    pushq %rax
    movq $SYS_CLOSE, %rax
    movq %r14, %rdi
    syscall
    popq %rcx       # Bytes read

    # Parse MemTotal and MemAvailable
    # Buffer now contains /proc/meminfo content
    testq %rcx, %rcx
    jle .mem_done       # No data read

    # Find "MemTotal:" line
    leaq path_buffer(%rip), %rdi
    leaq str_memtotal(%rip), %rsi
    call find_meminfo_line
    testq %rax, %rax
    jz .mem_done        # Not found
    movq %rax, %rdi
    call parse_meminfo_value
    movq %rax, %r12     # R12 = MemTotal (in kB)

    # Find "MemAvailable:" line
    leaq path_buffer(%rip), %rdi
    leaq str_memavail(%rip), %rsi
    call find_meminfo_line
    testq %rax, %rax
    jz .try_memfree     # MemAvailable not found, try MemFree
    movq %rax, %rdi
    call parse_meminfo_value
    movq %rax, %r13     # R13 = MemAvailable (in kB)
    jmp .calculate_usage

.try_memfree:
    # Fallback: Use MemFree if MemAvailable not available
    leaq path_buffer(%rip), %rdi
    leaq str_memfree(%rip), %rsi
    call find_meminfo_line
    testq %rax, %rax
    jz .mem_done        # Neither found
    movq %rax, %rdi
    call parse_meminfo_value
    movq %rax, %r13     # R13 = MemFree (in kB)

.calculate_usage:
    # Calculate percentage: (MemTotal - MemAvailable) * 100 / MemTotal
    movq %r12, %rax
    subq %r13, %rax     # Used = Total - Available
    imulq $100, %rax
    xorq %rdx, %rdx
    divq %r12           # RAX = (Used * 100) / Total = Usage %

    # Check if >80%
    cmpq $80, %rax
    jle .mem_done

    # High memory usage detected - print warning
    OS_WRITE $1, msg_high_memory, $37

.mem_done:
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# Helper Functions
# ------------------------------------------------------------------------------

# daemon_sleep(seconds)
daemon_sleep:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp

    # timespec: tv_sec, tv_nsec
    movq %rdi, 0(%rsp)      # seconds
    movq $0, 8(%rsp)        # nanoseconds

    movq $SYS_NANOSLEEP, %rax
    movq %rsp, %rdi         # req
    xorq %rsi, %rsi         # rem = NULL
    syscall

    leave
    ret

# strncmp_check(s1, s2, n) - Compare n bytes
strncmp_check:
    xorq %rax, %rax
.strncmp_loop:
    testq %rcx, %rcx
    jz .strncmp_equal
    movb (%rdi), %al
    cmpb (%rsi), %al
    jne .strncmp_not_equal
    incq %rdi
    incq %rsi
    decq %rcx
    jmp .strncmp_loop
.strncmp_equal:
    xorq %rax, %rax
    ret
.strncmp_not_equal:
    movq $1, %rax
    ret

# strcpy_simple(dest, src)
strcpy_simple:
    xorq %rax, %rax
.strcpy_loop:
    movb (%rsi), %al
    movb %al, (%rdi)
    testb %al, %al
    jz .strcpy_done
    incq %rsi
    incq %rdi
    jmp .strcpy_loop
.strcpy_done:
    ret

# strlen_simple(str) -> length
strlen_simple:
    xorq %rax, %rax
.strlen_loop:
    cmpb $0, (%rdi)
    je .strlen_done
    incq %rax
    incq %rdi
    jmp .strlen_loop
.strlen_done:
    ret

# atoi_simple(str) -> number
atoi_simple:
    xorq %rax, %rax     # Result
    xorq %rcx, %rcx     # Temp digit
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

# find_meminfo_line(buffer, search_str) -> ptr to value or 0
# Searches for a line starting with search_str in buffer
# Returns pointer to the start of the numeric value
find_meminfo_line:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12      # Buffer pointer
    pushq %r13      # Search string
    pushq %r14      # Current position

    movq %rdi, %r12
    movq %rsi, %r13
    movq %rdi, %r14

.find_line_loop:
    # Check for end of buffer (null or beyond 512 bytes)
    movq %r14, %rax
    subq %r12, %rax
    cmpq $512, %rax
    jge .find_not_found

    movb (%r14), %al
    testb %al, %al
    jz .find_not_found

    # Check if current position matches search string
    movq %r14, %rdi
    movq %r13, %rsi
    call substr_match
    testq %rax, %rax
    jnz .find_found

    # Move to next line
.find_next_line:
    movb (%r14), %al
    incq %r14
    cmpb $'\n', %al
    jne .find_next_line
    jmp .find_line_loop

.find_found:
    # Skip past the search string and ':'
    movq %r14, %rax
.skip_label:
    movb (%rax), %cl
    incq %rax
    cmpb $':', %cl
    jne .skip_label

    # Skip whitespace
.skip_space:
    movb (%rax), %cl
    cmpb $' ', %cl
    je .skip_space_inc
    cmpb $'\t', %cl
    jne .find_return
.skip_space_inc:
    incq %rax
    jmp .skip_space

.find_return:
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

.find_not_found:
    xorq %rax, %rax
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

# substr_match(str1, str2) -> 1 if str1 starts with str2, 0 otherwise
substr_match:
    pushq %rdi
    pushq %rsi
.substr_loop:
    movb (%rsi), %al
    testb %al, %al
    jz .substr_match_yes    # Reached end of str2, match!
    cmpb (%rdi), %al
    jne .substr_match_no
    incq %rdi
    incq %rsi
    jmp .substr_loop
.substr_match_yes:
    movq $1, %rax
    popq %rsi
    popq %rdi
    ret
.substr_match_no:
    xorq %rax, %rax
    popq %rsi
    popq %rdi
    ret

# parse_meminfo_value(ptr) -> value in kB
# Parse numeric value from meminfo line (assumes ptr points to start of number)
parse_meminfo_value:
    pushq %rbp
    movq %rsp, %rbp

    # Use atoi_simple to parse the number
    call atoi_simple

    leave
    ret

# Additional data
.section .data
root_dir: .asciz "/"
meminfo_path: .asciz "/proc/meminfo"
str_memtotal: .asciz "MemTotal"
str_memavail: .asciz "MemAvailable"
str_memfree: .asciz "MemFree"
