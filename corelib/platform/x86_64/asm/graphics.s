# corelib/platform/x86_64/asm/graphics.s
# Implementasi Grafis Native (Linux x86_64)
# X11 Protocol via Unix Socket (/tmp/.X11-unix/X0)

.include "corelib/platform/x86_64/asm/syscalls.inc"

.section .data
x11_socket_path: .string "/tmp/.X11-unix/X0"
xauth_proto_name: .string "MIT-MAGIC-COOKIE-1"
xauthority_path_prefix: .string "/.Xauthority"
display_num: .string "0"
.align 4

.section .bss
    .lcomm x11_fd, 8
    .lcomm x11_root_id, 4
    .lcomm x11_id_base, 4
    .lcomm x11_id_mask, 4
    .lcomm x11_my_win_id, 4
    .lcomm x11_my_gc_id, 4
    .lcomm x11_sockaddr, 110 # AF_UNIX (2) + Path (108)
    .lcomm x11_buffer, 4096  # Buffer for requests/replies
    .lcomm xauth_cookie, 16  # MIT-MAGIC-COOKIE-1 data (16 bytes)
    .lcomm xauth_path, 256   # Full path to .Xauthority
    .lcomm x11_handshake_buf, 128 # Dynamic handshake buffer

.section .text
.global __mf_window_create
.global __mf_draw_pixel
.global __mf_window_poll
.global __mf_draw_char
.global __mf_draw_string
.global __mf_fill_rect
.global __mf_draw_rect
.extern __mf_net_socket
.extern __mf_net_connect
.extern __mf_net_send
.extern __mf_net_recv
.extern __mf_net_close
.extern __mf_print_int
.extern __mf_font_bitmap

# ------------------------------------------------------------------------------
# Internal: read_xauthority() -> success (0=ok, -1=fail)
# Reads MIT-MAGIC-COOKIE-1 from ~/.Xauthority into xauth_cookie buffer
# FULLY IMPLEMENTED - No more stub!
# ------------------------------------------------------------------------------
read_xauthority:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    subq $512, %rsp  # Local buffer for environ parsing

    # Step 1: Read HOME from /proc/self/environ
    # Open /proc/self/environ
    leaq -512(%rbp), %rdi
    leaq proc_self_environ(%rip), %rsi
    movq $0, %rcx
.copy_proc_path:
    movb (%rsi, %rcx), %al
    movb %al, (%rdi, %rcx)
    testb %al, %al
    jz .open_environ
    incq %rcx
    cmpq $64, %rcx
    jl .copy_proc_path

.open_environ:
    leaq -512(%rbp), %rdi
    movq $0, %rsi      # O_RDONLY
    movq $0, %rdx      # mode (ignored for O_RDONLY)
    movq $2, %rax      # SYS_OPEN
    syscall
    testq %rax, %rax
    js .fail_no_home
    movq %rax, %r12    # r12 = environ_fd

    # Read environ data
    movq %r12, %rdi
    leaq -512(%rbp), %rsi
    movq $512, %rdx
    movq $0, %rax      # SYS_READ
    syscall
    movq %rax, %r13    # r13 = bytes_read

    # Close environ fd
    movq %r12, %rdi
    movq $3, %rax      # SYS_CLOSE
    syscall

    # Step 2: Parse environ to find HOME=
    leaq -512(%rbp), %rsi
    xorq %rcx, %rcx    # offset
.find_home:
    cmpq %r13, %rcx
    jge .fail_no_home

    # Check if starts with "HOME="
    cmpb $'H', (%rsi, %rcx)
    jne .next_env_var
    cmpb $'O', 1(%rsi, %rcx)
    jne .next_env_var
    cmpb $'M', 2(%rsi, %rcx)
    jne .next_env_var
    cmpb $'E', 3(%rsi, %rcx)
    jne .next_env_var
    cmpb $'=', 4(%rsi, %rcx)
    jne .next_env_var

    # Found HOME=, extract value
    addq $5, %rcx      # Skip "HOME="
    leaq (%rsi, %rcx), %r14  # r14 = HOME value start

    # Find length (until null)
    xorq %r15, %r15
.measure_home:
    movb (%r14, %r15), %al
    testb %al, %al
    jz .found_home
    incq %r15
    cmpq $200, %r15   # Max HOME length
    jl .measure_home
    jmp .fail_no_home

