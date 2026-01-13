# corelib/platform/x86_64/asm/snapshot.s
# Implementasi Snapshot Memory Dump
# Sesuai SSOT corelib/core/memory.fox

.include "corelib/platform/x86_64/asm/macros.inc"

.section .data
dump_prefix: .asciz "/tmp/morph_swap_monitor/dump_"
snap_prefix: .asciz "/tmp/morph_swap_monitor/snapshot_"
bin_ext:     .asciz ".bin"
magic_sig:   .quad 0x504E534850524F4D # "MORPHSNP"

.section .text
.global mem_snapshot_save
.global mem_snapshot_update_tracker
.extern current_page_ptr
.extern current_offset
.extern __mf_print_str
.extern __mf_print_asciz

.section .data
    err_snap_open: .asciz "[Snapshot] Error: Failed to open dump file.\n"
    err_snap_write: .asciz "[Snapshot] Error: Failed to write to dump file.\n"
    err_snap_recover: .asciz "[Snapshot] Error: Failed to recover from dump.\n"
    err_snap_mmap: .asciz "[Snapshot] Error: mmap failed during recovery.\n"
    msg_snap_ok:   .asciz "[Snapshot] Saved successfully.\n"
    msg_rec_ok:    .asciz "[Snapshot] Recovered successfully.\n"

.section .text
# ------------------------------------------------------------------------------
# func mem_snapshot_save()
# Logic: Dump semua memory pages ke file
# ------------------------------------------------------------------------------
mem_snapshot_save:
    pushq %rbp
    movq %rsp, %rbp
    subq $1024, %rsp    # Buffer Path (512) + Header (64) + Vars

    # Stack Layout:
    # -512(%rbp): Path Buffer
    # -576(%rbp): File Header
    # -584(%rbp): FD

    # 1. Get PID & Construct Filename
    movq $SYS_GETPID, %rax
    syscall
    movq %rax, %r12     # PID

    leaq -512(%rbp), %rdi # Buffer
    leaq dump_prefix(%rip), %rsi
    call strcpy_custom

    # Append PID
    movq %r12, %rdi
    leaq -512(%rbp), %rsi
    call append_pid

    # Append Ext
    leaq -512(%rbp), %rdi
    leaq bin_ext(%rip), %rsi
    call strcat_custom

    # 2. Open File
    movq $SYS_OPEN, %rax
    leaq -512(%rbp), %rdi
    movq $O_CREAT, %rsi
    orq $O_RDWR, %rsi
    orq $O_TRUNC, %rsi
    movq $0600, %rdx
    syscall

    testq %rax, %rax
    js .snap_err_open
    movq %rax, -584(%rbp) # Save FD

    # 3. Prepare File Header
    # Magic
    movq magic_sig(%rip), %rax
    movq %rax, -576(%rbp)
    # Version
    movq $1, -568(%rbp)
    # Timestamp
    movq $SYS_TIME, %rax
    xorq %rdi, %rdi
    syscall
    movq %rax, -560(%rbp)

    # Count Pages
    xorq %rcx, %rcx
    movq current_page_ptr(%rip), %rbx
.count_loop:
    testq %rbx, %rbx
    jz .count_done
    incq %rcx
    movq 8(%rbx), %rbx  # Prev
    jmp .count_loop
.count_done:
    movq %rcx, -552(%rbp) # Page Count

    # Zero Reserved
    # Store current_offset in the first reserved slot (Offset 32 / 0x20)
    movq current_offset(%rip), %rax
    movq %rax, -544(%rbp)
    movq $0, -536(%rbp)
    movq $0, -528(%rbp)
    movq $0, -520(%rbp)

    # 4. Write File Header
    movq $SYS_WRITE, %rax
    movq -584(%rbp), %rdi
    leaq -576(%rbp), %rsi
    movq $64, %rdx
    syscall

    # 5. Write Pages (Iterate again)
    movq current_page_ptr(%rip), %rbx
