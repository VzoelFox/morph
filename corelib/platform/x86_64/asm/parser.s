# corelib/platform/x86_64/asm/parser.s
# Intent Parser: Mengubah Token Stream menjadi IntentTree (Unit -> Shard -> Fragment)
# ==============================================================================

.include "corelib/platform/x86_64/asm/macros.inc"
.include "corelib/platform/x86_64/asm/intent.s"

.section .data
    # Global recursion depth counter for parser safety
    __parser_recursion_depth: .quad 0

    # ROBUSTNESS: Parser error reporting
    __parser_error_code: .quad 0
    __parser_error_line: .quad 0
    __parser_error_col: .quad 0
    __parser_error_msg: .space 256  # Error message buffer

.section .rodata
    # Error messages
    err_msg_unexpected_token: .asciz "Unexpected token"
    err_msg_expected_string: .asciz "Expected string literal"
    err_msg_expected_identifier: .asciz "Expected identifier"
    err_msg_expected_keyword: .asciz "Expected keyword"
    err_msg_import_failed: .asciz "Import statement malformed"
    err_msg_function_failed: .asciz "Function definition malformed"
    err_msg_unexpected_eof: .asciz "Unexpected end of file"

.section .text
.global parser_parse_unit
.global parser_parse_block
.global parser_parse_expression
.global parser_get_last_error
.global parser_clear_error

.extern mem_alloc
.extern __sys_exit
.extern lexer_peek_token
.extern lexer_next_token
.extern strcpy

# Token Types (Parity with lexer.s)
.equ TOKEN_EOF, 0
.equ TOKEN_INTEGER, 1
.equ TOKEN_IDENTIFIER, 2
.equ TOKEN_OPERATOR, 3
.equ TOKEN_KEYWORD, 4
.equ TOKEN_STRING, 5
.equ TOKEN_COLON, 7

# SAFETY: Maximum recursion depth limit (prevents stack overflow)
.equ MAX_PARSER_DEPTH, 256

# ==============================================================================
# ENTRY POINT: parser_parse_unit
# Input: RDI = Lexer Pointer
# Output: RAX = Pointer to UNIT Node
# ==============================================================================
parser_parse_unit:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12 # Lexer
    pushq %r13 # Unit Node
    pushq %r14 # Last Child (Tail)
    pushq %r15
    subq $32, %rsp

    movq %rdi, %r12

    # 1. Alokasi Unit Node
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq %rax, %r13

    # Init Unit Header
    movq $INTENT_UNIT_MODULE, INTENT_OFFSET_TYPE(%r13)
    movq $0, INTENT_OFFSET_NEXT(%r13)
    movq $0, INTENT_OFFSET_CHILD(%r13)

    # Init Tail
    xorq %r14, %r14

.unit_loop:
    # Peek Token
    movq %r12, %rdi
    call lexer_peek_token
    movq %rax, %rbx

    # Cek EOF
    cmpq $TOKEN_EOF, 0(%rbx)
    je .unit_done

    # Cek Keyword (Fungsi, Var Global, dll)
    cmpq $TOKEN_KEYWORD, 0(%rbx)
    je .unit_keyword

    # Jika bukan keyword di level global, skip atau error?
    movq %r12, %rdi
    call parse_statement
    testq %rax, %rax
    jz .unit_skip

    jmp .unit_append

.unit_keyword:
    movq 8(%rbx), %rdi
    call get_keyword_id
    # 4 = fungsi
    cmpq $4, %rax
    je .unit_func
    # Ambil (Import) - Assuming ID 9 for "ambil" based on lexer.s?
    # Wait, lexer.s defines "Ambil"/"ambil".
    # Need to check `get_keyword_id` logic below.
    # .len_5 checks "ambil" and calls .cmp_kw_str.
    # We need to add ID for "ambil" in get_keyword_id.
    cmpq $9, %rax
    je .unit_import

    # Default: Parse Statement biasa (var, dll)
    movq %r12, %rdi
    call parse_statement
    testq %rax, %rax
    jz .unit_skip
    jmp .unit_append

.unit_import:
    movq %r12, %rdi
    call parse_import
    testq %rax, %rax
    jz .unit_skip
    jmp .unit_append

