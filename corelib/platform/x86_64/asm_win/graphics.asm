; corelib/platform/x86_64/asm_win/graphics.asm
; Implementasi Grafis Native (Windows x86_64)
; Wrapper GDI32 / User32

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    class_name db "MorphFoxWindowClass", 0
    window_title db "MorphFox Window", 0
    wnd_class_registered db 0

    ; WNDCLASSA Structure (approx)
    ; UINT style; WNDPROC lpfnWndProc; int cbClsExtra; int cbWndExtra;
    ; HINSTANCE hInstance; HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground;
    ; LPCSTR lpszMenuName; LPCSTR lpszClassName;
    wnd_class resb 80 ; 10 * 8 bytes

section .bss
    msg_struct resb 48 ; MSG struct (HWND, UINT, WPARAM, LPARAM, TIME, POINT)
    global_hwnd resq 1 ; Global HWND storage

section .text
global __mf_window_create
global __mf_draw_pixel
global __mf_window_poll
global __mf_draw_char
global __mf_draw_string
global __mf_fill_rect
global __mf_draw_rect
global WindowProc ; Export for Windows Callback

extern __mf_font_bitmap

; ------------------------------------------------------------------------------
; WindowProc (Callback from Windows)
; Input: RCX=hwnd, RDX=uMsg, R8=wParam, R9=lParam
; ------------------------------------------------------------------------------
WindowProc:
    ; Standard Prologue
    push rbp
    mov rbp, rsp
    sub rsp, 32

    ; Check uMsg (RDX)
    cmp rdx, 0x0002 ; WM_DESTROY
    je .handle_destroy

    cmp rdx, 0x0010 ; WM_CLOSE
    je .handle_destroy

    ; Default Handler
    ; DefWindowProcA(hwnd, uMsg, wParam, lParam) -> Args match exactly
    call DefWindowProcA
    jmp .proc_ret

.handle_destroy:
    ; PostQuitMessage(0)
    xor rcx, rcx
    call PostQuitMessage
    xor rax, rax
    jmp .proc_ret

.proc_ret:
    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_window_create(width: i64, height: i64, title: ptr) -> hwnd
; Input: RCX, RDX, R8
; ------------------------------------------------------------------------------
__mf_window_create:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 80 ; Shadow + Locals

    mov r12, rcx ; Width
    mov r13, rdx ; Height
    mov r14, r8  ; Title

    ; 1. Register Class (Once)
    cmp byte [rel wnd_class_registered], 1
    je .create_window

    ; Setup WNDCLASSA
    lea rbx, [rel wnd_class]
    mov dword [rbx], 3 ; CS_HREDRAW | CS_VREDRAW
    lea rax, [rel WindowProc]
    mov [rbx + 8], rax ; lpfnWndProc
    mov dword [rbx + 16], 0 ; cbClsExtra (4 bytes)
    mov dword [rbx + 20], 0 ; cbWndExtra (4 bytes)
    mov qword [rbx + 24], 0 ; hInstance (GetModuleHandle(0)?)

    ; LoadCursor(NULL, IDC_ARROW)
    xor rcx, rcx
    mov rdx, 32512 ; IDC_ARROW
    call LoadCursorA
    mov [rbx + 40], rax ; hCursor (Offset 40, not 48)

    mov qword [rbx + 48], 5 ; hbrBackground (COLOR_WINDOW)
    mov qword [rbx + 56], 0 ; MenuName
    lea rax, [rel class_name]
    mov [rbx + 64], rax ; ClassName (Offset 64, not 72)

    ; RegisterClassA(&wc)
    mov rcx, rbx
    call RegisterClassA
    mov byte [rel wnd_class_registered], 1

.create_window:
    ; CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle,
    ;                 x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)

    ; Stack Args (Right to Left):
    mov qword [rsp + 88], 0 ; lpParam
    mov qword [rsp + 80], 0 ; hInstance
    mov qword [rsp + 72], 0 ; hMenu
    mov qword [rsp + 64], 0 ; Parent
    mov [rsp + 56], r13     ; Height
    mov [rsp + 48], r12     ; Width
    mov qword [rsp + 40], 0 ; Y (CW_USEDEFAULT)
    mov qword [rsp + 32], 0 ; X (CW_USEDEFAULT)

    mov r9, 0x10CF0000      ; WS_VISIBLE | WS_OVERLAPPEDWINDOW
    mov r8, r14             ; Title
    lea rdx, [rel class_name]
    xor rcx, rcx            ; ExStyle

    call CreateWindowExA

    ; RAX = HWND
    mov [rel global_hwnd], rax ; Save global HWND
    add rsp, 80
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_draw_pixel(hwnd: i64, x: i64, y: i64, color: i64)
; Input: RCX, RDX, R8, R9
; ------------------------------------------------------------------------------
__mf_draw_pixel:
    push rbp
    mov rbp, rsp
    push rbx
    push r12 ; HWND
    push r13 ; X
    push r14 ; Y
    push r15 ; Color
    sub rsp, 32

    mov r12, rcx
    mov r13, rdx
    mov r14, r8
    mov r15, r9

    ; 1. GetDC(hwnd)
    mov rcx, r12
    call GetDC
    mov rbx, rax ; HDC

    test rbx, rbx
    jz .draw_done

    ; 2. SetPixel(hdc, x, y, color)
    mov rcx, rbx
    mov rdx, r13
    mov r8, r14
    mov r9, r15
    call SetPixel

    ; 3. ReleaseDC(hwnd, hdc)
    mov rcx, r12
    mov rdx, rbx
    call ReleaseDC

