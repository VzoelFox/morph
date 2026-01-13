# corelib/platform/x86_64/asm/stack.s
# MorphStack: Virtual Stack Implementation (Linux)
# ==============================================================================
# Sesuai SSOT runtime.fox dan Paritas Windows
# [Offset 0x00] Stack Pointer (ptr) - Current Top
# [Offset 0x08] Stack Base (ptr)    - Bottom (High Address)
# [Offset 0x10] Stack Limit (ptr)   - Top Limit (Low Address)

.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global stack_new
.global stack_push
.global stack_pop
.extern mem_alloc

# NOTE: Windows `stack_create`, Linux usually uses `_new` convention in this repo.
# Test used `stack_new`. I will name it `stack_new`.

# ------------------------------------------------------------------------------
# func stack_new(size_bytes: i64) -> stack_struct_ptr
# Input: %rdi = Size
# ------------------------------------------------------------------------------
stack_new:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    subq $16, %rsp      # Alignment

    movq %rdi, %rbx     # Size

    # 1. Alloc Stack Struct (24 bytes needed, align to 32)
    movq $32, %rdi
    call mem_alloc
    testq %rax, %rax
    jz .fail
    movq %rax, %rcx     # Struct Ptr

    # 2. Alloc Raw Stack Memory
    movq %rbx, %rdi
    pushq %rcx          # Save Struct Ptr
    call mem_alloc
    popq %rcx           # Restore Struct Ptr

    testq %rax, %rax
    jz .fail

    # Setup Struct
    # Stack Grows Downwards: Base is Highest Address
    leaq (%rax, %rbx), %r8 # Base = Start + Size

    movq %r8, 0(%rcx)   # SP starts at Base
    movq %r8, 8(%rcx)   # Base
    movq %rax, 16(%rcx) # Limit (Start Address)

    movq %rcx, %rax     # Return Struct Ptr
    jmp .done

.fail:
    xorq %rax, %rax

.done:
    addq $16, %rsp
    popq %rbx
    leave
    ret

# ------------------------------------------------------------------------------
# func stack_push(stack_ptr: ptr, value: i64) -> success(1)/fail(0)
# Input: %rdi = Stack Ptr, %rsi = Value
# ------------------------------------------------------------------------------
stack_push:
    movq 0(%rdi), %rax  # Current SP
    movq 16(%rdi), %r8  # Limit

    # Check Overflow (SP - 8 < Limit)
    leaq -8(%rax), %r9
    cmpq %r8, %r9
    jl .overflow

    # Push
    movq %rsi, (%r9)
    movq %r9, 0(%rdi)   # Update SP

    movq $1, %rax       # Success
    ret

.overflow:
    xorq %rax, %rax     # Fail
    ret

# ------------------------------------------------------------------------------
# func stack_pop(stack_ptr: ptr) -> value (RAX)
# Input: %rdi = Stack Ptr
# Output: %rax = Value. Note: Windows return Status in RDX?
# Linux ABI return value in RAX.
# Windows impl: return value in RAX, status in RDX.
# Linux `executor.s` expects RAX to be value.
# Helper function logic:
# .do_add:
#   call stack_pop
#   pushq %rax (B)
# So checking status is skipped in `executor.s`.
# I will conform to this usage.
# ------------------------------------------------------------------------------
stack_pop:
    movq 0(%rdi), %r8   # Current SP
    movq 8(%rdi), %r9   # Base

    # Check Underflow (SP >= Base)
    cmpq %r9, %r8
    jge .underflow

    # Pop
    movq (%r8), %rax    # Get Value
    addq $8, %r8
    movq %r8, 0(%rdi)   # Update SP

    # Return Value in RAX
    ret

.underflow:
    xorq %rax, %rax
    ret

# ------------------------------------------------------------------------------
# func stack_free(stack_ptr: ptr) -> void
# Input: %rdi = Stack Ptr
# ------------------------------------------------------------------------------
.global stack_free
stack_free:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    subq $16, %rsp

    movq %rdi, %rbx     # Save Struct Ptr

    # 1. Free Buffer (Limit points to start of buffer)
    movq 16(%rbx), %rdi
    call mem_free

    # 2. Free Struct
    movq %rbx, %rdi
    call mem_free

    addq $16, %rsp
    popq %rbx
    leave
    ret