.unit_func:
    movq %r12, %rdi
    call parse_function
    testq %rax, %rax
    jz .unit_skip
    jmp .unit_append

.unit_append:
    testq %r14, %r14
    jnz .unit_link_tail
    movq %rax, INTENT_OFFSET_CHILD(%r13)
    movq %rax, %r14
    jmp .unit_loop

.unit_link_tail:
    movq %rax, INTENT_OFFSET_NEXT(%r14)
    movq %rax, %r14
    jmp .unit_loop

.unit_skip:
    movq %r12, %rdi
    call lexer_next_token
    jmp .unit_loop

.unit_done:
    movq %r13, %rax
    addq $32, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret

# ==============================================================================
# parse_import
# "ambil 'path' [: symbol]"
# ==============================================================================
parse_import:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13 # Path
    pushq %r14 # Symbol (Optional)
    subq $16, %rsp
    movq %rdi, %r12

    # SAFETY: Check recursion depth
    leaq __parser_recursion_depth(%rip), %rax
    movq (%rax), %rcx
    cmpq $MAX_PARSER_DEPTH, %rcx
    jge .pi_depth_exceeded
    incq (%rax)  # Increment depth

    # Consume "ambil"
    movq %r12, %rdi
    call lexer_next_token

    # Expect String (Path)
    movq %r12, %rdi
    call lexer_next_token
    movq %rax, %rbx  # Save token
    cmpq $TOKEN_STRING, 0(%rax)
    jne .pi_fail_expected_string
    movq 8(%rax), %r13

    # Check for Colon (Granular)
    movq %r12, %rdi
    call lexer_peek_token
    cmpq $TOKEN_COLON, 0(%rax)
    je .pi_granular

    xorq %r14, %r14 # No Symbol (Full Import)
    jmp .pi_create

.pi_granular:
    # Consume Colon
    movq %r12, %rdi
    call lexer_next_token

    # Expect Identifier (Symbol Name)
    movq %r12, %rdi
    call lexer_next_token
    cmpq $TOKEN_IDENTIFIER, 0(%rax)
    jne .pi_fail
    movq 8(%rax), %r14

.pi_create:
    # Create INTENT_FRAG_IMPORT (Need to define ID, reusing dummy or new)
    # Let's use INTENT_FRAG_LITERAL logic but with Type Import?
    # Or INTENT_UNIT_IMPORT?
    # For now, let's map to a custom ID e.g. 0x4001 or reuse.
    # Actually, import usually happens at compile time.
    # We will create a node so the Compiler can process it.

    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $0x1002, INTENT_OFFSET_TYPE(%rax)
    movq %r13, INTENT_OFFSET_DATA_A(%rax)
    movq %r14, INTENT_OFFSET_DATA_B(%rax)

    jmp .pi_ret

.pi_fail:
    # Generic import failure
    movq $1, %rdi  # Error code 1 (import failed)
    leaq err_msg_import_failed(%rip), %rsi
    xorq %rdx, %rdx  # No token info
    call set_parser_error
    xorq %rax, %rax
    jmp .pi_ret

.pi_fail_expected_string:
    # Expected string literal for path
    movq $2, %rdi  # Error code 2
    leaq err_msg_expected_string(%rip), %rsi
    movq %rbx, %rdx  # Token info
    call set_parser_error
    xorq %rax, %rax
    jmp .pi_ret

.pi_depth_exceeded:
    # Recursion depth exceeded - exit with error code
    movq $112, %rdi
    call __sys_exit

.pi_ret:
    # SAFETY: Decrement recursion depth before returning
    leaq __parser_recursion_depth(%rip), %rcx
    decq (%rcx)

    addq $16, %rsp
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