.next_env_var:
    # Skip to next null-terminated var
    incb (%rsi, %rcx)
    jz .skip_found
    incq %rcx
    jmp .next_env_var
.skip_found:
    incq %rcx          # Skip the null
    jmp .find_home

.found_home:
    # Step 3: Build ~/.Xauthority path
    leaq xauth_path(%rip), %rdi
    movq %r14, %rsi
    movq %r15, %rcx
    rep movsb          # Copy HOME

    # Append "/.Xauthority"
    leaq xauthority_path_prefix(%rip), %rsi
    movq $13, %rcx
    rep movsb
    movb $0, (%rdi)    # Null terminate

    # Step 4: Open .Xauthority file
    leaq xauth_path(%rip), %rdi
    movq $0, %rsi      # O_RDONLY
    movq $0, %rdx
    movq $2, %rax      # SYS_OPEN
    syscall
    testq %rax, %rax
    js .fail_no_file
    movq %rax, %r12    # r12 = xauth_fd

    # Step 5: Read .Xauthority file
    movq %r12, %rdi
    leaq x11_buffer(%rip), %rsi
    movq $4096, %rdx
    movq $0, %rax      # SYS_READ
    syscall
    movq %rax, %r13    # r13 = bytes_read

    # Close file
    pushq %r13
    movq %r12, %rdi
    movq $3, %rax      # SYS_CLOSE
    syscall
    popq %r13

    testq %r13, %r13
    jle .fail_parse

    # Step 6: Parse Xauthority format (Big Endian)
    leaq x11_buffer(%rip), %rsi
    xorq %rcx, %rcx    # offset

.parse_entry:
    # Check if we have enough bytes for header
    addq $12, %rcx     # Minimum: family(2) + addr_len(2) + num_len(2) + name_len(2) + data_len(2) + padding
    cmpq %r13, %rcx
    jge .fail_parse
    subq $12, %rcx

    # Read family (2 bytes, Big Endian)
    movzbq 0(%rsi, %rcx), %rax
    shlq $8, %rax
    movzbq 1(%rsi, %rcx), %rbx
    orq %rbx, %rax
    addq $2, %rcx

    # Read address length (2 bytes BE)
    movzbq 0(%rsi, %rcx), %r14
    shlq $8, %r14
    movzbq 1(%rsi, %rcx), %rbx
    orq %rbx, %r14
    addq $2, %rcx

    # Skip address
    addq %r14, %rcx

    # Read display number length (2 bytes BE)
    movzbq 0(%rsi, %rcx), %r14
    shlq $8, %r14
    movzbq 1(%rsi, %rcx), %rbx
    orq %rbx, %r14
    addq $2, %rcx

    # Skip display number (we assume display 0)
    addq %r14, %rcx

    # Read auth name length (2 bytes BE)
    movzbq 0(%rsi, %rcx), %r14
    shlq $8, %r14
    movzbq 1(%rsi, %rcx), %rbx
    orq %rbx, %r14
    addq $2, %rcx

    # Check if auth name is "MIT-MAGIC-COOKIE-1"
    cmpq $18, %r14
    jne .skip_entry

    # Compare auth name
    leaq xauth_proto_name(%rip), %rdi
    leaq (%rsi, %rcx), %r8
    movq $18, %r10
.cmp_proto:
    movb (%rdi), %al
    movb (%r8), %bl
    cmpb %al, %bl
    jne .skip_entry
    incq %rdi
    incq %r8
    decq %r10
    jnz .cmp_proto

    # Found MIT-MAGIC-COOKIE-1!
    addq $18, %rcx

    # Read auth data length (2 bytes BE)
    movzbq 0(%rsi, %rcx), %r14
    shlq $8, %r14
    movzbq 1(%rsi, %rcx), %rbx
    orq %rbx, %r14
    addq $2, %rcx

    # Verify data length is 16 bytes
    cmpq $16, %r14
    jne .fail_parse

    # Copy 16-byte cookie
    leaq xauth_cookie(%rip), %rdi
    leaq (%rsi, %rcx), %rsi
    movq $16, %rcx
    rep movsb

    # Success!
    xorq %rax, %rax
    jmp .cleanup

