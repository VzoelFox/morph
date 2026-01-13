# tools/morph.s
# The Native Morph Runner
# Usage: morph <file.fox|.morph> [args...]

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
    msg_usage:   .asciz "Penggunaan: morph <file.fox|.morph>\n"
    msg_start:   .asciz "Menjalankan "
    msg_dot:     .asciz "...\n"
    msg_err:     .asciz "Gagal.\n"
    msg_inv_ext: .asciz "Ekstensi file tidak dikenali (.fox atau .morph)\n"
    msg_bad_hdr: .asciz "Header .morph tidak valid (Magic/Versi salah)\n"

    ext_fox:     .asciz ".fox"
    ext_morph:   .asciz ".morph"
    newline:     .asciz "\n"

    # Header VZOELFOX + Ver 1
    header_magic: .byte 0x56, 0x5A, 0x4F, 0x45, 0x4C, 0x46, 0x4F, 0x58
    header_ver:   .quad 1

.section .bss
    .lcomm file_buffer, 1048576  # 1MB buffer for reading files
    .lcomm manual_fragment, 32   # Manual Fragment Struct [Type, Buffer, Len, ...]
    .lcomm intent_root, 8

.section .text
.global main
.extern lexer_create
.extern parser_parse_unit
.extern compile_ast_to_fragment
.extern executor_run_with_stack
.extern stack_new
.extern __mf_print_str
.extern __mf_print_int
.extern stack_pop

main:
    pushq %rbp
    movq %rsp, %rbp

    # RDI = argc, RSI = argv
    cmpq $2, %rdi
    jl .usage

    # Save filename (argv[1])
    movq 8(%rsi), %r12  # Filename
    movq %r12, %r15     # Keep copy in R15

    # Print Status
    OS_WRITE $1, msg_start, $12
    call print_str_r12
    OS_WRITE $1, msg_dot, $4

    # Detect Extension
    movq %r12, %rdi
    call get_extension
    testq %rax, %rax
    jz .err_ext

    # Compare extension
    movq %rax, %rbx # Ext Ptr

    # Debug: Print detected extension
    # movq %rbx, %rdi
    # call print_str_r12_custom # Reuse logic manually

    # Check .fox
    movq %rbx, %rdi
    leaq ext_fox(%rip), %rsi
    call str_equals
    testq %rax, %rax
    jnz .run_fox

    # Check .morph
    movq %rbx, %rdi
    leaq ext_morph(%rip), %rsi
    call str_equals
    testq %rax, %rax
    jnz .run_morph

    jmp .err_ext

.run_fox:
    # 1. Read File
    call read_file_r12
    testq %rax, %rax
    js .error

    # RAX = Length, Buffer in file_buffer
    movq %rax, %rsi
    leaq file_buffer(%rip), %rdi

    # 2. Compile Pipeline
    call lexer_create
    movq %rax, %rdi
    call parser_parse_unit
    movq %rax, intent_root(%rip)

    movq intent_root(%rip), %rdi
    call compile_ast_to_fragment
    movq %rax, %r14 # Fragment Ptr

    jmp .execute

.run_morph:
    # 1. Read File
    call read_file_r12
    testq %rax, %rax
    js .error

    # RAX = Length
    movq %rax, %r13 # Total Len

    # 2. Validate Header (16 bytes)
    cmpq $16, %r13
    jl .err_header

    # Check Magic
    leaq file_buffer(%rip), %rsi
    leaq header_magic(%rip), %rdi
    movq $8, %rcx
    call mem_cmp
    testq %rax, %rax
    jnz .err_header

    # Check Version (Offset 8)
    leaq file_buffer(%rip), %rsi
    addq $8, %rsi
    leaq header_ver(%rip), %rdi
    movq $8, %rcx
    call mem_cmp
    testq %rax, %rax
    jnz .err_header

    # 3. Create Manual Fragment
    # Fragment Struct Layout assumption:
    # Offset 0: Type (Ignore)
    # Offset 8: Code Ptr
    # Offset 16: Code Len

    leaq manual_fragment(%rip), %r14

    # Set Code Ptr (Buffer + 16)
    leaq file_buffer(%rip), %rax
    addq $16, %rax
    movq %rax, 8(%r14)

    # Set Code Len (Total - 16)
    movq %r13, %rax
    subq $16, %rax
    movq %rax, 16(%r14)

    jmp .execute