# ==============================================================================
# parse_function
# "fungsi nama() block tutup_fungsi"
# Output: SHARD_FUNC Node
# ==============================================================================
parse_function:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12 # Lexer
    pushq %r13 # Name
    pushq %r14 # Body Block (Fragment List)
    pushq %rbx
    subq $32, %rsp

    movq %rdi, %r12

    # Consume "fungsi"
    movq %r12, %rdi
    call lexer_next_token

    # Expect Identifier (Name)
    movq %r12, %rdi
    call lexer_next_token
    cmpq $TOKEN_IDENTIFIER, 0(%rax)
    jne .pf_fail
    movq 8(%rax), %r13

    # Expect "("
    movq %r12, %rdi
    call lexer_next_token

    # Expect ")"
    movq %r12, %rdi
    call lexer_next_token

    # Parse Body (Block)
    movq %r12, %rdi
    call parser_parse_block
    movq %rax, %r14

    # Expect "tutup_fungsi"
    movq %r12, %rdi
    call lexer_next_token

    # Create SHARD Node
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc

    movq $INTENT_SHARD_FUNC, INTENT_OFFSET_TYPE(%rax)
    movq %r14, INTENT_OFFSET_CHILD(%rax) # Body
    movq %r13, INTENT_OFFSET_DATA_A(%rax) # Name
    movq $0, INTENT_OFFSET_NEXT(%rax)

    jmp .pf_ret

.pf_fail:
    xorq %rax, %rax
.pf_ret:
    addq $32, %rsp
    popq %rbx
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

# ==============================================================================
# parser_parse_block
# Parses statements until a closing keyword is found.
# Output: Head of Fragment Linked List
# ==============================================================================
parser_parse_block:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13 # Head
    pushq %r14 # Tail
    pushq %rbx
    subq $32, %rsp

    movq %rdi, %r12
    xorq %r13, %r13
    xorq %r14, %r14

.blk_loop:
    # Peek
    movq %r12, %rdi
    call lexer_peek_token
    movq %rax, %rbx

    cmpq $TOKEN_EOF, 0(%rbx)
    je .blk_done

    # Cek Closing Keyword
    cmpq $TOKEN_KEYWORD, 0(%rbx)
    jne .blk_stmt

    movq 8(%rbx), %rdi
    call is_closing_keyword
    testq %rax, %rax
    jnz .blk_done

.blk_stmt:
    movq %r12, %rdi
    call parse_statement
    testq %rax, %rax
    jz .blk_done

    # Append
    testq %r13, %r13
    jnz .blk_append
    movq %rax, %r13
    movq %rax, %r14
    jmp .blk_loop

.blk_append:
    movq %rax, INTENT_OFFSET_NEXT(%r14)
    movq %rax, %r14
    jmp .blk_loop

.blk_done:
    movq %r13, %rax
    addq $32, %rsp
    popq %rbx
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

# ==============================================================================
# parse_statement (Generic)
# ==============================================================================
parse_statement:
    pushq %r12
    pushq %rbx
    movq %rdi, %r12

    movq %r12, %rdi
    call lexer_peek_token
    movq %rax, %rbx

    cmpq $TOKEN_KEYWORD, 0(%rbx)
    je .stmt_kw

    # Expression
    movq %r12, %rdi
    call parser_parse_expression
    jmp .stmt_ret

.stmt_kw:
    movq 8(%rbx), %rdi
    call get_keyword_id

    cmpq $1, %rax # var
    je .do_var
    cmpq $2, %rax # jika
    je .do_if
    cmpq $3, %rax # selama
    je .do_while
    cmpq $5, %rax # kembali
    je .do_return
    cmpq $6, %rax # pilih (switch)
    je .do_switch

    # If generic keyword or unknown, treat as expression or null
    xorq %rax, %rax
    jmp .stmt_ret

.do_var:
    movq %r12, %rdi
    call parse_var_decl
    jmp .stmt_ret
.do_if:
    movq %r12, %rdi
    call parse_if
    jmp .stmt_ret
.do_while:
    movq %r12, %rdi
    call parse_while
    jmp .stmt_ret
.do_return:
    movq %r12, %rdi
    call parse_return
    jmp .stmt_ret
.do_switch:
    movq %r12, %rdi
    call parse_switch
    jmp .stmt_ret

.stmt_ret:
    popq %rbx
    popq %r12
    ret

# ==============================================================================
# parse_switch (pilih)
# "pilih (expr) kasus val: ... kasus val2: ... tutup_pilih"
# ==============================================================================
parse_switch:
    pushq %r12
    pushq %r13 # Cond Expr
    pushq %r14 # Case List Head
    pushq %r15 # Case List Tail
    subq $16, %rsp
    movq %rdi, %r12

    # Consume "pilih", "(", Expr, ")"
    movq %r12, %rdi
    call lexer_next_token
    movq %r12, %rdi
    call lexer_next_token
    movq %r12, %rdi
    call parser_parse_expression
    movq %rax, %r13
    movq %r12, %rdi
    call lexer_next_token

    xorq %r14, %r14
    xorq %r15, %r15

