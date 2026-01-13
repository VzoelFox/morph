# corelib/platform/x86_64/asm/test_dom.s
# Test Program: Manual DOM Construction (Structure Verification)
# Creates: Document -> DIV -> P

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
tag_div: .asciz "div"
tag_p:   .asciz "p"
txt_val: .asciz "Hello MorphWeb"
msg_ok:  .asciz "[DOM] Structure Verified: Root -> Div -> P\n"
msg_err: .asciz "[DOM] Verification Failed!\n"

# Offsets (Match SSOT)
.equ DOM_OFF_TYPE, 0
.equ DOM_OFF_PARENT, 8
.equ DOM_OFF_CHILD, 16
.equ DOM_OFF_NEXT, 24
.equ DOM_OFF_TAG, 32
.equ DOM_OFF_ATTR, 40

.section .text
.global main
.extern __mf_runtime_init
.extern mem_alloc
.extern __mf_print_asciz
.extern __sys_exit

main:
    call __mf_runtime_init

    # 1. Alloc Root (Document)
    movq $64, %rdi
    call mem_alloc
    movq %rax, %r12 # R12 = Root
    movq $1, DOM_OFF_TYPE(%r12) # DOC

    # 2. Alloc Element (DIV)
    movq $64, %rdi
    call mem_alloc
    movq %rax, %r13 # R13 = DIV
    movq $2, DOM_OFF_TYPE(%r13) # ELEMENT
    leaq tag_div(%rip), %rcx
    movq %rcx, DOM_OFF_TAG(%r13)

    # Link Root -> DIV
    movq %r13, DOM_OFF_CHILD(%r12)
    movq %r12, DOM_OFF_PARENT(%r13)

    # 3. Alloc Element (P)
    movq $64, %rdi
    call mem_alloc
    movq %rax, %r14 # R14 = P
    movq $2, DOM_OFF_TYPE(%r14) # ELEMENT
    leaq tag_p(%rip), %rcx
    movq %rcx, DOM_OFF_TAG(%r14)

    # Link DIV -> P
    movq %r14, DOM_OFF_CHILD(%r13)
    movq %r13, DOM_OFF_PARENT(%r14)

    # 4. Verify Structure
    # Root->Child == DIV ?
    movq DOM_OFF_CHILD(%r12), %rax
    cmpq %r13, %rax
    jne .fail

    # DIV->Parent == Root ?
    movq DOM_OFF_PARENT(%r13), %rax
    cmpq %r12, %rax
    jne .fail

    # DIV->Child == P ?
    movq DOM_OFF_CHILD(%r13), %rax
    cmpq %r14, %rax
    jne .fail

    # P->Parent == DIV ?
    movq DOM_OFF_PARENT(%r14), %rax
    cmpq %r13, %rax
    jne .fail

    leaq msg_ok(%rip), %rdi
    call __mf_print_asciz

    movq $0, %rdi
    call __sys_exit

.fail:
    leaq msg_err(%rip), %rdi
    call __mf_print_asciz
    movq $1, %rdi
    call __sys_exit
