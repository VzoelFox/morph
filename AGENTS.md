# Agents.md - Source of Truth untuk AI Agent

## Metadata Dokumen
- **Versi**: 1.38.0
- **Tanggal Dibuat**: 2025-12-20 06.10 WIB
- **Terakhir Diupdate**: 2025-12-23
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

### Version 1.18.0 - 2025-12-22
**Checksum**: SHA256:REVERSE_ENGINEER_DESIGN
**Perubahan**:
- **Documentation**: Updated `DESIGN.md` (Reverse Engineering) to accurately reflect current Codebase state (Syntax, Types, Stdlib).
- **Documentation**: Added Technical Debt entry regarding `Int`/`int` capitalization.
- **Spec**: Updated `DESIGN.md` syntax sections (Keywords, Entry Point, Types).
- **Spec**: Standardized `DESIGN.md` language to Indonesian.
- **Roadmap**: Moved Concurrency and Memory Management to Roadmap section.

**Konteks Sesi**:
- **Strategic Alignment**: Making documentation the source of truth for the *current* implementation, acknowledging gaps as technical debt.
- **Clarification**: User confirmed "Reverse Engineering" means updating Design to match Code.

**File Terkait**:
- `DESIGN.md` (SHA256:aec9003fa1e5e788515eca9ad0f0aa21679c9e3e0e59ae70820c0625719417f2)
- `.vzoel.jules/technical-debt.morph.vz` (SHA256:8f897ac1779f19ec381fc98bf3dc345de285694ac5006b1078f2247803e64942)

### Version 1.17.0 - 2025-12-20
**Checksum**: SHA256:STDLIB_CONV
**Perubahan**:
- **Evaluator**: Implemented complete expression evaluation (`Infix`, `Prefix`, `If`, `NullLiteral`).
- **Evaluator**: Added `Tuple` support for multiple return values and assignment unpacking.
- **Checker**: Updated `BinaryOp` to allow comparison between `Null` and nullable types (e.g. `Error`).
- **Stdlib**: Added `stdlib/conv.fox` with `Atoi` (returning `(Int, Error)`) and `Itoa`.
- **Runtime**: Implemented `pkg/evaluator/builtins_conv.go`.
- **Driver**: Updated `cmd/morph/main.go` to register `conv`.
- **Example**: Added `examples/conv_test.fox`.

**Konteks Sesi**:
- **Runtime Maturity**: Evaluator di-upgrade besar-besaran untuk mendukung operasi logika (`==`, `!=`, `+`) dan Tuple, memungkinkan kode yang lebih kompleks.
- **Feature**: `stdlib/conv` memungkinkan konversi tipe string-int yang robust dengan error handling idiomatic.

**File Terkait**:
- `pkg/evaluator/evaluator.go`
- `pkg/evaluator/object.go`
- `pkg/checker/types.go`
- `stdlib/conv.fox`
- `pkg/evaluator/builtins_conv.go`
- `cmd/morph/main.go`

### Version 1.16.0 - 2025-12-20
**Checksum**: SHA256:STDLIB_TIME
**Perubahan**:
- **Stdlib**: Added `stdlib/time.fox` with `Now` (Unix ms) and `Sleep` (ms).
- **Runtime**: Implemented `pkg/evaluator/builtins_time.go` bridging to Go `time` package.
- **Example**: Added `examples/clock.fox` to demonstrate `time.Sleep`.

**Konteks Sesi**:
- **Feature Expansion**: Menambahkan modul `time` sebagai langkah "realistis" berikutnya untuk interaktivitas dasar.
- **Strategy**: Implementasi `Sleep` yang blocking dipilih untuk stabilitas API awal, dengan TODO untuk upgrade ke async saat Scheduler tersedia.

**File Terkait**:
- `stdlib/time.fox`
- `pkg/evaluator/builtins_time.go`
- `cmd/morph/main.go`
- `examples/clock.fox`

### Version 1.15.0 - 2025-12-20
**Checksum**: SHA256:RUNTIME_IO
**Perubahan**:
- **Core**: Implemented `pkg/evaluator` (Tree-walking Interpreter).
- **Core**: Added `native` keyword support in Lexer, Parser, and AST.
- **Checker**: Updated to skip body checks for `native` functions.
- **Stdlib**: Created `stdlib/io.fox` with native `Open`, `Read`, `Write`, `Close`.
- **Runtime**: Implemented native IO bindings (`builtins_io.go`) wrapping Go's `os` package.
- **Driver**: Updated `cmd/morph/main.go` to run the Evaluator after checking.
- **Example**: Created `examples/hello_world.fox` demonstrating working "Hello World".

**Konteks Sesi**:
- **Major Milestone**: Memasuki Fase 3 (Runtime). Morph sekarang bisa mengeksekusi kode!
- **Feature**: User meminta "Hello World" yang nyata (baca/tulis/buka).
- **Resolution**: Implementasi Interpreter + Stdlib IO + Native Bridge.
- **Learnings**: Parser ambiguity (Identifier as Return Type vs Body start) resolved by explicit return type `Void` in example.

**File Terkait**:
- `cmd/morph/main.go`
- `pkg/evaluator/*`
- `stdlib/io.fox`
- `examples/hello_world.fox`
- `pkg/parser/parser.go`
- `pkg/checker/checker.go`

### Version 1.14.1 - 2025-12-20
**Checksum**: SHA256:STDLIB_INFRA
**Perubahan**:
- **Tooling**: Enhanced `cmd/morph/main.go` to support multiple search paths (`FileImporter.SearchPaths`).
- **Stdlib**: Created `stdlib/` directory as the central location for standard libraries.
- **Stdlib**: Added `stdlib/math.fox` (moved from examples) and `stdlib/string.fox`.
- **Example**: Updated `examples/hello.fox` to use direct imports (`ambil "math"`, `ambil "string"`).

**Konteks Sesi**:
- **Infrastructure**: Membangun fondasi untuk "Standard Library Import" agar library standar bisa diakses tanpa relative path, sesuai strategi "Import Load" yang disepakati.

