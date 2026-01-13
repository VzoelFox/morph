(module
  ;; ===========================================================================
  ;; SYSCALLS (Platform: WASM)
  ;; ===========================================================================
  ;; Bridges Morph Intents to JavaScript.

  ;; Imports from Host (env)
  (import "env" "sys_write" (func $sys_write (param i64 i64 i64) (result i64)))
  (import "env" "sys_dom_create" (func $sys_dom_create (param i64) (result i64)))
  (import "env" "sys_dom_append" (func $sys_dom_append (param i64 i64)))
  (import "env" "sys_dom_set_attr" (func $sys_dom_set_attr (param i64 i64 i64)))
  (import "env" "sys_dom_set_text" (func $sys_dom_set_text (param i64 i64)))
  (import "env" "sys_dom_get_by_id" (func $sys_dom_get_by_id (param i64) (result i64)))
  (import "env" "sys_exit" (func $sys_exit (param i64)))

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
)
