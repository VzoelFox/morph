# Morph Memory Architecture V2 - Long-Term Design
**Status**: Design Document (Implementation Pending)
**Target**: N0 → N1 → N3 → N4 (FoxVM)
**Created**: 2025-12-27
**Lifetime**: 5+ years

---

## 🎯 Design Goals

### Primary Objectives
1. **Efficiency**: Support compilation workloads (high-churn, short-lived)
2. **Scalability**: Support long-running services (multi-GB heaps)
3. **Flexibility**: Multiple allocation strategies for different use cases
4. **Predictability**: Tunable GC with bounded pause times
5. **Observability**: Built-in profiling and metrics
6. **Future-Proof**: Ready for FoxVM bytecode + JIT

### Non-Goals (For Now)
- Real-time GC (soft real-time is enough)
- Distributed memory management
- Automatic NUMA optimization

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Morph Memory System V2                    │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Compiler   │  │   Runtime    │  │     VM       │      │
│  │     Mode     │  │     Mode     │  │    Mode      │      │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘      │
│         │                  │                  │              │
│         └──────────────────┼──────────────────┘              │
│                            ▼                                 │
│              ┌─────────────────────────┐                     │
│              │  Allocation Dispatcher  │                     │
│              └────────────┬────────────┘                     │
│                           │                                  │
│         ┌─────────────────┼─────────────────┐               │
│         ▼                 ▼                 ▼               │
│  ┌────────────┐    ┌────────────┐   ┌────────────┐         │
│  │   Arena    │    │    Pool    │   │   Heap     │         │
│  │ Allocator  │    │ Allocator  │   │ Allocator  │         │
│  └────────────┘    └────────────┘   └────────────┘         │
│         │                 │                 │               │
│         └─────────────────┼─────────────────┘               │
│                           ▼                                  │
│              ┌─────────────────────────┐                     │
│              │   Generational GC       │                     │
│              │  (Young + Old + Large)  │                     │
│              └─────────────────────────┘                     │
│                           │                                  │
│              ┌─────────────────────────┐                     │
│              │  Memory Metrics & Stats │                     │
│              └─────────────────────────┘                     │
└─────────────────────────────────────────────────────────────┘
```

---

## 📦 Component Design

### 1. **Allocation Dispatcher** (Smart Router)

```c
typedef enum {
    MORPH_MODE_COMPILER,    // Short-lived, high-churn
    MORPH_MODE_RUNTIME,     // Balanced workload
    MORPH_MODE_VM,          // Long-lived, low-churn
    MORPH_MODE_SERVER       // Continuous, bounded memory
} MorphWorkloadMode;

typedef struct MorphMemConfig {
    MorphWorkloadMode mode;
    size_t heap_size_hint;      // Expected max heap size
    size_t gc_threshold;         // GC trigger threshold
    uint8_t gc_pause_target_ms;  // Target pause time
    uint8_t enable_generational; // Use generational GC
    uint8_t enable_compaction;   // Use mark-compact
    uint8_t enable_metrics;      // Collect statistics
} MorphMemConfig;
```

**Strategy Selection:**
```
COMPILER Mode:
  → Arena Allocator (90% of allocations)
  → Pool Allocator (AST nodes, tokens)
  → No GC during compilation
  → Bulk free at end

RUNTIME Mode:
  → Generational GC (young + old)
  → Pool for common objects
  → Adaptive threshold tuning

VM Mode:
  → Advanced GC (incremental, concurrent)
  → JIT code cache management
  → Large object handling

SERVER Mode:
  → Bounded heap size
  → Predictable GC pauses
  → Memory pressure callbacks
```

---

### 2. **Object Header V2** (Minimal Overhead)

```c
// Base header: 16 bytes (down from 72!)
typedef struct ObjectHeader {
    uint32_t size : 24;        // 16MB max object (enough for most)
    uint32_t type_id : 7;      // 128 types (extendable via RTTI)
    uint32_t marked : 1;       // GC mark bit
    uint8_t  generation;       // 0=young, 1=old, 255=large
    uint8_t  flags;            // Custom flags
    uint16_t reserved;         // Future use
} ObjectHeader;

