# corelib/platform/x86_64/asm/scheduler.s
# Implementasi MorphRoutine Scheduler (Assembly - Linux x64)
# ==============================================================================

.global scheduler_init
.global scheduler_spawn
.global scheduler_yield
.global scheduler_get_current
.global scheduler_exit_current

.extern mem_alloc
.extern mem_free
.extern stack_new
.extern stack_free
.extern morph_switch_context
.extern __mf_print_str
.extern __mf_print_int
.extern __sys_exit

# ------------------------------------------------------------------------------
# KONSTANTA STRUKTUR (Sesuai runtime.fox & structures.fox)
# ------------------------------------------------------------------------------

# MorphRoutine (Size: 64)
.equ ROUTINE_OFFSET_RSP,       0
.equ ROUTINE_OFFSET_RIP,       8
.equ ROUTINE_OFFSET_STATUS,    16
.equ ROUTINE_OFFSET_FRAGMENT,  24
.equ ROUTINE_OFFSET_ID,        32
.equ ROUTINE_OFFSET_NEXT,      40
.equ ROUTINE_OFFSET_STACK_BASE, 48
.equ ROUTINE_OFFSET_STACK_LIMIT, 56

.equ ROUTINE_SIZE, 64
.equ STATUS_READY, 0
.equ STATUS_RUNNING, 1
.equ STATUS_TERMINATED, 3

# MorphScheduler (Size: 32)
.equ SCHED_OFFSET_HEAD,    0
.equ SCHED_OFFSET_TAIL,    8
.equ SCHED_OFFSET_CURRENT, 16
.equ SCHED_OFFSET_COUNT,   24
.equ SCHED_SIZE, 32

.data
    # Global Scheduler Instance
    .global __scheduler_instance
    __scheduler_instance: .zero 32

    # ID Counter for Routines
    __routine_id_counter: .quad 1

.text

# ------------------------------------------------------------------------------
# func scheduler_init()
# Inisialisasi scheduler global.
# ------------------------------------------------------------------------------
scheduler_init:
    pushq %rbp
    movq %rsp, %rbp

    # Reset scheduler memory
    leaq __scheduler_instance(%rip), %rdi
    movq $0, SCHED_OFFSET_HEAD(%rdi)
    movq $0, SCHED_OFFSET_TAIL(%rdi)
    movq $0, SCHED_OFFSET_CURRENT(%rdi)
    movq $0, SCHED_OFFSET_COUNT(%rdi)

    # Initialize Main Routine (Routine #0)
    # Kita harus membuat struktur untuk thread utama yang sedang berjalan ini
    # agar bisa di-switch 'out' nantinya.

    # 1. Alloc Main Routine Struct
    movq $ROUTINE_SIZE, %rdi
    call mem_alloc
    # RAX = New Routine Ptr

    # 2. Setup Main Routine
    movq $0, ROUTINE_OFFSET_ID(%rax)          # ID 0
    movq $STATUS_RUNNING, ROUTINE_OFFSET_STATUS(%rax)
    movq $0, ROUTINE_OFFSET_NEXT(%rax)
    # Note: RSP/RIP tidak perlu diset sekarang, akan diset oleh switch_context
    # Stack Base/Limit juga idealnya diset, tapi untuk Main thread agak tricky
    # tanpa info dari OS. Kita biarkan 0 dulu.

    # 3. Set as Current
    leaq __scheduler_instance(%rip), %rdi
    movq %rax, SCHED_OFFSET_CURRENT(%rdi)

    # 4. Add to Queue (Head & Tail)
    # Main routine juga masuk antrian agar bisa dipilih lagi nanti (Round Robin)
    movq %rax, SCHED_OFFSET_HEAD(%rdi)
    movq %rax, SCHED_OFFSET_TAIL(%rdi)
    movq $1, SCHED_OFFSET_COUNT(%rdi)

    popq %rbp
    ret

