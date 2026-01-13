# corelib/platform/x86_64/asm/test_registers.s
# Test Suite: Infinite Registers (PICK/POKE)
#
# Scenario:
# 1. Push 10 (Var A)
# 2. Push 20 (Var B)
# 3. PICK 1 (Read Var A -> Should be 10) -> Stack: [10, 20, 10]
# 4. LIT 99
# 5. LIT 2 (Depth of Var A now: Top=99, Depth1=10(Pick result), Depth2=20, Depth3=10(Var A))
#    Wait, stack grows downwards (Top is Low Addr).
#    Stack:
#    [0] Top
#    [1] ...
#    Let's trace carefully.
#
#    Init: []
#    LIT 10 -> [10]
#    LIT 20 -> [10, 20] (Top=20)
#    PICK 1 -> Read 10. Push 10. Stack: [10, 20, 10] (Top=10)
#    LIT 99 -> Stack: [10, 20, 10, 99] (Top=99)
#    LIT 3  -> Stack: [10, 20, 10, 99, 3] (Top=3) -> Depth to Var A (10) is 3?
#              Offset 0: 3
#              Offset 1: 99
#              Offset 2: 10
#              Offset 3: 20
#              Offset 4: 10 (Target)
#    POKE 4 -> Write 99 to Depth 4.
#    Result Stack: [99, 20, 10, 99] (Top=99, popped args)
#
#    Let's simplify.
#    [10] (Var A)
#    [20] (Var B)
#    LIT 99 (New Val)
#    LIT 1 (Depth of Var B is 1? No. Stack: [10, 20, 99, 1]. Top=1.
#           Depth 0: 1. Depth 1: 99. Depth 2: 20. Depth 3: 10.)
#    POKE 2 -> Target Var B (20).
#    Stack after POKE: [10, 99]
#    Verify Top is 99.

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/rpn_gas.inc"

.section .data
code_seq:
    .quad OP_LIT, 10    # [10]
    .quad OP_LIT, 20    # [10, 20]

    # Test POKE (Overwrite Var A at Depth 1)
    .quad OP_LIT, 99    # [10, 20, 99]
    .quad OP_LIT, 1     # [10, 20, 99, 1]
    .quad OP_POKE, 0    # Pops 1, Pops 99. Writes 99 to Depth 1 (Slot 10).
                        # Stack: [99, 20] (Logical)

    # Verify by PICKing Depth 1 (Var A)
    .quad OP_LIT, 1     # [99, 20, 1]
    .quad OP_PICK, 0    # Pops 1. Picks Depth 1 (99).
    .quad OP_EXIT, 0

code_end:
    .quad 0

.section .text
.global main
.extern executor_run_with_stack
.extern stack_new
.extern __mf_print_int
.extern __sys_exit
.extern __mf_runtime_init

main:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx          # Preserve RBX

    # 1. Create Data Stack
    movq $4096, %rdi
    call stack_new
    testq %rax, %rax
    jz .fail
    movq %rax, %rbx     # Save Stack Ptr in RBX (Callee-saved)

    # 2. Prepare Fragment
    # Stack alignment: We are at 16-byte align (pushed RBP).
    # Need to alloc struct (32 bytes?).
    subq $32, %rsp

    leaq code_seq(%rip), %rax
    movq %rax, 8(%rsp)  # Code Ptr
    movq $100, 16(%rsp) # Size

    # 3. Run Executor
    movq %rsp, %rdi     # Fragment Ptr
    movq %rbx, %rsi     # Stack Ptr
    call executor_run_with_stack

    addq $32, %rsp      # Restore Stack

    # 4. Check Result (Top should be 99)
    movq %rbx, %rsi     # Stack Ptr

    # Pop Result
    movq %rsi, %rdi
    call stack_pop

    # Print Result
    movq %rax, %rdi
    call __mf_print_int # Should be 99

    cmpq $99, %rax
    jne .fail

    # Exit 0
    movq $0, %rdi
    call __sys_exit

.fail:
    popq %rbx
    movq $1, %rdi
    call __sys_exit
