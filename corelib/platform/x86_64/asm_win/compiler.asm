; corelib/platform/x86_64/asm_win/compiler.asm
; Compiler: IntentAST -> Fragment Bytecode
; Supports: Control Flow, Vars, Fixed Logic
; ==============================================================================

%include "corelib/platform/x86_64/asm_win/macros.inc"
%include "corelib/platform/x86_64/asm/rpn.inc"
%include "corelib/platform/x86_64/asm_win/intent.inc"

section .data
    ; Strings for . accessor
    str_buffer db "buffer", 0
    str_panjang db "panjang", 0

section .text
global compile_ast_to_fragment
extern mem_alloc

; ------------------------------------------------------------------------------
; func compile_ast_to_fragment(node_ptr: ptr) -> fragment_ptr
; ------------------------------------------------------------------------------
compile_ast_to_fragment:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 32

    mov r12, rcx

    mov rcx, 32
    call mem_alloc
    mov r13, rax

    mov rcx, 8192
    call mem_alloc
    mov r14, rax

    mov [r13 + 8], r14
    xor r15, r15

    mov rcx, r12
    call compile_dispatch

    ; Finalize
    mov byte [r14 + r15], OP_EXIT
    mov qword [r14 + r15 + 8], 0
    add r15, 16

    mov [r13 + 16], r15

    mov rax, r13
    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; compile_dispatch(node)
; ------------------------------------------------------------------------------
compile_dispatch:
    push rbx
    mov rbx, rcx
    test rbx, rbx
    jz .cd_done

    mov rax, [rbx + INTENT_OFFSET_TYPE]

    cmp rax, INTENT_UNIT_MODULE
    je .c_container
    cmp rax, INTENT_SHARD_FUNC
    je .c_container
    cmp rax, INTENT_SHARD_BLOCK
    je .c_container

    cmp rax, INTENT_FRAG_LITERAL
    je .c_lit
    cmp rax, INTENT_FRAG_BINARY
    je .c_bin
    cmp rax, INTENT_FRAG_VAR
    je .c_var_read
    cmp rax, INTENT_FRAG_ASSIGN
    je .c_var_write
    cmp rax, INTENT_FRAG_IF
    je .c_if
    cmp rax, INTENT_FRAG_WHILE
    je .c_while
    cmp rax, INTENT_FRAG_RETURN
    je .c_return

    cmp rax, INTENT_FRAG_STRING
    je .c_string
    cmp rax, INTENT_FRAG_SYSCALL
    je .c_syscall

    ; Import
    cmp rax, 0x1002
    je .c_import

    jmp .cd_done

; ------------------------------------------------------------------------------
; Handlers
; ------------------------------------------------------------------------------
.c_container:
    mov rcx, [rbx + INTENT_OFFSET_CHILD]
    call compile_chain
    jmp .cd_done

.c_lit:
    mov byte [r14 + r15], OP_LIT
    mov rax, [rbx + INTENT_OFFSET_DATA_A]
    mov [r14 + r15 + 8], rax
    add r15, 16
    jmp .cd_done

.c_bin:
    ; Check Op Special Case (.)
    mov rax, [rbx + INTENT_OFFSET_DATA_A]
    cmp rax, '.'
    je .c_dot_special

    mov rcx, [rbx + INTENT_OFFSET_CHILD]
    call compile_dispatch

    mov rax, [rbx + INTENT_OFFSET_CHILD]
    mov rcx, [rax + INTENT_OFFSET_NEXT]
    call compile_dispatch

    mov rax, [rbx + INTENT_OFFSET_DATA_A]
    cmp rax, '+'
    je .emit_add
    cmp rax, '-'
    je .emit_sub
    cmp rax, '*'
    je .emit_mul
    cmp rax, '/'
    je .emit_div
    jmp .cd_done

.emit_add:
    mov byte [r14 + r15], OP_ADD
    jmp .emit_op_end
.emit_sub:
    mov byte [r14 + r15], OP_SUB
    jmp .emit_op_end
.emit_mul:
    mov byte [r14 + r15], OP_MUL
    jmp .emit_op_end
.emit_div:
    mov byte [r14 + r15], OP_DIV
    jmp .emit_op_end

.emit_op_end:
    mov qword [r14 + r15 + 8], 0
    add r15, 16
    jmp .cd_done

.c_var_read:
    mov rax, [rbx + INTENT_OFFSET_DATA_A] ; String Struct Ptr
    mov byte [r14 + r15], OP_LOAD
    mov [r14 + r15 + 8], rax
    add r15, 16
    jmp .cd_done

.c_var_write:
    mov rcx, [rbx + INTENT_OFFSET_CHILD]
    call compile_dispatch

    mov rax, [rbx + INTENT_OFFSET_DATA_A] ; String Struct Ptr
    mov byte [r14 + r15], OP_STORE
    mov [r14 + r15 + 8], rax
    add r15, 16
    jmp .cd_done

