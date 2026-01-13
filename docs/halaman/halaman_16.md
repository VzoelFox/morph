# Halaman 16: Rilis v1.1 - Finalisasi & Pembekuan Core
**Tanggal:** 2026-02-15
**Versi:** 1.1 (Stable Frozen)
**Status:** Selesai

## 1. Pendahuluan

Halaman ini merangkum sesi "Sprint Akhir" yang bertujuan untuk menutup semua celah teknis (technical gaps), menyamakan fitur antar platform (parity), memperkuat keamanan (hardening), dan menyiapkan fondasi untuk lingkungan virtual di Web. Dengan selesainya tahap ini, **Core Runtime MorphFox v1.1 dinyatakan BEKU (Frozen)**.

## 2. Rekapitulasi Perubahan Besar

### A. Windows Parity (Penyelesaian Hutang Teknis)
*   **Snapshot Recovery:** Implementasi `VirtualAlloc` dengan alamat eksplisit di `snapshot.asm` akhirnya menutup celah kritis di Windows. Sekarang, state memori bisa dipulihkan persis seperti di Linux.
*   **Networking:** Simbol `bind`, `listen`, dan `accept` ditambahkan ke `net.asm` (Winsock Wrapper) dan `net.s` (Linux Syscall), memungkinkan pembuatan server native.

### B. System Hardening (Pengerasan Keamanan)
*   **Stack Safety:** Executor kini memverifikasi setiap operasi `push`. Jika stack penuh, VM melakukan *panic* terkontrol (Exit Code 102) alih-alih merusak memori diam-diam.
*   **Math Safety:** Operasi aritmatika (`ADD`, `SUB`, `MUL`) kini menggunakan instruksi *Checked*. Overflow integer akan memicu *panic* (Exit Code 103).
*   **Fault Tolerance:** Penambahan handler untuk `OP_HINT` mencegah crash pada instruksi debug.

### C. Bytecode Portability ("Write Once, Run Anywhere")
*   **Hash-Based Linking:** Compiler dan Executor diubah untuk menggunakan **Hash 64-bit** sebagai referensi Variabel Global, menggantikan *Raw Pointers*.
*   **Dampak:** Binary RPN (`.bin`) hasil kompilasi di Linux (64-bit) kini dapat dimuat dan dijalankan dengan aman di WASM (32-bit memory space) tanpa relokasi pointer yang rumit.

### D. WASM Virtual Domain
*   **Runtime v1.1:** Alokator memori WASM diperbarui agar memiliki header yang kompatibel dengan standar Native.
*   **JS Host Modern:** `loader.js` ditulis ulang menggunakan pola *Functional Factory* untuk menghilangkan penggunaan `this` yang rawan masalah konteks, serta mendukung pemuatan binary offline (`loadBinary`).

## 3. Artefak Rilis

Script build otomatis telah disiapkan untuk menghasilkan binary tunggal yang independen:
*   **Linux:** `scripts/release_linux.sh` -> menghasilkan `morph_v1.1`
*   **Windows:** `scripts/release_windows.bat` -> menghasilkan `morph_v1.1.exe`

## 4. Kesimpulan

MorphFox v1.1 kini berdiri di atas tiga pilar yang kokoh:
1.  **Paritas:** Linux, Windows, dan Web memiliki kemampuan dasar yang setara.
2.  **Keamanan:** Runtime dilindungi dari Stack Overflow dan Integer Overflow.
3.  **Portabilitas:** Bytecode RPN bersifat universal.

Pengembangan selanjutnya (v1.2+) dapat berfokus sepenuhnya pada fitur bahasa tingkat tinggi (Standard Library, GC, Optimasi) tanpa perlu merombak ulang arsitektur dasar.
