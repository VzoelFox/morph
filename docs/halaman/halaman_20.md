# Halaman 20: Intent Tree Self-Hosting - Phase 2 Implementation

**Status: In Progress**
**Tanggal Mulai: 2026-01-12**
**Timeline: 4 weeks**
**Repository: morph**

Dokumen ini mendokumentasikan implementasi Phase 2: Intent Tree self-hosting menggunakan MorphFox.

## 1. Overview

Setelah bootstrap compiler dibekukan di v1.2-bootstrap, kita memulai Phase 2 dengan fokus pada **Intent Tree builder** yang ditulis dalam MorphFox sendiri.

### 1.1 Goals

- ✅ Implement Intent Tree builder API in MorphFox
- ⏳ Test memory builtins dengan real use cases
- ⏳ Create simple Intent nodes (48 bytes)
- ⏳ Build small Intent trees
- ⏳ Validate tree structure matches Assembly output

### 1.2 Architecture

```
Current State:
  Lexer (Asm) → Parser (Asm) → Intent Builder (Asm) → Codegen (Asm)
                                       ↓
                                  Bootstrap binary

Target State (Phase 2):
  Lexer (Asm) → Parser (Asm) → Intent Builder (MorphFox) → Codegen (Asm)
                                       ↓
                              Self-hosting component!
```

## 2. Memory Builtins Testing

### 2.1 Builtin Functions

```fox
; Memory allocation
fungsi __mf_mem_alloc(size: i64) -> ptr
fungsi __mf_mem_free(ptr: ptr, size: i64) -> void

; 64-bit memory access
fungsi __mf_load_i64(addr: ptr) -> i64
fungsi __mf_poke_i64(addr: ptr, value: i64) -> void

; Byte memory access
fungsi __mf_load_byte(addr: ptr) -> i64
fungsi __mf_poke_byte(addr: ptr, value: i64) -> void
```

### 2.2 Test Cases

**Test 1: Simple Allocation**
```fox
utama {
    var size = 48
    var ptr = __mf_mem_alloc(size)

    __mf_print_asciz("Allocated at: ")
    __mf_print_int_raw(ptr)
    __mf_print_asciz("\n")

    0
}
```

**Test 2: Write and Read i64**
```fox
utama {
    var ptr = __mf_mem_alloc(48)

    ; Write node type (INTENT_FRAG_LITERAL = 0x3001)
    __mf_poke_i64(ptr, 0x3001)

    ; Read back
    var type = __mf_load_i64(ptr)

    __mf_print_asciz("Node type: ")
    __mf_print_int_raw(type)
    __mf_print_asciz("\n")

    0
}
```

**Test 3: Intent Node Structure**
```fox
utama {
    var node = __mf_mem_alloc(48)

    ; Write node fields (offsets from intent.fox)
    __mf_poke_i64(node + 0,  0x3001)  ; Type: INTENT_FRAG_LITERAL
    __mf_poke_i64(node + 8,  0)       ; Next: NULL
    __mf_poke_i64(node + 16, 0)       ; Child: NULL
    __mf_poke_i64(node + 24, 0)       ; Hint: NULL
    __mf_poke_i64(node + 32, 42)      ; Data A: Literal value
    __mf_poke_i64(node + 40, 0)       ; Data B: unused

    ; Verify
    var type = __mf_load_i64(node + 0)
    var value = __mf_load_i64(node + 32)

    __mf_print_asciz("Created node type=")
    __mf_print_int_raw(type)
    __mf_print_asciz(", value=")
    __mf_print_int_raw(value)
    __mf_print_asciz("\n")

    0
}
```

### 2.3 Test Results

| Test | Platform | Status | Notes |
|------|----------|--------|-------|
| Simple alloc | Linux x86-64 | ⏳ Pending | - |
| Write/Read | Linux x86-64 | ⏳ Pending | - |
| Node structure | Linux x86-64 | ⏳ Pending | - |
| Simple alloc | Windows | ⏳ Pending | - |
| Simple alloc | WASM | ⏳ Pending | - |

## 3. Intent Tree Builder Implementation

### 3.1 Core API

Implemented in `morph/src/intent_builder.fox`:

```fox
; Constants
var INTENT_UNIT_MODULE = 0x1001
var INTENT_SHARD_FUNC = 0x2001
var INTENT_FRAG_LITERAL = 0x3001
var INTENT_FRAG_VAR = 0x3005
var INTENT_FRAG_ASSIGN = 0x3006
; ... etc

var INTENT_NODE_SIZE = 48

; Core functions
fungsi intent_new_node(type: i64) -> ptr {
    var node = mem_alloc(INTENT_NODE_SIZE)

    ; Initialize all fields to zero/null
    poke_i64(node + 0, type)      ; Type
    poke_i64(node + 8, 0)         ; Next
    poke_i64(node + 16, 0)        ; Child
    poke_i64(node + 24, 0)        ; Hint
    poke_i64(node + 32, 0)        ; Data A
    poke_i64(node + 40, 0)        ; Data B

    kembali node
}

fungsi intent_set_next(node: ptr, next: ptr) -> void {
    poke_i64(node + 8, next)
}

fungsi intent_set_child(node: ptr, child: ptr) -> void {
    poke_i64(node + 16, child)
}

fungsi intent_set_data_a(node: ptr, value: i64) -> void {
    poke_i64(node + 32, value)
}

fungsi intent_set_data_b(node: ptr, value: i64) -> void {
    poke_i64(node + 40, value)
}
```

