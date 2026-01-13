# Halaman 3: Refinement Memori & Alokator Lanjutan

**Status:** Selesai
**Fokus:** Memory Arena, Memory Pool, Allocator Refactoring (Header/Alignment), Dynamic Runtime.

---

## 1. Pendahuluan
Sesi ini berfokus pada pematangan sistem manajemen memori untuk mendukung visi **"Interpreted for human, readable for AI"**. Kita bergerak dari alokator dasar menuju strategi alokasi yang lebih *robust* (tangguh), aman, dan terstruktur.

## 2. Memory Arena (Scoped Allocator)
Kita mengimplementasikan **Arena Allocator** untuk menangani alokasi sementara yang memiliki siklus hidup yang sama (misalnya: alokasi per-frame atau per-request).

*   **Konsep:** *Bump Pointer*. Memori diambil secara linear dari blok besar.
*   **Fitur:**
    *   **Batch Reset:** Seluruh memori bisa dibebaskan sekaligus dengan me-reset pointer ke awal (`arena_reset`), tanpa perlu `free` satu per satu. Sangat cepat.
    *   **Embedded Header:** Struktur data Arena disimpan langsung di awal blok memori yang dikelolanya.
*   **Struktur (SSOT):**
    ```
    [Start Ptr] [Current Ptr] [End Ptr] [ID] [ ... User Data ... ]
    ```

## 3. Memory Pool (Fixed-Size Allocator)
Untuk objek-objek kecil yang ukurannya seragam dan sering dibuat/dihapus (misal: Node Symbol Table), kita menggunakan **Memory Pool**.

*   **Konsep:** *Free List*. Blok memori dibagi menjadi slot-slot berukuran tetap.
*   **Fitur:**
    *   **LIFO Reuse:** Slot yang dibebaskan (`pool_free`) disimpan dalam *Intrusive Linked List* (pointer `next` disimpan di dalam slot kosong itu sendiri). Alokasi berikutnya akan menggunakan slot bekas ini ("Reuse") sebelum mengambil slot baru.
    *   **Efisiensi:** Mencegah fragmentasi memori untuk objek sejenis.

## 4. Refactoring Core Allocator (`mem_alloc`)
Allocator global (`mem_alloc`) direvisi total untuk meningkatkan keamanan dan "kebersihan" data.

### A. Block Header (8 Byte)
Setiap alokasi kini memiliki header tersembunyi berukuran 8 byte tepat sebelum pointer user.
*   **Fungsi:** Menyimpan ukuran data yang diminta user (`User Size`).
*   **Manfaat:** Memungkinkan simplifikasi API dealokasi menjadi `mem_free(ptr)`. Sistem membaca header untuk mengetahui ukuran blok yang akan dibebaskan, menghilangkan risiko *human error* salah input size.

### B. Alignment 16-Byte
Setiap pointer yang dikembalikan ke user dijamin sejajar (aligned) pada kelipatan 16 byte.
*   **Alasan:**
    *   **Investasi Masa Depan:** Mendukung instruksi SIMD (SSE/AVX) yang membutuhkan alignment ketat.
    *   **Kinerja:** Akses memori yang aligned lebih ramah terhadap cache CPU modern.

### C. Padding "Nol Netral"
Ruang kosong yang timbul akibat pembulatan ukuran (alignment) kini secara eksplisit diisi dengan nilai `0` (Zeroed).
*   **Filosofi:** *"Readable for AI"*. Saat memori diinspeksi, area kosong terlihat jelas sebagai nol bersih, bukan sampah memori ("garbage") yang membingungkan. Ini menciptakan visualisasi memori yang deterministik dan "higienis".

## 5. Dynamic Page Size (Runtime)
Kita meninggalkan asumsi hardcoded `4096` bytes per page.
*   **Linux (x86_64):** Runtime kini memindai **Auxiliary Vector (AUXV)** di stack saat startup untuk mendeteksi `AT_PAGESZ` dari kernel secara dinamis.
*   **Fleksibilitas:** Sistem siap berjalan di arsitektur dengan page size non-standar (misal 16KB atau 64KB).

---

**Kesimpulan:**
Dengan fondasi memori yang disiplin ini (Header, Alignment, Padding, Arena, Pool), MorphFox siap melangkah ke tahap selanjutnya: Implementasi Struktur Data Kompleks (String & Symbol Table).