.skip_entry:
    # Skip auth name and auth data
    addq %r14, %rcx    # Skip name

    # Read data length
    movzbq 0(%rsi, %rcx), %r14
    shlq $8, %r14
    movzbq 1(%rsi, %rcx), %rbx
    orq %rbx, %r14
    addq $2, %rcx

    # Skip data
    addq %r14, %rcx

    # Try next entry
    cmpq %r13, %rcx
    jl .parse_entry

.fail_parse:
.fail_no_file:
.fail_no_home:
    movq $-1, %rax

.cleanup:
    addq $512, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret

.section .rodata
proc_self_environ: .asciz "/proc/self/environ"

# ------------------------------------------------------------------------------
# Internal: build_handshake(auth_available: i64) -> buf_ptr, buf_len
# Builds X11 handshake in x11_handshake_buf
# If auth_available=1, includes MIT-MAGIC-COOKIE-1 from xauth_cookie
# If auth_available=0, builds no-auth handshake
# Returns: RDI=buffer ptr, RSI=buffer length
# ------------------------------------------------------------------------------
build_handshake:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12

    movq %rdi, %r12  # auth_available flag

    leaq x11_handshake_buf(%rip), %rdi

    # Handshake Header (12 bytes):
    # [0] ByteOrder ('l' = LSB)
    # [1] Unused (0)
    # [2-3] Major version (11)
    # [4-5] Minor version (0)
    # [6-7] Auth protocol name length
    # [8-9] Auth protocol data length
    # [10-11] Unused (0)

    movb $0x6C, 0(%rdi)     # 'l' (little endian)
    movb $0, 1(%rdi)        # unused
    movw $11, 2(%rdi)       # major
    movw $0, 4(%rdi)        # minor

    testq %r12, %r12
    jz .no_auth_handshake

    # WITH AUTH
    movw $18, 6(%rdi)       # auth name len = 18 ("MIT-MAGIC-COOKIE-1")
    movw $16, 8(%rdi)       # auth data len = 16 (cookie)
    movw $0, 10(%rdi)       # unused

    # Auth name (18 bytes) + padding to multiple of 4
    # 18 bytes -> need 2 bytes padding -> total 20
    leaq xauth_proto_name(%rip), %rsi
    leaq 12(%rdi), %rdi
    movq $18, %rcx
.copy_auth_name:
    movb (%rsi), %al
    movb %al, (%rdi)
    incq %rsi
    incq %rdi
    decq %rcx
    jnz .copy_auth_name

    # Padding (2 bytes)
    movw $0, (%rdi)
    addq $2, %rdi

    # Auth data (16 bytes) - no padding needed (already multiple of 4)
    leaq xauth_cookie(%rip), %rsi
    movq $16, %rcx
.copy_auth_data:
    movb (%rsi), %al
    movb %al, (%rdi)
    incq %rsi
    incq %rdi
    decq %rcx
    jnz .copy_auth_data

    # Total length: 12 + 20 + 16 = 48 bytes
    leaq x11_handshake_buf(%rip), %rdi
    movq $48, %rsi
    jmp .build_done

.no_auth_handshake:
    movw $0, 6(%rdi)        # no auth name
    movw $0, 8(%rdi)        # no auth data
    movw $0, 10(%rdi)       # unused

    # Total length: 12 bytes
    leaq x11_handshake_buf(%rip), %rdi
    movq $12, %rsi

.build_done:
    popq %r12
    popq %rbp
    ret

# ------------------------------------------------------------------------------
# func __mf_window_create(width: i64, height: i64, title: ptr) -> handle
# Input: RDI=W, RSI=H, RDX=Title
# ------------------------------------------------------------------------------
__mf_window_create:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %rdi, %r12 # Width
    movq %rsi, %r13 # Height

    # 1. Open Socket
    # socket(AF_UNIX=1, SOCK_STREAM=1, 0)
    movq $1, %rdi
    movq $1, %rsi
    xorq %rdx, %rdx
    call __mf_net_socket
    movq %rax, x11_fd(%rip)
    testq %rax, %rax
    js .fail_open

    # 2. Connect
    # Setup sockaddr_un
    leaq x11_sockaddr(%rip), %rdi
    movw $1, 0(%rdi) # AF_UNIX

    # Copy Path
    leaq x11_socket_path(%rip), %rsi
    leaq 2(%rdi), %rdi # Dest path
.copy_path:
    movb (%rsi), %al
    testb %al, %al
    jz .connect_now
    movb %al, (%rdi)
    incq %rsi
    incq %rdi
    jmp .copy_path