// Extended header for special cases (24 bytes total)
typedef struct ObjectHeaderExt {
    ObjectHeader base;
    struct ObjectHeaderExt* next;  // For free lists / GC lists
    uint64_t metadata;             // App-specific
} ObjectHeaderExt;
```

**Benefits:**
- **16 bytes** vs 72 bytes = **78% reduction**
- Cache-friendly (fits in single cache line with payload)
- Bitfield packing for metadata
- Optional extension for advanced features

**Trade-offs:**
- 16MB object size limit (acceptable - large objects use special path)
- 128 type limit (use RTTI table for more)

---

### 3. **Arena Allocator** (Compiler Mode)

```c
typedef struct Arena {
    void* buffer;
    size_t capacity;
    size_t offset;
    struct Arena* next;
} Arena;

typedef struct ArenaAllocator {
    Arena* current;
    Arena* free_list;        // Reusable arenas
    size_t default_size;     // 2MB per arena
    size_t total_allocated;
} ArenaAllocator;

// Fast bump-pointer allocation
void* arena_alloc(ArenaAllocator* aa, size_t size) {
    Arena* a = aa->current;

    // Fast path: fits in current arena
    if (a->offset + size <= a->capacity) {
        void* ptr = (char*)a->buffer + a->offset;
        a->offset += ALIGN_UP(size, 8);
        return ptr;
    }

    // Slow path: need new arena
    return arena_alloc_slow(aa, size);
}

// Bulk free (entire arena)
void arena_reset(ArenaAllocator* aa) {
    Arena* a = aa->current;
    while (a) {
        a->offset = 0;  // Just reset pointer!
        a = a->next;
    }
}
```

**Performance:**
- **O(1) allocation** - bump pointer
- **O(1) free** - bulk reset
- **Zero GC overhead** during compilation
- **Memory reuse** between compilations

**Usage Pattern:**
```c
// Compile one file
ArenaAllocator* aa = arena_new(2 * 1024 * 1024);  // 2MB arenas
compile_file("main.fox", aa);
arena_reset(aa);  // Free everything instantly!

// Compile next file (reuse memory)
compile_file("lexer.fox", aa);
```

---

### 4. **Pool Allocator** (Fixed-Size Objects)

```c
#define POOL_BLOCK_SIZE 4096

typedef struct PoolBlock {
    void* memory;
    uint32_t free_bitmap[POOL_BLOCK_SIZE / 32 / sizeof(uint32_t)];
    uint16_t free_count;
    uint16_t next_hint;      // Fast allocation hint
    struct PoolBlock* next;
} PoolBlock;