.draw_done:
    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_window_poll(event_ptr: ptr) -> event_id
; Output: RAX (0=None, 1=Quit, 3=Mouse, 4=Click)
; ------------------------------------------------------------------------------
__mf_window_poll:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    sub rsp, 48

    mov r12, rcx ; Save event_ptr

    ; PeekMessageA(LPMSG, HWND, UINT, UINT, UINT)
    mov qword [rsp + 32], PM_REMOVE ; wRemoveMsg
    xor r9, r9 ; wMsgFilterMax = 0
    xor r8, r8 ; wMsgFilterMin = 0
    xor rdx, rdx ; HWND = NULL
    lea rcx, [rel msg_struct]

    call PeekMessageA

    test rax, rax ; 0 = No message
    jz .poll_none

    lea rbx, [rel msg_struct]
    mov eax, [rbx + 8] ; Message ID (Offset 8)

    cmp eax, 0x0012 ; WM_QUIT
    je .poll_quit

    cmp eax, 0x0201 ; WM_LBUTTONDOWN
    je .poll_click

    cmp eax, 0x0200 ; WM_MOUSEMOVE
    je .poll_mouse

    ; TranslateMessage(&msg)
    lea rcx, [rel msg_struct]
    call TranslateMessage

    ; DispatchMessage(&msg)
    lea rcx, [rel msg_struct]
    call DispatchMessageA

    mov rax, 0 ; Event processed, not returned
    jmp .poll_ret

.poll_quit:
    mov rax, 1 ; EVENT_QUIT
    jmp .poll_ret

.poll_click:
    test r12, r12
    jz .poll_click_ret

    mov qword [r12 + 0], 4 ; Type = CLICK

    ; LPARAM (Offset 32) contains X (Low) and Y (High)
    mov rax, [rbx + 32]
    mov rdx, rax
    and rax, 0xFFFF ; Low word = X
    shr rdx, 16     ; High word = Y

    mov [r12 + 16], rax ; X
    mov [r12 + 24], rdx ; Y

.poll_click_ret:
    mov rax, 4
    jmp .poll_ret

.poll_mouse:
    test r12, r12
    jz .poll_mouse_ret

    mov qword [r12 + 0], 3 ; Type = MOUSE

    mov rax, [rbx + 32]
    mov rdx, rax
    and rax, 0xFFFF ; Low word = X
    shr rdx, 16     ; High word = Y

    mov [r12 + 16], rax ; X
    mov [r12 + 24], rdx ; Y

.poll_mouse_ret:
    mov rax, 3
    jmp .poll_ret

.poll_none:
    mov rax, 0

.poll_ret:
    add rsp, 48
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_fill_rect(x: i64, y: i64, w: i64, h: i64, color: i64)
; ------------------------------------------------------------------------------
__mf_fill_rect:
    push rbp
    mov rbp, rsp
    push rbx
    push r12 ; X
    push r13 ; Y
    push r14 ; W
    push r15 ; H
    ; Need more registers? We need Color.
    ; Fastcall: RCX, RDX, R8, R9. 5th arg on stack [rbp + 48]
    sub rsp, 72 ; Shadow(32) + Locals(24) + Align

    mov r12, rcx
    mov r13, rdx
    mov r14, r8
    mov r15, r9

    mov rax, [rbp + 48] ; 5th Arg (Color)
    mov [rsp + 64], rax ; Store Color in local slot

    ; Loop Y
    xor r8, r8 ; Counter Y
.fill_loop_y:
    cmp r8, r15
    jge .fill_done

    ; Loop X
    xor r9, r9 ; Counter X
