; corelib/platform/x86_64/asm_win/debug_ast.asm
; AST Visualizer: Mencetak struktur IntentTree ke Console
; ==============================================================================

section .data
    str_lit     db "LITERAL: ", 0
    str_bin     db "BINARY OP: ", 0
    str_unknown db "UNKNOWN TYPE: ", 0
    str_indent  db "  ", 0
    newline     db 10, 0

    ; Op Strings
    str_op_plus  db "+", 0
    str_op_minus db "-", 0
    str_op_mul   db "*", 0
    str_op_div   db "/", 0

section .text
global __mf_print_ast
extern __mf_print_str
extern __mf_print_int

; Intent Types
INTENT_FRAG_LITERAL equ 0x3001
INTENT_FRAG_BINARY  equ 0x3002

; ------------------------------------------------------------------------------
; func __mf_print_ast(node_ptr: ptr, depth: i64)
; Mencetak pohon AST secara rekursif dengan indentasi.
; ------------------------------------------------------------------------------
__mf_print_ast:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13 ; Depth
    push r14 ; Node Ptr
    sub rsp, 32

    mov r14, rcx ; Node
    mov r13, rdx ; Depth

    test r14, r14
    jz .done

    ; Print Indentation
    xor rbx, rbx
.indent_loop:
    cmp rbx, r13
    jge .indent_done
    lea rcx, [rel str_indent]
    call __mf_print_str
    inc rbx
    jmp .indent_loop
.indent_done:

    ; Check Type
    mov rax, [r14 + 0] ; Type
    cmp rax, INTENT_FRAG_LITERAL
    je .print_lit
    cmp rax, INTENT_FRAG_BINARY
    je .print_bin

    lea rcx, [rel str_unknown]
    call __mf_print_str
    mov rcx, rax
    call __mf_print_int
    jmp .print_children

.print_lit:
    lea rcx, [rel str_lit]
    call __mf_print_str
    mov rcx, [r14 + 32] ; Data A (Value)
    call __mf_print_int
    jmp .done_self

.print_bin:
    lea rcx, [rel str_bin]
    call __mf_print_str

    ; Print Op Char
    mov rax, [r14 + 32] ; Data A (Op ID)
    cmp rax, '+'
    je .p_plus
    cmp rax, '*'
    je .p_mul
    ; Fallback print int
    mov rcx, rax
    call __mf_print_int
    jmp .after_op

.p_plus:
    lea rcx, [rel str_op_plus]
    call __mf_print_str
    jmp .after_op
.p_mul:
    lea rcx, [rel str_op_mul]
    call __mf_print_str
    jmp .after_op

.after_op:
    lea rcx, [rel newline]
    call __mf_print_str
    jmp .print_children

.done_self:
    lea rcx, [rel newline]
    call __mf_print_str

.print_children:
    ; Print First Child
    mov rcx, [r14 + 16] ; Child Ptr
    test rcx, rcx
    jz .print_sibling

    ; Recursive Call (Child, Depth + 1)
    ; rcx is already child ptr
    mov rdx, r13
    inc rdx
    call __mf_print_ast

    ; Now iterate siblings of the child manually?
    ; Wait, __mf_print_ast handles siblings?
    ; Usually AST print is: Print Self, Print Child. Child prints its siblings?
    ; No, simpler: Print Self, then iterate children list.
    ; Our structure is: Node -> First Child. Child -> Sibling -> Sibling.

    ; So inside .print_children, we call print on first child.
    ; But the recursive call will only print that one node and ITS children.
    ; It won't print the sibling of the child unless we loop here.

    mov rbx, [r14 + 16] ; Current Child

.child_loop:
    ; We already printed the first child above? No, let's restructure loop.
    ; Actually, let's make the function print ONE node and its children.
    ; It does NOT traverse siblings of the input node.
    ; EXCEPT: The "Children" are a linked list of siblings.

    mov rbx, [r14 + 16] ; First Child
.loop_children_list:
    test rbx, rbx
    jz .print_sibling

    mov rcx, rbx
    mov rdx, r13
    inc rdx
    call __mf_print_ast ; Print child (and its subtree)

    mov rbx, [rbx + 8] ; Next Sibling
    jmp .loop_children_list

.print_sibling:
    ; Do we print our own sibling? No, that's the caller's job (the parent loop above).
    ; So we are done.

.done:
    add rsp, 32
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
