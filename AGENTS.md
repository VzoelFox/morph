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
