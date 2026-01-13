; corelib/platform/x86_64/asm_win/test_net.asm
; Test Program: Resolve Domain & Send HTTP GET (Windows)
; Requires: google.com (or similar) to be resolvable via 8.8.8.8

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
hostname        db "google.com", 0
hostname_len    dq 10
http_req        db "GET / HTTP/1.0", 13, 10, "Host: google.com", 13, 10, 13, 10
http_req_len    dq 40
msg_resolving   db "[Net] Resolving google.com...", 10, 0
msg_connect     db "[Net] Connecting to IP...", 10, 0
msg_send        db "[Net] Sending HTTP Request...", 10, 0
msg_recv        db "[Net] Response Received:", 10, 0

section .bss
target_ip       resd 1
recv_buf        resb 1024
sockaddr_http   resb 16

section .text
global main
extern __mf_runtime_init
extern __mf_print_asciz
extern __mf_dns_resolve
extern __mf_net_socket
extern __mf_net_connect
extern __mf_net_send
extern __mf_net_recv
extern __mf_net_close
extern __mf_print_int
extern OS_EXIT

main:
    push rbp
    mov rbp, rsp
    sub rsp, 48

    call __mf_runtime_init

    ; 1. Resolve DNS
    lea rcx, [rel msg_resolving]
    call __mf_print_asciz

    lea rcx, [rel hostname]
    mov rdx, [rel hostname_len]
    call __mf_dns_resolve

    test eax, eax
    jz .fail_dns

    mov [rel target_ip], eax

    ; 2. Connect TCP to IP:80
    lea rcx, [rel msg_connect]
    call __mf_print_asciz

    ; socket(AF_INET=2, SOCK_STREAM=1, IPPROTO_TCP=6)
    mov rcx, 2
    mov rdx, 1
    mov r8, 6
    call __mf_net_socket
    mov r12, rax ; Socket FD

    ; Setup Address
    lea rdi, [rel sockaddr_http]
    mov word [rdi + 0], 2      ; AF_INET
    mov word [rdi + 2], 0x5000 ; Port 80 (0x0050 -> 0x5000 Big Endian)

    ; Copy IP
    mov eax, [rel target_ip]
    mov [rdi + 4], eax

    ; Connect
    mov rcx, r12
    lea rdx, [rel sockaddr_http]
    mov r8, 16
    call __mf_net_connect

    test rax, rax
    jnz .fail_connect

    ; 3. Send HTTP GET
    lea rcx, [rel msg_send]
    call __mf_print_asciz

    mov rcx, r12
    lea rdx, [rel http_req]
    mov r8, [rel http_req_len]
    call __mf_net_send

    ; 4. Recv Response
    lea rcx, [rel msg_recv]
    call __mf_print_asciz

    mov rcx, r12
    lea rdx, [rel recv_buf]
    mov r8, 1023
    call __mf_net_recv

    ; Null terminate and print
    test rax, rax
    jle .done

    lea rdx, [rel recv_buf]
    mov byte [rdx + rax], 0

    mov rcx, rdx
    call __mf_print_asciz

.done:
    mov rcx, r12
    call __mf_net_close

    mov rcx, 0
    call OS_EXIT

.fail_dns:
    mov rcx, 1
    call OS_EXIT

.fail_connect:
    mov rcx, 2
    call OS_EXIT
