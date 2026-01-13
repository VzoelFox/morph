# Halaman 10: Native Daemon & Windows Parity
**Versi:** 1.0 (2026-01-10)
**Status:** Stabil
**Fokus:** Infrastruktur Runtime Mandiri & Paritas Lintas Platform

---

## 1. Pendahuluan

Halaman ini mendokumentasikan dua pencapaian besar dalam evolusi MorphFox Runtime menuju kemandirian sistem (System Sovereignty):
1.  **Native Daemon Cleaner:** Transisi dari pengelolaan resource berbasis Bash script eksternal menjadi proses assembly murni yang terintegrasi.
2.  **Windows Parity:** Pencapaian kesetaraan fitur penuh antara Linux dan Windows, khususnya pada komponen kritis seperti Green Threads (MorphRoutines) dan Type System.

Kedua pencapaian ini menegaskan visi "Mencari Kekayaan Sendiri" dengan mengurangi ketergantungan pada tool eksternal dan memastikan determinisme di berbagai sistem operasi.

---

## 2. Native Daemon Cleaner (Linux)

Sebelumnya, kebersihan memori (snapshot/sandbox cleanup) ditangani oleh `scripts/morph_cleaner.sh`. Kini, fungsi tersebut telah ditulis ulang sepenuhnya dalam Assembly x86_64 murni.

### A. Arsitektur
Daemon baru terdiri dari dua komponen utama:
*   **`daemon_cleaner.s`**: Core logic yang menangani pemindaian direktori, perhitungan TTL (Time-To-Live), dan eksekusi syscall.
*   **`morph_daemon_main.s`**: Entry point yang menangani manajemen proses (start/stop/status), forking, dan PID file.

### B. Mekanisme Kerja
1.  **Monitoring Loop:** Daemon berjalan di background, bangun setiap 2 detik (configurable).
2.  **Snapshot Cleanup:** Memindai `/tmp/morph_swap_monitor/`. Jika file `snapshot_<PID>` lebih tua dari 5 menit, file dihapus.
3.  **Sandbox Cleanup:** Jika file `sandbox_<PID>` lebih tua dari 1 menit (terindikasi crash/hang), daemon mengirim sinyal `SIGUSR2` ke proses tersebut untuk memicu **System Reset** internal, lalu menghapus file tracking.
4.  **Zero Dependencies:** Tidak lagi membutuhkan `bash`, `grep`, atau `awk`. Hanya menggunakan Linux Syscalls langsung.

### C. Keunggulan
*   **Kecepatan:** Startup <1ms vs ~50ms (Bash).
*   **Efisiensi:** Memory footprint ~200KB vs ~10MB (Bash).
*   **Kontrol:** Memungkinkan interaksi sinyal presisi yang sulit dilakukan di shell script.

---

## 3. Windows Parity (Kesetaraan Platform)

Hingga versi v0.6, Windows tertinggal dalam fitur concurrency. Pada v0.7, kesenjangan ini ditutup sepenuhnya.

### A. MorphRoutines di Windows
File `corelib/platform/x86_64/asm_win/scheduler.asm` telah dibuat, mereplikasi logika `scheduler.s` Linux dengan adaptasi arsitektur:
*   **ABI Compliance:** Menggunakan Microsoft x64 Calling Convention (RCX, RDX, R8, R9) dan alokasi **Shadow Space** (32 bytes) yang wajib.
*   **Stack Management:** Menggunakan `stack_new` (diseragamkan dari `stack_create`) untuk manajemen memori stack routine.

### B. Type System
File `type.asm` ditambahkan untuk mendukung penciptaan struktur data dinamis, memungkinkan compiler Windows menghasilkan bytecode yang identik dengan Linux.

### C. Build System
Script `scripts/build_windows.bat` ditambahkan untuk memungkinkan kompilasi one-click di environment Windows native, serta mendukung cross-compilation testing via Wine di CI/CD.

---

## 4. Struktur File Baru

Perubahan ini menambah file-file berikut ke dalam Single Source of Truth (SSOT):

| Kategori | File | Deskripsi |
|----------|------|-----------|
| **Daemon** | `corelib/platform/x86_64/asm/daemon_cleaner.s` | Logika pembersihan resource (Linux). |
| | `corelib/platform/x86_64/asm/morph_daemon_main.s` | Manajemen proses daemon. |
| **Windows** | `corelib/platform/x86_64/asm_win/scheduler.asm` | Porting Green Thread Scheduler. |
| | `corelib/platform/x86_64/asm_win/type.asm` | Porting Type System helpers. |

---

## 5. Kesimpulan

Dengan selesainya Native Daemon dan Windows Parity, MorphFox Runtime kini memiliki fondasi yang kokoh dan agnostik-platform. Langkah selanjutnya adalah memperluas kapabilitas Daemon ke Windows dan memperkaya fitur monitoring memori (Memory Pressure Handling).
