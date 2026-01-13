; corelib/platform/x86_64/asm_win/dns.asm
; Manual DNS Resolver (UDP Port 53) for Windows
; Resolves A Record (IPv4)

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
dns_server_ip db 8, 8, 8, 8 ; Google DNS
dns_port      dw 0x3500     ; 53 Big Endian

section .bss
dns_packet_buf resb 512
recv_buf       resb 512
sockaddr_dest  resb 16

section .text
global __mf_dns_resolve
extern __mf_net_socket
extern __mf_net_connect
extern __mf_net_send
extern __mf_net_recv
extern __mf_net_close
extern __mf_memcpy

; ------------------------------------------------------------------------------
; func __mf_dns_resolve(hostname: ptr, hostname_len: i64) -> ipv4 (u32)
; Input: RCX, RDX
; ------------------------------------------------------------------------------
__mf_dns_resolve:
    push rbp
    mov rbp, rsp
    push rbx
    push r12 ; Hostname
    push r13 ; Len
    push r14 ; Socket FD
    push r15 ; Packet Len
    sub rsp, 32 ; Shadow

    mov r12, rcx
    mov r13, rdx

    ; 0. Safety Check: Hostname Length
    cmp r13, 255
    jg .dns_fail

    ; 1. Create UDP Socket
    mov rcx, 2 ; AF_INET
    mov rdx, 2 ; SOCK_DGRAM
    mov r8, 17 ; IPPROTO_UDP
    call __mf_net_socket

    test rax, rax
    js .dns_fail
    mov r14, rax

    ; 2. Construct DNS Packet
    lea rdi, [rel dns_packet_buf]

    ; Header
    mov word [rdi + 0], 0x3412 ; ID
    mov word [rdi + 2], 0x0001 ; Flags
    mov word [rdi + 4], 0x0100 ; QDCOUNT
    mov word [rdi + 6], 0x0000 ; ANCOUNT
    mov word [rdi + 8], 0x0000 ; NSCOUNT
    mov word [rdi + 10], 0x0000 ; ARCOUNT

    ; Question Section
    lea rbx, [rdi + 12] ; Current Dest Ptr
    mov rsi, r12        ; Source Ptr
    mov rcx, r13        ; Source Len

    mov r8, rbx         ; Segment Len Ptr
    inc rbx             ; Skip Len Byte
    xor r9, r9          ; Segment Len Counter

.qname_loop:
    test rcx, rcx
    jz .qname_segment_done

    mov al, [rsi]
    cmp al, '.'
    je .qname_segment_done

    mov [rbx], al
    inc rbx
    inc rsi
    inc r9
    dec rcx
    jmp .qname_loop

.qname_segment_done:
    mov [r8], r9b

    test rcx, rcx
    jz .qname_finish

    inc rsi
    dec rcx

    mov r8, rbx
    inc rbx
    xor r9, r9
    jmp .qname_loop

.qname_finish:
    mov byte [rbx], 0 ; Null terminator
    inc rbx

    ; QTYPE = A (1)
    mov word [rbx], 0x0100
    add rbx, 2
    ; QCLASS = IN (1)
    mov word [rbx], 0x0100
    add rbx, 2

    ; Packet Len
    lea rax, [rel dns_packet_buf]
    sub rbx, rax
    mov r15, rbx

    ; 3. Setup Dest Address (8.8.8.8:53)
    lea rdi, [rel sockaddr_dest]
    mov word [rdi + 0], 2 ; Family
    mov word [rdi + 2], 0x3500 ; Port 53
    mov byte [rdi + 4], 8
    mov byte [rdi + 5], 8
    mov byte [rdi + 6], 8
    mov byte [rdi + 7], 8

    ; 4. Connect UDP Socket
    mov rcx, r14
    lea rdx, [rel sockaddr_dest]
    mov r8, 16
    call __mf_net_connect

    test rax, rax
    jnz .dns_close_fail

    ; 5. Send Packet
    mov rcx, r14
    lea rdx, [rel dns_packet_buf]
    mov r8, r15
    call __mf_net_send

    ; 6. Recv Response
    mov rcx, r14
    lea rdx, [rel recv_buf]
    mov r8, 512
    call __mf_net_recv

    test rax, rax
    jle .dns_close_fail

    ; 7. Parse Response
    ; Check RCODE in DNS Header (byte 3, bits 0-3)
    ; RCODE 0 = No Error, RCODE 3 = NXDOMAIN, etc.
    lea rbx, [rel recv_buf]
    movzx rax, byte [rbx + 3]
    and al, 0x0F ; Mask to get RCODE (4 bits)
    test al, al
    jnz .dns_close_fail ; RCODE != 0, error occurred

    add rbx, 12 ; Skip Header

    ; Skip Name
.skip_name_loop:
    movzx rcx, byte [rbx]
    test rcx, rcx
    jz .skip_name_done

    mov al, [rbx]
    and al, 0xC0
    cmp al, 0xC0
    je .skip_ptr_done

    lea rbx, [rbx + rcx + 1]
    jmp .skip_name_loop

.skip_ptr_done:
    add rbx, 2
    jmp .parse_answer

.skip_name_done:
    inc rbx
    add rbx, 4

.parse_answer:
    add rbx, 4 ; Skip QTYPE, QCLASS of Question

    ; Answer Name
    mov al, [rbx]
    and al, 0xC0
    cmp al, 0xC0
    je .ans_ptr
    jmp .dns_close_fail

.ans_ptr:
    add rbx, 2 ; Skip Name Ptr

    add rbx, 4 ; Skip Type, Class
    add rbx, 4 ; Skip TTL

    ; RDLength
    movzx rcx, byte [rbx + 1]
    add rbx, 2

    cmp rcx, 4
    jne .dns_close_fail

    ; IP
    mov eax, [rbx]

    ; Clean up
    push rax
    mov rcx, r14
    sub rsp, 32
    call __mf_net_close
    add rsp, 32
    pop rax

    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

.dns_close_fail:
    mov rcx, r14
    sub rsp, 32
    call __mf_net_close
    add rsp, 32
.dns_fail:
    xor rax, rax
    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
