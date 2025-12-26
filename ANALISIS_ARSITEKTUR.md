# ANALISIS ARSITEKTUR MORPH COMPILER
**Tanggal**: 2025-12-26  
**Analyst**: Kiro AI  
**Status**: Reverse Engineering Complete

---

## EXECUTIVE SUMMARY

Morph adalah bahasa pemrograman dengan sintaks Bahasa Indonesia yang sedang dalam tahap pengembangan menuju **self-hosting** (compiler yang bisa compile dirinya sendiri). Compiler saat ini dalam status **PARTIAL WORKING** dengan critical bug di sistem concurrency.

### Status Komponen
- ✅ **Lexer**: Fully working
- ✅ **Parser**: Fully working  
- ✅ **Type Checker**: Fully working
- ✅ **Compiler (C Transpiler)**: Fully working
- ⚠️ **C Runtime**: Partial - Mutex corruption under load
- 🚧 **Self-Hosting**: In Progress (morphsh/ directory)

---

## ARSITEKTUR SISTEM

### 1. COMPILATION PIPELINE

```
┌─────────────────────────────────────────────────────────────┐
│                    SOURCE CODE (.fox)                        │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│  LEXER (pkg/lexer/lexer.go)                                 │
│  - Tokenization                                              │
│  - Keywords: fungsi, var, jika, selama, akhir, dll          │
│  - Operators: +, -, *, /, ==, !=, &&, ||, dll               │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│  PARSER (pkg/parser/parser.go)                              │
│  - AST Generation                                            │
│  - Nodes: FunctionLiteral, IfExpression, WhileExpression    │
│  - Struct, Interface, Import support                         │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│  TYPE CHECKER (pkg/checker/checker.go)                      │
│  - Static type validation                                    │
│  - Type inference                                            │
│  - Import resolution                                         │
│  - Cycle detection                                           │
└──────────────────────┬──────────────────────────────────────┘
                       │
           ┌───────────┴───────────┐
           │                       │
           ▼                       ▼
┌──────────────────┐    ┌──────────────────────┐
│   INTERPRETER    │    │   C COMPILER         │
│  (pkg/evaluator) │    │  (pkg/compiler)      │
│                  │    │                      │
│  - Direct exec   │    │  - Transpile to C    │
│  - Tree walking  │    │  - Generate runtime  │
│  - For testing   │    │  - GCC compilation   │
└──────────────────┘    └──────────┬───────────┘
                                   │
                                   ▼
                        ┌──────────────────────┐
                        │   C RUNTIME          │
                        │  (runtime.c)         │
                        │                      │
                        │  - Memory Management │
                        │  - GC                │
                        │  - Concurrency       │
                        │  - Tiered Memory     │
                        └──────────┬───────────┘
                                   │
                                   ▼
                        ┌──────────────────────┐
                        │   NATIVE BINARY      │
                        │   (ELF executable)   │
                        └──────────────────────┘
```

---

## 2. C RUNTIME ARCHITECTURE

### 2.1 Memory Management System

#### **Cabinet/Drawer/Tray Hierarchy**
```
MorphContext (Cabinet)
    ├── allocated_bytes
    ├── next_gc_threshold
    ├── root_stack (Shadow Stack untuk GC)
    └── pages[] (Drawer)
            └── MphPage (Tray)
                    ├── data[PAGE_SIZE]
                    ├── bump_ptr
                    ├── swapped (flag)
                    └── swap_fd (file descriptor)
```

#### **Object Layout**
```c
ObjectHeader {
    uint32_t type_id;        // RTTI
    uint32_t size;           // Allocation size
    uint32_t flags;          // FLAG_MARKED, FLAG_SWAPPED, FLAG_VALID
    uint32_t start_canary;   // 0xDEADBEEF (corruption detection)
    MphPage* page;           // Back pointer untuk fast lookup
    ObjectHeader* next_free; // Free list chain
    uint64_t last_access;    // LRU timestamp
    char zone_id[16];        // Memory zone untuk Morphroutines
    uint8_t required_clearance;
    uint32_t end_canary;     // 0xDEADBEEF
}
```

### 2.2 Garbage Collection

**Algorithm**: Mark-and-Sweep dengan Incremental Marking

**Phases**:
1. **Root Scanning**: Shadow stack + global variables
2. **Mark Phase**: BFS traversal dengan mark stack (block-based)
3. **Sweep Phase**: Reclaim unmarked objects → free list
4. **Compact Phase**: (Optional) Defragmentation

**Optimizations**:
- Free list per page (O(1) cleanup saat recycle)
- Block-based mark stack (menghindari realloc spike)
- Adaptive GC threshold (berdasarkan heap size)

