# corelib/platform/x86_64/asm/net.s
# Implementasi Networking (Linux x86_64) v1.1
# Wrapper System Calls

.include "corelib/platform/x86_64/asm/syscalls.inc"

.section .text
.global __mf_net_socket
.global __mf_net_connect
.global __mf_net_send
.global __mf_net_recv
.global __mf_net_close
.global __mf_net_bind
.global __mf_net_listen
.global __mf_net_accept

.extern __sys_exit

# SAFETY: Maximum reasonable buffer size (16MB)
.equ MAX_NET_BUFFER_SIZE, 16777216

# ------------------------------------------------------------------------------
# func __mf_net_socket(domain: i64, type: i64, proto: i64) -> fd
# ------------------------------------------------------------------------------
__mf_net_socket:
    # Args already in RDI, RSI, RDX match syscall args
    movq $SYS_SOCKET, %rax
    syscall
    ret

# ------------------------------------------------------------------------------
# func __mf_net_connect(fd: i64, addr_ptr: ptr, addr_len: i64) -> status
# ------------------------------------------------------------------------------
__mf_net_connect:
    # Args already in RDI, RSI, RDX match syscall args
    movq $SYS_CONNECT, %rax
    syscall
    ret

# ------------------------------------------------------------------------------
# func __mf_net_send(fd: i64, buf: ptr, len: i64) -> sent
# ------------------------------------------------------------------------------
__mf_net_send:
    # SAFETY: Validate buffer pointer is not NULL
    testq %rsi, %rsi
    jz .Lnet_null_buffer

    # SAFETY: Validate length is reasonable (not negative, not too large)
    testq %rdx, %rdx
    js .Lnet_invalid_length
    cmpq $MAX_NET_BUFFER_SIZE, %rdx
    jg .Lnet_buffer_too_large

    # sendto(fd, buf, len, flags, dest, addrlen)
    # Input: RDI, RSI, RDX
    xorq %r10, %r10     # flags = 0
    xorq %r8, %r8       # dest = NULL
    xorq %r9, %r9       # addrlen = 0

    movq $SYS_SENDTO, %rax
    syscall
    ret

# ------------------------------------------------------------------------------
# func __mf_net_recv(fd: i64, buf: ptr, len: i64) -> received
# ------------------------------------------------------------------------------
__mf_net_recv:
    # SAFETY: Validate buffer pointer is not NULL
    testq %rsi, %rsi
    jz .Lnet_null_buffer

    # SAFETY: Validate length is reasonable (not negative, not too large)
    testq %rdx, %rdx
    js .Lnet_invalid_length
    cmpq $MAX_NET_BUFFER_SIZE, %rdx
    jg .Lnet_buffer_too_large

    # recvfrom(fd, buf, len, flags, src, addrlen)
    xorq %r10, %r10     # flags = 0
    xorq %r8, %r8       # src = NULL
    xorq %r9, %r9       # addrlen = 0

    movq $SYS_RECVFROM, %rax
    syscall
    ret

# ------------------------------------------------------------------------------
# func __mf_net_bind(fd: i64, addr_ptr: ptr, addr_len: i64) -> status
# ------------------------------------------------------------------------------
__mf_net_bind:
    # bind(fd, addr, addrlen)
    # Input matches syscall args
    movq $SYS_BIND, %rax
    syscall
    ret

# ------------------------------------------------------------------------------
# func __mf_net_listen(fd: i64, backlog: i64) -> status
# ------------------------------------------------------------------------------
__mf_net_listen:
    # listen(fd, backlog)
    # Input matches syscall args
    movq $SYS_LISTEN, %rax
    syscall
    ret

# ------------------------------------------------------------------------------
# func __mf_net_accept(fd: i64, addr_ptr: ptr, len_ptr: ptr) -> new_fd
# ------------------------------------------------------------------------------
__mf_net_accept:
    # accept(fd, addr, addrlen)
    # Input matches syscall args
    movq $SYS_ACCEPT, %rax
    syscall
    ret

# ------------------------------------------------------------------------------
# func __mf_net_close(fd: i64) -> void
# ------------------------------------------------------------------------------
__mf_net_close:
    movq $3, %rax       # SYS_CLOSE
    syscall
    ret

# ------------------------------------------------------------------------------
# Error Handlers
# ------------------------------------------------------------------------------
.Lnet_null_buffer:
    # NULL buffer pointer provided
    movq $115, %rdi
    call __sys_exit

.Lnet_invalid_length:
    # Negative length provided
    movq $116, %rdi
    call __sys_exit

.Lnet_buffer_too_large:
    # Buffer size exceeds maximum
    movq $117, %rdi
    call __sys_exit
