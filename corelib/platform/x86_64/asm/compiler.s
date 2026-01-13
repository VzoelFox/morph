# corelib/platform/x86_64/asm/compiler.s
# Compiler: IntentAST -> Fragment Bytecode
# Supports: Control Flow, Vars, Fixed Logic
# ==============================================================================

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/rpn_gas.inc"
.include "corelib/platform/x86_64/asm/intent.s"

.section .text
.global compile_ast_to_fragment
.extern mem_alloc
.extern __mf_string_hash

# ------------------------------------------------------------------------------
# func compile_ast_to_fragment(node_ptr: ptr) -> fragment_ptr
# ------------------------------------------------------------------------------
compile_ast_to_fragment:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    subq $32, %rsp

    movq %rdi, %r12

    movq $32, %rdi
    call mem_alloc
    movq %rax, %r13

    movq $8192, %rdi # Larger buffer
    call mem_alloc
    movq %rax, %r14

    movq %r14, 8(%r13)
    xorq %r15, %r15

    # Compile Root (Unit or Node)
    # If Unit, we compile its children chain.
    # If Node, check type.
    movq %r12, %rdi
    call compile_dispatch

    # Finalize
    movq $OP_EXIT, 0(%r14, %r15, 1)
    movq $0, 8(%r14, %r15, 1)
    addq $16, %r15

    movq %r15, 16(%r13)

    movq %r13, %rax
    addq $32, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret

# ------------------------------------------------------------------------------
# compile_dispatch(node)
# Dispatches based on type. DOES NOT follow siblings automatically.
# ------------------------------------------------------------------------------
compile_dispatch:
    pushq %rbx
    movq %rdi, %rbx
    testq %rbx, %rbx
    jz .cd_done

    movq INTENT_OFFSET_TYPE(%rbx), %rax

    # Hierarchy
    cmpq $INTENT_UNIT_MODULE, %rax
    je .c_container
    cmpq $INTENT_SHARD_FUNC, %rax
    je .c_container
    cmpq $INTENT_SHARD_BLOCK, %rax
    je .c_container

    # Fragments
    cmpq $INTENT_FRAG_LITERAL, %rax
    je .c_lit
    cmpq $INTENT_FRAG_BINARY, %rax
    je .c_bin
    cmpq $INTENT_FRAG_VAR, %rax
    je .c_var_read
    cmpq $INTENT_FRAG_ASSIGN, %rax
    je .c_var_write
    cmpq $INTENT_FRAG_IF, %rax
    je .c_if
    cmpq $INTENT_FRAG_WHILE, %rax
    je .c_while
    cmpq $INTENT_FRAG_RETURN, %rax
    je .c_return

    # String & Syscall
    cmpq $0x4002, %rax
    je .c_string
    cmpq $0x4003, %rax
    je .c_syscall

    # Import
    cmpq $0x1002, %rax
    je .c_import

.cd_done:
    popq %rbx
    ret

# ------------------------------------------------------------------------------
# compile_chain(node)
# Compiles node and all its siblings.
# ------------------------------------------------------------------------------
compile_chain:
    pushq %rbx
    movq %rdi, %rbx
.cc_loop:
    testq %rbx, %rbx
    jz .cc_done

    movq %rbx, %rdi
    call compile_dispatch

    movq INTENT_OFFSET_NEXT(%rbx), %rbx
    jmp .cc_loop
.cc_done:
    popq %rbx
    ret

# ------------------------------------------------------------------------------
# Handlers
# ------------------------------------------------------------------------------
.c_container:
    # Compile Child Chain
    movq INTENT_OFFSET_CHILD(%rbx), %rdi
    call compile_chain
    jmp .cd_done

.c_lit:
    movq $OP_LIT, 0(%r14, %r15, 1)
    movq INTENT_OFFSET_DATA_A(%rbx), %rax
    movq %rax, 8(%r14, %r15, 1)
    addq $16, %r15
    jmp .cd_done