### 2.3 Tiered Memory (Swap System)

**Motivation**: Mengurangi memory footprint dengan swap ke disk

**Components**:
- **Swap Pool**: Pre-allocated 256MB dengan 64KB slots
- **Swap Worker Thread**: Async I/O untuk non-blocking operations
- **LRU Eviction**: `last_access` timestamp tracking
- **Swap Directory**: `.morph.vz/swap/`

**Operations**:
- `mph_swap_out()`: Evict page ke disk
- `mph_swap_in()`: Load page dari disk
- `mph_swap_in_locked()`: Lockless helper untuk GC

### 2.4 Concurrency (Morphroutines)

**Architecture**: Custom green threads (bukan OS threads)

**Hierarchy**:
```
Process
    └── Unit (max 3)
            ├── Memory Zone (isolated)
            ├── Security Clearance Level
            └── Shard
                    └── Fragment
                            └── Worker (mobile)
```

**Features**:
- **Worker Mobility**: Workers pindah antar Unit (bukan data yang dipindah)
- **Constitution-based Migration**: Security clearance check
- **Zone Isolation**: Cross-zone access forbidden
- **Async Swap**: Non-blocking I/O untuk swap operations

**Channels**:
- `saluran_baru()`: Create channel
- `kirim()`: Send (blocking)
- `terima()`: Receive (blocking)
- Implementation: Pthread mutex + condition variables

---

## 3. CRITICAL BUGS

### 3.1 Mutex Corruption (BLOCKER)

**Symptom**:
```
Fatal glibc error: pthread_mutex_lock.c:167 (___pthread_mutex_lock): 
assertion failed: mutex->__data.__owner == 0
```

**Root Cause Analysis**:
1. **Double Lock**: Thread mencoba lock mutex yang sudah di-lock oleh dirinya sendiri
2. **Uninitialized Mutex**: Mutex digunakan sebelum `pthread_mutex_init()`
3. **Memory Corruption**: Mutex structure ter-overwrite oleh buffer overflow

**Affected Components**:
- GC daemon thread (`mph_gc_daemon`)
- Swap worker thread (async swap operations)
- Channel operations (`kirim`, `terima`)

**Evidence from AGENTS.md**:
```
Status: ⚠️ PARTIAL IMPLEMENTATION
  - ❌ Mutex issues dalam stress testing (pthread assertion failures)
  - ❌ Complex allocation tracking disabled untuk stability
  - ❌ Stress tests cause mutex corruption
```

**Recommendation**:
1. **Immediate**: Disable GC daemon dan async swap untuk testing
2. **Short-term**: Add mutex debugging (PTHREAD_MUTEX_ERRORCHECK)
3. **Long-term**: Redesign locking strategy (lock-free data structures)

### 3.2 Memory Safety Issues

**Canary Protection**: ✅ Implemented
- Start/end magic values (0xDEADBEEF)
- Detect buffer overflows

**Zone Validation**: ✅ Implemented
- Cross-zone access detection
- Security clearance enforcement

**Known Gaps**:
- Stress tests masih crash
- Memory tracking simplified (race conditions)

---

## 4. SELF-HOSTING PROGRESS

### 4.1 MorphSH (Morph Shell)

**Goal**: Compiler ditulis dalam Morph sendiri

**Status**: Foundation Complete (70%)

**Components**:
```
morphsh/
├── token.fox              ✅ Token definitions
├── lexer.fox              ✅ Lexer implementation
├── parser.fox             🚧 Parser (partial)
├── ast.fox                ✅ AST nodes
├── typechecker.fox        🚧 Type checker (stub)
├── evaluator.fox          🚧 Evaluator (stub)
└── main.fox               🚧 Entry point
```

**Blockers**:
1. Memory limits saat compile complex modules
2. Mutex bugs mencegah testing
3. Missing features: Tuple returns, Error handling

### 4.2 Bootstrap Strategy

**Phase 1**: ✅ Lexer in Morph
**Phase 2**: 🚧 Parser in Morph  
**Phase 3**: ⏳ Type Checker in Morph
**Phase 4**: ⏳ C Codegen in Morph
**Phase 5**: ⏳ Full self-hosting

---

## 5. FITUR BAHASA

### 5.1 Syntax Highlights

**Keywords**:
- `fungsi`: Function definition
- `var`/`variabel`: Mutable variable
- `tetapan`/`const`: Constant
- `struktur`: Struct type
- `antarmuka`: Interface
- `jika`/`atau_jika`/`lainnya`: If/else-if/else
- `selama`: While loop
- `kembalikan`: Return
- `akhir`: Block terminator
- `ambil`: Import
- `kosong`: Null value

