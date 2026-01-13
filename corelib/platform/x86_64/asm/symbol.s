# corelib/platform/x86_64/asm/symbol.s
# Implementasi Symbol Table (Hash Map) untuk Linux x86_64
# Menggunakan Chaining untuk resolusi collision.

.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global sym_table_create
.global sym_table_put
.global sym_table_get
.global sym_table_put_by_hash
.global sym_table_get_by_hash

# ------------------------------------------------------------------------------
# func sym_table_create(capacity: i64) -> ptr
# Input:  %rdi = capacity (jumlah buckets)
# Output: %rax = pointer ke array buckets (semua 0)
# ------------------------------------------------------------------------------
sym_table_create:
    pushq %rbp
    movq %rsp, %rbp

    # 1. Hitung Ukuran Allocation: Capacity * 8 (Pointer Size)
    movq %rdi, %rax
    shlq $3, %rax       # rax = capacity * 8

    # 2. Alokasi Memori (Buckets Array)
    pushq %rdi          # Simpan capacity
    movq %rax, %rdi
    call mem_alloc
    popq %rdi

    # Cek gagal
    testq %rax, %rax
    jz .create_fail

    # 3. Zeroing
    pushq %rax

    movq %rax, %rdx     # rdx = current ptr
    movq %rdi, %rcx     # rcx = count loop

.zero_loop:
    movq $0, (%rdx)
    addq $8, %rdx
    decq %rcx
    jnz .zero_loop

    popq %rax
    leave
    ret

.create_fail:
    xorq %rax, %rax
    leave
    ret

# ------------------------------------------------------------------------------
# func sym_table_put(table: ptr, key_ptr: ptr, key_len: i64, value: i64, capacity: i64)
# Input: %rdi, %rsi, %rdx, %rcx, %r8
# Safe: Compares string content.
# ------------------------------------------------------------------------------
sym_table_put:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rbx

    # Simpan Argumen
    movq %rdi, %r12     # Table
    movq %rsi, %r13     # Key Ptr
    movq %rdx, %r14     # Key Len
    movq %rcx, %r15     # Value
    movq %r8,  %rbx     # Capacity

    # 1. Hash Key
    movq %r13, %rdi
    movq %r14, %rsi
    call __mf_string_hash
    pushq %rax          # Save Hash

    # 2. Index
    xorq %rdx, %rdx
    divq %rbx

    # 3. Akses Bucket
    leaq (%r12, %rdx, 8), %rbx  # Bucket Addr
    movq (%rbx), %r10   # First Node

    # Retrieve Hash
    movq (%rsp), %rax

.find_loop:
    testq %r10, %r10
    jz .add_new_node

    # Compare Hash First
    cmpq %rax, 8(%r10)
    jne .next_node

    # Compare String
    movq 16(%r10), %r9
    testq %r9, %r9
    jz .next_node

    movq 0(%r9), %rsi
    movq 8(%r9), %rdi

    pushq %r10
    pushq %rax # Hash
    pushq %rbx

    movq %r13, %rdx
    movq %r14, %rcx
    call __mf_string_equals

    popq %rbx
    popq %rax
    popq %r10

    cmpq $1, %rax
    jne .next_node

    # Update Value
    movq %r15, 24(%r10)
    popq %rax
    jmp .put_done

.next_node:
    movq 0(%r10), %r10
    jmp .find_loop

.add_new_node:
    # 5. Buat Node Baru
    pushq %rbx # Save Bucket Address
    pushq %rax # Save Hash

    movq $32, %rdi
    call mem_alloc

    popq %r9 # Hash
    popq %rbx

    # Setup Node (%rax)
    movq (%rbx), %r8
    movq %r8, 0(%rax)   # Next
    movq %r9, 8(%rax)   # Hash

    pushq %rax # Save Node
    pushq %rbx

    # Create String Struct
    movq $16, %rdi
    call mem_alloc

    movq %r14, 0(%rax) # Len
    movq %r13, 8(%rax) # Data

    movq %rax, %r8     # Struct Ptr

    popq %rbx
    popq %rax # Node

    movq %r8, 16(%rax)
    movq %r15, 24(%rax)

    # Link
    movq %rax, (%rbx)
    popq %rax # Clear stack (Hash was popped earlier actually)
    # Wait, earlier 'popq %rax' at .put_done handled the success path.
    # Here we came from .find_loop (stack has Hash).
    # I popped Hash into r9. Stack is clean. Correct.

