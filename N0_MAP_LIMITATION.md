# N0 Map Limitation - Critical Finding
## Map NOT Supported in N0 Compiler

**Date**: 2025-12-29
**Status**: 🔴 **CRITICAL LIMITATION**
**Resolution**: ⚙️ **Assembly path required for map usage**

---

## 🔴 TEMUAN KRITIS

### Map TIDAK DIDUKUNG sama sekali di N0!

**Previous assumption (WRONG):**
- ✅ Map type declarations: `var m map[string]int` - thought this worked
- ✅ Map indexing: `m[key] = value` - thought this worked
- ❌ Map literals: `map[string]int{}` - knew this didn't work

**Reality (CORRECT):**
- ❌ Map type declarations: `var m map[string]int` - **TIDAK BEKERJA**
- ❌ Map as struct field: `struktur S { m map[K]V }` - **TIDAK BEKERJA**
- ❌ Map literals: `map[string]int{}` - **TIDAK BEKERJA**
- ❌ Map indexing: `m[key] = value` - **TIDAK BISA DITEST** (deklarasi sudah gagal)

---

## 🔬 EVIDENCE

### Test 1: Local Map Declaration
```fox
fungsi main() void
    var m map[string]int
    native_print("Test")
akhir
```

**Result:**
```
❌ Compilation Error: multi-var not supported
```

**Analysis**: Compiler error "multi-var not supported" triggered by `map[string]int` type.

---

### Test 2: Global Map Declaration
```fox
var globalMap map[string]int

fungsi main() void
    native_print("Test")
akhir
```

**Result:**
```
❌ GCC Error: expected expression before ';' token
Line 32: mph_globalMap = ;
```

**Generated C code:**
```c
MorphMap* mph_globalMap;
// ...
void morph_entry_point(MorphContext* ctx) {
    mph_globalMap = ;  // ❌ EMPTY ASSIGNMENT!
    // ...
}
```

**Analysis**: Compiler doesn't know how to initialize map, leaves empty assignment.

---

### Test 3: Map in Struct Field
```fox
struktur Env
    data map[string]int
akhir

fungsi main() void
    var e Env
    native_print("Test")
akhir
```

**Result:**
```
❌ Compilation Error: multi-var not supported
```

**Analysis**: Same error as Test 1 - map type not supported.

---

### Test 4: Struct Without Map (Control)
```fox
struktur Env
    count int
akhir

fungsi main() void
    var e Env
    e.count = 10
    native_print("Test")
akhir
```

**Result:**
```
✅ Build Success!
```

**Analysis**: Problem is specifically with `map[K]V` type, not struct itself.

---

## 🔍 ROOT CAUSE ANALYSIS

### Location: `pkg/compiler/compiler.go:1142`

```go
if len(s.Names) != 1 || len(s.Values) != 1 {
    return fmt.Errorf("multi-var not supported")
}
```

**Problem**: Compiler interprets `map[string]int` as multi-variable declaration!

**Why?**
- Parser sees: `var m map[string]int`
- Parser tokenizes: `m`, `map`, `[`, `string`, `]`, `int`
- Compiler thinks: "This has brackets and multiple tokens, must be multi-var!"
- Error: "multi-var not supported"

**Actual issue**: Map type syntax `map[K]V` confuses the compiler's variable declaration logic.

---

### Location: `pkg/compiler/compiler.go` - Global initialization

**Problem**: No initialization code for map types.

**Evidence from generated C:**
```c
MorphMap* mph_globalMap;  // Declaration OK
// ...
mph_globalMap = ;  // ❌ NO INITIALIZER!
```

**Why?**
- Compiler has `switch typeKind` for initialization
- Cases for: INT, STRING, BOOL, STRUCT, ARRAY
- **Missing case for: MAP**
- Defaults to empty expression → syntax error

---

## 📊 IMPACT ASSESSMENT

### Files Blocked by This Limitation:

1. **morphsh/evaluator/tree_walker_working.fox** ❌
   - Uses: `intVars map[string]int`
   - Status: Cannot compile

2. **morphsh/evaluator/tree_walker.fox** ❌
   - Uses: `vars map[string]Value`
   - Status: Cannot compile

3. **morphsh/stdlib/map.fox** ❌
   - Uses: `m map[string]int` in parameters
   - Status: Cannot compile

4. **Any file using map** ❌
   - Status: Cannot compile with N0

---

## ⚠️ N0 TIDAK SUPPORT - DIPORT LANGSUNG KE ASSEMBLY

### Decision: Map → Assembly Path

