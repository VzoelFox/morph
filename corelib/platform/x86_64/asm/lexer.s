# corelib/platform/x86_64/asm/lexer.s
# Implementasi Lexer Sederhana (Linux x86_64)
# Mengubah string input menjadi stream of tokens.

.include "corelib/platform/x86_64/asm/macros.inc"

.section .text
.global lexer_create
.global lexer_next_token
.global lexer_peek_token

# Konstanta Tipe Token
.equ TOKEN_EOF, 0
.equ TOKEN_INTEGER, 1
.equ TOKEN_IDENTIFIER, 2
.equ TOKEN_OPERATOR, 3
.equ TOKEN_KEYWORD, 4
.equ TOKEN_STRING, 5
.equ TOKEN_DELIMITER, 6
.equ TOKEN_COLON, 7
.equ TOKEN_MARKER, 8

.section .rodata
# Keyword Strings
kw_fungsi:       .asciz "fungsi"
kw_tutup_fungsi: .asciz "tutup_fungsi"
kw_jika:         .asciz "jika"
kw_tutup_jika:   .asciz "tutup_jika"
kw_selama:       .asciz "selama"
kw_tutup_selama: .asciz "tutup_selama"
kw_lain:         .asciz "lain"
kw_kembali:      .asciz "kembali"
kw_var:          .asciz "var"
kw_const:        .asciz "const"
kw_ambil:        .asciz "Ambil"
kw_ambil_lc:     .asciz "ambil"
kw_indeks:       .asciz "indeks"
kw_pilih:        .asciz "pilih"
kw_tutup_pilih:  .asciz "tutup_pilih"
kw_kasus:        .asciz "kasus"
kw_sistem:       .asciz "sistem"

# Struktur Lexer State (Disimpan di Heap)
# [00] Input Ptr (Start)
# [08] Input Len
# [16] Current Pos (Offset)
# [24] Line (Current)
# [32] Column (Current) -> Total 40 bytes

.section .text

# ------------------------------------------------------------------------------
# func lexer_create(input_ptr: ptr, input_len: i64) -> ptr
# ------------------------------------------------------------------------------
lexer_create:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13

    movq %rdi, %r12 # Input Ptr
    movq %rsi, %r13 # Input Len

    # Alloc Lexer State (40 bytes)
    movq $40, %rdi
    call mem_alloc

    testq %rax, %rax
    jz .create_fail

    # Init State
    movq %r12, 0(%rax)  # Input Ptr
    movq %r13, 8(%rax)  # Input Len
    movq $0, 16(%rax)   # Current Pos
    movq $1, 24(%rax)   # Line
    movq $1, 32(%rax)   # Col

.create_fail:
    popq %r13
    popq %r12
    leave
    ret

# ------------------------------------------------------------------------------
# func lexer_next_token(lexer: ptr) -> ptr (Token Struct)
# ------------------------------------------------------------------------------
lexer_next_token:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13 # Lexer Ptr

    movq %rdi, %r13

    # 1. Skip Whitespace
.skip_ws:
    # Cek EOF
    movq 16(%r13), %rcx # Pos
    cmpq 8(%r13), %rcx  # Len
    jge .return_eof

    # Ambil Char
    movq 0(%r13), %rax  # Input Base
    movzbq (%rax, %rcx), %rdx # Char

    # Cek Spasi (32), Tab (9), Newline (10), CR (13)
    cmpb $32, %dl
    je .is_ws
    cmpb $9, %dl
    je .is_ws
    cmpb $10, %dl
    je .is_ws_nl
    cmpb $13, %dl
    je .is_ws

    jmp .parse_token # Bukan WS

.is_ws:
    incq 16(%r13)       # Pos++
    incq 32(%r13)       # Col++
    jmp .skip_ws

.is_ws_nl:
    incq 16(%r13)       # Pos++
    incq 24(%r13)       # Line++
    movq $1, 32(%r13)   # Col=1
    jmp .skip_ws

    # --------------------------------------------------------------------------
    # COMMENT HANDLING (Start with ';')
    # --------------------------------------------------------------------------
