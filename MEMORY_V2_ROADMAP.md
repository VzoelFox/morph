# Memory V2 Implementation Roadmap

**Created**: 2025-12-27
**Target**: Ready for N1 → FoxVM transition
**Timeline**: 12 weeks (3 months)

---

## 🎯 Goals

### Primary
- ✅ Enable N1 (MorphSH) compilation on 8GB machines
- ✅ Reduce memory usage by 60-70%
- ✅ Improve compilation speed by 40-50%
- ✅ Zero OOM crashes

### Secondary
- ✅ Generational GC for runtime workloads
- ✅ Tunable GC for different use cases
- ✅ Memory profiling infrastructure
- ✅ Future-proof for FoxVM

---

## 📅 Week-by-Week Plan

### **PHASE 1: Foundation (Weeks 1-2)**

#### Week 1: Core Infrastructure
**Tasks:**
- [ ] Create `pkg/compiler/runtime/morph_mem_v2.h` (clean version)
- [ ] Create `pkg/compiler/runtime/morph_mem_v2.c` (implementation)
- [ ] Implement `ObjectHeader` V2 (16 bytes)
- [ ] Implement `MorphMemConfig` system
- [ ] Implement allocation dispatcher skeleton
- [ ] Write unit tests for header/config

**Deliverables:**
```c
// Compiles and passes tests
MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER_INIT);
morph_mem_destroy(ctx);
```

**Success Metrics:**
- All unit tests pass
- Header size = 16 bytes (verified)
- Config system working

---

#### Week 2: Arena Allocator
**Tasks:**
- [ ] Implement `ArenaAllocator` (arena.c)
- [ ] Implement `arena_alloc()` - bump pointer
- [ ] Implement `arena_reset()` - bulk free
- [ ] Implement `arena_destroy()` - cleanup
- [ ] Write unit tests for arena
- [ ] Benchmark arena performance

**Deliverables:**
```c
ArenaAllocator* aa = arena_allocator_new(2 * 1024 * 1024);
for (int i = 0; i < 1000000; i++) {
    void* ptr = arena_alloc(aa, 32);  // ← Fast!
}
arena_reset(aa);  // ← Instant!
```

**Success Metrics:**
- Arena alloc < 10 CPU cycles
- Arena reset < 1ms for 100MB
- Memory reuse working
- No leaks (valgrind clean)

---

### **PHASE 2: Allocators (Weeks 3-4)**

#### Week 3: Pool Allocator
**Tasks:**
- [ ] Implement `Pool` allocator (pool.c)
- [ ] Implement bitmap-based allocation
- [ ] Implement size classes (32, 64, 128, 256 bytes)
- [ ] Integrate pools into dispatcher
- [ ] Write unit tests
- [ ] Benchmark pool performance

**Deliverables:**
```c
Pool* pool32 = pool_new(32);
void* ptr = pool_alloc(pool32);  // ← O(1)
pool_free(pool32, ptr);
```

**Success Metrics:**
- Pool alloc < 20 CPU cycles
- Pool free < 15 CPU cycles
- Zero fragmentation
- Tests pass

---

#### Week 4: Integration & Testing
**Tasks:**
- [ ] Integrate arena + pools into `MorphContextV2`
- [ ] Implement allocation dispatcher logic
- [ ] Test COMPILER mode end-to-end
- [ ] Benchmark vs V1
- [ ] Fix issues found in testing

**Deliverables:**
```c
MorphContextV2* ctx = morph_mem_init(MORPH_CONFIG_COMPILER_INIT);
void* p1 = morph_alloc(ctx, 24, TYPE_IDENTIFIER);  // → Pool
void* p2 = morph_alloc(ctx, 512, TYPE_AST_NODE);   // → Arena
morph_mem_destroy(ctx);
```

**Success Metrics:**
- Dispatcher routes correctly
- Memory usage < V1
- All tests pass

---

### **PHASE 3: N0 Integration (Weeks 5-6)**

#### Week 5: N0 Compiler Integration
**Tasks:**
- [ ] Create compatibility layer for N0
- [ ] Add V2 init code to `cmd/morph/main.go`
- [ ] Test N0 with V2 (small files first)
- [ ] Compare V1 vs V2 performance
- [ ] Fix bugs

