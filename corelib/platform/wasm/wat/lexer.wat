(module
  ;; ===========================================================================
  ;; LEXER (Platform: WASM)
  ;; ===========================================================================

  (import "env" "mem_alloc" (func $mem_alloc (param i64) (result i64)))
  (import "env" "memory" (memory 10))

  ;; ---------------------------------------------------------------------------
  ;; CONSTANTS
  ;; ---------------------------------------------------------------------------
  (global $TOKEN_EOF i64 (i64.const 0))
  (global $TOKEN_INTEGER i64 (i64.const 1))
  (global $TOKEN_STRING i64 (i64.const 3))
  (global $TOKEN_IDENTIFIER i64 (i64.const 4))
  (global $TOKEN_SYMBOL i64 (i64.const 5))
  (global $TOKEN_KEYWORD i64 (i64.const 6))
  (global $TOKEN_MARKER i64 (i64.const 8)) ;; ###

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
                         (local.get $vec) (local.get $token_count) (global.get $TOKEN_MARKER) (i64.const 0) (local.get $line) (local.get $col)
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
                (local.get $vec) (local.get $token_count) (global.get $TOKEN_INTEGER)
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
                (local.get $vec) (local.get $token_count) (global.get $TOKEN_STRING) (local.get $str_ptr) (local.get $line) (local.get $col)
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
                (local.get $vec) (local.get $token_count) (global.get $TOKEN_IDENTIFIER) (local.get $str_ptr) (local.get $line) (local.get $col)
             )
             (local.set $token_count (i64.add (local.get $token_count) (i64.const 1)))
             (br $loop_top)
          )
        )

        ;; SYMBOLS
        (call $write_token
            (local.get $vec) (local.get $token_count) (global.get $TOKEN_SYMBOL) (i64.extend_i32_u (local.get $char)) (local.get $line) (local.get $col)
        )
        (local.set $curr (i32.add (local.get $curr) (i32.const 1)))
        (local.set $col (i64.add (local.get $col) (i64.const 1)))
        (local.set $token_count (i64.add (local.get $token_count) (i64.const 1)))

      )
    )

    (call $write_token
        (local.get $vec) (local.get $token_count) (global.get $TOKEN_EOF) (i64.const 0) (local.get $line) (local.get $col)
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
