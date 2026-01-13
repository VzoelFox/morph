; corelib/platform/x86_64/asm_win/string.asm
; Implementasi Operasi String (Windows x86_64)
; Sesuai SSOT corelib/core/structures.fox

section .text
global __mf_string_hash
global __mf_string_equals

; ------------------------------------------------------------------------------
; func __mf_string_hash(ptr: ptr, len: i64) -> i64
; Algoritma: FNV-1a 64-bit
; Input:  RCX = ptr, RDX = len
; Output: RAX = hash
; ------------------------------------------------------------------------------
__mf_string_hash:
    ; FNV_OFFSET_BASIS_64 = 0xcbf29ce484222325
    ; FNV_PRIME_64        = 0x100000001b3

    mov rax, 0xcbf29ce484222325 ; Init hash
    mov r8, 0x100000001b3       ; Prime

    test rdx, rdx
    jz .hash_done

.hash_loop:
    ; XOR hash with byte
    movzx r9, byte [rcx]
    xor rax, r9

    ; Multiply by prime
    imul rax, r8

    ; Next char
    inc rcx
    dec rdx
    jnz .hash_loop

.hash_done:
    ret

; ------------------------------------------------------------------------------
; func __mf_string_equals(ptr1: ptr, len1: i64, ptr2: ptr, len2: i64) -> bool
; Input:  RCX = ptr1, RDX = len1, R8 = ptr2, R9 = len2
; Output: RAX = 1 (True) or 0 (False)
; ------------------------------------------------------------------------------
__mf_string_equals:
    ; 1. Cek Panjang (RDX vs R9)
    cmp rdx, r9
    jne .not_equal

    ; 2. Cek Panjang 0
    test rdx, rdx
    jz .is_equal

    ; 3. Loop Compare
    ; RCX = ptr1, R8 = ptr2, RDX = count

    push rcx
    push rdx
    push r8

.cmp_loop:
    mov al, byte [rcx]
    cmp byte [r8], al
    jne .cmp_fail

    inc rcx
    inc r8
    dec rdx
    jnz .cmp_loop

    ; Equal
    pop r8
    pop rdx
    pop rcx

.is_equal:
    mov rax, 1
    ret

.cmp_fail:
    pop r8
    pop rdx
    pop rcx

.not_equal:
    xor rax, rax
    ret
