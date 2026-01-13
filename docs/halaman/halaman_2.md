# Halaman 2: Struktur Data Linear & Ekspansi Memori

**Status: Selesai (Sesi Vector & RPN)**

Halaman ini mendokumentasikan langkah evolusi dari sekadar "alokator memori" menjadi sistem yang memiliki "wadah" untuk menyimpan logika program.

## 1. Evolusi Allocator (Multi-Page Support)
Kita telah mengatasi batasan alokasi 4KB.
- **Masalah:** Versi awal menolak request > 4KB (`.alloc_fail_too_big`).
- **Solusi:**
    - Jika request + Header > 4KB, Allocator menghitung jumlah halaman yang dibutuhkan.
    - Memanggil OS (`mmap`/`VirtualAlloc`) dengan ukuran total yang dibulatkan ke kelipatan 4KB.
    - Halaman besar ini ditandai sebagai "penuh" untuk alokasi berikutnya (simplifikasi untuk menghindari fragmentasi di halaman besar).

## 2. Fondasi Bahasa: Strategi RPN
Kita memutuskan arah arsitektur **Microkernel Language**:
- **Core (Assembly):** Hanya mengerti instruksi linear sederhana. Pilihan jatuh pada **RPN (Reverse Polish Notation)**.
- **Alasan:** Menghindari kompleksitas "Pohon Pointer" (AST) di level assembly. RPN ramah stack, ramah cache, dan strukturnya datar (linear).

## 3. MorphVector: Wadah Logika
Untuk menampung token RPN, kita membutuhkan Dynamic Array.
- **Implementasi:** `vector.s` (Linux) dan `vector.asm` (Windows).
- **Struktur (32 Bytes):**
    - `Buffer Ptr`: Pointer ke data heap.
    - `Length`: Jumlah item aktif.
    - `Capacity`: Kapasitas buffer saat ini.
    - `Item Size`: Ukuran per elemen (byte).
- **Fitur:** Automatic Resizing (Geometric Growth 2x). Saat penuh, vector mengalokasikan buffer baru yang lebih besar dan menyalin data lama.

## 4. Observabilitas & Utilitas
- **`__mf_print_int` / `__mf_print_str`**: Kemampuan debugging dasar tanpa libc.
- **`__mf_memcpy`**: Utilitas copy memori byte-per-byte, krusial untuk operasi resize vector.

---

## 5. Codebase Rapuh (Technical Debt & Gaps)
Bagian ini mencatat area yang "bolong" atau berisiko tinggi saat ini:

### A. Memory Leaks (By Design)
- **Status:** Kritis.
- **Deskripsi:** Fungsi `mem_free` masih kosong (no-op). Saat Vector melakukan resize, buffer lama (`Old Buffer`) ditinggalkan begitu saja di heap tanpa dikembalikan ke OS atau ditandai bebas.
- **Dampak:** Penggunaan memori akan terus naik (monotonic growth).
- **Solusi Nanti:** Implementasi `mem_free` atau Garbage Collector (Daemon Cleaner).

### B. Inefisiensi Big Page
- **Status:** Moderat.
- **Deskripsi:** Jika kita mengalokasikan 5KB (butuh 2 Page / 8KB), sisa 3KB di halaman tersebut tidak digunakan lagi oleh allocator bump pointer saat ini. Halaman dianggap "penuh" segera setelah alokasi besar.
- **Dampak:** Pemborosan memori (Internal Fragmentation).

### C. Manual Register Management
- **Status:** Risiko Tinggi.
- **Deskripsi:** Kita menulis assembly murni. Bug seperti korupsi register Callee-Saved (`%r12`) sudah terjadi. Tidak ada compiler yang menjaga kita.
- **Mitigasi:** Disiplin coding, komentar ketat, dan unit test per fungsi.

### D. Windows Handle Dependency
- **Status:** Terkelola.
- **Deskripsi:** `builtins` di Windows bergantung pada `GetStdHandle(-11)`. Jika di masa depan kita membuat binary tanpa subsystem Console standar, ini mungkin gagal.