**File Terkait**:
- `cmd/morph/main.go` (SHA256:6f9e4c4f1813412724ba75e2cfed1e634db97bbdd47efc438ed0c62b9fffab28)
- `stdlib/math.fox` (SHA256:b109da33bb229f33a4a7a80e8707b9c3645de01c19e6cc6c6d88080e70b176a9)
- `stdlib/string.fox` (SHA256:87d8f4db419659037af0ae5c5a3092343eea44bb413d6c46b2dc43baa2a3a988)
- `examples/hello.fox` (SHA256:c2a0f0ce680c71d0da04fa52e8b540f55d69a056640739be67282509dcb99f4f)

### Version 1.14.0 - 2025-12-20
**Checksum**: SHA256:CLI_DRIVER
**Perubahan**:
- **Tooling**: Created `cmd/morph/main.go` as the CLI Driver (Frontend Compiler).
- **Tooling**: Implemented `FileImporter` in `main.go` to support importing modules from the file system.
- **Example**: Created `examples/hello.fox` and `examples/std/math.fox` to demonstrate functionality.

**Konteks Sesi**:
- **Milestone**: Memasuki Fase 3 (awal) dengan menyediakan cara untuk menjalankan compiler (parsing & type checking) pada file `.fox` nyata.
- **Verification**: User meminta "menjalankan compiler yang sudah ada".

**File Terkait**:
- `cmd/morph/main.go` (SHA256:ccbd734cf102c9a5bfbbc27cf7bb55d09c5780b74dce907c4de4622fe0f2b363)

### Version 1.13.1 - 2025-12-20
**Checksum**: SHA256:BITWISE_TESTS
**Perubahan**:
- **Tests**: Added `pkg/checker/bitwise_test.go` to confirm existence and correctness of bitwise operations (`&`, `|`, `^`, `<<`, `>>`, `~`).

**Konteks Sesi**:
- **Verification**: User requested update on bitwise operations. Investigation showed implementation was complete but lacked dedicated tests. Added test suite to close the gap.

**File Terkait**:
- `pkg/checker/bitwise_test.go` (SHA256:00268e7307bd5bf4130ff32b167ae064dc9644f2839e2e0ef7e56dc175d2bed3)

### Version 1.13.0 - 2025-12-20
**Checksum**: SHA256:STRUCT_IMPROVEMENTS
**Perubahan**:
- **Type System**: Added `FieldOrder` to `StructType` to support ordered fields.
- **Type System**: Added `IsComparable` method to `Type` interface and implementations.
- **Checker**: Implemented `StructType.Call` to support Struct Constructor syntax (e.g., `User(1, "Name")`).
- **Checker**: Implemented strict Struct Equality logic in `StructType.BinaryOp` (structs with non-comparable fields like maps/arrays cannot be compared).
- **Tests**: Added `pkg/checker/struct_extra_test.go` to verify constructors and comparability.

**Konteks Sesi**:
- **Feature**: Memenuhi request user untuk mendukung syntax constructor struct yang lebih ringkas dan memastikan keamanan tipe saat membandingkan struct (deep equality safety).

**File Terkait**:
- `pkg/checker/types.go` (SHA256:a511933792bfe3cff4c4292650bc4b4bccc3332f047f37cb62695e3874f8f9cc)
- `pkg/checker/checker.go` (SHA256:60bfa7d8b70b908f865e007ae66314bc6f3eb51d1ec6c8f6a7dd12dd58993df8)
- `pkg/checker/struct_extra_test.go` (SHA256:29e0a9a3ed7ac5d1424a0ca682c442209005fb3160e664bd221e14f177670de5)

### Version 1.12.0 - 2025-12-20
**Checksum**: SHA256:STRING_INDEXING
**Perubahan**:
- **Type System**: Updated `BasicType.Index` to allow indexing `KindString` with `KindInt`, returning `KindInt` (representing byte/char value).
- **Checker**: Updated `checkAssignment` to strictly forbid assigning to string indices (String Immutability), ensuring `str[0] = x` is rejected.

**Konteks Sesi**:
- **Closing Gap**: Mengimplementasikan String Indexing (`s[i]`) yang merupakan fitur dasar, sekaligus menegakkan aturan immutability string pada level Checker.

**File Terkait**:
- `pkg/checker/checker.go` (SHA256:25a9f90997fb540fc39a688cc34d3723df6d25601f2d24d555af5300422d7197)
- `pkg/checker/types.go` (SHA256:7a5dae8380e06102519cb557bab2c63a0c63f3281edaaac15b59611784ecabf7)
### Version 1.11.0 - 2025-12-20
**Checksum**: SHA256:MODULO_SUPPORT
**Perubahan**:
- **Lexer**: Added `PERCENT` (%) token support.
- **Parser**: Added `MODULO` operator precedence (same as `PRODUCT`) and registered infix parser for `%`.
- **Checker**: Implemented `BinaryOp` support for `%` (Modulo) on `Int` types.

**Konteks Sesi**:
- **Closing Gap**: Mengimplementasikan operator Modulo (`%`) yang sebelumnya hilang dari Type System, sesuai dengan standar bahasa pemrograman umum dan implikasi aritmatika.

**File Terkait**:
- `pkg/checker/types.go` (SHA256:6179aad042f344d7682ef0bd3dcc4d0d18da157afb0c1abd2c507c5a537ece5f)
- `pkg/lexer/lexer.go` (SHA256:97d9e016d7438e81e15e1f60971309310d1992df1c1c97e69a9f453c8f289780)
- `pkg/lexer/token.go` (SHA256:3cd2d8157be7c04e3d2ded1f3d96b8c505532d8954882b599555d98350d07df0)
- `pkg/parser/parser.go` (SHA256:b9d9f683546fa2ead6f2eb267afb9dec176ee64de6d9c351bb4b00364f030e69)
### Version 1.10.0 - 2025-12-20
**Checksum**: SHA256:IMPORT_FIX
**Perubahan**:
- **Checker**: Fixed Import System to correctly distinguish between Exported Types (Structs/Interfaces) and Exported Values (Functions/Vars).
- **Types**: Added `ExportInfo` struct to `ModuleType.Exports` to track `IsType` metadata.
- **Parser**: Fixed critical panic in `ReturnStatement.String()` when expression is missing (nil pointer dereference).
- **Parser**: Updated `parseReturnStatement` to correctly handle empty returns (e.g. `kembalikan;`).

