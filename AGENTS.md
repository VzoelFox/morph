# Agents.md - Source of Truth untuk AI Agent

## Metadata Dokumen
- **Versi**: 1.48.0
- **Tanggal Dibuat**: 2025-12-20 06.10 WIB
- **Terakhir Diupdate**: 2025-12-25
- **Status**: Active

---

## Tujuan Dokumen
Dokumen ini adalah **single source of truth** untuk AI Agent dalam pengembangan bahasa pemrograman Morph. Setiap perubahan pada codebase akan tercatat di sini dengan checksum, timestamp, dan konteks untuk meminimalisir halusinasi dan menjaga konsistensi.

---



# ERROR as Value & Explisit Over implisit

---

## Struktur Project

```
project-root/
├── agents.md                 # File ini (source of truth)
├── .morph.vz/                # Direktori konteks dan snapshot
│   ├── snapshots/           # Snapshot state per sesi
│   ├── checksums/           # Checksum file per versi
│   └── context/             # Konteks sesi development
└── .vzoel.jules/            # Catatan hutang teknis & rekomendasi
    ├── technical-debt.morph.vz
    ├── next-improvements.morph.vz
    └── session-notes/
```

---

## Spesifikasi Bahasa Morph

### Status: Dalam Perencanaan
**Target**: Bahasa pemrograman yang bisa self-host dengan compiler bootstrap (Go -> LLVM)

### Fitur Core (Planned)
- [ ] Lexer & Parser
- [ ] AST Generator
- [ ] Type Checker (Static)
- [ ] Code Generator (LLVM)
- [ ] Self-hosting capability
- [ ] AI-assisted development dengan context tracking
- [ ] Concurrency (Goroutine-like)
- [ ] Semi-manual Memory Management

---

## Riwayat Perubahan
### Version 1.52.0 - 2025-12-25
**Checksum**: SHA256:GC_MARK_STACK_BLOCKS
**Perubahan**:
- **Runtime/GC**: Mengganti mark stack jadi block-based untuk menghindari realloc besar dan menurunkan spike latency saat GC.
- **Runtime/GC**: Menambahkan helper push/pop mark stack berbasis block agar pertumbuhan stack hanya alokasi block baru.
- **Runtime/Tests**: Menambahkan regresi test untuk struktur mark stack block.

**Konteks Sesi**:
- **Latency**: Mengurangi spike saat mark stack membesar karena tidak lagi realloc seluruh buffer.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:ba44b8fdd73fb5cee9aa3b0bc2105f15769ef72efd8f89f1655d3985d513ce0e)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:535e0ca5f7738edc5cb1758148eb751c83164a0eaf8a850704503493ecacd445)
- `pkg/compiler/runtime/runtime_test.go` (SHA256:08623f14dcfbff3b3fcb8be7aba9ad157a53ec11985f7c7aff881e4638df9b32)

### Version 1.51.0 - 2025-12-25
**Checksum**: SHA256:GC_FREE_LIST_PAGE_INDEX
**Perubahan**:
- **Runtime/GC**: Menambahkan free list per page dan pointer bebas ganda agar pembersihan free list saat recycle page tidak lagi scan global.
- **Runtime/GC**: Update allocator untuk melepas free entry dari daftar global dan per-page secara O(1).
- **Runtime/Tests**: Menambahkan regresi test untuk field free list per page di header runtime.

**Konteks Sesi**:
- **Latency**: Menghilangkan O(pages × free_list) scan pada GC sweep saat recycle page.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:b3590d3460cde7cfbbfb1939ecc37d69b6bf229e25f16c2527059022a5dd6220)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:5da99f8c1c3b4c7a497227e621b3de2baf7df80dcf4495304156095b493bc605)
- `pkg/compiler/runtime/runtime_test.go` (SHA256:358b22efdc1342badc3444ee777dd2a881cc264b4cd3ded868c421c7483835c4)

### Version 1.50.0 - 2025-12-25
**Checksum**: SHA256:MAP_TOMBSTONE_REHASH
**Perubahan**:
- **Runtime/Map**: Menambahkan `deleted_count` dan rehash saat tombstone tinggi untuk menekan latensi probing.
- **Runtime/Map**: Reuse slot deleted saat insert dan reset `deleted_count` saat resize.
- **Runtime/Tests**: Menambahkan regresi test untuk tombstone cleanup dan field header map.

**Konteks Sesi**:
- **Latency**: Mengurangi latency map akibat akumulasi tombstone pada operasi delete.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:464b52d020fc4cd8ca29f152af7f6c8a6925ea412e3b2239d942f8667be2b310)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:ee936f271b5a3fcdad9470496a753efdf9be94d6c165900eb182f6d824a400e5)
- `pkg/compiler/runtime/runtime_test.go` (SHA256:4e7270543e5e653e9ca5267508523884d784e144db0ed6d3f2ab4f9c5c8b9f9b)

### Version 1.49.0 - 2025-12-25
**Checksum**: SHA256:MAP_RUNTIME_TESTS
**Perubahan**:
- **Runtime/Tests**: Menambahkan regression test untuk memastikan `map->entries` selalu di-swap-in saat operasi set/get/delete dan `mph_map_resize` tetap tersedia serta dipanggil.

**Konteks Sesi**:
- **Testing**: Mengunci perilaku map swap-in/resize agar perubahan runtime berikutnya tidak menyebabkan regresi.

**File Terkait**:
- `pkg/compiler/runtime/runtime_test.go` (SHA256:6456f94906c5a97a422ede27efb2f6d4455479bcc2e8bf537c07a4e3228e93c8)

### Version 1.48.0 - 2025-12-25
**Checksum**: SHA256:MAP_ENTRIES_SWAPIN
**Perubahan**:
- **Runtime/Map**: Menambahkan swap-in pada `map->entries` untuk operasi set/get/delete agar akses aman saat page terswap.

**Konteks Sesi**:
- **Latency & Correctness**: Menghindari access ke entries yang masih terswap dan mengurangi risiko crash pada operasi map.

**File Terkait**:
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:3c1c4fe283b233218a3f74b29a0d0a02ad9fbe7259dbf964d46f167f7bd2d1fe)

### Version 1.47.0 - 2025-12-25
**Checksum**: SHA256:GC_PAGE_POINTER_AND_MAP_RESIZE
**Perubahan**:
- **Runtime/GC**: Menambahkan pointer page pada `ObjectHeader` untuk mempercepat lookup page saat GC mark dan swap-in.
- **Runtime/Map**: Menambahkan `mph_map_resize` dan auto-resize saat load factor >= 0.75 untuk mengurangi latency operasi map.

**Konteks Sesi**:
- **Latency**: Mengurangi overhead GC (page lookup) dan menghindari probing berlebih saat map penuh.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:44f2215cb0967590132756b8ab8b10a6a93fce36d4e9836897b7fac2c59294a2)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:031dd553c1598d6c85aadefc18655c0dd2fc8b42210157cb37d325bea14e9567)

