# Halaman 11: Manajemen Siklus Hidup & Pemulihan Memori

Halaman ini mendokumentasikan mekanisme kritis yang ditambahkan untuk menangani siklus hidup Routine (Thread) dan persistensi memori melalui Snapshot. Fitur ini menutup celah "Memory Leak" dan memungkinkan "System Resilience".

## 1. Routine Disposal (Pembersihan Zombie)

Dalam arsitektur MorphRoutine, setiap routine memiliki Stack dan Struct sendiri di Heap. Masalah muncul ketika sebuah routine selesai: ia tidak bisa membebaskan stack-nya sendiri karena ia *sedang berjalan di atas stack tersebut*.

### Masalah: The Suicide Paradox
Jika routine memanggil `stack_free(current_stack)`:
1. Allocator menandai memori sebagai bebas.
2. Routine mencoba melakukan `ret` atau context switch.
3. CPU mengakses stack yang sudah "free" -> **Use-After-Free** atau **Corruption**.

### Solusi: Temporary Exit Stack
Kami mengimplementasikan strategi "Lompat ke Sekoci" di `scheduler_exit_current`:

1. **Switch Stack Awal:** Sebelum melakukan apa pun, routine berpindah ke stack statis global kecil bernama `__scheduler_exit_stack_top`.
2. **Free Resources:** Karena sekarang berjalan di stack aman, routine bisa memanggil `stack_free` untuk stack lamanya dan `mem_free` untuk struct-nya.
3. **Context Switch:** Scheduler memilih routine berikutnya dan melakukan switch.

```asm
scheduler_exit_current:
    # 0. Pindah ke Stack Sementara (Sekoci)
    leaq __scheduler_exit_stack_top(%rip), %rsp

    # 1. Hapus dari Antrian Scheduler
    # ... (Update Linked List) ...

    # 2. Bebaskan Memori (Aman dilakukan sekarang)
    call stack_free
    call mem_free

    # 3. Switch ke Next Routine
    call morph_switch_context
```

## 2. Snapshot Recovery (Pemulihan Presisi)

Sistem snapshot sebelumnya hanya menyimpan *isi* memori, tapi tidak *lokasi* (alamat) memori. Karena MorphFox menggunakan Raw Pointers (bukan handle), memuat data ke alamat yang berbeda akan merusak semua pointer (Linked List, String, Struct).

### Format Snapshot Baru
Setiap Page dalam file dump sekarang memiliki format:
1. **Page Address (8 Bytes):** Alamat virtual asli page tersebut.
2. **Page Header (48 Bytes):** Metadata page (Next, Prev, Size, dll).
3. **Content (Size Bytes):** Isi raw data.

### Mekanisme Recovery (`MAP_FIXED`)
Saat memuat snapshot (`mem_snapshot_recover`):
1. Baca **Page Address**.
2. Baca **Page Size**.
3. Minta OS untuk mengalokasikan memori **tepat di alamat tersebut**:
   - **Linux:** `mmap(addr, size, ..., MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, ...)`
   - **Windows:** `VirtualAlloc(addr, size, MEM_COMMIT | MEM_RESERVE, ...)`
4. Salin konten dari file ke memori yang baru dipetakan.
5. Pulihkan global state (`current_page_ptr`, `current_offset`).

Ini menjamin bahwa `0x7F...123` di sesi sebelumnya tetap `0x7F...123` di sesi baru, menjaga integritas pointer.

## 3. Catatan Implementasi Teknis

### Register Clobbering (Bug %r8)
Selama pengembangan, ditemukan bug di mana register `%r8` (yang menyimpan pointer Scheduler Instance) rusak setelah memanggil `mem_free`. Ini menyebabkan Segfault saat mencoba update status scheduler.
**Fix:** Selalu reload pointer global ke register setelah memanggil fungsi eksternal (ABI System V / Win64 volatile registers).

### Paritas Windows
Implementasi di `corelib/platform/x86_64/asm_win/` telah disesuaikan:
- **Scheduler:** Menggunakan Shadow Space (32 bytes) sebelum memanggil `stack_free`/`mem_free`.
- **Snapshot:** Menggunakan `VirtualAlloc` untuk emulasi `MAP_FIXED`.
- **Builtins:** Menambahkan `__mf_print_asciz` untuk memudahkan debugging string null-terminated.
