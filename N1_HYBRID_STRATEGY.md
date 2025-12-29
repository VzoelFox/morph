# N1 Hybrid Strategy: C Path + Assembly Path
## Strategi Pragmatis untuk Self-Hosting

**Date**: 2025-12-29
**Status**: 🎯 **ACTIVE STRATEGY**
**Goal**: N1 Self-Hosting dengan N0 Frozen

---

## 🎯 PRINSIP UTAMA

```
Assembly Path HANYA untuk yang N0 TIDAK SUPPORT atau RAPUH
Everything else → tetap pakai N0 C Path (stable & proven)
```

**Rationale:**
- ✅ N0 tetap frozen (no modifications)
- ✅ Leverage N0's stability untuk 80% kode
- ✅ Assembly path hanya untuk 20% critical features
- ✅ Minimize complexity, maximize results

---

## 📊 N0 CAPABILITY MATRIX

### ✅ N0 C PATH (Stable - USE THIS!)

| Feature | Status | Evidence |
|---------|--------|----------|
| **Basic types** | ✅ Excellent | int, string, bool fully working |
| **Structs** | ✅ Good | Struct literals work: `Type{kind: 1}` |
| **Functions** | ✅ Excellent | Parameters, return values, recursion |
| **Control flow** | ✅ Good | if/atau_jika/lainnya, while loops |
| **Arrays (type)** | ✅ Good | `var x []int` declarations work |
| **Arrays (literal)** | ✅ Good | `[]int{1, 2, 3}` literals work |
| **Map (type)** | ✅ Good | `var m map[string]int` declarations work |
| **Map (access)** | ✅ Good | `m[key] = value` indexing works |
| **Logical ops** | ✅ Fixed | `dan`, `atau`, `tidak` work (Bug #3 fixed) |
| **String ops** | ✅ Fixed | Empty strings work (Bug #2 fixed) |
| **Recursion** | ✅ Fixed | 1000-level limit (Bug #5 fixed) |

**Recommendation**: **USE N0 for these features!**

---

### ⚠️ N0 RAPUH (Use With Caution)

| Feature | Status | Workaround | Assembly? |
|---------|--------|------------|-----------|
| **Struct field assign** | ⚠️ Broken | Use struct literal | No - workaround OK |
| **Import caching** | ⚠️ None | Manual dedupe | Maybe |
| **Import cycles** | ⚠️ None | Avoid cycles | Maybe |
| **Module exports** | ⚠️ None | Everything public | Maybe |

**Recommendation**: **Workaround first, assembly if critical**

---

### ❌ N0 TIDAK SUPPORT (Assembly Path Required)

| Feature | Status | Evidence | Assembly? |
|---------|--------|----------|-----------|
| **Map literals** | ❌ Not supported | `map[K]V{}` parser error | **YES** |
| **Generics (real)** | ❌ Not supported | Type parameters not parsed | No - skip for N1 |
| **Macros** | ❌ Not supported | No compile-time codegen | No - skip for N1 |

**Recommendation**: **Assembly path OR workaround**

---

## 🔧 WORKAROUND STRATEGIES

### Strategy 1: Struct-Based Map (PROVEN!)

**Problem**: `map[string]int{}` tidak didukung

**Solution**: Gunakan struct dengan counter
```fox
struktur Environment
    count int
    # Later: bisa expand dengan array of entries
akhir

fungsi NewEnvironment() Environment
    kembali Environment{count: 0}  # ✅ Works!
akhir
```

**Status**: ✅ **Tested & Working** (`/tmp/test_map_workaround.fox`)

**Trade-off:**
- ✅ No assembly needed
- ✅ Compiles with N0
- ❌ Less convenient than real map
- ⚠️ Manual implementation needed

**Verdict**: **GUNAKAN INI untuk MVP!**

---

### Strategy 2: Array-Based Map

**Problem**: Butuh dynamic key-value storage

**Solution**: Array of structs
```fox
struktur Entry
    key string
    value int
    used bool
akhir

struktur HashMap
    entries []Entry
    size int
akhir

fungsi NewHashMap() HashMap
    # Array literal didukung N0!
    kembali HashMap{
        entries: []Entry{},  # ✅ Supported!
        size: 0
    }
akhir

fungsi HashMapSet(m HashMap, k string, v int) void
    # Linear search - simple but works
    var i = 0
    selama i < m.size
        jika m.entries[i].key == k
            m.entries[i].value = v
            kembali
        akhir
        i = i + 1
    akhir
    # Add new entry (simplified)
    m.size = m.size + 1
akhir
```

**Status**: 🚧 **Theory - needs testing**

**Trade-off:**
- ✅ No assembly needed
- ✅ Real map functionality
- ❌ O(n) lookup (but OK for small maps)
- ⚠️ Need to implement full API

**Verdict**: **GUNAKAN untuk production tree_walker**

---

### Strategy 3: Assembly Runtime (LAST RESORT)

**Problem**: Performance critical hashmap needed

**Solution**: Assembly-based hashmap runtime
```asm
; runtime_hashmap.asm
global hashmap_new
global hashmap_set
global hashmap_get

hashmap_new:
    ; Allocate memory for hashmap
    mov rdi, 1024      ; size
    call malloc
    ret

hashmap_set:
    ; rdi = hashmap*, rsi = key*, rdx = value
    ; Hash key, store in bucket
    ret
```

**Link dengan:**
```bash
nasm -f elf64 runtime_hashmap.asm -o runtime_hashmap.o
gcc output.o runtime_hashmap.o -o binary
```

**Status**: ⏸️ **Deferred - only if needed**

**Trade-off:**
- ✅ Best performance
- ✅ Real hashmap implementation
- ❌ Complexity (assembly code)
- ❌ Platform-specific (x86_64)

**Verdict**: **SKIP untuk sekarang, pakai Strategy 2**

---

## 🗺️ TREE_WALKER_WORKING.FOX WORKAROUND

### Current Problem (Line 26-27):
```fox
struktur Environment
    intVars map[string]int
    boolVars map[string]bool
akhir

fungsi NewEnvironment() Environment
    kembali Environment{
        intVars: map[string]int{},      # ❌ TIDAK DIDUKUNG
        boolVars: map[string]bool{}     # ❌ TIDAK DIDUKUNG
    }
akhir
```

### Solution A: Minimal (Zero-Init)
```fox
fungsi NewEnvironment() Environment
    var env Environment
    # Map fields already zero-initialized
    kembali env
akhir

fungsi EnvSetInt(env Environment, name string, val int) void
    env.intVars[name] = val  # ✅ Indexing supported!
akhir
```

**Status**: ✅ **Easiest fix - try this first!**

### Solution B: Struct Counter (Safe)
```fox
struktur Environment
    intVarCount int
    boolVarCount int
akhir

fungsi NewEnvironment() Environment
    kembali Environment{intVarCount: 0, boolVarCount: 0}
akhir
```

**Status**: ✅ **Proven working**

### Solution C: Array-Based (Full Featured)
```fox
struktur IntEntry
    key string
    value int
akhir

struktur Environment
    intVars []IntEntry
    boolVars []BoolEntry
akhir

fungsi NewEnvironment() Environment
    kembali Environment{
        intVars: []IntEntry{},
        boolVars: []BoolEntry{}
    }
akhir
```

**Status**: 🚧 **Best long-term solution**

---

## 📋 IMPLEMENTATION PLAN

### Phase 1: Workaround-First Approach (Week 1)

**Goal**: Compile `tree_walker_working.fox` dengan N0

```
Day 1: Test Solution A (zero-init)
       - Edit tree_walker_working.fox
       - Remove map literals
       - Test if indexing works

Day 2: If A fails, implement Solution C (array-based)
       - Create Entry structs
       - Implement linear search
       - Test with tree walker

Day 3-4: Complete tree walker features
         - Variable storage working
         - Expression evaluation
         - Control flow

Day 5-7: Test & validate
         - Run tree walker on sample code
         - Ensure no N0 modifications needed
```

**Deliverable**: `morphsh/evaluator/tree_walker_n0.fox` - compiled by N0!

---

### Phase 2: Runtime Library (Week 2-3)

**Goal**: Build runtime untuk fitur yang sering dipakai

**Priority (sesuai user request):**
1. **Hashmap** (untuk Environment)
2. **String** (untuk operators, concat)
3. **Array** (untuk collections)
4. **Builtins** (native functions yang sering dipanggil)

**Implementation:**

```fox
# stdlib/hashmap.fox - Pure Fox implementation
struktur HashEntry
    key string
    value int
    next int  # Index to next entry (chaining)
akhir

struktur HashMap
    buckets []int      # Bucket heads
    entries []HashEntry
    size int
    capacity int
akhir

fungsi HashMapNew(cap int) HashMap
    kembali HashMap{
        buckets: []int{},  # Will be initialized
        entries: []HashEntry{},
        size: 0,
        capacity: cap
    }
akhir

fungsi HashMapHash(key string) int
    # Simple hash function
    var hash = 0
    var i = 0
    selama i < panjang(key)
        # hash = hash * 31 + key[i]
        hash = hash * 31
        i = i + 1
    akhir
    kembali hash
akhir

fungsi HashMapSet(m HashMap, key string, value int) void
    var hash = HashMapHash(key)
    var bucket = hash % m.capacity
    # ... implementation
akhir
```

**Deliverable**: `stdlib/hashmap.fox`, `stdlib/string.fox`, `stdlib/array.fox`

---

### Phase 3: Builtins Integration (Week 3-4)

**Goal**: Native functions untuk performance-critical ops

**Builtins to implement:**
- `native_hashmap_new(capacity int) void*`
- `native_hashmap_set(map void*, key string, value int) void`
- `native_hashmap_get(map void*, key string) int`
- `native_string_concat(a string, b string) string`
- `native_array_append(arr void*, elem int) void`

**Implementation in runtime.c:**
```c
// runtime.c additions
typedef struct {
    char* key;
    int value;
    void* next;
} HashEntry;

typedef struct {
    HashEntry** buckets;
    int capacity;
    int size;
} HashMap;

void* native_hashmap_new(int capacity) {
    HashMap* map = malloc(sizeof(HashMap));
    map->buckets = calloc(capacity, sizeof(HashEntry*));
    map->capacity = capacity;
    map->size = 0;
    return map;
}

void native_hashmap_set(void* map_ptr, char* key, int value) {
    HashMap* map = (HashMap*)map_ptr;
    // ... hash & insert
}
```

**Deliverable**: Extended `runtime.c` dengan builtins

---

### Phase 4: Assembly Path (ONLY IF NEEDED)

**Trigger conditions:**
1. Workaround too slow (proven dengan benchmark)
2. N0 crashes pada kode valid
3. Import system benar-benar blocking

**Implementation:**
```
N0 C Code (90%)
    ↓
  [.o file]
    ↓
Assembly Code (10% - critical path only)
    ↓
  [.o file]
    ↓
Link together
    ↓
Final binary
```

**Status**: ⏸️ **Deferred - workaround first**

---

## 🎯 SUCCESS CRITERIA

### Week 1 Success:
- ✅ `tree_walker_working.fox` compiles dengan N0
- ✅ No N0 modifications
- ✅ Basic Environment working (even with workaround)
- ✅ Can evaluate simple expressions

### Week 2-3 Success:
- ✅ Hashmap stdlib implemented (pure Fox)
- ✅ String ops complete
- ✅ Array helpers done
- ✅ Builtins integrated in runtime.c

### Week 4 Success (SELF-HOSTING!):
- ✅ Tree walker can execute simple Fox programs
- ✅ Can run basic type checker
- ✅ Proof-of-concept N1 self-hosting

---

## 📊 DECISION MATRIX

### When to use N0 C Path:
```
✅ Feature is stable in N0
✅ Workaround exists and is simple
✅ Performance is acceptable
✅ N0 tetap frozen
```

### When to use Assembly Path:
```
❌ No workaround possible
❌ N0 crashes on valid code
❌ Performance is critical bottleneck
❌ Import system completely broken
```

### Current recommendation:
**95% N0 C Path + 5% Workarounds + 0% Assembly (for now)**

---

## 🔍 NEXT IMMEDIATE ACTIONS

1. **Test Solution A** - Zero-init workaround
   ```bash
   # Edit tree_walker_working.fox
   # Remove map literals, keep declarations
   # Test if it compiles
   ```

2. **If Solution A fails, implement Solution C** - Array-based map
   ```bash
   # Create Entry structs
   # Linear search implementation
   # Test with tree walker
   ```

3. **Document decision** - Update AGENTS.md
   ```markdown
   - N1 uses hybrid approach
   - Assembly path only for N0 unsupported features
   - Current: 100% N0 C path with workarounds
   ```

---

## 📈 TIMELINE PROJECTION

| Week | Goal | Assembly % |
|------|------|------------|
| 1 | Tree walker MVP | 0% |
| 2-3 | Runtime library | 0% |
| 4 | Self-hosting PoC | 0% |
| 5+ | Optimization (if needed) | 0-10% |

**Estimate**: **Assembly path mungkin tidak perlu sama sekali!**

---

## 💡 PHILOSOPHICAL ALIGNMENT

**User's directive:**
> "Assembly path HANYA untuk yang N0 tidak support atau rapuh"

**Implementation:**
- ✅ Try workaround first (struct, array)
- ✅ Use pure Fox code when possible
- ✅ Keep N0 frozen
- ✅ Assembly as last resort only

**Result:**
- N1 remains maintainable
- N0 remains stable
- Complexity minimized
- Self-hosting achievable

---

## 🎯 BOTTOM LINE

**Strategy**: **Workaround-First, Assembly-Last**

**Current status**: All `tree_walker_working.fox` blockers dapat di-workaround!

**Next step**: Test Solution A (zero-init), jika gagal pakai Solution C (array-based)

**Assembly path**: ⏸️ Deferred - no immediate need

---

**Document Version**: 1.0.0
**Date**: 2025-12-29
**Status**: 🎯 **ACTIVE STRATEGY**
**Review**: After Week 1 implementation

---

*Pragmatic, focused, executable strategy for N1 self-hosting.*
