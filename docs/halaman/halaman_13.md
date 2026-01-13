# Halaman 13: Morph as WASM & Hybrid Architecture

## Pendahuluan
Halaman ini mendokumentasikan evolusi strategis MorphFox menuju platform WebAssembly (WASM). Tujuannya adalah memungkinkan MorphFox berjalan secara *native* di dalam browser, memanipulasi DOM secara langsung, dan mempertahankan filosofi "Assembly First" tanpa bergantung pada *toolchain* eksternal yang berat (seperti Emscripten).

## Arsitektur "Hybrid"
Untuk mengatasi tantangan *relokasi pointer* dan kompleksitas kompilasi di lingkungan browser, kami mengadopsi arsitektur "Hybrid":

1.  **WASM Lexer (`lexer.wat`):**
    *   Implementasi *hand-written* dalam WebAssembly Text Format.
    *   Bertugas mengubah *Source Code* mentah menjadi vektor Token.
    *   Menyimpan string literal (Identifier, String) di Heap WASM dan mengembalikan pointer valid.
    *   **Keuntungan:** Performa tinggi untuk pemrosesan teks intensif.

2.  **JS Compiler (`compiler.js`):**
    *   Berjalan di sisi Host (JavaScript).
    *   Membaca token dari memori WASM.
    *   Membangun logika (Parsing) dan menghasilkan *Bytecode RPN*.
    *   Menulis Bytecode kembali ke memori WASM untuk dieksekusi.
    *   **Keuntungan:** Mempermudah logika *Tree Parsing* yang kompleks menggunakan fleksibilitas JS, sambil tetap menghasilkan output biner standar Morph.

3.  **WASM Executor (`executor.wat`):**
    *   Mesin Virtual (VM) RPN murni dalam WASM.
    *   Mengeksekusi instruksi RPN (Stack Machine).
    *   Mengirim *Intents* (Syscall) ke Host (JS) untuk interaksi DOM.

## Pembaruan Core (Linux & Windows)
Untuk mendukung paritas sintaksis dengan kebutuhan web dan masa depan, Core Parser (Assembly x86_64) telah diperbarui:

### 1. Granular Import (`Ambil ID`)
Syntax baru diperkenalkan untuk mengimpor modul berdasarkan ID numerik (bukan path string), memfasilitasi linking internal yang lebih efisien.
*   **Keyword:** `Ambil` (Kapital 'A').
*   **Format:** `Ambil <Integer>` (Contoh: `Ambil 123`).
*   **Intent Node:** Tipe `0x1002`, Data A = ID.

### 2. Block Markers (`###`)
Dukungan penuh untuk komentar blok menggunakan marker `###`.
*   Lexer menghasilkan `TOKEN_MARKER` (ID 8).
*   Parser (Unit & Block loop) kini secara eksplisit mendeteksi dan melewati token ini, mencegah *parse error*.

## Manajemen Memori WASM
Layout memori linear WASM (640KB - 10 Pages) diatur secara manual untuk mencegah tabrakan:
*   **0 - 64KB:** Stack Area (Tumbuh ke bawah dari 65536) & Reserved.
*   **64KB (65536):** Heap Base (Allocator tumbuh ke atas).
*   **200KB+:** Area Kode (Bytecode RPN).
*   **300KB+:** Area Source Code Input.

## DOM Intents (Syscall Extension)
Sistem *Intent* diperluas untuk manipulasi DOM (Range 100+):
*   `100`: Create Element
*   `101`: Append Child
*   `102`: Set Attribute
*   `103`: Set Inner Text
*   `105`: Get Element By ID

## Kesimpulan
Arsitektur ini membuktikan fleksibilitas desain "Intent-Based" MorphFox. Dengan memisahkan Lexer (Low-level), Compiler (Logic/Bridge), dan Executor (Low-level), kita dapat beradaptasi dengan lingkungan Web tanpa mengubah kontrak dasar bahasa.
