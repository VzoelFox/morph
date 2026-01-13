; corelib/platform/x86_64/asm_win/test_string.asm
; Test Suite untuk Operasi String (Windows)

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
str_hello db "hello"
str_hello_len dq 5
str_world db "world"
str_world_len dq 5
str_hello2 db "hello"
expected_hash_hello dq 0xa430d84680aabd0b

section .text
global main
extern __mf_string_hash
extern __mf_string_equals
extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    ; TEST 1: Hash Calculation
    lea rcx, [rel str_hello]
    mov rdx, [rel str_hello_len]
    call __mf_string_hash

    cmp rax, [rel expected_hash_hello]
    jne .fail

    ; TEST 2: Equality (Same String)
    lea rcx, [rel str_hello]
    mov rdx, [rel str_hello_len]
    lea r8, [rel str_hello2]
    mov r9, [rel str_hello_len]
    call __mf_string_equals

    cmp rax, 1
    jne .fail

    ; TEST 3: Inequality (Different Content)
    lea rcx, [rel str_hello]
    mov rdx, [rel str_hello_len]
    lea r8, [rel str_world]
    mov r9, [rel str_world_len]
    call __mf_string_equals

    cmp rax, 0
    jne .fail

    ; Success
    xor rax, rax
    add rsp, 32
    pop rbp
    ret

.fail:
    mov rax, 1
    add rsp, 32
    pop rbp
    ret