.c_bin:
    # Check Op Special Case (.)
    movq INTENT_OFFSET_DATA_A(%rbx), %rax
    cmpq $'.', %rax
    je .c_dot_special

    # Left (Child)
    movq INTENT_OFFSET_CHILD(%rbx), %rdi
    call compile_dispatch

    # Right (Child->Next)
    movq INTENT_OFFSET_CHILD(%rbx), %rax
    movq INTENT_OFFSET_NEXT(%rax), %rdi
    call compile_dispatch

    # Op
    movq INTENT_OFFSET_DATA_A(%rbx), %rax
    # Map Op to RPN
    cmpq $'+', %rax
    je .emit_add
    cmpq $'-', %rax
    je .emit_sub
    cmpq $'*', %rax
    je .emit_mul
    cmpq $'/', %rax
    je .emit_div
    # ... more ops ...
    jmp .cd_done

.emit_add:
    movq $OP_ADD, 0(%r14, %r15, 1)
    jmp .emit_op_end
.emit_sub:
    movq $OP_SUB, 0(%r14, %r15, 1)
    jmp .emit_op_end
.emit_mul:
    movq $OP_MUL, 0(%r14, %r15, 1)
    jmp .emit_op_end
.emit_div:
    movq $OP_DIV, 0(%r14, %r15, 1)
    jmp .emit_op_end

.emit_op_end:
    movq $0, 8(%r14, %r15, 1)
    addq $16, %r15
    jmp .cd_done

.c_var_read:
    # Emit OP_LOAD (Operand = String Hash)
    movq INTENT_OFFSET_DATA_A(%rbx), %rdi
    call get_string_hash
    movq $OP_LOAD, 0(%r14, %r15, 1)
    movq %rax, 8(%r14, %r15, 1) # Write Hash to Bytecode
    addq $16, %r15
    jmp .cd_done

.c_var_write:
    # Expr (Child)
    movq INTENT_OFFSET_CHILD(%rbx), %rdi
    call compile_dispatch

    # Emit OP_STORE (Operand = String Hash)
    movq INTENT_OFFSET_DATA_A(%rbx), %rdi
    call get_string_hash
    movq $OP_STORE, 0(%r14, %r15, 1)
    movq %rax, 8(%r14, %r15, 1) # Write Hash to Bytecode
    addq $16, %r15
    jmp .cd_done

.c_if:
    # Cond (Child)
    movq INTENT_OFFSET_CHILD(%rbx), %rdi
    call compile_dispatch # Dispatch only (don't follow siblings)

    # JmpFalse Placeholder
    movq %r15, %r10
    movq $OP_JMP_FALSE, 0(%r14, %r15, 1)
    addq $16, %r15

    # True Block (Cond->Next)
    movq INTENT_OFFSET_CHILD(%rbx), %rax
    movq INTENT_OFFSET_NEXT(%rax), %rdi
    call compile_dispatch # It's a SHARD_BLOCK, so it calls chain

    # Jmp Placeholder (Skip Else)
    movq %r15, %r11
    movq $OP_JMP, 0(%r14, %r15, 1)
    addq $16, %r15

    # Patch JmpFalse
    movq %r15, %rax
    subq %r10, %rax
    subq $16, %rax # Relative from End of JMP instruction?
    # Executor logic: addq %rdx, %r15. %r15 is at end of JMP.
    # So Offset = Target - EndOfJmp.
    # Current %r15 is Target (Start of Else).
    # EndOfJmp was R10 + 16.
    # So (R15) - (R10+16)
    # Wait, my logic above: subq $16, %rax.
    # R10 = Start of Jmp. R10+16 = End.
    # R15 - (R10 + 16) = R15 - R10 - 16. Correct.
    movq %rax, 8(%r14, %r10, 1)

    # False Block (Cond->Next->Next)
    movq INTENT_OFFSET_CHILD(%rbx), %rax
    movq INTENT_OFFSET_NEXT(%rax), %rcx
    movq INTENT_OFFSET_NEXT(%rcx), %rdi
    testq %rdi, %rdi
    jz .c_if_end

    call compile_dispatch

.c_if_end:
    # Patch Jmp (R11)
    movq %r15, %rax
    subq %r11, %rax
    subq $16, %rax
    movq %rax, 8(%r14, %r11, 1)

    jmp .cd_done

