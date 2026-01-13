# tools/dump_rpn.s
# Utility to Compile .fox files and Dump RPN Bytecode to a file
# Usage: ./dump_rpn input.fox output.morph

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
    msg_start: .asciz "Mengkompilasi "
    msg_to: .asciz " ke "
    msg_done: .asciz "Selesai.\n"
    msg_err: .asciz "Gagal.\n"
    msg_usage: .asciz "Penggunaan: dump_rpn input.fox output.morph\n"
    newline: .asciz "\n"

    # .morph Header: Magic (8 bytes) + Version (8 bytes)
    # Magic: "VZOELFOX" = 0x584F464C454F5A56 (Little Endian)
    # Version: 1
    morph_header:
        .byte 0x56, 0x5A, 0x4F, 0x45, 0x4C, 0x46, 0x4F, 0x58  # VZOELFOX
        .quad 1                                               # Version 1

.section .bss
    .lcomm file_buffer, 65536
    .lcomm intent_root, 8

.section .text
.global main
.extern mem_alloc
.extern lexer_create
.extern parser_parse_unit
.extern compile_ast_to_fragment
.extern __mf_print_str

main:
    pushq %rbp
    movq %rsp, %rbp

    # RDI = argc, RSI = argv
    cmpq $3, %rdi
    jl .usage

    # Save argv
    movq %rsi, %r15

    # Arg 1: Input File
    movq 8(%r15), %r12
    # Arg 2: Output File
    movq 16(%r15), %r13

    # Print Status
    OS_WRITE $1, msg_start, $14
    call print_str_r12
    OS_WRITE $1, msg_to, $4
    call print_str_r13
    OS_WRITE $1, newline, $1

    # 1. Read Input File
    call read_input_file
    testq %rax, %rax
    js .error

    # RAX has length
    movq %rax, %rsi # Arg 2: Len
    leaq file_buffer(%rip), %rdi # Arg 1: Ptr

    # 2. Tokenize
    call lexer_create
    movq %rax, %rbx # Lexer Ptr

    # 3. Parse
    movq %rbx, %rdi
    call parser_parse_unit
    movq %rax, intent_root(%rip)

    # 4. Compile
    movq intent_root(%rip), %rdi
    call compile_ast_to_fragment
    # Returns Fragment Ptr.
    movq %rax, %r14

    movq 8(%r14), %rsi # Buffer
    movq 16(%r14), %rdx # Length

    # 5. Write to Output File
    call write_output_file

    OS_WRITE $1, msg_done, $9

    movq $0, %rax
    leave
    ret

.error:
    OS_WRITE $1, msg_err, $7
    movq $1, %rax
    leave
    ret

.usage:
    OS_WRITE $1, msg_usage, $44
    movq $1, %rax
    leave
    ret

# Helpers
read_input_file:
    movq $2, %rax # SYS_OPEN
    movq %r12, %rdi
    movq $0, %rsi # O_RDONLY
    movq $0, %rdx
    syscall
    testq %rax, %rax
    js .rif_err
    movq %rax, %r15 # FD

    movq $0, %rax # SYS_READ
    movq %r15, %rdi
    leaq file_buffer(%rip), %rsi
    movq $65536, %rdx
    syscall

    pushq %rax
    movq $3, %rax # SYS_CLOSE
    movq %r15, %rdi
    syscall
    popq %rax
    ret
.rif_err:
    movq $-1, %rax
    ret

write_output_file:
    # Inputs: %rsi (Payload Ptr), %rdx (Payload Len), %r13 (Filename)
    pushq %rsi # Save Payload Ptr
    pushq %rdx # Save Payload Len

    movq $2, %rax # SYS_OPEN
    movq %r13, %rdi
    movq $0102, %rsi # O_CREAT | O_WRONLY
    movq $0644, %rdx
    syscall
    testq %rax, %rax
    js .wof_err_pop
    movq %rax, %r15 # File Descriptor

    # 1. Write Header (16 bytes)
    movq $1, %rax # SYS_WRITE
    movq %r15, %rdi
    leaq morph_header(%rip), %rsi
    movq $16, %rdx
    syscall

    # 2. Write RPN Payload
    # Restore Payload Args
    popq %rdx # Restore Len
    popq %rsi # Restore Ptr

    # Write Payload
    movq $1, %rax # SYS_WRITE
    movq %r15, %rdi
    # %rsi (Ptr) and %rdx (Len) are set
    syscall

    # 3. Close File
    movq $3, %rax
    movq %r15, %rdi
    syscall
    ret

.wof_err_pop:
    popq %rdx
    popq %rsi
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

print_str_r13:
    movq %r13, %rdi
    call strlen
    movq %rax, %rdx
    movq $1, %rax
    movq $1, %rdi
    movq %r13, %rsi
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
