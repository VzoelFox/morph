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