.dump_loop:
    testq %rbx, %rbx
    jz .dump_done

    # A. Write Page Address (8 bytes) - CRITICAL for Recovery
    # We must write the pointer 'rbx' itself to the file
    movq %rbx, -600(%rbp) # Use temp slot
    movq $SYS_WRITE, %rax
    movq -584(%rbp), %rdi
    leaq -600(%rbp), %rsi
    movq $8, %rdx
    syscall
    testq %rax, %rax
    js .snap_err_write

    # B. Write Page Header Copy (48 bytes) from Memory
    # Ptr in RBX.
    movq $SYS_WRITE, %rax
    movq -584(%rbp), %rdi
    movq %rbx, %rsi     # Buffer = Page Ptr (Header start)
    movq $48, %rdx      # Size
    syscall
    testq %rax, %rax
    js .snap_err_write

    # C. Write Page Content
    # Size at Offset 24
    movq 24(%rbx), %rdx # Size
    movq $SYS_WRITE, %rax
    movq -584(%rbp), %rdi
    movq %rbx, %rsi     # Buffer = Page Ptr
    syscall             # RDX = Size

    # Next (Prev)
    movq 8(%rbx), %rbx
    jmp .dump_loop

.dump_done:
    # Close
    movq $SYS_CLOSE, %rax
    movq -584(%rbp), %rdi
    syscall

    # Print Success
    leaq msg_snap_ok(%rip), %rdi
    call __mf_print_asciz

    leave
    ret

.snap_err_open:
    leaq err_snap_open(%rip), %rdi
    call __mf_print_asciz
    leave
    ret

.snap_err_write:
    leaq err_snap_write(%rip), %rdi
    call __mf_print_asciz
    # Try close
    movq $SYS_CLOSE, %rax
    movq -584(%rbp), %rdi
    syscall
    leave
    ret

# ------------------------------------------------------------------------------
# func mem_snapshot_recover()
# Logic: Load memory dump and restore state
# ------------------------------------------------------------------------------
.global mem_snapshot_recover
mem_snapshot_recover:
    pushq %rbp
    movq %rsp, %rbp
    subq $1024, %rsp

    # 1. Get PID & Construct Filename
    movq $SYS_GETPID, %rax
    syscall
    movq %rax, %r12     # PID

    leaq -512(%rbp), %rdi # Buffer
    leaq dump_prefix(%rip), %rsi
    call strcpy_custom

    movq %r12, %rdi
    leaq -512(%rbp), %rsi
    call append_pid

    leaq -512(%rbp), %rdi
    leaq bin_ext(%rip), %rsi
    call strcat_custom

    # 2. Open File (Read Only)
    movq $SYS_OPEN, %rax
    leaq -512(%rbp), %rdi
    movq $O_RDONLY, %rsi
    xorq %rdx, %rdx
    syscall

    testq %rax, %rax
    js .rec_err_open
    movq %rax, -584(%rbp) # FD

    # 3. Read File Header (64 bytes)
    movq $SYS_READ, %rax
    movq -584(%rbp), %rdi
    leaq -576(%rbp), %rsi
    movq $64, %rdx
    syscall

    # Verify Magic (Optional but recommended)
    movq magic_sig(%rip), %rax
    cmpq %rax, -576(%rbp)
    jne .rec_err_fmt

    # Restore current_offset from Padding (Offset 32 / 0x20 in Header)
    # Header starts at -576(%rbp).
    # Layout:
    # 0: Magic
    # 8: Version
    # 16: Timestamp
    # 24: Count
    # 32: Padding1 (Offset) -> -576 + 32 = -544
    movq -544(%rbp), %rax
    movq %rax, current_offset(%rip)

    # Get Page Count
    movq -552(%rbp), %rcx # Count
    movq %rcx, %r13       # Loop Counter
    xorq %r12, %r12       # Page Index (for detecting first page)

    # 4. Loop Read Pages
.rec_loop:
    testq %r13, %r13
    jz .rec_done

    # A. Read Page Address (8 bytes)
    movq $SYS_READ, %rax
    movq -584(%rbp), %rdi
    leaq -600(%rbp), %rsi
    movq $8, %rdx
    syscall

    movq -600(%rbp), %r14 # Target Address

    # CRITICAL: Save first page address as current_page_ptr
    testq %r12, %r12      # Check if this is first iteration
    jnz .skip_first_page
    movq %r14, current_page_ptr(%rip)