.connect_now:
    # First byte of path might need to be 0 for abstract?
    # Usually /tmp/.X11-unix/X0 is a file socket. Length = 2 + strlen.
    # Strlen("/tmp/.X11-unix/X0") = 17. Total = 19.
    movq x11_fd(%rip), %rdi
    leaq x11_sockaddr(%rip), %rsi
    movq $110, %rdx # Safe size
    call __mf_net_connect

    testq %rax, %rax
    jnz .fail_connect

    # 3. Try to read Xauthority (optional)
    call read_xauthority
    # rax = 0 if success, -1 if failed
    # Negate to get auth_available flag (0 = no auth, 1 = has auth)
    negq %rax
    incq %rax
    # Now rax = 1 if success, 0 if failed

    # Build handshake with or without auth
    movq %rax, %rdi
    call build_handshake
    # rdi = buffer ptr, rsi = buffer len

    # Send Handshake
    pushq %rsi  # Save length
    pushq %rdi  # Save buffer ptr
    movq x11_fd(%rip), %rdi
    popq %rsi   # buffer ptr
    popq %rdx   # length
    call __mf_net_send

    # 4. Receive Server Response
    # Header: 8 bytes. [0]=1 (Success), [1]=Unused, [2..3]=Major, [4..5]=Minor, [6..7]=AddLen (4-byte units)
    movq x11_fd(%rip), %rdi
    leaq x11_buffer(%rip), %rsi
    movq $8, %rdx
    call __mf_net_recv

    movb x11_buffer(%rip), %al
    cmpb $1, %al
    jne .fail_auth

    # Read Additional Data
    # AddLen is at offset 6 (u16).
    leaq x11_buffer(%rip), %rbx
    movzwq 6(%rbx), %rax # Additional Data Length (quads)
    imulq $4, %rax       # Bytes

    # Read the rest (Vendor info + Roots)
    # We need to read 'rax' bytes.
    # For now, just read 4096 or less.
    pushq %rax # Save len
    movq x11_fd(%rip), %rdi
    leaq x11_buffer(%rip), %rsi
    movq $4096, %rdx
    call __mf_net_recv
    popq %rcx # Len needed

    # Parse Setup Info
    # Structure:
    # 0-3: Release
    # 4-7: Resource ID Base (SAVE THIS!)
    # 8-11: Resource ID Mask (SAVE THIS!)
    # 12-15: Motion Buffer Size
    # 16-17: Vendor Len
    # 18-19: Max Req Len
    # 20-20: Roots Count
    # 21-21: Formats Count
    # 22-23: Image Byte Order
    # 24-25: Bitmap Bit Order
    # 26-27: Scanline Unit/Pad
    # 28-31: Min/Max Keycode
    # 32-35: Unused
    # 36...: Vendor Str (Pad 4)
    # ...  : Formats
    # ...  : Roots (Screen Info)

    leaq x11_buffer(%rip), %rbx
    movl 4(%rbx), %eax
    movl %eax, x11_id_base(%rip)
    movl 8(%rbx), %eax
    movl %eax, x11_id_mask(%rip)

    # Calculate offset to Roots
    # Offset = 32 + Align(VendorLen) + (FormatsCount * 8)
    movzwq 16(%rbx), %r8 # Vendor Len
    addq $3, %r8
    andq $-4, %r8 # Align 4

    movzbq 21(%rbx), %r9 # Formats Count
    imulq $8, %r9

    addq $32, %rbx
    addq %r8, %rbx
    addq %r9, %rbx # Now at First Root (Screen)

    # Root Window ID is at offset 0 of Root structure
    movl (%rbx), %eax
    movl %eax, x11_root_id(%rip)

    # 5. Generate Window ID
    movl x11_id_base(%rip), %eax
    inc %eax
    movl %eax, x11_my_win_id(%rip)

    # 6. CreateWindow Request
    # Opcode 1 (CreateWindow). Len 8+attrs.
    # [0] 1 (Op), [1] Depth, [2-3] Len
    # [4] WinID
    # [8] Parent (Root)
    # [12] X, [14] Y
    # [16] W, [18] H
    # [20] BorderW
    # [22] Class (1=InputOutput)
    # [24] VisualID (Copy from Root or 0 for CopyFromParent)
    # [28] ValueMask
    # [32] Values...

    leaq x11_buffer(%rip), %rbx

    movb $1, 0(%rbx) # Opcode CreateWindow
    movb $0, 1(%rbx) # Depth (CopyFromParent)
    movw $8, 2(%rbx) # Length (8 quads = 32 bytes)

    movl x11_my_win_id(%rip), %eax
    movl %eax, 4(%rbx)

    movl x11_root_id(%rip), %eax
    movl %eax, 8(%rbx)

    movw $100, 12(%rbx) # X
    movw $100, 14(%rbx) # Y
    movw %r12w, 16(%rbx) # W
    movw %r13w, 18(%rbx) # H
    movw $0, 20(%rbx)   # Border
    movw $1, 22(%rbx)   # Class (InputOutput)
    movl $0, 24(%rbx)   # Visual (CopyFromParent) - Safe for all color depths

    movl $0x802, 28(%rbx) # Mask: BG_PIXEL (2) | EVENT_MASK (0x800)
    # Values: BgPixel, EventMask
    # Wait, values must follow bit order. 2 is bit 1. 800 is bit 11.
    # Order: BG_PIXEL, then EVENT_MASK.

    # We assume White/Black pixel is 0 or 1. Use 0 (Black).
    # But wait, Root has White/Black pixel.
    # For now, pass 0.

    # Need to expand request size?
    # Request is 8 units (32 bytes). Header (28) + Mask (4). Values start at 32?
    # No. Fixed part is 32 bytes (up to Mask).
    # Values follow.
    # So if we have values, Length must increase.
    # 2 values = 8 bytes. Total 40 bytes (10 units).

    movw $10, 2(%rbx) # Update Len

    movl $0x00FF0000, 32(%rbx) # BgPixel (Red?) - Actually depends on visual. 0 is safe.
    # EventMask: Exposure (0x8000) | KeyPress (1)
    movl $0x8001, 36(%rbx)

    # Send CreateWindow
    pushq %rbx
    movq x11_fd(%rip), %rdi
    movq %rbx, %rsi
    movq $40, %rdx
    call __mf_net_send
    popq %rbx

    # 7. CreateGC (Graphics Context) for drawing
    # Opcode 55.
    movl x11_id_base(%rip), %eax
    addl $2, %eax
    movl %eax, x11_my_gc_id(%rip)

    movb $55, 0(%rbx)
    movb $0, 1(%rbx)
    movw $4, 2(%rbx) # Len 4 (16 bytes)
    movl %eax, 4(%rbx) # GC ID
    movl x11_my_win_id(%rip), %eax
    movl %eax, 8(%rbx) # Drawable
    movl $0, 12(%rbx) # Mask (None)

    # Send CreateGC
    pushq %rbx
    movq x11_fd(%rip), %rdi
    movq %rbx, %rsi
    movq $16, %rdx
    call __mf_net_send
    popq %rbx

    # 8. MapWindow
    # Opcode 8.
    movb $8, 0(%rbx)
    movb $0, 1(%rbx)
    movw $2, 2(%rbx) # Len 2 (8 bytes)
    movl x11_my_win_id(%rip), %eax
    movl %eax, 4(%rbx)

    # Send MapWindow
    movq x11_fd(%rip), %rdi
    movq %rbx, %rsi
    movq $8, %rdx
    call __mf_net_send

    movq x11_fd(%rip), %rax # Return FD/Handle

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