### Version 1.46.0 - 2025-12-25
**Checksum**: SHA256:GC_SIZE_ACCOUNTING_FIX
**Perubahan**:
- **Runtime**: Memakai `ObjectHeader.size` untuk menghitung `live_bytes`, logging zombie, dan pengurangan `allocated_bytes` agar akurat untuk alokasi raw/buffer.
- **Runtime**: Menyetel ulang `size` saat reuse free list agar konsisten pada alokasi ulang.

**Konteks Sesi**:
- **Stabilization**: Memastikan akurasi accounting memori GC sehingga threshold dan statistik tidak meleset.

**File Terkait**:
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:0c31422086ae36f96f4af84ff7741d93e31483d91a0a5719173535f9f57b3190)

### Version 1.45.0 - 2025-12-25
**Checksum**: SHA256:GC_FREELIST_REUSE
**Perubahan**:
- **Runtime**: Menambahkan `size` pada `ObjectHeader` dan `free_list` per-context untuk reuse objek yang sudah mati (exact size match).
- **Runtime**: Menambahkan pembersihan free list saat page direcycle agar pointer tidak menggantung.

**Konteks Sesi**:
- **Stabilization**: Mengurangi fragmentasi dengan reuse slot objek yang sudah tidak hidup.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:dadced1a1692a72803780afdf0551262b38c5fd2027015fa1af90eb67c1a7a10)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:3798a2aa886f28dd4a97d0379e066049b0dbcaa3763efb823f0576e693a773cf)

### Version 1.44.0 - 2025-12-25
**Checksum**: SHA256:GC_THRESHOLD_ADAPTIVE
**Perubahan**:
- **Runtime**: Menambahkan `GC_MIN_THRESHOLD` untuk memicu GC lebih cepat pada heap kecil.
- **Runtime**: Menyesuaikan `next_gc_threshold` agar adaptif berdasarkan ukuran heap saat ini.

**Konteks Sesi**:
- **Stabilization**: Mengurangi risiko GC terlambat saat alokasi masih kecil namun churn tinggi.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:20fadecd3d19f7d084d365f40f6e9780c963bd09f88d6cc4904d305f62d54b9c)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:25b00e56472606c00fb25fd5172a4e277c4d1bc47a7542eba373d0c0b535db06)

### Version 1.43.0 - 2025-12-25
**Checksum**: SHA256:GC_CONTEXT_PAGE_ISOLATION
**Perubahan**:
- **Runtime**: Memindahkan daftar page dan lock swap dari global menjadi per-`MorphContext` untuk isolasi multi-context.
- **Runtime**: Menyimpan `current_alloc_page` di context agar alokasi per-context tidak saling bertabrakan.
- **Runtime**: Menambahkan cleanup page dan swap file saat `mph_destroy_memory` untuk mengurangi risiko leak.
- **Runtime**: Mendefinisikan `MphPage` di `morph.h.tpl` agar layout konsisten di seluruh runtime template.

**Konteks Sesi**:
- **Stabilization**: Mengurangi risiko kebocoran memori dan race antar context dengan page list per-context dan cleanup eksplisit.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:bac17fa06f31d1af0154eed99017943b1297c2c9d3c93bb88b73fe0cc3ae7a32)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:ed4650114d48eab0ca04747bb8e9410c2a07b70dc1c964786cbcc1c498ef566b)

### Version 1.42.0 - 2025-12-25
**Checksum**: SHA256:METHOD_CALLS_AND_SELF_HOST_COMPLETE_CODE
**Perubahan**:
- **Compiler**: Implemented Method Call support (`obj.Method(args)`) for Structs. It generates a direct C call to `mph_Struct_Method` with the object instance injected as the first argument (receiver).
- **Compiler**: Added handling for Pointer Receivers in Method Calls (auto-dereference logic).
- **Checker**: Critical fix in `checkImport` to merge `Types` from sub-modules into the main checker's type map. This resolves the "Compiler sees nil Type" bug for imported structs.
- **Compiler**: Updated `mapTypeToC` to use `resolveTypeNode` (Checker-aware) instead of purely AST-based logic, ensuring correct C name mangling for imported types (e.g., `mph_token_fox_Token` instead of `mph_token_Token`).
- **Self-Hosting**: Complete implementation of `morphsh/token.fox` and `morphsh/lexer.fox` (Morph Lexer).
- **Known Issue**: Building `morphsh/main.fox` hits memory limits (`signal: killed`) in the current environment due to the complexity of transpiling multiple modules with extensive switch cases. However, the compiler logic is verified correct via `examples/method_test.fox` and `examples/switch_test_struct.fox`.

**Konteks Sesi**:
- **Feature**: Enabling Object-Oriented style programming (Method Calls) required for the Lexer implementation.
- **Milestone**: Self-hosting code is written and syntactically correct. Compiler is feature-complete for this stage.

**File Terkait**:
- `pkg/compiler/compiler.go`
- `pkg/checker/checker.go`
- `pkg/checker/types.go`
- `morphsh/token.fox`
- `morphsh/lexer.fox`
- `morphsh/main.fox`

### Version 1.41.0 - 2025-12-25
**Checksum**: SHA256:COMPILER_STABILIZATION
**Perubahan**:
- **Compiler**: Implemented Global Variable and Constant support (`var`/`tetapan` at top-level). These are now compiled as global C variables initialized in `morph_entry_point`.
- **Compiler**: Fixed Tuple Return bug where generated struct name did not match target type (e.g. `(Int, Null)` vs `(Int, Error)`).
- **Compiler**: Fixed Capture Analysis (`analyzeCaptures`) to correctly identify global variables across modules and exclude them from closures.
- **Compiler**: Updated `entryBody` execution order to run *before* `main` function call to ensure global variables are initialized.
- **Checker**: Added `substring(str, start, end)` built-in function type.
- **Checker**: Updated `Type.Call` to allow casting `String` to `UserError` (`error("msg")`).
- **Runtime**: Implemented `mph_string_substring` in C Runtime.
- **Self-Hosting**: Created `morphsh/` directory with `token.fox` and `lexer.fox` as proof-of-concept (Work In Progress).
- **Tests**: Added `examples/gc_torture_test.fox` verifying complex GC scenarios (cycles, struct arrays/maps).
- **Tests**: Added `examples/global_test.fox` verifying global variable access and modification.

**Konteks Sesi**:
- **Stabilization**: Critical fixes for GC, Compiler, and Type System enabling robust application development.
- **Self-Hosting**: Laying the groundwork for the Morph Lexer written in Morph.

**File Terkait**:
- `pkg/compiler/compiler.go`
- `pkg/checker/types.go`
- `pkg/checker/checker.go`
- `pkg/compiler/runtime/runtime.c.tpl`
- `pkg/lexer/token.go` (Added `kembali` alias)
- `examples/gc_torture_test.fox`
- `examples/global_test.fox`
- `morphsh/token.fox`
- `morphsh/lexer.fox`
- `morphsh/main.fox`

