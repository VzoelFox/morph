# corelib/platform/x86_64/asm/executor.s
# Executor: Menjalankan Fragment RPN (Linux)
# Implementasi Lengkap + Call Stack + Extended Ops + Concurrency

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/rpn_gas.inc"
.include "corelib/platform/x86_64/asm/syscalls.inc"

.section .text
.global executor_run_with_stack
.global executor_exec_label
.extern __mf_print_int
.extern __sys_write
.extern __sys_exit
.extern stack_push
.extern stack_pop
.extern stack_new
.extern mem_alloc
.extern global_sym_table
.extern global_sym_cap
.extern sym_table_get_by_hash
.extern sym_table_put_by_hash
.extern scheduler_spawn
.extern scheduler_yield
.extern __mf_print_token
.extern context_push
.extern context_pop
.extern context_peek
.extern scheduler_init

.extern __mf_net_socket
.extern __mf_net_connect
.extern __mf_net_send
.extern __mf_net_recv
.extern __mf_net_close
.extern __mf_net_bind
.extern __mf_net_listen
.extern __mf_net_accept

.extern __mf_window_create
.extern __mf_draw_pixel
.extern __mf_window_poll

# ------------------------------------------------------------------------------
# func executor_run_with_stack(fragment_ptr: ptr, stack_ptr: ptr)
# ------------------------------------------------------------------------------
executor_run_with_stack:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %r10
    subq $48, %rsp

    movq %rdi, %r12
    pushq %rsi
    popq %rbx           # RBX = Stack Ptr

    # SAFETY: Validate Fragment Pointer
    testq %r12, %r12
    jz .err_null_fragment

    # 1. Initialize Call Stack
    pushq %rdi
    movq $8192, %rdi
    call stack_new
    popq %rdi
    testq %rax, %rax
    jz .err_stack_alloc_failed
    movq %rax, %r10     # R10 = CallStack Ptr

    movq 8(%r12), %r13  # Code Ptr
    movq 16(%r12), %r14 # Code Size

    # SAFETY: Validate Code Pointer and Size
    testq %r13, %r13
    jz .err_null_code_ptr
    testq %r14, %r14
    jz .err_zero_code_size

    # SAFETY: Check for reasonable code size limit (16MB max)
    movq $16777216, %rax
    cmpq %rax, %r14
    jg .err_code_too_large

    xorq %r15, %r15     # IP = 0

.fetch:
    cmpq %r14, %r15
    jge .done

    # SSOT: Opcode is i64 at Offset 0
    movq (%r13, %r15), %rax
    addq $16, %r15      # Advance IP (Instruction Size = 16)

    cmpq $OP_EXIT, %rax
    je .done

    # Data
    cmpq $OP_LIT, %rax
    je .do_lit
    cmpq $OP_DUP, %rax
    je .do_dup
    cmpq $OP_POP, %rax
    je .do_pop
    cmpq $OP_PICK, %rax
    je .do_pick
    cmpq $OP_POKE, %rax
    je .do_poke
    cmpq $OP_LOAD, %rax
    je .do_load
    cmpq $OP_STORE, %rax
    je .do_store

    # Arithmetic
    cmpq $OP_ADD, %rax
    je .do_add
    cmpq $OP_SUB, %rax
    je .do_sub
    cmpq $OP_MUL, %rax
    je .do_mul
    cmpq $OP_DIV, %rax
    je .do_div
    cmpq $OP_MOD, %rax
    je .do_mod

    # Logic
    cmpq $OP_EQ, %rax
    je .do_eq
    cmpq $OP_NEQ, %rax
    je .do_neq
    cmpq $OP_LT, %rax
    je .do_lt
    cmpq $OP_GT, %rax
    je .do_gt
    cmpq $OP_LEQ, %rax
    je .do_leq
    cmpq $OP_GEQ, %rax
    je .do_geq

    # Control Flow
    cmpq $OP_JMP, %rax
    je .do_jmp
    cmpq $OP_JMP_IF, %rax
    je .do_jmp_if
    cmpq $OP_JMP_FALSE, %rax
    je .do_jmp_false
    cmpq $OP_SWITCH, %rax
    je .do_switch
    cmpq $OP_LABEL, %rax
    je .do_label

    # System Interface
    cmpq $OP_SYSCALL, %rax
    je .do_syscall

    # Memory
    cmpq $OP_MEM_READ, %rax
    je .do_mem_read
    cmpq $OP_MEM_WRITE, %rax
    je .do_mem_write

    # Concurrency
    cmpq $OP_SPAWN, %rax
    je .do_spawn
    cmpq $OP_YIELD, %rax
    je .do_yield

    # Calls
    cmpq $OP_CALL, %rax
    je .do_call
    cmpq $OP_RET, %rax
    je .do_ret

    # Debug
    cmpq $OP_PRINT, %rax
    je .do_print
    cmpq $OP_HINT, %rax
    je .do_hint

    jmp .fetch

