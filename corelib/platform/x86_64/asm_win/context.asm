; corelib/platform/x86_64/asm_win/context.asm
; Implementasi Context Switching untuk MorphRoutine (Windows x64)
; ==============================================================================

section .text
global morph_switch_context

; ------------------------------------------------------------------------------
; func morph_switch_context(old_rsp_ptr: ptr, new_rsp: ptr)
; Input:
;   RCX = old_rsp_ptr (Pointer ke lokasi memori untuk menyimpan RSP lama)
;   RDX = new_rsp     (Nilai RSP baru yang akan dimuat)
;
; Tugas:
; 1. Push semua Callee-Saved Register ke Stack LAMA.
; 2. Simpan nilai RSP (setelah push) ke [RCX].
; 3. Ganti RSP dengan RDX (Stack BARU).
; 4. Pop semua Callee-Saved Register dari Stack BARU.
; 5. Return (ke alamat yang ada di Stack BARU).
; ------------------------------------------------------------------------------
morph_switch_context:
    ; 1. Save Callee-Saved Registers (Windows x64 ABI)
    ; Non-Volatile: RBX, RBP, RDI, RSI, R12, R13, R14, R15, XMM6-XMM15
    ; Kita simpan General Purpose Registers dulu.
    ; (XMM disimpan hanya jika fitur float diaktifkan nanti, untuk sekarang basic GPR)

    push rbx
    push rbp
    push rdi
    push rsi
    push r12
    push r13
    push r14
    push r15

    ; 2. Store Old RSP
    mov [rcx], rsp

    ; 3. Load New RSP
    mov rsp, rdx

    ; 4. Restore Callee-Saved Registers (Order: Reverse of Push)
    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbp
    pop rbx

    ; 5. Return (Pop RIP from new stack)
    ret