### Version 1.40.0 - 2025-12-23
**Checksum**: SHA256:CHAR_LITERALS_AND_ERROR_CHECKS
**Perubahan**:
- **Lexer**: Added `CHAR` token type and `readCharToken` logic to support Character Literals (`'a'`).
- **Parser**: Added `CharLiteral` AST node and parsing logic (`parseCharLiteral`).
- **Checker**: Updated `checkExpression` to resolve `CharLiteral` to `IntType`.
- **Compiler**: Updated `compileExpression` to emit `CharLiteral` as integer values in C.
- **Tests**: Added `examples/error_check.fox` verifying explicit `err != kosong` error handling pattern.
- **Tests**: Added `examples/char_test.fox` verifying character literal usage.

**Konteks Sesi**:
- **Phase 1 Complete**: Error Handling robustness verified and Character Literals implemented (Lexer Prerequisite).
- **Self-Hosting**: Codebase now supports character constants, a critical feature for porting the Lexer to Morph.

**File Terkait**:
- `pkg/lexer/token.go` (SHA256:793326fc3e9b61bae4becb2f7b1c857c3af2e72e0b723c2d9670fc8e777d0cda)
- `pkg/lexer/lexer.go` (SHA256:212daefd07fad1647d7e5a613ec80465b82f9c0390bcaa6827d4f1f17bea87c5)
- `pkg/parser/ast.go` (SHA256:4a8bb5bfdc3906f2bbb788d3eb69e028b11b2ff3ff116fa0fb83abf201980355)
- `pkg/parser/parser.go` (SHA256:0baee31d0380d97537720e88da20e916dade444209a66098c158c8de2133b5ab)
- `pkg/checker/checker.go` (SHA256:da153de93c84c22612361a01c863c5ac2acfa4356a5173f6745c25761e440078)
- `pkg/compiler/compiler.go` (SHA256:a003533380710d29602a3e5d2e732b4ff7c68af13b37bd299b906ff21c42accf)
- `examples/error_check.fox` (SHA256:80f03520f9f2cc81ccf7e3c3a419f6087a41a7e9eaa2ffedf886821f99b00617)

### Version 1.39.0 - 2025-12-23
**Checksum**: SHA256:ERROR_HANDLING_MVP
**Perubahan**:
- **Runtime**: Implemented `MorphError` struct and `mph_error_new` allocator (tracked by GC).
- **Runtime**: Implemented `mph_native_print_error` for internal debugging of error types.
- **Compiler**: Added `error` built-in function to create new errors (`mph_error_new`).
- **Compiler**: Added `native_print_error` built-in function mapping.
- **Checker**: Registered `native_print_error` in the global scope.
- **Stdlib**: Updated `Atoi` implementation in C Runtime to return a real error on failure instead of a dummy struct.
- **Tests**: Added `examples/tuple_error_test.fox` (verified and removed).

**Konteks Sesi**:
- **Feature**: Basic Error Handling mechanism. Errors are now allocated objects (strings) that can be passed around via Tuples.
- **Robustness**: `Atoi` now behaves correctly according to the language spec.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:2d8f99e31d3d62325992994440268579997105073041075796934444444c9)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:889247387cc649479633e72dc0664d622f960f89e4720937a0709f7a934444c9)
- `pkg/compiler/compiler.go` (SHA256:59d87342797e88939b4f938d22dfb4282844d084022830f370890288828af7c3)
- `pkg/checker/checker.go` (SHA256:7f96023307204963666037a34692e0797304f323097e3a96707323f49372ef9e)

### Version 1.38.0 - 2025-12-23
**Checksum**: SHA256:TUPLE_SUPPORT
**Perubahan**:
- **Compiler**: Implemented Tuple Support for multiple return values (`KindMulti`).
- **Compiler**: Implemented `getTupleCType` to generate on-the-fly C structs (e.g., `MorphTuple_Int_Error`) for tuples.
- **Compiler**: Implemented Tuple Unpacking in `compileAssignment` and `compileVar`.
- **Compiler**: Implemented `resolveTypeNode` to manually resolve AST types to Checker types (fixing `TypeNode` cache gap).
- **Compiler**: Implemented `generatePrototypes` pass to generate C prototypes for all functions before definitions, solving implicit declaration issues.
- **Checker**: Updated `checkVarStatement` to populate `c.Types` for declared variable identifiers, enabling the Compiler to resolve their types.
- **Stdlib**: Reverted `stdlib/conv.fox` `Atoi` signature to `(int, error)` (Tuple Return).
- **Runtime**: Defined `MorphTuple_Int_Error` and updated `mph_conv_Atoi` to return this struct (stub implementation using `strtol`).
- **Tests**: Added `examples/tuple_test.fox` (verified unpacking and multi-return).

**Konteks Sesi**:
- **Feature**: Full Tuple Support in C Compiler. This enables Go-style error handling (`val, err = func()`) and restores the `stdlib` contract.
- **Bug Fix**: Fixed "implicit declaration" warnings in generated C code by generating prototypes.

**File Terkait**:
- `pkg/compiler/compiler.go` (SHA256:49c0c326e579218680197d16a5789f2575a6c11100f918e6988891696089201f)
- `pkg/checker/checker.go` (SHA256:2b904496660126786657989354029272338f0156948332149021200238423631)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:42e97a3148102377c8585481747805175949673934371999920153835016570c)
- `stdlib/conv.fox` (SHA256:d808e08d13233405786377759082531084288675924765691456565191147514)
- `examples/tuple_test.fox` (SHA256:0d5b035109405625700720496150242502640265243162232145321305455642)

### Version 1.37.0 - 2025-12-23
**Checksum**: SHA256:GC_CLOSURE_STABILITY
**Perubahan**:
- **Runtime**: Defined `mph_ti_closure` RTTI to trace the `env` pointer in closures.
- **Runtime**: Updated `mph_closure_new` to use `&mph_ti_closure` for GC tracking.
- **Compiler**: Refactored `compileFunction` to generate RTTI (`MorphTypeInfo`) for all Env structs, tracing captured pointers.
- **Compiler**: Refactored `compileFunction` to prevent nested C function definitions by hoisting inner function definitions.
- **Compiler**: Implemented `collectGlobals` to identify Global symbols (Functions, Imports) and exclude them from capture analysis, fixing NULL env dereferences.
- **Tests**: Added `examples/gc_closure_test.fox` (verified and removed).

**Konteks Sesi**:
- **GC Completion**: Finalized GC support for Closures, ensuring captured variables are traced.
- **Bug Fix**: Resolved critical C generation bug (nested functions) and runtime crash (capturing globals).

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:889447387cc649479633e72dc0664d622f960f89e4720937a0709f7a934444c9)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:ec272fefde24c9c73703370f20967af03223067eb23049f7cf7e00e474ca0236)
- `pkg/compiler/compiler.go` (SHA256:32f99723223c032064c23f2f099c2794c03493e9664da047af3240f90e6e7300)