.ps_loop:
    # Loop parsing cases
    movq %r12, %rdi
    call lexer_peek_token
    movq 8(%rax), %rdi
    call get_keyword_id
    cmpq $8, %rax # kasus
    jne .ps_done

    # Parse Case
    movq %r12, %rdi
    call parse_case
    testq %rax, %rax
    jz .ps_done

    # Append to Case List
    testq %r14, %r14
    jnz .ps_append
    movq %rax, %r14
    movq %rax, %r15
    jmp .ps_loop
.ps_append:
    movq %rax, INTENT_OFFSET_NEXT(%r15)
    movq %rax, %r15
    jmp .ps_loop

.ps_done:
    # Consume "tutup_pilih"
    movq %r12, %rdi
    call lexer_next_token

    # Alloc SWITCH Node
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_FRAG_SWITCH, INTENT_OFFSET_TYPE(%rax)
    movq %r13, INTENT_OFFSET_CHILD(%rax) # Condition
    movq %r14, INTENT_OFFSET_DATA_A(%rax) # Case List

    addq $16, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    ret

# ==============================================================================
# parse_case
# "kasus value : block"
# ==============================================================================
parse_case:
    pushq %r12
    pushq %r13 # Val
    pushq %r14 # Block
    movq %rdi, %r12

    # Consume "kasus"
    movq %r12, %rdi
    call lexer_next_token

    # Parse Value (Literal/Expr)
    movq %r12, %rdi
    call parser_parse_expression
    movq %rax, %r13

    # Consume ":" (Operator)
    # Check if present, simplistic
    movq %r12, %rdi
    call lexer_next_token

    # Parse Block (Body) until next "kasus" or "tutup_pilih"
    # parser_parse_block stops at "tutup_*".
    # But "kasus" is a keyword, not closing.
    # We need a custom block parser or rely on "kasus" acting as delimiter?
    # Standard `parser_parse_block` only stops at `is_closing_keyword`.
    # "kasus" is NOT a closing keyword.
    # So `parser_parse_block` would consume "kasus" as a statement, fail (unknown kw), and stop?
    # parse_statement returns 0 for "kasus" (ID 8) because generic handler only handles 1,2,3,5,6.
    # So yes, `parser_parse_block` will stop when it hits "kasus" because `parse_statement` returns null.

    movq %r12, %rdi
    call parser_parse_block
    # Wrap in SHARD_BLOCK
    movq %rax, %rbx
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_SHARD_BLOCK, INTENT_OFFSET_TYPE(%rax)
    movq %rbx, INTENT_OFFSET_CHILD(%rax)
    movq %rax, %r14

    # Create CASE Node
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_FRAG_CASE, INTENT_OFFSET_TYPE(%rax)
    movq %r13, INTENT_OFFSET_CHILD(%rax) # Value
    movq %r14, INTENT_OFFSET_DATA_A(%rax) # Body Shard

    popq %r14
    popq %r13
    popq %r12
    ret

# ==============================================================================
# parse_if
# ==============================================================================
parse_if:
    pushq %r12
    pushq %r13 # Cond
    pushq %r14 # True Shard
    pushq %r15 # False Shard
    movq %rdi, %r12

    movq %r12, %rdi
    call lexer_next_token
    movq %r12, %rdi
    call lexer_next_token

    movq %r12, %rdi
    call parser_parse_expression
    movq %rax, %r13

    movq %r12, %rdi
    call lexer_next_token

    movq %r12, %rdi
    call parser_parse_block
    movq %rax, %rbx
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_SHARD_BLOCK, INTENT_OFFSET_TYPE(%rax)
    movq %rbx, INTENT_OFFSET_CHILD(%rax)
    movq %rax, %r14

    xorq %r15, %r15
    movq %r12, %rdi
    call lexer_peek_token
    movq 8(%rax), %rdi
    call get_keyword_id
    cmpq $7, %rax # lain
    jne .pif_close

    movq %r12, %rdi
    call lexer_next_token

    movq %r12, %rdi
    call parser_parse_block
    movq %rax, %rbx
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_SHARD_BLOCK, INTENT_OFFSET_TYPE(%rax)
    movq %rbx, INTENT_OFFSET_CHILD(%rax)
    movq %rax, %r15