**Konteks Sesi**:
- **Bug Fix**: Menyelesaikan isu di mana exported variable salah dikenali sebagai type definition, dan sebaliknya.
- **Robustness**: Menangani edge case pada `ReturnStatement` yang menyebabkan crash pada compiler jika AST di-dump (debug).

**File Terkait**:
- `pkg/checker/checker.go` (SHA256:d316a9c683e2fe62892fbfc60a7047d4dc71df63e6d27279b3b93395981bd521)
- `pkg/checker/types.go` (SHA256:770accfdab1cd5bcd76b01c59489cc639f98567079faa05bd0410dbd6325f138)
- `pkg/parser/parser.go` (SHA256:8e23a7939ccb4bd9f112acd82675b046f07530f6fd0422aee13f318b554096ec)

### Version 1.9.0 - 2025-12-20
**Checksum**: SHA256:REMOVE_ANALYSIS
**Perubahan**:
- **Cleanup**: Removed `pkg/analysis` (Legacy Analyzer) to resolve architectural duplication.
- **Docs**: Updated technical debt to reflect removal.

**Konteks Sesi**:
- **Architectural Cleanup**: Removing the "Zombie" analyzer that conflicted with the robust Checker.

### Version 1.8.1 - 2025-12-20
**Checksum**: SHA256:REFACTOR_TYPE_SYSTEM
**Perubahan**:
- **Type System**: Expanded `Type` interface with `GetMember` and `Index` methods. Implemented behavior in `StructType`, `ModuleType`, `ArrayType`, `MapType`.
- **Checker**: Refactored `checker.go` to use `Type` interface methods instead of manual type casting for member access and indexing.
- **Robustness**: Improved error handling for indexing operations in Type System.

**Konteks Sesi**:
- **Refactoring**: Addressing "Anemic Domain Model" in Type System. Moving logic from Controller (Checker) to Model (Types).

**File Terkait**:
- `pkg/checker/types.go` (SHA256:f886047e21bd3c6bc8444ac85749eb1cc5dd927b9a4716a489a3afdb8893af2d)
- `pkg/checker/checker.go` (SHA256:83ca925d87949073eb64b6802c144b7104c29fa2b57c0f75ff457fb0a7f86958)

### Version 1.8.0 - 2025-12-20
**Checksum**: SHA256:BASELINE_RESET
**Perubahan**:
- **INTEGRITY RESET**: Melakukan "Baseline Validation" untuk seluruh file dalam project.
- **Audit**: Verifikasi manual Lexer & Parser (Robustness confirmed).
- **Cleanup**: Memastikan tidak ada sisa referensi `morphlang` dalam kode.
- **Snapshot**: Mencatat ulang checksum seluruh file core untuk menghapus keraguan "untracked source of truth".

**Konteks Sesi**:
- **Total Reconciliation**: Mengoreksi kesalahan sejarah penamaan repo dengan menetapkan baseline baru yang bersih di bawah `github.com/VzoelFox/morph`.
- Semua file di bawah ini telah diverifikasi eksistensi dan integritasnya.

**File Terkait (Integrity Snapshot)**:
- `go.mod` (SHA256:c7de783d9e3378d10d7865c5c05a3d05652403e1016f735455185a905f911340)
- `pkg/checker/casting_test.go` (SHA256:c3d45c5384f975bc9ac059a19999797dc5ef385b11770e58ed7c1fd857badfc6)
- `pkg/checker/checker.go` (SHA256:a838dd5e2b9b6bdfdebda2bce6c3124e25ed2ed98ba7730d8f7df85ffa5429fd)
- `pkg/checker/checker_test.go` (SHA256:78f07e699921a3c0fea532e8b34779d51e758fdfaeeabb1f999ec27724dadfa1)
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
- `pkg/checker/robustness_test.go` (SHA256:9a533733b7cee0b03650ff2106ca733b02a0090a99f783c1812ae05eb92b29e2)
- `pkg/checker/scope.go` (SHA256:88e4cf157d74d0dc9bc7f5386bbcb001c85f41092c7329c071b2a29ee6182d48)
- `pkg/checker/shadow_test.go` (SHA256:0d322aff551049f20f6dc9658b2745d6c0d45c9188d56568f0c996841634aff5)
- `pkg/checker/types.go` (SHA256:9daab5997e83b86b48b9e5107325b14719da4afc6701a5f2ae36309091b578c1)
- `pkg/checker/unused_test.go` (SHA256:391450fd74e3c68b1339ae52eadb21f3f2955ae089afa09411af08631f549879)
- `pkg/lexer/lexer.go` (SHA256:67eecc541c9c62537d64a5a89c32cfafadd24fd8bd0e3925abe8f61d86f36168)
- `pkg/lexer/lexer_extra_test.go` (SHA256:d01711a23bb91c9f31311f3fc110fd975bf45f0880a8ea67a4145c7cfe64b1c0)
- `pkg/lexer/lexer_float_test.go` (SHA256:ff1cd427658b44afcfd6fa8f2d9cbf64178b3a23868c22d265dba88c510eb09c)
- `pkg/lexer/lexer_test.go` (SHA256:11e08b90bee1f2d870b31ec032b1bdd9d7e2e37adea3c281751a8e9726584305)
- `pkg/lexer/token.go` (SHA256:4aec196a9d797ffc2ce893f0390dc0df4abece2bd4999444d4f1684b905fe1fe)
- `pkg/lexer/token_test.go` (SHA256:75e77d83a49979fe01699b5715da6f7e200d6ee55afdc7bed4f7f706040fa3ec)
- `pkg/lexer/utils.go` (SHA256:9fa86f0fd053d13368bd946ca3d09b72e8d2835c510cb4179c2bf21885a1f233)
- `pkg/parser/ast.go` (SHA256:db046bb39eddfe463415312ef30ae4bc687d54f0befffba084a8de06148a0d3f)
- `pkg/parser/ast_test.go` (SHA256:0c316b04de9c188ea2459e7d7992a9b20507d5791f8f02072b65f8fe05514217)
- `pkg/parser/comment_test.go` (SHA256:e8eaf4bda6a04c91794e970d4e372fe228d883ddf01ea38271503a65c9e4fa21)
- `pkg/parser/interface_test.go` (SHA256:2963cc5915f265bdb8041f8ff83fd1a05a3973bf5267671d7f38d51cb17e94b3)
- `pkg/parser/method_test.go` (SHA256:4e6f447524c64fad5fba414e8529792124afa3661f1ff636475b7340bc0f3c10)
- `pkg/parser/multi_return_test.go` (SHA256:158778277a88e205ba3e4edc1d960e29240a595b949c1b2e6399f020445e4298)
- `pkg/parser/parser.go` (SHA256:25bb8247ed272723f781a49eb31b97213b97f52266ccc315bce02af104e2c4ac)
- `pkg/parser/parser_extra_test.go` (SHA256:34f4990d9c764a53b104542e8c7baa6f3c7f82288d1b4ee4cae3b04e478f868a)
- `pkg/parser/parser_fixes_test.go` (SHA256:400c4ee9580a628eec4057bac2b2673c3c7844237c73d4053db07fd6796244ec)
- `pkg/parser/parser_test.go` (SHA256:2272df775fdfbf6650c2c8c205dcb24e3dec8c091b95dd86d3b9bca8177f1a1e)
- `pkg/parser/struct_literal_test.go` (SHA256:7e0f3c6b6c42eac6d7aa15ebcea9756825748cf11deebd8a5de54f78fe8c1bf5)
- `pkg/parser/struct_test.go` (SHA256:e2b7ec62f1040c2f0f00ac201d3086a1db11ed51d6f0b8b5cc0999364a07afd8)
- `pkg/parser/var_test.go` (SHA256:7a87f92a373259a0114d4964f3c9a17ee91c896465066761c3175d1eef8995de)

