; corelib/platform/x86_64/asm_win/control_flow.asm
; Implementasi Control Flow Helpers (Windows NASM)

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .text
global ctx_switch_new
global ctx_switch_add_case
global ctx_switch_free
extern mem_alloc
extern vector_new
extern vector_push

; ------------------------------------------------------------------------------
; func ctx_switch_new(type: i64) -> ptr
; ------------------------------------------------------------------------------
ctx_switch_new:
    push rbp
    mov rbp, rsp
    push rbx
    mov rbx, rcx        ; Type (Arg1 in RCX)

    ; Alloc Context (32 bytes)
    mov rcx, 32
    sub rsp, 32
    call mem_alloc
    add rsp, 32

    test rax, rax
    jz .new_fail

    ; Init Fields
    mov qword [rax + 8], -1  ; Default
    mov qword [rax + 16], 0  ; End
    mov [rax + 24], rbx      ; Type

    push rax            ; Save Ctx

    ; Alloc Vector
    mov rcx, 16         ; ItemSize 16
    sub rsp, 32
    call vector_new
    add rsp, 32

    pop rcx             ; Restore Ctx
    mov [rcx + 0], rax  ; Vector Ptr
    mov rax, rcx        ; Return Ctx

    pop rbx
    leave
    ret

.new_fail:
    pop rbx
    leave
    ret

; ------------------------------------------------------------------------------
; func ctx_switch_add_case(ctx: ptr, val: i64, label: i64)
; Input: RCX, RDX, R8
; ------------------------------------------------------------------------------
ctx_switch_add_case:
    push rbp
    mov rbp, rsp
    sub rsp, 16         ; Struct CasePair

    mov [rsp + 0], rdx  ; Value
    mov [rsp + 8], r8   ; Label

    ; Vector Push(vec, item_ptr)
    ; Arg1 RCX = ctx->cases
    mov rcx, [rcx]
    ; Arg2 RDX = Stack Ptr
    lea rdx, [rsp + 0]

    sub rsp, 32         ; Shadow
    call vector_push
    add rsp, 32

    leave
    ret

; ------------------------------------------------------------------------------
ctx_switch_free:
    ret

global ctx_loop_new
global ctx_stack_push
global ctx_stack_pop

; ------------------------------------------------------------------------------
; func ctx_loop_new(start: i64, end: i64) -> ptr
; ------------------------------------------------------------------------------
ctx_loop_new:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13

    mov r12, rcx ; Start
    mov r13, rdx ; End

    mov rcx, 16
    sub rsp, 32
    call mem_alloc
    add rsp, 32
    test rax, rax
    jz .loop_fail

    mov [rax], r12
    mov [rax+8], r13

    pop r13
    pop r12
    pop rbx
    leave
    ret

.loop_fail:
    xor rax, rax
    pop r13
    pop r12
    pop rbx
    leave
    ret

; ------------------------------------------------------------------------------
; func ctx_stack_push(stack_top: ptr, ctx: ptr, type: i64) -> ptr
; ------------------------------------------------------------------------------
ctx_stack_push:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14

    mov r12, rcx ; Old Top
    mov r13, rdx ; Ctx
    mov r14, r8  ; Type

    mov rcx, 24
    sub rsp, 32
    call mem_alloc
    add rsp, 32
    test rax, rax
    jz .push_fail

    mov [rax], r12
    mov [rax+8], r13
    mov [rax+16], r14

    pop r14
    pop r13
    pop r12
    pop rbx
    leave
    ret

.push_fail:
    xor rax, rax
    pop r14
    pop r13
    pop r12
    pop rbx
    leave
    ret

; ------------------------------------------------------------------------------
; func ctx_stack_pop(stack_top: ptr) -> ptr
; ------------------------------------------------------------------------------
ctx_stack_pop:
    test rcx, rcx
    jz .pop_null
    mov rax, [rcx]
    ret
.pop_null:
    xor rax, rax
    ret