# ------------------------------------------------------------------------------
# func scheduler_spawn(entry_label_addr: ptr, arg1: i64)
# Membuat routine baru dan menambahkannya ke antrian.
# Input:
#   RDI = Alamat instruksi awal (Entry Point / Trampoline)
#   RSI = Argument pertama (Arg1) yang akan dipop oleh Trampoline.
# ------------------------------------------------------------------------------
scheduler_spawn:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx                 # Save RBX (Callee-saved)
    pushq %r12                 # Save R12
    pushq %r13                 # Save R13 (Arg1)

    movq %rdi, %r12            # Simpan Entry Point di R12
    movq %rsi, %r13            # Simpan Arg1 di R13

    # 1. Allocate Routine Struct
    movq $ROUTINE_SIZE, %rdi
    call mem_alloc
    movq %rax, %rbx            # RBX = Routine Ptr

    # 2. Allocate New Stack
    movq $8192, %rdi           # Set Stack Size (8KB)
    call stack_new

    # SAFETY: Validate stack allocation succeeded
    testq %rax, %rax
    jz .Lspawn_stack_alloc_failed

    movq %rax, ROUTINE_OFFSET_STACK_BASE(%rbx) # SAVE STRUCT PTR at offset 48

    # SAFETY: Calculate and store stack limits
    movq 0(%rax), %r8          # R8 = Stack Base (Data Ptr)
    movq 8(%rax), %rdx         # RDX = Stack Top (Initial RSP)

    # Store stack limit (base) in routine struct for bounds checking
    movq %r8, ROUTINE_OFFSET_STACK_LIMIT(%rbx)

    # --- STACK PREPARATION FOR CONTEXT SWITCH ---
    # Layout Stack Baru (Top to Bottom):
    # [ Arg1 (R13) ] <- Akan dipop oleh Trampoline
    # [ RIP (R12)  ] <- Return Address dari switch_context
    # [ Registers  ] <- Saved regs

    # SAFETY: Validate we have enough stack space (need 64 bytes minimum)
    movq %rdx, %r9
    subq %r8, %r9              # R9 = Available stack space
    cmpq $64, %r9
    jl .Lspawn_stack_too_small

    # Push Arg1
    subq $8, %rdx
    movq %r13, (%rdx)

    # Push RIP (Entry Point)
    subq $8, %rdx
    movq %r12, (%rdx)

    # Push Dummy Registers (RBX, RBP, R12-R15) -> 6 * 8 = 48 bytes
    subq $48, %rdx
    # Isi dengan 0

    # Simpan RSP yang sudah disiapkan ke Routine Struct
    movq %rdx, ROUTINE_OFFSET_RSP(%rbx)

    # 3. Setup Metadata Routine
    movq $STATUS_READY, ROUTINE_OFFSET_STATUS(%rbx)

    # Assign ID
    leaq __routine_id_counter(%rip), %rcx
    movq (%rcx), %rax
    movq %rax, ROUTINE_OFFSET_ID(%rbx)
    incq (%rcx)                # Increment global counter

    # 4. Enqueue (Add to Tail)
    leaq __scheduler_instance(%rip), %rdi

    # Cek apakah queue kosong?
    cmpq $0, SCHED_OFFSET_TAIL(%rdi)
    je .Lspawn_empty_queue

    # Queue tidak kosong: Tail->Next = New
    movq SCHED_OFFSET_TAIL(%rdi), %rax
    movq %rbx, ROUTINE_OFFSET_NEXT(%rax)

    # Update Tail = New
    movq %rbx, SCHED_OFFSET_TAIL(%rdi)
    jmp .Lspawn_done

.Lspawn_empty_queue:
    # Queue kosong: Head = Tail = New
    movq %rbx, SCHED_OFFSET_HEAD(%rdi)
    movq %rbx, SCHED_OFFSET_TAIL(%rdi)

.Lspawn_done:
    incq SCHED_OFFSET_COUNT(%rdi)

    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret

.Lspawn_stack_alloc_failed:
    # Stack allocation failed - exit with error code
    movq $113, %rdi
    call __sys_exit

.Lspawn_stack_too_small:
    # Stack size too small for context setup
    movq $114, %rdi
    call __sys_exit