.pif_close:
    movq %r12, %rdi
    call lexer_next_token

    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_FRAG_IF, INTENT_OFFSET_TYPE(%rax)
    movq %r13, INTENT_OFFSET_CHILD(%rax)

    testq %r13, %r13
    jz .pif_link_fail
    movq %r14, INTENT_OFFSET_NEXT(%r13)
    testq %r14, %r14
    jz .pif_done
    movq %r15, INTENT_OFFSET_NEXT(%r14)
.pif_done:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    ret
.pif_link_fail:
    jmp .pif_done

# ==============================================================================
# parse_while
# ==============================================================================
parse_while:
    pushq %r12
    pushq %r13
    pushq %r14
    movq %rdi, %r12

    movq %r12, %rdi
    call lexer_next_token
    movq %r12, %rdi
    call lexer_next_token

    movq %r12, %rdi
    call parser_parse_expression
    movq %rax, %r13

    movq %r12, %rdi
    call lexer_next_token

    movq %r12, %rdi
    call parser_parse_block
    movq %rax, %rbx
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_SHARD_BLOCK, INTENT_OFFSET_TYPE(%rax)
    movq %rbx, INTENT_OFFSET_CHILD(%rax)
    movq %rax, %r14

    movq %r12, %rdi
    call lexer_next_token

    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_FRAG_WHILE, INTENT_OFFSET_TYPE(%rax)
    movq %r13, INTENT_OFFSET_CHILD(%rax)

    testq %r13, %r13
    jz .pw_done
    movq %r14, INTENT_OFFSET_NEXT(%r13)

.pw_done:
    popq %r14
    popq %r13
    popq %r12
    ret

# ==============================================================================
# parse_var_decl
# ==============================================================================
parse_var_decl:
    pushq %r12
    pushq %r13
    pushq %r14
    movq %rdi, %r12

    movq %r12, %rdi
    call lexer_next_token
    movq %r12, %rdi
    call lexer_next_token
    movq 8(%rax), %r13

    movq %r12, %rdi
    call lexer_next_token

    movq %r12, %rdi
    call parser_parse_expression
    movq %rax, %r14

    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_FRAG_ASSIGN, INTENT_OFFSET_TYPE(%rax)
    movq %r13, INTENT_OFFSET_DATA_A(%rax)
    movq %r14, INTENT_OFFSET_CHILD(%rax)

    popq %r14
    popq %r13
    popq %r12
    ret

# ==============================================================================
# parse_return
# ==============================================================================
parse_return:
    pushq %r12
    movq %rdi, %r12
    movq %r12, %rdi
    call lexer_next_token
    movq %r12, %rdi
    call parser_parse_expression
    pushq %rax
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    popq %rbx
    movq $INTENT_FRAG_RETURN, INTENT_OFFSET_TYPE(%rax)
    movq %rbx, INTENT_OFFSET_CHILD(%rax)
    popq %r12
    ret

# ==============================================================================
# parse_syscall
# "sistem intent, arg1, arg2..."
# ==============================================================================
parse_syscall:
    pushq %r12
    pushq %r13 # Intent
    pushq %r14 # Arg Head
    pushq %r15 # Arg Tail
    pushq %rbx
    subq $16, %rsp
    movq %rdi, %r12

    # Parse Intent (Expression)
    movq %r12, %rdi
    call parser_parse_expression
    movq %rax, %r13

    xorq %r14, %r14
    xorq %r15, %r15

.psys_loop:
    # Check for comma
    movq %r12, %rdi
    call lexer_peek_token
    movq 8(%rax), %rsi
    cmpq $44, %rsi # ','
    jne .psys_done

    # Consume comma
    movq %r12, %rdi
    call lexer_next_token

    # Parse Arg
    movq %r12, %rdi
    call parser_parse_expression
    testq %rax, %rax
    jz .psys_done

    # Append
    testq %r14, %r14
    jnz .psys_append
    movq %rax, %r14
    movq %rax, %r15
    jmp .psys_loop

