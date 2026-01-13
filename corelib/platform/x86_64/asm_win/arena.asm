; corelib/platform/x86_64/asm_win/arena.asm
; Implementasi Memory Arena (Windows x86_64)
; Sesuai SSOT corelib/core/memory.fox

section .text
global arena_create
global arena_alloc
global arena_reset
global arena_get_usage
global arena_get_capacity
extern mem_alloc

; ------------------------------------------------------------------------------
; func arena_create(size: i64) -> ptr
; Input:  RCX = size
; Output: RAX = pointer to Arena struct
; ------------------------------------------------------------------------------
arena_create:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    sub rsp, 32         ; Shadow Space for call mem_alloc

    ; 1. Save size
    mov rbx, rcx

    ; 2. Call mem_alloc(size)
    ; RCX is already size
    call mem_alloc

    test rax, rax
    jz .create_fail

    ; RAX is Base Address
    mov r12, rax

    ; 3. Init Header
    ; [0x00] Start = Base + 32
    lea rdx, [r12 + 32]
    mov [r12 + 0], rdx

    ; [0x08] Current = Start
    mov [r12 + 8], rdx

    ; [0x10] End = Base + Size
    mov r8, r12
    add r8, rbx
    mov [r12 + 16], r8

    ; [0x18] ID = 0
    mov qword [r12 + 24], 0

    mov rax, r12

    add rsp, 32
    pop r12
    pop rbx
    leave
    ret

.create_fail:
    xor rax, rax
    add rsp, 32
    pop r12
    pop rbx
    leave
    ret

; ------------------------------------------------------------------------------
; func arena_alloc(arena: ptr, size: i64) -> ptr
; Input:  RCX = arena
;         RDX = size
; Output: RAX = ptr
; ------------------------------------------------------------------------------
arena_alloc:
    push rbp
    mov rbp, rsp

    ; 1. Load State
    mov rax, [rcx + 8]  ; Current
    mov r8, [rcx + 16]  ; End

    ; 2. Calc New Current
    mov r9, rax         ; Old Current (Result)
    add rax, rdx        ; New Current

    ; 3. Check Bounds
    cmp rax, r8
    jg .alloc_oom

    ; 4. Update
    mov [rcx + 8], rax

    ; 5. Return Old Current
    mov rax, r9

    leave
    ret

.alloc_oom:
    xor rax, rax
    leave
    ret

; ------------------------------------------------------------------------------
; func arena_reset(arena: ptr) -> void
; Input: RCX = arena
; ------------------------------------------------------------------------------
arena_reset:
    push rbp
    mov rbp, rsp

    mov rax, [rcx + 0] ; Start
    mov [rcx + 8], rax ; Current = Start

    leave
    ret

; ------------------------------------------------------------------------------
; func arena_get_usage(arena: ptr) -> i64
; Input: RCX = arena
; Output: RAX = usage
; ------------------------------------------------------------------------------
arena_get_usage:
    mov rax, [rcx + 8] ; Current
    sub rax, [rcx + 0] ; Start
    ret

; ------------------------------------------------------------------------------
; func arena_get_capacity(arena: ptr) -> i64
; Input: RCX = arena
; Output: RAX = capacity
; ------------------------------------------------------------------------------
arena_get_capacity:
    mov rax, [rcx + 16] ; End
    sub rax, [rcx + 0]  ; Start
    ret