### Version 1.7.3 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Analysis**: Fixed "Tree Walk Blindness" in `analyzer.go` by adding traversal for `ArrayLiteral`, `HashLiteral`, `IndexExpression`, and `MemberExpression`.
- **Analysis**: Improved `inferType` to support `MemberExpression` and Literals.
- **Tests**: Added tests for Call Graph completeness and Type Inference.

**Konteks Sesi**:
- **Analysis Foundation**: Memperkuat pondasi alat analisis agar tidak "buta" terhadap struktur AST yang lebih dalam, sesuai arahan user.

**File Terkait**:
- `pkg/analysis/analyzer.go` (SHA256:6affdb6232dad3e4e4e3d4192b583169b98679f9adf7c97121bb1d8b7ea837a0)
- `pkg/analysis/analyzer_test.go` (SHA256:9921017608e692e25746ad25b14114d319de5d82a4137254ae8f851f690138af)

### Version 1.7.2 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Integrity**: Synchronized checksums for `pkg/checker/checker.go`, `pkg/analysis/analyzer.go`, `pkg/checker/import_test.go`, and `pkg/checker/import_type_test.go`.
- **Audit**: Verified no legacy `morphlang` import paths remain. Confirmed all imports use `github.com/VzoelFox/morph`.

**Konteks Sesi**:
- **Integrity Restoration**: Mengatasi "Ghost Changes" yang terjadi akibat refactor path/URL sebelumnya yang tidak tercatat sempurna.
- Memastikan AGENTS.md kembali menjadi Single Source of Truth yang akurat.

**File Terkait**:
- `pkg/checker/checker.go` (SHA256:91b3244ed88920549664767185e0b378b2c4c8175900414459bac7727c22c257)
- `pkg/analysis/analyzer.go` (SHA256:5f3ac30b83dde6109daae28cb4517c0eed086ab97842e316a4b922b606f3ad49)
- `pkg/checker/import_test.go` (SHA256:6af0dab28a04048c6ad7f84f75a4d6a4778babfd90372e2154d88326ea7c6a81)
- `pkg/checker/import_type_test.go` (SHA256:d3e4e47f4952872b084fa8534cae971e07d2d2dd35ee777fb14cda471b6f14b5)

### Version 1.7.1 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Parser**: Updated `parseType` to support Qualified Types (e.g., `Module.Type`).
- **AST**: Added `QualifiedType` node.
- **Checker**: Updated `resolveType` to resolve Qualified Types from imported modules.
- **Checker**: Updated `checkImport` to export Types (Structs/Interfaces) from modules.
- **Tests**: Added `pkg/checker/import_type_test.go` verifying cross-module type usage.

**Konteks Sesi**:
- **Tightening Integration**: Menutup gap fundamental di mana tipe data antar-module tidak bisa digunakan.
- Sekarang developer bisa melakukan `var u model.User` (Type Resolution) selain `model.NewUser()` (Value Resolution).

**File Terkait**:
- `pkg/parser/ast.go` (SHA256:db046bb39eddfe463415312ef30ae4bc687d54f0befffba084a8de06148a0d3f)
- `pkg/parser/parser.go` (SHA256:8279f1bd69b0a8894ea65a4870a19a54af85c992e90e1e7edd801856cd1e8441)
- `pkg/checker/types.go` (SHA256:9daab5997e83b86b48b9e5107325b14719da4afc6701a5f2ae36309091b578c1)
- `pkg/checker/checker.go` (SHA256:679519a419be7de45c0aae08b880b9622295144d1fa6890fe1512657807ce31e)
- `pkg/checker/import_type_test.go` (SHA256:a799b6e8c2bd3840399173959734442d9d1e1fe37d1a9e93aed4db0449643eb7)
- `.vzoel.jules/technical-debt.morph.vz` (SHA256:c52593ebdd0bd3d57ad2970c79459bd17c84e77647867d2f1c87b8ac0d240195)

