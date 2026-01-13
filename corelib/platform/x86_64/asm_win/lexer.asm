; corelib/platform/x86_64/asm_win/lexer.asm
; Implementasi Lexer Sederhana (Windows x86_64)
; Mengubah string input menjadi stream of tokens.

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .data
    ; Keyword Strings
    kw_fungsi       db "fungsi", 0
    kw_tutup_fungsi db "tutup_fungsi", 0
    kw_jika         db "jika", 0
    kw_tutup_jika   db "tutup_jika", 0
    kw_selama       db "selama", 0
    kw_tutup_selama db "tutup_selama", 0
    kw_lain         db "lain", 0
    kw_kembali      db "kembali", 0
    kw_var          db "var", 0
    kw_const        db "const", 0
    kw_ambil        db "ambil", 0
    kw_Ambil        db "Ambil", 0
    kw_indeks       db "indeks", 0
    kw_pilih        db "pilih", 0
    kw_tutup_pilih  db "tutup_pilih", 0
    kw_kasus        db "kasus", 0
    kw_sistem       db "sistem", 0

section .text
global lexer_create
global lexer_next_token
global lexer_peek_token
extern mem_alloc

TOKEN_EOF        equ 0
TOKEN_INTEGER    equ 1
TOKEN_IDENTIFIER equ 2
TOKEN_OPERATOR   equ 3
TOKEN_KEYWORD    equ 4
TOKEN_STRING     equ 5
TOKEN_DELIMITER  equ 6
TOKEN_MARKER     equ 7
TOKEN_SYSCALL    equ 20

; ------------------------------------------------------------------------------
; func lexer_create(input_ptr: ptr, input_len: i64) -> ptr
; Input: RCX, RDX
; ------------------------------------------------------------------------------
lexer_create:
    push rbp
    mov rbp, rsp
    push rdi        ; Save RDI (Non-Volatile)
    sub rsp, 32     ; Shadow

    push rcx ; Save Input Ptr
    push rdx ; Save Input Len

    ; Alloc 40 bytes
    mov rcx, 40
    call mem_alloc

    pop rdx
    pop rcx

    test rax, rax
    jz .fail

    mov [rax + 0], rcx  ; Input Ptr
    mov [rax + 8], rdx  ; Input Len
    mov qword [rax + 16], 0   ; Pos
    mov qword [rax + 24], 1   ; Line
    mov qword [rax + 32], 1   ; Col

.fail:
    add rsp, 32
    pop rdi
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func lexer_next_token(lexer: ptr) -> ptr
; Input: RCX
; ------------------------------------------------------------------------------
lexer_next_token:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 32

    mov r13, rcx ; Lexer Ptr

.skip_ws:
    ; Cek EOF
    mov rax, [r13 + 16] ; Pos
    cmp rax, [r13 + 8]  ; Len
    jge .return_eof

    ; Get Char
    mov rbx, [r13 + 0]  ; Input Base
    movzx rdx, byte [rbx + rax] ; Char

    cmp dl, 32
    je .is_ws
    cmp dl, 9
    je .is_ws
    cmp dl, 10
    je .is_ws_nl
    cmp dl, 13
    je .is_ws

    jmp .parse_token

.is_ws:
    inc qword [r13 + 16]
    inc qword [r13 + 32]
    jmp .skip_ws

.is_ws_nl:
    inc qword [r13 + 16]
    inc qword [r13 + 24]
    mov qword [r13 + 32], 1
    jmp .skip_ws

.return_eof:
    mov rcx, TOKEN_EOF
    xor rdx, rdx
    call .make_token
    jmp .done

; ------------------------------------------------------------------------------
; func lexer_peek_token(lexer: ptr) -> ptr
; ------------------------------------------------------------------------------
lexer_peek_token:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 32

    mov r12, rcx ; Lexer Ptr

    ; Save State
    mov r13, [r12 + 16]
    mov r14, [r12 + 24]
    mov r15, [r12 + 32]

    mov rcx, r12
    call lexer_next_token
    mov rbx, rax

    ; Restore State
    mov [r12 + 16], r13
    mov [r12 + 24], r14
    mov [r12 + 32], r15

    mov rax, rbx
    add rsp, 32
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

