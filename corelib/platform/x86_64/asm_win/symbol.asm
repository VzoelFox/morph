; corelib/platform/x86_64/asm_win/symbol.asm
; Implementasi Symbol Table (Hash Map) untuk Windows x86_64
; Menggunakan Chaining untuk resolusi collision.

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .text
global sym_table_create
global sym_table_put
global sym_table_get
global sym_table_get_by_hash
global sym_table_put_by_hash
extern mem_alloc
extern __mf_string_hash
extern __mf_string_equals

; ------------------------------------------------------------------------------
; func sym_table_create(capacity: i64) -> ptr
; Input:  RCX = capacity
; Output: RAX = ptr
; ------------------------------------------------------------------------------
sym_table_create:
    push rbp
    mov rbp, rsp
    push rdi
    sub rsp, 32     ; Shadow Space

    ; 1. Size = Cap * 8
    mov rax, rcx
    shl rax, 3

    ; 2. Alloc
    push rcx        ; Save capacity
    mov rcx, rax
    call mem_alloc
    pop rcx

    test rax, rax
    jz .create_fail

    ; 3. Zeroing
    mov rdx, rax    ; Current Ptr
    mov r8, rcx     ; Count

.zero_loop:
    mov qword [rdx], 0
    add rdx, 8
    dec r8
    jnz .zero_loop

    ; Return RAX (already holds base ptr)
    add rsp, 32
    pop rdi
    pop rbp
    ret

.create_fail:
    xor rax, rax
    add rsp, 32
    pop rdi
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func sym_table_put(table, key_ptr, key_len, value, capacity)
; Input: RCX, RDX, R8, R9. Stack: [rsp+48] = capacity
; Safe: Compares String content.
; ------------------------------------------------------------------------------
sym_table_put:
    push rbp
    mov rbp, rsp
    push rbx
    push rdi        ; SAVE RDI (Non-Volatile)
    push r12
    push r13
    push r14
    push r15
    sub rsp, 48     ; Shadow + alignment

    mov r12, rcx    ; Table
    mov r13, rdx    ; Key Ptr
    mov r14, r8     ; Key Len
    mov r15, r9     ; Value
    mov rbx, [rbp + 48] ; Capacity

    ; 1. Hash
    mov rcx, r13
    mov rdx, r14
    call __mf_string_hash
    mov rdi, rax    ; Simpan Hash

    ; 2. Index
    xor rdx, rdx
    div rbx         ; rax / capacity

    ; 3. Bucket Address
    lea rbx, [r12 + rdx*8]

    ; 4. Find Loop
    mov r10, [rbx]

.find_loop:
    test r10, r10
    jz .add_new

    ; Compare Hash First (Optimization)
    cmp [r10 + 8], rdi
    jne .next

    ; Compare String
    mov r11, [r10 + 16] ; String Struct
    mov rcx, [r11 + 8]  ; Data
    mov rdx, [r11 + 0]  ; Len

    mov r8, r13         ; Input Data
    mov r9, r14         ; Input Len

    push r10
    push rbx
    sub rsp, 32     ; Shadow internal call
    call __mf_string_equals
    add rsp, 32
    pop rbx
    pop r10

    cmp rax, 1
    jne .next

    ; Found
    mov [r10 + 24], r15
    jmp .done

.next:
    mov r10, [r10]
    jmp .find_loop

.add_new:
    ; Alloc Node (32 bytes)
    mov rcx, 32
    sub rsp, 32
    call mem_alloc
    add rsp, 32

    ; Setup Node
    mov r8, [rbx]
    mov [rax + 0], r8   ; Next
    mov [rax + 8], rdi  ; Hash

    push rax            ; Save Node

    ; Alloc String Struct (16 bytes)
    mov rcx, 16
    sub rsp, 32
    call mem_alloc
    add rsp, 32

    mov [rax + 0], r14  ; Len
    mov [rax + 8], r13  ; Data

    mov r11, rax        ; Struct Ptr
    pop rax             ; Restore Node

    mov [rax + 16], r11 ; Store Struct Ptr
    mov [rax + 24], r15 ; Value

    ; Link
    mov [rbx], rax

.done:
    add rsp, 48
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi         ; RESTORE RDI
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func sym_table_put_by_hash(table, hash, value, capacity)
; Input: RCX, RDX, R8, R9
; Unsafe: Stores value solely based on Hash. If collision, overwrites first match
; or appends. For RPN portability where we lose string data.
; ------------------------------------------------------------------------------
sym_table_put_by_hash:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 32

    mov r12, rcx ; Table
    mov r13, rdx ; Hash
    mov r14, r8  ; Value
    mov rbx, r9  ; Capacity

    ; Index
    mov rax, r13
    xor rdx, rdx
    div rbx

    lea r10, [r12 + rdx*8] ; Bucket
    mov r11, [r10] ; First Node

.ph_loop:
    test r11, r11
    jz .ph_add_new

    cmp [r11 + 8], r13 ; Compare Hash
    je .ph_update

    mov r11, [r11]
    jmp .ph_loop

.ph_update:
    mov [r11 + 24], r14
    jmp .ph_done

.ph_add_new:
    ; Alloc Node
    mov rcx, 32
    sub rsp, 32
    call mem_alloc
    add rsp, 32

    mov r8, [r10]
    mov [rax + 0], r8 ; Next
    mov [rax + 8], r13 ; Hash
    mov qword [rax + 16], 0 ; NO STRING DATA (NULL)
    mov [rax + 24], r14 ; Value

    mov [r10], rax

.ph_done:
    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func sym_table_get(table, key_ptr, key_len, capacity)
; Input: RCX, RDX, R8, R9
; Safe: Compares String content.
; ------------------------------------------------------------------------------
sym_table_get:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 32

    mov r12, rcx
    mov r13, rdx
    mov r14, r8
    mov rbx, r9

    ; Hash
    mov rcx, r13
    mov rdx, r14
    call __mf_string_hash
    mov r15, rax ; Hash

    xor rdx, rdx
    div rbx

    lea r10, [r12 + rdx*8]
    mov r10, [r10]

.loop:
    test r10, r10
    jz .not_found

    ; Hash Check Optimization
    cmp [r10 + 8], r15
    jne .next

    ; String Check
    mov r11, [r10 + 16]
    test r11, r11
    jz .next ; Skip node without string data (created by put_by_hash?)

    mov rcx, [r11 + 8]
    mov rdx, [r11 + 0]
    mov r8, r13
    mov r9, r14

    push r10
    push r15
    sub rsp, 32
    call __mf_string_equals
    add rsp, 32
    pop r15
    pop r10

    cmp rax, 1
    jne .next

    mov rax, [r10 + 24]
    jmp .exit

.next:
    mov r10, [r10]
    jmp .loop

.not_found:
    mov rax, -1

.exit:
    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func sym_table_get_by_hash(table, hash, capacity)
; Input: RCX, RDX, R8
; Unsafe: Returns value of first node matching hash.
; ------------------------------------------------------------------------------
sym_table_get_by_hash:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 32

    mov r12, rcx ; Table
    mov r13, rdx ; Hash
    mov rbx, r8  ; Capacity

    mov rax, r13
    xor rdx, rdx
    div rbx

    lea r10, [r12 + rdx*8]
    mov r10, [r10]

.gh_loop:
    test r10, r10
    jz .gh_not_found

    cmp [r10 + 8], r13
    je .gh_found

    mov r10, [r10]
    jmp .gh_loop

.gh_found:
    mov rax, [r10 + 24]
    jmp .gh_done

.gh_not_found:
    mov rax, -1

.gh_done:
    add rsp, 32
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