.put_done:
    popq %rbx
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func sym_table_put_by_hash(table, hash, value, capacity)
# Input: %rdi, %rsi, %rdx, %rcx
# Unsafe: Uses only hash.
# ------------------------------------------------------------------------------
sym_table_put_by_hash:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %rbx

    movq %rdi, %r12 # Table
    movq %rsi, %r13 # Hash
    movq %rdx, %r14 # Value
    movq %rcx, %rbx # Capacity

    movq %r13, %rax
    xorq %rdx, %rdx
    divq %rbx

    leaq (%r12, %rdx, 8), %rbx # Bucket Addr
    movq (%rbx), %r10

.ph_loop:
    testq %r10, %r10
    jz .ph_add

    cmpq %r13, 8(%r10)
    je .ph_update

    movq 0(%r10), %r10
    jmp .ph_loop

.ph_update:
    movq %r14, 24(%r10)
    jmp .ph_done

.ph_add:
    pushq %rbx
    movq $32, %rdi
    call mem_alloc
    popq %rbx

    movq (%rbx), %r8
    movq %r8, 0(%rax)
    movq %r13, 8(%rax)
    movq $0, 16(%rax) # NULL string
    movq %r14, 24(%rax)

    movq %rax, (%rbx)

.ph_done:
    popq %rbx
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func sym_table_get(table, key_ptr, key_len, capacity) -> value
# ------------------------------------------------------------------------------
sym_table_get:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %rbx

    movq %rdi, %r12
    movq %rsi, %r13
    movq %rdx, %r14
    movq %rcx, %rbx

    movq %r13, %rdi
    movq %r14, %rsi
    call __mf_string_hash
    movq %rax, %r15 # Hash

    xorq %rdx, %rdx
    divq %rbx

    leaq (%r12, %rdx, 8), %rax
    movq (%rax), %r10

.get_loop:
    testq %r10, %r10
    jz .not_found

    cmpq %r15, 8(%r10)
    jne .get_next

    movq 16(%r10), %r9
    testq %r9, %r9
    jz .get_next

    movq 0(%r9), %rsi
    movq 8(%r9), %rdi

    pushq %r10
    pushq %r15
    movq %r13, %rdx
    movq %r14, %rcx
    call __mf_string_equals
    popq %r15
    popq %r10

    cmpq $1, %rax
    jne .get_next

    movq 24(%r10), %rax
    jmp .get_done

.get_next:
    movq 0(%r10), %r10
    jmp .get_loop

.not_found:
    movq $-1, %rax

.get_done:
    popq %rbx
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func sym_table_get_by_hash(table, hash, capacity)
# Input: %rdi, %rsi, %rdx
# ------------------------------------------------------------------------------
sym_table_get_by_hash:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %rbx

    movq %rdi, %r12
    movq %rsi, %r13
    movq %rdx, %rbx

    movq %r13, %rax
    xorq %rdx, %rdx
    divq %rbx

    leaq (%r12, %rdx, 8), %rax
    movq (%rax), %r10

.gh_loop:
    testq %r10, %r10
    jz .gh_not_found

    cmpq %r13, 8(%r10)
    je .gh_found

    movq 0(%r10), %r10
    jmp .gh_loop

.gh_found:
    movq 24(%r10), %rax
    jmp .gh_done

.gh_not_found:
    movq $-1, %rax

.gh_done:
    popq %rbx
    popq %r13
    popq %r12
    leave
    ret
