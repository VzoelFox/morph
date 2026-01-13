# Halaman 12: Bytecode as Glue & System Sovereignty (Parity Completed)
**Versi:** 1.0 (2026-02-15)
**Status:** Stabil
**Fokus:** Integrasi Native IO, Windows Parity Final, & VM Dispatch

---

## 1. Pendahuluan

Halaman ini menandai tonggak sejarah penting dalam proyek MorphFox: **Penyelesaian Arsitektur "Bytecode as Glue"**.

Hingga titik ini, VM memiliki kemampuan komputasi (RPN) dan concurrency (MorphRoutines), namun terisolasi dari dunia luar. Dengan implementasi dispatch `OP_SYSCALL` untuk **Network** dan **Graphics**, MorphFox kini benar-benar berdaulat—mampu berbicara langsung dengan Kernel (Linux) dan WinAPI (Windows) tanpa perantara library C eksternal.

Selain itu, halaman ini mendokumentasikan penyelesaian paritas fitur maintenance (Daemon Cleaner) di Windows, memastikan siklus hidup aplikasi terjamin di kedua OS utama.

---

## 2. Bytecode as Glue: Realisasi Visi

Konsep "Bytecode as Glue" berarti instruksi RPN `OP_SYSCALL` (Code 40) bertindak sebagai antarmuka universal yang menyatukan logika tingkat tinggi dengan kapabilitas native sistem operasi.

### A. Implementasi Dispatch Table
File `executor.s` (Linux) dan `executor.asm` (Windows) kini memiliki tabel dispatch lengkap untuk intents berikut:

| Kategori | Syscall Intent ID | Fungsi Native (Assembly) | Deskripsi |
| :--- | :--- | :--- | :--- |
| **Network** | `SYS_INTENT_SOCKET` (10) | `__mf_net_socket` | Membuka socket (AF_INET/UNIX). |
| | `SYS_INTENT_CONNECT` (11) | `__mf_net_connect` | Menghubungkan ke remote host. |
| | `SYS_INTENT_SEND` (12) | `__mf_net_send` | Mengirim data raw. |
| | `SYS_INTENT_RECV` (13) | `__mf_net_recv` | Menerima data raw. |
| | `SYS_INTENT_BIND` (14) | `__mf_net_bind` | Mengikat port (Server). |
| | `SYS_INTENT_LISTEN` (15) | `__mf_net_listen` | Menunggu koneksi. |
| | `SYS_INTENT_ACCEPT` (16) | `__mf_net_accept` | Menerima koneksi masuk. |
| **Graphics** | `SYS_INTENT_WINDOW_CREATE` (20) | `__mf_window_create` | Membuat native window (X11/WinAPI). |
| | `SYS_INTENT_DRAW_PIXEL` (21) | `__mf_draw_pixel` | Menggambar pixel langsung ke buffer. |
| | `SYS_INTENT_EVENT_POLL` (22) | `__mf_window_poll` | Non-blocking event loop (Input). |

### B. Dampak Arsitektural
1.  **System Sovereignty:** Code MorphFox (.fox) kini dapat membuat Web Server atau Game Engine tanpa ketergantungan eksternal.
2.  **Platform Agnostic:** Logika IO yang sama berjalan di Linux (via Syscalls) dan Windows (via WinAPI) tanpa perubahan kode sumber di level userland.
3.  **Strict Boundary:** `OP_SYSCALL` tetap menjadi satu-satunya "Effect Boundary", menjaga determinisme di sisa pipeline eksekusi.

---

## 3. Windows Daemon Parity: Finalisasi

Komponen terakhir untuk mencapai paritas penuh antara Linux dan Windows adalah **Daemon Cleaner**.

### A. Tantangan Stack (ABI)
Implementasi `daemon_clean_sandboxes` di Windows menghadapi tantangan unik Win64 ABI. Fungsi `CreateFileA` menerima 7 argumen.
*   **Masalah:** Argumen ke-5 dan seterusnya harus diletakkan di stack *di atas* Shadow Space (32 bytes).
*   **Solusi:** Stack frame dialokasikan sebesar **56 bytes** (32 bytes Shadow + 24 bytes untuk 3 argumen tambahan).
*   **Hasil:** Stabilitas penuh. Fungsi ini tidak lagi mengkorupsi register `R12`-`R14` yang disimpan di stack (Caller-Saved).

### B. Fitur Lengkap
Daemon Windows (`daemon_cleaner.asm`) kini memiliki kemampuan setara Linux:
1.  **Snapshot Cleanup:** Menghapus file `snapshot_*` yang kedaluwarsa (TTL 5 menit).
2.  **Sandbox Cleanup:** Menghapus file `sandbox_*` yang kedaluwarsa (TTL 1 menit), mencegah kebocoran file monitoring.

---

## 4. Kesimpulan & Langkah Selanjutnya

Dengan selesainya integrasi ini, fondasi **Low-Level Runtime** dianggap lengkap (v1.0 Candidate). Fokus pengembangan selanjutnya dapat bergeser sepenuhnya ke **High-Level Features** (Compiler Optimization, Standard Library, dan MorphWeb UI Framework) di atas landasan yang kokoh ini.

> *"Memory adapts to the brain. The brain now speaks to the world."*