# --- Handlers ---
.do_lit:
    # Operand at Offset -8 from Current IP
    movq -8(%r13, %r15), %rdx
    movq %rbx, %rdi
    movq %rdx, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_dup:
    movq (%rbx), %r8
    movq 8(%rbx), %r9
    cmpq %r9, %r8
    jge .fetch
    movq (%r8), %rdx
    movq %rbx, %rdi
    movq %rdx, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_pop:
    movq %rbx, %rdi
    call stack_pop
    jmp .fetch

.do_pick:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx     # Depth

    # Addr = SP + (Depth * 8)
    movq 0(%rbx), %r8   # SP
    leaq (%r8, %rcx, 8), %r9

    # Check Bounds
    movq 8(%rbx), %r10
    cmpq %r10, %r9
    jge .pick_fail

    movq (%r9), %rdx
    movq %rbx, %rdi
    movq %rdx, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.pick_fail:
    movq %rbx, %rdi
    movq $0, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_poke:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx     # Depth
    pushq %rcx
    movq %rbx, %rdi
    call stack_pop
    popq %rcx
    movq %rax, %r11     # Value

    movq 0(%rbx), %r8
    leaq (%r8, %rcx, 8), %r9

    movq 8(%rbx), %r10
    cmpq %r10, %r9
    jge .poke_fail

    movq %r11, (%r9)
    jmp .fetch

.poke_fail:
    jmp .fetch

.do_load:
    # Operand: Key Hash (from Compiler v1.1 Portable)
    movq -8(%r13, %r15), %rsi   # Hash

    movq global_sym_table(%rip), %rdi
    movq global_sym_cap(%rip), %rdx # Capacity (3rd arg for _by_hash)

    # sym_table_get_by_hash(table:rdi, hash:rsi, capacity:rdx)
    call sym_table_get_by_hash

    # Check result (-1 = Not Found)
    cmpq $-1, %rax
    je .load_null

    # Push Value
    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow

    jmp .fetch

.load_null:
    movq %rbx, %rdi
    movq $0, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_store:
    # Operand: Key Hash. Stack Top: Value.
    movq -8(%r13, %r15), %rsi   # Hash

    # Pop Value
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdx             # Value (3rd arg)

    movq global_sym_table(%rip), %rdi
    movq global_sym_cap(%rip), %rcx # Capacity (4th arg)

    # sym_table_put_by_hash(table:rdi, hash:rsi, value:rdx, capacity:rcx)
    call sym_table_put_by_hash

    jmp .fetch

.store_fail:
    jmp .fetch

