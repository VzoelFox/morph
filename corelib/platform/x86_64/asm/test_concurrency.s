# corelib/platform/x86_64/asm/test_concurrency.s
# Test Suite: Concurrency (Spawn & Yield)
# Scenario: Main spawns Routine B. Both yield cooperatively to print interleaved messages.
# Output should be: 1, 2, 3, 4 (representing A1, B1, A2, B2)

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/rpn_gas.inc"

.section .data

    # Define Fragment Struct globally so Executor can recover it
    .global fragment_struct
    fragment_struct:
        .quad 1             # ID
        .quad code_buffer   # Code Ptr
        .quad code_end - code_buffer # Size
        .quad 0             # Next

    # Code Buffer containing RPN instructions
    code_buffer:
        # --- ENTRY (Offset 0) ---
        # MAIN ROUTINE (A)
        # Spawn Routine B (Jump to Offset 144 / Label B)
        .quad OP_SPAWN, 144 # Arg1 for Trampoline (Offset)

        # Print "1" (A1)
        .quad OP_LIT, 1
        .quad OP_PRINT, 0

        # Yield to B
        .quad OP_YIELD, 0

        # Print "3" (A2)
        .quad OP_LIT, 3
        .quad OP_PRINT, 0

        # Yield (Just to be safe)
        .quad OP_YIELD, 0

        # Exit Main
        .quad OP_LIT, 0
        .quad OP_EXIT, 0

        # --- ROUTINE B (Offset 128) ---
        # Label B starts here (8 instructions * 16 bytes = 128)

        # Print "2" (B1)
        .quad OP_LIT, 2
        .quad OP_PRINT, 0

        # Yield back to A
        .quad OP_YIELD, 0

        # Print "4" (B2)
        .quad OP_LIT, 4
        .quad OP_PRINT, 0

        # Exit Routine B (Terminates thread)
        .quad OP_LIT, 0
        .quad OP_EXIT, 0

        # Offset 144 + 96 = 240
    code_end:

.text
.global main
.extern __mf_runtime_init
.extern scheduler_init
.extern stack_new
.extern executor_run_with_stack
.extern OS_EXIT
.extern executor_exec_label

main:
    # 1. Runtime Init (Monitor, etc)
    call __mf_runtime_init

    # 2. Scheduler Init (Initialize Main Routine as #0)
    call scheduler_init

    # 3. Create Main Stack
    movq $8192, %rdi
    call stack_new
    movq %rax, %rbx     # RBX = Stack Ptr

    # 4. Run Executor (Main Routine)
    leaq fragment_struct(%rip), %rdi
    movq %rbx, %rsi
    call executor_run_with_stack

    # 5. Exit Process
    # OS_EXIT is a macro, not a function symbol. We must use sys_exit directly or via a wrapper.
    # But builtins.s usually exports __sys_exit.
    movq $0, %rdi
    call __sys_exit
