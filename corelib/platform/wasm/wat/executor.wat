(module
  ;; ===========================================================================
  ;; EXECUTOR (Platform: WASM) v1.2
  ;; ===========================================================================
  ;; Parity with Native v1.2: Crypto, File I/O (Syscall 6 args), Mem Ops.

  (import "env" "memory" (memory 10))
  ;; Syscall: Intent + 6 Args
  (import "env" "syscall" (func $syscall (param i64 i64 i64 i64 i64 i64 i64) (result i64)))
  (import "env" "mem_alloc" (func $mem_alloc (param i64) (result i64)))
  (import "env" "stack_new" (func $stack_new (param i64) (result i64)))

  ;; ---------------------------------------------------------------------------
  ;; STACK HELPERS
  ;; ---------------------------------------------------------------------------
  (func $stack_push (param $stack_ptr i64) (param $val i64)
    (local $sp i32)
    (local $limit i32)
    (local $new_sp i32)

    (local.set $sp (i32.wrap_i64 (i64.load (i32.wrap_i64 (local.get $stack_ptr)))))
    (local.set $limit (i32.wrap_i64 (i64.load (i32.add (i32.wrap_i64 (local.get $stack_ptr)) (i32.const 16)))))

    (local.set $new_sp (i32.sub (local.get $sp) (i32.const 8)))

    (if (i32.lt_u (local.get $new_sp) (local.get $limit))
      (then (unreachable))
    )

    (i64.store (local.get $new_sp) (local.get $val))
    (i64.store (i32.wrap_i64 (local.get $stack_ptr)) (i64.extend_i32_u (local.get $new_sp)))
  )

  (func $stack_pop (param $stack_ptr i64) (result i64)
    (local $sp i32)
    (local $base i32)
    (local $val i64)

    (local.set $sp (i32.wrap_i64 (i64.load (i32.wrap_i64 (local.get $stack_ptr)))))
    (local.set $base (i32.wrap_i64 (i64.load (i32.add (i32.wrap_i64 (local.get $stack_ptr)) (i32.const 8)))))

    (if (i32.ge_u (local.get $sp) (local.get $base))
      (then (unreachable))
    )

    (local.set $val (i64.load (local.get $sp)))
    (i64.store (i32.wrap_i64 (local.get $stack_ptr)) (i64.extend_i32_u (i32.add (local.get $sp) (i32.const 8))))

    (local.get $val)
  )

  ;; ---------------------------------------------------------------------------
  ;; EXECUTE
  ;; ---------------------------------------------------------------------------
  (func $execute (export "execute") (param $code_ptr i64) (param $len i64)
    (local $stack i64)
    (local $opcode i64)
    (local $operand i64)
    (local $current_ip i32)
    (local $end_ip i32)
    (local $tmp_a i64)
    (local $tmp_b i64)
    (local $intent i64)
    (local $arg1 i64)
    (local $arg2 i64)
    (local $arg3 i64)
    (local $arg4 i64)
    (local $arg5 i64)
    (local $arg6 i64)

    (local.set $stack (call $stack_new (i64.const 8192)))
    (local.set $current_ip (i32.wrap_i64 (local.get $code_ptr)))
    (local.set $end_ip (i32.add (local.get $current_ip) (i32.wrap_i64 (local.get $len))))

    (block $break_loop
      (loop $loop_top
        (br_if $break_loop (i32.ge_u (local.get $current_ip) (local.get $end_ip)))

        (local.set $opcode (i64.load (local.get $current_ip)))
        (local.set $operand (i64.load (i32.add (local.get $current_ip) (i32.const 8))))
        (local.set $current_ip (i32.add (local.get $current_ip) (i32.const 16)))

        ;; OP_LIT (1)
        (if (i64.eq (local.get $opcode) (i64.const 1))
          (then
            (call $stack_push (local.get $stack) (local.get $operand))
            (br $loop_top)
          )
        )

        ;; OP_LOAD (2)
        (if (i64.eq (local.get $opcode) (i64.const 2))
          (then
            ;; TODO: Symbol Table Load
            (br $loop_top)
          )
        )

        ;; OP_STORE (3)
        (if (i64.eq (local.get $opcode) (i64.const 3))
          (then
            ;; TODO: Symbol Table Store
            (local.set $tmp_a (call $stack_pop (local.get $stack)))
            (br $loop_top)
          )
        )

        ;; OP_ADD (10)
        (if (i64.eq (local.get $opcode) (i64.const 10))
          (then
            (local.set $tmp_b (call $stack_pop (local.get $stack)))
            (local.set $tmp_a (call $stack_pop (local.get $stack)))
            (call $stack_push (local.get $stack) (i64.add (local.get $tmp_a) (local.get $tmp_b)))
            (br $loop_top)
          )
        )

        ;; OP_SUB (11)
        (if (i64.eq (local.get $opcode) (i64.const 11))
          (then
            (local.set $tmp_b (call $stack_pop (local.get $stack)))
            (local.set $tmp_a (call $stack_pop (local.get $stack)))
            (call $stack_push (local.get $stack) (i64.sub (local.get $tmp_a) (local.get $tmp_b)))
            (br $loop_top)
          )
        )

        ;; OP_MUL (12)
        (if (i64.eq (local.get $opcode) (i64.const 12))
          (then
            (local.set $tmp_b (call $stack_pop (local.get $stack)))
            (local.set $tmp_a (call $stack_pop (local.get $stack)))
            (call $stack_push (local.get $stack) (i64.mul (local.get $tmp_a) (local.get $tmp_b)))
            (br $loop_top)
          )
        )

        ;; OP_JMP (30)
        (if (i64.eq (local.get $opcode) (i64.const 30))
          (then
             ;; Operand is Offset
             (local.set $current_ip (i32.add (local.get $current_ip) (i32.wrap_i64 (local.get $operand))))
             (br $loop_top)
          )
        )

        ;; OP_JMP_FALSE (32)
        (if (i64.eq (local.get $opcode) (i64.const 32))
          (then
             (local.set $tmp_a (call $stack_pop (local.get $stack)))
             (if (i64.eq (local.get $tmp_a) (i64.const 0))
               (then
                 (local.set $current_ip (i32.add (local.get $current_ip) (i32.wrap_i64 (local.get $operand))))
               )
             )
             (br $loop_top)
          )
        )

        ;; OP_EXIT (35)
        (if (i64.eq (local.get $opcode) (i64.const 35))
          (then (br $break_loop))
        )

        ;; OP_SYSCALL (40)
        (if (i64.eq (local.get $opcode) (i64.const 40))
          (then
            (local.set $intent (call $stack_pop (local.get $stack)))
            (local.set $arg6 (call $stack_pop (local.get $stack)))
            (local.set $arg5 (call $stack_pop (local.get $stack)))
            (local.set $arg4 (call $stack_pop (local.get $stack)))
            (local.set $arg3 (call $stack_pop (local.get $stack)))
            (local.set $arg2 (call $stack_pop (local.get $stack)))
            (local.set $arg1 (call $stack_pop (local.get $stack)))

            (call $stack_push (local.get $stack)
              (call $syscall
                (local.get $intent)
                (local.get $arg1)
                (local.get $arg2)
                (local.get $arg3)
                (local.get $arg4)
                (local.get $arg5)
                (local.get $arg6)
              )
            )
            (br $loop_top)
          )
        )

        ;; OP_MEM_READ (45)
        (if (i64.eq (local.get $opcode) (i64.const 45))
          (then
            (local.set $tmp_a (call $stack_pop (local.get $stack))) ;; Addr
            (local.set $tmp_b (i64.load (i32.wrap_i64 (local.get $tmp_a))))
            (call $stack_push (local.get $stack) (local.get $tmp_b))
            (br $loop_top)
          )
        )

        ;; OP_MEM_WRITE (46)
        (if (i64.eq (local.get $opcode) (i64.const 46))
          (then
            (local.set $tmp_b (call $stack_pop (local.get $stack))) ;; Value
            (local.set $tmp_a (call $stack_pop (local.get $stack))) ;; Addr
            (i64.store (i32.wrap_i64 (local.get $tmp_a)) (local.get $tmp_b))
            (br $loop_top)
          )
        )

        ;; OP_PRINT (90)
        (if (i64.eq (local.get $opcode) (i64.const 90))
          (then
             ;; No-Op in WASM or call console.log via syscall?
             (br $loop_top)
          )
        )
      )
    )
  )
)
