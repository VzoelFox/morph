; corelib/platform/x86_64/asm_win/scheduler.asm
; Implementasi MorphRoutine Scheduler (Assembly - Windows x64)
; ==============================================================================

%include "corelib/platform/x86_64/asm_win/macros.inc"

global scheduler_init
global scheduler_spawn
global scheduler_yield
global scheduler_get_current
global scheduler_exit_current

extern mem_alloc
extern mem_free
extern stack_new
extern stack_free
extern morph_switch_context
extern __mf_print_str
extern __mf_print_int
extern OS_EXIT
extern __sys_exit

; ------------------------------------------------------------------------------
; KONSTANTA STRUKTUR (Sesuai runtime.fox & structures.fox)
; ------------------------------------------------------------------------------

; MorphRoutine (Size: 64)
%define ROUTINE_OFFSET_RSP 0
%define ROUTINE_OFFSET_RIP 8
%define ROUTINE_OFFSET_STATUS 16
%define ROUTINE_OFFSET_FRAGMENT 24
%define ROUTINE_OFFSET_ID 32
%define ROUTINE_OFFSET_NEXT 40
%define ROUTINE_OFFSET_STACK_BASE 48
%define ROUTINE_OFFSET_STACK_LIMIT 56

%define ROUTINE_SIZE 64
%define STATUS_READY 0
%define STATUS_RUNNING 1
%define STATUS_TERMINATED 3

; MorphScheduler (Size: 32)
%define SCHED_OFFSET_HEAD 0
%define SCHED_OFFSET_TAIL 8
%define SCHED_OFFSET_CURRENT 16
%define SCHED_OFFSET_COUNT 24
%define SCHED_SIZE 32

section .data
    ; Global Scheduler Instance
    global __scheduler_instance
    __scheduler_instance: times 32 db 0

    ; ID Counter for Routines
    __routine_id_counter: dq 1

section .text

