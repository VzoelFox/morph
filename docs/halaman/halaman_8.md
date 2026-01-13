# Halaman 8: Infinite Registers, Bytecode as Glue & SSOT Purity

**Status:** Stabil / Terimplementasi
**Versi:** MorphFox VM v0.5

Dokumen ini merangkum pencapaian arsitektur dalam mewujudkan mesin RPN yang Turing-Complete, sadar konteks (Context Aware), dan memiliki paritas penuh antar platform (Windows & Linux).

## 1. Arsitektur "Bytecode as Glue"

Kami mendefinisikan ulang peran Bytecode RPN bukan hanya sebagai kalkulator tumpukan, tetapi sebagai **Lapisan Lem (Glue Layer)** yang deterministik antara logika Userland dan efek samping Sistem Operasi.

### Effect Boundary (`OP_SYSCALL`)
Instruksi `OP_SYSCALL` (Code 40) bertindak sebagai gerbang tunggal untuk semua interaksi eksternal.
- **IntentTree:** Menggunakan "Intent ID" abstrak (misal: `SYS_INTENT_WRITE`) alih-alih nomor syscall OS mentah. Ini menjamin portabilitas bytecode.
- **Hook Deterministik:** Label `glue_pre_syscall` ditempatkan tepat sebelum eksekusi syscall, memungkinkan hook masa depan untuk Snapshotting, Replay, atau Debugging sebelum efek samping terjadi.
- **Reverse Stack Convention:** Argumen dipush dengan urutan terbalik (Arg N ... Arg 1) diikuti Intent ID, memungkinkan Executor melakukan `POP` sekuensial yang efisien.

## 2. Infinite Registers (Simulasi Variabel)

Untuk menjembatani kesenjangan antara Stack Machine (RPN) dan Register Machine (CPU/SSA), kami mengimplementasikan fitur "Register Tak Terbatas" menggunakan stack itu sendiri.

### Mekanisme Random Access
Alih-alih hanya mengakses Top of Stack (TOS), VM kini mendukung akses acak berdasarkan kedalaman (Depth):

*   **`OP_PICK N` (Read):** Menyalin nilai dari kedalaman `N` ke Top. Mensimulasikan pembacaan variabel lokal tanpa menghapusnya dari memori.
*   **`OP_POKE N` (Write):** Mengambil nilai dari Top dan menimpanya ke slot pada kedalaman `N`. Mensimulasikan *assignment* variabel (`x = val`).

Fitur ini memungkinkan kompilator masa depan untuk memetakan variabel lingkup (scope variables) langsung ke offset stack relatif, tanpa perlu alokasi register fisik yang rumit.

## 3. SSOT Purity & AI Readability

Kami memperketat kepatuhan terhadap **Single Source of Truth (SSOT)** dengan strategi dokumentasi baru.

*   **Technical Hints:** Setiap definisi Opcode di `rpn.fox` kini dilengkapi dengan "Hint" teknis.
    *   *Tujuan:* Menjelaskan **mekanik** (bagaimana stack berubah) dan **intensi** (mengapa opcode ini ada) secara eksplisit.
    *   *Hasil:* AI atau manusia dapat memahami arsitektur VM hanya dengan membaca file `.fox`, tanpa perlu menebak-nebak abstraksi level tinggi.
*   **Struktur Instruksi 16-Byte:** Executor diperbaiki untuk mematuhi alignment instruksi 16-byte (8 byte Opcode + 8 byte Operand) secara ketat, menghilangkan ambiguitas parsing byte stream.

## 4. Paritas Platform

Executor Windows (`executor.asm`) dan Linux (`executor.s`) kini memiliki paritas fitur 100%:
*   **Recursion:** `OP_CALL` dan `OP_RET` menggunakan Call Stack terpisah.
*   **Control Flow:** `OP_JMP`, `OP_SWITCH`, `OP_CYCLE` (Circuit Breaker).
*   **Snapshot:** Implementasi `snapshot.asm` di Windows menggunakan WinAPI untuk setara dengan versi Linux.

## 5. Technical Debt & Gaps

Catatan untuk pengembangan selanjutnya:
1.  **Inkonsistensi Penamaan:** Fungsi stack dinamai `stack_new` di implementasi Linux dan `stack_create` di Windows. Perlu disagamkan.
2.  **Global Variable Stub:** Opcode `OP_LOAD` dan `OP_STORE` (untuk variabel global) masih berupa stub (placeholder). Membutuhkan integrasi penuh dengan Symbol Table (Hash Map).
3.  **Binary Artifacts:** Pipeline perlu memastikan file binary hasil tes (`test_registers`, `test_alloc`) tidak mencemari repository git.