# ------------------------------------------------------------------------------
# func scheduler_yield()
# Menyerahkan eksekusi ke routine berikutnya (Round Robin).
# ------------------------------------------------------------------------------
scheduler_yield:
    # Tidak perlu push RBP standar karena morph_switch_context akan handle full save.

    # 1. Ambil Current Routine
    leaq __scheduler_instance(%rip), %r8
    movq SCHED_OFFSET_CURRENT(%r8), %rsi  # RSI = Current Routine (Old)

    # Safety Check: Jika tidak ada current (misal init belum dipanggil), return.
    testq %rsi, %rsi
    jz .Lyield_ret

    # 2. Tentukan Next Routine
    # Strategi: Round Robin.
    # Current ada di Head? Tidak selalu (jika yield dipanggil berantai).
    # Namun dalam model antrian sederhana:
    # Ambil Head. Jika Head == Current, pindahkan ke Tail. Ambil Head baru.

    # Simplifikasi:
    # Queue: [A (Running)] -> [B] -> [C]
    # Yield A:
    # 1. Set A status = READY
    # 2. Dequeue A (Head maju ke B)
    # 3. Enqueue A (Tail jadi A)
    # 4. Switch to B.

    movq SCHED_OFFSET_HEAD(%r8), %rax     # RAX = Head

    # Jika hanya ada 1 routine, yield tidak melakukan apa-apa.
    cmpq $0, ROUTINE_OFFSET_NEXT(%rax)
    je .Lyield_ret                        # Tidak ada next, return langsung.

    # -- Rotasi Queue --
    # Old Head (Current)
    movq %rax, %rcx                       # RCX = Old Head

    # New Head
    movq ROUTINE_OFFSET_NEXT(%rcx), %rdx  # RDX = Next (New Head)
    movq %rdx, SCHED_OFFSET_HEAD(%r8)

    # Move Old Head to Tail
    movq $0, ROUTINE_OFFSET_NEXT(%rcx)    # Putus link Old Head

    movq SCHED_OFFSET_TAIL(%r8), %r9      # R9 = Current Tail
    movq %rcx, ROUTINE_OFFSET_NEXT(%r9)   # Tail->Next = Old Head
    movq %rcx, SCHED_OFFSET_TAIL(%r8)     # Update Tail Pointer

    # -- Context Switch --
    # Old Routine: RSI
    # New Routine: RDX

    # Update Current Pointer Global
    movq %rdx, SCHED_OFFSET_CURRENT(%r8)

    # Update Status
    movq $STATUS_READY, ROUTINE_OFFSET_STATUS(%rsi)
    movq $STATUS_RUNNING, ROUTINE_OFFSET_STATUS(%rdx)

    # Panggil morph_switch_context(old_rsp_ptr, new_rsp)
    # old_rsp_ptr = Alamat field RSP di struct Old Routine
    # new_rsp     = Nilai RSP dari struct New Routine

    # RSI menunjuk ke awal struct Old Routine. Field RSP ada di offset 0.
    # Jadi RDI (arg1) = RSI.
    movq %rsi, %rdi

    # RDX menunjuk ke awal struct New Routine. Field RSP ada di offset 0.
    # Kita butuh valuenya, bukan alamatnya.
    movq ROUTINE_OFFSET_RSP(%rdx), %rsi   # RSI (arg2) = [NewRoutine.RSP]

    # Call Switch (Assembly contract wrapper)
    call morph_switch_context

    # Kembali dari switch (saat giliran kita lagi)
.Lyield_ret:
    ret

# ------------------------------------------------------------------------------
# func scheduler_get_current() -> ptr
# Mengembalikan pointer ke routine struct yang sedang berjalan.
# ------------------------------------------------------------------------------
scheduler_get_current:
    leaq __scheduler_instance(%rip), %rdi
    movq SCHED_OFFSET_CURRENT(%rdi), %rax
    ret

# ------------------------------------------------------------------------------
# func scheduler_exit_current()
# Terminates the current routine and switches to the next one.
# ------------------------------------------------------------------------------
scheduler_exit_current:
    # 1. Get Current Routine (Should be Head in RR)
    leaq __scheduler_instance(%rip), %r8
    movq SCHED_OFFSET_HEAD(%r8), %rsi  # RSI = Current Routine

    testq %rsi, %rsi
    jz .Lexit_panic # Should not happen

    # 2. Remove from List (Dequeue Head)
    movq ROUTINE_OFFSET_NEXT(%rsi), %rax # RAX = Next
    movq %rax, SCHED_OFFSET_HEAD(%r8)

    # If Head became NULL, Queue is empty -> Tail = NULL
    testq %rax, %rax
    jnz .Lexit_update_count
    movq $0, SCHED_OFFSET_TAIL(%r8)

