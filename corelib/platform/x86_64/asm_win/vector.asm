; corelib/platform/x86_64/asm_win/vector.asm
; Implementasi MorphVector (Dynamic Array) - Windows x86_64

section .text
global vector_new
global vector_push
global vector_get

extern mem_alloc
extern mem_free
extern __mf_memcpy

; Struktur Vector (32 bytes)
; [0] Buffer Ptr
; [8] Length
; [16] Capacity
; [24] Item Size

; ------------------------------------------------------------------------------
; func vector_new(item_size: i64) -> ptr
; Input: RCX = item_size
; Output: RAX = pointer struct
; ------------------------------------------------------------------------------
vector_new:
    push rbp
    mov rbp, rsp
    push rbx            ; Save non-volatile
    sub rsp, 32         ; Shadow Space for call mem_alloc

    mov rbx, rcx        ; Simpan item_size

    ; Alloc struct (32 bytes)
    mov rcx, 32
    call mem_alloc
    test rax, rax
    jz .vnew_fail

    ; Init
    mov qword [rax + 0], 0  ; buffer
    mov qword [rax + 8], 0  ; len
    mov qword [rax + 16], 0 ; cap
    mov qword [rax + 24], rbx ; item_size

    add rsp, 32
    pop rbx
    leave
    ret

.vnew_fail:
    xor rax, rax
    add rsp, 32
    pop rbx
    leave
    ret

; ------------------------------------------------------------------------------
; func vector_push(vec: ptr, item_ptr: ptr) -> i64
; Input: RCX = vec, RDX = item_ptr
; ------------------------------------------------------------------------------
vector_push:
    push rbp
    mov rbp, rsp

    ; Save Non-Volatile Regs
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15

    ; Align stack if necessary?
    ; Push 7 regs * 8 = 56 bytes. RBP+Ret = 16. Total pushed = 72.
    ; Stack misaligned by 8?
    ; Call needs 16-byte align.
    ; Sub shadow space 32. 72+32 = 104. 104 is div by 8, not 16.
    ; Need extra 8 bytes padding.
    sub rsp, 40         ; 32 Shadow + 8 Align

    mov r12, rcx        ; Vec Ptr
    mov r13, rdx        ; Item Ptr

    ; 1. Check Cap
    mov rax, [r12 + 8]  ; Length
    mov rcx, [r12 + 16] ; Cap

    cmp rax, rcx
    jl .vpush_insert

    ; 2. Resize
    mov r14, 8          ; Default 8
    test rcx, rcx
    cmovnz r14, rcx     ; If cap!=0, use cap
    test rcx, rcx
    jz .calc_size
    add r14, rcx        ; Double it

.calc_size:
    mov rax, [r12 + 24] ; ItemSize
    imul rax, r14       ; Total Bytes

    ; Alloc
    mov rcx, rax
    call mem_alloc
    test rax, rax
    jz .vpush_fail

    mov r15, rax        ; New Buf

    ; Copy Old
    mov rsi, [r12 + 0]  ; Old Buf
    test rsi, rsi
    jz .update_vec

    ; Memcpy(dest, src, size)
    mov rcx, r15        ; Dest
    mov rdx, rsi        ; Src
    mov rax, [r12 + 8]  ; Len
    imul rax, [r12 + 24]; Bytes
    mov r8, rax         ; Size
    call __mf_memcpy

    ; Free Old Buffer
    mov rcx, [r12 + 0]  ; Ptr
    call mem_free

.update_vec:
    mov [r12 + 0], r15
    mov [r12 + 16], r14

.vpush_insert:
    ; 3. Insert
    mov rcx, [r12 + 0]  ; Buf Base
    mov rax, [r12 + 8]  ; Len
    imul rax, [r12 + 24]; Offset
    add rcx, rax        ; Dest Ptr

    mov rdx, r13        ; Src (Item Ptr)
    mov r8, [r12 + 24]  ; Size
    call __mf_memcpy

    ; 4. Inc Len
    inc qword [r12 + 8]

    xor rax, rax        ; Success
    jmp .vpush_end

.vpush_fail:
    mov rax, -1

.vpush_end:
    add rsp, 40
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    leave
    ret

; ------------------------------------------------------------------------------
; func vector_get(vec: ptr, index: i64) -> ptr
; Input: RCX = vec, RDX = index
; Output: RAX = ptr
; ------------------------------------------------------------------------------
vector_get:
    mov rax, [rcx + 8]  ; Len
    cmp rdx, rax
    jge .vget_fail
    test rdx, rdx
    js .vget_fail

    mov rax, [rcx + 24] ; ItemSize
    imul rax, rdx       ; Offset
    add rax, [rcx + 0]  ; Base + Offset
    ret

.vget_fail:
    xor rax, rax
    ret