typedef struct Pool {
    size_t object_size;
    PoolBlock* blocks;
    PoolBlock* current;      // Block with free slots
} Pool;
```

**Size Classes:**
```c
// Common object sizes for Morph
Pool pools[] = {
    { .object_size = 32  },  // Small AST nodes, tokens
    { .object_size = 64  },  // Medium nodes, identifiers
    { .object_size = 128 },  // Large nodes, strings
    { .object_size = 256 },  // Complex structures
};
```

**Fast Allocation:**
```c
void* pool_alloc(Pool* p) {
    PoolBlock* b = p->current;

    // Find free slot using bitmap
    int slot = __builtin_ffs(b->free_bitmap[b->next_hint]);
    if (slot) {
        b->free_bitmap[b->next_hint] &= ~(1 << (slot - 1));
        b->free_count--;
        return (char*)b->memory + (slot - 1) * p->object_size;
    }

    // Fallback: find next block with space
    return pool_alloc_slow(p);
}
```

**Benefits:**
- **O(1) alloc/free** with bitmap
- **Zero fragmentation** (fixed size)
- **Cache-friendly** (objects clustered)
- **Predictable** memory usage

---

### 5. **Generational GC** (Runtime Mode)

```
┌─────────────────────────────────────────────────────┐
│                 Generational Heap                    │
├─────────────────────────────────────────────────────┤
│                                                       │
│  ┌────────────────┐  ┌────────────────┐  ┌────────┐ │
│  │  Young Gen     │  │    Old Gen     │  │ Large  │ │
│  │  (2MB)         │  │    (32MB)      │  │ Object │ │
│  │                │  │                │  │ Space  │ │
│  │  [Nursery]     │  │  [Tenured]     │  │ (>1MB) │ │
│  │                │  │                │  │        │ │
│  │  Fast bump     │  │  Mark-sweep    │  │ Direct │ │
│  │  allocation    │  │  or compact    │  │ mmap   │ │
│  │                │  │                │  │        │ │
│  └────────────────┘  └────────────────┘  └────────┘ │
│         │                    │                 │     │
│         └────────────────────┼─────────────────┘     │
│                              ▼                       │
│                    ┌──────────────────┐              │
│                    │  Write Barrier   │              │
│                    │  (Track Old→Young│              │
│                    │   references)    │              │
│                    └──────────────────┘              │
└─────────────────────────────────────────────────────┘
```

**Algorithm:**

```c
typedef struct GenerationalHeap {
    // Young generation (evacuated)
    void* young_start;
    void* young_current;
    void* young_end;

    // Old generation (mark-sweep or mark-compact)
    ObjectHeaderExt* old_objects;

    // Remembered set (old → young pointers)
    void** remembered_set;
    size_t remembered_count;
    size_t remembered_capacity;

    // Statistics
    uint64_t young_gc_count;
    uint64_t old_gc_count;
    uint64_t promoted_count;
} GenerationalHeap;
```

**Minor GC (Young Gen Only):**
```c
void gc_minor(GenerationalHeap* gh) {
    // Roots: stack + remembered set
    mark_from_roots();
    mark_from_remembered_set(gh);

    // Evacuate live objects
    void* scan = young_start;
    void* free = young_start;

    while (scan < young_current) {
        ObjectHeader* obj = (ObjectHeader*)scan;
        if (obj->marked) {
            // Promote old objects to old gen
            if (obj->generation > GEN_PROMOTION_THRESHOLD) {
                promote_to_old(gh, obj);
            } else {
                // Copy to compacted young gen
                memmove(free, scan, obj->size + sizeof(ObjectHeader));
                forward_references(obj, free);
                free += obj->size + sizeof(ObjectHeader);
                obj->generation++;
            }
        }
        scan += obj->size + sizeof(ObjectHeader);
    }

    young_current = free;  // New allocation point
}
```

**Performance Characteristics:**
```
Minor GC:
  Frequency: Every 2-4MB allocated
  Pause time: 1-5ms (linear in live objects)
  Throughput: 95%+ (5% GC overhead)

Major GC:
  Frequency: Every 32MB allocated (or threshold)
  Pause time: 10-50ms (tunable)
  Throughput: 90%+ (10% GC overhead)

Large Objects:
  No copying overhead
  Direct mmap/munmap
  Rare in practice
```

---

### 6. **Configuration Presets**

```c
// Preset configurations for common use cases

MorphMemConfig MORPH_CONFIG_COMPILER = {
    .mode = MORPH_MODE_COMPILER,
    .heap_size_hint = 256 * 1024 * 1024,  // 256MB
    .gc_threshold = 0,                     // No GC during compilation
    .gc_pause_target_ms = 0,
    .enable_generational = 0,
    .enable_compaction = 0,
    .enable_metrics = 0
};

MorphMemConfig MORPH_CONFIG_RUNTIME = {
    .mode = MORPH_MODE_RUNTIME,
    .heap_size_hint = 512 * 1024 * 1024,  // 512MB
    .gc_threshold = 64 * 1024 * 1024,     // 64MB
    .gc_pause_target_ms = 10,
    .enable_generational = 1,
    .enable_compaction = 0,
    .enable_metrics = 1
};

