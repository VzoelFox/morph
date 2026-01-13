; macros.inc - Windows x86_64 (NASM Syntax)
; ==============================================================================
; Abstraksi WinAPI untuk Windows
; ==============================================================================

%define MEM_COMMIT  0x1000
%define MEM_RESERVE 0x2000
%define PAGE_READWRITE 0x04
%define STD_OUTPUT_HANDLE -11

extern ExitProcess
extern WriteFile
extern VirtualAlloc
extern GetStdHandle
extern GetSystemTimeAsFileTime
extern CreateDirectoryA
extern CreateFileA
extern CloseHandle
extern GetCurrentProcessId

; Winsock Imports
extern WSAStartup
extern socket
extern connect
extern send
extern recv
extern closesocket
extern getaddrinfo
extern freeaddrinfo

; GDI/User32 Imports
extern RegisterClassA
extern CreateWindowExA
extern DefWindowProcA
extern GetMessageA
extern TranslateMessage
extern DispatchMessageA
extern PostQuitMessage
extern GetDC
extern ReleaseDC
extern SetPixel
extern LoadCursorA
extern PeekMessageA

%define PM_REMOVE 0x0001
%define GENERIC_READ 0x80000000
%define GENERIC_WRITE 0x40000000
%define FILE_SHARE_READ 1
%define OPEN_ALWAYS 4
%define FILE_ATTRIBUTE_NORMAL 0x80

; ------------------------------------------------------------------------------
; Macro: OS_EXIT
; Arg: status_code
; ------------------------------------------------------------------------------
%macro OS_EXIT 1
    mov rcx, %1
    sub rsp, 32         ; Shadow space
    call ExitProcess
    ; No return
%endmacro

; ------------------------------------------------------------------------------
; Macro: OS_WRITE
; Arg: handle, buffer, length
; Input:
;   %1 = Handle (RCX target)
;   %2 = Buffer (RDX target)
;   %3 = Length (R8 target)
; ------------------------------------------------------------------------------
%macro OS_WRITE 3
    sub rsp, 48             ; 32 (shadow) + 8 (arg5) + 8 (alignment/bytesWritten storage)

    ; Move arguments to WinAPI registers
    ; We handle the buffer specially to support both Labels and Registers.
    ; If %2 is a register (e.g., rdx), `lea rdx, [rdx]` is valid (copy).
    ; If %2 is a label (e.g., msg), `lea rdx, [msg]` loads the address.

    mov r8, %3              ; Length -> R8
    lea rdx, [%2]           ; Buffer -> RDX
    mov rcx, %1             ; Handle -> RCX

    lea r9, [rsp + 40]      ; Storage for bytes written
    mov qword [rsp + 32], 0 ; lpOverlapped = NULL

    call WriteFile
    add rsp, 48
%endmacro

; ------------------------------------------------------------------------------
; Macro: OS_ALLOC_PAGE
; Arg: size (register or immediate)
; Output: RAX (Pointer)
; ------------------------------------------------------------------------------
%macro OS_ALLOC_PAGE 1
    ; VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect)
    ; RCX = 0 (lpAddress)
    ; RDX = Size
    ; R8  = MEM_COMMIT | MEM_RESERVE
    ; R9  = PAGE_READWRITE

    sub rsp, 32         ; Shadow space

    xor rcx, rcx                ; lpAddress = NULL
    mov rdx, %1                 ; dwSize = Size
    mov r8, MEM_COMMIT | MEM_RESERVE
    mov r9, PAGE_READWRITE

    call VirtualAlloc

    add rsp, 32
%endmacro
