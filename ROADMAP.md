# Roadmap Pengembangan Bahasa Morph

## Visi
Membangun bahasa pemrograman prosedural yang aman (Error as Value), mudah dipahami (Bahasa Indonesia), dan mampu melakukan self-hosting.

## Fase 1: Fondasi (✅ Selesai)
- [x] **Identitas Project**: Rename Fox -> Morph.
- [x] **Spesifikasi Desain**: Draft awal (`DESIGN.md`) disetujui.
- [x] **Lexer & Parser**: Porting dari referensi dan disesuaikan.
- [x] **Tooling Awal**: `pkg/analysis` untuk generate context AI.

## Fase 2: Semantik & Tipe (🚧 Selanjutnya)
- [ ] **Type Checker**: Validasi tipe statis (Int, String, Bool, Struct).
- [ ] **Semantic Analyzer**: Validasi scope variable dan function signature (lebih mendalam dari `pkg/analysis`).
- [ ] **AST Validations**: Pengecekan error logic (misal: return di luar fungsi).

## Fase 3: Code Generation (Target: LLVM)
- [ ] **LLVM IR Generator**: Mengubah AST menjadi LLVM IR (`.ll`).
- [ ] **Compiler Driver**: CLI untuk build file `.fox` menjadi executable.
- [ ] **Basic Runtime**: Print, String operations, Memory alloc (malloc wrapper).

## Fase 4: Self-Hosting
- [ ] **Morph-in-Morph**: Menulis ulang Lexer & Parser menggunakan bahasa Morph.
- [ ] **Bootstrap**: Mengompilasi compiler Morph menggunakan compiler Go yang sudah dibuat.

## Fase 5: Fitur Lanjutan (Deferred)
*Akan dikerjakan setelah Self-Hosting stabil.*
- [ ] **Manajemen Memori**: Implementasi Region-based Memory (Arenas).
- [ ] **Morphroutines**: Green threads dan Channels.
- [ ] **Stdlib Expansion**: Network, File IO, dll.
