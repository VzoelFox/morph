; corelib/platform/x86_64/asm_win/stack.asm
; MorphStack: Virtual Stack Implementation
; ==============================================================================
; Sesuai SSOT runtime.fox:
; [Offset 0x00] Stack Pointer (ptr) - Current Top
; [Offset 0x08] Stack Base (ptr)    - Bottom (High Address)
; [Offset 0x10] Stack Limit (ptr)   - Top Limit (Low Address)

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .text
global stack_new
global stack_push
global stack_pop
extern mem_alloc

; ------------------------------------------------------------------------------
; func stack_new(size_bytes: i64) -> stack_struct_ptr
; Allocates a stack structure AND the raw stack memory.
; ------------------------------------------------------------------------------
stack_new:
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 32

    mov rbx, rcx ; Size

    ; 1. Alloc Stack Struct (24 bytes needed, align to 32)
    mov rcx, 32
    call mem_alloc
    test rax, rax
    jz .fail
    mov rdx, rax ; Struct Ptr

    ; 2. Alloc Raw Stack Memory
    mov rcx, rbx
    push rdx
    sub rsp, 32
    call mem_alloc
    add rsp, 32
    pop rdx

    test rax, rax
    jz .fail

    ; Setup Struct
    ; Stack Grows Downwards: Base is Highest Address
    lea r8, [rax + rbx] ; Base = Start + Size

    mov [rdx + 0], r8   ; SP starts at Base
    mov [rdx + 8], r8   ; Base
    mov [rdx + 16], rax ; Limit (Start Address)

    mov rax, rdx ; Return Struct Ptr
    jmp .done

.fail:
    xor rax, rax

.done:
    add rsp, 32
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func stack_push(stack_ptr: ptr, value: i64) -> success(1)/fail(0)
; ------------------------------------------------------------------------------
stack_push:
    ; RCX = Stack Ptr
    ; RDX = Value

    mov rax, [rcx + 0]  ; Current SP
    mov r8,  [rcx + 16] ; Limit

    ; Check Overflow (SP - 8 < Limit)
    lea r9, [rax - 8]
    cmp r9, r8
    jl .overflow

    ; Push
    mov [r9], rdx
    mov [rcx + 0], r9   ; Update SP

    mov rax, 1 ; Success
    ret

.overflow:
    xor rax, rax ; Fail
    ret

; ------------------------------------------------------------------------------
; func stack_pop(stack_ptr: ptr) -> value (RAX), status (RDX)
; ------------------------------------------------------------------------------
stack_pop:
    ; RCX = Stack Ptr

    mov r8, [rcx + 0] ; Current SP
    mov r9, [rcx + 8] ; Base

    ; Check Underflow (SP >= Base)
    cmp r8, r9
    jge .underflow

    ; Pop
    mov rax, [r8]     ; Get Value
    add r8, 8
    mov [rcx + 0], r8 ; Update SP

    mov rdx, 1 ; Success status
    ret

.underflow:
    xor rax, rax
    xor rdx, rdx ; Fail status
    ret

; ------------------------------------------------------------------------------
; func stack_free(stack_ptr: ptr) -> void
; ------------------------------------------------------------------------------
global stack_free
extern mem_free

stack_free:
    ; RCX = Stack Ptr
    push rbp
    mov rbp, rsp
    push rbx
    sub rsp, 32

    mov rbx, rcx ; Save Struct Ptr

    ; 1. Free Buffer (Limit points to start of buffer)
    mov rcx, [rbx + 16]
    call mem_free

    ; 2. Free Struct
    mov rcx, rbx
    call mem_free

    add rsp, 32
    pop rbx
    pop rbp
    ret
