; corelib/platform/x86_64/asm_win/test_dom.asm
; Test Program: Manual DOM Construction (Structure Verification)
; Creates: Document -> DIV -> P

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    tag_div db "div", 0
    tag_p   db "p", 0
    msg_ok  db "[DOM] Structure Verified: Root -> Div -> P", 10, 0
    msg_err db "[DOM] Verification Failed!", 10, 0

; Offsets (Match SSOT)
DOM_OFF_TYPE   equ 0
DOM_OFF_PARENT equ 8
DOM_OFF_CHILD  equ 16
DOM_OFF_NEXT   equ 24
DOM_OFF_TAG    equ 32
DOM_OFF_ATTR   equ 40

section .text
global main
extern __mf_runtime_init
extern mem_alloc
extern __mf_print_asciz
extern OS_EXIT

main:
    push rbp
    mov rbp, rsp
    sub rsp, 48

    call __mf_runtime_init

    ; 1. Alloc Root (Document)
    mov rcx, 64
    call mem_alloc
    mov r12, rax ; R12 = Root
    mov qword [r12 + DOM_OFF_TYPE], 1 ; DOC

    ; 2. Alloc Element (DIV)
    mov rcx, 64
    call mem_alloc
    mov r13, rax ; R13 = DIV
    mov qword [r13 + DOM_OFF_TYPE], 2 ; ELEMENT
    lea rcx, [rel tag_div]
    mov [r13 + DOM_OFF_TAG], rcx

    ; Link Root -> DIV
    mov [r12 + DOM_OFF_CHILD], r13
    mov [r13 + DOM_OFF_PARENT], r12

    ; 3. Alloc Element (P)
    mov rcx, 64
    call mem_alloc
    mov r14, rax ; R14 = P
    mov qword [r14 + DOM_OFF_TYPE], 2 ; ELEMENT
    lea rcx, [rel tag_p]
    mov [r14 + DOM_OFF_TAG], rcx

    ; Link DIV -> P
    mov [r13 + DOM_OFF_CHILD], r14
    mov [r14 + DOM_OFF_PARENT], r13

    ; 4. Verify Structure
    ; Root->Child == DIV ?
    mov rax, [r12 + DOM_OFF_CHILD]
    cmp rax, r13
    jne .fail

    ; DIV->Parent == Root ?
    mov rax, [r13 + DOM_OFF_PARENT]
    cmp rax, r12
    jne .fail

    ; DIV->Child == P ?
    mov rax, [r13 + DOM_OFF_CHILD]
    cmp rax, r14
    jne .fail

    ; P->Parent == DIV ?
    mov rax, [r14 + DOM_OFF_PARENT]
    cmp rax, r13
    jne .fail

    lea rcx, [rel msg_ok]
    call __mf_print_asciz

    mov rcx, 0
    call OS_EXIT

.fail:
    lea rcx, [rel msg_err]
    call __mf_print_asciz
    mov rcx, 1
    call OS_EXIT