.check_comment:
    # Dipanggil dari loop utama jika char == ';'
    # Loop consume sampai ketemu '\n' atau EOF
.consume_comment:
    incq 16(%r13)       # Pos++
    incq 32(%r13)       # Col++

    # Cek EOF
    movq 16(%r13), %rcx
    cmpq 8(%r13), %rcx
    jge .return_eof

    # Cek Newline
    movq 0(%r13), %rax
    movzbq (%rax, %rcx), %rdx
    cmpb $10, %dl       # \n
    je .is_ws_nl        # Handle newline (inc Line, reset Col) -> back to skip_ws

    jmp .consume_comment

.return_eof:
    # Buat Token EOF
    movq $0, %rdi # Type EOF
    movq $0, %rsi # Val 0
    call .make_token
    jmp .done

# ------------------------------------------------------------------------------
# func lexer_peek_token(lexer: ptr) -> ptr (Token Struct)
# Returns the next token without advancing state.
# Uses a "lookahead" buffer or saves state/restores.
# Simple implementation: Save state, call next, restore state.
# ------------------------------------------------------------------------------
lexer_peek_token:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rbx

    movq %rdi, %r12 # Lexer Ptr

    # Save State [16] Pos, [24] Line, [32] Col
    movq 16(%r12), %r13 # Pos
    movq 24(%r12), %r14 # Line
    movq 32(%r12), %r15 # Col

    # Call Next
    movq %r12, %rdi
    call lexer_next_token
    movq %rax, %rbx # Result Token

    # Restore State
    movq %r13, 16(%r12)
    movq %r14, 24(%r12)
    movq %r15, 32(%r12)

    movq %rbx, %rax

    popq %rbx
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    leave
    ret

.parse_token:
    # RDX = Current Char
    # 2. Cek Digit (0-9)
    cmpb $'0', %dl
    jb .check_op
    cmpb $'9', %dl
    ja .check_alpha

    # ---> PARSE INTEGER
    # Loop ambil digit
    xorq %r8, %r8 # Result Value

.parse_int_loop:
    subq $'0', %rdx
    imulq $10, %r8
    addq %rdx, %r8

    # Advance
    incq 16(%r13)
    incq 32(%r13)

    # Peek Next
    movq 16(%r13), %rcx
    cmpq 8(%r13), %rcx
    jge .int_done

    movq 0(%r13), %rax
    movzbq (%rax, %rcx), %rdx

    cmpb $'0', %dl
    jb .int_done
    cmpb $'9', %dl
    ja .int_done

    jmp .parse_int_loop

.int_done:
    movq $TOKEN_INTEGER, %rdi
    movq %r8, %rsi
    call .make_token
    jmp .done

.check_alpha:
    # 3. Cek Identifier atau String
    # Jika diawali " atau ' -> String Literal
    cmpb $'"', %dl
    je .parse_string
    cmpb $39, %dl   # Single Quote '
    je .parse_string

    # Cek Identifier (a-z, A-Z, _)
    # Simple Alpha Check:
    # (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'
    cmpb $'A', %dl
    jb .check_op # Mungkin simbol lain

    # ---> PARSE IDENTIFIER
    # Kita perlu copy stringnya.
    # Mark Start Pos
    movq 16(%r13), %rbx # Start Offset

.parse_id_loop:
    incq 16(%r13)
    incq 32(%r13)

    # Peek Next
    movq 16(%r13), %rcx
    cmpq 8(%r13), %rcx
    jge .id_done

    movq 0(%r13), %rax
    movzbq (%rax, %rcx), %rdx

    # Valid ID char? (AlphaNum + _)
    # Simplifikasi: Selama bukan spasi dan bukan OP/Delimiter
    cmpb $32, %dl
    je .id_done
    cmpb $10, %dl
    je .id_done
    # Cek op? +, -, =, etc.
    cmpb $'+', %dl
    je .id_done
    cmpb $'-', %dl
    je .id_done
    cmpb $':', %dl
    je .id_done
    # Cek delimiters
    cmpb $'(', %dl
    je .id_done
    cmpb $')', %dl
    je .id_done
    cmpb $',', %dl
    je .id_done
    # ...

    jmp .parse_id_loop

