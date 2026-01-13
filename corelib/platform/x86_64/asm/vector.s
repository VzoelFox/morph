# corelib/platform/x86_64/asm/vector.s
# Implementasi MorphVector (Dynamic Array) - Linux x86_64

.section .text
.global vector_new
.global vector_push
.global vector_get

# External functions
.extern mem_alloc
.extern mem_free
.extern __mf_memcpy

# Struktur Vector (32 bytes)
# [0] Buffer Ptr
# [8] Length
# [16] Capacity
# [24] Item Size

# ------------------------------------------------------------------------------
# func vector_new(item_size: i64) -> ptr (Vector*)
# Input: %rdi = item_size
# Output: %rax = pointer to Vector struct
# ------------------------------------------------------------------------------
vector_new:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12          # Save callee-saved reg

    movq %rdi, %r12     # Simpan item_size

    # 1. Alokasi struct Vector (32 bytes)
    movq $32, %rdi
    call mem_alloc
    testq %rax, %rax
    jz .vnew_fail

    # 2. Inisialisasi
    movq $0, 0(%rax)    # buffer = NULL
    movq $0, 8(%rax)    # length = 0
    movq $0, 16(%rax)   # capacity = 0
    movq %r12, 24(%rax) # item_size

    # Return pointer struct
    popq %r12
    leave
    ret

.vnew_fail:
    xorq %rax, %rax
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func vector_push(vec: ptr, item_ptr: ptr) -> i64 (0=Success, -1=Fail)
# Input: %rdi = vec pointer, %rsi = item pointer (pointer ke data yang mau dicopy)
# Deskripsi: Menambahkan elemen ke akhir vector. Resize otomatis.
# ------------------------------------------------------------------------------
vector_push:
    pushq %rbp
    movq %rsp, %rbp

    # Simpan register callee-saved karena kita panggil fungsi lain (alloc/memcpy)
    pushq %r12          # vec ptr
    pushq %r13          # item ptr
    pushq %r14          # temp
    pushq %r15          # temp

    movq %rdi, %r12
    movq %rsi, %r13

    # 1. Cek Capacity
    movq 8(%r12), %rax  # length
    movq 16(%r12), %rcx # capacity

    cmpq %rcx, %rax
    jl .vpush_insert    # Jika length < capacity, langsung insert

    # 2. Resize Needed
    # New Cap = (Capacity == 0) ? 8 : Capacity * 2
    movq $8, %r14       # Default 8
    testq %rcx, %rcx
    cmovnzq %rcx, %r14  # Jika cap != 0, r14 = cap
    testq %rcx, %rcx
    jz .calc_size
    addq %rcx, %r14     # r14 = cap * 2 (jika tidak 0)

.calc_size:
    # Size in bytes = NewCap * ItemSize
    movq 24(%r12), %rax # ItemSize
    imulq %r14, %rax    # Total Bytes

    # Alloc New Buffer
    movq %rax, %rdi
    call mem_alloc
    testq %rax, %rax
    jz .vpush_fail      # Alloc fail

    movq %rax, %r15     # New Buffer Ptr

    # Copy Old Buffer (If exists)
    movq 0(%r12), %rsi  # Old Buffer
    testq %rsi, %rsi
    jz .update_vec

    # Memcpy(dest=new, src=old, size=len*itemsize)
    movq %r15, %rdi
    # %rsi sudah old buffer
    movq 8(%r12), %rax  # Length
    imulq 24(%r12), %rax # Total Bytes Used
    movq %rax, %rdx     # Size

    # Simpan register volatile sebelum call (tapi r12-r15 aman)
    call __mf_memcpy

    # Free Old Buffer
    movq 0(%r12), %rdi  # Ptr to free
    call mem_free

.update_vec:
    # Update Struct
    movq %r15, 0(%r12)  # Buffer = New Buffer
    movq %r14, 16(%r12) # Cap = New Cap

.vpush_insert:
    # 3. Insert Item
    # Target Addr = Buffer + (Length * ItemSize)
    movq 0(%r12), %rdi  # Buffer Base
    movq 8(%r12), %rax  # Length
    imulq 24(%r12), %rax # Offset
    addq %rax, %rdi     # Target Ptr

    # Copy Item Data
    # Src = Item Ptr (%r13)
    movq %r13, %rsi
    movq 24(%r12), %rdx # Size = Item Size
    call __mf_memcpy

    # 4. Increment Length
    incq 8(%r12)

    xorq %rax, %rax     # Return 0 (Success)
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

.vpush_fail:
    movq $-1, %rax
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func vector_get(vec: ptr, index: i64) -> ptr (Item Ptr)
# Input: %rdi = vec, %rsi = index
# Output: %rax = pointer to item data (or NULL if OOB)
# ------------------------------------------------------------------------------
vector_get:
    # 1. Bounds Check
    cmpq 8(%rdi), %rsi  # cmp index, length
    jge .vget_fail      # If index >= length
    testq %rsi, %rsi
    js .vget_fail       # If index < 0

    # 2. Calculate Addr
    # Addr = Buffer + (Index * ItemSize)
    movq 24(%rdi), %rax # ItemSize
    imulq %rsi, %rax    # Offset
    addq 0(%rdi), %rax  # Base + Offset
    ret

.vget_fail:
    xorq %rax, %rax
    ret