.skip_first_page:
    incq %r12             # Increment page index

    # B. Read Page Header (48 bytes) to Temp Buffer
    # We need to know SIZE before mapping.
    # Page Header contains Page Size at offset 24?
    # Actually, Page Header (48 bytes) has: Next(0), Prev(8), TS(16), Size(24), Magic(32), Padding(40).
    # Wait, `alloc.s` stores `Size` at offset 24.

    # Read Header to Stack Buffer
    movq $SYS_READ, %rax
    movq -584(%rbp), %rdi
    leaq -648(%rbp), %rsi # 48 bytes buffer
    movq $48, %rdx
    syscall

    # Extract Size
    movq -624(%rbp), %r15 # Offset 24 in buffer (-648 + 24 = -624)

    # Verify Size
    testq %r15, %r15
    jz .rec_err_fmt

    # C. Map Memory at Target Address
    # mmap(addr, length, prot, flags, fd, offset)
    # We map ANONYMOUS | FIXED | PRIVATE
    # Addr = R14
    # Length = R15 (Wait, R15 is Page Size or User Size?)
    # In `alloc.s`: 24(%rax) is `Page Size` (Total allocation size).
    # So we map that size.

    movq $SYS_MMAP, %rax
    movq %r14, %rdi     # Addr
    movq %r15, %rsi     # Length
    movq $0x7, %rdx     # PROT_READ | WRITE | EXEC (Safety first)
    movq $0x32, %r10    # MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS (0x20+0x10+0x02)
    movq $-1, %r8       # FD -1
    movq $0, %r9        # Offset
    syscall

    # Check mmap result
    cmpq %r14, %rax     # Should match requested address
    jne .rec_err_mmap

    # D. Copy Header Data to Mapped Memory
    # We read header to stack, now copy to memory.
    # Copy 48 bytes from -648(%rbp) to R14
    movq %r14, %rdi
    leaq -648(%rbp), %rsi
    movq $48, %rcx
    rep movsb

    # E. Read Content
    # We need to read (Size - 48) bytes?
    # `mem_snapshot_save` writes: Header (48) then Content (Size).
    # Wait, `mem_snapshot_save` logic:
    #   Write Header (48)
    #   Write Content (Size) <-- This writes the WHOLE page including header again?
    #   Let's check `mem_snapshot_save`:
    #   `movq %rbx, %rsi` (Buffer=PagePtr), `syscall` (RDX=Size).
    #   This writes the WHOLE page (Header + Body).
    #   BUT before that, it wrote Header (48) SEPARATELY.
    #   So the file format is: [Addr 8] [Header 48] [WholePage Size].
    #   This duplicates the header in the file!
    #   `mem_snapshot_save` logic:
    #     Write Header (48)
    #     Write Content (Size) -> This starts from RBX (Header start).
    #   Yes, duplicated.
    #   So in recovery:
    #     Read Addr (8)
    #     Read Header (48) -> Used to get Size.
    #     Map Memory.
    #     Read Whole Page (Size) -> Overwrites the manually copied header.
    #   This is fine, just redundant IO.

    movq $SYS_READ, %rax
    movq -584(%rbp), %rdi
    movq %r14, %rsi     # Buffer = Mapped Mem
    movq %r15, %rdx     # Size
    syscall

    # Next Page
    decq %r13
    jmp .rec_loop

.rec_done:
    # Global state already restored:
    # - current_page_ptr set to first page (in loop)
    # - current_offset restored from header
    movq $SYS_CLOSE, %rax
    movq -584(%rbp), %rdi
    syscall

    leaq msg_rec_ok(%rip), %rdi
    call __mf_print_asciz

    leave
    ret

.rec_err_open:
    leaq err_snap_open(%rip), %rdi
    call __mf_print_asciz
    leave
    ret
.rec_err_fmt:
    leaq err_snap_recover(%rip), %rdi
    call __mf_print_asciz
    jmp .rec_cleanup
.rec_err_mmap:
    leaq err_snap_mmap(%rip), %rdi
    call __mf_print_asciz
    jmp .rec_cleanup
.rec_cleanup:
    movq $SYS_CLOSE, %rax
    movq -584(%rbp), %rdi
    syscall
    leave
    ret