.c_while:
    # Start Label
    movq %r15, %r10

    # Cond
    movq INTENT_OFFSET_CHILD(%rbx), %rdi
    call compile_dispatch

    # JmpFalse Placeholder
    movq %r15, %r11
    movq $OP_JMP_FALSE, 0(%r14, %r15, 1)
    addq $16, %r15

    # Block (Cond->Next)
    movq INTENT_OFFSET_CHILD(%rbx), %rax
    movq INTENT_OFFSET_NEXT(%rax), %rdi
    call compile_dispatch

    # Loop Back
    movq $OP_JMP, 0(%r14, %r15, 1)
    movq %r10, %rax
    subq %r15, %rax
    subq $16, %rax
    movq %rax, 8(%r14, %r15, 1)
    addq $16, %r15

    # Patch JmpFalse
    movq %r15, %rax
    subq %r11, %rax
    subq $16, %rax
    movq %rax, 8(%r14, %r11, 1)

    jmp .cd_done

.c_return:
    movq INTENT_OFFSET_CHILD(%rbx), %rdi
    testq %rdi, %rdi
    jz .emit_ret_void
    call compile_dispatch
.emit_ret_void:
    movq $OP_RET, 0(%r14, %r15, 1)
    movq $0, 8(%r14, %r15, 1)
    addq $16, %r15
    jmp .cd_done

.c_string:
    movq $OP_LIT, 0(%r14, %r15, 1)
    movq INTENT_OFFSET_DATA_A(%rbx), %rax # Ptr
    movq %rax, 8(%r14, %r15, 1)
    addq $16, %r15
    jmp .cd_done

.c_syscall:
    # 1. Compile Args (DataA = List)
    movq INTENT_OFFSET_DATA_A(%rbx), %rdi
    call compile_chain # Iterate list and compile each expr

    # 2. Compile Intent (Child)
    movq INTENT_OFFSET_CHILD(%rbx), %rdi
    call compile_dispatch

    # 3. Emit OP_SYSCALL
    movq $OP_SYSCALL, 0(%r14, %r15, 1)
    movq $0, 8(%r14, %r15, 1)
    addq $16, %r15
    jmp .cd_done

.c_import:
    # Placeholder: Treat import as a No-Op for the bytecode.
    # In V0.3, we rely on the build system or runtime pre-loading
    # the core modules so symbols are globally available.

    # We simply emit nothing or a hint.
    movq $OP_HINT, 0(%r14, %r15, 1)
    movq INTENT_OFFSET_DATA_A(%rbx), %rax
    movq %rax, 8(%r14, %r15, 1)
    addq $16, %r15
    jmp .cd_done

# Helpers
# --- Dot Access Handler ---
.c_dot_special:
    # Compile Left
    movq INTENT_OFFSET_CHILD(%rbx), %rdi
    call compile_dispatch

    # Inspect Right
    movq INTENT_OFFSET_CHILD(%rbx), %rax
    movq INTENT_OFFSET_NEXT(%rax), %rcx # Right Node

    # Get Name (String Struct Ptr)
    movq INTENT_OFFSET_DATA_A(%rcx), %rdi

    # Check "buffer" (Len 6)
    cmpq $6, 0(%rdi)
    jne .check_len
    # Check chars 'buffer'
    movq 8(%rdi), %rsi
    cmpb $'b', 0(%rsi)
    jne .dot_err
    cmpb $'u', 1(%rsi)
    jne .dot_err
    jmp .dot_buffer

.check_len:
    # Check "panjang" (Len 7)
    cmpq $7, 0(%rdi)
    jne .dot_err
    movq 8(%rdi), %rsi
    cmpb $'p', 0(%rsi)
    jne .dot_err
    jmp .dot_len

.dot_buffer:
    # Emit LIT 8, ADD, MEM_READ
    movq $OP_LIT, 0(%r14, %r15, 1)
    movq $8, 8(%r14, %r15, 1)
    addq $16, %r15

    movq $OP_ADD, 0(%r14, %r15, 1)
    movq $0, 8(%r14, %r15, 1)
    addq $16, %r15

    movq $OP_MEM_READ, 0(%r14, %r15, 1)
    movq $0, 8(%r14, %r15, 1)
    addq $16, %r15
    jmp .cd_done

.dot_len:
    # Emit MEM_READ (Offset 0)
    movq $OP_MEM_READ, 0(%r14, %r15, 1)
    movq $0, 8(%r14, %r15, 1)
    addq $16, %r15
    jmp .cd_done

.dot_err:
    jmp .cd_done

get_string_hash:
    movq 0(%rdi), %rsi
    movq 8(%rdi), %rdi
    jmp __mf_string_hash
