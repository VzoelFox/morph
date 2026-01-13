(module
  ;; ===========================================================================
  ;; MEMORY BUILTINS (Platform: WASM) - Intent Tree Support
  ;; ===========================================================================
  ;; Memory operation wrappers for MorphFox Intent Tree building.
  ;; WASM has built-in memory operations, so these are thin wrappers.
  ;; ===========================================================================

  ;; Import memory from runtime module
  (import "runtime" "memory" (memory 10))
  (import "runtime" "mem_alloc" (func $mem_alloc (param i64) (result i64)))
  (import "runtime" "mem_free" (func $mem_free (param i64 i64)))

  ;; -------------------------------------------------------------------------
  ;; func __mf_mem_alloc(size: i64) -> ptr (i64)
  ;; Wrapper around mem_alloc
  ;; -------------------------------------------------------------------------
  (func $__mf_mem_alloc (export "__mf_mem_alloc") (param $size i64) (result i64)
    (call $mem_alloc (local.get $size))
  )

  ;; -------------------------------------------------------------------------
  ;; func __mf_mem_free(ptr: ptr, size: i64) -> void
  ;; Wrapper around mem_free
  ;; -------------------------------------------------------------------------
  (func $__mf_mem_free (export "__mf_mem_free") (param $ptr i64) (param $size i64)
    (call $mem_free (local.get $ptr) (local.get $size))
  )

  ;; -------------------------------------------------------------------------
  ;; func __mf_load_i64(addr: ptr) -> i64
  ;; Load 64-bit integer from memory address
  ;; -------------------------------------------------------------------------
  (func $__mf_load_i64 (export "__mf_load_i64") (param $addr i64) (result i64)
    ;; WASM i64.load: Load 8 bytes from memory at address
    ;; Requires i32 address, so wrap from i64
    (i64.load (i32.wrap_i64 (local.get $addr)))
  )

  ;; -------------------------------------------------------------------------
  ;; func __mf_poke_i64(addr: ptr, value: i64) -> void
  ;; Store 64-bit integer to memory address
  ;; -------------------------------------------------------------------------
  (func $__mf_poke_i64 (export "__mf_poke_i64") (param $addr i64) (param $value i64)
    ;; WASM i64.store: Store 8 bytes to memory at address
    (i64.store (i32.wrap_i64 (local.get $addr)) (local.get $value))
  )

  ;; -------------------------------------------------------------------------
  ;; func __mf_load_byte(addr: ptr) -> i64
  ;; Load single byte from memory (zero-extended to i64)
  ;; -------------------------------------------------------------------------
  (func $__mf_load_byte (export "__mf_load_byte") (param $addr i64) (result i64)
    ;; WASM i32.load8_u: Load 1 byte (unsigned) from memory
    ;; Extend to i64
    (i64.extend_i32_u
      (i32.load8_u (i32.wrap_i64 (local.get $addr)))
    )
  )

  ;; -------------------------------------------------------------------------
  ;; func __mf_poke_byte(addr: ptr, value: i64) -> void
  ;; Store single byte to memory address
  ;; -------------------------------------------------------------------------
  (func $__mf_poke_byte (export "__mf_poke_byte") (param $addr i64) (param $value i64)
    ;; WASM i32.store8: Store 1 byte to memory
    ;; Only lowest 8 bits of value are used
    (i32.store8
      (i32.wrap_i64 (local.get $addr))
      (i32.wrap_i64 (local.get $value))
    )
  )
)
