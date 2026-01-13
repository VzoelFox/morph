; corelib/platform/x86_64/asm_win/executor.asm
; Executor: Menjalankan Fragment RPN (Windows)
; Implementasi Lengkap: Aritmatika, Logic, Control Flow, Calls, Syscalls

%include "corelib/platform/x86_64/asm_win/macros.inc"
%include "corelib/platform/x86_64/asm/rpn.inc"
%include "corelib/platform/x86_64/asm/syscalls_nasm.inc"

section .text
global executor_run_with_stack
global executor_exec_label
executor_exec_label equ executor_run_with_stack.trampoline_entry

extern __mf_print_int
extern stack_push
extern stack_pop
extern stack_new
extern __sys_write
extern __sys_read
extern __sys_open
extern __sys_close
extern __sys_exit
extern GetSystemTimeAsFileTime
extern mem_alloc
extern global_sym_table
extern global_sym_cap
extern sym_table_get_by_hash
extern sym_table_put_by_hash
extern scheduler_get_current
extern scheduler_spawn
extern scheduler_yield

; Math Safety (Externs)
extern __mf_add_checked
extern __mf_sub_checked
extern __mf_mul_checked

; Network Externs (Windows)
extern __mf_net_socket
extern __mf_net_connect
extern __mf_net_send
extern __mf_net_recv
extern __mf_net_bind
extern __mf_net_listen
extern __mf_net_accept

; Graphics Externs (Windows)
extern __mf_window_create
extern __mf_draw_pixel
extern __mf_window_poll

; Crypto Externs
extern __mf_sha256_init
extern __mf_sha256_update
extern __mf_sha256_final
extern __mf_chacha20_block
extern __mf_chacha20_xor_stream

; ------------------------------------------------------------------------------
; func executor_run_with_stack(fragment_ptr: ptr, stack_ptr: ptr)
; Input: RCX = Fragment, RDX = MorphStack
; ------------------------------------------------------------------------------
executor_run_with_stack:
    push rbp
    mov rbp, rsp
    push rbx
    push r12 ; Fragment
    push r13 ; Code Ptr
    push r14 ; Code Size
    push r15 ; IP Offset
    push rsi ; Stack Ptr
    push rdi ; Call Stack Ptr
    sub rsp, 40 ; 8 pushes (64) + 40 = 104. 104+8(Ret) = 112 (16*7). Aligned.

    mov r12, rcx
    mov rsi, rdx

    ; SAFETY: Validate Fragment Pointer
    test r12, r12
    jz .err_null_fragment

    ; 1. Initialize Call Stack
    mov rcx, 8192
    sub rsp, 32
    call stack_new
    add rsp, 32
    test rax, rax
    jz .err_stack_alloc_failed
    mov rdi, rax

    mov r13, [r12 + 8]
    mov r14, [r12 + 16]

    ; SAFETY: Validate Code Pointer and Size
    test r13, r13
    jz .err_null_code_ptr
    test r14, r14
    jz .err_zero_code_size

    ; SAFETY: Check for reasonable code size limit (16MB max)
    mov rax, 16777216
    cmp r14, rax
    jg .err_code_too_large

    xor r15, r15