.id_done:
    # Hitung Len
    movq 16(%r13), %rcx # End
    subq %rbx, %rcx     # Len = End - Start

    # Alloc String Struct & Buffer
    # Buffer = InputBase + Start
    movq 0(%r13), %rax
    addq %rbx, %rax     # Ptr to start of ID in input

    # Copy to new String Struct?
    pushq %rax # Save Ptr
    pushq %rcx # Save Len

    movq $16, %rdi
    call mem_alloc # Alloc String Struct

    testq %rax, %rax
    jz .create_fail # Fail safe

    popq %rcx
    popq %rdx # Ptr

    movq %rcx, 0(%rax) # Len
    movq %rdx, 8(%rax) # Data Ptr

    # --- KEYWORD CHECK ---
    pushq %rax # Save String Struct Ptr

    movq %rax, %rdi
    call .check_if_keyword

    movq %rax, %rdi # Type
    popq %rsi       # Value (String Struct Ptr) is restored to RSI

    call .make_token
    jmp .done

.parse_string:
    # ---> PARSE STRING LITERAL
    # RDX holds quote char (" or ')
    movb %dl, %r11b     # Simpan Quote Char di R11

    # Advance past opening quote
    incq 16(%r13)
    incq 32(%r13)

    movq 16(%r13), %rbx # Start Content Offset

.parse_str_loop:
    # Cek EOF
    movq 16(%r13), %rcx
    cmpq 8(%r13), %rcx
    jge .str_error_eof

    # Get Char
    movq 0(%r13), %rax
    movzbq (%rax, %rcx), %rdx

    # Cek Closing Quote
    cmpb %r11b, %dl
    je .str_done

    # Advance
    incq 16(%r13)
    incq 32(%r13)
    jmp .parse_str_loop

.str_done:
    # Hitung Len Content (Tanpa Quote)
    movq 16(%r13), %rcx # End (at quote)
    subq %rbx, %rcx     # Len

    # Advance past closing quote
    incq 16(%r13)
    incq 32(%r13)

    # Alloc String Struct
    # Content = InputBase + Start
    movq 0(%r13), %rax
    addq %rbx, %rax     # Ptr to start of Content in Input

    pushq %rax # Save Input Ptr
    pushq %rcx # Save Len

    # Alloc New Buffer (Len + 1) for Null Term
    movq %rcx, %rdi
    incq %rdi
    call mem_alloc
    movq %rax, %r14 # New Buffer Ptr

    # Restore
    popq %rcx # Len
    popq %rsi # Input Ptr

    # Copy Loop
    pushq %rcx
    pushq %rsi
    pushq %r14
    movq %rcx, %r8
    xorq %r9, %r9
.ps_copy:
    testq %r8, %r8
    jz .ps_copy_done
    movb (%rsi, %r9, 1), %al
    movb %al, (%r14, %r9, 1)
    incq %r9
    decq %r8
    jmp .ps_copy
.ps_copy_done:
    movb $0, (%r14, %r9, 1) # Null Terminate
    popq %r14
    popq %rsi
    popq %rcx

    # Alloc String Struct
    pushq %rcx
    pushq %r14
    movq $16, %rdi
    call mem_alloc
    popq %r14
    popq %rcx

    movq %rcx, 0(%rax) # Len
    movq %r14, 8(%rax) # Data Ptr (New Buffer)

    movq $TOKEN_STRING, %rdi # Type 5
    movq %rax, %rsi # Value = String Struct Ptr
    call .make_token
    jmp .done

