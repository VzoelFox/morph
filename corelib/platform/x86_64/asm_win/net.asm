; corelib/platform/x86_64/asm_win/net.asm
; Implementasi Networking (Windows x86_64) v1.1
; Wrapper Winsock

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .bss
    wsa_initialized resb 1
    wsa_data resb 408 ; WSADATA struct

section .text
global __mf_net_socket
global __mf_net_connect
global __mf_net_send
global __mf_net_recv
global __mf_net_close
global __mf_net_bind
global __mf_net_listen
global __mf_net_accept

extern WSAStartup
extern socket
extern connect
extern send
extern recv
extern bind
extern listen
extern accept
extern closesocket
extern __sys_exit

; SAFETY: Maximum reasonable buffer size (16MB)
MAX_NET_BUFFER_SIZE equ 16777216

; ------------------------------------------------------------------------------
; Internal: Lazy Init WSA
; ------------------------------------------------------------------------------
_init_wsa:
    cmp byte [rel wsa_initialized], 1
    je .done

    push rbp
    mov rbp, rsp
    sub rsp, 48

    mov rcx, 0x0202 ; Version 2.2
    lea rdx, [rel wsa_data]
    call WSAStartup

    mov byte [rel wsa_initialized], 1

    add rsp, 48
    pop rbp
.done:
    ret

; ------------------------------------------------------------------------------
; func __mf_net_socket(domain: i64, type: i64, proto: i64) -> fd
; Input: RCX, RDX, R8
; ------------------------------------------------------------------------------
__mf_net_socket:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    push rcx
    push rdx
    push r8
    call _init_wsa
    pop r8
    pop rdx
    pop rcx

    call socket

    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_net_connect(fd: i64, addr_ptr: ptr, addr_len: i64) -> status
; Input: RCX, RDX, R8
; ------------------------------------------------------------------------------
__mf_net_connect:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    call connect

    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_net_send(fd: i64, buf: ptr, len: i64) -> sent
; Input: RCX, RDX, R8
; ------------------------------------------------------------------------------
__mf_net_send:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    ; SAFETY: Validate buffer pointer is not NULL
    test rdx, rdx
    jz L_net_null_buffer

    ; SAFETY: Validate length is reasonable (not negative, not too large)
    test r8, r8
    js L_net_invalid_length
    cmp r8, MAX_NET_BUFFER_SIZE
    jg L_net_buffer_too_large

    ; send(s, buf, len, flags)
    xor r9, r9
    call send

    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_net_recv(fd: i64, buf: ptr, len: i64) -> received
; Input: RCX, RDX, R8
; ------------------------------------------------------------------------------
__mf_net_recv:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    ; SAFETY: Validate buffer pointer is not NULL
    test rdx, rdx
    jz L_net_null_buffer

    ; SAFETY: Validate length is reasonable (not negative, not too large)
    test r8, r8
    js L_net_invalid_length
    cmp r8, MAX_NET_BUFFER_SIZE
    jg L_net_buffer_too_large

    ; recv(s, buf, len, flags)
    xor r9, r9
    call recv

    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_net_bind(fd: i64, addr_ptr: ptr, addr_len: i64) -> status
; Input: RCX, RDX, R8
; ------------------------------------------------------------------------------
__mf_net_bind:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    call bind

    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_net_listen(fd: i64, backlog: i64) -> status
; Input: RCX, RDX
; ------------------------------------------------------------------------------
__mf_net_listen:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    call listen

    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_net_accept(fd: i64, addr_ptr: ptr, len_ptr: ptr) -> new_fd
; Input: RCX, RDX, R8
; ------------------------------------------------------------------------------
__mf_net_accept:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    call accept

    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_net_close(fd: i64) -> void
; Input: RCX
; ------------------------------------------------------------------------------
__mf_net_close:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    call closesocket

    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; Error Handlers (Global Labels for Scoping)
; ------------------------------------------------------------------------------
L_net_null_buffer:
    ; NULL buffer pointer provided
    mov rcx, 115
    call __sys_exit

L_net_invalid_length:
    ; Negative length provided
    mov rcx, 116
    call __sys_exit

L_net_buffer_too_large:
    ; Buffer size exceeds maximum
    mov rcx, 117
    call __sys_exit