.fail_open:
    movq $-1, %rax
    jmp .create_ret
.fail_connect:
    movq $-2, %rax
    jmp .create_ret
.fail_auth:
    movq $-3, %rax
.create_ret:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

# ------------------------------------------------------------------------------
# func __mf_draw_pixel(handle: i64, x: i64, y: i64, color: i64)
# Note: Handle is FD? Or Window ID?
# We used FD as handle in create. But we need Window ID for X11 requests.
# We stored x11_my_win_id globally. Use that.
# Color: Change GC Foreground first.
# ------------------------------------------------------------------------------
__mf_draw_pixel:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    pushq %r12 # X
    pushq %r13 # Y
    pushq %r14 # Color

    movq %rsi, %r12
    movq %rdx, %r13
    movq %rcx, %r14

    leaq x11_buffer(%rip), %rbx

    # 1. ChangeGC (Foreground)
    # Opcode 56.
    movb $56, 0(%rbx)
    movb $0, 1(%rbx)
    movw $3, 2(%rbx) # Len 3 (12 bytes)
    movl x11_my_gc_id(%rip), %eax
    movl %eax, 4(%rbx)
    movl $4, 8(%rbx) # Mask: Foreground (bit 2)

    # Value (Color)
    movl %r14d, 12(%rbx)

    # Increase Len if needed? No, struct is fixed + values.
    # Request: Header(4) + GC(4) + Mask(4) + Val(4) = 16 bytes.
    # Len should be 4.
    movw $4, 2(%rbx)

    # Send ChangeGC
    pushq %rbx
    movq x11_fd(%rip), %rdi
    movq %rbx, %rsi
    movq $16, %rdx
    call __mf_net_send
    popq %rbx

    # 2. PolyPoint
    # Opcode 64. Mode 0 (Origin).
    movb $64, 0(%rbx)
    movb $0, 1(%rbx) # CoordModeOrigin
    movw $3, 2(%rbx) # Len 3 (12 bytes) -> Header(4) + Drawable(4) + GC(4) + Point(4) = 16?
    # Request:
    # 1 Op, 1 Mode, 2 Len
    # 4 Drawable
    # 4 GC
    # 4 Points (x, y) - 2 bytes each
    # Total 16 bytes = 4 units.

    movw $4, 2(%rbx)
    movl x11_my_win_id(%rip), %eax
    movl %eax, 4(%rbx)
    movl x11_my_gc_id(%rip), %eax
    movl %eax, 8(%rbx)

    movw %r12w, 12(%rbx) # X
    movw %r13w, 14(%rbx) # Y

    # Send PolyPoint
    movq x11_fd(%rip), %rdi
    movq %rbx, %rsi
    movq $16, %rdx
    call __mf_net_send

    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret

# ------------------------------------------------------------------------------
# func __mf_window_poll(event_ptr: ptr) -> event_type (0=None, 1=Quit, 2=Key, 3=Mouse, 4=Click)
# ------------------------------------------------------------------------------
__mf_window_poll:
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx
    subq $24, %rsp      # Space for pollfd + align

    # Input: RDI = event_ptr
    movq %rdi, %rbx     # Save event_ptr in RBX

    # Setup pollfd struct
    movq x11_fd(%rip), %rax
    movl %eax, -16(%rbp)        # fd
    movw $1, -12(%rbp)          # events = POLLIN (1)
    movw $0, -10(%rbp)          # revents = 0

    leaq -16(%rbp), %rdi        # fds ptr
    movq $1, %rsi               # nfds
    movq $0, %rdx               # timeout (0ms)
    movq $SYS_POLL, %rax
    syscall

    cmpq $0, %rax
    jle .poll_none

    movzwl -10(%rbp), %eax
    testl $1, %eax              # POLLIN?
    jz .poll_none

    # Read XEvent (32 bytes)
    movq x11_fd(%rip), %rdi
    leaq x11_buffer(%rip), %rsi
    movq $32, %rdx
    call __mf_net_recv

    testq %rax, %rax
    jle .poll_err

    # Check Event Type
    leaq x11_buffer(%rip), %r8
    movb (%r8), %al
    andb $0x7F, %al

    cmpb $2, %al                # KeyPress (2)
    je .poll_key
    cmpb $4, %al                # ButtonPress (4)
    je .poll_click
    cmpb $6, %al                # MotionNotify (6)
    je .poll_mouse
    cmpb $33, %al               # ClientMessage (33)
    je .poll_quit_msg

    # Ignore others (Expose, etc), return 0 to keep loop
    xorq %rax, %rax
    jmp .poll_ret

.poll_key:
    testq %rbx, %rbx
    jz .poll_key_ret

    # Extract Keycode (Offset 1, byte)
    # Just simplistic for now
    movq $2, 0(%rbx) # Type = KEY
    movzbq 1(%r8), %rcx
    movq %rcx, 8(%rbx) # Key Code

.poll_key_ret:
    movq $2, %rax
    jmp .poll_ret

