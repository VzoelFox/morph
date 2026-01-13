# corelib/platform/x86_64/asm/hello.s
# Program Demo "Hello World" menggunakan Runtime MorphFox
# Tujuan: Membuktikan bahwa kontrak ABI dan Runtime Adapter berfungsi.

.section .data
msg:
    .ascii "MorphFox Hidup!\n"
    .set len, . - msg

.section .text
.global main

# Fungsi 'main' ini akan dipanggil oleh _start di runtime.s
main:
    # Siapkan argumen untuk __sys_write
    # Arg 1: fd = 1 (stdout)
    mov $1, %rdi

    # Arg 2: buffer = alamat msg
    lea msg(%rip), %rsi

    # Arg 3: length = len
    mov $len, %rdx

    # Panggil fungsi runtime
    call __sys_write

    # Return 0 (Sukses)
    mov $0, %rax
    ret
