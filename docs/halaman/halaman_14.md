# Halaman 14: Snapshot Recovery & System Hardening
**Versi:** 1.2 (2026-02-15)
**Status:** Stabil
**Fokus:** Perbaikan Critical Gap di Windows (Recovery), Implementasi Global Variable, dan Hardening VM.

---

## 1. Pendahuluan

Halaman ini mendokumentasikan penyelesaian gap kritis, implementasi fitur global variable, dan langkah-langkah pengerasan sistem (hardening) untuk meningkatkan stabilitas dan keamanan runtime MorphFox.

---

## 2. Snapshot V1.1 (Enhanced Format)

Untuk mendukung pemulihan memori yang handal, format snapshot diperbarui untuk menyimpan status global allocator.

### A. Format Header Baru (64 Bytes)
Padding di header tidak lagi kosong, melainkan digunakan untuk menyimpan `current_offset`.

| Offset | Ukuran | Nama | Deskripsi Baru |
|--------|--------|------|----------------|
| 0x00 | 8 | Magic | Signature 'MORPHSNP' |
| 0x08 | 8 | Version | Versi Format (1) |
| 0x10 | 8 | Timestamp | Waktu pembuatan |
| 0x18 | 8 | Page Count | Jumlah halaman memori |
| **0x20** | **8** | **Current Offset** | **Posisi pointer di halaman aktif** |
| 0x28 | 24 | Padding | Reserved (0) |

### B. Windows Recovery (`snapshot.asm`)
Implementasi `mem_snapshot_recover` di Windows kini menggunakan strategi berikut untuk mencapai paritas dengan Linux (`mmap MAP_FIXED`):

1.  **Baca Alamat Target:** Membaca alamat virtual asli dari file dump.
2.  **VirtualAlloc Eksplisit:** Meminta OS mengalokasikan memori **tepat** di alamat tersebut.
    *   Flags: `MEM_COMMIT | MEM_RESERVE`
    *   Protection: `PAGE_EXECUTE_READWRITE` (RWX)
3.  **Restorasi Konten:** Menyalin data dari file ke memori yang baru dialokasikan.
4.  **Restorasi Global:** Mengembalikan nilai `current_offset` dan `current_page_ptr`.

---

## 3. Implementasi Global Variable (Portable)

Instruksi `OP_LOAD` dan `OP_STORE` kini berfungsi penuh, menggunakan **Hash 64-bit** sebagai identitas variabel untuk menjamin portabilitas bytecode RPN antar platform (Native vs WASM).

### A. Perubahan Compiler
Compiler kini meng-emit **Hash String** (bukan Pointer) sebagai operand untuk instruksi `LOAD` dan `STORE`. Ini memungkinkan binary RPN (`.bin`) yang dihasilkan di Linux dapat dijalankan di Windows atau Web tanpa masalah relokasi memori.

*   **Operand:** Hash FNV-1a (64-bit) dari nama variabel.

### B. Executor Logic
Logika eksekusi diperbarui untuk menggunakan lookup berbasis hash (`sym_table_get_by_hash`).
*   **Trade-off:** Metode ini mengabaikan potensi *hash collision* demi portabilitas di v1.1. Resolusi collision penuh memerlukan serialisasi String Table di masa depan (v2).

---

## 4. System Hardening (Pengerasan Sistem)

Untuk memenuhi standar stabilitas produksi, kami telah menerapkan langkah-langkah hardening pada core Executor:

### A. Stack Safety
*   **Masalah:** Sebelumnya, kegagalan `stack_push` (Overflow) diabaikan, menyebabkan korupsi data diam-diam.
*   **Solusi:** Executor kini memeriksa nilai balik setiap `stack_push`.
*   **Aksi:** Jika gagal (0), VM langsung terminasi dengan **Exit Code 102 (Stack Overflow)**.

### B. Math Safety (Checked Arithmetic)
*   **Masalah:** Operasi matematika dasar (`ADD`, `MUL`, dll) menggunakan instruksi CPU mentah yang rentan overflow/underflow.
*   **Solusi:** Mengganti instruksi mentah dengan pemanggilan fungsi builtin yang aman (`__mf_add_checked`, `__mf_sub_checked`, `__mf_mul_checked`).
*   **Aksi:** Jika deteksi overflow terjadi (Error Flag = 1), VM terminasi dengan **Exit Code 103 (Math Overflow)**.

### C. Readable for AI (`OP_HINT`)
*   **Fitur:** Menambahkan handler untuk instruksi `OP_HINT` (Code 99).
*   **Fungsi:** Saat ini berfungsi sebagai *Safe NOP* untuk memungkinkan penyisipan metadata debugging di masa depan tanpa menabrakkan VM.

---

## 5. Status Paritas

Dengan perubahan ini, MorphFox mencapai tingkat paritas yang lebih tinggi:

| Fitur | Linux (Native) | Windows (Native) | Status |
| :--- | :--- | :--- | :--- |
| **Snapshot Save** | ✅ `write` | ✅ `WriteFile` | Parity |
| **Snapshot Recover** | ✅ `mmap FIXED` | ✅ `VirtualAlloc` | **Resolved** |
| **Symbol Table** | ✅ `symbol.s` | ✅ `symbol.asm` | Parity (Hash Based) |
| **Global Vars** | ✅ `executor.s` | ✅ `executor.asm` | **Resolved** |
| **Math Safety** | ✅ `__mf_*_checked` | ✅ `__mf_*_checked` | **Hardened** |
| **Stack Safety** | ✅ Check & Panic | ✅ Check & Panic | **Hardened** |

---

*Dokumen ini dibuat otomatis setelah implementasi perbaikan.*