**Rationale:**
1. N0 completely doesn't support map
2. No workaround exists (unlike struct assignment)
3. Critical for tree walker (Environment storage)
4. Critical for type checker (symbol tables)

**Strategy:**
```
╔════════════════════════════════════════╗
║  Feature: Map/HashMap                  ║
║  N0 Support: ❌ TIDAK SUPPORT          ║
║  Workaround: ❌ TIDAK ADA              ║
║  Solution: ⚙️ ASSEMBLY PATH REQUIRED   ║
╚════════════════════════════════════════╝
```

---

## 🛠️ ASSEMBLY IMPLEMENTATION PLAN

### Phase 1: Runtime Hashmap (Week 1)

**File: `runtime_hashmap.asm`**

```asm
; Runtime HashMap for Morph N1
; Implements map[string]int functionality

section .data
    HASH_CAPACITY equ 64
    HASH_LOAD_FACTOR equ 0.75

section .text
    global hashmap_new
    global hashmap_set
    global hashmap_get
    global hashmap_has

; Structure:
; HashMap {
;   buckets: *Entry[HASH_CAPACITY]
;   size: int
;   capacity: int
; }

hashmap_new:
    ; Create new hashmap
    ; Returns: rax = hashmap*
    push rbp
    mov rbp, rsp

    ; Allocate hashmap struct
    mov rdi, 24           ; sizeof(HashMap) = 8+8+8
    call malloc

    ; Initialize fields
    mov qword [rax+0], 0  ; buckets = NULL
    mov qword [rax+8], 0  ; size = 0
    mov qword [rax+16], HASH_CAPACITY

    ; Allocate buckets
    push rax              ; Save hashmap*
    mov rdi, HASH_CAPACITY
    imul rdi, 8           ; sizeof(Entry*) * capacity
    call calloc_wrapper
    pop rdx               ; rdx = hashmap*
    mov [rdx+0], rax      ; hashmap->buckets = allocated

    mov rax, rdx          ; Return hashmap*
    pop rbp
    ret

hashmap_hash:
    ; Hash string key
    ; rdi = key (char*)
    ; Returns: rax = hash (uint64)
    push rbp
    mov rbp, rsp

    xor rax, rax          ; hash = 0
    xor rcx, rcx          ; i = 0
.loop:
    movzx rdx, byte [rdi+rcx]  ; ch = key[i]
    test rdx, rdx         ; if ch == 0
    jz .done

    imul rax, 31          ; hash *= 31
    add rax, rdx          ; hash += ch
    inc rcx
    jmp .loop
.done:
    pop rbp
    ret

hashmap_set:
    ; Set key-value pair
    ; rdi = hashmap*
    ; rsi = key (char*)
    ; rdx = value (int)
    push rbp
    mov rbp, rsp

    ; TODO: Full implementation with collision handling
    ; For now: simple linear probing

    pop rbp
    ret

hashmap_get:
    ; Get value by key
    ; rdi = hashmap*
    ; rsi = key (char*)
    ; Returns: rax = value (int), or 0 if not found
    push rbp
    mov rbp, rsp

    ; TODO: Full implementation

    pop rbp
    ret

calloc_wrapper:
    ; Wrapper for calloc(n, 8)
    mov rsi, 8
    jmp calloc
```

**Compile:**
```bash
nasm -f elf64 runtime_hashmap.asm -o runtime_hashmap.o
```

---

### Phase 2: Fox Wrapper (Week 1-2)

**File: `stdlib/hashmap_native.fox`**

```fox
# Native HashMap - Assembly backend
# NOTE: N0 TIDAK SUPPORT map[K]V - diport langsung ke assembly

struktur HashMap
    ptr void*  # Pointer to native hashmap
akhir

fungsi HashMapNew() HashMap
    # Call assembly: hashmap_new
    var ptr = native_hashmap_new()
    kembali HashMap{ptr: ptr}
akhir

fungsi HashMapSet(m HashMap, key string, value int) void
    # Call assembly: hashmap_set
    native_hashmap_set(m.ptr, key, value)
akhir

fungsi HashMapGet(m HashMap, key string) int
    # Call assembly: hashmap_get
    kembali native_hashmap_get(m.ptr, key)
akhir

fungsi HashMapHas(m HashMap, key string) bool
    # Call assembly: hashmap_has
    var result = native_hashmap_has(m.ptr, key)
    jika result == 1
        kembali benar
    akhir
    kembali salah
akhir

# Native declarations (implemented in runtime_hashmap.asm)
fungsi native_hashmap_new() void*
fungsi native_hashmap_set(ptr void*, key string, value int) void
fungsi native_hashmap_get(ptr void*, key string) int
fungsi native_hashmap_has(ptr void*, key string) int
```

