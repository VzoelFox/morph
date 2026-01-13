(module
  ;; ===========================================================================
  ;; RUNTIME (Platform: WASM) v1.1
  ;; ===========================================================================
  ;; Manages Memory and Stack.
  ;; Parity with Native v1.1: 48-byte Page Header, Bump Pointer logic within Page.

  ;; Memory: 1 Page = 64KB. Start with 10 pages.
  (memory $memory (export "memory") 10)

  ;; Global Globals
  ;; Page Size is 64KB (65536) in WASM.
  ;; We simulate "Pages" inside Linear Memory?
  ;; Or we just treat the whole Linear Memory as one big Arena?
  ;; Native uses linked list of 4KB/Variable pages.
  ;; For WASM v1.1, let's keep it simple: One Giant Page (The Heap).
  ;; BUT we need the 48-byte Header parity if we want snapshot compatibility?
  ;; No, WASM Snapshot is just serializing the Memory object.
  ;; The key is `mem_alloc` header (8 bytes) for compatibility with Native data structures.

  (global $heap_base (mut i32) (i32.const 65536)) ;; Start Heap at 64KB (Reserve first 64KB for Stack/Static)
  (global $stack_ptr (mut i32) (i32.const 65520)) ;; Stack grows down from 64KB

  ;; Header Constants
  (global $BLOCK_HEADER_SIZE i32 (i32.const 8))

  ;; ---------------------------------------------------------------------------
  ;; MEMORY ALLOCATOR (Bump Pointer with 8-byte Header)
  ;; ---------------------------------------------------------------------------
  ;; func mem_alloc(size: i64) -> ptr (i64)

  (func $mem_alloc (export "mem_alloc") (param $user_size i64) (result i64)
    (local $ptr i32)
    (local $total_size i32)
    (local $new_base i32)
    (local $aligned_size i32)

    ;; 1. Calculate Total Size: User + 8
    (local.set $total_size
      (i32.add (i32.wrap_i64 (local.get $user_size)) (global.get $BLOCK_HEADER_SIZE))
    )

    ;; 2. Align to 16 bytes
    ;; (size + 15) & -16
    (local.set $aligned_size
      (i32.and
        (i32.add (local.get $total_size) (i32.const 15))
        (i32.const -16)
      )
    )

    ;; 3. Current Heap Base
    (local.set $ptr (global.get $heap_base))

    ;; 4. New Base
    (local.set $new_base (i32.add (local.get $ptr) (local.get $aligned_size)))

    ;; 5. Check OOM (Simple check against max memory, e.g. 10 * 64KB)
    ;; For now, just grow if needed? WASM `memory.grow`.
    ;; If new_base > current_pages * 64KB, grow.
    (if (i32.gt_u (local.get $new_base) (i32.mul (memory.size) (i32.const 65536)))
      (then
        ;; Grow by 1 page at least
        (drop (memory.grow (i32.const 1)))
      )
    )

    ;; 6. Write Header (User Size)
    (i64.store (local.get $ptr) (local.get $user_size))

    ;; 7. Zero Out Padding? (Optional for now, Native does it)

    ;; 8. Update Global
    (global.set $heap_base (local.get $new_base))

    ;; 9. Return User Ptr (Ptr + 8)
    (i64.extend_i32_u (i32.add (local.get $ptr) (global.get $BLOCK_HEADER_SIZE)))
  )

  ;; ---------------------------------------------------------------------------
  ;; STACK ALLOCATOR (Safe Stack)
  ;; ---------------------------------------------------------------------------
  ;; Used by MorphStack. Just delegates to mem_alloc.

  (func $stack_new (export "stack_new") (param $size i64) (result i64)
    ;; Alloc Struct (32 bytes)
    (local $struct_ptr i64)
    (local $buffer_ptr i64)
    (local $base_addr i64)

    (local.set $struct_ptr (call $mem_alloc (i64.const 32)))

    ;; Alloc Buffer
    (local.set $buffer_ptr (call $mem_alloc (local.get $size)))

    ;; Setup Struct
    ;; [0] SP = Base
    ;; [8] Base = Start + Size
    ;; [16] Limit = Start

    (local.set $base_addr (i64.add (local.get $buffer_ptr) (local.get $size)))

    (i64.store (i32.wrap_i64 (local.get $struct_ptr)) (local.get $base_addr)) ;; SP
    (i64.store (i32.add (i32.wrap_i64 (local.get $struct_ptr)) (i32.const 8)) (local.get $base_addr)) ;; Base
    (i64.store (i32.add (i32.wrap_i64 (local.get $struct_ptr)) (i32.const 16)) (local.get $buffer_ptr)) ;; Limit

    (local.get $struct_ptr)
  )
)