**Types**:
- Primitives: `int`, `float`, `string`, `bool`, `void`
- Composites: `[]T` (array), `map[K]V` (map)
- User-defined: `struktur`, `antarmuka`
- Special: `error` (error type)

**Operators**:
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
- Logical: `&&`, `||`, `!`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`

### 5.2 Standard Library

**Modules**:
- `io`: File I/O (`Open`, `Read`, `Write`, `Close`)
- `math`: Math functions (`Sin`, `Cos`, `Sqrt`, `Abs`, `PI`)
- `string`: String manipulation (`Len`, `Concat`)
- `time`: Time operations (`Now`, `Sleep`)
- `conv`: Type conversion (`Atoi`, `Itoa`)

---

## 6. TESTING & VALIDATION

### 6.1 Test Suite

**Location**: `examples/`

**Categories**:
1. **Basic**: hello_world, fibonacci, array_test, struct_test
2. **Concurrency**: concurrency.fox, async_swap_test.fox
3. **Memory**: gc_torture_test, memory_safety_test, memory_leak_test
4. **Stress**: stress_test, number_chess_stress, torture_test
5. **Features**: closure_test, tuple_test, error_check, interface_test

**Status**:
- ✅ Basic tests: Pass (interpreter)
- ⚠️ Compiled tests: Crash (mutex bug)
- ❌ Stress tests: Fail (memory corruption)

### 6.2 Checksum System

**Purpose**: Integrity verification dan change tracking

**Location**: `.morph.vz/checksums/`

**Format**: SHA256 per file

**Usage**: Detect "Ghost Changes" (undocumented modifications)

---

## 7. DEVELOPMENT WORKFLOW

### 7.1 Context System

**Files**:
- `AGENTS.md`: Source of truth (semua perubahan tercatat)
- `.morph.vz/`: Context snapshots
- `.vzoel.jules/`: Technical debt tracking

**Principles**:
- **Kejujuran**: Semua perubahan harus dicatat
- **Ketelitian**: Checksum verification
- **Konsistensi**: Single source of truth

### 7.2 Git Workflow

**Strategy**: Whitelist `.gitignore`

**Ignored**:
- Build artifacts: `*.o`, `*.c`, `*.h` (in examples/)
- Binaries: executables
- Swap files: `.morph.vz/swap/`

---

## 8. REKOMENDASI AKSI

### 8.1 Immediate (Critical)

1. **Fix Mutex Bug**:
   - Add mutex debugging flags
   - Audit all `pthread_mutex_lock()` calls
   - Check initialization order

2. **Disable Problematic Features**:
   - GC daemon thread (use manual GC)
   - Async swap (use synchronous)
   - Stress tests (until stable)

3. **Add Logging**:
   - Mutex operations
   - Thread lifecycle
   - Memory allocations

### 8.2 Short-term (Stabilization)

1. **Memory Safety**:
   - Enable AddressSanitizer (ASan)
   - Enable ThreadSanitizer (TSan)
   - Valgrind testing

2. **Testing**:
   - Unit tests untuk runtime
   - Integration tests
   - Regression tests

3. **Documentation**:
   - Runtime API docs
   - Memory layout diagrams
   - Concurrency model explanation

### 8.3 Long-term (Self-Hosting)

1. **Complete MorphSH**:
   - Finish parser implementation
   - Implement type checker
   - Implement C codegen

2. **Optimize Runtime**:
   - Lock-free data structures
   - Better GC algorithm (generational?)
   - Reduce memory overhead

3. **Language Features**:
   - Generics
   - Pattern matching
   - Async/await syntax

---

## 9. KESIMPULAN

Morph adalah proyek ambisius dengan arsitektur yang solid namun masih dalam tahap **early development**. Compiler berhasil transpile ke C, namun runtime memiliki **critical bug** di sistem concurrency yang harus diperbaiki sebelum bisa digunakan untuk production.

**Kekuatan**:
- ✅ Clean architecture (separation of concerns)
- ✅ Comprehensive type system
- ✅ Advanced memory management (tiered, GC, zones)
- ✅ Self-hosting vision (morphsh/)

**Kelemahan**:
- ❌ Mutex corruption (blocker)
- ❌ Memory safety gaps (stress tests fail)
- ❌ Incomplete self-hosting
- ❌ Limited standard library

**Next Steps**:
1. Debug dan fix mutex issues
2. Stabilize runtime dengan extensive testing
3. Complete morphsh implementation
4. Achieve self-hosting milestone

---

**Catatan**: Dokumen ini dibuat berdasarkan reverse engineering dari source code, AGENTS.md, dan testing. Untuk informasi terbaru, selalu rujuk ke `AGENTS.md` sebagai single source of truth.
