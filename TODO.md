# TODO.md - Roadmap N2: Self-Hosting Preparation

## Status: Active
**Target**: N2 Self-Hosting Ready
**Timeline**: 4-6 weeks
**Priority**: CRITICAL for bootstrap independence

---

## 🔥 CRITICAL PRIORITY (Week 1-2)

### 1. **Memory Management Stdlib** 
- [ ] `stdlib/memory.fox` - Custom allocator interface
  - [ ] `Allocator` struct dengan function pointers
  - [ ] `make_allocator()` - Factory function
  - [ ] `alloc()`, `free_memory()`, `realloc()` - Core operations
  - [ ] Replace syscall malloc/free dependencies
  - **Rationale**: N2 harus independen dari libc malloc
  - **Impact**: Foundation untuk semua memory operations

### 2. **GC Control Stdlib**
- [ ] `stdlib/gc.fox` - GC management API
  - [ ] `gc_collect()` - Manual collection trigger
  - [ ] `gc_set_threshold()` - Threshold control
  - [ ] `gc_get_stats()` - Memory statistics
  - [ ] `gc_disable()`, `gc_enable()` - GC control
  - **Rationale**: N2 perlu fine-grained GC control
  - **Impact**: Performance tuning untuk self-hosting

### 3. **Morphroutines Core Stdlib**
- [ ] `stdlib/morphroutines.fox` - Replace goroutines
  - [ ] `MorphUnit` struct - Execution unit
  - [ ] `morph_spawn()` - Replace `luncurkan()`
  - [ ] `morph_channel()` - Replace `saluran_baru()`
  - [ ] `morph_send()`, `morph_receive()` - Channel ops
  - [ ] Worker mobility functions
  - **Rationale**: Eliminate Go goroutine dependency
  - **Impact**: Core concurrency independence

---

## 🚨 HIGH PRIORITY (Week 2-3)

### 4. **Memory Buffer I/O**
- [ ] `stdlib/buffer.fox` - Replace file I/O syscalls
  - [ ] `MemoryBuffer` struct
  - [ ] `buffer_write()`, `buffer_read()` - Memory-based I/O
  - [ ] `buffer_create()`, `buffer_destroy()` - Lifecycle
  - **Rationale**: Eliminate file syscall dependencies
  - **Impact**: Pure memory-based compilation

### 5. **String Operations (Pure)**
- [ ] `stdlib/strings.fox` - No-syscall string ops
  - [ ] `string_concat()` - Pure concatenation
  - [ ] `string_length()`, `string_substring()` - Core ops
  - [ ] `string_compare()`, `string_contains()` - Comparison
  - [ ] String interning system
  - **Rationale**: Remove libc string dependencies
  - **Impact**: Pure string processing

### 6. **Hash Functions**
- [ ] `stdlib/hash.fox` - Hash algorithms
  - [ ] `hash_string()` - String hashing
  - [ ] `hash_combine()` - Hash combination
  - [ ] `hash_map_key()` - Map key hashing
  - **Rationale**: Maps need independent hash functions
  - **Impact**: Container independence

---

## ⚡ MEDIUM PRIORITY (Week 3-4)

### 7. **Time Operations (Pure)**
- [ ] `stdlib/time.fox` - Replace time syscalls
  - [ ] `time_counter()` - Monotonic counter
  - [ ] `time_diff()` - Time difference calculation
  - [ ] `time_sleep_cycles()` - CPU cycle-based delay
  - **Rationale**: Remove time syscall dependencies
  - **Impact**: Timing operations independence

### 8. **Format Operations**
- [ ] `stdlib/format.fox` - String formatting
  - [ ] `format_int()` - Integer to string
  - [ ] `format_string()` - String formatting
  - [ ] `parse_int()` - String to integer
  - **Rationale**: Printf/scanf replacement
  - **Impact**: I/O formatting independence

### 9. **Error Handling**
- [ ] `stdlib/errors.fox` - Error management
  - [ ] `Error` struct enhancement
  - [ ] `error_chain()` - Error chaining
  - [ ] `error_format()` - Error formatting
  - **Rationale**: Robust error handling for N2
  - **Impact**: Better error reporting

---

## 🔧 LOW PRIORITY (Week 4-6)

### 10. **Sort Algorithms**
- [ ] `stdlib/sort.fox` - Sorting functions
  - [ ] `sort_array()` - Generic array sorting
  - [ ] `sort_strings()` - String array sorting
  - **Rationale**: Compiler optimization needs
  - **Impact**: Code generation optimization

### 11. **Math Extensions**
- [ ] `stdlib/math_extended.fox` - Extended math
  - [ ] `math_power()` - Power function
  - [ ] `math_sqrt()` - Square root
  - **Rationale**: Compiler optimizations
  - **Impact**: Code generation improvements

### 12. **Debug Utilities**
- [ ] `stdlib/debug.fox` - Debug support
  - [ ] `debug_print()` - Debug output
  - [ ] `debug_assert()` - Assertions
  - **Rationale**: N2 debugging support
  - **Impact**: Development productivity

---

## 🎯 ELIMINATION TARGETS (Throughout)

### **Remove from N2:**
- [ ] **Goroutines**: Replace `luncurkan()` dengan `morph_spawn()`
- [ ] **Go Channels**: Replace `saluran_baru()` dengan `morph_channel()`
- [ ] **File I/O Syscalls**: Replace dengan memory buffers
- [ ] **Time Syscalls**: Replace dengan pure functions
- [ ] **Libc Dependencies**: Replace malloc/free dengan custom allocator
- [ ] **Printf/Scanf**: Replace dengan format functions

### **Keep in N2:**
- ✅ **Type System**: All 5 phases complete
- ✅ **GC System**: Enhanced dengan control API
- ✅ **Memory Management**: Custom allocator
- ✅ **Morphroutines**: Pure concurrency system

---

## 📊 SUCCESS METRICS

### **N2 Independence Criteria:**
1. **Zero Goroutine Dependencies** - All `luncurkan()` replaced
2. **Zero Syscall Dependencies** - All I/O memory-based
3. **Zero Libc Dependencies** - Custom allocator working
4. **Self-Compilation** - N2 can compile itself
5. **Performance Parity** - N2 ≥ N1 performance

### **Testing Requirements:**
- [ ] All stdlib modules have comprehensive tests
- [ ] Self-compilation test suite
- [ ] Performance benchmarks vs N1
- [ ] Memory usage profiling
- [ ] Concurrency stress tests

---

## 🚀 IMPLEMENTATION STRATEGY

### **Week 1-2: Foundation**
Focus on memory.fox, gc.fox, morphroutines.fox

### **Week 2-3: I/O Independence** 
Focus on buffer.fox, strings.fox, hash.fox

### **Week 3-4: System Independence**
Focus on time.fox, format.fox, errors.fox

### **Week 4-6: Optimization & Testing**
Focus on sort.fox, math_extended.fox, debug.fox + comprehensive testing

### **Continuous: Elimination**
Remove goroutines, syscalls, libc dependencies throughout all phases

---

**Status**: Ready for implementation
**Next Action**: Start with `stdlib/memory.fox` implementation
**Owner**: Development Team
**Review**: Weekly progress review required