.str_error_eof:
    jmp .return_eof

.check_op:
    # 4. Delimiters ((, ), ,) & Operators
    # RDX holds current char

    cmpb $'(', %dl
    je .make_delim
    cmpb $')', %dl
    je .make_delim
    cmpb $',', %dl
    je .make_delim
    cmpb $':', %dl
    je .make_colon

    # Cek Komentar (;)
    cmpb $';', %dl
    je .check_comment

    # Cek Marker (###)
    cmpb $'#', %dl
    je .check_marker

    # Operator (+, -, *, /, =, ==, !=, <=, >=, ->)

    # PEAK AHEAD
    movq 16(%r13), %rcx # Current Pos
    incq %rcx           # Next Pos
    cmpq 8(%r13), %rcx  # Check Bounds
    jge .single_char_op # EOF reached

    movq 0(%r13), %rax  # Input Base
    movzbq (%rax, %rcx), %rbx # Next Char in RBX

    # Match Logic
    cmpb $'=', %dl
    jne .try_bang
    cmpb $'=', %bl
    je .make_eq_eq      # '=='
    jmp .single_char_op

.try_bang:
    cmpb $'!', %dl
    jne .try_less
    cmpb $'=', %bl
    je .make_ne_eq      # '!='
    jmp .single_char_op

.try_less:
    cmpb $0x3C, %dl     # ASCII '<'
    jne .try_greater
    cmpb $'=', %bl
    je .make_le_eq      # '<='
    jmp .single_char_op

.try_greater:
    cmpb $'>', %dl
    jne .try_minus
    cmpb $'=', %bl
    je .make_ge_eq      # '>='
    jmp .single_char_op

.try_minus:
    cmpb $'-', %dl
    jne .single_char_op
    cmpb $'>', %bl
    je .make_arrow      # '->'
    jmp .single_char_op

.make_eq_eq:
    movq $0x3D3D, %rsi
    jmp .advance_two
.make_ne_eq:
    movq $0x3D21, %rsi
    jmp .advance_two
.make_le_eq:
    movq $0x3D3C, %rsi
    jmp .advance_two
.make_ge_eq:
    movq $0x3D3E, %rsi
    jmp .advance_two
.make_arrow:
    movq $0x3E2D, %rsi
    jmp .advance_two

.advance_two:
    addq $2, 16(%r13)
    addq $2, 32(%r13)
    movq $TOKEN_OPERATOR, %rdi
    call .make_token
    jmp .done

.make_delim:
    incq 16(%r13)
    incq 32(%r13)
    movq $TOKEN_DELIMITER, %rdi
    movq %rdx, %rsi # Value = ASCII
    call .make_token
    jmp .done

.make_colon:
    incq 16(%r13)
    incq 32(%r13)
    movq $TOKEN_COLON, %rdi
    movq $0, %rsi
    call .make_token
    jmp .done

# ------------------------------------------------------------------------------
# MARKER CHECK (###)
# ------------------------------------------------------------------------------
.check_marker:
    movq 16(%r13), %rax
    addq $2, %rax
    cmpq 8(%r13), %rax
    jge .single_char_op

    movq 0(%r13), %rbx

    # Peek +1
    movq 16(%r13), %rax
    incq %rax
    movzbq (%rbx, %rax), %r8
    cmpb $'#', %r8b
    jne .single_char_op

    # Peek +2
    incq %rax
    movzbq (%rbx, %rax), %r8
    cmpb $'#', %r8b
    jne .single_char_op

    addq $3, 16(%r13)
    addq $3, 32(%r13)

    movq $TOKEN_MARKER, %rdi
    movq $0x232323, %rsi
    call .make_token
    jmp .done

.single_char_op:
    incq 16(%r13)
    incq 32(%r13)
    movq $TOKEN_OPERATOR, %rdi
    movq %rdx, %rsi # Value = ASCII code
    call .make_token
    jmp .done