### Version 1.7.0 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Import System**: Implemented `ImportStatement` processing in `pkg/checker/checker.go`.
- **Type System**: Added `KindModule` and `ModuleType` to support module namespaces.
- **Checker**: Added `Importer` interface and `SetImporter` to `Checker` for loading external modules (robust integration foundation).
- **Checker**: Implemented Cycle Detection and Module Caching.
- **Checker**: Implemented `MemberExpression` logic for `KindModule` access (e.g. `math.PI`).
- **Tests**: Added `pkg/checker/import_test.go` with integration tests for imports, visibility, and cycles.

**Konteks Sesi**:
- **Building Foundation**: Mengimplementasikan sistem Import yang robust sebagai pondasi integrasi.
- Menyelesaikan gap fungsional utama di mana `ambil` sebelumnya diabaikan oleh Checker.
- Mendukung "Exported Symbols" (Uppercase) dan "Private Symbols" (Lowercase).

**File Terkait**:
- `pkg/checker/types.go` (SHA256:9daab5997e83b86b48b9e5107325b14719da4afc6701a5f2ae36309091b578c1)
- `pkg/checker/checker.go` (SHA256:1835cc16b7eab89d15bf36dac78c266da02079d77cd8d7dad9f7069cf45d1d0e)
- `pkg/checker/import_test.go` (SHA256:228faec0cf5bfee2b14a8a2e94682453905966fa43922b645f5eebcccccd420f)
- `.vzoel.jules/technical-debt.morph.vz` (SHA256:5bf1f421c6712d68e2a12db07762e1a36176c83dcff34daeffdbce840cb8d984)

### Version 1.6.2 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Type System**: Added `KindUserError` and `UserErrorType` to `pkg/checker/types.go` to distinguish user-facing `Error` from internal compiler errors.
- **Type System**: Updated `AssignableTo` to allow `Null` assignment to `UserErrorType`.
- **Checker**: Registered `Error` as `UserErrorType` in global scope in `pkg/checker/checker.go`.
- **Tests**: Added `pkg/checker/error_type_test.go` verifying `Error` usage and `nil` assignment.

**Konteks Sesi**:
- **Closing Gap**: Menyelesaikan gap kedua di mana tipe `Error` (User-Facing) belum didefinisikan, menghalangi pola "Error as Value".
- Memisahkan `KindError` (Internal) dan `KindUserError` (Eksternal) untuk keamanan.

**File Terkait**:
- `pkg/checker/types.go` (SHA256:c9a585076ffa13a9926395270bf8c009be40b85050a0aab687c9d26841c1e71c)
- `pkg/checker/checker.go` (SHA256:fcae13d286847ba2998e097969f726980e57c55887f1c4e4230edcf4ae2963a3)
- `pkg/checker/error_type_test.go` (SHA256:2a9797abbbaa40a5bf5edf116e25ee67ba5abb16dcc4e33973b27128e85c0c69)
- `.vzoel.jules/technical-debt.morph.vz` (SHA256:c97935f7ae70410fa92dcbbf6bb33a869d6ad4aafd7b1400cf033eb3ae1d0816)

### Version 1.6.1 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Type System**: Added `AssignableTo` method to `Type` interface in `pkg/checker/types.go` and implemented logic for all types.
- **Type System**: Implemented Interface Satisfaction check (Duck Typing) in `StructType.AssignableTo`.
- **Checker**: Updated `pkg/checker/checker.go` to use `AssignableTo` instead of `Equals` for assignment, variable initialization, call arguments, and return values.
- **Tests**: Added `pkg/checker/interface_impl_test.go` covering valid/invalid interface implementations.

**Konteks Sesi**:
- **Closing Gap**: Memperbaiki gap di mana Type Checker sudah mendukung definisi Interface tapi Type System belum mendukung pengecekan implementasi (Interface Satisfaction).
- Menggantikan pengecekan Strict Equality (`Equals`) dengan `AssignableTo` untuk memungkinkan polymorphism (Struct -> Interface).

**File Terkait**:
- `pkg/checker/types.go` (SHA256:ef34daa0c8fd582511ed149ce49e89775a5240375c8c66f6860f36a8e6f32c56)
- `pkg/checker/checker.go` (SHA256:14693b66b636b24654f1bc46156c9d9b4ab3914669df44ea9ea75064d5e35394)
- `pkg/checker/interface_impl_test.go` (SHA256:bc43296c6708234da50fa1322c41abf708ae62c38eb699a914a89d68df9e05c9)
- `.vzoel.jules/technical-debt.morph.vz` (SHA256:7a0e8d7dcfa881108e600d2c03a37db6c26d01b5d2f4296e53b321414926839d)

### Version 1.6.0 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Refactor**: Renamed module from `morphlang` to `morph` in `go.mod` and all import paths.
- **Tooling**: Created `checksum_gen.go` to automate SHA256 checksum generation for `AGENTS.md` compliance.
- **Fix**: Updated `pkg/analysis/analyzer.go` to support current AST structure (fixing build errors exposed by refactor verification).
- **Scope**: Updated all references in `pkg/` to use `github.com/VzoelFox/morph`.

**Konteks Sesi**:
- **Refactor & Tooling**: Melakukan refactor nama module sesuai permintaan user dan membuat tool otomatisasi checksum ("cide generator" / `jules` tool).
- Memastikan integritas build dengan memperbaiki `pkg/analysis` yang tertinggal dari perubahan AST sebelumnya.

**File Terkait**:
- `go.mod` (SHA256:c7de783d9e3378d10d7865c5c05a3d05652403e1016f735455185a905f911340)
- `pkg/analysis/analyzer.go` (SHA256:6dbd67bd3e12e9a7fb4c0af1b41ffc039675b9a14a7257749ae32d6d83350d60)
- `checksum_gen.go` (SHA256:b06c465cd0259a681ff641323145fe87531dd3c7c947462b61c15ba63850ab76)

