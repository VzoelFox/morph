(module
  ;; Imports
  (import "env" "sys_write" (func $sys_write (param i64 i64 i64) (result i64)))
  (import "env" "sys_dom_create" (func $sys_dom_create (param i64) (result i64)))
  (import "env" "sys_dom_append" (func $sys_dom_append (param i64 i64)))
  (import "env" "sys_dom_set_attr" (func $sys_dom_set_attr (param i64 i64 i64)))
  (import "env" "sys_dom_set_text" (func $sys_dom_set_text (param i64 i64)))
  (import "env" "sys_dom_get_by_id" (func $sys_dom_get_by_id (param i64) (result i64)))
  (import "env" "sys_exit" (func $sys_exit (param i64)))

  ;; Memory (10 Pages = 640KB)
  (memory $memory (export "memory") 10)

  ;; Globals
  (global $heap_base (mut i32) (i32.const 65536))   ;; Heap starts at 64KB
  (global $stack_ptr (mut i32) (i32.const 65536))   ;; Stack Top at 64KB
  (global $sp (mut i32) (i32.const 0))              ;; Morph VM SP (Internal)
  (global $stack_base (mut i32) (i32.const 65536))  ;; Morph Stack Base
  (global $ip (mut i32) (i32.const 0))

  ;; Constants for Lexer
  (global $TOKEN_EOF i64 (i64.const 0))
  (global $TOKEN_INTEGER i64 (i64.const 1))
  (global $TOKEN_STRING i64 (i64.const 3))
  (global $TOKEN_IDENTIFIER i64 (i64.const 4))
  (global $TOKEN_SYMBOL i64 (i64.const 5))
  (global $TOKEN_KEYWORD i64 (i64.const 6))
  ;; ===========================================================================
  ;; RUNTIME (Platform: WASM)
  ;; ===========================================================================
  ;; Manages Memory and Stack.

  ;; Memory: 1 Page = 64KB. Start with 10 pages.

  ;; Global Globals
  ;; Actually, let's put stack at the end of the first few pages or grow up.
  ;; Convention: Linear Memory
  ;; 0 - 1024: Reserved / Null
  ;; 1024 - ...: Heap (grows up)
  ;; Let's define a separate stack region or just use heap for MorphStack?
  ;; The Morph VM uses a custom "MorphStack" structure allocated on the heap.
  ;; But we need a system stack for the WASM execution itself (call stack).
  ;; WASM has its own implicit call stack.
  ;; We need `mem_alloc`.

  ;; ---------------------------------------------------------------------------
  ;; MEMORY ALLOCATOR (Bump Pointer)
  ;; ---------------------------------------------------------------------------
  ;; Simplest allocator: never frees, just bumps.
  ;; func mem_alloc(size: i64) -> ptr (i64)

  (func $mem_alloc (export "mem_alloc") (param $size i64) (result i64)
    (local $ptr i32)
    (local $new_base i32)

    ;; Load current heap base

    ;; Calculate new base: ptr + size
    (local.set $new_base
      (i32.add
        (local.get $ptr)
        (i32.wrap_i64 (local.get $size)) ;; Wrap i64 to i32 for address calculation
      )
    )

    ;; Update global heap base

    ;; Return original ptr extended to i64
    (i64.extend_i32_u (local.get $ptr))
  )
  ;; ===========================================================================
  ;; SYSCALLS (Platform: WASM)
  ;; ===========================================================================
  ;; Bridges Morph Intents to JavaScript.

  ;; Imports from Host (env)

  ;; Intent IDs (Must match SSOT)
  ;; 1: WRITE
  ;; 6: EXIT
  ;; 100: DOM_CREATE
  ;; 101: DOM_APPEND
  ;; 102: DOM_SET_ATTR
  ;; 103: DOM_SET_TEXT
  ;; 105: DOM_GET_BY_ID

  ;; Dispatcher
  ;; func syscall(id: i64, arg1: i64, arg2: i64, arg3: i64) -> result (i64)
  (func $syscall (export "syscall") (param $id i64) (param $a1 i64) (param $a2 i64) (param $a3 i64) (result i64)
    (local $res i64)

    (block $block_ret
      (block $block_default
        (block $case_105
          (block $case_103
            (block $case_102
              (block $case_101
                (block $case_100
                  (block $case_6
                    (block $case_1
                      (br_table $case_1 $block_default $block_default $block_default $block_default $case_6
                                ;; ... filling gaps is hard with br_table if IDs are sparse.
                                ;; Use if/else for sparse IDs.
                                (i32.wrap_i64 (local.get $id))
                      )
                    )
                  )
                )
              )
            )
          )
        )
      )
    )

    ;; Simple If/Else Chain for sparse IDs

    ;; ID 1: WRITE (fd, ptr, len)
    (if (i64.eq (local.get $id) (i64.const 1))
      (then
        (call $sys_write (local.get $a1) (local.get $a2) (local.get $a3))
        return
      )
    )

    ;; ID 6: EXIT (code)
    (if (i64.eq (local.get $id) (i64.const 6))
      (then
        (call $sys_exit (local.get $a1))
        (i64.const 0) ;; Void return
        return
      )
    )

    ;; ID 100: DOM_CREATE (tag_ptr)
    (if (i64.eq (local.get $id) (i64.const 100))
      (then
        (call $sys_dom_create (local.get $a1))
        return
      )
    )

    ;; ID 101: DOM_APPEND (parent, child)
    (if (i64.eq (local.get $id) (i64.const 101))
      (then
        (call $sys_dom_append (local.get $a1) (local.get $a2))
        (i64.const 0)
        return
      )
    )

    ;; ID 102: DOM_SET_ATTR (el, key, val)
    (if (i64.eq (local.get $id) (i64.const 102))
      (then
        (call $sys_dom_set_attr (local.get $a1) (local.get $a2) (local.get $a3))
        (i64.const 0)
        return
      )
    )

    ;; ID 103: DOM_SET_TEXT (el, text)
    (if (i64.eq (local.get $id) (i64.const 103))
      (then
        (call $sys_dom_set_text (local.get $a1) (local.get $a2))
        (i64.const 0)
        return
      )
    )

    ;; ID 105: DOM_GET_BY_ID (id_str)
    (if (i64.eq (local.get $id) (i64.const 105))
      (then
        (call $sys_dom_get_by_id (local.get $a1))
        return
      )
    )

    (i64.const -1) ;; Error: Unknown Syscall
  )
  ;; ===========================================================================
  ;; EXECUTOR (Platform: WASM)
  ;; ===========================================================================



  ;; ---------------------------------------------------------------------------
  ;; STACK HELPERS
  ;; ---------------------------------------------------------------------------

  (func $push (param $val i64)
    (local $ptr i32)
  )

  (func $pop (result i64)
    (local $ptr i32)
    (local $val i64)

    ;; Decrement SP
    (local.set $ptr (i32.sub (local.get $ptr) (i32.const 8)))

    ;; Load value to local

    (local.get $val)
  )

  ;; ---------------------------------------------------------------------------
  ;; EXECUTOR RUN LOOP
  ;; ---------------------------------------------------------------------------
  (func $execute (export "execute") (param $code_ptr i64) (param $len i64)
    (local $opcode i64)
    (local $operand i64)
    (local $current_ip i32)
    (local $end_ip i32)
    (local $intent i64)
    (local $arg1 i64)
    (local $arg2 i64)
    (local $arg3 i64)

    (local.set $current_ip (i32.wrap_i64 (local.get $code_ptr)))
    (local.set $end_ip (i32.add (local.get $current_ip) (i32.wrap_i64 (local.get $len))))

    (block $break_loop
      (loop $loop_top
        (br_if $break_loop (i32.ge_u (local.get $current_ip) (local.get $end_ip)))

        (local.set $opcode (i64.load (local.get $current_ip)))
        (local.set $operand (i64.load (i32.add (local.get $current_ip) (i32.const 8))))
        (local.set $current_ip (i32.add (local.get $current_ip) (i32.const 16)))

        ;; DISPATCH
        (if (i64.eq (local.get $opcode) (i64.const 0))
          (then
            (call $push (local.get $operand))
            (br $loop_top)
          )
        )

        (if (i64.eq (local.get $opcode) (i64.const 10))
          (then
            (call $push (i64.add (call $pop) (call $pop)))
            (br $loop_top)
          )
        )

        (if (i64.eq (local.get $opcode) (i64.const 11))
          (then
            (local.set $operand (call $pop))
            (call $push (i64.sub (call $pop) (local.get $operand)))
            (br $loop_top)
          )
        )

        (if (i64.eq (local.get $opcode) (i64.const 40))
          (then
            (local.set $intent (call $pop))
            (local.set $arg1 (call $pop))
            (local.set $arg2 (call $pop))
            (local.set $arg3 (call $pop))

            (call $push
              (call $syscall
                (local.get $intent)
                (local.get $arg1)
                (local.get $arg2)
                (local.get $arg3)
              )
            )
            (br $loop_top)
          )
        )
      )
    )
  )
  ;; ===========================================================================
  ;; LEXER (Platform: WASM)
  ;; ===========================================================================


  ;; ---------------------------------------------------------------------------
  ;; CONSTANTS
  ;; ---------------------------------------------------------------------------

  ;; Helpers
  (func $is_digit (param $char i32) (result i32)
    (i32.and (i32.ge_u (local.get $char) (i32.const 48)) (i32.le_u (local.get $char) (i32.const 57)))
  )

  (func $is_alpha (param $char i32) (result i32)
    (i32.or
      (i32.and (i32.ge_u (local.get $char) (i32.const 65)) (i32.le_u (local.get $char) (i32.const 90))) ;; A-Z
      (i32.and (i32.ge_u (local.get $char) (i32.const 97)) (i32.le_u (local.get $char) (i32.const 122))) ;; a-z
    )
  )

  (func $is_whitespace (param $char i32) (result i32)
    (i32.or
      (i32.eq (local.get $char) (i32.const 32))
      (i32.or (i32.eq (local.get $char) (i32.const 10)) (i32.eq (local.get $char) (i32.const 13)))
    )
  )

  ;; ---------------------------------------------------------------------------
  ;; TOKENIZER
  ;; ---------------------------------------------------------------------------
  (func $tokenize (export "tokenize") (param $src i64) (param $len i64) (result i64)
    (local $vec i64)
    (local $curr i32)
    (local $end i32)
    (local $char i32)
    (local $next1 i32)
    (local $next2 i32)
    (local $start i32)
    (local $line i64)
    (local $col i64)
    (local $token_count i64)
    (local $str_ptr i64)

    (local.set $vec (call $mem_alloc (i64.const 32768)))
    (local.set $curr (i32.wrap_i64 (local.get $src)))
    (local.set $end (i32.add (local.get $curr) (i32.wrap_i64 (local.get $len))))
    (local.set $line (i64.const 1))
    (local.set $col (i64.const 1))
    (local.set $token_count (i64.const 0))

    (block $break_loop
      (loop $loop_top
        (br_if $break_loop (i32.ge_u (local.get $curr) (local.get $end)))

        (local.set $char (i32.load8_u (local.get $curr)))

        ;; WHITESPACE
        (if (call $is_whitespace (local.get $char))
          (then
            (if (i32.eq (local.get $char) (i32.const 10))
              (then
                (local.set $line (i64.add (local.get $line) (i64.const 1)))
                (local.set $col (i64.const 1))
              )
              (else
                (local.set $col (i64.add (local.get $col) (i64.const 1)))
              )
            )
            (local.set $curr (i32.add (local.get $curr) (i32.const 1)))
            (br $loop_top)
          )
        )

        ;; MARKER (###)
        (if (i32.eq (local.get $char) (i32.const 35)) ;; '#'
          (then
             ;; Peek next 2 chars
             (if (i32.lt_u (i32.add (local.get $curr) (i32.const 2)) (local.get $end))
               (then
                 (local.set $next1 (i32.load8_u (i32.add (local.get $curr) (i32.const 1))))
                 (local.set $next2 (i32.load8_u (i32.add (local.get $curr) (i32.const 2))))

                 (if (i32.and (i32.eq (local.get $next1) (i32.const 35)) (i32.eq (local.get $next2) (i32.const 35)))
                   (then
                      (call $write_token
                      )
                      (local.set $curr (i32.add (local.get $curr) (i32.const 3)))
                      (local.set $col (i64.add (local.get $col) (i64.const 3)))
                      (local.set $token_count (i64.add (local.get $token_count) (i64.const 1)))
                      (br $loop_top)
                   )
                 )
               )
             )
          )
        )

        ;; INTEGER
        (if (call $is_digit (local.get $char))
          (then
             (call $write_token
                (call $parse_int (local.get $curr) (local.get $end))
                (local.get $line) (local.get $col)
             )
             (block $digit_scan
               (loop $d_loop
                 (br_if $digit_scan (i32.ge_u (local.get $curr) (local.get $end)))
                 (local.set $char (i32.load8_u (local.get $curr)))
                 (br_if $digit_scan (i32.eqz (call $is_digit (local.get $char))))
                 (local.set $curr (i32.add (local.get $curr) (i32.const 1)))
                 (local.set $col (i64.add (local.get $col) (i64.const 1)))
                 (br $d_loop)
               )
             )
             (local.set $token_count (i64.add (local.get $token_count) (i64.const 1)))
             (br $loop_top)
          )
        )

        ;; STRING (" or ')
        (if (i32.or (i32.eq (local.get $char) (i32.const 34)) (i32.eq (local.get $char) (i32.const 39)))
          (then
             (local.set $curr (i32.add (local.get $curr) (i32.const 1)))
             (local.set $col (i64.add (local.get $col) (i64.const 1)))
             (local.set $start (local.get $curr))
             (block $str_scan
                (loop $s_loop
                   (br_if $str_scan (i32.ge_u (local.get $curr) (local.get $end)))
                   (local.set $char (i32.load8_u (local.get $curr)))
                   (br_if $str_scan (i32.or (i32.eq (local.get $char) (i32.const 34)) (i32.eq (local.get $char) (i32.const 39))))
                   (local.set $curr (i32.add (local.get $curr) (i32.const 1)))
                   (local.set $col (i64.add (local.get $col) (i64.const 1)))
                   (br $s_loop)
                )
             )
             (local.set $str_ptr (call $alloc_string_copy (local.get $start) (i32.sub (local.get $curr) (local.get $start))))
             (local.set $curr (i32.add (local.get $curr) (i32.const 1)))
             (local.set $col (i64.add (local.get $col) (i64.const 1)))
             (call $write_token
             )
             (local.set $token_count (i64.add (local.get $token_count) (i64.const 1)))
             (br $loop_top)
          )
        )

        ;; IDENTIFIER (Alpha start)
        (if (call $is_alpha (local.get $char))
          (then
             (local.set $start (local.get $curr))
             (block $id_scan
               (loop $id_loop
                 (br_if $id_scan (i32.ge_u (local.get $curr) (local.get $end)))
                 (local.set $char (i32.load8_u (local.get $curr)))
                 ;; Identifiers can contain digits or underscores after first char
                 (br_if $id_scan
                    (i32.eqz
                        (i32.or
                            (call $is_alpha (local.get $char))
                            (i32.or (call $is_digit (local.get $char)) (i32.eq (local.get $char) (i32.const 95))) ;; '_'
                        )
                    )
                 )
                 (local.set $curr (i32.add (local.get $curr) (i32.const 1)))
                 (local.set $col (i64.add (local.get $col) (i64.const 1)))
                 (br $id_loop)
               )
             )
             ;; Alloc String
             (local.set $str_ptr (call $alloc_string_copy (local.get $start) (i32.sub (local.get $curr) (local.get $start))))
             ;; Write Token
             (call $write_token
             )
             (local.set $token_count (i64.add (local.get $token_count) (i64.const 1)))
             (br $loop_top)
          )
        )

        ;; SYMBOLS
        (call $write_token
        )
        (local.set $curr (i32.add (local.get $curr) (i32.const 1)))
        (local.set $col (i64.add (local.get $col) (i64.const 1)))
        (local.set $token_count (i64.add (local.get $token_count) (i64.const 1)))

      )
    )

    (call $write_token
    )

    (local.get $vec)
  )

  (func $alloc_string_copy (param $start i32) (param $len i32) (result i64)
    (local $ptr i64)
    (local $dest i32)
    (local $src i32)
    (local $end i32)
    (local.set $ptr (call $mem_alloc (i64.extend_i32_u (i32.add (local.get $len) (i32.const 1)))))
    (local.set $dest (i32.wrap_i64 (local.get $ptr)))
    (local.set $src (local.get $start))
    (local.set $end (i32.add (local.get $start) (local.get $len)))
    (block $copy_done
      (loop $copy_loop
        (br_if $copy_done (i32.ge_u (local.get $src) (local.get $end)))
        (i32.store8 (local.get $dest) (i32.load8_u (local.get $src)))
        (local.set $src (i32.add (local.get $src) (i32.const 1)))
        (local.set $dest (i32.add (local.get $dest) (i32.const 1)))
        (br $copy_loop)
      )
    )
    (i32.store8 (local.get $dest) (i32.const 0))
    (local.get $ptr)
  )

  (func $parse_int (param $start i32) (param $end i32) (result i64)
    (local $val i64)
    (local $curr i32)
    (local $char i32)
    (local.set $val (i64.const 0))
    (local.set $curr (local.get $start))
    (block $done
      (loop $loop
         (br_if $done (i32.ge_u (local.get $curr) (local.get $end)))
         (local.set $char (i32.load8_u (local.get $curr)))
         (br_if $done (i32.eqz (call $is_digit (local.get $char))))
         (local.set $val (i64.mul (local.get $val) (i64.const 10)))
         (local.set $val (i64.add (local.get $val) (i64.extend_i32_u (i32.sub (local.get $char) (i32.const 48)))))
         (local.set $curr (i32.add (local.get $curr) (i32.const 1)))
         (br $loop)
      )
    )
    (local.get $val)
  )

  (func $write_token (param $vec i64) (param $idx i64) (param $type i64) (param $val i64) (param $line i64) (param $col i64)
    (local $offset i32)
    (local.set $offset
        (i32.add (i32.wrap_i64 (local.get $vec)) (i32.wrap_i64 (i64.mul (local.get $idx) (i64.const 32))))
    )
    (i64.store (local.get $offset) (local.get $type))
    (i64.store (i32.add (local.get $offset) (i32.const 8)) (local.get $val))
    (i64.store (i32.add (local.get $offset) (i32.const 16)) (local.get $line))
    (i64.store (i32.add (local.get $offset) (i32.const 24)) (local.get $col))
  )
)