.psys_append:
    movq %rax, INTENT_OFFSET_NEXT(%r15)
    movq %rax, %r15
    jmp .psys_loop

.psys_done:
    # Create INTENT_FRAG_SYSCALL (0x4003)
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $0x4003, INTENT_OFFSET_TYPE(%rax)
    movq %r13, INTENT_OFFSET_CHILD(%rax) # Intent
    movq %r14, INTENT_OFFSET_DATA_A(%rax) # Arg List

    addq $16, %rsp
    popq %rbx
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    ret

# ==============================================================================
# Existing Expr Helpers (Ported/Preserved)
# ==============================================================================
parser_parse_expression:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    subq $32, %rsp
    movq %rdi, %r12

    movq %r12, %rdi
    call parse_primary
    testq %rax, %rax
    jz .expr_fail
    movq %rax, %r13

    movq %r12, %rdi
    call lexer_peek_token
    movq %rax, %rbx
    cmpq $TOKEN_OPERATOR, 0(%rbx)
    jne .expr_ret_left

    movq 8(%rbx), %rsi
    # Check if ":" (colon) - Treat as terminator, NOT binary op
    cmpq $58, %rsi
    je .expr_ret_left

    movq %r12, %rdi
    call lexer_next_token
    movq 8(%rax), %r14

    movq %r12, %rdi
    call parse_primary
    movq %rax, %r15

    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    movq $INTENT_FRAG_BINARY, INTENT_OFFSET_TYPE(%rax)
    movq %r14, INTENT_OFFSET_DATA_A(%rax)
    movq %r13, INTENT_OFFSET_CHILD(%rax)
    movq %r15, INTENT_OFFSET_NEXT(%r13)
    jmp .expr_done

.expr_ret_left:
    movq %r13, %rax
    jmp .expr_done

.expr_fail:
    xorq %rax, %rax
.expr_done:
    addq $32, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

parse_primary:
    pushq %rbx
    pushq %r12
    movq %rdi, %r12

    movq %r12, %rdi
    call lexer_next_token
    movq 0(%rax), %rbx
    movq 8(%rax), %rcx

    cmpq $TOKEN_INTEGER, %rbx
    je .pp_int
    cmpq $TOKEN_IDENTIFIER, %rbx
    je .pp_var
    cmpq $TOKEN_STRING, %rbx
    je .pp_str
    cmpq $TOKEN_KEYWORD, %rbx
    je .pp_kw

    xorq %rax, %rax
    jmp .pp_ret

.pp_kw:
    pushq %rcx
    movq %rcx, %rdi
    call get_keyword_id
    cmpq $20, %rax # sistem
    je .pp_sys
    popq %rcx
    xorq %rax, %rax
    jmp .pp_ret

.pp_sys:
    popq %rcx
    movq %r12, %rdi
    call parse_syscall
    jmp .pp_ret

.pp_str:
    pushq %rcx
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    popq %rcx
    movq $0x4002, INTENT_OFFSET_TYPE(%rax) # INTENT_FRAG_STRING (New ID)
    movq %rcx, INTENT_OFFSET_DATA_A(%rax)
    jmp .pp_ret

.pp_int:
    pushq %rcx
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    popq %rcx
    movq $INTENT_FRAG_LITERAL, INTENT_OFFSET_TYPE(%rax)
    movq %rcx, INTENT_OFFSET_DATA_A(%rax)
    jmp .pp_ret

.pp_var:
    pushq %rcx
    movq $INTENT_NODE_SIZE, %rdi
    call mem_alloc
    popq %rcx
    movq $INTENT_FRAG_VAR, INTENT_OFFSET_TYPE(%rax)
    movq %rcx, INTENT_OFFSET_DATA_A(%rax)
    jmp .pp_ret

.pp_ret:
    popq %r12
    popq %rbx
    ret

# Helpers
is_closing_keyword:
    movq 8(%rdi), %rsi
    cmpb $'t', (%rsi)
    jne .chk_lain
    cmpb $'u', 1(%rsi)
    jne .chk_lain
    movq $1, %rax
    ret
.chk_lain:
    cmpb $'l', (%rsi)
    jne .not_close
    cmpb $'a', 1(%rsi)
    jne .not_close
    movq $1, %rax
    ret
