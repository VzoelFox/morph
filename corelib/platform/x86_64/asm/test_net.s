# corelib/platform/x86_64/asm/test_net.s
# Test Program: Resolve Domain & Send HTTP GET
# Requires: google.com (or similar) to be resolvable via 8.8.8.8

.include "corelib/platform/x86_64/asm/syscalls.inc"

.section .data
hostname:       .asciz "google.com"
hostname_len:   .quad 10
http_req:       .ascii "GET / HTTP/1.0\r\nHost: google.com\r\n\r\n"
http_req_end:
msg_resolving:  .asciz "[Net] Resolving google.com...\n"
msg_connect:    .asciz "[Net] Connecting to IP...\n"
msg_send:       .asciz "[Net] Sending HTTP Request...\n"
msg_recv:       .asciz "[Net] Response Received:\n"

.section .bss
target_ip:      .skip 4
recv_buf:       .skip 1024
sockaddr_http:  .skip 16

.section .text
.global main
.extern __mf_runtime_init
.extern __mf_print_asciz
.extern __mf_dns_resolve
.extern __mf_net_socket
.extern __mf_net_connect
.extern __mf_net_send
.extern __mf_net_recv
.extern __mf_net_close
.extern __mf_print_int

main:
    call __mf_runtime_init

    # 1. Resolve DNS
    leaq msg_resolving(%rip), %rdi
    call __mf_print_asciz

    leaq hostname(%rip), %rdi
    movq hostname_len(%rip), %rsi
    call __mf_dns_resolve

    testl %eax, %eax
    jz .fail_dns

    movl %eax, target_ip(%rip)

    # 2. Connect TCP to IP:80
    leaq msg_connect(%rip), %rdi
    call __mf_print_asciz

    # socket(AF_INET=2, SOCK_STREAM=1, IPPROTO_TCP=6)
    movq $2, %rdi
    movq $1, %rsi
    movq $6, %rdx
    call __mf_net_socket
    movq %rax, %r12 # Socket FD

    # Setup Address
    leaq sockaddr_http(%rip), %rdi
    movw $2, 0(%rdi)      # AF_INET
    movw $0x5000, 2(%rdi) # Port 80 (0x0050 -> 0x5000 Big Endian)

    # Copy IP
    movl target_ip(%rip), %eax
    movl %eax, 4(%rdi)

    # Connect
    movq %r12, %rdi
    leaq sockaddr_http(%rip), %rsi
    movq $16, %rdx
    call __mf_net_connect

    testq %rax, %rax
    jnz .fail_connect

    # 3. Send HTTP GET
    leaq msg_send(%rip), %rdi
    call __mf_print_asciz

    movq %r12, %rdi
    leaq http_req(%rip), %rsi
    leaq http_req_end(%rip), %rdx
    subq %rsi, %rdx # Len
    call __mf_net_send

    # 4. Recv Response
    leaq msg_recv(%rip), %rdi
    call __mf_print_asciz

    movq %r12, %rdi
    leaq recv_buf(%rip), %rsi
    movq $1023, %rdx
    call __mf_net_recv

    # Null terminate and print
    testq %rax, %rax
    jle .done

    leaq recv_buf(%rip), %rsi
    movb $0, (%rsi, %rax)

    movq %rsi, %rdi
    call __mf_print_asciz

.done:
    movq %r12, %rdi
    call __mf_net_close

    movq $60, %rax
    xorq %rdi, %rdi
    syscall

.fail_dns:
    movq $60, %rax
    movq $1, %rdi
    syscall

.fail_connect:
    movq $60, %rax
    movq $2, %rdi
    syscall
