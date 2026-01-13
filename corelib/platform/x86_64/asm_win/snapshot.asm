; corelib/platform/x86_64/asm_win/snapshot.asm
; Implementasi Snapshot Memory Dump (Windows)
; Sesuai SSOT corelib/core/memory.fox

section .data
    dump_prefix db "morph_swap_monitor\dump_", 0
    snap_prefix db "morph_swap_monitor\snapshot_", 0
    bin_ext     db ".bin", 0
    magic_sig   dq 0x504E534850524F4D ; "MORPHSNP"

section .text
    global mem_snapshot_save
    global mem_snapshot_recover
    global mem_snapshot_update_tracker
    extern current_page_ptr
    extern current_offset
    extern GetCurrentProcessId
    extern CreateFileA
    extern WriteFile
    extern ReadFile
    extern CloseHandle
    extern GetSystemTimeAsFileTime
    extern VirtualAlloc

    ; WinAPI Constants
    GENERIC_WRITE     equ 0x40000000
    GENERIC_READ      equ 0x80000000
    FILE_SHARE_READ   equ 1
    CREATE_ALWAYS     equ 2
    OPEN_EXISTING     equ 3
    FILE_ATTRIBUTE_NORMAL equ 128
    INVALID_HANDLE_VALUE equ -1
    MEM_COMMIT        equ 0x1000
    MEM_RESERVE       equ 0x2000
    PAGE_EXECUTE_READWRITE equ 0x40

; ------------------------------------------------------------------------------
; func mem_snapshot_save()
; ------------------------------------------------------------------------------
mem_snapshot_save:
    push rbp
    mov rbp, rsp
    sub rsp, 1024 + 32 ; Buffer + Shadow Space
    ; Layout:
    ; -512(rbp): Path Buffer
    ; -576(rbp): File Header (64 bytes)
    ; -584(rbp): FD/Handle
    ; Shadow Space at bottom

    ; 1. Construct Filename
    lea rcx, [rbp - 512]
    lea rdx, [rel dump_prefix]
    call strcpy_custom

    call GetCurrentProcessId
    mov rbx, rax    ; PID

    lea rcx, [rbp - 512]
    mov rdx, rbx
    call append_pid

    lea rcx, [rbp - 512]
    lea rdx, [rel bin_ext]
    call strcat_custom

    ; 2. Create File
    ; CreateFileA(path, GENERIC_WRITE, SHARE_READ, NULL, CREATE_ALWAYS, NORMAL, NULL)
    mov qword [rsp + 48], 0     ; hTemplate
    mov qword [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp + 32], CREATE_ALWAYS
    xor r9, r9                  ; Security
    mov r8, FILE_SHARE_READ
    mov rdx, GENERIC_WRITE
    lea rcx, [rbp - 512]
    call CreateFileA

    cmp rax, INVALID_HANDLE_VALUE
    je .snap_err
    mov [rbp - 584], rax        ; Save Handle

    ; 3. Prepare File Header
    ; Magic
    mov rax, [rel magic_sig]
    mov [rbp - 576], rax
    ; Version
    mov qword [rbp - 568], 1
    ; Timestamp (Unix)
    call get_unix_timestamp
    mov [rbp - 560], rax

    ; Count Pages
    xor rcx, rcx
    mov rbx, [rel current_page_ptr]
.count_loop:
    test rbx, rbx
    jz .count_done
    inc rcx
    mov rbx, [rbx + 8] ; Prev
    jmp .count_loop
.count_done:
    mov [rbp - 552], rcx ; Page Count

    ; Zero Padding
    ; Store current_offset at Offset 32 (rbp - 544)
    mov rax, [rel current_offset]
    mov [rbp - 544], rax

    mov qword [rbp - 536], 0
    mov qword [rbp - 528], 0
    mov qword [rbp - 520], 0

    ; 4. Write File Header
    ; WriteFile(handle, buf, len, &written, NULL)
    mov qword [rsp + 32], 0         ; Overlapped
    lea r9, [rbp - 600]             ; written ptr
    mov r8, 64                      ; Size
    lea rdx, [rbp - 576]            ; Buffer
    mov rcx, [rbp - 584]            ; Handle
    call WriteFile

    ; 5. Write Pages
    mov rbx, [rel current_page_ptr]