# ------------------------------------------------------------------------------
# func mem_snapshot_update_tracker()
# ------------------------------------------------------------------------------
mem_snapshot_update_tracker:
    pushq %rbp
    movq %rsp, %rbp
    subq $512, %rsp

    # Get PID
    movq $SYS_GETPID, %rax
    syscall
    movq %rax, %r12

    # Construct Path: snapshot_<PID>
    leaq -512(%rbp), %rdi
    leaq snap_prefix(%rip), %rsi
    call strcpy_custom

    movq %r12, %rdi
    leaq -512(%rbp), %rsi
    call append_pid

    # Open
    movq $SYS_OPEN, %rax
    leaq -512(%rbp), %rdi
    movq $O_CREAT, %rsi
    orq $O_RDWR, %rsi
    orq $O_TRUNC, %rsi
    movq $0600, %rdx
    syscall
    testq %rax, %rax
    js .track_err
    movq %rax, %r13 # FD

    # Get Time
    movq $SYS_TIME, %rax
    xorq %rdi, %rdi
    syscall

    # Convert Time to String (Simple Itoa reuse logic needed)
    # I'll just write raw bytes?
    # No, script reads text `ts=$(cat ...)`.
    # I need itoa.

    # Inline Itoa
    movq %rax, %rax # Time
    leaq -100(%rbp), %rdi # Temp buffer
    movq %rdi, %rsi # Start
    movq $10, %rbx
.t_loop:
    xorq %rdx, %rdx
    divq %rbx
    addb $'0', %dl
    movb %dl, (%rdi)
    incq %rdi
    testq %rax, %rax
    jnz .t_loop
    # Reverse
    # ... (Reuse reuse reuse... I should have made a library)
    # Doing quick reverse
    movq %rdi, %rcx
    decq %rcx
    movq %rsi, %r8
.t_rev:
    cmpq %rcx, %r8
    jge .t_done
    movb (%r8), %al
    movb (%rcx), %bl
    movb %bl, (%r8)
    movb %al, (%rcx)
    incq %r8
    decq %rcx
    jmp .t_rev
.t_done:
    subq %rsi, %rdi # Len

    # Write
    movq $SYS_WRITE, %rax
    movq %r13, %rdi # FD
    # Buffer is %rsi (Start)
    movq %rdi, %rdx # Len
    syscall

    # Close
    movq $SYS_CLOSE, %rax
    movq %r13, %rdi
    syscall

.track_err:
    leave
    ret

# Helpers (Local)
strcpy_custom: # rdi=dest, rsi=src
    xorq %rax, %rax
.sc_loop:
    movb (%rsi), %al
    testb %al, %al
    jz .sc_done
    movb %al, (%rdi)
    incq %rsi
    incq %rdi
    jmp .sc_loop
.sc_done:
    movb $0, (%rdi)
    ret

strcat_custom: # rdi=dest (find end), rsi=src
    xorq %rax, %rax
.find_end:
    movb (%rdi), %al
    testb %al, %al
    jz .cat_loop
    incq %rdi
    jmp .find_end
.cat_loop:
    movb (%rsi), %al
    testb %al, %al
    jz .cat_done
    movb %al, (%rdi)
    incq %rsi
    incq %rdi
    jmp .cat_loop
.cat_done:
    movb $0, (%rdi)
    ret

append_pid: # rdi=pid(val), rsi=dest_buffer (find end)
    # Find end
    pushq %rdi
    movq %rsi, %rdi
    call .find_end_pid
    popq %rax # PID

    # Itoa
    movq $10, %rbx
    movq %rdi, %rsi # Save start for reverse

    testq %rax, %rax
    jnz .ap_loop
    movb $'0', (%rdi)
    incq %rdi
    jmp .ap_done

.ap_loop:
    xorq %rdx, %rdx
    divq %rbx
    addb $'0', %dl
    movb %dl, (%rdi)
    incq %rdi
    testq %rax, %rax
    jnz .ap_loop

    # Reverse
    movq %rdi, %rcx
    decq %rcx
    movq %rsi, %r8
.ap_rev:
    cmpq %rcx, %r8
    jge .ap_done
    movb (%r8), %al
    movb (%rcx), %bl
    movb %bl, (%r8)
    movb %al, (%rcx)
    incq %r8
    decq %rcx
    jmp .ap_rev
.ap_done:
    movb $0, (%rdi)
    ret

.find_end_pid:
    movb (%rdi), %al
    testb %al, %al
    jz .fep_done
    incq %rdi
    jmp .find_end_pid
.fep_done:
    ret