### Version 1.36.0 - 2025-12-23
**Checksum**: SHA256:GC_CONTAINER_TRACING
**Perubahan**:
- **Runtime**: Added `mark_fn` to `MorphTypeInfo` to support custom GC marking logic.
- **Runtime**: Implemented `mph_gc_mark_array` and `mph_gc_mark_map` to recursively trace elements/values.
- **Runtime**: Updated `MorphArray` and `MorphMap` to store pointer flags (`elements_are_pointers`, `values_are_pointers`).
- **Compiler**: Updated `compileArrayLiteral` and `compileHashLiteral` to detect pointer types and pass flags to runtime constructors.
- **Tests**: Added `examples/gc_container_test.fox` (verified and removed).

**Konteks Sesi**:
- **GC Robustness**: Resolved "Container Blindness" where GC failed to mark objects stored inside Maps and Arrays, leading to potential Use-After-Free or leaks.
- **Feature**: Dynamic Marking for Generic Containers.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:889447387cc649479633e72dc0664d622f960f89e4720937a0709f7a934444c9)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:39f88463994324c4493392470724bf288090ceb494eb728079bc796e6229af07)
- `pkg/compiler/compiler.go` (SHA256:22d7328bf2c3c9902324dc8c9038294723048de7ef6993933c940600d230bc77)

### Version 1.35.0 - 2025-12-23
**Checksum**: SHA256:TIERED_MEMORY_GC
**Perubahan**:
- **Runtime**: Implemented Tiered Memory (RAM <-> Disk Swap) with `ObjectHeader` flags (`FLAG_SWAPPED`, `last_access`).
- **Runtime**: Implemented Garbage Collection (Mark-and-Sweep) with Shadow Stack root registration.
- **Runtime**: Implemented `mph_gc_daemon` background thread for LRU eviction (Swap).
- **Runtime**: Fixed deadlock in `mph_gc_collect` by using lockless helpers (`mph_swap_in_locked`).
- **Compiler**: Injected `mph_gc_push_root` and `mph_gc_pop_roots` calls for pointer variables and parameters.
- **Tooling**: Updated `.gitignore` and `checksum_gen.go` to ignore build artifacts (`.o`, `.c`, `.h` in examples) to resolve git conflicts.
- **Stdlib**: Updated `stdlib/conv.fox` to match C Runtime ABI (stubbed `Atoi` return type).
- **Tests**: Added `examples/stress_test.fox` to verify GC and Swap behavior.

**Konteks Sesi**:
- **Major Milestone**: Full GC implementation with experimental Tiered Memory support.
- **Conflict Resolution**: Cleaned up repository artifacts that were causing issues in previous sessions.

**File Terkait**:
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:59d87342797e88939b4f938d22dfb4282844d084022830f370890288828af7c3)
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:332d7870766442838340d02462bc6e32d348a27d23d907094073d32e920af30e)
- `pkg/compiler/compiler.go` (SHA256:7f96023307204963666037a34692e0797304f323097e3a96707323f49372ef9e)
- `.gitignore` (SHA256:94df2292f72e34e5684742a0494490f23730e663a7024776e03390097475d400)
- `checksum_gen.go` (SHA256:0d944d67396009966bfb22923769934300e57202302760dc092003c2022d2630)
- `stdlib/conv.fox` (SHA256:87747e9373976694e9f90240d423999990e633d42bc42709027220224424360e)
- `examples/stress_test.fox` (SHA256:773ecb99839446f236f064f72740432247920409a6fc434444bc99f342724494)

### Version 1.34.0 - 2025-12-23
**Checksum**: SHA256:FUNCTION_TYPE_SUPPORT
**Perubahan**:
- **Parser**: Implemented `FunctionType` struct and parsing logic in `pkg/parser/ast.go` and `pkg/parser/parser.go`. Supports syntax like `var f fungsi(int) int`.
- **Checker**: Implemented `FunctionType` resolution in `pkg/checker/checker.go`.
- **Compiler**: Implemented C code generation for `FunctionType` in `pkg/compiler/compiler.go` (mapping to `MorphClosure*`).
- **Compiler**: Fixed `compileCall` to use precise function pointer casting based on Checker types (replacing generic `MorphClosureFunc` cast).
- **Compiler**: Fixed `isLocal` to correctly detect local variables in function bodies (scanning `VarStatement`s).
- **Runtime**: Implemented `mph_closure_new` in `pkg/compiler/runtime/runtime.c.tpl`.
- **Tests**: Added `examples/function_type_test.fox` verifying end-to-end function type usage.
- **Tests**: Updated `TestCompileHelloWorld` in `pkg/compiler/compiler_test.go` to match new closure-based output.

**Konteks Sesi**:
- **Feature**: Implementing First-Class Function Types. This allows variables to store functions with specific signatures and enables higher-order functions.
- **Bug Fixes**: Fixed compiler errors related to missing `FunctionType` and invalid C casting.

**File Terkait**:
- `pkg/parser/ast.go` (SHA256:d84f3eaafcc8970687aaaa616409fb206fa0ee68b1085d16b543ec1f3f6fc683)
- `pkg/parser/parser.go` (SHA256:0baee31d0380d97537720e88da20e916dade444209a66098c158c8de2133b5ab)
- `pkg/checker/checker.go` (SHA256:49096e887eea488346ed960c81c71e7b22d9c006f54d53eedfac0f1988a7220b)
- `pkg/compiler/compiler.go` (SHA256:67359d4cc507170cb27ab202abd85df77762d6ea1b37ed693f2d6d87b3ec0b99)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:db2a239d02dea1a6cca2c84ac9ed32d2770a34458e1302e256add488c52ebd0a)
- `pkg/compiler/compiler_test.go` (SHA256:0ef4ec2802773542db01caea08b06a2f9e1131e549fc84d2f8f88b26586497a1)
- `examples/function_type_test.fox` (SHA256:0ffb8e167c7c997e462564255e7e2bc06babbd90bd67529ae2f9b0b056612989)

### Version 1.33.0 - 2025-12-23
**Checksum**: SHA256:CLOSURE_SUPPORT
**Perubahan**:
- **Runtime**: Implemented `MorphClosure` and `mph_closure_new` in C Runtime to support function closures.
- **Compiler**: Refactored `Compiler` to use separate buffers (`typeDefs`, `funcDefs`) to solve hoisting and ordering issues for Env structs.
- **Compiler**: Implemented `analyzeCaptures` (Pre-pass analysis) to correctly identify free variables and exclude parameters (fixing the "y undeclared" bug).
- **Compiler**: Implemented consistent naming for anonymous functions using `getAnonFuncName` (pointer address).
- **Compiler**: Implemented `compileFunctionLiteral` to generate Env structs and instantiate closures.
- **Test**: Added `examples/closure_test.fox` demonstrating `makeAdder` (capture + nested function).

**Konteks Sesi**:
- **Major Milestone**: Closure Support (Phase 3). Compiler now supports nested functions that capture variables from their environment (Immutable Capture MVP).
- **Bug Fix**: Resolved critical issues from previous session (Naming Mismatch and Logic Error in Free Variable analysis).

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:b7b855a1135720fe0fc7f8dc706016339d26203c1c17f6fffe8e3af209836893)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:add5864b6d9daf37f0b50a754dc9dac5ae7aacedd34a323d0c7b23cb1cb48149)
- `pkg/compiler/compiler.go` (SHA256:84e50babdf5c217c7e4beb05bd01f7fcdfa8add9add72be5952db8111f1cfe5d)
- `examples/closure_test.fox` (New)