### Version 1.5.3 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Checker**: Implemented Cyclic Struct Dependency Detection (Error).
- **Checker**: Implemented Dead Code Detection (Warning for code after return).
- **Checker**: Implemented Unused Variable Detection (Warning).
- **Checker**: Added support for Prefix Expressions (`!` and `-`).
- **Scope**: Enhanced to track variable usage with `SymbolInfo`.
- **Tests**: Added `cycle_test.go`, `deadcode_test.go`, `unused_test.go` and updated `shadow_test.go`.

**Konteks Sesi**:
- **The Last Mile**: Menuntaskan robustness Type Checker hingga 100%.
- Mencegah crash akibat recursive struct definition.
- Meningkatkan kualitas kode developer dengan warning dead code dan unused variables.
- Memperbaiki parsing/checking prefix operator yang sempat terlewat.

**File Terkait**:
- `pkg/checker/checker.go` (SHA256:30d44903ff0863e42616c26f183d20caa27003806c227b2bc321db9c7c914a31)
- `pkg/checker/scope.go` (SHA256:88e4cf157d74d0dc9bc7f5386bbcb001c85f41092c7329c071b2a29ee6182d48)
- `pkg/checker/cycle_test.go` (SHA256:4ab36e3ad47d9904bff3738fe7fc24542d903dbbab3697f4d3be61efa98578b2)
- `pkg/checker/deadcode_test.go` (SHA256:fb440573764fad90c9a865184693010c4589802738621e5695715e9e4a8ab14b)
- `pkg/checker/unused_test.go` (SHA256:391450fd74e3c68b1339ae52eadb21f3f2955ae089afa09411af08631f549879)
- `pkg/checker/shadow_test.go` (SHA256:0d322aff551049f20f6dc9658b2745d6c0d45c9188d56568f0c996841634aff5)

### Version 1.5.2 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Checker**: Implemented strict homogeneity check for Array and Map literals.
- **Checker**: Implemented Shadowing Warning for local variables shadowing outer declarations.
- **Checker**: Implemented Error for same-scope redeclaration (Global and Local).
- **Scope**: Updated `DefineVariable` to return detailed Shadow Warning.
- **Tests**: Added `pkg/checker/literal_strict_test.go` and updated `pkg/checker/shadow_test.go`.

**Konteks Sesi**:
- **Sprint 1 Final Push**: Menyelesaikan validasi ketat dan sistem warning.
- Memastikan array/map literal konsisten (misal `[1, "a"]` adalah error).
- Memastikan variable shadowing terdeteksi dan redeklarasi dalam scope yang sama ditolak.

**File Terkait**:
- `pkg/checker/checker.go` (SHA256:969b6f849c0bbe60d0b66c5e1d341320d53424ad456cfdbbfe8c37cb2c6d6460)
- `pkg/checker/scope.go` (SHA256:2010ce96891e08816ed221275ad5dfdc9c91c5be31c9d277fee76f9d0a2f38cc)
- `pkg/checker/literal_strict_test.go` (SHA256:56cbd508515ca0f37c9fccd33049abe75793bc6dff2244899fb6ca48ab451669)
- `pkg/checker/shadow_test.go` (SHA256:e8fc9cd0d6a17f246fbba561e675dc832ddca3d4b2db31103756170e9ca317de)

### Version 1.5.1 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Checker**: Updated `ArrayType.Equals` and `MapType.Equals` to allow `UnknownType` (wildcard) compatibility for empty literals.
- **Checker**: Implemented `isUnresolved` helper to strictly forbid type inference from empty literals (`var x = []`).
- **Tests**: Added `pkg/checker/robustness_test.go` covering empty literal scenarios.

**Konteks Sesi**:
- **Emergency Fix**: Memperbaiki bug di mana empty literal ditolak saat assign ke typed variable.
- Memastikan kebijakan "Explicit over Implicit": Variable tanpa tipe tidak boleh di-infer dari empty literal.

**File Terkait**:
- `pkg/checker/types.go` (SHA256:80541bf5d78617608ec12b3c0ed2b6dbf6c4c93c36c6fd45267c00128e246c56)
- `pkg/checker/checker.go` (SHA256:25f8a8a0bf65272546a2d6552f83d3d0fd5d297c792a3359fee9e3101cf7ef27)
- `pkg/checker/robustness_test.go` (SHA256:30126f1369a04cfec5b82a0540354368813e542699bf8d3bbfcf145b7ae73015)

### Version 1.5.0 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Parser**: Support syntax Method Receiver `fungsi (u User) Name()`.
- **Parser**: Implementasi 2-token lookahead (`peekToken2`) untuk disambiguasi method vs anonymous function.
- **Checker**: Method binding ke `StructType`.
- **Checker**: Resolution method call via `MemberExpression` (`obj.method()`).
- **Checker**: Support receiver variable access inside method body.
- **AST**: Update `FunctionLiteral` untuk menyimpan `Receiver`.
- **Types**: Update `StructType` untuk menyimpan `Methods`.

**Konteks Sesi**:
- Menyelesaikan **Sprint 2: Method Receivers**.
- Memungkinkan definisi method pada struct dengan syntax Go-like.
- Menangani parsing ambiguity antara method dan anonymous function.
- Mengintegrasikan method ke dalam sistem tipe struct.

**File Terkait**:
- `pkg/parser/ast.go` (SHA256:f088eeca9a386e86d7e5bfe9edbd320ab5eb5cdf25c7a7b959c17624fa6924c8)
- `pkg/parser/parser.go` (SHA256:cf582d7517d14ab1ca74b32aa9946702ab7a8d3f3801abf0b078a0bafcf52c00)
- `pkg/checker/types.go` (SHA256:956beaeb46cb3f5f04f73120c6c23355b9f386d27d961bab56b35fa81b844759)
- `pkg/checker/checker.go` (SHA256:85dc8da982070702bc472c1a02f37fb5cbdc51f4ef8a7764b96b6fca44292f67)