.fetch:
    cmp r15, r14
    jge .done

    ; SSOT: 8-byte Opcode at +0
    mov rax, [r13 + r15]
    add r15, 16 ; Advance IP (16 bytes instruction)

    cmp rax, OP_EXIT
    je .done

    ; Data
    cmp rax, OP_LIT
    je .do_lit
    cmp rax, OP_DUP
    je .do_dup
    cmp rax, OP_POP
    je .do_pop
    cmp rax, OP_PICK
    je .do_pick
    cmp rax, OP_POKE
    je .do_poke
    cmp rax, OP_LOAD
    je .do_load
    cmp rax, OP_STORE
    je .do_store

    ; Arithmetic
    cmp rax, OP_ADD
    je .do_add
    cmp rax, OP_SUB
    je .do_sub
    cmp rax, OP_MUL
    je .do_mul
    cmp rax, OP_DIV
    je .do_div
    cmp rax, OP_MOD
    je .do_mod

    ; Logic
    cmp rax, OP_EQ
    je .do_eq
    cmp rax, OP_NEQ
    je .do_neq
    cmp rax, OP_LT
    je .do_lt
    cmp rax, OP_GT
    je .do_gt
    cmp rax, OP_LEQ
    je .do_leq
    cmp rax, OP_GEQ
    je .do_geq

    ; Control Flow
    cmp rax, OP_JMP
    je .do_jmp
    cmp rax, OP_JMP_IF
    je .do_jmp_if
    cmp rax, OP_JMP_FALSE
    je .do_jmp_false
    cmp rax, OP_SWITCH
    je .do_switch
    cmp rax, OP_LABEL
    je .do_label

    ; System Interface
    cmp rax, OP_SYSCALL
    je .do_syscall

    ; Memory
    cmp rax, OP_MEM_READ
    je .do_mem_read
    cmp rax, OP_MEM_WRITE
    je .do_mem_write

    ; Concurrency
    cmp rax, OP_SPAWN
    je .do_spawn
    cmp rax, OP_YIELD
    je .do_yield

    ; Calls
    cmp rax, OP_CALL
    je .do_call
    cmp rax, OP_RET
    je .do_ret

    ; Debug
    cmp rax, OP_PRINT
    je .do_print
    cmp rax, OP_HINT
    je .do_hint

    jmp .fetch

.do_lit:
    ; Operand at -8
    mov rdx, [r13 + r15 - 8]
    mov rcx, rsi
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_dup:
    mov r8, [rsi + 0]
    mov r9, [rsi + 8]
    cmp r8, r9
    jge .fetch
    mov rdx, [r8]
    mov rcx, rsi
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_pop:
    mov rcx, rsi
    call stack_pop
    jmp .fetch

.do_pick:
    mov rcx, rsi
    call stack_pop
    mov rcx, rax ; Depth

    mov r8, [rsi + 0] ; SP
    lea r9, [r8 + rcx * 8] ; Target

    mov r10, [rsi + 8] ; Base
    cmp r9, r10
    jge .pick_fail

    mov rdx, [r9]
    mov rcx, rsi
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.pick_fail:
    mov rcx, rsi
    mov rdx, 0
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_poke:
    ; 1. Pop Depth
    mov rcx, rsi
    call stack_pop
    mov rcx, rax ; Depth

    ; 2. Pop New Value
    push rcx
    mov rcx, rsi
    call stack_pop
    pop rcx
    mov r11, rax ; New Value

    ; 3. Target Address
    mov r8, [rsi + 0] ; SP
    lea r9, [r8 + rcx * 8] ; Target

    ; 4. Check Bounds
    mov r10, [rsi + 8] ; Base
    cmp r9, r10
    jge .poke_fail

    ; 5. Write
    mov [r9], r11
    jmp .fetch

.poke_fail:
    jmp .fetch

.do_load:
    mov rdx, [r13 + r15 - 8]    ; Hash
    mov rcx, [rel global_sym_table]
    mov r8, [rel global_sym_cap]
    sub rsp, 32
    call sym_table_get_by_hash
    add rsp, 32
    cmp rax, -1
    je .load_null
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.load_null:
    mov rcx, rsi
    mov rdx, 0
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_store:
    mov rdx, [r13 + r15 - 8]    ; Hash
    mov rcx, rsi
    push rdx
    call stack_pop
    pop rdx
    mov r8, rax
    mov rcx, [rel global_sym_table]
    mov r9, [rel global_sym_cap]
    sub rsp, 32
    call sym_table_put_by_hash
    add rsp, 32
    jmp .fetch

.store_fail:
    jmp .fetch

.do_add:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rbx
    mov rcx, rax
    sub rsp, 32
    call __mf_add_checked
    add rsp, 32
    test rdx, rdx
    jnz .err_math_overflow
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_sub:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rbx
    mov rcx, rax
    sub rsp, 32
    call __mf_sub_checked
    add rsp, 32
    test rdx, rdx
    jnz .err_math_overflow
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_mul:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rbx
    mov rcx, rax
    sub rsp, 32
    call __mf_mul_checked
    add rsp, 32
    test rdx, rdx
    jnz .err_math_overflow
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_div:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop

    ; SAFETY: Check Division by Zero
    test rbx, rbx
    jz .err_division_by_zero

    cqo
    idiv rbx
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_mod:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop

    ; SAFETY: Check Division by Zero (Modulo)
    test rbx, rbx
    jz .err_division_by_zero

    cqo
    idiv rbx
    mov rcx, rsi
    mov rdx, rdx
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_eq:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop
    cmp rax, rbx
    sete al
    movzx rax, al
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_neq:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop
    cmp rax, rbx
    setne al
    movzx rax, al
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_lt:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop
    cmp rax, rbx
    setl al
    movzx rax, al
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_gt:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop
    cmp rax, rbx
    setg al
    movzx rax, al
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_leq:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop
    cmp rax, rbx
    setle al
    movzx rax, al
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_geq:
    mov rcx, rsi
    call stack_pop
    mov rbx, rax
    mov rcx, rsi
    call stack_pop
    cmp rax, rbx
    setge al
    movzx rax, al
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_jmp:
    mov rdx, [r13 + r15 - 8]
    add r15, rdx
    jmp .fetch

.do_jmp_if:
    mov rdx, [r13 + r15 - 8]
    push rdx
    mov rcx, rsi
    call stack_pop
    pop rdx
    test rax, rax
    jnz .apply_jump
    jmp .fetch

.apply_jump:
    add r15, rdx
    jmp .fetch

.do_jmp_false:
    mov rdx, [r13 + r15 - 8]
    push rdx
    mov rcx, rsi
    call stack_pop
    pop rdx
    test rax, rax
    jz .apply_jump
    jmp .fetch

.do_label:
    jmp .fetch

.do_switch:
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rdx, r15
    mov r9, [r13 + r15]
    add r15, 8
    mov rcx, [r13 + r15]
    add r15, 8
.switch_loop_2:
    test rcx, rcx
    jz .switch_apply_default
    mov r10, [r13 + r15]
    mov r11, [r13 + r15 + 8]
    cmp r8, r10
    je .switch_apply_case
    add r15, 16
    dec rcx
    jmp .switch_loop_2
.switch_apply_case:
    mov r15, rdx
    add r15, r11
    jmp .fetch
.switch_apply_default:
    mov r15, rdx
    add r15, r9
    jmp .fetch

; --- SYSCALL HANDLER ---
.do_syscall:
    ; 1. Pop Intent ID
    mov rcx, rsi
    call stack_pop
    mov r8, rax ; Intent ID

    ; 2. Boundary Hook
.glue_pre_syscall:
    nop

    ; 3. Dispatch
    cmp r8, SYS_INTENT_WRITE
    je .sys_write
    cmp r8, SYS_INTENT_READ
    je .sys_read
    cmp r8, SYS_INTENT_OPEN
    je .sys_open
    cmp r8, SYS_INTENT_CLOSE
    je .sys_close
    cmp r8, SYS_INTENT_EXIT
    je .sys_exit
    cmp r8, SYS_INTENT_TIME
    je .sys_time
    cmp r8, SYS_INTENT_MMAP
    je .sys_mmap

    ; Crypto
    cmp r8, SYS_INTENT_SHA256_INIT
    je .sys_sha256_init
    cmp r8, SYS_INTENT_SHA256_UPDATE
    je .sys_sha256_update
    cmp r8, SYS_INTENT_SHA256_FINAL
    je .sys_sha256_final
    cmp r8, SYS_INTENT_CHACHA_BLOCK
    je .sys_chacha_block
    cmp r8, SYS_INTENT_CHACHA_STREAM
    je .sys_chacha_stream

    ; Graphics
    cmp r8, SYS_INTENT_WINDOW_CREATE
    je .sys_win_create
    cmp r8, SYS_INTENT_DRAW_PIXEL
    je .sys_draw_pixel
    cmp r8, SYS_INTENT_EVENT_POLL
    je .sys_event_poll

    ; Unknown
    mov rcx, rsi
    mov rdx, -1
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_write:
    mov rcx, rsi
    call stack_pop
    mov r9, rax ; Len
    mov rcx, rsi
    call stack_pop
    mov rdx, rax ; Ptr
    mov rcx, rsi
    call stack_pop
    mov rcx, rax ; FD
    mov r8, r9
    sub rsp, 32
    call __sys_write
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_read:
    mov rcx, rsi
    call stack_pop
    mov r8, rax ; Len
    mov rcx, rsi
    call stack_pop
    mov rdx, rax ; Buf
    mov rcx, rsi
    call stack_pop
    mov rcx, rax ; FD
    sub rsp, 32
    call __sys_read
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_open:
    mov rcx, rsi
    call stack_pop
    mov r8, rax ; Mode
    mov rcx, rsi
    call stack_pop
    mov rdx, rax ; Flags
    mov rcx, rsi
    call stack_pop
    mov rcx, rax ; Path
    sub rsp, 32
    call __sys_open
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_close:
    mov rcx, rsi
    call stack_pop
    mov rcx, rax ; FD
    sub rsp, 32
    call __sys_close
    add rsp, 32
    mov rcx, rsi
    mov rdx, 0
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_mmap:
    mov rcx, rsi
    call stack_pop ; Offset (Ignored)
    mov rcx, rsi
    call stack_pop ; FD (Ignored)
    mov rcx, rsi
    call stack_pop ; Flags (Ignored)
    mov rcx, rsi
    call stack_pop ; Prot (Ignored)
    mov rcx, rsi
    call stack_pop ; Len
    mov r8, rax    ; Size for Alloc
    mov rcx, rsi
    call stack_pop ; Addr (Ignored)
    mov rcx, r8
    sub rsp, 32
    call mem_alloc
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_exit:
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    call __sys_exit

.sys_time:
    sub rsp, 40
    lea rcx, [rsp + 32]
    call GetSystemTimeAsFileTime
    mov rax, [rsp + 32]
    mov rdx, 0x019DB1DED53E8000
    sub rax, rdx
    mov rcx, 10000000
    xor rdx, rdx
    div rcx
    add rsp, 40
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_socket:
    mov rcx, rsi
    call stack_pop
    mov r9, rax
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    mov rdx, r8
    mov r8, r9
    call __mf_net_socket
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_connect:
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_net_connect
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_send:
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_net_send
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_recv:
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_net_recv
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_bind:
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_net_bind
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_listen:
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_net_listen
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_accept:
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_net_accept
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_win_create:
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_window_create
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_draw_pixel:
    mov rcx, rsi
    call stack_pop
    mov r9, rax
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_draw_pixel
    add rsp, 32
    mov rcx, rsi
    mov rdx, 0
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_event_poll:
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_window_poll
    add rsp, 32
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