### 3.2 Convenience Builders

```fox
; Create literal node
fungsi intent_new_literal(value: i64) -> ptr {
    var node = intent_new_node(INTENT_FRAG_LITERAL)
    intent_set_data_a(node, value)
    kembali node
}

; Create variable access node
fungsi intent_new_var(name_ptr: ptr) -> ptr {
    var node = intent_new_node(INTENT_FRAG_VAR)
    intent_set_data_a(node, name_ptr)
    kembali node
}

; Create binary operation node
fungsi intent_new_binary(op: i64, left: ptr, right: ptr) -> ptr {
    var node = intent_new_node(INTENT_FRAG_BINARY)
    intent_set_data_a(node, op)
    intent_set_child(node, left)
    intent_set_next(left, right)
    kembali node
}

; Create assignment node
fungsi intent_new_assign(name_ptr: ptr, value: ptr) -> ptr {
    var node = intent_new_node(INTENT_FRAG_ASSIGN)
    intent_set_data_a(node, name_ptr)
    intent_set_child(node, value)
    kembali node
}
```

### 3.3 Example Programs

**Example 1: Single Literal**
```fox
utama {
    ; Create: literal(42)
    var lit = intent_new_literal(42)

    ; Verify
    var type = load_i64(lit + 0)
    var value = load_i64(lit + 32)

    __mf_print_asciz("Literal node: type=")
    __mf_print_int_raw(type)
    __mf_print_asciz(", value=")
    __mf_print_int_raw(value)
    __mf_print_asciz("\n")

    0
}
```

**Example 2: Variable Assignment**
```fox
utama {
    ; Create: x = 42
    var lit = intent_new_literal(42)
    var assign = intent_new_assign("x", lit)

    ; Verify structure
    var type = load_i64(assign + 0)
    var child = load_i64(assign + 16)

    __mf_print_asciz("Assignment node created\n")
    __mf_print_asciz("  Type: ")
    __mf_print_int_raw(type)
    __mf_print_asciz("\n")
    __mf_print_asciz("  Child ptr: ")
    __mf_print_int_raw(child)
    __mf_print_asciz("\n")

    0
}
```

**Example 3: Binary Expression**
```fox
utama {
    ; Create: x + 42
    var var_x = intent_new_var("x")
    var lit = intent_new_literal(42)
    var binop = intent_new_binary(43, var_x, lit)  ; 43 = ASCII '+'

    ; Verify
    var type = load_i64(binop + 0)
    var op = load_i64(binop + 32)
    var child = load_i64(binop + 16)

    __mf_print_asciz("Binary op node:\n")
    __mf_print_asciz("  Type: ")
    __mf_print_int_raw(type)
    __mf_print_asciz("\n")
    __mf_print_asciz("  Operator: ")
    __mf_print_int_raw(op)
    __mf_print_asciz("\n")

    0
}
```

## 4. Integration Strategy

### 4.1 Current Pipeline (Bootstrap)

```
Source Code (.fox)
      ↓
  Lexer (Asm)  ────→  Tokens
      ↓
  Parser (Asm) ────→  Intent Tree (Asm-built)
      ↓
  Compiler (Asm) ──→  RPN Bytecode
      ↓
  Executor (Asm) ──→  Execution
```

### 4.2 Target Pipeline (Phase 2)

```
Source Code (.fox)
      ↓
  Lexer (Asm)  ────→  Tokens
      ↓
  Parser (Asm) ────→  Token stream
      ↓
  Intent Builder  ──→  Intent Tree (MorphFox-built)
  (MorphFox)
      ↓
  Compiler (Asm) ──→  RPN Bytecode
      ↓
  Executor (Asm) ──→  Execution
```

### 4.3 Integration Points

1. **Parser output** → Intent Builder input
   - Parser emits tokens or simplified AST
   - Intent Builder consumes and builds tree

2. **Intent Tree** → Compiler input
   - Tree format must match Assembly version
   - Same 48-byte node structure
   - Same Type/Next/Child/Data layout

3. **Verification**
   - Compare MorphFox-built tree with Assembly-built tree
   - Check byte-for-byte equivalence
   - Validate with existing test suite

## 5. Challenges & Solutions

### 5.1 Challenge: No String Type

