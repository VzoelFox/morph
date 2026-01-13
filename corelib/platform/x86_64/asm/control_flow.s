# corelib/platform/x86_64/asm/control_flow.s
# Implementasi Control Flow Helpers (Switch Case Context)
# Sesuai SSOT corelib/core/structures.fox

.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global ctx_switch_new
.global ctx_switch_add_case
.global ctx_switch_free
.extern mem_alloc
.extern vector_new
.extern vector_push

# Struct CasePair (16 bytes)
# [0] Value (i64)
# [8] LabelID (i64)

# ------------------------------------------------------------------------------
# func ctx_switch_new(type: i64) -> ptr
# ------------------------------------------------------------------------------
ctx_switch_new:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    movq %rdi, %r12     # Type

    # Alloc Context (32 bytes)
    movq $32, %rdi
    call mem_alloc
    testq %rax, %rax
    jz .new_fail

    pushq %rax          # Save Ctx Ptr

    # Init Fields
    movq $-1, 8(%rax)   # Default Label = -1
    movq $0, 16(%rax)   # End Label = 0 (Placeholder)
    movq %r12, 24(%rax) # Type

    # Alloc Vector for Cases (Item Size = 16 bytes for CasePair)
    movq $16, %rdi
    call vector_new

    popq %rcx           # Restore Ctx Ptr
    movq %rax, 0(%rcx)  # Cases Vector Ptr
    movq %rcx, %rax     # Return Ctx

    popq %r12
    leave
    ret

.new_fail:
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func ctx_switch_add_case(ctx: ptr, val: i64, label: i64)
# ------------------------------------------------------------------------------
ctx_switch_add_case:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp      # Temp CasePair Struct

    # Construct CasePair on stack
    movq %rsi, -16(%rbp) # Value
    movq %rdx, -8(%rbp)  # Label

    # Vector Push
    # Arg1: Vec Ptr (ctx->cases)
    # Arg2: Item Ptr (Stack)
    movq 0(%rdi), %rdi  # Vec Ptr
    leaq -16(%rbp), %rsi
    call vector_push

    leave
    ret

# ------------------------------------------------------------------------------
# func ctx_switch_free(ctx: ptr)
# ------------------------------------------------------------------------------
ctx_switch_free:
    # TODO: Deep free (vector buffer).
    # For now, memory arena reset handles bulk free.
    ret

.global ctx_loop_new
.global ctx_stack_push
.global ctx_stack_pop

# ------------------------------------------------------------------------------
# func ctx_loop_new(start: i64, end: i64) -> ptr
# ------------------------------------------------------------------------------
ctx_loop_new:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13

    movq %rdi, %r12 # Start
    movq %rsi, %r13 # End

    movq $16, %rdi
    call mem_alloc
    testq %rax, %rax
    jz .loop_fail

    movq %r12, 0(%rax)
    movq %r13, 8(%rax)

    popq %r13
    popq %r12
    leave
    ret

.loop_fail:
    xorq %rax, %rax
    popq %r13
    popq %r12
    leave
    ret

# Stack Node (24 bytes)
# [0] Prev Ptr
# [8] Ctx Ptr
# [16] Type

# ------------------------------------------------------------------------------
# func ctx_stack_push(stack_top: ptr, ctx: ptr, type: i64) -> ptr (New Top)
# ------------------------------------------------------------------------------
ctx_stack_push:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %r14

    movq %rdi, %r12 # Old Top
    movq %rsi, %r13 # Ctx
    movq %rdx, %r14 # Type

    movq $24, %rdi
    call mem_alloc
    testq %rax, %rax
    jz .push_fail

    movq %r12, 0(%rax) # Prev = Old Top
    movq %r13, 8(%rax) # Ctx
    movq %r14, 16(%rax) # Type

    # Return New Top (RAX)
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

.push_fail:
    xorq %rax, %rax
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func ctx_stack_pop(stack_top: ptr) -> ptr (New Top)
# ------------------------------------------------------------------------------
ctx_stack_pop:
    testq %rdi, %rdi
    jz .pop_null

    movq 0(%rdi), %rax # Return Prev
    ret

.pop_null:
    xorq %rax, %rax
    ret