.dump_loop:
    test rbx, rbx
    jz .dump_done

    ; A. Write Page Address (8 bytes)
    ; We write the pointer itself to the file
    mov [rbp - 600], rbx            ; Use temp slot for addr
    mov qword [rsp + 32], 0
    lea r9, [rbp - 608]             ; written (dummy)
    mov r8, 8                       ; Size
    lea rdx, [rbp - 600]            ; Buffer
    mov rcx, [rbp - 584]            ; Handle
    call WriteFile

    ; B. Write Header (48 bytes)
    mov qword [rsp + 32], 0
    lea r9, [rbp - 608]
    mov r8, 48
    mov rdx, rbx                    ; Buffer = Page Ptr
    mov rcx, [rbp - 584]
    call WriteFile

    ; C. Write Content
    ; Size at offset 24
    mov r8, [rbx + 24]              ; Size

    mov qword [rsp + 32], 0
    lea r9, [rbp - 608]
    ; r8 set above
    mov rdx, rbx                    ; Buffer = Page Ptr
    mov rcx, [rbp - 584]            ; Handle
    call WriteFile

    ; Next
    mov rbx, [rbx + 8]
    jmp .dump_loop

.dump_done:
    mov rcx, [rbp - 584]
    call CloseHandle

.snap_err:
    leave
    ret

; ------------------------------------------------------------------------------
; func mem_snapshot_recover()
; Logic: Load memory dump and restore state using VirtualAlloc at specific addresses
; ------------------------------------------------------------------------------
mem_snapshot_recover:
    push rbp
    mov rbp, rsp
    sub rsp, 1024 + 32

    ; 1. Construct Filename
    lea rcx, [rbp - 512]
    lea rdx, [rel dump_prefix]
    call strcpy_custom

    call GetCurrentProcessId
    mov rbx, rax

    lea rcx, [rbp - 512]
    mov rdx, rbx
    call append_pid

    lea rcx, [rbp - 512]
    lea rdx, [rel bin_ext]
    call strcat_custom

    ; 2. Open File
    mov qword [rsp + 48], 0
    mov qword [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp + 32], OPEN_EXISTING
    xor r9, r9
    mov r8, FILE_SHARE_READ
    mov rdx, GENERIC_READ
    lea rcx, [rbp - 512]
    call CreateFileA

    cmp rax, INVALID_HANDLE_VALUE
    je .rec_err
    mov [rbp - 584], rax        ; Save Handle

    ; 3. Read File Header (64 bytes)
    mov qword [rsp + 32], 0
    lea r9, [rbp - 600]         ; read count
    mov r8, 64
    lea rdx, [rbp - 576]        ; buffer
    mov rcx, [rbp - 584]        ; handle
    call ReadFile

    ; Verify Magic
    mov rax, [rel magic_sig]
    cmp rax, [rbp - 576]
    jne .rec_err_fmt

    ; Restore current_offset (Offset 32 / 0x20)
    mov rax, [rbp - 544]
    mov [rel current_offset], rax

    ; Get Page Count
    mov rcx, [rbp - 552]        ; Count
    mov r13, rcx                ; Loop Counter
    xor r12, r12                ; Page Index (for detecting first page)

    ; 4. Loop Read Pages
.rec_loop:
    test r13, r13
    jz .rec_done

    ; A. Read Page Address (8 bytes)
    mov qword [rsp + 32], 0
    lea r9, [rbp - 600]
    mov r8, 8
    lea rdx, [rbp - 608]        ; Buffer temp
    mov rcx, [rbp - 584]
    call ReadFile

    mov r14, [rbp - 608]        ; Target Address

    ; CRITICAL: Save first page address as current_page_ptr
    test r12, r12               ; Check if this is first iteration
    jnz .skip_first_page
    mov [rel current_page_ptr], r14
.skip_first_page:
    inc r12                     ; Increment page index

    ; B. Read Page Header (48 bytes) to Temp Buffer to get Size
    mov qword [rsp + 32], 0
    lea r9, [rbp - 600]
    mov r8, 48
    lea rdx, [rbp - 656]        ; Temp Buffer (48 bytes)
    mov rcx, [rbp - 584]
    call ReadFile

    ; Extract Size (Offset 24 in buffer)
    mov r15, [rbp - 632]        ; -656 + 24 = -632

    ; C. Alloc Memory at Target Address
    ; VirtualAlloc(addr, size, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE)
    mov r9, PAGE_EXECUTE_READWRITE
    mov r8, MEM_COMMIT | MEM_RESERVE
    mov rdx, r15                ; Size
    mov rcx, r14                ; Address
    call VirtualAlloc

    cmp rax, r14
    jne .rec_err_alloc

    ; D. Copy Header Data (Already read) to Memory
    ; We manually copy the 48 bytes we read
    mov rcx, 48
    lea rsi, [rbp - 656]
    mov rdi, r14
    rep movsb

    ; E. Read Content (Whole Page)
    ; In snapshot.s (Linux) I realized the file contains:
    ; [Addr 8] [Header 48] [WholePage Size (Header+Data)]
    ; So we need to read Size bytes INTO r14.
    ; This overwrites the manually copied header, which is fine.

    mov qword [rsp + 32], 0
    lea r9, [rbp - 600]
    mov r8, r15                 ; Size
    mov rdx, r14                ; Buffer = Mapped Memory
    mov rcx, [rbp - 584]        ; Handle
    call ReadFile

    ; Next Page
    dec r13
    jmp .rec_loop