**Deliverables:**
```bash
# Rebuild N0 with V2
go build ./cmd/morph

# Test compilation
./morph build examples/hello.fox  # Small file
./morph build morphsh/token.fox   # Medium file
```

**Success Metrics:**
- N0 compiles successfully with V2
- Memory usage down 30%+
- Time down 20%+
- No crashes

---

#### Week 6: N1 Compilation Testing
**Tasks:**
- [ ] Test N0 + V2 on MorphSH files
- [ ] Compile `morphsh/lexer.fox` (203 lines)
- [ ] Compile `morphsh/parser.fox` (350 lines)
- [ ] Compile `morphsh/ast.fox` (272 lines)
- [ ] Monitor memory usage
- [ ] Tune arena sizes if needed

**Deliverables:**
```bash
# All MorphSH files compile successfully
./morph build morphsh/token.fox
./morph build morphsh/lexer.fox
./morph build morphsh/parser.fox
./morph build morphsh/ast.fox
```

**Success Metrics:**
- ✅ All files compile on 8GB machine
- ✅ Peak memory < 4GB
- ✅ No OOM crashes
- ✅ Faster than V1

---

### **PHASE 4: Generational GC (Weeks 7-8)**

#### Week 7: Young Generation
**Tasks:**
- [ ] Implement `GenerationalHeap` structure
- [ ] Implement young gen bump allocator
- [ ] Implement minor GC (evacuation)
- [ ] Implement write barrier stub
- [ ] Write unit tests

**Deliverables:**
```c
GenerationalHeap* gh = gen_heap_new(2 * 1024 * 1024, 32 * 1024 * 1024);
void* obj = gen_heap_alloc(gh, 32, TYPE_INT);
gen_heap_gc_minor(gh);  // ← Fast (1-5ms)
```

**Success Metrics:**
- Minor GC < 5ms for 2MB young gen
- Evacuation working
- No corruption

---

#### Week 8: Old Generation & Promotion
**Tasks:**
- [ ] Implement old gen (mark-sweep)
- [ ] Implement promotion logic
- [ ] Implement major GC
- [ ] Implement write barrier (full)
- [ ] Test generational GC end-to-end

**Deliverables:**
```c
// Long-running program
for (int i = 0; i < 100000; i++) {
    void* obj = gen_heap_alloc(gh, 64, TYPE_NODE);
    // Old objects promoted automatically
}
```

**Success Metrics:**
- Major GC < 50ms for 32MB old gen
- Promotion working
- Write barrier correct
- Tests pass

---

### **PHASE 5: Optimization (Weeks 9-10)**

#### Week 9: Performance Tuning
**Tasks:**
- [ ] Profile allocation hot paths
- [ ] Optimize mark phase (cache-friendly)
- [ ] Optimize evacuation (batch copy)
- [ ] Tune GC thresholds
- [ ] Add SIMD optimizations (if applicable)

**Deliverables:**
- Performance report
- Optimized code paths
- Benchmarks

**Success Metrics:**
- GC pause < 5ms (p99)
- Throughput > 95%
- Memory overhead < 20%

---

#### Week 10: Metrics & Observability
**Tasks:**
- [ ] Implement `MorphMemStats` collection
- [ ] Implement `morph_mem_print_stats()`
- [ ] Implement `morph_mem_dump_stats()` (JSON/CSV)
- [ ] Add histogram for GC pauses
- [ ] Add memory timeline tracking

**Deliverables:**
```bash
./morph build program.fox
# Automatic stats output:
# Memory: 45MB peak, 2 GCs (avg 3.2ms)
# Objects: 125K allocated, 98K live
```

**Success Metrics:**
- Stats accurate
- Overhead < 1%
- Useful for debugging

---

### **PHASE 6: Production (Weeks 11-12)**

#### Week 11: Stress Testing
**Tasks:**
- [ ] Run fuzzer on allocator
- [ ] Test with very large files (10K+ LoC)
- [ ] Test under memory pressure
- [ ] Memory leak detection (valgrind)
- [ ] Edge case testing

**Deliverables:**
- Fuzzing report
- Stress test results
- Bug fixes

