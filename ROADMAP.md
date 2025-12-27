# Roadmap Pengembangan Bahasa Morph

**Last Updated**: 2025-12-27 20:15 WIB

## Visi
Membangun bahasa pemrograman prosedural yang aman (Error as Value), mudah dipahami (Bahasa Indonesia), dan mampu melakukan self-hosting dengan compiler yang robust dan production-ready.

---

## Fase 1: Fondasi (✅ Selesai)
- [x] **Identitas Project**: Rename Fox → Morph
- [x] **Spesifikasi Desain**: Draft awal (`DESIGN.md`) disetujui
- [x] **Lexer & Parser**: Porting dari referensi dan disesuaikan
- [x] **Tooling Awal**: `pkg/analysis` untuk generate context AI

---

## Fase 2: Semantik & Tipe (✅ Selesai)
- [x] **Type Checker**: Validasi tipe statis (int, string, bool, struct, array, map)
- [x] **Semantic Analyzer**: Validasi scope variable dan function signature
- [x] **AST Validations**: Pengecekan error logic (return di luar fungsi, dll)
- [x] **Interface System**: Duck typing, interface implementation check
- [x] **Generic Types**: []T arrays, map[K]V maps dengan type parameter checking
- [x] **Import System**: Module loading, cycle detection, qualified types

---

## Fase 3: Code Generation & Runtime (✅ Selesai)
- [x] **Tree-Walking Interpreter**: Eksekusi langsung kode Morph (`pkg/evaluator`)
- [x] **C Output Generator**: Transpile AST menjadi kode C
- [x] **Compiler Driver**: CLI untuk build file `.fox` menjadi executable
- [x] **Runtime System**: GC (Mark-and-Sweep), String ops, Array/Map, Channels
- [x] **Concurrency**: Pthreads integration, channel communication
- [x] **Error Handling**: Error as value, panic handling

---

## Fase 4: N0 Compiler Hardening (✅ Selesai - 2025-12-27)

### 4.1 N0 Resurrection ✅
- [x] **N0 Recovery**: Restored Go compiler from commit f2e51c4
- [x] **Component Validation**: 13,028 LoC across 66 files verified
- [x] **Documentation**: N0_ARCHITECTURE.md, N0_RESURRECTION_SUMMARY.md
- [x] **Backup Branch**: n0-resurrection-backup established

### 4.2 Security Hardening ✅
**Achievement**: 7.8/10 → 9.5/10 (PRODUCTION READY)

- [x] **Recursion Limits**: MAX_RECURSION_DEPTH=1000 in parser/checker/evaluator
- [x] **Panic Recovery**: Main wrapper for graceful internal error handling
- [x] **Memory V2 Fixes**: 3 critical bugs fixed (alignment, constants, pointers)
- [x] **Testing**: Deeply nested expressions caught gracefully, no regressions

**Status**: N0 is now safe for self-hosting and development work.

---

## Fase 5: Memory System Evolution (⏸️ Paused - Future)

### 5.1 Memory V1 Optimization ⏸️
- [x] **GC Verification**: Proven working (33MB → 2.7MB with low threshold)
- [x] **Swap Daemon**: LRU-based eviction to disk (.morph.vz/swap/)
- [x] **Page-Based Allocation**: Fast allocation with page reuse
- [ ] **Production Tuning**: Restore 64MB/8MB thresholds when needed

### 5.2 Memory V2 Integration (Future)
- [x] **Design Complete**: 16-byte ObjectHeader, generational GC, arena/pool allocators
- [x] **Bugs Fixed**: Alignment, constants, void pointer dereference
- [x] **Tests Pass**: All compilation errors resolved
- [ ] **Integration**: Test with full compiler workload
- [ ] **Validation**: Prove 99% memory reduction claim

**Expected**: 99% memory reduction vs Memory V1

---

## Fase 6: Self-Hosting (🔄 In Progress)

### 6.1 Bootstrap Type System ✅
- [x] **Phase 1**: Parser integration foundation (AST type annotation)
- [x] **Phase 2**: Real AST integration (pkg/parser node types)
- [x] **Phase 3**: Scope management (variable tracking)
- [x] **Phase 4**: Generic types ([]T arrays, map[K]V maps)
- [x] **Phase 5**: Interface type checking (duck typing validation)

### 6.2 Tree Walker Implementation (Pending)
- [ ] **Port morphsh/tree_walker.fox** to morphfox with essential functions
- [ ] **String & Logic**: IntToString, BoolToString, StringEquals (done via workaround)
- [ ] **Evaluation Core**: Expression evaluation, control flow

### 6.3 Self-Compilation Test (Pending)
- [ ] **N3 Compile N3**: Test self-hosting capability
- [ ] **Bootstrap Chain**: N0 → N1 → N2 → N3 validation
- [ ] **Robustness**: Ensure N3 maintains quality of N0

---

## Fase 7: Compiler Optimization (⏸️ Paused - Reset Needed)