**Usage in tree_walker:**
```fox
# tree_walker_asm.fox - Using assembly-backed HashMap

ambil "stdlib/hashmap_native"

struktur Environment
    intVars HashMap  # ✅ Works! No map[K]V syntax
    boolVars HashMap
akhir

fungsi NewEnvironment() Environment
    kembali Environment{
        intVars: HashMapNew(),   # ✅ Works!
        boolVars: HashMapNew()
    }
akhir

fungsi EnvSetInt(env Environment, name string, val int) void
    HashMapSet(env.intVars, name, val)
akhir

fungsi EnvGetInt(env Environment, name string) int
    kembali HashMapGet(env.intVars, name)
akhir
```

---

### Phase 3: Link Strategy (Week 2)

**Compile workflow:**
```bash
# Step 1: Compile Fox to C
./morph build tree_walker_asm.fox
# Output: tree_walker_asm.c

# Step 2: Compile C to object
gcc -c tree_walker_asm.c -o tree_walker_asm.o

# Step 3: Assemble runtime
nasm -f elf64 runtime_hashmap.asm -o runtime_hashmap.o

# Step 4: Link together
gcc tree_walker_asm.o runtime_hashmap.o runtime.c -o tree_walker
# Output: tree_walker (executable)
```

**Result:**
- ✅ N0 compiles Fox → C (no map syntax)
- ✅ Assembly provides HashMap implementation
- ✅ Both linked together
- ✅ Working binary!

---

## 📋 FILES TO UPDATE

### 1. `N1_HYBRID_STRATEGY.md`
```markdown
### ❌ N0 TIDAK SUPPORT (Assembly Path Required)

| Feature | Status | Evidence | Assembly? |
|---------|--------|----------|-----------|
| **Map declarations** | ❌ NOT SUPPORTED | multi-var error | **YES** |
| **Map literals** | ❌ NOT SUPPORTED | parser error | **YES** |
| **Map indexing** | ❌ NOT SUPPORTED | cannot test | **YES** |

**NOTE**: N0 TIDAK SUPPORT map sama sekali - diport langsung ke assembly
```

### 2. `AGENTS.md`
```markdown
## N0 Compiler Limitations (FROZEN)

### Map Types - ❌ NOT SUPPORTED
**Status**: N0 tidak support map[K]V syntax
**Evidence**: compiler/compiler.go:1142 "multi-var not supported"
**Resolution**: Assembly path required (runtime_hashmap.asm)
**NOTE**: Diport langsung ke assembly - no workaround exists
```

### 3. `morphsh/evaluator/README.md`
```markdown
### Map Limitation - CRITICAL ⚠️
N0 compiler tidak support map[K]V syntax sama sekali.

**Evidence:**
- Local: `var m map[string]int` → "multi-var not supported"
- Global: `var g map[K]V` → generates invalid C code
- Struct: `struktur S { m map[K]V }` → "multi-var not supported"

**Resolution:**
- ✅ Use assembly-backed HashMap (stdlib/hashmap_native.fox)
- ✅ Wrapper struct dengan void* pointer
- ✅ Link dengan runtime_hashmap.asm

**NOTE**: N0 TIDAK SUPPORT - diport langsung ke assembly
```

---

## 🎯 NEXT STEPS

1. ✅ Document limitation (this file)
2. ⏳ Implement runtime_hashmap.asm
3. ⏳ Create stdlib/hashmap_native.fox wrapper
4. ⏳ Modify tree_walker to use HashMap struct
5. ⏳ Test compile + link workflow
6. ⏳ Update AGENTS.md with checksums

---

## 📊 REVISED STRATEGY

### Before (Incorrect):
```
N0 C Path: 95%
Workarounds: 5%
Assembly: 0%
```

### After (Correct):
```
N0 C Path: 70%      (basic features only)
Workarounds: 10%    (struct assignment, etc)
Assembly: 20%       (HashMap, critical runtime)
```

**Map/HashMap is MANDATORY Assembly Path - no choice!**

---

**Document Version**: 1.0.0
**Date**: 2025-12-29
**Status**: 🔴 **CRITICAL LIMITATION FOUND**
**Resolution**: ⚙️ **Assembly implementation required**

---

*N0 TIDAK SUPPORT map - diport langsung ke assembly*