**Success Metrics:**
- No crashes in 1M+ iterations
- No memory leaks
- Handles edge cases

---

#### Week 12: Documentation & Release
**Tasks:**
- [ ] Write user guide (V1 → V2 migration)
- [ ] Update AGENTS.md
- [ ] Create release notes
- [ ] Code review
- [ ] Merge to main branch

**Deliverables:**
- Documentation complete
- V2 merged
- V1 deprecated notice

**Success Metrics:**
- All docs updated
- Clean code review
- Ready for production

---

## 🎓 Decision Points

### Week 2 Checkpoint
**Question**: Is arena performance acceptable?
- **If YES**: Continue to Week 3
- **If NO**: Investigate (profile, tune)

### Week 6 Checkpoint
**Question**: Does N1 compile successfully?
- **If YES**: Continue to generational GC
- **If NO**: Debug, tune, or pivot strategy

### Week 10 Checkpoint
**Question**: Are performance targets met?
- **If YES**: Move to production phase
- **IF NO**: Additional optimization week

---

## 🔧 Tools & Infrastructure

### Development
```bash
# Build with V2
go build -tags=memv2 ./cmd/morph

# Run tests
go test ./pkg/compiler/runtime/...

# Benchmark
go test -bench=. ./pkg/compiler/runtime/

# Memory profiling
valgrind --leak-check=full ./morph build test.fox
```

### Debugging
```bash
# Enable debug mode
export MORPH_MEM_DEBUG=1
./morph build test.fox

# Dump stats
export MORPH_MEM_STATS=stats.json
./morph build test.fox
```

---

## 📊 Success Metrics Summary

| Metric                  | V1 Current | V2 Target | Improvement |
|-------------------------|------------|-----------|-------------|
| Peak Memory (N1)        | 8GB+ (OOM) | < 4GB     | ↓50%+       |
| Object Header Size      | 72 bytes   | 16 bytes  | ↓78%        |
| Compilation Time        | 950ms      | < 500ms   | ↓47%+       |
| GC Pause (minor)        | 100ms+     | < 5ms     | ↓95%+       |
| GC Pause (major)        | 200ms+     | < 50ms    | ↓75%+       |
| Memory Overhead         | 70%+       | < 20%     | ↓71%+       |
| Crashes (OOM)           | Common     | Zero      | ↓100%       |

---

## 🚨 Risks & Mitigations

### Risk 1: Arena too memory hungry
**Mitigation**: Use smaller arenas (1MB instead of 2MB), more overhead but safer

### Risk 2: Generational GC bugs
**Mitigation**: Extensive testing, write barrier verification, use conservative GC fallback

### Risk 3: Performance regression
**Mitigation**: Continuous benchmarking, rollback to V1 if needed

### Risk 4: Timeline slippage
**Mitigation**: Focus on COMPILER mode first (Weeks 1-6), defer GC if needed

---

## ✅ Go/No-Go Criteria

### Minimum Viable Product (Week 6)
- ✅ N1 compiles successfully on 8GB machine
- ✅ Memory usage < 50% of V1
- ✅ No crashes or corruption
- ✅ All unit tests pass

**If these are met → Continue to generational GC**
**If not → Debug and fix before proceeding**

### Production Ready (Week 12)
- ✅ All MorphSH files compile
- ✅ Stress tests pass
- ✅ No memory leaks
- ✅ Documentation complete
- ✅ Performance targets met

**If these are met → Merge to main**
**If not → Additional stabilization week**

---

## 📝 Weekly Reports

Each week, document:
1. Tasks completed
2. Metrics achieved
3. Issues encountered
4. Next week plan

Store in: `.vzoel.jules/session-notes/memory-v2-week-N.md`

---

## 🎯 Post-Launch

### Month 1-3: Stabilization
- Monitor production usage
- Fix bugs as discovered
- Tune based on real workloads

### Month 4-6: Advanced Features
- Incremental GC
- Concurrent marking
- Compaction

### Month 7-12: FoxVM Preparation
- JIT code cache
- Register allocation
- Stack maps for precise GC

---

*Created: 2025-12-27*
*Owner: Morph Core Team*
*Status: Approved for Implementation*
