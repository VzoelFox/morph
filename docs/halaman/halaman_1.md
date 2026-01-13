# Halaman 1: Fondasi Memori & Runtime

**Status: Dalam Pengembangan**

Dokumen ini menjelaskan implementasi teknis dari lapisan paling bawah MorphFox: Manajemen Memori dan Abstraksi Runtime.

## 1. Single Source of Truth (SSOT)
Segala definisi dimulai dari kontrak deskriptif di folder `corelib/core/`.
- **`prelude.fox`**: Aturan main global.
- **`memory.fox`**: Spesifikasi struktur data memori.

## 2. Abstraksi Platform (Macros)
Untuk menjaga kode logika tetap bersih dan konsisten di berbagai OS, kita menggunakan lapisan Macro Assembly.

- **Masalah:** Linux menggunakan syscall langsung (3 instruksi), Windows menggunakan Call Kernel32 (5+ instruksi + Shadow Space).
- **Solusi:** Macro `OS_ALLOC_PAGE`, `OS_WRITE`, `OS_EXIT`.
- **Lokasi:** `macros.inc` (di masing-masing folder platform).

## 3. Morph Allocator (Implementasi Memori)
Allocator saat ini menggunakan strategi **Bump Pointer** di dalam **Linked List of Pages**.

### Struktur Halaman (4KB)
Setiap halaman memori yang diminta dari OS memiliki Header 32-byte:

| Offset | Ukuran | Nama | Deskripsi |
|--------|--------|------|-----------|
| 0x00 | 8 byte | Next Ptr | Pointer ke halaman berikutnya (Linked List). |
| 0x08 | 8 byte | Prev Ptr | Pointer ke halaman sebelumnya. |
| 0x10 | 8 byte | Metadata | (Reserved) Ukuran blok / info lainnya. |
| 0x18 | 8 byte | Magic/Flags| `0xDEADBEEF` untuk validasi integritas. |

### Alur Alokasi (`mem_alloc`)
1.  Align ukuran request ke 8 byte.
2.  Cek apakah muat di halaman aktif.
    -   **Ya:** Geser pointer (bump), kembalikan alamat.
    -   **Tidak:** Minta halaman baru dari OS via `OS_ALLOC_PAGE`.
3.  Saat halaman baru dibuat:
    -   Tulis Header (Next=0, Prev=OldPage, Magic).
    -   Update `Next` di halaman lama agar menunjuk ke halaman baru.
4.  Jika request > (4KB - Header), alokator akan melakukan **Multi-Page Allocation** (Lihat Halaman 2).

## 4. Rencana Selanjutnya
- Implementasi `mem_free` (saat ini no-op).
- Integrasi Daemon Cleaner untuk pembersihan otomatis.
