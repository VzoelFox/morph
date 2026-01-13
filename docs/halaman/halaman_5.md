# Halaman 5: Pematangan Lexer, Sistem Tipe, dan RPN

**Status:** Selesai (dengan catatan Teknis)
**Fokus:** Lexer Syntax Morph, Runtime Type System, RPN SSOT, Memory Hardening.

---

## 1. Pendahuluan
Sesi ini berfokus pada transisi dari infrastruktur dasar (Lexer minimal) menuju infrastruktur yang mampu menangani sintaks bahasa MorphFox yang sesungguhnya. Kami juga meletakkan dasar untuk "Otak" sistem yaitu Type System dan RPN Mapping.

## 2. Pencapaian Utama

### A. Lexer: Dukungan Sintaks Penuh
Lexer Assembly (`lexer.s`) telah dirombak total untuk mendukung spesifikasi Morph:
*   **Keywords:** Deteksi manual untuk `fungsi`, `tutup_fungsi`, `jika`, `selama`, `var`, dll.
*   **Delimiters:** `(`, `)`, `,`.
*   **Operators:** Deteksi *Lookahead* untuk operator majemuk `==`, `!=`, `<=`, `>=`, `->`.
*   **String Literal:** Dukungan parsing `"string"` dengan alokasi struktur di heap.
*   **Komentar:** Dukungan komentar satu baris diawali `;`.

**Implementasi Rigor:** Kami menggunakan pendekatan *Manual Unrolling* (Switch-Case Length + String Compare) di assembly untuk deteksi keyword, menghindari kompleksitas dan potensi error dari hash table dinamis di level lexer dasar.

### B. Sistem Tipe (Runtime)
Kami mendefinisikan SSOT `corelib/core/type_system.fox` dan mengimplementasikan `type.s` di Linux.
*   **TypeDescriptor:** Metadata runtime untuk struct (Nama, Ukuran, Field).
*   **FieldDescriptor:** Metadata field (Nama, Offset, Tipe).
*   **Offset Calculation:** Runtime secara otomatis menghitung offset field saat field ditambahkan, memungkinkan definisi struct dinamis.

### C. RPN (Reverse Polish Notation)
Sebagai langkah persiapan Parser, kami menetapkan `corelib/core/rpn.fox` sebagai SSOT instruksi mesin.
*   **Filosofi:** Menghindari AST (Tree) demi struktur Linear yang "Readable for AI".
*   **OP_HINT:** Opcode khusus (99) ditambahkan untuk menyimpan metadata semantik di bytecode, mempermudah AI memahami maksud blok kode tanpa harus reverse-engineering logika stack.

### D. Memory Allocator Hardening
*   **Magic Header:** Signature header memori diganti dari `0xDEADBEEF` menjadi `VZOELFOX` (`0x584F464C454F5A56`).
*   **Optimasi:** Header tetap dipertahankan efisien pada 32 byte.

---

## 3. Analisis Gap & Inkonsistensi (Retrospektif)

Dalam proses review, kami menemukan **ketimpangan pengembangan (Development Gap)** antara platform Linux dan Windows yang perlu segera diatasi sebelum masuk ke Parser.

### Gap 1: Windows Assembly Lagging
Saat ini, **Linux (x86_64/asm)** jauh lebih maju daripada **Windows (x86_64/asm_win)**.
*   **Lexer:** `lexer.asm` (Windows) masih versi minimal. Belum mendukung Keywords, Delimiters, String Literals, atau Operator Majemuk yang baru saja diimplementasikan di Linux.
*   **Type System:** `type.asm` (Windows) **BELUM ADA**. Fitur ini baru diimplementasikan di Linux (`type.s`).
*   **Dampak:** Codebase saat ini tidak portabel. Kita tidak bisa mengkompilasi kode Morph syntax di Windows.

### Gap 2: Builtins Consistency
*   `rpn.inc` baru tersedia untuk GAS (Linux). Versi NASM (`rpn.inc` untuk Windows) belum dibuat.

### Gap 3: Testing Framework
*   Script test suite (`test_lexer_morph.s`) hanya dibuat untuk Linux.

## 4. Rencana Perbaikan (Next Steps)
Sebelum melangkah ke Parser, kita **WAJIB** menyejajarkan (Synchronize) implementasi Windows dengan Linux untuk menjaga integritas "Platform Agnostic".

1.  **Porting Lexer:** Tulis ulang logika `lexer.s` ke `lexer.asm` (NASM).
2.  **Porting Type System:** Implementasi `type.asm` (NASM).
3.  **Porting Definitions:** Buat `rpn.inc` versi NASM.

*Catatan: Dokumentasi ini dibuat otomatis oleh Jules sebagai refleksi status proyek.*