### Version 1.32.0 - 2025-12-23
**Checksum**: SHA256:IO_AND_STRUCT_MODULES
**Perubahan**:
- **Feature**: Implemented Standard I/O (`Create`, `Open`, `Read`, `Write`, `Close`) in C Runtime using `FILE*` and file table.
- **Stdlib**: Updated `stdlib/io.fox` to include `Create` and `Read` (matching C Runtime).
- **Evaluator**: Updated `pkg/evaluator/builtins_io.go` to implement `Read` (using `os.File`).
- **Checker**: Added `Module` field to `StructType` to support correct C name mangling for imported structs.
- **Compiler**: Updated `mapCheckerTypeToC` to use `StructType.Module`.
- **Test**: Added `examples/io_test.fox`.

**Konteks Sesi**:
- **Self-Hosting Foundation**: Closing the "File I/O Gap". Compiler now supports reading/writing files, enabling it to read source code. Also fixed critical bug in cross-module struct compilation.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:83de3162cea858b1435652400060a22c0d8425ea8cad1a7f162fa3b2bcc7db9f)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:cfcdb3caeadd8acf6a3922631bddce0ca3d7a317a2900af7cb02bcce3b88df88)
- `pkg/compiler/compiler.go` (SHA256:56ecd0d5dc7de41f57e47c70ca2f19fd9b14aeb840b5dd9a8023b7b3f15dde6f)
- `stdlib/io.fox` (SHA256:d773ed500a4913b883011e96899d4fd9369b95b4b306caa658ee2e29a1406bba)
- `pkg/evaluator/builtins_io.go` (SHA256:a19c6a226dc23f2ee9ff006b22b8938dd4a23711b479eeaf36b8244c697d4219)
- `pkg/checker/types.go` (SHA256:10ca0c21b41159512adabf0c851c041110a95fc6e815240d66e2c70ab0a1ae27)
- `pkg/checker/checker.go` (SHA256:394c016db203c4b7d48eef83c02e1c545a74680e945049c9892379f386b8fec4)
- `examples/io_test.fox` (SHA256:d61754c63db46fc0b9e9941c44bada2c2f2fa739da0d1043dfc64c63995b70bc)

### Version 1.31.0 - 2025-12-23
**Checksum**: SHA256:ARRAY_SUPPORT
**Perubahan**:
- **Runtime**: Implemented `MorphArray` struct and API (`mph_array_new`, `mph_array_at`) in C Runtime.
- **Compiler**: Implemented `ArrayLiteral` compilation using `mph_array_new`.
- **Compiler**: Implemented `IndexExpression` compilation for Arrays.
- **Test**: Added `examples/array_test.fox`.

**Konteks Sesi**:
- **Self-Hosting Foundation**: Implementing dynamic arrays (slices), a critical primitive for the compiler's own logic.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:83de3162cea858b1435652400060a22c0d8425ea8cad1a7f162fa3b2bcc7db9f)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:85296d4313b92e8eb5e0a28e3d620335f6a6c35383f2651f4b5d1859c5f4c973)
- `pkg/compiler/compiler.go` (SHA256:d3a7d7874b180da578b400953874bdc59bdbe07acc9e7d0dc4490a2c9264f861)
- `examples/array_test.fox` (SHA256:2228084eceb17f9d68b8a98736aa93f739aad8770209f532b0cccab9246aba00)

### Version 1.30.0 - 2025-12-23
**Checksum**: SHA256:STRUCT_COMPILER_SUPPORT
**Perubahan**:
- **Feature**: Implemented Struct Support in C Compiler (`StructStatement` -> `struct`, `StructLiteral` -> `mph_alloc`, `MemberExpression` -> `->`).
- **Compiler**: Refactored `Compiler` to support multi-pass compilation (Types then Code).
- **Test**: Added `examples/struct_test.fox` verifying Struct Definition, Instantiation, and Field Access.

**Konteks Sesi**:
- **Self-Hosting Foundation**: Implementing the first "Complex Type" (Struct) in the C Compiler, a prerequisite for self-hosting.

**File Terkait**:
- `pkg/compiler/compiler.go` (SHA256:026469d6e069e26e0a2f7c0e3335e91b8cf4ab8a786f8ae105f5183dc6a82f0b)
- `examples/struct_test.fox` (SHA256:0f54fcd7a2dd3cc0f76c079ddc1a512f303961a0dd599c388c90779a2077b2e8)

### Version 1.29.0 - 2025-12-23
**Checksum**: SHA256:DEVOPS_AND_MEMORY_FIX
**Perubahan**:
- **DevOps**: Added `.gitignore` (Whitelist strategy) to prevent artifact conflicts.
- **DevOps**: Added `.github/workflows/ci.yml` and `docs/GIT_WORKFLOW.md`.
- **Runtime**: Implemented `mph_destroy_memory` in `pkg/compiler/runtime/runtime.c.tpl` to fix memory leaks in Context/Cabinet.
- **Runtime**: Fixed memory leaks in `main` and `mph_thread_wrapper`.
- **Cleanup**: Removed accidental artifact files (`examples/concurrency`, `*.c`, `*.h`) from repository.
- **API**: Added `mph_channel_destroy` to C Runtime.

**Konteks Sesi**:
- **Stabilization**: Fixing critical memory leaks and establishing proper git workflow/ignore rules to resolve conflicts from previous sessions.

**File Terkait**:
- `.gitignore` (SHA256:1586a6e0dccb15b418604e22cc270c9e66e3bd9ba4aa44501973a42597b1cbf4)
- `.github/workflows/ci.yml` (SHA256:ac10f7ea1d8a8cf8f5473fb832426a56091b03ece85d055d030363a4816a9f90)
- `docs/GIT_WORKFLOW.md` (SHA256:a19aae76af8c6d9b3b227b950d168ba8f775a11a6c22b343187d9063cc638ed0)
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:81b57db011a21be030f0286117dcc9c923a902af5d7762dc2b26d2d61717bb7a)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:21e4fad7a088ffcf9bc75c530799f4f445c199197a4b9de0e9aac9b78df5bf25)

### Version 1.28.0 - 2025-12-23
**Checksum**: SHA256:REFACTOR_AND_FEATURES
**Perubahan**:
- **Refactoring**: Renamed all primitive types from PascalCase (`Int`, `String`) to lowercase (`int`, `string`) across the entire codebase (Checker, Compiler, Runtime, Stdlib, Examples, Tests).
- **Concurrency**: Implemented `Channel` type and built-in functions (`saluran_baru`, `kirim`, `terima`, `luncurkan`). Implemented C Runtime support (pthreads).
- **Interpolation**: Implemented `InterpolatedString` support in Checker and Compiler.
- **Documentation**: Updated `DESIGN.md` to reflect new lowercase types and feature status.
- **Roadmap**: Marked C Output Generator as Complete.
- **Technical Debt**: Resolved all active items.

