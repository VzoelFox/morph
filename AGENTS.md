# Agents.md - Source of Truth untuk AI Agent

## Metadata Dokumen
- **Versi**: 1.1.0
- **Tanggal Dibuat**: 2025-12-20 06.10 WIB
- **Terakhir Diupdate**: 2025-12-20
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
### Version 1.14.0 - 2025-12-20
**Checksum**: SHA256:CLI_DRIVER
**Perubahan**:
- **Tooling**: Created `cmd/morph/main.go` as the CLI Driver (Frontend Compiler).
- **Tooling**: Implemented `FileImporter` in `main.go` to support importing modules from the file system.
- **Example**: Created `examples/hello.fox` and `examples/std/math.fox` to demonstrate functionality.

**Konteks Sesi**:
- **Milestone**: Memasuki Fase 3 (awal) dengan menyediakan cara untuk menjalankan compiler (parsing & type checking) pada file `.fox` nyata.
- **Verification**: User meminta untuk "menjalankan compiler yang sudah ada".

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
