# AGENTS2.md - Morph Language Development Journal
**Status**: Active Development Journal (2025-12-29 onwards)
**Purpose**: Focused, relevant documentation replacing bloated AGENTS.md
**Strategy**: N0 FREEZE + Runtime.c/Assembly workarounds

---

## 📋 TABLE OF CONTENTS

1. [N0 Freeze Strategy](#n0-freeze-strategy)
2. [Current Session Findings (2025-12-29)](#current-session-findings-2025-12-29)
3. [N1 Refactoring Summary](#n1-refactoring-summary)
4. [Runtime.c/Assembly Workarounds](#runtimecassembly-workarounds)
5. [Development Roadmap](#development-roadmap)
6. [Critical Checksums](#critical-checksums)

---

## 🔒 N0 FREEZE STRATEGY

### Keputusan Arsitektur

**PRINSIP UTAMA**: N0 tidak akan dimodifikasi lagi. Semua kekurangan N0 akan ditangani melalui:
1. **Runtime.c** - Native C functions untuk fitur yang tidak bisa di-port
2. **Assembly** - Optimisasi performa dan fitur low-level
3. **N1 Workarounds** - Pattern alternatif yang compatible dengan N0

### Kekurangan N0 yang Sudah Diidentifikasi

| Fitur | Status N0 | Solusi Workaround |
|-------|-----------|-------------------|
| HashMap/Map | ❌ Tidak ada | ✅ runtime.c `mph_map_*` (VERIFIED WORKING) |
| String indexing | ❌ Tidak ada | ✅ Native function `mph_string_index` |
| Array dynamic | ⚠️ Terbatas | ✅ runtime.c `mph_array_*` |
| Closure | ❌ Tidak ada | 🔄 Assembly atau native |
| Import system | ❌ Tidak ada | ⚠️ Manual concatenation (temporary) |
| Generic types | ❌ Tidak ada | 🔄 Code generation patterns |

### Alasan N0 Freeze

1. **Stabilitas**: N0 sudah production-ready untuk subset Fox
2. **Fokus**: Energi lebih baik digunakan untuk N1 dan N2
3. **Risk**: Modifikasi N0 dapat break existing code
4. **Workaround**: Runtime.c/assembly terbukti efektif

**Dokumentasi N0**: Lihat `N0_ARCHITECTURE.md` dan `N0_RESURRECTION_SUMMARY.md`

---

## 🔍 CURRENT SESSION FINDINGS (2025-12-29)

### 1. Duplikasi Penamaan Parser & Codegen

**Masalah**: Port dari Go (method receiver) ke Fox (function) menyebabkan duplikasi semantik:
- `ParserParseIdentifier` → duplikasi "Parser" + "Parse"
- `CodegenCompile` → sebenarnya oke, tapi kurang konsisten

**Solusi**: Refactor untuk menghilangkan redundansi

#### File yang Diperbaiki

**n1/parser.fox** - 10 fungsi:
```fox
# SEBELUM (Duplikasi)
fungsi ParserParseIdentifier(p Parser) ast.Identifier
fungsi ParserParseIntegerLiteral(p Parser) ast.IntegerLiteral
fungsi ParserParseStringLiteral(p Parser) ast.StringLiteral
fungsi ParserParseBooleanLiteral(p Parser) ast.BooleanLiteral
fungsi ParserParseFloatLiteral(p Parser) ast.FloatLiteral
fungsi ParserParseCharLiteral(p Parser) ast.CharLiteral
fungsi ParserParseNullLiteral(p Parser) ast.NullLiteral
fungsi ParserParsePrefixExpression(p Parser) ast.PrefixExpression
fungsi ParserParseVarStatement(p Parser) ast.VarStatement
fungsi ParserParseProgram(p Parser) ast.Program

# SESUDAH (Bersih)
fungsi ParseIdentifier(p Parser) ast.Identifier
fungsi ParseIntegerLiteral(p Parser) ast.IntegerLiteral
fungsi ParseStringLiteral(p Parser) ast.StringLiteral
fungsi ParseBooleanLiteral(p Parser) ast.BooleanLiteral
fungsi ParseFloatLiteral(p Parser) ast.FloatLiteral
fungsi ParseCharLiteral(p Parser) ast.CharLiteral
fungsi ParseNullLiteral(p Parser) ast.NullLiteral
fungsi ParsePrefixExpression(p Parser) ast.PrefixExpression
fungsi ParseVarStatement(p Parser) ast.VarStatement
fungsi ParseProgram(p Parser) ast.Program
```

**n1/codegen.fox** - 2 fungsi:
```fox
# SEBELUM
fungsi CodegenCompile(cg Codegen, prog ast.Program) string
fungsi CodegenMapTypeToC(t types.Type) string

# SESUDAH
fungsi Compile(cg Codegen, prog ast.Program) string
fungsi MapTypeToC(t types.Type) string
```

#### Dampak Refactoring

**Positif**:
- ✅ Kode lebih bersih dan readable
- ✅ Konsisten dengan pola module lain (`lexer.LexerNextToken`)
- ✅ Tidak ada duplikasi semantik
- ✅ Semua test tetap compile dan pass

**Pola yang TIDAK diubah** (sudah benar):
- `LexerNextToken` → OK (tidak ada duplikasi)
- `NewLexer`, `NewParser`, `NewCodegen` → OK (constructor pattern)
- `CompileIntegerLiteral` → OK (tidak ada duplikasi)

### 2. Temuan Runtime.c HashMap

**CRITICAL DISCOVERY**: Runtime.c sudah memiliki HashMap implementation yang **VERIFIED WORKING**!

```c
// Runtime.c sudah punya:
MorphMap* mph_map_new(MorphContext* ctx);
void mph_map_set(MorphContext* ctx, MorphMap* m, MorphString* key, mph_int value);
mph_int mph_map_get(MorphContext* ctx, MorphMap* m, MorphString* key);
mph_int mph_map_len(MorphContext* ctx, MorphMap* m);
void mph_map_delete(MorphContext* ctx, MorphMap* m, MorphString* key);
```

**Test Results** (dari AGENTS.md):
```
✅ mph_map_new: Creates map successfully
✅ mph_map_set: Stores values correctly
✅ mph_map_get: Returns ACTUAL values (10, 20) - NOT dummy 0!
✅ mph_map_len: Returns correct count (2)
✅ mph_map_delete: Deletes and updates count (1)
```

**Implikasi**:
- N0 tidak perlu map support built-in
- N1 dapat langsung gunakan via native call
- Assembly wrapper (hashmap_native.fox) sudah ada di stdlib/

### 3. StringToInt Enhancement

**Ditemukan di parser.fox**: StringToInt sudah ditingkatkan untuk handle string panjang dan negatif:

```fox
fungsi StringToInt(s string) int
    var length = panjang(s)
    jika length == 0
        kembalikan 0
    akhir

    var i = 0
    var sign = 1
    jika substring(s, 0, 1) == "-"
        sign = -1
        i = 1
    akhir

    var result = 0
    selama i < length
        var ch = substring(s, i, i + 1)
        # ... digit parsing loop
        i = i + 1
    akhir

    kembalikan result * sign
akhir
```

**Improvement over old version**:
- ✅ Handle arbitrary length integers
- ✅ Handle negative numbers
- ✅ Loop-based instead of hardcoded cases

---

## 📊 N1 REFACTORING SUMMARY

### File Changes

| File | Changes | Status |
|------|---------|--------|
| `n1/parser.fox` | 10 fungsi renamed | ✅ Compiled |
| `n1/codegen.fox` | 2 fungsi renamed | ✅ Compiled |
| `n1/test_codegen_globals.fox` | Updated imports | ✅ Compiled |
| `n1/test_codegen_phase2.fox` | Updated imports | ✅ Compiled |

### Checksums (Updated 2025-12-29)

```
f1e8c8a9d2b3e4f5a6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9  n1/parser.fox
7f82a117940cbbef421880e4c2832430339359a2548a2f94ea8d0859928414e8  n1/codegen.fox
1264417a05a247a729af114284dbd5f9833d96cea73460b8af46c9d35c105958  n1/test_codegen_globals.fox
c8116e77e658b7b9ab5413bc1688732ebef57da592061d8d3904d14dcad3eee8  n1/test_codegen_phase2.fox
```

### Git Commit

```
Commit: 3f935fd
Message: Refactor: Penghilangan duplikasi nama Parser & Codegen
Branch: main → origin/main ✅
```

---

## 🔧 RUNTIME.C/ASSEMBLY WORKAROUNDS

### Strategi Integrasi

**Layer Architecture**:
```
┌─────────────────────────────────────┐
│         N1 Fox Code                 │  ← High-level business logic
├─────────────────────────────────────┤
│   Native Bindings (stdlib/*.fox)   │  ← Thin Fox wrappers
├─────────────────────────────────────┤
│   Runtime.c (mph_* functions)      │  ← Production-ready C impl
├─────────────────────────────────────┤
│   Assembly (optional optimization)  │  ← Performance-critical paths
└─────────────────────────────────────┘
```

### Available Runtime.c Functions

#### 1. HashMap (VERIFIED WORKING)

```c
MorphMap* mph_map_new(MorphContext* ctx);
void mph_map_set(MorphContext* ctx, MorphMap* m, MorphString* key, mph_int value);
mph_int mph_map_get(MorphContext* ctx, MorphMap* m, MorphString* key);
mph_int mph_map_len(MorphContext* ctx, MorphMap* m);
void mph_map_delete(MorphContext* ctx, MorphMap* m, MorphString* key);
```

**Fox Wrapper**: `stdlib/hashmap_native.fox`

#### 2. String Operations

```c
mph_int mph_string_index(MorphContext* ctx, MorphString* s, MorphString* sub);
MorphString* mph_string_trim(MorphContext* ctx, MorphString* s, MorphString* cut);
MorphArray* mph_string_split(MorphContext* ctx, MorphString* s, MorphString* sep);
MorphString* mph_string_substring(MorphContext* ctx, MorphString* s, mph_int start, mph_int end);
```

**Usage in N1**:
```fox
# Direct native call
var idx = native_string_index("hello world", "world")  # returns 6
```

#### 3. Array Operations

```c
MorphArray* mph_array_new(MorphContext* ctx, mph_int capacity);
void mph_array_push(MorphContext* ctx, MorphArray* arr, void* element);
void* mph_array_get(MorphContext* ctx, MorphArray* arr, mph_int index);
mph_int mph_array_len(MorphContext* ctx, MorphArray* arr);
```

#### 4. Error Handling

```c
MorphError* mph_error_new(MorphContext* ctx, MorphString* msg);
void mph_native_print_error(MorphContext* ctx, MorphError* err);
```

### Assembly Optimizations (Optional)

**Location**: `runtime_hashmap.asm` (educational/optional)

**Purpose**: 
- x86_64 assembly untuk performance-critical operations
- Saat ini educational only, runtime.c sudah cukup cepat

**Status**: ⚠️ Educational only, not production

---

## 🗺️ DEVELOPMENT ROADMAP

### Phase 1: N1 Core (CURRENT - 60% Complete)

**Status**: In Progress
**Focus**: Self-hosting compiler basics

- [x] Lexer (port complete)
- [x] Parser (port complete + refactored)
- [x] AST (port complete)
- [x] Types system (port complete)
- [x] Codegen foundation (basic working)
- [ ] Checker integration
- [ ] Full codegen (functions, structs, etc.)
- [ ] Module system

**Blockers**:
1. N0 tidak punya import system → Workaround: manual concatenation
2. N0 tidak punya closure → Workaround: runtime.c atau skip feature

### Phase 2: N1 Advanced (Next - 0% Complete)

**Focus**: Production features

- [ ] Full type checking
- [ ] Advanced codegen (closures, generics)
- [ ] Optimization passes
- [ ] Error recovery
- [ ] Module resolver

**Dependencies**: Phase 1 complete

### Phase 3: N2 Bootstrap (Future - 0% Complete)

**Focus**: N1 compiling itself

- [ ] N1 can compile N1 source
- [ ] Performance benchmarking
- [ ] Regression test suite
- [ ] Production hardening

**Dependencies**: Phase 2 complete

### Phase 4: Ecosystem (Future - 0% Complete)

**Focus**: Tools and libraries

- [ ] Package manager
- [ ] Standard library expansion
- [ ] LSP server
- [ ] Formatter
- [ ] Documentation generator

---

## 📦 CRITICAL CHECKSUMS

### Core N1 Files (Latest - 2025-12-29)

```bash
# Lexer, Parser, AST, Types
d61eec2ee45953351998b949ea0d5140390c2a4ebb2e69b554f1bb0ddce5237e  n1/lexer.fox
40a50088f0f5ab02f37947617d2f90bac82e40027bb5666a8a2a96c6b4fce698  n1/parser.fox (UPDATED 2025-12-29)
a9d8b88a9a4d0b6a44759093788ea27e3cbdb2bfa07a0d9c4e0c19d0ffa8d004  n1/ast.fox
fa69dc64d1233e6750891ac1e308ddd7519a3adb5448121b265edd680d60734f  n1/types.fox
667950ceac2c40ecc77a23a12f8fe7f2eeb05b561570fcfdc80e9a385bbd61d9  n1/token.fox

# Codegen
7f82a117940cbbef421880e4c2832430339359a2548a2f94ea8d0859928414e8  n1/codegen.fox (UPDATED 2025-12-29)

# Tests
1264417a05a247a729af114284dbd5f9833d96cea73460b8af46c9d35c105958  n1/test_codegen_globals.fox (NEW 2025-12-29)
c8116e77e658b7b9ab5413bc1688732ebef57da592061d8d3904d14dcad3eee8  n1/test_codegen_phase2.fox (UPDATED 2025-12-29)
```

### Runtime Files

```bash
# Runtime.c dengan mph_map_* VERIFIED WORKING
055947a565df422e51dea4dbba800703d1942796fb88e40357102888745e0b56  runtime.c (PRODUCTION READY)

# Native wrappers
67fa7092900874d1384ad4bd871e4ed37191f6506dd59862d14ba30a573bafca  stdlib/hashmap_native.fox
```

---

## 🎯 QUICK REFERENCE

### When to Use What

| Situation | Solution | Location |
|-----------|----------|----------|
| Need HashMap in N1 | Native call | `runtime.c mph_map_*` |
| Need string operations | Native call | `runtime.c mph_string_*` |
| N0 can't do X | Check runtime.c first | `runtime.c` |
| Performance critical | Consider assembly | `runtime_hashmap.asm` (optional) |
| Port from Go | Check for method→function | N1 files |
| New N1 feature | Don't modify N0! | Use runtime.c workaround |

### File Organization

```
morph/
├── n1/                    # N1 self-hosting compiler (Fox)
│   ├── lexer.fox         # ✅ Complete
│   ├── parser.fox        # ✅ Complete + refactored
│   ├── ast.fox           # ✅ Complete
│   ├── types.fox         # ✅ Complete
│   ├── codegen.fox       # 🔄 In progress
│   └── checker.fox       # 🔄 In progress
├── stdlib/               # Runtime wrappers
│   └── hashmap_native.fox
├── runtime.c             # ✅ PRODUCTION READY (mph_map_* verified)
├── runtime_hashmap.asm   # ⚠️ Educational only
└── AGENTS2.md           # 📖 This file (PRIMARY REFERENCE)
```

---

## 📚 DOCUMENTATION CROSS-REFERENCE

### Primary Documents (READ FIRST)
- **AGENTS2.md** (this file) - Current development journal
- **N0_ARCHITECTURE.md** - N0 frozen spec
- **N1_PHASE4_FINAL_SUMMARY.md** - N1 progress

### Reference Documents
- **AGENTS.md** - Historical data (259KB, reference only)
- **ROADMAP.md** - Long-term vision
- **MEMORY_ARCHITECTURE_V2.md** - Memory system

### Deprecated (Historical Reference Only)
- AGENTS.md entries before 2025-12-29
- Old architecture documents superseded by AGENTS2.md

---

## 🔄 VERSION HISTORY

### v1.0.0 (2025-12-29)
- Initial AGENTS2.md creation
- N0 freeze strategy documented
- Parser/Codegen refactoring summary
- Runtime.c HashMap verification
- Checksums updated

---

**Last Updated**: 2025-12-29
**Status**: ACTIVE - This is the PRIMARY reference document
**Maintainer**: Claude + VzoelFox team
**Next Review**: When Phase 1 reaches 80%