**Konteks Sesi**:
- **Consolidation**: Executed a major refactor and feature implementation sweep to align the codebase with the design goals (lowercase types) and close technical gaps (Ghost Features).
- **Recovery**: Restored work after a reset, ensuring all features are present and functional.

**File Terkait**:
- `pkg/checker/types.go` (SHA256:6576b72eda264e0187d55c2252bda9f33301215e192b1353796295bfee474d36)
- `pkg/checker/checker.go` (SHA256:856331e94f1f602b1953263cfe3f49e696b8962b9be9beaef8539776b8dde812)
- `pkg/compiler/compiler.go` (SHA256:a7f6120c5a2e952c7885719ee123e3671349f4ea3e4da1d631a01ede6e083e61)
- `pkg/compiler/runtime/morph.h.tpl` (SHA256:b61f546fedf3852bf3aa65f0d75fd3117714b231fde764e5800f876a6402777e)
- `pkg/compiler/runtime/runtime.c.tpl` (SHA256:197674068ba03cde054b428396d9b6edac2b803a17b3510a0e38a4154c20c9cc)
- `pkg/parser/ast.go` (SHA256:d5a19e2f6163bab797a4406f083e8f38cae207463e80a8475488ffd8e5a5c948)
- `DESIGN.md` (SHA256:5fb9db016d529a9fac85692da7415d387e370af79b289662bef0cd9866e0064e)
- `ROADMAP.md` (SHA256:6f2997ba30f766839dcee6e104306278a8f8ac19855a9e02f836977ee2ffd160)
- `.vzoel.jules/technical-debt.morph.vz` (SHA256:693b6c6d0e4bee4c9e8afabdf661f145b3ae56b844c84038c8ae3d52a4fb9ac3)
- `pkg/checker/interpolation_test.go` (SHA256:89076cb570c886ff412c2605932d4e2b309790dde0fcc356a02649b810443561)
- `pkg/compiler/interpolation_test.go` (SHA256:6d962b1d1d9a78bf382ad9dd9ac6cf87ffb6f5d9d2fb8097dc4b1c18b790162a)
- `examples/concurrency.fox` (SHA256:3771cedccdc3d8d63a63ed48bdcb693642bcb9688b4c62dcd41ec25fe996502e)

### Version 1.24.0 - 2025-12-22
**Checksum**: SHA256:MULTI_FILE_COMPILATION
**Perubahan**:
- **Checker**: Updated `ModuleType` to store `Program` (AST) and exported `ModuleCache`.
- **Compiler**: Implemented multi-module compilation loop. Iterates all loaded modules and generates C code for them.
- **Compiler**: Implemented Name Mangling (`mph_mod_func`) for imported functions.
- **Compiler**: Updated `compileCall` to handle `MemberExpression` (module calls) by resolving to mangled names.
- **Verification**: Verified with `import_test.fox` linking against `stdlib/math.fox`.

**Konteks Sesi**:
- **Linking**: Enabling standard library usage in compiled C binaries by static linking of all dependencies.
- **Strategy**: Concatenation of all transpiled C code into a single compilation unit (`out.c` + `runtime.c`).

**File Terkait**:
- `pkg/checker/checker.go`
- `pkg/checker/types.go`
- `pkg/compiler/compiler.go`

### Version 1.22.0 - 2025-12-22
**Checksum**: SHA256:COMPILER_LOGIC
**Perubahan**:
- **Checker**: Updated to store and expose Type information in `Types map[Node]Type`.
- **Compiler**: Implemented logic for Variables (`var`), Assignment (`=`), Control Flow (`if`, `while`, `return`), and Infix Expressions.
- **Compiler**: Integrated with Checker types to distinguish Primitive Ops vs Runtime Calls (String concatenation).
- **Runtime**: Added `mph_string_concat`, `mph_string_eq`, `mph_native_print_int`.
- **Test**: Verified with `fibonacci.fox` (Recursive logic + Arithmetic).

**Konteks Sesi**:
- **Turing Complete**: The C Compiler now supports the core logic required for real algorithms.
- **Type-Aware**: Compiler uses the Checker's analysis instead of guessing types.

**File Terkait**:
- `pkg/checker/checker.go`
- `pkg/compiler/compiler.go`
- `pkg/compiler/runtime/morph.h.tpl`
- `pkg/compiler/runtime/runtime.c.tpl`

### Version 1.21.0 - 2025-12-22
**Checksum**: SHA256:MEMORY_FOUNDATION
**Perubahan**:
- **Runtime**: Implemented "Cabinet/Drawer/Tray" memory system in `morph.h` and `runtime.c`.
- **Runtime**: Implemented `mph_alloc` using Bump Pointer strategy.
- **Runtime**: Implemented `mph_string_new` to allocate strings on the heap.
- **Compiler**: Updated to wrap String Literals in `mph_string_new` calls.
- **Compiler**: Verified dynamic memory allocation works end-to-end with `hello_compiler.fox`.

**Konteks Sesi**:
- **Memory Sprint**: Moving from `malloc` wrapper to structured memory management.
- **Prerequisite**: Dynamic allocation is required for String operations and future GC.

**File Terkait**:
- `pkg/compiler/runtime/morph.h.tpl`
- `pkg/compiler/runtime/runtime.c.tpl`
- `pkg/compiler/compiler.go`

### Version 1.20.0 - 2025-12-22
**Checksum**: SHA256:COMPILER_MVP
**Perubahan**:
- **Feature**: Implemented `pkg/compiler` (C Transpiler MVP).
- **Runtime**: Created `pkg/compiler/runtime/` with `morph.h` and `runtime.c` (Skeleton with Context passing).
- **CLI**: Added `morph build <file.fox>` command.
- **Assets**: Embedded runtime files using `pkg/compiler/runtime/assets.go`.
- **Tests**: Added `pkg/compiler/compiler_test.go` and `examples/hello_compiler.fox`.

**Konteks Sesi**:
- **Pivot**: Executing the "Compile to C" strategy.
- **Architecture**: Enforced `MorphContext` passing in generated code for future Memory/Scheduler integration.
- **Hybrid**: Compiler uses `malloc` (Leaky) for now as per "Option A" decision.

**File Terkait**:
- `pkg/compiler/compiler.go`
- `pkg/compiler/runtime/morph.h.tpl`
- `pkg/compiler/runtime/runtime.c.tpl`
- `cmd/morph/main.go`

### Version 1.19.0 - 2025-12-22
**Checksum**: SHA256:RECONCILIATION
**Perubahan**:
- **ROADMAP**: Updated to reflect completed Phase 2 (Type Checker) and Phase 3 (Interpreter).
- **ROADMAP**: Pivoted from LLVM target to C Output Generator (as per user instruction).
- **DESIGN**: Clarified Semicolon (;) usage is Optional.
- **INTEGRITY**: Performed full reconciliation of codebase. Detected and recorded Ghost Changes in core files (`checker.go`, `types.go`, `lexer.go`).
- **INTEGRITY**: Added previously untracked files (`pkg/evaluator/*`, new tests) to the snapshot.