### Version 1.4.0 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Core**: Implementasi `MemberExpression` untuk akses struct field (`obj.field`).
- **Checker**: Implementasi Control Flow Analysis (Validasi missing return paths).
- **Checker**: Implementasi validasi tipe Array Literal & Map Literal (homogenous types).
- **Checker**: Implementasi Warning untuk Variable Shadowing.
- **AST**: Penambahan node `MemberExpression`.
- **Tests**: Penambahan test suite (`control_flow`, `literal`, `member`, `shadow`).

**Konteks Sesi**:
- Menyelesaikan **Sprint 1: Foundation Fixes**.
- Memperbaiki bug kritis pada akses struct member (sebelumnya pakai hack IndexExpression).
- Menambahkan validasi return path yang lebih ketat.
- Menambahkan warning sistem untuk shadowing.

**File Terkait**:
- `pkg/parser/ast.go` (SHA256:b86de603216438e0ac31af404a832cd8140a87e63372a3abe99c6caecbaf5aeb)
- `pkg/parser/parser.go` (SHA256:b41dc3aa9db1fe4bdea900af9367ff7db972c731c2490daf0da71abbaad84f2c)
- `pkg/checker/checker.go` (SHA256:bf499d379114ea138f4a0f22872c5013acafdfe4d583d57aa8713c01ef65391d)
- `pkg/checker/scope.go` (SHA256:ca6a09bba41d64540f1f164568ae4d6fa01894e6baff311ef77a51c53e03ef7e)

### Version 1.3.2 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Checker**: Added Control Flow type checking (`If`, `While` condition must be Bool).
- **Checker**: Implemented robust Return Type checking using Context Stack (supports nested functions).
- **Documentation**: Updated `technical-debt.morph.vz` with detailed Analysis Tool improvement plan (deferred).
- **Documentation**: Created `.vzoel.jules/next-improvements.morph.vz`.

**Konteks Sesi**:
- Meningkatkan robustness Type Checker sesuai definisi "fully robust".
- Mencatat saran improvement dari Claude AI untuk `pkg/analysis` sebagai hutang teknis yang terstruktur.

**File Terkait**:
- `pkg/checker/checker.go` (SHA256:VARIOUS)
- `.vzoel.jules/technical-debt.morph.vz` (SHA256:VARIOUS)

### Version 1.3.1 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Syntax**: Added Struct Literal syntax `User{id: 1}`.
- **Checker**: Support for Struct Instantiation, Field Access (`x.field`), and Null (`kosong`) assignment.
- **AST**: Added `StructLiteral` node.
- **Analyzer**: Updated to traverse Struct Literals.

**Konteks Sesi**:
- Melengkapi Type Checker dengan fitur Struct dan Null Safety.
- Menambahkan test coverage untuk syntax baru.

**File Terkait**:
- `pkg/parser/ast.go` (SHA256:VARIOUS)
- `pkg/parser/parser.go` (SHA256:VARIOUS)
- `pkg/checker/checker.go` (SHA256:VARIOUS)
- `pkg/checker/types.go` (SHA256:VARIOUS)

### Version 1.3.0 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- **Core**: Implemented `pkg/checker` for Type Checking (Pass 1 & 2).
- **Parser**: Updated AST (`TypeNode`, `VarStatement`) and Parser (Typed functions, explicit types).
- **Lexer**: Added `VAR` ("var") and `MAP` ("map") tokens.
- **Analysis**: Updated `pkg/analysis` to support `VarStatement` and typed AST.
- **Tests**: Added `pkg/checker` tests, `pkg/parser/var_test.go`, and updated existing tests.

**Konteks Sesi**:
- Memulai **Fase 2: Semantik & Tipe**.
- Menetapkan sintaks tipe Go-style (`name Type`, `[]T`, `map[K]V`).
- Mewajibkan deklarasi eksplisit dengan `var`.
- Implementasi Type Checker dasar untuk assignment, binary ops, dan function calls.

**File Terkait**:
- `pkg/lexer/token.go` (SHA256:cdfad9b7c2096d53f4ab00f77429656e63d7a9c3f7ef14ba812904a4ffce29ab)
- `pkg/parser/ast.go` (SHA256:fcd635f877c2d689a2f7625163a8a52fa187381a12321ac8fc3ff2bcc09f9dfa)
- `pkg/parser/parser.go` (SHA256:0ef22bee4f447226104d82222264b1f23161f2448b0e9569e3f8a1d9f2f95fc7)
- `pkg/checker/checker.go` (SHA256:efd0bc77dee133005c5937a7b3e77e07b91858a80513cfb7d1806fb75d87b69e)

### Version 1.2.3 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- Parser: Fixed `StructStatement` parsing to include field types.
- Analysis: Added support for `StructStatement` and `ImportStatement`.
- Analysis: Updated `Context` struct to store `Structs`.
- Testing: Added `struct_test.go` and updated `analyzer_test.go`.
- Technical Debt: Mark Analysis Robustness as Resolved.

**Konteks Sesi**:
- Meningkatkan robustness Analysis tool.
- Memperbaiki struktur data aktual (Parser AST) untuk Struct.

**File Terkait**:
- `pkg/parser/parser.go` (SHA256:e8d586f047391ec75a4138ba3dd7c76e34afa01dd64b3229eb6175647bc07562)
- `pkg/analysis/analyzer.go` (SHA256:f70d8d0ca5ea60ebb86ced21268ee2c12e6c1f501ee69c3fc4a6f6f48e9ea080)
- `pkg/analysis/context.go` (SHA256:ac8b6422aedd2122b7546a841a30e99f212714fbbc8b2e419a176e5e501788a4)