.done:
    popq %r13
    popq %r12
    popq %rbx
    leave
    ret

# ------------------------------------------------------------------------------
# Internal: .check_if_keyword(str_struct_ptr: ptr) -> type (i64)
# Returns TOKEN_KEYWORD or TOKEN_IDENTIFIER
# ------------------------------------------------------------------------------
.check_if_keyword:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %rbx

    movq %rdi, %r12 # String Struct [Len][Data]
    movq 0(%r12), %rcx # Len Input
    movq 8(%r12), %rdx # Data Input

    # Switch on Length
    cmpq $3, %rcx
    je .len_3
    cmpq $4, %rcx
    je .len_4
    cmpq $5, %rcx
    je .len_5
    cmpq $6, %rcx
    je .len_6
    cmpq $7, %rcx
    je .len_7
    cmpq $10, %rcx
    je .len_10
    cmpq $11, %rcx
    je .len_11
    cmpq $12, %rcx
    je .len_12

    jmp .not_kw

.len_3:
    # "var"
    leaq kw_var(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw
    jmp .not_kw

.len_4:
    # "jika", "lain"
    leaq kw_jika(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw

    leaq kw_lain(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw
    jmp .not_kw

.len_5:
    # "const", "Ambil", "ambil", "pilih", "kasus"
    leaq kw_const(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw

    leaq kw_ambil(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw

    leaq kw_ambil_lc(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw

    leaq kw_pilih(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw

    leaq kw_kasus(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw
    jmp .not_kw

.len_6:
    # "fungsi", "selama", "indeks"
    leaq kw_fungsi(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw

    leaq kw_selama(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw

    leaq kw_indeks(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw

    leaq kw_sistem(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw
    jmp .not_kw

.len_7:
    # "kembali"
    leaq kw_kembali(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw
    jmp .not_kw

.len_10:
    # "tutup_jika"
    leaq kw_tutup_jika(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw
    jmp .not_kw

.len_11:
    # "tutup_pilih"
    leaq kw_tutup_pilih(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw
    jmp .not_kw

.len_12:
    # "tutup_fungsi", "tutup_selama"
    leaq kw_tutup_fungsi(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw

    leaq kw_tutup_selama(%rip), %rdi
    call .cmp_kw_str
    testq %rax, %rax
    jz .found_kw
    jmp .not_kw

.found_kw:
    movq $TOKEN_KEYWORD, %rax
    jmp .ck_ret

.not_kw:
    movq $TOKEN_IDENTIFIER, %rax

.ck_ret:
    popq %rbx
    popq %r13
    popq %r12
    leave
    ret

# Helper: compare string in %rdx (input) and %rdi (kw), length in %rcx
# returns 0 if match, 1 if not
.cmp_kw_str:
    pushq %rsi
    pushq %rdi
    pushq %rcx
    pushq %r8

    movq %rdx, %rsi # Input
    movq %rdi, %r8  # Keyword
    # RCX is count

    xorq %rax, %rax # Assume match (0)
.cks_loop:
    testq %rcx, %rcx
    jz .cks_done

    movb (%rsi), %r9b
    cmpb (%r8), %r9b
    jne .cks_fail

    incq %rsi
    incq %r8
    decq %rcx
    jmp .cks_loop

.cks_fail:
    movq $1, %rax

.cks_done:
    popq %r8
    popq %rcx
    popq %rdi
    popq %rsi
    ret

# Internal: .make_token(type, val) -> ptr
.make_token:
    pushq %r12
    movq %rdi, %r12 # Type
    pushq %rsi      # Val (Stack Align?) -> Saved in stack

    movq $32, %rdi
    call mem_alloc

    popq %rsi       # Restore Val

    movq %r12, 0(%rax) # Type
    movq %rsi, 8(%rax) # Value

    movq 24(%r13), %rcx
    movq %rcx, 16(%rax)
    movq 32(%r13), %rcx
    movq %rcx, 24(%rax)

    popq %r12
    ret