.fill_loop_x:
    cmp r9, r14
    jge .fill_next_y

    ; Draw Pixel (StartX + CtrX, StartY + CtrY)
    mov [rsp + 32], r8 ; Save Y Loop
    mov [rsp + 40], r9 ; Save X Loop

    mov rcx, [rel global_hwnd]

    mov rdx, r12
    add rdx, r9 ; X

    mov r8, r13
    add r8, [rsp + 32] ; Y (Reload r8 from stack because used for arg)

    mov r9, [rsp + 64] ; Color

    call __mf_draw_pixel

    mov r8, [rsp + 32] ; Restore Y Loop
    mov r9, [rsp + 40] ; Restore X Loop

    inc r9
    jmp .fill_loop_x

.fill_next_y:
    inc r8
    jmp .fill_loop_y

.fill_done:
    add rsp, 72
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_draw_rect(x: i64, y: i64, w: i64, h: i64, color: i64)
; ------------------------------------------------------------------------------
__mf_draw_rect:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 72

    mov r12, rcx ; X
    mov r13, rdx ; Y
    mov r14, r8  ; W
    mov r15, r9  ; H
    mov rax, [rbp + 48] ; Color
    mov rbx, rax ; Store Color

    ; Top Line (x, y, w, 1)
    mov rcx, r12
    mov rdx, r13
    mov r8, r14
    mov r9, 1
    mov [rsp + 32], rbx ; 5th arg
    call __mf_fill_rect

    ; Bottom Line (x, y+h-1, w, 1)
    mov rcx, r12
    mov rdx, r13
    add rdx, r15
    dec rdx
    mov r8, r14
    mov r9, 1
    mov [rsp + 32], rbx
    call __mf_fill_rect

    ; Left Line (x, y, 1, h)
    mov rcx, r12
    mov rdx, r13
    mov r8, 1
    mov r9, r15
    mov [rsp + 32], rbx
    call __mf_fill_rect

    ; Right Line (x+w-1, y, 1, h)
    mov rcx, r12
    add rcx, r14
    dec rcx
    mov rdx, r13
    mov r8, 1
    mov r9, r15
    mov [rsp + 32], rbx
    call __mf_fill_rect

    add rsp, 72
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_draw_char(x: i64, y: i64, char: i64, color: i64)
; Input: RCX, RDX, R8, R9
; ------------------------------------------------------------------------------
__mf_draw_char:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 72 ; Alloc 72 bytes (Align 16: 5 pushes=40, +72=112, 112%16=0)

    mov r12, rcx ; X
    mov r13, rdx ; Y
    mov r14, r8  ; Char
    mov r15, r9  ; Color

    ; Get Bitmap Address
    lea rbx, [rel __mf_font_bitmap]
    imul r14, 8
    add rbx, r14

    ; Loop Rows (0-7)
    xor r8, r8

.char_row_loop:
    cmp r8, 8
    ge .char_done

    mov al, [rbx + r8]

    ; Loop Bits (0-7)
    mov r9, 7

.char_bit_loop:
    cmp r9, -1
    je .char_row_next

    bt rax, r9
    jnc .char_bit_skip

    ; Draw Pixel at (X + (7-Bit), Y + Row)
    ; Save volatile registers (rax, r8, r9) before call to __mf_draw_pixel
    ; Stack layout: [rsp+0..31] = shadow space, [rsp+32..71] = local storage
    mov [rsp + 32], rax ; Save Bitmap Row
    mov [rsp + 40], r8  ; Save Row Index
    mov [rsp + 48], r9  ; Save Bit Index

    mov rcx, [rel global_hwnd] ; Handle

    mov rdx, r12    ; X
    mov r10, 7
    sub r10, r9
    add rdx, r10    ; X + (7 - Bit)

    mov r8, r13     ; Y
    add r8, [rsp + 40] ; Y + Row

    mov r9, r15     ; Color

    call __mf_draw_pixel

    mov rax, [rsp + 32] ; Restore
    mov r8,  [rsp + 40] ; Restore
    mov r9,  [rsp + 48] ; Restore

.char_bit_skip:
    dec r9
    jmp .char_bit_loop

.char_row_next:
    inc r8
    jmp .char_row_loop

.char_done:
    add rsp, 72
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func __mf_draw_string(x: i64, y: i64, str_ptr: ptr, color: i64)
; ------------------------------------------------------------------------------
__mf_draw_string:
    push rbp
    mov rbp, rsp
    push r12
    push r13
    push r14
    push r15
    sub rsp, 32

    mov r12, rcx ; X
    mov r13, rdx ; Y
    mov r14, r8  ; Str Ptr
    mov r15, r9  ; Color

.str_loop:
    movzx rdx, byte [r14]
    test rdx, rdx
    jz .str_done

    ; Draw Char(x, y, char, color)
    mov rcx, r12
    mov rdx, r13
    mov r8, [r14]
    and r8, 0xFF ; Char
    mov r9, r15
    call __mf_draw_char

    add r12, 8
    inc r14
    jmp .str_loop

.str_done:
    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    ret