### Version 1.2.2 - 2025-12-20
**Checksum**: SHA256:VARIOUS
**Perubahan**:
- Update `DESIGN.md`: Ubah referensi ekstensi dari `.morph` ke `.fox`.
- Update `ROADMAP.md`: Ubah referensi ekstensi dari `.morph` ke `.fox`.
- Added `pkg/parser/comment_test.go`: Verifikasi handling token COMMENT.
- Update `technical-debt.morph.vz`: Mark Parser & Naming debt as resolved.

**Konteks Sesi**:
- Alignment fase pengembangan: Meresmikan nama Morph dan ekstensi .fox.
- Membersihkan hutang teknis sebelum lanjut ke Fase 2.
- Verifikasi parser sudah robust terhadap token COMMENT.

**File Terkait**:
- `DESIGN.md` (SHA256:bd1aad3b7f49fe45f98afefb3f69908865c9668d9efb511e0c1f022faa05115a)
- `ROADMAP.md` (SHA256:d93c5fb3e6c05e09bdadf08648aa20532331686a07099c9e868dcbada0bcd5ee)
- `pkg/parser/comment_test.go` (SHA256:eaf8d83cf65292e7d4b6480bd3227df6f043dae2b178862b3be6449b22311fab)
- `.vzoel.jules/technical-debt.morph.vz` (SHA256:6a783170d7b7a2ce7001a2e13abb2ccdb213ac76895c19b6c26cdd00292db511)

### Version 1.2.1 - 2025-12-20
**Perubahan**:
- Menambahkan `ROADMAP.md` (Fase 1-5).
- Menambahkan dokumentasi hutang teknis di `.vzoel.jules/technical-debt.morph.vz`.

### Version 1.2.0 - 2025-12-20
**Perubahan**:
- Porting `pkg/lexer` & `pkg/parser` dari repo referensi.
- Implementasi `pkg/analysis` (Analyzer, Context generation).
- Fix Lexer tests (handle COMMENT token).
- Init go.mod.

### Version 1.1.1 - 2025-12-20
**Perubahan**:
- Update DESIGN.md: Defer Memory and Concurrency specs to post-self-host phase.

### Version 1.1.0 - 2025-12-20
**Perubahan**:
- Rename project from "Fox" to "Morph".
- Update directory structure references to `.morph.vz`.
- Define bootstrap strategy: Go language.

### Version 1.0.0 - 2025-12-20 06.10 WIB
**Checksum**: SHA256:INITIAL  
**Perubahan**:
- Initial creation of agents.md
- Mendefinisikan struktur project
- Menetapkan workflow untuk tracking perubahan

**Konteks Sesi**:
- User ingin membuat bahasa pemrograman dengan AI agent
- Fokus pada minimalisir halusinasi dengan tracking ketat
- Menggunakan checksum dan timestamp untuk setiap perubahan
- File konteks disimpan di `.morph.vz`
- Technical debt tracking di `.vzoel.jules`

**File Terkait**:
- `agents.md` (file ini)

---

## Protokol Update

### Setiap Kali Ada Perubahan File:

1. **Generate Checksum**
   ```bash
   sha256sum <filename> > .morph.vz/checksums/<filename>.sha256
   ```

2. **Catat di agents.md**
   - Versi baru (semantic versioning)
   - Timestamp ISO 8601
   - Checksum file yang berubah
   - Catatan perubahan detail
   - Konteks sesi (apa yang sedang dikerjakan)

3. **Simpan Snapshot Konteks**
   ```bash
   # Simpan state conversation di .morph.vz/context/
   session-YYYY-MM-DD-HHMMSS.md
   ```

4. **Update Technical Debt**
   ```bash
   # Catat di .vzoel.jules/
   - Apa yang belum sempurna
   - Rekomendasi perbaikan
   - TODO items
   ```

---

## Template Entry Perubahan

```markdown
### Version X.Y.Z - YYYY-MM-DD HH:MM:SS UTC
**Checksum**: SHA256:<hash>  
**Perubahan**:
- [Daftar perubahan detail]

**Konteks Sesi**:
- [Apa yang sedang dikerjakan]
- [Keputusan design yang diambil]
- [Alasan di balik perubahan]

**File Terkait**:
- `path/to/file1.ext` (SHA256:hash)
- `path/to/file2.ext` (SHA256:hash)

**Input User**:
```
[Copy paste input user jika relevan]
```

**Output Agent**:
```
[Summary output/code yang dihasilkan]
```
```

---

## Aturan untuk AI Agent

1. **SELALU** cek `agents.md` sebelum membuat atau memodifikasi file
2. **JANGAN UBAH AGENTS.md** karena ini sumber kebenaran (kecuali mencatat log perubahan)
3. **SELALU** generate checksum untuk file yang berubah
4. **SELALU** catat konteks lengkap di setiap entry
5. **JANGAN PERNAH** membuat file tanpa mencatatnya di sini
6. **JANGAN PERNAH** mengklaim mengingat sesuatu tanpa cek agents.md
7. **RUJUK** agents.md sebagai satu-satunya sumber kebenaran

---

## Definisi Status File

- **Active**: File sedang aktif digunakan
- **Deprecated**: File lama, diganti file baru
- **Archived**: File disimpan untuk referensi historis
- **Planned**: File yang akan dibuat

---

## Kontrak dengan AI Agent

Saya, Claude AI, berkomitmen untuk:
- Menggunakan `agents.md` sebagai satu-satunya sumber kebenaran
- Tidak mengklaim ingat sesuatu yang tidak tercatat di sini
- Selalu update dokumen ini setelah setiap perubahan
- Mencatat semua input/output di `.vzoel.jules` untuk tracking
- Menjaga konsistensi dan akurasi informasi

---

## Next Steps

1. Diskusi spesifikasi bahasa Morph
2. Tentukan syntax dan semantics (Design Spec)
3. Implementasi Lexer & Parser (in Go)
4. Setup tooling untuk auto-checksum

---

**Catatan**: File ini akan terus di-update seiring development. Selalu cek versi terbaru dan checksum sebelum melanjutkan pekerjaan.



**Founder** : Vzoel Fox's 