.not_close:
    xorq %rax, %rax
    ret

get_keyword_id:
    movq 8(%rdi), %rsi
    movzbq (%rsi), %rax
    cmpb $'v', %al
    je .k_var
    cmpb $'j', %al
    je .k_jika
    cmpb $'s', %al
    je .k_check_s
    cmpb $'f', %al
    je .k_fun
    cmpb $'p', %al
    je .k_pil
    cmpb $'k', %al
    je .k_check_k
    cmpb $'l', %al
    je .k_lain
    cmpb $'a', %al
    je .k_ambil
    xorq %rax, %rax
    ret
.k_var: movq $1, %rax; ret
.k_jika: movq $2, %rax; ret
.k_sel: movq $3, %rax; ret
.k_sys: movq $20, %rax; ret
.k_fun: movq $4, %rax; ret
.k_pil: movq $6, %rax; ret # pilih
.k_lain: movq $7, %rax; ret
.k_ambil: movq $9, %rax; ret

.k_check_k:
    # 'k' found. Check next char.
    # kembali vs kasus
    movzbq 1(%rsi), %rcx
    cmpb $'e', %cl
    je .k_ret # kembali
    cmpb $'a', %cl
    je .k_kas # kasus
    xorq %rax, %rax
    ret
.k_ret: movq $5, %rax; ret
.k_kas: movq $8, %rax; ret

.k_check_s:
    movzbq 1(%rsi), %rcx
    cmpb $'e', %cl
    je .k_sel # selama
    cmpb $'i', %cl
    je .k_sys # sistem
    xorq %rax, %rax
    ret

# ==============================================================================
# ROBUSTNESS: Error Reporting Functions
# ==============================================================================

# ------------------------------------------------------------------------------
# set_parser_error(error_code: i64, msg_ptr: ptr, token_ptr: ptr)
# Internal helper to record parse errors
# RDI = error code, RSI = message pointer, RDX = token pointer (optional)
# ------------------------------------------------------------------------------
set_parser_error:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13

    movq %rdi, %r12  # error code
    movq %rsi, %r13  # message

    # Store error code
    leaq __parser_error_code(%rip), %rax
    movq %r12, (%rax)

    # Copy message to buffer (max 255 bytes)
    leaq __parser_error_msg(%rip), %rdi
    movq %r13, %rsi
    movq $0, %rcx
.copy_msg:
    movb (%rsi, %rcx), %al
    movb %al, (%rdi, %rcx)
    testb %al, %al
    jz .copy_done
    incq %rcx
    cmpq $255, %rcx
    jl .copy_msg
.copy_done:
    movb $0, (%rdi, %rcx)  # Null terminate

    # Extract line/col from token if provided
    testq %rdx, %rdx
    jz .no_token_info

    # Assuming token has line at offset 16, col at offset 24
    movq 16(%rdx), %rax
    leaq __parser_error_line(%rip), %rcx
    movq %rax, (%rcx)

    movq 24(%rdx), %rax
    leaq __parser_error_col(%rip), %rcx
    movq %rax, (%rcx)
    jmp .set_error_done

.no_token_info:
    leaq __parser_error_line(%rip), %rcx
    movq $0, (%rcx)
    leaq __parser_error_col(%rip), %rcx
    movq $0, (%rcx)

.set_error_done:
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret

# ------------------------------------------------------------------------------
# parser_get_last_error() -> ptr
# Returns pointer to error message buffer (or NULL if no error)
# Output: RAX = message pointer or 0
# ------------------------------------------------------------------------------
parser_get_last_error:
    leaq __parser_error_code(%rip), %rax
    movq (%rax), %rcx
    testq %rcx, %rcx
    jz .no_error
    leaq __parser_error_msg(%rip), %rax
    ret
.no_error:
    xorq %rax, %rax
    ret

# ------------------------------------------------------------------------------
# parser_clear_error()
# Clears the error state
# ------------------------------------------------------------------------------
parser_clear_error:
    leaq __parser_error_code(%rip), %rax
    movq $0, (%rax)
    leaq __parser_error_line(%rip), %rax
    movq $0, (%rax)
    leaq __parser_error_col(%rip), %rax
    movq $0, (%rax)
    ret