.poll_click:
    testq %rbx, %rbx
    jz .poll_click_ret

    # Extract X, Y from ButtonPress event
    # Structure: Type(1), Serial(4), SendEvent(1), Display(4), Window(4), Root(4), SubWindow(4), Time(4), X(2), Y(2), ...
    # Offsets in XButtonEvent (32bit):
    # 0: type
    # ...
    # 24: x (int? no, short?) -> Actually X11 protocol structure:
    # 1 byte type, 1 byte detail (button)
    # 2 bytes seq
    # 4 bytes time
    # 4 bytes root
    # 4 bytes event
    # 4 bytes child
    # 2 bytes rootX
    # 2 bytes rootY
    # 2 bytes eventX
    # 2 bytes eventY
    # 2 bytes state
    # 1 byte sameScreen
    # 1 byte pad

    # Offset of eventX = 24?
    # 1+1+2+4+4+4+4+2+2 = 24. Yes.

    movq $4, 0(%rbx) # Type = CLICK

    # X (Offset 24, signed short)
    movswq 24(%r8), %rcx
    movq %rcx, 16(%rbx) # Mouse X

    # Y (Offset 26, signed short)
    movswq 26(%r8), %rcx
    movq %rcx, 24(%rbx) # Mouse Y

.poll_click_ret:
    movq $4, %rax
    jmp .poll_ret

.poll_mouse:
    testq %rbx, %rbx
    jz .poll_mouse_ret

    movq $3, 0(%rbx) # Type = MOUSE

    # MotionNotify struct is similar to ButtonPress for X/Y
    movswq 24(%r8), %rcx
    movq %rcx, 16(%rbx) # Mouse X

    movswq 26(%r8), %rcx
    movq %rcx, 24(%rbx) # Mouse Y

.poll_mouse_ret:
    movq $3, %rax
    jmp .poll_ret

.poll_quit_msg:
    movq $1, %rax
    jmp .poll_ret

.poll_err:
    movq $1, %rax
    jmp .poll_ret

.poll_none:
    xorq %rax, %rax

.poll_ret:
    addq $24, %rsp
    popq %rbx
    popq %rbp
    ret

# ------------------------------------------------------------------------------
# func __mf_fill_rect(x: i64, y: i64, w: i64, h: i64, color: i64)
# ------------------------------------------------------------------------------
__mf_fill_rect:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12 # X
    pushq %r13 # Y
    pushq %r14 # W (Counter X)
    pushq %r15 # H (Counter Y)
    pushq %rbx # Color
    subq $8, %rsp # Align

    movq %rdi, %r12 # Start X
    movq %rsi, %r13 # Start Y
    movq %rdx, %r14 # W
    movq %rcx, %r15 # H
    movq %r8, %rbx  # Color

    # Loop Y
    xorq %r9, %r9 # Counter Y
.fill_loop_y:
    cmpq %r15, %r9
    jge .fill_done

    # Loop X
    xorq %r10, %r10 # Counter X
.fill_loop_x:
    cmpq %r14, %r10
    jge .fill_next_y

    # Draw Pixel (StartX + CtrX, StartY + CtrY)
    movq %r12, %rsi
    addq %r10, %rsi # X

    movq %r13, %rdx
    addq %r9, %rdx # Y

    movq %rbx, %rcx # Color

    pushq %r9
    pushq %r10
    call __mf_draw_pixel
    popq %r10
    popq %r9

    incq %r10
    jmp .fill_loop_x

.fill_next_y:
    incq %r9
    jmp .fill_loop_y

.fill_done:
    addq $8, %rsp
    popq %rbx
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

# ------------------------------------------------------------------------------
# func __mf_draw_rect(x: i64, y: i64, w: i64, h: i64, color: i64)
# Outline Only.
# ------------------------------------------------------------------------------
__mf_draw_rect:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12 # X
    pushq %r13 # Y
    pushq %r14 # W
    pushq %r15 # H
    pushq %rbx # Color
    subq $8, %rsp

    movq %rdi, %r12
    movq %rsi, %r13
    movq %rdx, %r14
    movq %rcx, %r15
    movq %r8, %rbx

    # Top Line (x, y, w, 1)
    movq %r12, %rdi
    movq %r13, %rsi
    movq %r14, %rdx
    movq $1, %rcx
    movq %rbx, %r8
    call __mf_fill_rect

    # Bottom Line (x, y+h-1, w, 1)
    movq %r12, %rdi
    movq %r13, %rsi
    addq %r15, %rsi
    decq %rsi
    movq %r14, %rdx
    movq $1, %rcx
    movq %rbx, %r8
    call __mf_fill_rect

    # Left Line (x, y, 1, h)
    movq %r12, %rdi
    movq %r13, %rsi
    movq $1, %rdx
    movq %r15, %rcx
    movq %rbx, %r8
    call __mf_fill_rect

    # Right Line (x+w-1, y, 1, h)
    movq %r12, %rdi
    addq %r14, %rdi
    decq %rdi
    movq %r13, %rsi
    movq $1, %rdx
    movq %r15, %rcx
    movq %rbx, %r8
    call __mf_fill_rect

    addq $8, %rsp
    popq %rbx
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