.do_add:
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # b
    movq %rbx, %rdi
    call stack_pop
    popq %rsi  # b (pop to RSI as 2nd arg)
    movq %rax, %rdi # a (1st arg)

    # __mf_add_checked(a, b) -> RAX=val, RDX=err
    call __mf_add_checked
    testq %rdx, %rdx
    jnz .err_math_overflow

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_sub:
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # b
    movq %rbx, %rdi
    call stack_pop
    popq %rsi  # b
    movq %rax, %rdi # a

    # __mf_sub_checked(a, b) -> RAX=val, RDX=err
    call __mf_sub_checked
    testq %rdx, %rdx
    jnz .err_math_overflow

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_mul:
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # b
    movq %rbx, %rdi
    call stack_pop
    popq %rsi  # b
    movq %rax, %rdi # a

    # __mf_mul_checked(a, b) -> RAX=val, RDX=err
    call __mf_mul_checked
    testq %rdx, %rdx
    jnz .err_math_overflow

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_div:
    movq %rbx, %rdi
    call stack_pop
    pushq %rax
    movq %rbx, %rdi
    call stack_pop
    popq %rcx

    # SAFETY: Check Division by Zero
    testq %rcx, %rcx
    jz .err_division_by_zero

    cqo
    idivq %rcx
    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_mod:
    movq %rbx, %rdi
    call stack_pop
    pushq %rax
    movq %rbx, %rdi
    call stack_pop
    popq %rcx

    # SAFETY: Check Division by Zero (Modulo)
    testq %rcx, %rcx
    jz .err_division_by_zero

    cqo
    idivq %rcx
    movq %rbx, %rdi
    movq %rdx, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_eq:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx
    movq %rbx, %rdi
    call stack_pop
    cmpq %rcx, %rax
    sete %al
    movzx %al, %rax
    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_neq:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx
    movq %rbx, %rdi
    call stack_pop
    cmpq %rcx, %rax
    setne %al
    movzx %al, %rax
    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_lt:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx
    movq %rbx, %rdi
    call stack_pop
    cmpq %rcx, %rax
    setl %al
    movzx %al, %rax
    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_gt:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx
    movq %rbx, %rdi
    call stack_pop
    cmpq %rcx, %rax
    setg %al
    movzx %al, %rax
    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_leq:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx
    movq %rbx, %rdi
    call stack_pop
    cmpq %rcx, %rax
    setle %al
    movzx %al, %rax
    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_geq:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx
    movq %rbx, %rdi
    call stack_pop
    cmpq %rcx, %rax
    setge %al
    movzx %al, %rax
    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_jmp:
    movq -8(%r13, %r15), %rdx
    addq %rdx, %r15
    jmp .fetch

.do_jmp_if:
    movq -8(%r13, %r15), %rdx
    pushq %rdx
    movq %rbx, %rdi
    call stack_pop
    popq %rdx
    testq %rax, %rax
    jnz .apply_jump
    jmp .fetch

.apply_jump:
    addq %rdx, %r15
    jmp .fetch

.do_jmp_false:
    movq -8(%r13, %r15), %rdx
    pushq %rdx
    movq %rbx, %rdi
    call stack_pop
    popq %rdx
    testq %rax, %rax
    jz .apply_jump
    jmp .fetch

.do_label:
    jmp .fetch

.do_switch:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %r8
    movq %r15, %rdx
    jmp .fetch

# --- SYSTEM CALLS ---
.do_syscall:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %r8