### 7.1 Optimizer Roadmap - Honest Assessment ❌

**Previous Status**: Claimed "OPTIMIZATION COMPLETE" with 20-40x gains
**Reality**: Never successfully compiled, archived as aspirational

**Files Archived** (2025-12-27):
- `morphsh/archive_aspirational/lexer_optimized_ASPIRATIONAL.fox` (10KB)
- `morphsh/archive_aspirational/parser_optimized_ASPIRATIONAL.fox` (11KB)
- `morphsh/archive_aspirational/compiler_optimized_ASPIRATIONAL.fox` (11KB)

**Root Cause**: N0 compiler OOM at 31GB for 283-line files (algorithmic limitation)

### 7.2 New Optimization Strategy (Future)

**Approach**: Minimal, Incremental, Tested

#### Phase 1: Minimal Lexer Optimization (Week 1)
- [ ] Create `morphsh/lexer_minimal.fox` (<100 lines)
- [ ] Single optimization: Whitespace skipping OR single-char tokens
- [ ] **Validation**: Must compile with N0
- [ ] **Benchmark**: Prove 5-10% improvement on real files

#### Phase 2: Measure & Iterate (Week 2)
- [ ] Benchmark before/after with real metrics
- [ ] If successful (even 5% gain), add one more optimization
- [ ] **Rule**: No new code until previous code compiles

#### Phase 3: Parser Optimization (Week 3-4)
- [ ] Only if lexer successful
- [ ] One optimization at a time
- [ ] Continuous compilation testing
- [ ] Target: 10-15% improvement

#### Phase 4: Compiler Optimization (Week 5-6)
- [ ] Only if parser successful
- [ ] Incremental approach
- [ ] Target: 15-20% improvement

**Realistic Total Gain**: 30-45% overall (not 2000-4000% claimed earlier)

**Prerequisites** (one of):
1. Fix N0 memory algorithm fundamentally, OR
2. Integrate Memory V2 (99% reduction), OR
3. Build in tiny increments with continuous testing

---

## Fase 8: Fitur Lanjutan (Deferred)

*Akan dikerjakan setelah Self-Hosting stabil dan optimizer validated.*

- [ ] **Manajemen Memori**: Region-based Memory (Arenas) - Optional enhancement
- [ ] **Stdlib Expansion**: Network, File IO, JSON parsing
- [ ] **Tooling**: Language Server Protocol (LSP), debugger integration
- [ ] **Package Manager**: Dependency management, versioning
- [ ] **Cross-Platform**: Windows, macOS support (currently Linux only)

---

## Current Priorities (2025-12-27)

### High Priority
1. ✅ **N0 Security Hardening** (DONE - 9.5/10)
2. 🔄 **Self-Hosting Tree Walker** (In Progress)
3. ⏳ **N3 Self-Compilation Test** (Pending)

### Medium Priority
4. ⏸️ **Memory V2 Integration** (Tests pass, needs full integration)
5. ⏸️ **Production GC Tuning** (Restore 64MB/8MB when needed)

### Low Priority / Future
6. ⏸️ **Optimizer Rebuild** (Paused until prerequisites met)
7. ⏸️ **Stdlib Expansion** (After self-hosting stable)
8. ⏸️ **Error Deduplication** (Nice-to-have, not critical)

---

## Success Metrics

### Compiler Robustness
- ✅ Security: 9.5/10 (was 7.8/10)
- ✅ Recursion: Safe for 1000-level nesting
- ✅ Panic Handling: Graceful internal error recovery
- ⏳ Memory: Waiting for V2 integration

### Self-Hosting Capability
- ✅ Type System: Complete with interfaces & generics
- ⏳ Tree Walker: Partial (needs porting)
- ⏳ Self-Compilation: Not tested yet

### Performance
- ✅ GC: Proven working (380 cycles for 100K iterations)
- ⏸️ Compiler Speed: Baseline measured, optimization deferred
- ⏳ Memory Usage: V1 working, V2 ready for integration

---

## References

### Documentation
- **Design**: DESIGN.md
- **Security**: .vzoel.jules/session-2025-12-27-n0-security-hardening.md
- **Memory Investigation**: .vzoel.jules/session-2025-12-27-memory-investigation.md
- **N0 Architecture**: docs/N0_ARCHITECTURE.md
- **Optimizer Reality Check**: SWITCH_CASE_OPTIMIZATION.md

### Technical Debt
- **Tracker**: .vzoel.jules/technical-debt.morph.vz
- **Archive**: morphsh/archive_aspirational/README.md

### Git Commits
- **N0 Resurrection**: Commit f2e51c4 → 9592753
- **Security Hardening**: Commit 9592753
- **Documentation**: Commit 5908546

---

**Note**: This roadmap reflects honest assessment of current state. Previous optimistic claims about "OPTIMIZATION COMPLETE" have been corrected. We prioritize working, tested code over theoretical performance gains.

**Philosophy**: Ship working features incrementally > Ship perfect features never.