.parse_token:
    ; RDX = Char
    cmp dl, '"'
    je .parse_string
    cmp dl, 39      ; Single Quote '
    je .parse_string

    cmp dl, '0'
    jb .check_op
    cmp dl, '9'
    ja .check_alpha

    ; INT
    xor r8, r8

.int_loop:
    sub dl, '0'
    imul r8, 10
    add r8, rdx

    inc qword [r13 + 16]
    inc qword [r13 + 32]

    ; Peek
    mov rax, [r13 + 16]
    cmp rax, [r13 + 8]
    jge .int_done

    mov rbx, [r13 + 0]
    movzx rdx, byte [rbx + rax]

    cmp dl, '0'
    jb .int_done
    cmp dl, '9'
    ja .int_done

    jmp .int_loop

.int_done:
    mov rcx, TOKEN_INTEGER
    mov rdx, r8
    call .make_token
    jmp .done

.check_alpha:
    cmp dl, 'A'
    jb .check_op

    ; IDENTIFIER
    mov r12, [r13 + 16] ; Start Offset

.id_loop:
    inc qword [r13 + 16]
    inc qword [r13 + 32]

    mov rax, [r13 + 16]
    cmp rax, [r13 + 8]
    jge .id_done

    mov rbx, [r13 + 0]
    movzx rdx, byte [rbx + rax]

    cmp dl, 32
    je .id_done
    cmp dl, 10
    je .id_done
    cmp dl, 13
    je .id_done
    cmp dl, '+'
    je .id_done
    cmp dl, '-'
    je .id_done
    cmp dl, '('
    je .id_done
    cmp dl, ')'
    je .id_done
    cmp dl, ','
    je .id_done
    cmp dl, ':'
    je .id_done
    cmp dl, '#'
    je .id_done
    cmp dl, ';'
    je .id_done

    jmp .id_loop

.id_done:
    mov rcx, [r13 + 16]
    sub rcx, r12        ; Len

    mov rax, [r13 + 0]
    add rax, r12        ; Ptr

    ; Alloc String Struct
    push rax
    push rcx

    mov rcx, 16
    sub rsp, 32
    call mem_alloc
    add rsp, 32

    pop rdx ; Len
    pop r8  ; Ptr

    mov [rax + 0], rdx
    mov [rax + 8], r8

    ; --- KEYWORD CHECK ---
    mov rcx, rax
    push rax ; Save String Struct
    call .check_if_keyword
    mov rcx, rax ; Type (KEYWORD/IDENTIFIER)
    pop rdx      ; Value (String Struct Ptr)

    call .make_token
    jmp .done