MorphMemConfig MORPH_CONFIG_VM = {
    .mode = MORPH_MODE_VM,
    .heap_size_hint = 2ULL * 1024 * 1024 * 1024,  // 2GB
    .gc_threshold = 128 * 1024 * 1024,            // 128MB
    .gc_pause_target_ms = 5,
    .enable_generational = 1,
    .enable_compaction = 1,  // For long-running VMs
    .enable_metrics = 1
};

MorphMemConfig MORPH_CONFIG_SERVER = {
    .mode = MORPH_MODE_SERVER,
    .heap_size_hint = 1024 * 1024 * 1024,  // 1GB (bounded)
    .gc_threshold = 128 * 1024 * 1024,     // 128MB
    .gc_pause_target_ms = 5,
    .enable_generational = 1,
    .enable_compaction = 1,
    .enable_metrics = 1
};

// Auto-detection based on environment
MorphMemConfig morph_detect_config() {
    size_t available_ram = get_available_ram();

    if (available_ram < 512 * 1024 * 1024) {
        // Low memory: aggressive GC
        return (MorphMemConfig){
            .heap_size_hint = available_ram / 4,
            .gc_threshold = available_ram / 8,
            // ...
        };
    }

    // Default: balanced runtime mode
    return MORPH_CONFIG_RUNTIME;
}
```

---

## 📊 Memory Metrics & Observability

```c
typedef struct MorphMemStats {
    // Allocation stats
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t current_live;
    uint64_t peak_live;

    // GC stats
    uint64_t gc_count;
    uint64_t gc_time_us;
    uint64_t gc_pause_max_us;
    uint64_t gc_pause_avg_us;

    // Object stats
    uint64_t object_count;
    uint64_t object_count_by_type[128];
    uint64_t bytes_by_type[128];

    // Heap stats
    uint64_t arena_count;
    uint64_t arena_bytes;
    uint64_t pool_count;
    uint64_t pool_bytes;
    uint64_t heap_bytes;

    // Generational stats
    uint64_t young_gc_count;
    uint64_t old_gc_count;
    uint64_t promoted_objects;
    uint64_t promoted_bytes;
} MorphMemStats;

// Export metrics to file
void morph_mem_dump_stats(MorphContext* ctx, const char* path);

// Live monitoring
void morph_mem_print_stats(MorphContext* ctx);
```

**Output Example:**
```
=== Morph Memory Statistics ===
Mode: COMPILER
Uptime: 2.5s

Allocations:
  Total: 125,430 objects (245 MB)
  Live:   89,241 objects (198 MB)
  Peak:  102,356 objects (215 MB)

Breakdown:
  Arena:  95% (237 MB in 119 arenas)
  Pool:    4% (8 MB in 4 pools)
  Heap:    1% (2 MB)

GC:
  Collections: 0
  Time: 0ms
  Max Pause: N/A

Top Types:
  1. Identifier      45,231 objects (12 MB)
  2. InfixExpr       23,145 objects (18 MB)
  3. StringLiteral   15,678 objects (25 MB)
```

---

## 🛠️ Implementation Plan

### **Phase 1: Foundation (Week 1-2)**
```
✓ Design document (this file)
□ Create morph_mem_v2.h header
□ Implement ObjectHeader V2
□ Implement MorphMemConfig
□ Create allocation dispatcher
□ Write unit tests
```

### **Phase 2: Allocators (Week 3-4)**
```
□ Implement Arena Allocator
□ Implement Pool Allocator
□ Implement size class routing
□ Benchmark allocation performance
□ Compare vs current system
```

### **Phase 3: Generational GC (Week 5-6)**
```
□ Implement young generation (copying)
□ Implement old generation (mark-sweep)
□ Implement write barrier
□ Implement promotion logic
□ Tune thresholds
```

### **Phase 4: Integration (Week 7-8)**
```
□ Integrate with N0 compiler
□ Test N1 compilation (MorphSH)
□ Validate memory usage < 8GB
□ Performance benchmarks
□ Documentation
```

### **Phase 5: Optimization (Week 9-10)**
```
□ Profile hot paths
□ Optimize GC marking
□ Implement incremental GC (optional)
□ Add concurrent marking (optional)
□ Final tuning
```

### **Phase 6: Production (Week 11-12)**
```
□ Stress testing
□ Memory leak detection
□ Release candidate
□ Documentation
□ Merge to main
```

---

## 🎯 Success Metrics

### **Compiler Mode (N1 compilation)**
```
Current (V1):
  Memory: 4-6 GB peak (OOM at 8GB with large threshold)
  Time:   Variable (GC thrashing)
  Success: 50% (depends on threshold tuning)