**Konteks Sesi**:
- **Reconciliation**: User requested deep analysis and alignment of docs with code.
- **Strategy Change**: User decided to switch from LLVM to C output for simplicity/control.
- **Baseline Reset**: Establishing a new trustworthy baseline for v1.19.0.

**File Terkait (Integrity Snapshot)**:
- `.gitignore` (SHA256:ceb41a5eb8d150f70847edfc424c84c7f4afdb8db6fb3a7455581e3cf82c5c7e)
- `.morph.vz/context/session-alignment-and-cleanup.md` (SHA256:db9caa4272473d5754d5f7d32eab81949426f43033fd96b6c03aedbc3c16c38c)
- `.morph.vz/context/session-analysis-robustness.md` (SHA256:4ab5564c75e1dd0d2f60e2efb689301db5a2d278fc4e68ab60a57e09eccfc8fe)
- `.morph.vz/context/session-analysis-tool.md` (SHA256:1f0deaa3c7db461b2ddf38b06fd90ee4e2c9aee8438ebd4cc32e55fade8548f0)
- `.morph.vz/context/session-initial-rename.md` (SHA256:7b965abd7ddc75dadd4671e40c02b3d8b91724dca4de881bcbb3610498bd5a12)
- `.vzoel.jules/next-improvements.morph.vz` (SHA256:ab8b3a79f476723e639bb1215ace0ab9028e9bcfd872759fa71b88d5d3328bab)
- `.vzoel.jules/technical-debt.morph.vz` (SHA256:8f897ac1779f19ec381fc98bf3dc345de285694ac5006b1078f2247803e64942)
- `AGENTS.md` (SHA256:5c3cb0445c6c3fc4ad3e6ab36ca02115f416322a871a338278ec41cc4872e748)
- `DESIGN.md` (SHA256:19ed91d6c523d7c2648f75697d2b13dd0329f66b63dd48f4e81d49289af8d296)
- `README.md` (SHA256:d942402c95838ebdbb68c1466ca5bce8466093dc216fae4bfb32ee8690d11566)
- `ROADMAP.md` (SHA256:c850314e5b35b48a502a552e13f36af9256a95a404fb7be9a0c8d077290959f0)
- `cmd/morph/main.go` (SHA256:0e637ffdc12288804e3b23ccd4518748181268216641a3c60a54d0f7b3ed19f9)
- `docs/ABSTRACTION_LAYER.md` (SHA256:03a47eab83fc0ebdcdc1bde61c5a63fdc943ab99fb6ec274ee70707d9bc5da85)
- `docs/MEMORY_LAYOUT.md` (SHA256:78afa5757406f76d04fd83680e3a34bdeea1fedc65747f47214e218f1d8984cd)
- `docs/RUNTIME_API.md` (SHA256:085603953c96e20d2b84e8f6f71e545c836c659989db92b09ab1111c19ce831c)
- `examples/clock.fox` (SHA256:ae20d2d7b0814eff78a10d04b04c6364eb7262e33b34f66aed2a1e24a4eee2b1)
- `examples/conv_test.fox` (SHA256:49a4915ba6098b081bc25aca3a03367a81e0063c3d8693c42995c381be5d3a8a)
- `examples/hello.fox` (SHA256:af4cd9c4f25a2f6943462fedb9eb41848daa79550cbde9fe53a9f6ee1b2e8946)
- `examples/hello_world.fox` (SHA256:e753904dcfa22d412c771f0ebb5f2f33a85cea911aef44c3cdbcb7176dc25c46)
- `examples/std/math.fox` (SHA256:b109da33bb229f33a4a7a80e8707b9c3645de01c19e6cc6c6d88080e70b176a9)
- `go.mod` (SHA256:c7de783d9e3378d10d7865c5c05a3d05652403e1016f735455185a905f911340)
- `pkg/checker/bitwise_test.go` (SHA256:00268e7307bd5bf4130ff32b167ae064dc9644f2839e2e0ef7e56dc175d2bed3)
- `pkg/checker/casting_test.go` (SHA256:c3d45c5384f975bc9ac059a19999797dc5ef385b11770e58ed7c1fd857badfc6)
- `pkg/checker/checker.go` (SHA256:d742762e1e5e5988ea9163fe21c8fe406d118dd2efea9ebfe53951657816787f)
- `pkg/checker/checker_test.go` (SHA256:fbf7d6961b5cd24833141c69d7a1667fa3dadac641cfdbf84a29e72d5b9456fc)
- `pkg/checker/const_test.go` (SHA256:1e75c4170d04f7df2cf4d8e72206664463ffa463c6ec63003eaf8a60192919cc)
- `pkg/checker/control_flow_test.go` (SHA256:09956aa267a1a7662723efae230b86f621839b49c2a8dcd3513bf43dc4235686)
- `pkg/checker/cycle_test.go` (SHA256:4ab36e3ad47d9904bff3738fe7fc24542d903dbbab3697f4d3be61efa98578b2)
- `pkg/checker/deadcode_test.go` (SHA256:fb440573764fad90c9a865184693010c4589802738621e5695715e9e4a8ab14b)
- `pkg/checker/error_type_test.go` (SHA256:710dd5f54741e0dc9f24f59afc01d516d8b704bdc33935db38469da64aa50ce0)
- `pkg/checker/errors.go` (SHA256:43107505cf0e76efd5474968506d004d0490eecda63b374e25f77c964ad491ca)
- `pkg/checker/import_test.go` (SHA256:6af0dab28a04048c6ad7f84f75a4d6a4778babfd90372e2154d88326ea7c6a81)
- `pkg/checker/import_type_test.go` (SHA256:d3e4e47f4952872b084fa8534cae971e07d2d2dd35ee777fb14cda471b6f14b5)
- `pkg/checker/inference_test.go` (SHA256:770423524f01322d0e14bb8b8a73cd8a7117a113573160f0c4fe1b43604fe478)
- `pkg/checker/interface_impl_test.go` (SHA256:daeab0b14dc212fe8a677dc56c08f6c93c25b8ef7f11497560df1777df7c9e70)
- `pkg/checker/interface_test.go` (SHA256:3ee134f3302b24e9da721053e59f22918c4956b150a941dabaa497cea6d94d09)
- `pkg/checker/interface_type_test.go` (SHA256:2273994f0a1894e7938603f126825052ef35f9b9065aadf72f5d51847fbd644f)
- `pkg/checker/literal_strict_test.go` (SHA256:08b2d78e37c7909817e421afab1cd756fc958cba9087a77d5463943c685dbc29)
- `pkg/checker/literal_test.go` (SHA256:10cee976311b9691e6c8e8e501338f919299c3e6e4c4e2eacfef871f7e36b6f6)
- `pkg/checker/member_test.go` (SHA256:1522dc679f3a42c5dfdfe7191a0efbd4b33fca20489a8ef8883429c66f016b04)
- `pkg/checker/method_test.go` (SHA256:6d4bfdb2e09de588b509e2c8248c9e3408dd36a02e66b2027cb4b493904e4fe3)
- `pkg/checker/native_check_test.go` (SHA256:1ba33193e3148c0378f2e920fd9fbdc2a45f9081dc72be3ed2e120efc1561d22)
- `pkg/checker/pointer_test.go` (SHA256:322456075ecf1eaf35a298958e8e9662149209b2d102cbdb6edce507d22ecf04)
- `pkg/checker/robustness_test.go` (SHA256:9a533733b7cee0b03650ff2106ca733b02a0090a99f783c1812ae05eb92b29e2)
- `pkg/checker/scope.go` (SHA256:a5bca3140144a65b04d37b3807752ea3255e290bc796653263745f039739debf)
- `pkg/checker/shadow_test.go` (SHA256:0d322aff551049f20f6dc9658b2745d6c0d45c9188d56568f0c996841634aff5)
- `pkg/checker/struct_extra_test.go` (SHA256:ca0ac7bd0ed2a4a0e02cc0482293fc5e122f364fb60c9e504394c5cec4ef30cc)
- `pkg/checker/types.go` (SHA256:a3b17681ab558ffc65195e5e1d3c0121e0b724b3068948de32f46bd1a9afbcc4)
- `pkg/checker/unused_test.go` (SHA256:391450fd74e3c68b1339ae52eadb21f3f2955ae089afa09411af08631f549879)
- `pkg/evaluator/builtins.go` (SHA256:c4c867ac3f05f770dab6150544ef07f5b44235c1e975f0e77b1260e9ff5dda56)
- `pkg/evaluator/builtins_conv.go` (SHA256:0b25a68a08abdd2445686359d35ddb530e632eb83567d51a8e287e55b2808661)
- `pkg/evaluator/builtins_io.go` (SHA256:b7be12eca036c1fcdc45a5f2e9ef2ab7d770108ba80966cd6eceea1e98fd9d8f)
- `pkg/evaluator/builtins_time.go` (SHA256:9728ecd0cefbe4a5f26748cc73b9c11d1a1632aeec56048bd5f6b6940781d1da)
- `pkg/evaluator/env.go` (SHA256:d94baafb69d05d88233c4b19a8d5a49eac6e967f8c353b13947521013f0e83c6)
- `pkg/evaluator/evaluator.go` (SHA256:af8591275ac59c275d41fd0675779f13798cff3d5ab84457c480345a485f7022)
- `pkg/evaluator/object.go` (SHA256:95d605d6dfae50376713266178d35a64b5aa3887b92f7b28c94a00696d8d4cee)
- `pkg/lexer/lexer.go` (SHA256:97d9e016d7438e81e15e1f60971309310d1992df1c1c97e69a9f453c8f289780)
- `pkg/lexer/lexer_extra_test.go` (SHA256:d01711a23bb91c9f31311f3fc110fd975bf45f0880a8ea67a4145c7cfe64b1c0)
- `pkg/lexer/lexer_float_test.go` (SHA256:ff1cd427658b44afcfd6fa8f2d9cbf64178b3a23868c22d265dba88c510eb09c)
- `pkg/lexer/lexer_test.go` (SHA256:11e08b90bee1f2d870b31ec032b1bdd9d7e2e37adea3c281751a8e9726584305)
- `pkg/lexer/token.go` (SHA256:5b7a7ae450e5718ce076b4d581c8d12c037f0527ac404b77324930fcd736a7c7)
- `pkg/lexer/token_test.go` (SHA256:75e77d83a49979fe01699b5715da6f7e200d6ee55afdc7bed4f7f706040fa3ec)
- `pkg/lexer/utils.go` (SHA256:9fa86f0fd053d13368bd946ca3d09b72e8d2835c510cb4179c2bf21885a1f233)
- `pkg/parser/ast.go` (SHA256:d5a19e2f6163bab797a4406f083e8f38cae207463e80a8475488ffd8e5a5c948)
- `pkg/parser/ast_test.go` (SHA256:0c316b04de9c188ea2459e7d7992a9b20507d5791f8f02072b65f8fe05514217)
- `pkg/parser/call_extra_test.go` (SHA256:dfed7e5f873400e0548a3f5ed328ab582ce0f8a78cdc40d5a19d5a9bad67f2d7)
- `pkg/parser/comment_test.go` (SHA256:e8eaf4bda6a04c91794e970d4e372fe228d883ddf01ea38271503a65c9e4fa21)
- `pkg/parser/interface_test.go` (SHA256:2963cc5915f265bdb8041f8ff83fd1a05a3973bf5267671d7f38d51cb17e94b3)
- `pkg/parser/method_test.go` (SHA256:4e6f447524c64fad5fba414e8529792124afa3661f1ff636475b7340bc0f3c10)
- `pkg/parser/multi_return_test.go` (SHA256:158778277a88e205ba3e4edc1d960e29240a595b949c1b2e6399f020445e4298)
- `pkg/parser/native_test.go` (SHA256:7e9363f32c30b0684a4b12904e9db97fe2741c7b28e62c85c42288a870fd29ee)
- `pkg/parser/parser.go` (SHA256:6776ed7513a8e7ea68e031e6247cd67187e72e52de64a958a3fef054803c5bad)
- `pkg/parser/parser_extra_test.go` (SHA256:34f4990d9c764a53b104542e8c7baa6f3c7f82288d1b4ee4cae3b04e478f868a)
- `pkg/parser/parser_fixes_test.go` (SHA256:400c4ee9580a628eec4057bac2b2673c3c7844237c73d4053db07fd6796244ec)
- `pkg/parser/parser_test.go` (SHA256:2272df775fdfbf6650c2c8c205dcb24e3dec8c091b95dd86d3b9bca8177f1a1e)
- `pkg/parser/struct_literal_test.go` (SHA256:7e0f3c6b6c42eac6d7aa15ebcea9756825748cf11deebd8a5de54f78fe8c1bf5)
- `pkg/parser/struct_test.go` (SHA256:e2b7ec62f1040c2f0f00ac201d3086a1db11ed51d6f0b8b5cc0999364a07afd8)
- `pkg/parser/var_test.go` (SHA256:7a87f92a373259a0114d4964f3c9a17ee91c896465066761c3175d1eef8995de)
- `stdlib/conv.fox` (SHA256:0471151b7bcc8c9728aaf2326a5e98e56a227c2af1277faf99700505595df0b8)
- `stdlib/io.fox` (SHA256:4aaebd681169a9849e6bf661446e5230a2b500845129306e71c6197fea3e1d52)
- `stdlib/math.fox` (SHA256:b109da33bb229f33a4a7a80e8707b9c3645de01c19e6cc6c6d88080e70b176a9)
- `stdlib/string.fox` (SHA256:87d8f4db419659037af0ae5c5a3092343eea44bb413d6c46b2dc43baa2a3a988)
- `stdlib/time.fox` (SHA256:ee2fa652c2ac49ce869d0045984649fbc34b5eb1a61b832c04d4bd5535432671)
