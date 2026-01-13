; corelib/platform/x86_64/asm_win/type.asm
; Implementasi Sistem Tipe (Runtime Type Information) untuk Windows x86_64
; Mengelola pembuatan struct metadata dan offset calculation.

%include "corelib/platform/x86_64/asm_win/macros.inc"

section .text
global type_create
global type_add_field
global type_get_size

extern mem_alloc

; Konstanta Ukuran Tipe Primitif
%define SIZE_I64 8
%define SIZE_PTR 8
%define SIZE_STRING 16

; ------------------------------------------------------------------------------
; func type_create(name_ptr: ptr) -> ptr (TypeDescriptor)
; Windows Input: RCX = name_ptr
; ------------------------------------------------------------------------------
type_create:
    push rbp
    mov rbp, rsp
    push rbx                ; Save Name Ptr
    sub rsp, 32             ; Shadow space

    mov rbx, rcx            ; Save name_ptr

    ; Alloc TypeDescriptor (32 bytes)
    mov rcx, 32
    call mem_alloc

    test rax, rax
    jz .create_fail

    ; Init TypeDescriptor
    ; [00] Name
    mov [rax + 0], rbx
    ; [08] Size = 0
    mov qword [rax + 8], 0
    ; [10] Count = 0
    mov qword [rax + 16], 0
    ; [18] Fields = 0
    mov qword [rax + 24], 0

.create_fail:
    add rsp, 32
    pop rbx
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func type_add_field(type_ptr: ptr, name_ptr: ptr, type_id: i64) -> void
; Menambah field ke struct dan mengupdate total size.
; Windows Input:
;   RCX = type_ptr
;   RDX = name_ptr
;   R8  = type_id
; ------------------------------------------------------------------------------
type_add_field:
    push rbp
    mov rbp, rsp
    push r12                ; Type Ptr
    push r13                ; Name Ptr
    push r14                ; Type ID
    push rbx                ; Field Descriptor Ptr
    sub rsp, 32             ; Shadow space

    mov r12, rcx            ; Save type_ptr
    mov r13, rdx            ; Save name_ptr
    mov r14, r8             ; Save type_id

    ; 1. Alokasi FieldDescriptor (32 bytes)
    mov rcx, 32
    call mem_alloc
    test rax, rax
    jz .add_fail
    mov rbx, rax            ; RBX = Field Descriptor Ptr

    ; 2. Isi Metadata Field
    ; [00] Next (will be set later)
    ; [08] Name
    mov [rbx + 8], r13
    ; [16] Type ID
    mov [rbx + 16], r14
    ; [24] Offset (will be set next)

    ; 3. Hitung Offset
    ; Offset = Current Size dari TypeDescriptor
    mov rcx, [r12 + 8]      ; RCX = Current Size
    mov [rbx + 24], rcx     ; Set Offset

    ; 4. Update Size Struct
    ; Size += Sizeof(TypeID)
    ; Switch Type ID
    cmp r14, 1              ; I64
    je .add_i64
    cmp r14, 2              ; PTR
    je .add_ptr
    cmp r14, 3              ; STRING
    je .add_string
    ; Default/Unknown -> 8 byte fallback
    add rcx, 8
    jmp .update_size

.add_i64:
    add rcx, SIZE_I64
    jmp .update_size
.add_ptr:
    add rcx, SIZE_PTR
    jmp .update_size
.add_string:
    add rcx, SIZE_STRING
    jmp .update_size

.update_size:
    mov [r12 + 8], rcx      ; Update Type Size

    ; 5. Link ke List (Prepend)
    ; Old Head
    mov r8, [r12 + 24]
    mov [rbx + 0], r8       ; New->Next = Old Head
    mov [r12 + 24], rbx     ; Head = New

    ; 6. Inc Count
    inc qword [r12 + 16]

.add_fail:
    add rsp, 32
    pop rbx
    pop r14
    pop r13
    pop r12
    pop rbp
    ret

; ------------------------------------------------------------------------------
; func type_get_size(type_ptr: ptr) -> i64
; Windows Input: RCX = type_ptr
; ------------------------------------------------------------------------------
type_get_size:
    mov rax, [rcx + 8]
    ret