glue_pre_syscall:
    nop

    cmpq $SYS_INTENT_WRITE, %r8
    je .sys_write
    cmpq $SYS_INTENT_READ, %r8
    je .sys_read
    cmpq $SYS_INTENT_OPEN, %r8
    je .sys_open
    cmpq $SYS_INTENT_CLOSE, %r8
    je .sys_close
    cmpq $SYS_INTENT_EXIT, %r8
    je .sys_exit
    cmpq $SYS_INTENT_TIME, %r8
    je .sys_time

    # Network
    cmpq $SYS_INTENT_SOCKET, %r8
    je .sys_socket
    cmpq $SYS_INTENT_CONNECT, %r8
    je .sys_connect
    cmpq $SYS_INTENT_SEND, %r8
    je .sys_send
    cmpq $SYS_INTENT_RECV, %r8
    je .sys_recv
    cmpq $SYS_INTENT_BIND, %r8
    je .sys_bind
    cmpq $SYS_INTENT_LISTEN, %r8
    je .sys_listen
    cmpq $SYS_INTENT_ACCEPT, %r8
    je .sys_accept
    cmpq $SYS_INTENT_MMAP, %r8
    je .sys_mmap

    # Crypto
    cmpq $SYS_INTENT_SHA256_INIT, %r8
    je .sys_sha256_init
    cmpq $SYS_INTENT_SHA256_UPDATE, %r8
    je .sys_sha256_update
    cmpq $SYS_INTENT_SHA256_FINAL, %r8
    je .sys_sha256_final
    cmpq $SYS_INTENT_CHACHA_BLOCK, %r8
    je .sys_chacha_block
    cmpq $SYS_INTENT_CHACHA_STREAM, %r8
    je .sys_chacha_stream

    # Graphics
    cmpq $SYS_INTENT_WINDOW_CREATE, %r8
    je .sys_win_create
    cmpq $SYS_INTENT_DRAW_PIXEL, %r8
    je .sys_draw_pixel
    cmpq $SYS_INTENT_EVENT_POLL, %r8
    je .sys_event_poll

    movq %rbx, %rdi
    movq $-1, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_write:
    movq %rbx, %rdi
    call stack_pop
    pushq %rax

    movq %rbx, %rdi
    call stack_pop
    pushq %rax

    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi

    popq %rsi
    popq %rdx

    call __sys_write

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_read:
    # Args: FD, Buf, Len
    movq %rbx, %rdi
    call stack_pop # Len
    movq %rax, %rdx

    movq %rbx, %rdi
    call stack_pop # Buf
    movq %rax, %rsi

    movq %rbx, %rdi
    call stack_pop # FD
    movq %rax, %rdi

    movq $0, %rax # SYS_READ
    syscall

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_open:
    # Args: Path, Flags, Mode
    movq %rbx, %rdi
    call stack_pop # Mode
    movq %rax, %rdx

    movq %rbx, %rdi
    call stack_pop # Flags
    movq %rax, %rsi

    movq %rbx, %rdi
    call stack_pop # Path
    movq %rax, %rdi

    movq $2, %rax # SYS_OPEN
    syscall

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_close:
    # Args: FD
    movq %rbx, %rdi
    call stack_pop # FD
    movq %rax, %rdi

    movq $3, %rax # SYS_CLOSE
    syscall

    movq %rbx, %rdi
    movq $0, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_exit:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi
    call __sys_exit

.sys_time:
    movq $SYS_TIME, %rax
    xorq %rdi, %rdi
    syscall

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_socket:
    # Args: Domain, Type, Proto
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Proto
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Type
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # Domain

    popq %rsi # Type
    popq %rdx # Proto
    call __mf_net_socket

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_connect:
    # Args: FD, AddrPtr, AddrLen
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Len
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Ptr
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # FD

    popq %rsi # Ptr
    popq %rdx # Len
    call __mf_net_connect

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_send:
    # Args: FD, Buf, Len
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Len
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Buf
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # FD

    popq %rsi # Buf
    popq %rdx # Len
    call __mf_net_send

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_recv:
    # Args: FD, Buf, Len
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Len
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Buf
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # FD

    popq %rsi # Buf
    popq %rdx # Len
    call __mf_net_recv

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_bind:
    # Args: FD, AddrPtr, AddrLen
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Len
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Ptr
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # FD

    popq %rsi # Ptr
    popq %rdx # Len
    call __mf_net_bind

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_listen:
    # Args: FD, Backlog
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rsi # Backlog
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # FD

    call __mf_net_listen

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_accept:
    # Args: FD, AddrPtr, AddrLenPtr
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # LenPtr
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # AddrPtr
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # FD

    popq %rsi # AddrPtr
    popq %rdx # LenPtr
    call __mf_net_accept

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_mmap:
    # Args: Addr(0), Len, Prot, Flags, FD, Offset(0)
    # Stack Order: [Addr, Len, Prot, Flags, FD, Offset]
    # Pops: Offset, FD, Flags, Prot, Len, Addr
    movq %rbx, %rdi
    call stack_pop # Offset (R9)
    movq %rax, %r9

    movq %rbx, %rdi
    call stack_pop # FD (R8)
    movq %rax, %r8

    movq %rbx, %rdi
    call stack_pop # Flags (R10)
    movq %rax, %r10

    movq %rbx, %rdi
    call stack_pop # Prot (RDX)
    movq %rax, %rdx

    movq %rbx, %rdi
    call stack_pop # Len (RSI)
    movq %rax, %rsi

    movq %rbx, %rdi
    call stack_pop # Addr (RDI)
    movq %rax, %rdi

    movq $9, %rax # SYS_MMAP
    syscall

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

# --- CRYPTO HANDLERS ---
.sys_sha256_init:
    movq %rbx, %rdi
    call stack_pop # Ctx
    movq %rax, %rdi
    call __mf_sha256_init

    movq %rbx, %rdi
    movq $0, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_sha256_update:
    movq %rbx, %rdi
    call stack_pop # Len
    movq %rax, %rdx
    movq %rbx, %rdi
    call stack_pop # Data
    movq %rax, %rsi
    movq %rbx, %rdi
    call stack_pop # Ctx
    movq %rax, %rdi
    call __mf_sha256_update

    movq %rbx, %rdi
    movq $0, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_sha256_final:
    movq %rbx, %rdi
    call stack_pop # Digest
    movq %rax, %rsi
    movq %rbx, %rdi
    call stack_pop # Ctx
    movq %rax, %rdi
    call __mf_sha256_final

    movq %rbx, %rdi
    movq $0, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_chacha_block:
    movq %rbx, %rdi
    call stack_pop # Out
    movq %rax, %rcx
    movq %rbx, %rdi
    call stack_pop # Counter
    movq %rax, %rdx
    movq %rbx, %rdi
    call stack_pop # Nonce
    movq %rax, %rsi
    movq %rbx, %rdi
    call stack_pop # Key
    movq %rax, %rdi
    call __mf_chacha20_block

    movq %rbx, %rdi
    movq $0, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_chacha_stream:
    # Args: Key, Nonce, Counter, In, Out, Len
    # Pops: Len, Out, In, Counter, Nonce, Key
    movq %rbx, %rdi
    call stack_pop # Len (R9)
    movq %rax, %r9

    movq %rbx, %rdi
    call stack_pop # Out (R8)
    movq %rax, %r8

    movq %rbx, %rdi
    call stack_pop # In (RCX)
    movq %rax, %rcx

    movq %rbx, %rdi
    call stack_pop # Counter (RDX)
    movq %rax, %rdx

    movq %rbx, %rdi
    call stack_pop # Nonce (RSI)
    movq %rax, %rsi

    movq %rbx, %rdi
    call stack_pop # Key (RDI)
    movq %rax, %rdi

    call __mf_chacha20_xor_stream

    movq %rbx, %rdi
    movq $0, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_win_create:
    # Args: Width, Height, Title
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Title
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Height
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # Width

    popq %rsi # Height
    popq %rdx # Title
    call __mf_window_create

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_draw_pixel:
    # Args: Handle, X, Y, Color
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Color
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # Y
    movq %rbx, %rdi
    call stack_pop
    pushq %rax # X
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # Handle

    popq %rsi # X
    popq %rdx # Y
    popq %rcx # Color
    call __mf_draw_pixel

    movq %rbx, %rdi
    movq $0, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.sys_event_poll:
    # Args: EventPtr
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi # EventPtr
    call __mf_window_poll

    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

# --- CONCURRENCY ---
.do_spawn:
    # OP_SPAWN (Operand: Label ID / Offset)
    movq -8(%r13, %r15), %rsi   # RSI = Offset (Arg1 for new routine)
    leaq executor_exec_label(%rip), %rdi # RDI = Trampoline Entry Point
    call scheduler_spawn
    # Routine ID is in RAX
    jmp .fetch

.do_yield:
    call scheduler_yield
    jmp .fetch

.do_call:
    # Operand at -8
    movq -8(%r13, %r15), %rdx
    movq %r10, %rdi
    movq %r15, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    addq %rdx, %r15
    jmp .fetch

.do_ret:
    movq %r10, %rdi
    call stack_pop
    movq %rax, %r15
    jmp .fetch

.do_print:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdi
    call __mf_print_int
    jmp .fetch

.do_hint:
    # Operand at -8 is Hint ID
    # For now, just consume it.
    # Future: Print "[HINT] ID: <val>" if Debug Mode is on.
    jmp .fetch

.do_mem_read:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx # Addr

    # SAFETY: Validate address is not NULL
    testq %rcx, %rcx
    jz .err_null_ptr_deref

    # SAFETY: Check if address is in reasonable range (below kernel space)
    # On x86-64 Linux, user space is typically 0x0000000000000000 - 0x00007FFFFFFFFFFF
    movq %rcx, %rdx
    shrq $47, %rdx  # Check if bit 47 is set (kernel space marker)
    testq %rdx, %rdx
    jnz .err_invalid_memory_access

    movq (%rcx), %rax # Read value
    movq %rbx, %rdi
    movq %rax, %rsi
    call stack_push
    testq %rax, %rax
    jz .err_stack_overflow
    jmp .fetch

.do_mem_write:
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rdx # Value
    movq %rbx, %rdi
    call stack_pop
    movq %rax, %rcx # Addr

    # SAFETY: Validate address is not NULL
    testq %rcx, %rcx
    jz .err_null_ptr_deref

    # SAFETY: Check if address is in reasonable range
    movq %rcx, %r8
    shrq $47, %r8
    testq %r8, %r8
    jnz .err_invalid_memory_access

    movq %rdx, (%rcx) # Write value
    jmp .fetch

.err_stack_overflow:
    movq $102, %rdi
    call __sys_exit

.err_math_overflow:
    movq $103, %rdi
    call __sys_exit

.err_division_by_zero:
    movq $104, %rdi
    call __sys_exit

.err_null_fragment:
    movq $105, %rdi
    call __sys_exit

.err_null_code_ptr:
    movq $106, %rdi
    call __sys_exit

.err_zero_code_size:
    movq $107, %rdi
    call __sys_exit

.err_code_too_large:
    movq $108, %rdi
    call __sys_exit

.err_stack_alloc_failed:
    movq $109, %rdi
    call __sys_exit

.err_null_ptr_deref:
    movq $110, %rdi
    call __sys_exit

.err_invalid_memory_access:
    movq $111, %rdi
    call __sys_exit

.done:
    addq $48, %rsp
    popq %r10
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    leave
    ret

# ------------------------------------------------------------------------------
# Helper for Scheduler Bootstrap
# ------------------------------------------------------------------------------
executor_exec_label:
    # Input: Arg1 is on Stack (pushed by scheduler_spawn).
    # We must pop it into R15 (Non-Volatile) to preserve across scheduler_get_current
    popq %r15

    # 1. Recover Code Pointer (Hardcoded for test_concurrency.s)
    # This is a HACK for testing. In real impl, Scheduler Routine has FRAGMENT offset.
    # But for test_concurrency, we use a global 'fragment_struct'.
    # For general use, we should get it from the routine struct.
    # scheduler_get_current() -> RAX
    # RAX + 24 (OFFSET_FRAGMENT) -> R12 (Fragment Ptr)

    call scheduler_get_current
    movq 24(%rax), %r12 # Get Fragment Ptr from Routine
    testq %r12, %r12
    jz .exec_panic # Panic if no fragment

    # Setup R13 (Code Ptr) and R14 (Size)
    movq 8(%r12), %r13
    movq 16(%r12), %r14

    # Setup R15 (IP) = Label ID (Offset)
    # Already in R15

    # Initialize RBX (Stack Ptr)
    movq $8192, %rdi
    call stack_new
    movq %rax, %rbx # RBX = Data Stack Ptr

    # Initialize R10 (Call Stack Ptr)
    movq $8192, %rdi
    call stack_new
    movq %rax, %r10

    # Jump into fetch loop
    jmp .fetch

.exec_panic:
    movq $101, %rdi
    call __sys_exit