.c_if:
    mov rcx, [rbx + INTENT_OFFSET_CHILD]
    call compile_dispatch

    mov r10, r15
    mov byte [r14 + r15], OP_JMP_FALSE
    add r15, 16

    mov rax, [rbx + INTENT_OFFSET_CHILD]
    mov rcx, [rax + INTENT_OFFSET_NEXT]
    call compile_dispatch

    mov r11, r15
    mov byte [r14 + r15], OP_JMP
    add r15, 16

    mov rax, r15
    sub rax, r10
    sub rax, 16
    mov [r14 + r10 + 8], rax

    mov rax, [rbx + INTENT_OFFSET_CHILD]
    mov rcx, [rax + INTENT_OFFSET_NEXT]
    mov rcx, [rcx + INTENT_OFFSET_NEXT]
    test rcx, rcx
    jz .c_if_end

    call compile_dispatch

.c_if_end:
    mov rax, r15
    sub rax, r11
    sub rax, 16
    mov [r14 + r11 + 8], rax

    jmp .cd_done

.c_while:
    mov r10, r15

    mov rcx, [rbx + INTENT_OFFSET_CHILD]
    call compile_dispatch

    mov r11, r15
    mov byte [r14 + r15], OP_JMP_FALSE
    add r15, 16

    mov rax, [rbx + INTENT_OFFSET_CHILD]
    mov rcx, [rax + INTENT_OFFSET_NEXT]
    call compile_dispatch

    mov byte [r14 + r15], OP_JMP
    mov rax, r10
    sub rax, r15
    sub rax, 16
    mov [r14 + r15 + 8], rax
    add r15, 16

    mov rax, r15
    sub rax, r11
    sub rax, 16
    mov [r14 + r11 + 8], rax

    jmp .cd_done

.c_return:
    mov rcx, [rbx + INTENT_OFFSET_CHILD]
    test rcx, rcx
    jz .emit_ret_void
    call compile_dispatch
.emit_ret_void:
    mov byte [r14 + r15], OP_RET
    mov qword [r14 + r15 + 8], 0
    add r15, 16
    jmp .cd_done

.c_string:
    mov byte [r14 + r15], OP_LIT
    mov rax, [rbx + INTENT_OFFSET_DATA_A]
    mov [r14 + r15 + 8], rax
    add r15, 16
    jmp .cd_done

.c_syscall:
    ; 1. Compile Args
    mov rcx, [rbx + INTENT_OFFSET_DATA_A]
    call compile_chain

    ; 2. Compile Intent
    mov rcx, [rbx + INTENT_OFFSET_CHILD]
    call compile_dispatch

    ; 3. Emit SYSCALL
    mov byte [r14 + r15], OP_SYSCALL
    mov qword [r14 + r15 + 8], 0
    add r15, 16
    jmp .cd_done

.c_dot_special:
    ; Compile Left
    mov rcx, [rbx + INTENT_OFFSET_CHILD]
    call compile_dispatch

    ; Inspect Right
    mov rax, [rbx + INTENT_OFFSET_CHILD]
    mov rcx, [rax + INTENT_OFFSET_NEXT] ; Right Node

    ; Get Name (String Struct Ptr)
    mov rdi, [rcx + INTENT_OFFSET_DATA_A]

    ; Check "buffer" (Len 6)
    cmp qword [rdi], 6
    jne .check_len
    ; Check chars
    mov rsi, [rdi + 8]
    cmp byte [rsi], 'b'
    jne .dot_err
    cmp byte [rsi+1], 'u'
    jne .dot_err
    jmp .dot_buffer

.check_len:
    ; Check "panjang" (Len 7)
    cmp qword [rdi], 7
    jne .dot_err
    mov rsi, [rdi + 8]
    cmp byte [rsi], 'p'
    jne .dot_err
    jmp .dot_len

.dot_buffer:
    ; LIT 8, ADD, MEM_READ
    mov byte [r14 + r15], OP_LIT
    mov qword [r14 + r15 + 8], 8
    add r15, 16

    mov byte [r14 + r15], OP_ADD
    mov qword [r14 + r15 + 8], 0
    add r15, 16

    mov byte [r14 + r15], OP_MEM_READ
    mov qword [r14 + r15 + 8], 0
    add r15, 16
    jmp .cd_done

.dot_len:
    ; MEM_READ
    mov byte [r14 + r15], OP_MEM_READ
    mov qword [r14 + r15 + 8], 0
    add r15, 16
    jmp .cd_done

.dot_err:
    jmp .cd_done

.c_import:
    mov byte [r14 + r15], OP_HINT
    mov rax, [rbx + INTENT_OFFSET_DATA_A]
    mov [r14 + r15 + 8], rax
    add r15, 16
    jmp .cd_done

.cd_done:
    pop rbx
    ret

; ------------------------------------------------------------------------------
; compile_chain(node)
; ------------------------------------------------------------------------------
compile_chain:
    push rbx
    mov rbx, rcx
.cc_loop:
    test rbx, rbx
    jz .cc_done

    mov rcx, rbx
    call compile_dispatch

    mov rbx, [rbx + INTENT_OFFSET_NEXT]
    jmp .cc_loop
.cc_done:
    pop rbx
    ret
