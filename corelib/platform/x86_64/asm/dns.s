# corelib/platform/x86_64/asm/dns.s
# Manual DNS Resolver (UDP Port 53) for Linux
# Resolves A Record (IPv4)

.include "corelib/platform/x86_64/asm/syscalls.inc"

.section .data
dns_server_ip: .byte 8, 8, 8, 8 # Google DNS
dns_port:      .word 0x3500     # 53 Big Endian (0x0035 -> 0x3500)

.section .bss
dns_packet_buf: .skip 512
recv_buf:       .skip 512
sockaddr_dest:  .skip 16

.section .text
.global __mf_dns_resolve
.extern __mf_net_socket
.extern __mf_net_send
.extern __mf_net_recv
.extern __mf_net_close
.extern __mf_memcpy

# ------------------------------------------------------------------------------
# func __mf_dns_resolve(hostname: ptr, hostname_len: i64) -> ipv4 (u32)
# Returns 0 on failure.
# ------------------------------------------------------------------------------
__mf_dns_resolve:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12 # Hostname
    pushq %r13 # Len
    pushq %r14 # Socket FD
    pushq %r15 # Packet Len

    movq %rdi, %r12
    movq %rsi, %r13

    # 0. Safety Check: Hostname Length
    cmpq $255, %r13
    jg .dns_fail

    # 1. Create UDP Socket
    # socket(AF_INET=2, SOCK_DGRAM=2, IPPROTO_UDP=17)
    movq $2, %rdi
    movq $2, %rsi
    movq $17, %rdx
    call __mf_net_socket

    cmpq $0, %rax
    js .dns_fail
    movq %rax, %r14

    # 2. Construct DNS Packet
    leaq dns_packet_buf(%rip), %rdi

    # Header (12 bytes)
    # ID = 0x1234
    movw $0x3412, 0(%rdi)
    # Flags = 0x0100 (Standard Query, Recursion Desired) -> 0x0001 Big Endian?
    # 0x0100 in Big Endian is 0x01 0x00.
    # Flags: QR(0), Op(0), AA(0), TC(0), RD(1).
    # Byte 0: 00000001 = 0x01. Byte 1: 00000000 = 0x00.
    # So 0x0100.
    movw $0x0001, 2(%rdi)
    # QDCOUNT = 1
    movw $0x0100, 4(%rdi)
    # ANCOUNT = 0
    movw $0x0000, 6(%rdi)
    # NSCOUNT = 0
    movw $0x0000, 8(%rdi)
    # ARCOUNT = 0
    movw $0x0000, 10(%rdi)

    # Question Section (QNAME)
    # Convert "google.com" to 6google3com0
    leaq 12(%rdi), %rbx # Current Dest Ptr
    movq %r12, %rsi     # Source Ptr
    movq %r13, %rcx     # Remaining Source Len

    # Logic: Read segment until dot or end. Write len, then segment.

    # Save start of segment in output
    movq %rbx, %r8      # Segment Len Ptr
    incq %rbx           # Skip Len Byte
    xorq %r9, %r9       # Segment Len Counter

.qname_loop:
    testq %rcx, %rcx
    jz .qname_segment_done

    movb (%rsi), %al
    cmpb $'.', %al
    je .qname_segment_done

    movb %al, (%rbx)
    incq %rbx
    incq %rsi
    incq %r9
    decq %rcx
    jmp .qname_loop

.qname_segment_done:
    # Write Len to %r8
    movb %r9b, (%r8)

    # Check if more
    testq %rcx, %rcx
    jz .qname_finish

    # Skip dot
    incq %rsi
    decq %rcx

    # New Segment
    movq %rbx, %r8
    incq %rbx
    xorq %r9, %r9
    jmp .qname_loop

.qname_finish:
    movb $0, (%rbx) # Null terminator
    incq %rbx

    # QTYPE = A (1)
    movw $0x0100, (%rbx)
    addq $2, %rbx
    # QCLASS = IN (1)
    movw $0x0100, (%rbx)
    addq $2, %rbx

    # Packet Len
    leaq dns_packet_buf(%rip), %rax
    subq %rax, %rbx
    movq %rbx, %r15

    # 3. Setup Dest Address (8.8.8.8:53)
    leaq sockaddr_dest(%rip), %rdi
    # Family AF_INET = 2
    movw $2, 0(%rdi)
    # Port 53 (0x3500 Big Endian)
    movw $0x3500, 2(%rdi)
    # Addr 8.8.8.8
    movb $8, 4(%rdi)
    movb $8, 5(%rdi)
    movb $8, 6(%rdi)
    movb $8, 7(%rdi)

    # 4. Connect UDP Socket (to set default dest for send/recv)
    # connect(fd, &addr, 16)
    movq %r14, %rdi
    leaq sockaddr_dest(%rip), %rsi
    movq $16, %rdx
    call __mf_net_connect

    cmpq $0, %rax
    jnz .dns_close_fail

    # 5. Send Packet
    # send(fd, buf, len)
    movq %r14, %rdi
    leaq dns_packet_buf(%rip), %rsi
    movq %r15, %rdx
    call __mf_net_send

    # 6. Recv Response
    movq %r14, %rdi
    leaq recv_buf(%rip), %rsi
    movq $512, %rdx
    call __mf_net_recv

    cmpq $0, %rax
    jle .dns_close_fail

    # 7. Parse Response
    # Header (12 bytes). Check RCODE then skip.
    # Question. Skip.
    # Answer. Read.

    # Check RCODE in DNS Header (byte 3, bits 0-3)
    # RCODE 0 = No Error, RCODE 3 = NXDOMAIN, etc.
    leaq recv_buf(%rip), %rbx
    movzbq 3(%rbx), %rax
    andb $0x0F, %al # Mask to get RCODE (4 bits)
    testb %al, %al
    jnz .dns_close_fail # RCODE != 0, error occurred

    # We need to skip the Question section in the Response.
    # It mirrors the Request.
    # Packet Start: recv_buf
    addq $12, %rbx # Skip Header

    # Skip Name (Labels)
.skip_name_loop:
    movzbq (%rbx), %rcx
    testq %rcx, %rcx
    jz .skip_name_done

    # Check Compression Pointer (Top 2 bits 11)
    # 0xC0 = 11000000
    movb (%rbx), %al
    andb $0xC0, %al
    cmpb $0xC0, %al
    je .skip_ptr_done

    # Label: Len + Bytes
    leaq 1(%rbx, %rcx), %rbx
    jmp .skip_name_loop

.skip_ptr_done:
    addq $2, %rbx
    jmp .parse_answer

.skip_name_done:
    incq %rbx # Skip null byte
    addq $4, %rbx # Skip Type & Class (4 bytes)

.parse_answer:
    # Now at Answer Section?
    # Wait, we just skipped Question.
    # QTYPE (2) + QCLASS (2)
    # If we used compression pointer logic above, we might be at Type/Class.
    # If standard name, we are at Type/Class.
    # Let's align.

    # Logic:
    # 1. Skip Name (Variable)
    # 2. Skip Type (2)
    # 3. Skip Class (2)
    # 4. TTL (4)
    # 5. RDLENGTH (2)
    # 6. RDATA (Variable)

    # The loop above skipped Name.
    # If ptr, we added 2. Done.
    # If labels, we ended at null + 1. Done.

    # BUT, we need to skip QTYPE and QCLASS of the QUESTION section first.
    # The loop above ends at start of QTYPE.
    addq $4, %rbx # Skip QTYPE, QCLASS

    # NOW we are at Answer Section (if ANCOUNT > 0)
    # Assume 1 Answer.

    # Answer Name (Compressed usually c0 0c)
    movb (%rbx), %al
    andb $0xC0, %al
    cmpb $0xC0, %al
    je .ans_ptr
    # Else loop name... assume pointer for simplicity with 8.8.8.8
    jmp .dns_close_fail

.ans_ptr:
    addq $2, %rbx # Skip Name Ptr

    # Type (2) - Expect 1 (A)
    # Class (2) - Expect 1 (IN)
    addq $4, %rbx

    # TTL (4)
    addq $4, %rbx

    # RDLength (2)
    # Expect 4 for IPv4
    movzbq 1(%rbx), %rcx # Lower byte (Big Endian)
    # High byte at 0(%rbx) should be 0
    addq $2, %rbx

    cmpq $4, %rcx
    jne .dns_close_fail

    # IP is here! 4 bytes.
    # Load 32-bit IP
    movl (%rbx), %eax
    # Result in RAX (Low 32 bits)

    # Clean up
    pushq %rax
    movq %r14, %rdi
    call __mf_net_close
    popq %rax

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

.dns_close_fail:
    movq %r14, %rdi
    call __mf_net_close
.dns_fail:
    xorq %rax, %rax
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret
