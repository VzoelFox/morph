# macros.inc - Linux x86_64 (GAS Syntax)
# ==============================================================================
# Abstraksi System Call untuk Linux
# ==============================================================================

.equ SYS_READ, 0
.equ SYS_WRITE, 1
.equ SYS_OPEN, 2
.equ SYS_CLOSE, 3
.equ SYS_STAT, 4
.equ SYS_MMAP, 9
.equ SYS_MUNMAP, 11
.equ SYS_RT_SIGACTION, 13
.equ SYS_RT_SIGRETURN, 15
.equ SYS_PAUSE, 34
.equ SYS_GETPID, 39
.equ SYS_EXIT, 60
.equ SYS_MKDIR, 83
.equ SYS_TIME, 201

# File Flags
.equ O_RDONLY, 0
.equ O_RDWR, 2
.equ O_CREAT, 64
.equ O_TRUNC, 512

# Signals
.equ SIGUSR1, 10
.equ SIGUSR2, 12

.equ PROT_READ_WRITE, 0x3   # PROT_READ | PROT_WRITE
.equ MAP_PRIVATE_ANON, 0x22 # MAP_PRIVATE | MAP_ANONYMOUS

# ------------------------------------------------------------------------------
# Macro: OS_EXIT
# Arg: status_code (register or immediate)
# ------------------------------------------------------------------------------
.macro OS_EXIT status
    movq \status, %rdi
    movq $SYS_EXIT, %rax
    syscall
.endm

# ------------------------------------------------------------------------------
# Macro: OS_WRITE
# Arg: fd, buffer, length
# ------------------------------------------------------------------------------
.macro OS_WRITE fd, buffer, length
    # Order matters to avoid clobbering if using registers RDI, RSI, RDX as sources.
    # Target: RDI=fd, RSI=buf, RDX=len
    # We move in reverse order of likelihood of conflict?
    # Actually, best is to use scratch if needed, but for now strict ordering:
    # 1. RDX (Length) - Safe usually
    # 2. RSI (Buffer)
    # 3. RDI (FD) - Last because it's the first arg in ABI

    movq \length, %rdx
    leaq \buffer, %rsi
    movq \fd, %rdi
    movq $SYS_WRITE, %rax
    syscall
.endm

# ------------------------------------------------------------------------------
# Macro: OS_ALLOC_PAGE
# Arg: size (register or immediate)
# Output: %rax (Pointer to new page or error)
# ------------------------------------------------------------------------------
.macro OS_ALLOC_PAGE size
    movq $0, %rdi               # addr = NULL
    movq \size, %rsi            # len = size
    movq $PROT_READ_WRITE, %rdx # prot = READ | WRITE
    movq $MAP_PRIVATE_ANON, %r10 # flags = PRIVATE | ANON
    movq $-1, %r8               # fd = -1
    movq $0, %r9                # offset = 0
    movq $SYS_MMAP, %rax
    syscall
.endm