# ------------------------------------------------------------------------------
# func __mf_draw_char(x: i64, y: i64, char: i64, color: i64)
# Draws a single 8x8 character using __mf_font_bitmap.
# ------------------------------------------------------------------------------
__mf_draw_char:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12 # X
    pushq %r13 # Y
    pushq %r14 # Char
    pushq %r15 # Color
    pushq %rbx # Bitmap Ptr / Row

    movq %rdi, %r12
    movq %rsi, %r13
    movq %rdx, %r14
    movq %rcx, %r15

    # Get Bitmap Address
    # Base + (Char * 8)
    leaq __mf_font_bitmap(%rip), %rbx
    imulq $8, %r14
    addq %r14, %rbx

    # Loop Rows (0-7)
    xorq %r8, %r8 # Row Index

.char_row_loop:
    cmpq $8, %r8
    jge .char_done

    movb (%rbx, %r8), %al # Load Row Byte

    # Loop Bits (0-7)
    # Bit 7 is left-most pixel (x).
    movq $7, %r9 # Bit Index

.char_bit_loop:
    cmpq $-1, %r9
    je .char_row_next

    bt %r9, %rax
    jnc .char_bit_skip

    # Draw Pixel at (X + (7-Bit), Y + Row)
    # Calcluate bit pos: 7 - r9
    movq $7, %r10
    subq %r9, %r10

    # ABI Fix: Stack Alignment
    # Prologue pushed 48 bytes (Aligned 16).
    # We need to save 3 regs (24 bytes). Misaligned by 8.
    # Fix: Push dummy/padding (8 bytes) -> 32 bytes total.

    pushq %rax # Save Bitmap Row
    pushq %r8
    pushq %r9
    subq $8, %rsp # Padding for Alignment

    # Setup args for __mf_draw_pixel(handle, x, y, color)
    # Handle is global x11_my_win_id ?? No, wait.
    # __mf_draw_pixel takes HANDLE as first arg?
    # No, our impl uses FD implicitly stored in globals?
    # Wait, check __mf_draw_pixel impl.
    # It takes (handle, x, y, color). Handle (RDI) is passed but ignored?
    # "Note: Handle is FD? Or Window ID? We stored x11_my_win_id globally. Use that."
    # The code uses globals. But we should pass something valid.

    movq $0, %rdi # Handle (Ignored/Global)

    movq %r12, %rsi
    addq %r10, %rsi # X + Offset

    movq %r13, %rdx
    addq %r8, %rdx # Y + Row

    movq %r15, %rcx # Color

    call __mf_draw_pixel

    addq $8, %rsp # Remove Padding
    popq %r9
    popq %r8
    popq %rax

.char_bit_skip:
    decq %r9
    jmp .char_bit_loop

.char_row_next:
    incq %r8
    jmp .char_row_loop

.char_done:
    popq %rbx
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret

# ------------------------------------------------------------------------------
# func __mf_draw_string(x: i64, y: i64, str_ptr: ptr, color: i64)
# Draws a null-terminated string.
# ------------------------------------------------------------------------------
__mf_draw_string:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12 # X
    pushq %r13 # Y
    pushq %r14 # Str Ptr
    pushq %r15 # Color

    movq %rdi, %r12
    movq %rsi, %r13
    movq %rdx, %r14
    movq %rcx, %r15

.str_loop:
    movzbq (%r14), %rdx
    testq %rdx, %rdx
    jz .str_done

    # Draw Char(x, y, char, color)
    movq %r12, %rdi
    movq %r13, %rsi
    # RDX already has char
    movq %r15, %rcx
    call __mf_draw_char

    # Advance X (8 pixels)
    addq $8, %r12
    incq %r14
    jmp .str_loop

.str_done:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret
