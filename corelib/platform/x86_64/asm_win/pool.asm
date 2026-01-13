; corelib/platform/x86_64/asm_win/pool.asm
; Implementasi Memory Pool (Windows x86_64)
; Sesuai SSOT corelib/core/memory.fox

section .text
global pool_create
global pool_alloc
global pool_free
extern mem_alloc

; ------------------------------------------------------------------------------
; func pool_create(obj_size: i64, capacity: i64) -> ptr
; Input:  RCX = obj_size
;         RDX = capacity
; Output: RAX = pointer to Pool struct
; ------------------------------------------------------------------------------
pool_create:
    push rbp
    mov rbp, rsp

    ; Stack Frame:
    ; [rbp] = Old RBP
    ; [rbp-8] = obj_size
    ; [rbp-16] = total_size
    ; RSP need to be aligned 16 bytes before CALL.
    ; Pushed RBP -> Stack 16-byte aligned (if caller aligned).
    ; We need 32 bytes shadow space + 16 bytes locals = 48 bytes.

    sub rsp, 48

    ; 1. Validasi obj_size >= 8
    cmp rcx, 8
    jl .create_fail

    ; Save obj_size
    mov [rbp - 8], rcx

    ; 2. Calc Total Size = (obj_size * capacity) + 48
    mov rax, rcx
    mul rdx             ; RDX:RAX = obj_size * capacity

    add rax, 48
    mov [rbp - 16], rax ; Save total_size

    ; 3. Call mem_alloc
    mov rcx, rax        ; arg1 = total_size
    call mem_alloc

    test rax, rax
    jz .create_fail

    ; Init Header
    ; RAX = Base Address

    ; [0x00] Start = Base + 48
    lea rdx, [rax + 48]
    mov [rax + 0], rdx

    ; [0x08] Current = Start
    mov [rax + 8], rdx

    ; [0x10] End = Base + Total Size
    mov r8, [rbp - 16]   ; Restore total_size
    add r8, rax
    mov [rax + 16], r8

    ; [0x18] Obj Size
    mov rcx, [rbp - 8]   ; Restore obj_size
    mov [rax + 24], rcx

    ; [0x20] Free List Head = 0
    mov qword [rax + 32], 0

    ; [0x28] Padding = 0
    mov qword [rax + 40], 0

    ; RAX already Base
    leave
    ret

.create_fail:
    xor rax, rax
    leave
    ret

; ------------------------------------------------------------------------------
; func pool_alloc(pool: ptr) -> ptr
; Input:  RCX = pool
; Output: RAX = obj_ptr
; ------------------------------------------------------------------------------
pool_alloc:
    push rbp
    mov rbp, rsp

    ; 1. Check Free List [0x20]
    mov rax, [rcx + 32]
    test rax, rax
    jnz .alloc_from_free_list

    ; 2. Bump Pointer
    mov rax, [rcx + 8]   ; Current
    mov r8, [rcx + 24]   ; Obj Size
    mov r9, [rcx + 16]   ; End

    ; Calc New Current
    mov r10, rax         ; Old Current
    add rax, r8

    ; Check Bounds
    cmp rax, r9
    jg .alloc_oom

    ; Update Current
    mov [rcx + 8], rax

    ; Return Old Current
    mov rax, r10
    leave
    ret

.alloc_from_free_list:
    ; RAX is Free Head
    mov rdx, [rax]       ; Next Free
    mov [rcx + 32], rdx  ; Update Head
    leave
    ret

.alloc_oom:
    xor rax, rax
    leave
    ret

; ------------------------------------------------------------------------------
; func pool_free(pool: ptr, obj_ptr: ptr) -> void
; Input:  RCX = pool
;         RDX = obj_ptr
; ------------------------------------------------------------------------------
pool_free:
    push rbp
    mov rbp, rsp

    ; 1. Get Old Head
    mov rax, [rcx + 32]

    ; 2. Store Old Head into *obj_ptr
    mov [rdx], rax

    ; 3. Update Head = obj_ptr
    mov [rcx + 32], rdx

    leave
    ret