**Problem**: MorphFox v0 only has i64 and ptr. No proper string manipulation.

**Solution**:
- Use raw pointers for string literals
- Implement basic string helpers in stdlib
- Future: Add String type in v2

### 5.2 Challenge: No Dynamic Arrays

**Problem**: Can't easily store lists of child nodes.

**Solution**:
- Use linked list via Next pointers (already in Intent design)
- Implement Vector type in stdlib if needed
- Keep trees simple initially

### 5.3 Challenge: Manual Memory Management

**Problem**: No garbage collection, must track allocations.

**Solution**:
- Use arena allocator for batch cleanup
- Keep tree lifetimes simple
- Future: Add GC in v2

### 5.4 Challenge: Limited Debugging

**Problem**: No debugger, stack traces, or print helpers.

**Solution**:
- Add print_node() helper
- Use __mf_print_int_raw() extensively
- Build incrementally, test often

## 6. Testing Strategy

### 6.1 Unit Tests

Test individual functions:
- [ ] intent_new_node() allocates 48 bytes
- [ ] intent_set_next() writes to offset 8
- [ ] intent_set_child() writes to offset 16
- [ ] intent_new_literal() sets type correctly

### 6.2 Integration Tests

Test small trees:
- [ ] Single literal node
- [ ] Assignment (var = literal)
- [ ] Binary expression (var + literal)
- [ ] Linked statements (assign1 → assign2)
- [ ] Function with body

### 6.3 Comparison Tests

Compare with Assembly parser:
- [ ] Parse simple.fox with Assembly → tree1
- [ ] Build tree manually with MorphFox → tree2
- [ ] Compare tree1 and tree2 structure
- [ ] Verify byte-for-byte match

## 7. Metrics & Progress

### 7.1 Completion Checklist

**Week 1** (Current):
- [x] Memory builtins implemented (Linux/Windows/WASM)
- [x] Intent Tree API designed
- [ ] Builtins tested with real programs
- [ ] Simple node creation working

**Week 2**:
- [ ] All convenience builders implemented
- [ ] Unit tests passing
- [ ] Can build literal/var/binary nodes
- [ ] Tree linking (Next pointers) working

**Week 3**:
- [ ] Complex trees (functions, assignments)
- [ ] Integration tests passing
- [ ] Comparison with Assembly output
- [ ] Bug fixes and refinement

**Week 4**:
- [ ] Full integration with parser
- [ ] All test suite passing
- [ ] Documentation complete
- [ ] Phase 2 complete! 🎉

### 7.2 Success Metrics

| Metric | Target | Current |
|--------|--------|---------|
| **Builtins coverage** | 6/6 functions | 6/6 implemented |
| **Platform coverage** | 3/3 platforms | 3/3 implemented |
| **Node types** | 9 types | 9 defined |
| **Test programs** | 10+ | 0 passing |
| **Tree examples** | 5+ | 0 built |

## 8. Next Steps

### 8.1 Immediate (This Week)

1. **Test builtins**
   - Run test programs on Linux
   - Verify allocation works
   - Test read/write operations

2. **Create first node**
   - Use intent_new_literal(42)
   - Verify 48 bytes allocated
   - Check Type field = 0x3001

3. **Build simple tree**
   - Create assignment (x = 42)
   - Link nodes with Next/Child
   - Print tree structure

### 8.2 Short-term (Next 2 Weeks)

1. **Expand test coverage**
2. **Implement all convenience builders**
3. **Add tree printing/debugging**
4. **Compare with Assembly output**

### 8.3 Long-term (Weeks 3-4)

1. **Parser integration**
2. **Full test suite**
3. **Performance testing**
4. **Phase 2 completion**

## 9. Documentation Updates

### 9.1 Files Added

- `morph/src/intent_builder.fox` - Intent Tree API
- `morph/corelib/lib/memory.fox` - Memory helpers
- `morph/docs/INTENT_SELFHOST.md` - Strategy doc
- `morphfox/corelib/platform/*/builtins.*` - Memory builtins
- `morphfox/corelib/platform/wasm/wat/builtins.wat` - New file

### 9.2 Files Modified

- `morph/corelib/core/builtins.fox` - Added memory function specs
- `morphfox/docs/halaman/halaman_19.md` - Bootstrap freeze doc (this file)
- `morphfox/docs/halaman/halaman_20.md` - Phase 2 doc (this file)

## 10. Conclusion

Phase 2 implementation is underway with:
- ✅ Memory builtins implemented (all platforms)
- ✅ Intent Tree API designed
- ✅ Documentation complete
- ⏳ Testing in progress

**Next**: Run tests, create first Intent nodes, build simple trees.

Timeline: **4 weeks** to Intent Tree self-hosting complete.

---

**Status**: 🚧 In Progress - Week 1
**Last Updated**: 2026-01-12
**Next Review**: End of Week 1