.check_op:
    ; Cek Delimiter ((, ), ,)
    cmp dl, '('
    je .make_delim
    cmp dl, ')'
    je .make_delim
    cmp dl, ','
    je .make_delim
    cmp dl, ':'
    je .make_delim

    ; Cek Komentar (;)
    cmp dl, ';'
    je .check_comment

    ; Cek Marker (###)
    cmp dl, '#'
    je .check_marker

    ; Operator (+, -, *, /, =, ==, !=, <=, >=, ->)
    ; PEAK AHEAD
    mov rax, [r13 + 16] ; Current Pos
    inc rax             ; Next Pos
    cmp rax, [r13 + 8]  ; Check Bounds
    jge .single_char_op ; EOF reached

    mov rbx, [r13 + 0]  ; Input Base
    movzx rbx, byte [rbx + rax] ; Next Char in RBX

    ; Match Logic
    cmp dl, '='
    jne .try_bang
    cmp bl, '='
    je .make_eq_eq      ; '=='
    jmp .single_char_op

.try_bang:
    cmp dl, '!'
    jne .try_less
    cmp bl, '='
    je .make_ne_eq      ; '!='
    jmp .single_char_op

.try_less:
    cmp dl, 0x3C        ; '<'
    jne .try_greater
    cmp bl, '='
    je .make_le_eq      ; '<='
    jmp .single_char_op

.try_greater:
    cmp dl, '>'
    jne .try_minus
    cmp bl, '='
    je .make_ge_eq      ; '>='
    jmp .single_char_op

.try_minus:
    cmp dl, '-'
    jne .single_char_op
    cmp bl, '>'
    je .make_arrow      ; '->'
    jmp .single_char_op

.make_eq_eq:
    mov rdx, 0x3D3D
    jmp .advance_two
.make_ne_eq:
    mov rdx, 0x3D21
    jmp .advance_two
.make_le_eq:
    mov rdx, 0x3D3C
    jmp .advance_two
.make_ge_eq:
    mov rdx, 0x3D3E
    jmp .advance_two
.make_arrow:
    mov rdx, 0x3E2D
    jmp .advance_two

.advance_two:
    add qword [r13 + 16], 2
    add qword [r13 + 32], 2
    mov rcx, TOKEN_OPERATOR
    call .make_token
    jmp .done

.make_delim:
    inc qword [r13 + 16]
    inc qword [r13 + 32]
    mov rcx, TOKEN_DELIMITER
    movzx rdx, dl ; Value = ASCII
    call .make_token
    jmp .done

.single_char_op:
    inc qword [r13 + 16]
    inc qword [r13 + 32]
    mov rcx, TOKEN_OPERATOR
    movzx rdx, dl ; Value = ASCII code
    call .make_token
    jmp .done

; ------------------------------------------------------------------------------
; MARKER CHECK (###)
; ------------------------------------------------------------------------------
.check_marker:
    mov rax, [r13 + 16]
    add rax, 2
    cmp rax, [r13 + 8]
    jge .single_char_op

    mov rbx, [r13 + 0]

    ; Peek +1
    mov rax, [r13 + 16]
    inc rax
    movzx r8, byte [rbx + rax]
    cmp r8b, '#'
    jne .single_char_op

    ; Peek +2
    inc rax
    movzx r8, byte [rbx + rax]
    cmp r8b, '#'
    jne .single_char_op

    add qword [r13 + 16], 3
    add qword [r13 + 32], 3

    mov rcx, TOKEN_MARKER
    mov rdx, 0x232323
    call .make_token
    jmp .done

; ------------------------------------------------------------------------------
; COMMENT HANDLING
; ------------------------------------------------------------------------------
.check_comment:
.consume_comment:
    inc qword [r13 + 16]
    inc qword [r13 + 32]

    mov rax, [r13 + 16]
    cmp rax, [r13 + 8]
    jge .return_eof

    mov rbx, [r13 + 0]
    movzx rdx, byte [rbx + rax]
    cmp dl, 10
    je .is_ws_nl

    jmp .consume_comment

; ------------------------------------------------------------------------------
; STRING PARSING (Updated: Alloc+Copy+NullTerminate)
; ------------------------------------------------------------------------------
.parse_string:
    mov r11b, dl ; Quote Char

    inc qword [r13 + 16]
    inc qword [r13 + 32]

    mov r12, [r13 + 16] ; Start Content Offset

.parse_str_loop:
    mov rax, [r13 + 16]
    cmp rax, [r13 + 8]
    jge .str_error_eof

    mov rbx, [r13 + 0]
    movzx rdx, byte [rbx + rax]

    cmp dl, r11b
    je .str_done

    inc qword [r13 + 16]
    inc qword [r13 + 32]
    jmp .parse_str_loop

.str_done:
    mov rcx, [r13 + 16]
    sub rcx, r12        ; Len (Content)

    inc qword [r13 + 16]
    inc qword [r13 + 32]

    mov rax, [r13 + 0]
    add rax, r12        ; Input Content Ptr

    ; Alloc New Buffer (Len + 1)
    push rax ; Save Input Ptr
    push rcx ; Save Len

    inc rcx ; Len + 1
    sub rsp, 32
    call mem_alloc
    add rsp, 32

    mov r14, rax ; New Buffer

    pop rcx ; Restore Len
    pop rsi ; Restore Input Ptr (RSI for copy)

    ; Copy
    mov rdi, r14 ; Dest
    mov r8, rcx  ; Len

    push rcx ; Save Len for Struct
    push r14 ; Save Buf Ptr

    ; Manual Copy Loop (Win64 ABI doesn't guarantee rdi/rsi preserved in memcpy wrapper?)
    ; I'll use manual loop to be safe.

    xor r9, r9
.ps_copy:
    test r8, r8
    jz .ps_copy_done
    mov al, [rsi + r9]
    mov [rdi + r9], al
    inc r9
    dec r8
    jmp .ps_copy

.ps_copy_done:
    mov byte [rdi + r9], 0 ; Null Terminate

    ; Alloc String Struct (16 bytes)
    mov rcx, 16
    sub rsp, 32
    call mem_alloc
    add rsp, 32

    pop r8  ; Buf Ptr
    pop rdx ; Len

    mov [rax + 0], rdx
    mov [rax + 8], r8

    mov rcx, TOKEN_STRING
    mov rdx, rax
    call .make_token
    jmp .done

.str_error_eof:
    jmp .return_eof

.done:
    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; .make_token(type, val) -> ptr
; Input: RCX, RDX
.make_token:
    push rbx
    push r12
    mov rbx, rcx
    mov r12, rdx

    mov rcx, 32
    sub rsp, 32
    call mem_alloc
    add rsp, 32

    mov [rax + 0], rbx ; Type
    mov [rax + 8], r12 ; Val

    mov rcx, [r13 + 24]
    mov [rax + 16], rcx
    mov rcx, [r13 + 32]
    mov [rax + 24], rcx

    pop r12
    pop rbx
    ret

; ------------------------------------------------------------------------------
; Internal: .check_if_keyword(str_struct_ptr: ptr) -> type (i64)
; Returns TOKEN_KEYWORD or TOKEN_IDENTIFIER
; ------------------------------------------------------------------------------
.check_if_keyword:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    sub rsp, 32

    mov r12, rcx ; String Struct [Len][Data]
    mov rcx, [r12 + 0] ; Len Input
    mov rdx, [r12 + 8] ; Data Input

    ; Switch on Length
    cmp rcx, 3
    je .len_3
    cmp rcx, 4
    je .len_4
    cmp rcx, 5
    je .len_5
    cmp rcx, 6
    je .len_6
    cmp rcx, 7
    je .len_7
    cmp rcx, 10
    je .len_10
    cmp rcx, 11
    je .len_11
    cmp rcx, 12
    je .len_12

    jmp .not_kw

.len_3:
    lea r8, [rel kw_var]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw
    jmp .not_kw

.len_4:
    lea r8, [rel kw_jika]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw

    lea r8, [rel kw_lain]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw
    jmp .not_kw

.len_5:
    lea r8, [rel kw_const]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw

    lea r8, [rel kw_ambil]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw

    lea r8, [rel kw_Ambil]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw

    lea r8, [rel kw_pilih]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw

    lea r8, [rel kw_kasus]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw
    jmp .not_kw

.len_6:
    lea r8, [rel kw_fungsi]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw

    lea r8, [rel kw_selama]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw

    lea r8, [rel kw_indeks]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw

    lea r8, [rel kw_sistem]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw
    jmp .not_kw

.len_7:
    lea r8, [rel kw_kembali]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw
    jmp .not_kw

.len_10:
    lea r8, [rel kw_tutup_jika]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw
    jmp .not_kw

.len_11:
    lea r8, [rel kw_tutup_pilih]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw
    jmp .not_kw

.len_12:
    lea r8, [rel kw_tutup_fungsi]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw

    lea r8, [rel kw_tutup_selama]
    call .cmp_kw_str
    test rax, rax
    jz .found_kw
    jmp .not_kw

.found_kw:
    mov rax, TOKEN_KEYWORD
    jmp .ck_ret

.not_kw:
    mov rax, TOKEN_IDENTIFIER

.ck_ret:
    add rsp, 32
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; Helper: compare string in RDX (input) and R8 (kw), length in RCX
; returns 0 if match, 1 if not
.cmp_kw_str:
    push rsi
    push rdi
    push rcx
    push r8
    push rdx

    mov rsi, rdx ; Input
    mov rdi, r8  ; Keyword
    ; RCX is count

    xor rax, rax ; Assume match (0)
.cks_loop:
    test rcx, rcx
    jz .cks_done

    mov r9b, [rsi]
    cmp byte [rdi], r9b
    jne .cks_fail

    inc rsi
    inc rdi
    dec rcx
    jmp .cks_loop

.cks_fail:
    mov rax, 1

.cks_done:
    pop rdx
    pop r8
    pop rcx
    pop rdi
    pop rsi
    ret