Target (V2):
  Memory: < 2GB peak (75% reduction)
  Time:   20% faster (no GC overhead)
  Success: 100% (arena allocation, no GC)
```

### **Runtime Mode**
```
Target:
  GC Pause: < 10ms (p99)
  Throughput: > 95%
  Heap Size: Bounded, predictable
```

### **VM Mode (Future - FoxVM)**
```
Target:
  GC Pause: < 5ms (p99)
  Throughput: > 97%
  Heap: 2GB+ supported
  Concurrent: Yes
```

---

## 🔄 Migration Path from V1

### **Step 1: Compatibility Layer**
```c
// Keep V1 API working
#define mph_alloc(ctx, size, type) \
    morph_alloc_v2(ctx, size, type, ALLOC_DEFAULT)

// New V2 API
void* morph_alloc_v2(MorphContext* ctx, size_t size,
                     MorphTypeInfo* type, AllocFlags flags);
```

### **Step 2: Gradual Adoption**
```c
// Runtime flag to switch modes
if (getenv("MORPH_MEM_V2")) {
    ctx->allocator = &v2_allocator;
} else {
    ctx->allocator = &v1_allocator;  // Fallback
}
```

### **Step 3: Feature Flag Rollout**
```
Week 1-2: V2 in testing branch
Week 3-4: V2 enabled for compiler mode only
Week 5-6: V2 enabled for runtime mode
Week 7-8: V2 default, V1 deprecated
Week 9+:  V1 removed
```

---

## 📚 References & Inspiration

- **Go GC**: Concurrent mark-sweep, write barriers
- **JVM G1GC**: Generational, region-based
- **V8 (JavaScript)**: Young/old generations, scavenger
- **Rust Allocator**: jemalloc, size classes
- **LuaJIT**: Arena allocation for short-lived data
- **OCaml**: Minor heap (young gen) + major heap

---

## 🔮 Future Enhancements (Post-FoxVM)

### **Advanced GC Modes**
- **Concurrent GC**: Background marking
- **Incremental GC**: Bounded pause times
- **Parallel GC**: Multi-threaded collection
- **Real-time GC**: Hard pause guarantees

### **Platform-Specific Optimizations**
- **NUMA awareness**: Allocate on local nodes
- **Huge pages**: Reduce TLB misses
- **Memory compression**: Compress old gen

### **JIT Integration**
- **Code cache**: Separate region for JIT code
- **Deoptimization**: Discard compiled code
- **Tiering**: Interpret → baseline JIT → optimizing JIT

---

## ✅ Approval & Next Steps

**Author**: Claude Sonnet 4.5 (with robust-reverse-engineer methodology)
**Reviewer**: VzoelFox
**Status**: Awaiting Approval

**Questions for Review:**
1. Is COMPILER mode (arena-based, no GC) the right choice for N1?
2. Should we implement all 3 allocators or start with arena + heap?
3. Is 16-byte ObjectHeader acceptable? (vs 32-byte for more features)
4. Target timeline: 12 weeks realistic or too aggressive?

**Next Action:**
- [ ] Review and approve design
- [ ] Create implementation branch
- [ ] Start Phase 1: Foundation code

---

*End of Design Document*