; --- CRYPTO HANDLERS ---
.sys_sha256_init:
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_sha256_init
    add rsp, 32
    mov rcx, rsi
    mov rdx, 0
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_sha256_update:
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_sha256_update
    add rsp, 32
    mov rcx, rsi
    mov rdx, 0
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_sha256_final:
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_sha256_final
    add rsp, 32
    mov rcx, rsi
    mov rdx, 0
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_chacha_block:
    mov rcx, rsi
    call stack_pop
    mov r9, rax
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_chacha20_block
    add rsp, 32
    mov rcx, rsi
    mov rdx, 0
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.sys_chacha_stream:
    mov rcx, rsi
    call stack_pop
    push rax
    mov rcx, rsi
    call stack_pop
    push rax
    mov rcx, rsi
    call stack_pop
    mov r9, rax
    mov rcx, rsi
    call stack_pop
    mov r8, rax
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    sub rsp, 32
    call __mf_chacha20_xor_stream
    add rsp, 32
    pop rax
    pop rax
    mov rcx, rsi
    mov rdx, 0
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_mem_read:
    mov rcx, rsi
    call stack_pop
    mov rcx, rax

    ; SAFETY: Validate address is not NULL
    test rcx, rcx
    jz .err_null_ptr_deref

    ; SAFETY: Check if address is in user space (bit 47 must be 0)
    mov rdx, rcx
    shr rdx, 47
    test rdx, rdx
    jnz .err_invalid_memory_access

    mov rax, [rcx]
    mov rcx, rsi
    mov rdx, rax
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    jmp .fetch

.do_mem_write:
    mov rcx, rsi
    call stack_pop
    mov rdx, rax
    mov rcx, rsi
    call stack_pop
    mov rcx, rax

    ; SAFETY: Validate address is not NULL
    test rcx, rcx
    jz .err_null_ptr_deref

    ; SAFETY: Check if address is in user space
    mov r8, rcx
    shr r8, 47
    test r8, r8
    jnz .err_invalid_memory_access

    mov [rcx], rdx
    jmp .fetch

.do_spawn:
    mov rdx, [r13 + r15 - 8]
    lea rcx, [rel executor_exec_label]
    sub rsp, 32
    call scheduler_spawn
    add rsp, 32
    jmp .fetch

.do_yield:
    sub rsp, 32
    call scheduler_yield
    add rsp, 32
    jmp .fetch

.do_call:
    mov rdx, [r13 + r15 - 8]
    mov rcx, rdi
    mov r8, rdx
    mov rdx, r15
    call stack_push
    test rax, rax
    jz .err_stack_overflow
    add r15, r8
    jmp .fetch

.do_ret:
    mov rcx, rdi
    call stack_pop
    mov r15, rax
    jmp .fetch

.do_print:
    mov rcx, rsi
    call stack_pop
    mov rcx, rax
    call __mf_print_int
    jmp .fetch

.do_hint:
    jmp .fetch

.err_stack_overflow:
    mov rcx, 102
    call __sys_exit

.err_math_overflow:
    mov rcx, 103
    call __sys_exit

.err_division_by_zero:
    mov rcx, 104
    call __sys_exit

.err_null_fragment:
    mov rcx, 105
    call __sys_exit

.err_null_code_ptr:
    mov rcx, 106
    call __sys_exit

.err_zero_code_size:
    mov rcx, 107
    call __sys_exit

.err_code_too_large:
    mov rcx, 108
    call __sys_exit

.err_stack_alloc_failed:
    mov rcx, 109
    call __sys_exit

.err_null_ptr_deref:
    mov rcx, 110
    call __sys_exit

.err_invalid_memory_access:
    mov rcx, 111
    call __sys_exit

.done:
    add rsp, 40
    pop rdi
    pop rsi
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

.trampoline_entry:
    pop r15
    sub rsp, 32
    call scheduler_get_current
    add rsp, 32
    mov r12, [rax + 24]
    test r12, r12
    jz .exec_panic
    mov r13, [r12 + 8]
    mov r14, [r12 + 16]
    mov rcx, 8192
    sub rsp, 32
    call stack_new
    add rsp, 32
    mov rsi, rax
    mov rcx, 8192
    sub rsp, 32
    call stack_new
    add rsp, 32
    mov rdi, rax
    jmp .fetch

.exec_panic:
    mov rcx, 101
    call __sys_exit
