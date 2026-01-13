# bootstrap/asm/arena.s
# Implementasi Memory Arena (Linux x86_64)
# Sesuai SSOT corelib/core/memory.fox

.section .text
.global arena_create
.global arena_alloc
.global arena_reset
.global arena_get_usage
.global arena_get_capacity

# ------------------------------------------------------------------------------
# func arena_create(size: i64) -> ptr
# Input:  %rdi = size (total bytes requested)
# Output: %rax = pointer to Arena struct (or 0 if failed)
# ------------------------------------------------------------------------------
arena_create:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12          # Callee-saved
    pushq %r13          # Callee-saved

    # 1. Simpan size asli
    movq %rdi, %r12

    # 2. Alokasi memori mentah via mem_alloc
    # Panggil: mem_alloc(size)
    call mem_alloc

    # Cek error
    testq %rax, %rax
    jz .create_fail

    # %rax adalah Base Address dari blok memori
    movq %rax, %r13

    # 3. Inisialisasi Header Arena (32 bytes)
    # [0x00] Start Ptr = Base + 32
    leaq 32(%r13), %rcx
    movq %rcx, 0(%r13)

    # [0x08] Current Ptr = Start Ptr
    movq %rcx, 8(%r13)

    # [0x10] End Ptr = Base + Size
    movq %r13, %rdx
    addq %r12, %rdx
    movq %rdx, 16(%r13)

    # [0x18] ID (Default 0, bisa diset user nanti)
    movq $0, 24(%r13)

    # Return pointer ke Arena Struct (Base)
    movq %r13, %rax

    popq %r13
    popq %r12
    leave
    ret

.create_fail:
    xorq %rax, %rax
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func arena_alloc(arena: ptr, size: i64) -> ptr
# Input:  %rdi = arena pointer
#         %rsi = size
# Output: %rax = allocated pointer (or 0 if OOM)
# ------------------------------------------------------------------------------
arena_alloc:
    pushq %rbp
    movq %rsp, %rbp

    # 1. Ambil Current Ptr & End Ptr
    movq 8(%rdi), %rax  # Current
    movq 16(%rdi), %rdx # End

    # 2. Hitung New Current (Current + Size)
    movq %rax, %rcx     # rcx = Old Current (Result)
    addq %rsi, %rax     # rax = New Current

    # 3. Cek Bounds (New Current <= End)
    cmpq %rdx, %rax
    jg .alloc_oom

    # 4. Update Current Ptr
    movq %rax, 8(%rdi)

    # 5. Return Old Current
    movq %rcx, %rax

    leave
    ret

.alloc_oom:
    xorq %rax, %rax
    leave
    ret

# ------------------------------------------------------------------------------
# func arena_reset(arena: ptr) -> void
# Input:  %rdi = arena pointer
# ------------------------------------------------------------------------------
arena_reset:
    pushq %rbp
    movq %rsp, %rbp

    # Set Current = Start
    movq 0(%rdi), %rax
    movq %rax, 8(%rdi)

    leave
    ret

# ------------------------------------------------------------------------------
# func arena_get_usage(arena: ptr) -> i64
# Input:  %rdi = arena pointer
# Output: %rax = used bytes
# ------------------------------------------------------------------------------
arena_get_usage:
    # Usage = Current - Start
    movq 8(%rdi), %rax # Current
    subq 0(%rdi), %rax # Start
    ret

# ------------------------------------------------------------------------------
# func arena_get_capacity(arena: ptr) -> i64
# Input:  %rdi = arena pointer
# Output: %rax = total capacity bytes
# ------------------------------------------------------------------------------
arena_get_capacity:
    # Capacity = End - Start
    movq 16(%rdi), %rax # End
    subq 0(%rdi), %rax  # Start
    ret