.execute:
    # 4. Setup Stack
    movq $8192, %rdi
    call stack_new
    movq %rax, %rsi # Stack Ptr (Arg2)

    # 5. Run Executor
    movq %r14, %rdi # Fragment Ptr (Arg1)
    pushq %rsi      # Save Stack Ptr
    call executor_run_with_stack
    popq %rdi       # Restore Stack Ptr to RDI

    # Print Result (Top of Stack)
    # We assume the script leaves something on the stack (e.g., the result of the last expression)
    # If stack is empty, stack_pop might return 0 or error, but let's try.
    call stack_pop
    movq %rax, %rdi
    call __mf_print_int

    # Print newline
    OS_WRITE $1, newline, $1

    # Success
    movq $0, %rax
    leave
    ret

.error:
    OS_WRITE $1, msg_err, $7
    movq $1, %rax
    leave
    ret

.err_ext:
    OS_WRITE $1, msg_inv_ext, $48
    movq $1, %rax
    leave
    ret

.err_header:
    OS_WRITE $1, msg_bad_hdr, $46
    movq $1, %rax
    leave
    ret

.usage:
    OS_WRITE $1, msg_usage, $40
    movq $1, %rax
    leave
    ret

# --- Helpers ---

# read_file_r12 -> RAX=Len or -1
read_file_r12:
    movq $2, %rax # OPEN
    movq %r12, %rdi
    movq $0, %rsi # RDONLY
    movq $0, %rdx
    syscall
    testq %rax, %rax
    js .rf_fail
    movq %rax, %r15 # FD

    movq $0, %rax # READ
    movq %r15, %rdi
    leaq file_buffer(%rip), %rsi
    movq $1048576, %rdx
    syscall
    pushq %rax # Save Len

    movq $3, %rax # CLOSE
    movq %r15, %rdi
    syscall

    popq %rax
    ret
.rf_fail:
    movq $-1, %rax
    ret

# get_extension(ptr) -> ptr to dot (last occurrence)
get_extension:
    xorq %rax, %rax
    movq %rdi, %rcx
.ext_loop:
    movb (%rcx), %dl
    testb %dl, %dl
    jz .ext_done
    cmpb $'.', %dl
    cmove %rcx, %rax # Update if dot found
    incq %rcx
    jmp .ext_loop
.ext_done:
    ret

# str_equals(s1, s2) -> 1 if match, 0 if not
str_equals:
    xorq %rax, %rax
.seq_loop:
    movb (%rdi), %al
    movb (%rsi), %bl
    cmpb %al, %bl
    jne .seq_fail
    testb %al, %al
    jz .seq_match
    incq %rdi
    incq %rsi
    jmp .seq_loop
.seq_match:
    movq $1, %rax
    ret
.seq_fail:
    xorq %rax, %rax
    ret

# mem_cmp(p1, p2, len) -> 0 if match, else diff
mem_cmp:
    xorq %rax, %rax
.mc_loop:
    movb (%rdi), %al
    movb (%rsi), %bl
    subb %bl, %al
    jnz .mc_fail
    incq %rdi
    incq %rsi
    decq %rcx
    jnz .mc_loop
    xorq %rax, %rax
    ret
.mc_fail:
    movq $1, %rax
    ret

print_str_r12:
    movq %r12, %rdi
    call strlen
    movq %rax, %rdx
    movq $1, %rax
    movq $1, %rdi
    movq %r12, %rsi
    syscall
    ret

strlen:
    xorq %rax, %rax
.sl_loop:
    cmpb $0, (%rdi, %rax, 1)
    je .sl_done
    incq %rax
    jmp .sl_loop
.sl_done:
    ret