; ------------------------------------------------------------------------------
; func scheduler_init()
; Inisialisasi scheduler global.
; Windows Calling Convention: No args
; ------------------------------------------------------------------------------
scheduler_init:
    push rbp
    mov rbp, rsp
    sub rsp, 32             ; Shadow space

    ; Reset scheduler memory
    lea rdi, [rel __scheduler_instance]
    mov qword [rdi + SCHED_OFFSET_HEAD], 0
    mov qword [rdi + SCHED_OFFSET_TAIL], 0
    mov qword [rdi + SCHED_OFFSET_CURRENT], 0
    mov qword [rdi + SCHED_OFFSET_COUNT], 0

    ; Initialize Main Routine (Routine #0)
    ; Kita harus membuat struktur untuk thread utama yang sedang berjalan ini
    ; agar bisa di-switch 'out' nantinya.

    ; 1. Alloc Main Routine Struct
    mov rcx, ROUTINE_SIZE
    call mem_alloc
    ; RAX = New Routine Ptr
    test rax, rax
    jz .init_fail

    ; 2. Setup Main Routine
    mov qword [rax + ROUTINE_OFFSET_ID], 0          ; ID 0
    mov qword [rax + ROUTINE_OFFSET_STATUS], STATUS_RUNNING
    mov qword [rax + ROUTINE_OFFSET_NEXT], 0
    ; Note: RSP/RIP tidak perlu diset sekarang, akan diset oleh switch_context
    ; Stack Base/Limit juga idealnya diset, tapi untuk Main thread agak tricky
    ; tanpa info dari OS. Kita biarkan 0 dulu.

    ; 3. Set as Current
    lea rdi, [rel __scheduler_instance]
    mov [rdi + SCHED_OFFSET_CURRENT], rax

    ; 4. Add to Queue (Head & Tail)
    ; Main routine juga masuk antrian agar bisa dipilih lagi nanti (Round Robin)
    mov [rdi + SCHED_OFFSET_HEAD], rax
    mov [rdi + SCHED_OFFSET_TAIL], rax
    mov qword [rdi + SCHED_OFFSET_COUNT], 1

.init_fail:
    add rsp, 32
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func scheduler_spawn(entry_label_addr: ptr, arg1: i64)
; Membuat routine baru dan menambahkannya ke antrian.
; Windows Input:
;   RCX = Alamat instruksi awal (Entry Point / Trampoline)
;   RDX = Argument pertama (Arg1) yang akan dipop oleh Trampoline.
; ------------------------------------------------------------------------------
scheduler_spawn:
    push rbp
    mov rbp, rsp
    push rbx                 ; Save RBX (Callee-saved)
    push r12                 ; Save R12
    push r13                 ; Save R13 (Arg1)
    push r14                 ; For alignment
    sub rsp, 32              ; Shadow space

    mov r12, rcx             ; Simpan Entry Point di R12
    mov r13, rdx             ; Simpan Arg1 di R13

    ; 1. Allocate Routine Struct
    mov rcx, ROUTINE_SIZE
    call mem_alloc
    test rax, rax
    jz .spawn_fail
    mov rbx, rax             ; RBX = Routine Ptr

    ; 2. Allocate New Stack
    mov rcx, 8192            ; Set Stack Size (8KB)
    call stack_new

    ; SAFETY: Validate stack allocation succeeded
    test rax, rax
    jz .spawn_stack_alloc_failed

    mov [rbx + ROUTINE_OFFSET_STACK_BASE], rax ; SAVE STRUCT PTR at offset 48

    ; SAFETY: Calculate and store stack limits
    mov r8, [rax + 0]        ; R8 = Stack Base (Data Ptr)
    mov rdx, [rax + 8]       ; RDX = Stack Top (Initial RSP)

    ; Store stack limit (base) in routine struct for bounds checking
    mov [rbx + ROUTINE_OFFSET_STACK_LIMIT], r8

    ; --- STACK PREPARATION FOR CONTEXT SWITCH ---
    ; Layout Stack Baru (Top to Bottom):
    ; [ Arg1 (R13) ] <- Akan dipop oleh Trampoline
    ; [ RIP (R12)  ] <- Return Address dari switch_context
    ; [ Registers  ] <- Saved regs

    ; SAFETY: Validate we have enough stack space (need 64 bytes minimum)
    mov r9, rdx
    sub r9, r8               ; R9 = Available stack space
    cmp r9, 64
    jl .spawn_stack_too_small

    ; Push Arg1
    sub rdx, 8
    mov [rdx], r13

    ; Push RIP (Entry Point)
    sub rdx, 8
    mov [rdx], r12

    ; Push Dummy Registers (RBX, RBP, R12-R15) -> 6 * 8 = 48 bytes
    sub rdx, 48
    ; Isi dengan 0 (optional, tidak critical)

    ; Simpan RSP yang sudah disiapkan ke Routine Struct
    mov [rbx + ROUTINE_OFFSET_RSP], rdx

    ; 3. Setup Metadata Routine
    mov qword [rbx + ROUTINE_OFFSET_STATUS], STATUS_READY

    ; Assign ID
    lea rcx, [rel __routine_id_counter]
    mov rax, [rcx]
    mov [rbx + ROUTINE_OFFSET_ID], rax
    inc qword [rcx]          ; Increment global counter

    ; 4. Enqueue (Add to Tail)
    lea rdi, [rel __scheduler_instance]

    ; Cek apakah queue kosong?
    cmp qword [rdi + SCHED_OFFSET_TAIL], 0
    je .spawn_empty_queue

    ; Queue tidak kosong: Tail->Next = New
    mov rax, [rdi + SCHED_OFFSET_TAIL]
    mov [rax + ROUTINE_OFFSET_NEXT], rbx

    ; Update Tail = New
    mov [rdi + SCHED_OFFSET_TAIL], rbx
    jmp .spawn_done

.spawn_empty_queue:
    ; Queue kosong: Head = Tail = New
    mov [rdi + SCHED_OFFSET_HEAD], rbx
    mov [rdi + SCHED_OFFSET_TAIL], rbx

.spawn_done:
    inc qword [rdi + SCHED_OFFSET_COUNT]

.spawn_fail:
    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

.spawn_stack_alloc_failed:
    ; Stack allocation failed - exit with error code
    mov rcx, 113
    call __sys_exit

.spawn_stack_too_small:
    ; Stack size too small for context setup
    mov rcx, 114
    call __sys_exit

; ------------------------------------------------------------------------------
; func scheduler_yield()
; Menyerahkan eksekusi ke routine berikutnya (Round Robin).
; ------------------------------------------------------------------------------
scheduler_yield:
    ; Tidak perlu push RBP standar karena morph_switch_context akan handle full save.

    ; 1. Ambil Current Routine
    lea r8, [rel __scheduler_instance]
    mov rsi, [r8 + SCHED_OFFSET_CURRENT]  ; RSI = Current Routine (Old)

    ; Safety Check: Jika tidak ada current (misal init belum dipanggil), return.
    test rsi, rsi
    jz .yield_ret

    ; 2. Tentukan Next Routine
    ; Strategi: Round Robin.
    ; Current ada di Head? Tidak selalu (jika yield dipanggil berantai).
    ; Namun dalam model antrian sederhana:
    ; Ambil Head. Jika Head == Current, pindahkan ke Tail. Ambil Head baru.

    ; Simplifikasi:
    ; Queue: [A (Running)] -> [B] -> [C]
    ; Yield A:
    ; 1. Set A status = READY
    ; 2. Dequeue A (Head maju ke B)
    ; 3. Enqueue A (Tail jadi A)
    ; 4. Switch to B.

    mov rax, [r8 + SCHED_OFFSET_HEAD]     ; RAX = Head

    ; Jika hanya ada 1 routine, yield tidak melakukan apa-apa.
    cmp qword [rax + ROUTINE_OFFSET_NEXT], 0
    je .yield_ret                          ; Tidak ada next, return langsung.

    ; -- Rotasi Queue --
    ; Old Head (Current)
    mov rcx, rax                           ; RCX = Old Head

    ; New Head
    mov rdx, [rcx + ROUTINE_OFFSET_NEXT]  ; RDX = Next (New Head)
    mov [r8 + SCHED_OFFSET_HEAD], rdx

    ; Move Old Head to Tail
    mov qword [rcx + ROUTINE_OFFSET_NEXT], 0    ; Putus link Old Head

    mov r9, [r8 + SCHED_OFFSET_TAIL]      ; R9 = Current Tail
    mov [r9 + ROUTINE_OFFSET_NEXT], rcx   ; Tail->Next = Old Head
    mov [r8 + SCHED_OFFSET_TAIL], rcx     ; Update Tail Pointer

    ; -- Context Switch --
    ; Old Routine: RSI
    ; New Routine: RDX

    ; Update Current Pointer Global
    mov [r8 + SCHED_OFFSET_CURRENT], rdx

    ; Update Status
    mov qword [rsi + ROUTINE_OFFSET_STATUS], STATUS_READY
    mov qword [rdx + ROUTINE_OFFSET_STATUS], STATUS_RUNNING

    ; Panggil morph_switch_context(old_rsp_ptr, new_rsp)
    ; Windows: RCX, RDX
    ; old_rsp_ptr = Alamat field RSP di struct Old Routine
    ; new_rsp     = Nilai RSP dari struct New Routine

    ; RSI menunjuk ke awal struct Old Routine. Field RSP ada di offset 0.
    ; Jadi RCX (arg1) = RSI.
    mov rcx, rsi

    ; RDX menunjuk ke awal struct New Routine. Field RSP ada di offset 0.
    ; Kita butuh valuenya, bukan alamatnya.
    mov rdx, [rdx + ROUTINE_OFFSET_RSP]   ; RDX (arg2) = [NewRoutine.RSP]

    ; Call Switch (Assembly contract wrapper)
    call morph_switch_context

    ; Kembali dari switch (saat giliran kita lagi)
.yield_ret:
    ret

; ------------------------------------------------------------------------------
; func scheduler_get_current() -> ptr
; Mengembalikan pointer ke routine struct yang sedang berjalan.
; ------------------------------------------------------------------------------
scheduler_get_current:
    lea rdi, [rel __scheduler_instance]
    mov rax, [rdi + SCHED_OFFSET_CURRENT]
    ret

; ------------------------------------------------------------------------------
; func scheduler_exit_current()
; Terminates the current routine and switches to the next one.
; ------------------------------------------------------------------------------
scheduler_exit_current:
    ; 0. Switch to Safe Stack immediately
    lea rsp, [rel __scheduler_exit_stack_top]

    ; Shadow space on new stack
    sub rsp, 40

    ; 1. Get Current Routine
    lea r8, [rel __scheduler_instance]
    mov rsi, [r8 + SCHED_OFFSET_HEAD]  ; RSI = Current Routine

    test rsi, rsi
    jz .Lexit_panic

    ; 2. Remove from List (Dequeue Head)
    mov rax, [rsi + ROUTINE_OFFSET_NEXT] ; RAX = Next
    mov [r8 + SCHED_OFFSET_HEAD], rax

    ; If Head became NULL, Queue is empty -> Tail = NULL
    test rax, rax
    jnz .Lexit_update_count
    mov qword [r8 + SCHED_OFFSET_TAIL], 0

.Lexit_update_count:
    dec qword [r8 + SCHED_OFFSET_COUNT]

    ; 3. Check if Last Routine
    cmp qword [r8 + SCHED_OFFSET_COUNT], 0
    je .Lexit_process_terminate

    ; 4. Free Resources (Old Routine = RSI)
    ; Save Next Routine in Callee-saved register
    push rax ; RAX is Next (New Head)

    ; Free Stack
    mov rcx, [rsi + ROUTINE_OFFSET_STACK_BASE] ; Struct Ptr
    call stack_free

    ; Free Routine Struct
    mov rcx, rsi
    call mem_free

    pop rax ; Restore Next Routine

    ; 5. Switch to Next Routine
    ; Since current is dead, we do NOT save its context.
    ; We just load the new context.

    ; Update Current Global
    lea r8, [rel __scheduler_instance]
    mov [r8 + SCHED_OFFSET_CURRENT], rax
    mov qword [rax + ROUTINE_OFFSET_STATUS], STATUS_RUNNING

    ; Prepare for Switch
    ; morph_switch_context expects (OldRSP_Ptr, NewRSP_Value).
    ; RCX = OldRSP_Ptr, RDX = NewRSP_Value
    lea rcx, [rel __scheduler_scratch_storage]
    mov rdx, [rax + ROUTINE_OFFSET_RSP]

    call morph_switch_context
    ; Never returns
    ret

.Lexit_process_terminate:
    mov rcx, 0
    call OS_EXIT
    ret

.Lexit_panic:
    mov rcx, 1
    call OS_EXIT
    ret

section .bss
    align 16
__scheduler_exit_stack_top_block:
    resb 4096
__scheduler_exit_stack_top:
    resb 8
__scheduler_scratch_storage:
    resb 8
