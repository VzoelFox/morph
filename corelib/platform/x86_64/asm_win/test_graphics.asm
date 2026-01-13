; corelib/platform/x86_64/asm_win/test_graphics.asm
; Test Program: Native Window & Pixel Drawing
; "The First Pixel"

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    title_str db "MorphFox Native Graphics (The First Pixel)", 0

section .text
global main
extern __mf_window_create
extern __mf_draw_pixel
extern __mf_window_poll
extern __mf_runtime_init
extern OS_EXIT

main:
    push rbp
    mov rbp, rsp
    sub rsp, 48

    call __mf_runtime_init

    ; 1. Create Window (800x600)
    mov rcx, 800
    mov rdx, 600
    lea r8, [rel title_str]
    call __mf_window_create
    mov r12, rax ; HWND

    test r12, r12
    jz .fail

.loop:
    ; 2. Poll Events
    call __mf_window_poll
    cmp rax, 1 ; EVENT_QUIT
    je .done

    ; 3. Draw Pixel (Center, Red)
    ; X=400, Y=300, Color=0x00FF0000 (Red in 0x00BBGGRR format usually, but SetPixel is COLORREF 0x00bbggrr)
    ; Red is 0x000000FF. Wait, COLORREF is 0x00bbggrr. So Red is 0x000000FF.
    ; SSOT says: COLOR_RED 0x00FF0000. That's ARGB or RGBA?
    ; GDI uses 0x00bbggrr. Red = 0x000000FF.
    ; Let's try drawing Red (0x000000FF)

    mov rcx, r12
    mov rdx, 400
    mov r8, 300
    mov r9, 0x000000FF
    call __mf_draw_pixel

    ; Draw some more to be visible (a small cross)
    ; Horizontal
    mov rcx, r12
    mov rdx, 399
    mov r8, 300
    mov r9, 0x000000FF
    call __mf_draw_pixel
    mov rcx, r12
    mov rdx, 401
    mov r8, 300
    mov r9, 0x000000FF
    call __mf_draw_pixel
    ; Vertical
    mov rcx, r12
    mov rdx, 400
    mov r8, 299
    mov r9, 0x000000FF
    call __mf_draw_pixel
    mov rcx, r12
    mov rdx, 400
    mov r8, 301
    mov r9, 0x000000FF
    call __mf_draw_pixel

    jmp .loop

.done:
    mov rcx, 0
    call OS_EXIT

.fail:
    mov rcx, 1
    call OS_EXIT
