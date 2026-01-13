# corelib/platform/x86_64/asm/type.s
# Implementasi Sistem Tipe (Runtime Type Information) untuk Linux x86_64
# Mengelola pembuatan struct metadata dan offset calculation.

.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global type_create
.global type_add_field
.global type_get_size

# Konstanta Ukuran Tipe Primitif
.equ SIZE_I64, 8
.equ SIZE_PTR, 8
.equ SIZE_STRING, 16

# ------------------------------------------------------------------------------
# func type_create(name_ptr: ptr) -> ptr (TypeDescriptor)
# ------------------------------------------------------------------------------
type_create:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx  # Save Name Ptr

    movq %rdi, %rbx

    # Alloc TypeDescriptor (32 bytes)
    movq $32, %rdi
    call mem_alloc

    testq %rax, %rax
    jz .create_fail

    # Init
    # [00] Name
    movq %rbx, 0(%rax)
    # [08] Size = 0
    movq $0, 8(%rax)
    # [10] Count = 0
    movq $0, 16(%rax)
    # [18] Fields = 0
    movq $0, 24(%rax)

.create_fail:
    popq %rbx
    leave
    ret

# ------------------------------------------------------------------------------
# func type_add_field(type_ptr: ptr, name_ptr: ptr, type_id: i64) -> void
# Menambah field ke struct dan mengupdate total size.
# ------------------------------------------------------------------------------
type_add_field:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12  # Type Ptr
    pushq %r13  # Name Ptr
    pushq %r14  # Type ID
    pushq %rbx  # Field Descriptor Ptr

    movq %rdi, %r12
    movq %rsi, %r13
    movq %rdx, %r14

    # 1. Alokasi FieldDescriptor (32 bytes)
    movq $32, %rdi
    call mem_alloc
    testq %rax, %rax
    jz .add_fail
    movq %rax, %rbx

    # 2. Isi Metadata Field
    # Name
    movq %r13, 8(%rbx)
    # Type ID
    movq %r14, 16(%rbx)

    # 3. Hitung Offset
    # Offset = Current Size dari TypeDescriptor
    movq 8(%r12), %rcx
    movq %rcx, 24(%rbx) # Set Offset

    # 4. Update Size Struct
    # Size += Sizeof(TypeID)
    # Switch Type ID
    cmpq $1, %r14   # I64
    je .add_i64
    cmpq $2, %r14   # PTR
    je .add_ptr
    cmpq $3, %r14   # STRING
    je .add_string
    # Default/Unknown -> 8 byte fallback?
    addq $8, %rcx
    jmp .update_size

.add_i64:
    addq $SIZE_I64, %rcx
    jmp .update_size
.add_ptr:
    addq $SIZE_PTR, %rcx
    jmp .update_size
.add_string:
    addq $SIZE_STRING, %rcx
    jmp .update_size

.update_size:
    movq %rcx, 8(%r12) # Update Type Size

    # 5. Link ke List (Prepend / Append?)
    # Prepend lebih mudah (O(1)).
    # Old Head
    movq 24(%r12), %r8
    movq %r8, 0(%rbx) # New->Next = Old Head
    movq %rbx, 24(%r12) # Head = New

    # 6. Inc Count
    incq 16(%r12)

.add_fail:
    popq %rbx
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func type_get_size(type_ptr: ptr) -> i64
# ------------------------------------------------------------------------------
type_get_size:
    movq 8(%rdi), %rax
    ret
