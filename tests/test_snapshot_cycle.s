# tests/test_snapshot_cycle.s
# Test Case: Verifikasi Siklus Snapshot (Save -> Reset -> Recover)
# Skenario:
# 1. Alokasi memori dan tulis data ("MAGIC_DATA").
# 2. Simpan Snapshot.
# 3. Reset Memory (Unmap semua).
# 4. Recover Snapshot.
# 5. Baca alamat lama, verifikasi data masih ada.

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
msg_start:   .asciz "[Test] Starting Snapshot Cycle...\n"
msg_check:   .asciz "[Test] Checking data integrity...\n"
msg_ok:      .asciz "[Test] SUCCESS: Data preserved after recovery!\n"
msg_fail:    .asciz "[Test] FAIL: Data corrupted or lost!\n"
data_magic:  .asciz "MAGIC_DATA_12345"

.section .bss
    .lcomm saved_ptr, 8

.section .text
.global main
.extern __mf_runtime_init
.extern mem_alloc
.extern mem_snapshot_save
.extern mem_reset
.extern mem_snapshot_recover
.extern __mf_print_asciz
.extern __mf_string_equals

main:
    call __mf_runtime_init

    leaq msg_start(%rip), %rdi
    call __mf_print_asciz

    # 1. Alloc & Write
    movq $32, %rdi
    call mem_alloc
    movq %rax, saved_ptr(%rip) # Simpan pointer

    # Tulis data
    leaq data_magic(%rip), %rsi
    movq %rax, %rdi
    call strcpy_test

    # 2. Save
    call mem_snapshot_save

    # 3. Reset
    # PENTING: Reset akan unmap page tempat 'saved_ptr' berada jika itu di heap?
    # Tidak, 'saved_ptr' ada di .bss section (Global), aman.
    # Tapi yang di-unmap adalah Heap Pages.
    call mem_reset

    # 4. Recover
    call mem_snapshot_recover

    # 5. Check
    leaq msg_check(%rip), %rdi
    call __mf_print_asciz

    movq saved_ptr(%rip), %rdi
    leaq data_magic(%rip), %rsi
    call streq_test

    testq %rax, %rax
    jz .fail

    leaq msg_ok(%rip), %rdi
    call __mf_print_asciz

    movq $60, %rax
    xorq %rdi, %rdi
    syscall

.fail:
    leaq msg_fail(%rip), %rdi
    call __mf_print_asciz
    movq $60, %rax
    movq $1, %rdi
    syscall

# Helpers
strcpy_test:
    movb (%rsi), %al
    movb %al, (%rdi)
    testb %al, %al
    jz .cpy_done
    incq %rsi
    incq %rdi
    jmp strcpy_test
.cpy_done:
    ret

streq_test: # rdi, rsi strings. ret 1 if eq, 0 if not
    xorq %rax, %rax
.eq_loop:
    movb (%rdi), %al
    movb (%rsi), %bl
    cmpb %al, %bl
    jne .eq_fail
    testb %al, %al
    jz .eq_pass
    incq %rdi
    incq %rsi
    jmp .eq_loop
.eq_pass:
    movq $1, %rax
    ret
.eq_fail:
    xorq %rax, %rax
    ret
