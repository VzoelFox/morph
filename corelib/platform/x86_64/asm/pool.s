# corelib/platform/x86_64/asm/pool.s
# Implementasi Memory Pool (Linux x86_64)
# Sesuai SSOT corelib/core/memory.fox

.section .text
.global pool_create
.global pool_alloc
.global pool_free

# ------------------------------------------------------------------------------
# func pool_create(obj_size: i64, capacity: i64) -> ptr
# Input:  %rdi = obj_size
#         %rsi = capacity
# Output: %rax = pointer to Pool struct (or 0 if failed)
# ------------------------------------------------------------------------------
pool_create:
    pushq %rbp
    movq %rsp, %rbp

    # Allocate stack for locals
    subq $16, %rsp
    # -8(%rbp): obj_size
    # -16(%rbp): total_size

    # 1. Validasi obj_size >= 8
    cmpq $8, %rdi
    jl .create_fail

    # Simpan obj_size
    movq %rdi, -8(%rbp)

    # 2. Hitung Total Size = (obj_size * capacity) + 48
    movq %rdi, %rax
    mulq %rsi           # rax = obj_size * capacity

    addq $48, %rax
    movq %rax, -16(%rbp) # Simpan total_size

    # 3. Alokasi memori
    movq %rax, %rdi     # rdi = total_size
    call mem_alloc

    testq %rax, %rax
    jz .create_fail

    # %rax = Base Address
    # Init Header (48 bytes)

    # [0x00] Start Ptr = Base + 48
    leaq 48(%rax), %rcx
    movq %rcx, 0(%rax)

    # [0x08] Current Ptr = Start Ptr
    movq %rcx, 8(%rax)

    # [0x10] End Ptr = Base + Total Size
    movq -16(%rbp), %rdx # Restore total_size
    addq %rax, %rdx      # Base + Total Size
    movq %rdx, 16(%rax)

    # [0x18] Object Size
    movq -8(%rbp), %rcx  # Restore obj_size
    movq %rcx, 24(%rax)

    # [0x20] Free List Head = 0 (NULL)
    movq $0, 32(%rax)

    # [0x28] Padding = 0
    movq $0, 40(%rax)

    # Return Base Address
    leave
    ret

.create_fail:
    xorq %rax, %rax
    leave
    ret

# ------------------------------------------------------------------------------
# func pool_alloc(pool: ptr) -> ptr
# Input:  %rdi = pool
# Output: %rax = obj_ptr (or 0 if OOM)
# ------------------------------------------------------------------------------
pool_alloc:
    pushq %rbp
    movq %rsp, %rbp

    # 1. Cek Free List Head [0x20]
    movq 32(%rdi), %rax
    testq %rax, %rax
    jnz .alloc_from_free_list

    # 2. Alloc dari Bump Pointer
    movq 8(%rdi), %rax  # Current
    movq 24(%rdi), %rcx # Obj Size
    movq 16(%rdi), %rdx # End

    # Calc New Current
    movq %rax, %r8      # Old Current (Result)
    addq %rcx, %rax     # New Current

    # Check Bounds
    cmpq %rdx, %rax
    ja .alloc_oom

    # Update Current
    movq %rax, 8(%rdi)

    # Return Old Current
    movq %r8, %rax
    leave
    ret

.alloc_from_free_list:
    # %rax contains current Free List Head (Obj Ptr)
    # Next free block ptr is stored at start of Obj Ptr
    movq (%rax), %rcx   # Next Free Node
    movq %rcx, 32(%rdi) # Update Free List Head

    # Return Obj Ptr (%rax)
    leave
    ret

.alloc_oom:
    xorq %rax, %rax
    leave
    ret

# ------------------------------------------------------------------------------
# func pool_free(pool: ptr, obj_ptr: ptr) -> void
# Input:  %rdi = pool
#         %rsi = obj_ptr
# ------------------------------------------------------------------------------
pool_free:
    pushq %rbp
    movq %rsp, %rbp

    # 1. Ambil Current Free List Head
    movq 32(%rdi), %rax

    # 2. Store Old Head into *obj_ptr
    movq %rax, (%rsi)

    # 3. Update Head = obj_ptr
    movq %rsi, 32(%rdi)

    leave
    ret