.rec_done:
    ; Global state already restored:
    ; - current_page_ptr set to first page (in loop)
    ; - current_offset restored from header
    mov rcx, [rbp - 584]
    call CloseHandle
    leave
    ret

.rec_err:
    leave
    ret
.rec_err_fmt:
    jmp .rec_cleanup
.rec_err_alloc:
    jmp .rec_cleanup
.rec_cleanup:
    mov rcx, [rbp - 584]
    call CloseHandle
    leave
    ret

; ------------------------------------------------------------------------------
; func mem_snapshot_update_tracker()
; ------------------------------------------------------------------------------
mem_snapshot_update_tracker:
    push rbp
    mov rbp, rsp
    sub rsp, 512 + 32

    ; Construct Path
    lea rcx, [rbp - 512]
    lea rdx, [rel snap_prefix]
    call strcpy_custom

    call GetCurrentProcessId
    mov rbx, rax

    lea rcx, [rbp - 512]
    mov rdx, rbx
    call append_pid

    ; Create File
    mov qword [rsp + 48], 0
    mov qword [rsp + 40], FILE_ATTRIBUTE_NORMAL
    mov qword [rsp + 32], CREATE_ALWAYS
    xor r9, r9
    mov r8, FILE_SHARE_READ
    mov rdx, GENERIC_WRITE
    lea rcx, [rbp - 512]
    call CreateFileA

    cmp rax, INVALID_HANDLE_VALUE
    je .track_err
    mov rbx, rax            ; Handle

    ; Get Timestamp
    call get_unix_timestamp
    mov rdi, rax            ; Unix TS

    ; Itoa to Buffer (use offset 256)
    lea rcx, [rbp - 256]    ; Buffer
    mov rdx, rdi            ; Value
    call itoa_custom        ; Returns Len in RAX

    ; Write
    mov qword [rsp + 32], 0
    lea r9, [rbp - 100]     ; written
    mov r8, rax             ; Len
    lea rdx, [rbp - 256]    ; Buffer
    mov rcx, rbx            ; Handle
    call WriteFile

    mov rcx, rbx
    call CloseHandle

.track_err:
    leave
    ret

; ------------------------------------------------------------------------------
; Helpers
; ------------------------------------------------------------------------------

; get_unix_timestamp -> RAX
get_unix_timestamp:
    sub rsp, 40
    lea rcx, [rsp + 32]     ; FileTime struct (64-bit)
    call GetSystemTimeAsFileTime
    mov rax, [rsp + 32]     ; Load 64-bit FileTime

    ; Convert to Unix
    mov rdx, 0x019DB1DED53E8000 ; 116444736000000000
    sub rax, rdx
    mov rcx, 10000000
    xor rdx, rdx
    div rcx                 ; RAX = Unix Timestamp
    add rsp, 40
    ret

; strcpy_custom(rcx=dest, rdx=src)
strcpy_custom:
.loop:
    mov al, [rdx]
    mov [rcx], al
    test al, al
    jz .done
    inc rcx
    inc rdx
    jmp .loop
.done:
    ret

; strcat_custom(rcx=dest, rdx=src)
strcat_custom:
.find_end:
    mov al, [rcx]
    test al, al
    jz .cat_loop
    inc rcx
    jmp .find_end
.cat_loop:
    mov al, [rdx]
    mov [rcx], al
    test al, al
    jz .done
    inc rcx
    inc rdx
    jmp .cat_loop
.done:
    ret

; append_pid(rcx=dest, rdx=pid)
append_pid:
    ; Find end
    push rdx
    mov r8, rcx
.fe:
    mov al, [r8]
    test al, al
    jz .fe_done
    inc r8
    jmp .fe
.fe_done:
    pop rdx ; PID

    ; Itoa at R8
    mov rcx, r8
    call itoa_custom
    ret

; itoa_custom(rcx=buffer, rdx=value) -> rax=len
itoa_custom:
    push rbx
    mov r8, rcx         ; Start
    mov rax, rdx        ; Value
    mov rbx, 10

    test rax, rax
    jnz .loop
    mov byte [rcx], '0'
    inc rcx
    jmp .term

.loop:
    xor rdx, rdx
    div rbx
    add dl, '0'
    mov [rcx], dl
    inc rcx
    test rax, rax
    jnz .loop

.term:
    mov byte [rcx], 0   ; Null term

    ; Reverse
    mov r9, rcx
    dec r9              ; End
    mov r10, r8         ; Start
.rev:
    cmp r10, r9
    jge .rev_done
    mov al, [r10]
    mov bl, [r9]
    mov [r10], bl
    mov [r9], al
    inc r10
    dec r9
    jmp .rev
.rev_done:
    ; Calc len
    mov rax, rcx
    sub rax, r8
    pop rbx
    ret
