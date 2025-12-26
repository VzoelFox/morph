# FoxVM WASM Backend Design

## WASM Compilation Strategy

### Instruction Mapping
```
Morph/FoxVM → WASM
LOAD_CONST  → i32.const
LOAD_LOCAL  → local.get
STORE_LOCAL → local.set
ADD         → i32.add
SUB         → i32.sub
MUL         → i32.mul
CALL        → call
RETURN      → return
```

### WASM Module Structure
```wasm
(module
  ;; Memory for heap
  (memory (export "memory") 1)
  
  ;; Stack pointer global
  (global $sp (mut i32) (i32.const 1024))
  
  ;; Main function
  (func $main (export "main")
    ;; Generated code here
  )
  
  ;; Runtime functions
  (func $print (import "env" "print") (param i32))
  (func $gc_alloc (import "env" "gc_alloc") (param i32) (result i32))
)
```

### Compilation Example
```fox
# Morph source
var x = 10 + 20
native_print_int(x)
```

```wasm
(func $main (export "main")
  (local $x i32)
  
  ;; x = 10 + 20
  i32.const 10
  i32.const 20
  i32.add
  local.set $x
  
  ;; native_print_int(x)
  local.get $x
  call $print
)
```

### Performance Characteristics
- **WASM overhead**: ~2-5x slower than native
- **Startup time**: Instant (no JIT warmup)
- **Memory usage**: Linear memory model
- **Portability**: Runs everywhere (browser, Node.js, WASI)