.Lexit_update_count:
    decq SCHED_OFFSET_COUNT(%r8)

    # 3. Check if Last Routine
    cmpq $0, SCHED_OFFSET_COUNT(%r8)
    je .Lexit_process_terminate

    # 4. Free Resources (Old Routine = RSI)
    # Save Next Routine in Callee-saved register before calling free
    pushq %rax # RAX is Next (New Head)

    # Free Stack
    movq ROUTINE_OFFSET_STACK_BASE(%rsi), %rdi # Offset 48 = Struct Ptr
    call stack_free

    # Free Routine Struct
    movq %rsi, %rdi
    call mem_free

    popq %rax # Restore Next Routine

    # 5. Switch to Next Routine
    # Since current is dead, we do NOT save its context.
    # We just load the new context.

    # Update Current Global
    movq %rax, SCHED_OFFSET_CURRENT(%r8)
    movq $STATUS_RUNNING, ROUTINE_OFFSET_STATUS(%rax)

    # Load New RSP
    movq ROUTINE_OFFSET_RSP(%rax), %rsi   # RSI = New RSP

    # Call SwitchContext with Dummy Save Location
    # We use a scratch space on our current stack (which is about to be discarded)
    # Or a global scratch. But current stack is fine because we haven't unmapped it?
    # Wait, we just called stack_free!
    # stack_free -> mem_free -> unmaps if page empty?
    # Yes, mem_free might unmap if it was the only block in page.
    # If we are running ON the stack we just freed, we are in trouble.

    # CRITICAL: We are executing on the stack we just freed.
    # We must switch stack BEFORE freeing?
    # Or use a temporary system stack?

    # Solution: We can't free the stack we are running on.
    # We need a "Zombie" state? Or "Garbage Collector"?
    # Or, simpler: Switch to Next Routine, and have Next Routine free the previous one?
    # Complexity explosion.

    # Alternative: Switch to a temporary "Scheduler Stack" before freeing?
    # Global scheduler stack.
    # 1. Switch RSP to __scheduler_stack.
    # 2. Free Old Stack.
    # 3. Switch to New Routine.

    # Let's implement a small static stack for this operation.
    leaq __scheduler_exit_stack_top(%rip), %rsp

    # Now we are safe to free the old stack.
    # We need to have saved RSI (Old Routine) and RAX (New Routine) somewhere.
    # But registers are preserved across RSP switch if we don't touch stack.
    # Valid.

    # Refetch pointers because we might have lost them? No, registers are fine.
    # RSI was Old Routine. RAX was New Routine.

    # Free Stack
    movq ROUTINE_OFFSET_STACK_BASE(%rsi), %rdi
    # Save RAX (New Routine) on the NEW stack (Scheduler Stack)
    pushq %rax
    # Save RSI (Old Routine) just in case (for mem_free routine)
    pushq %rsi

    call stack_free

    popq %rdi # Restore Old Routine to RDI for mem_free
    call mem_free

    popq %rax # Restore New Routine

    # Update Current Global (again, strictly safe)
    leaq __scheduler_instance(%rip), %r8
    movq %rax, SCHED_OFFSET_CURRENT(%r8)
    movq $STATUS_RUNNING, ROUTINE_OFFSET_STATUS(%rax)

    # Prepare for Switch
    # morph_switch_context expects (OldRSP_Ptr, NewRSP_Value).
    # We don't care about saving the current context (Scheduler Stack).
    # We can pass a dummy pointer for OldRSP.
    leaq __scheduler_scratch_storage(%rip), %rdi
    movq ROUTINE_OFFSET_RSP(%rax), %rsi

    call morph_switch_context
    # Never returns
    ret

.Lexit_process_terminate:
    movq $0, %rdi
    call __sys_exit
    ret

.Lexit_panic:
    movq $1, %rdi
    call __sys_exit
    ret

.section .bss
    .align 16
    .skip 4096
__scheduler_exit_stack_top:
    .skip 8
__scheduler_scratch_storage:
    .skip 8
