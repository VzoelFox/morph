# Halaman 6: Arsitektur IntentAST dan Pipeline Eksekusi

**Status:** Selesai (Implementasi Pilot di Windows)
**Fokus:** IntentAST, MorphRoutine Runtime, MorphStack, Parser, Compiler, Executor.

---

## 1. Pendahuluan
Sesi ini menandai tonggak sejarah penting dalam evolusi MorphFox. Kami berhasil merealisasikan visi **"Interpreted for Human, Readable for AI"** melalui pemisahan tegas antara **Intent (Niat)** dan **Execution (Eksekusi)**.

Sebelumnya, kami mencoba melompat langsung dari Token ke RPN. Kami menyadari ini melanggar prinsip "Readable for AI", karena RPN terlalu abstrak dan kehilangan konteks struktural. Solusinya adalah **IntentAST**.

## 2. Arsitektur Baru

### A. IntentAST (Abstract Syntax Tree)
SSOT: `corelib/core/intent.fox`

IntentAST adalah representasi struktural kode yang kaya akan metadata.
*   **Unit (Global):** Wadah untuk modul/file.
*   **Shard (Scope):** Wadah untuk fungsi/blok logika.
*   **Fragment (Expression):** Unit atomik logika (Statements).
*   **Hint System:** Setiap node AST memiliki pointer ke metadata asli (Baris, Kolom, Snippet Code) untuk membantu AI memahami konteks.

### B. MorphRoutine (Runtime)
SSOT: `corelib/core/runtime.fox`

Runtime environment tempat kode dijalankan.
*   **MorphRoutine:** "Green Thread" dengan stack terisolasi.
*   **MorphStack:** Stack virtual (di Heap) dengan proteksi overflow/underflow, menggantikan native CPU stack untuk keamanan eksekusi.
*   **Context Switching:** Mekanisme assembly low-level untuk menyimpan/memulihkan register CPU saat berpindah routine.

---

## 3. Pipeline Eksekusi (The Engine)

Kami telah membangun pipeline lengkap dari Hulu ke Hilir:

1.  **Lexer:** Mengubah Source Code menjadi Token Stream.
2.  **Parser:** Mengubah Token Stream menjadi pohon **IntentAST** di memori.
    *   *Pilot Implementation:* Parsing ekspresi biner (`10 + 20`).
3.  **Compiler:** Mengubah IntentAST menjadi **Fragment Bytecode** (RPN).
    *   Menggunakan traversal Post-Order (Left -> Right -> Op).
4.  **Executor:** Menjalankan Bytecode di atas **MorphStack**.
    *   Mendukung instruksi dasar: `LIT`, `ADD`, `SUB`, `MUL`, `DIV`, `PRINT`.

**Bukti Keberhasilan:**
Script `test_compiler_exec` berhasil menjalankan alur penuh: `Source "10 + 20" -> Result 30` tanpa crash.

---

## 4. Pencapaian Teknis (Windows & Linux)

### Windows (x86_64/asm_win)
*   **Lexer Parity:** Lexer Windows kini setara dengan Linux (String, Comments, Keywords).
*   **Engine Pilot:** Implementasi Parser, Compiler, Stack, dan Executor dilakukan pertama kali di platform ini.

### Linux (x86_64/asm)
*   **Context Switch:** Implementasi `context.s` untuk Linux sudah tersedia.
*   **Lagging:** Komponen Parser, Compiler, dan Executor belum di-porting ke GAS (Lihat Technical Debt).

---

## 5. Kesimpulan
Kami kini memiliki "Jantung" yang berdetak. MorphFox bukan lagi sekadar Lexer, tapi sudah menjadi Bahasa Pemrograman yang dapat mengeksekusi logika matematika sederhana melalui pipeline yang terstruktur dan aman.

*Dokumentasi ini dibuat otomatis oleh Jules.*
