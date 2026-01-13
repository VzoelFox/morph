; ==============================================================================
; RPN INSTRUCTION SET (SSOT) - NASM Syntax
; ==============================================================================

; KATEGORI MASK
%define OPCODE_CAT_DATA   0
%define OPCODE_CAT_ARITH  10
%define OPCODE_CAT_LOGIC  20
%define OPCODE_CAT_CTRL   30
%define OPCODE_CAT_SYS    40
%define OPCODE_CAT_DEBUG  90

; ------------------------------------------------------------------------------
; DATA & STORAGE (0-9)
; ------------------------------------------------------------------------------
%define OP_LIT        1   ; Push literal
%define OP_LOAD       2   ; Load var
%define OP_STORE      3   ; Store var
%define OP_DUP        4   ; Duplicate Top
%define OP_POP        5   ; Pop (Discard)
%define OP_PICK       6   ; Pick item at depth N
%define OP_POKE       7   ; Poke item at depth N

; ------------------------------------------------------------------------------
; ARITHMETIC (10-19)
; ------------------------------------------------------------------------------
%define OP_ADD        10
%define OP_SUB        11
%define OP_MUL        12
%define OP_DIV        13
%define OP_MOD        14

; ------------------------------------------------------------------------------
; LOGIC & COMPARISON (20-29)
; ------------------------------------------------------------------------------
%define OP_EQ         20
%define OP_NEQ        21
%define OP_LT         22
%define OP_GT         23
%define OP_LEQ        24
%define OP_GEQ        25

; ------------------------------------------------------------------------------
; CONTROL FLOW (30-39)
; ------------------------------------------------------------------------------
%define OP_JMP        30
%define OP_JMP_IF     31
%define OP_JMP_FALSE  32
%define OP_CALL       33
%define OP_RET        34
%define OP_EXIT       35
%define OP_LABEL      36
%define OP_SWITCH     37
%define OP_CASE       38
%define OP_CYCLE      39

; ------------------------------------------------------------------------------
; SYSTEM INTERFACE (40-49)
; ------------------------------------------------------------------------------
%define OP_SYSCALL    40

; ------------------------------------------------------------------------------
; MEMORY ACCESS (45-49)
; ------------------------------------------------------------------------------
%define OP_MEM_READ   45
%define OP_MEM_WRITE  46

; ------------------------------------------------------------------------------
; CONCURRENCY (50-59)
; ------------------------------------------------------------------------------
%define OP_SPAWN      50
%define OP_YIELD      51

; ------------------------------------------------------------------------------
; DEBUG & META (90-99)
; ------------------------------------------------------------------------------
%define OP_PRINT      90
%define OP_DUMP       91
%define OP_HINT       99
