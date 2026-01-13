# corelib/platform/x86_64/asm/test_graphics.s
# Test Program: Native Window & Pixel Drawing (Linux X11)
# "The First Pixel"

.include "corelib/platform/x86_64/asm/syscalls.inc"

.section .data
    title_str: .asciz "MorphFox Native Graphics (The First Pixel)"

.section .text
.global main
.extern __mf_window_create
.extern __mf_draw_pixel
.extern __mf_window_poll
.extern __mf_runtime_init
.extern __sys_exit
.extern __mf_print_asciz

main:
    call __mf_runtime_init

    # 1. Create Window (800x600)
    movq $800, %rdi
    movq $600, %rsi
    leaq title_str(%rip), %rdx
    call __mf_window_create
    movq %rax, %r12 # Handle

    cmpq $0, %r12
    js .fail

.loop:
    # 2. Poll Events
    call __mf_window_poll
    cmpq $1, %rax # EVENT_QUIT
    je .done

    # 3. Draw Pixel (Center, Red)
    # X=400, Y=300, Color=0x00FF0000 (Red)
    # X11 Colors are usually RGB. 0x00FF0000 is Red.

    movq %r12, %rdi
    movq $400, %rsi
    movq $300, %rdx
    movq $0x00FF0000, %rcx
    call __mf_draw_pixel

    # Draw Cross
    movq %r12, %rdi
    movq $399, %rsi
    movq $300, %rdx
    movq $0x00FF0000, %rcx
    call __mf_draw_pixel
    movq %r12, %rdi
    movq $401, %rsi
    movq $300, %rdx
    movq $0x00FF0000, %rcx
    call __mf_draw_pixel

    movq %r12, %rdi
    movq $400, %rsi
    movq $299, %rdx
    movq $0x00FF0000, %rcx
    call __mf_draw_pixel
    movq %r12, %rdi
    movq $400, %rsi
    movq $301, %rdx
    movq $0x00FF0000, %rcx
    call __mf_draw_pixel

    # Small delay or yield?
    # For now tight loop is fine for test.
    jmp .loop

.done:
    movq $0, %rdi
    call __sys_exit

.fail:
    movq $1, %rdi
    call __sys_exit
