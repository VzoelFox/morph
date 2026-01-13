# Halaman 9: MorphRoutines & Cooperative Concurrency

**Status:** Stabil (Linux/Windows Parity Achieved)
**Versi:** MorphFox Runtime v0.7

Dokumen ini merangkum implementasi **MorphRoutines**, sistem *Green Threads* (User-Level Threads) yang memungkinkan eksekusi konkuren ringan di dalam MorphFox VM.

## 1. Filosofi "Workers move to Units"

Berbeda dengan model threading tradisional di mana kode "berjalan di atas thread", MorphFox mengadopsi pendekatan di mana **Worker (Executor)** mengunjungi **Unit (Routine)**.
- **MorphRoutine:** Struktur data pasif yang menyimpan State (Stack Pointer, Status, Fragment Ptr).
- **Executor:** Mesin loop tunggal yang mengambil Routine dari antrian, menjalankannya sampai *Yield*, lalu menyimpannya kembali.

## 2. Arsitektur Scheduler

Scheduler diimplementasikan di level Assembly (`scheduler.s`) dengan strategi **Round-Robin** sederhana.

### Struktur Data (SSOT)
- **Ready Queue:** Linked List dari Routine yang berstatus `READY`.
- **Current Routine:** Pointer ke Routine yang sedang dieksekusi (`RUNNING`).

### Lifecycle Routine
1.  **Spawn:** Routine baru dibuat, Stack dialokasikan.
2.  **Bootstrap:** Stack dipersiapkan (fake stack frame) seolah-olah routine baru saja melakukan *Context Switch*, dengan `RIP` mengarah ke **Trampoline** (`executor_exec_label`).
3.  **Run:** Executor menjalankan instruksi RPN.
4.  **Yield:** Routine sukarela menyerahkan CPU (`OP_YIELD`), status disimpan, dan routine dipindahkan ke ekor antrian.
5.  **Exit:** (Planned) Routine selesai dan dibersihkan.

## 3. Instruksi Baru (Concurrency Opcodes)

Dua instruksi RPN ditambahkan untuk mengontrol konkurensi:

- **`OP_SPAWN` (Code 50):**
    - Membuat Routine baru.
    - Operand: Offset/Label entry point.
    - Routine baru mewarisi *Code Pointer* dari parent, tapi memiliki *Data Stack* dan *Call Stack* sendiri.
- **`OP_YIELD` (Code 51):**
    - Memicu *Context Switch*.
    - Menyimpan register callee-saved (`RBX, RBP, R12-R15`) ke stack routine saat ini.
    - Memuat stack routine berikutnya dari antrian.

## 4. Pencapaian Teknis

- **Context Switching Assembly:** Implementasi manual penyimpanan/pemulihan register CPU untuk berpindah antar stack tanpa intervensi OS Kernel.
- **Trampoline Bootstrapping:** Teknik memalsukan *Return Address* di stack baru agar routine yang belum pernah jalan bisa "kembali" (return) ke fungsi inisialisasi Executor.
- **Interleaved Execution:** Diverifikasi melalui `test_concurrency.s` di mana Main Routine dan Spawned Routine saling bergantian mencetak output (A1 -> B1 -> A2 -> B2).

---

## 5. Technical Debt & Langkah Selanjutnya

Meskipun fitur dasar berjalan di Linux, terdapat beberapa hutang teknis yang dicatat untuk iterasi berikutnya:

### A. Windows Porting Gap ✅ **RESOLVED (2026-01-10)**
- **Status:** ~~Kritis untuk Paritas~~ → **COMPLETED**
- **Deskripsi:** `scheduler.s` (Linux/GAS) telah di-port ke `scheduler.asm` (Windows/NASM) dengan 273 baris kode.
- **Tindakan Completed:**
  - Port `scheduler.s` → `scheduler.asm` (273 lines)
  - Port `type.s` → `type.asm` (132 lines)
  - Fix naming inconsistency: standardisasi `stack_new` di seluruh codebase (Linux & Windows)
  - Verifikasi syntax NASM: All files compile cleanly
  - Created `build_windows.bat` script untuk Windows build automation
- **Files Added:**
  - `corelib/platform/x86_64/asm_win/scheduler.asm`
  - `corelib/platform/x86_64/asm_win/type.asm`
  - `scripts/build_windows.bat`
- **Files Modified:**
  - `corelib/platform/x86_64/asm_win/stack.asm` (renamed stack_create → stack_new)
  - `corelib/platform/x86_64/asm_win/executor.asm` (renamed stack_create → stack_new)
  - `corelib/platform/x86_64/asm_win/test_infinite_registers.asm` (renamed stack_create → stack_new)

### B. Graceful Routine Exit
- **Status:** Major.
- **Deskripsi:** Saat ini, routine menggunakan `OP_EXIT` yang memanggil syscall `exit`, mematikan **seluruh proses OS**.
- **Target:** Routine harus memanggil mekanisme internal (`scheduler_exit_current`) untuk hanya menghentikan dirinya sendiri, membersihkan resource, dan membiarkan Scheduler memilih routine lain.

### C. Resource Cleanup (GC)
- **Status:** Moderate.
- **Deskripsi:** Stack yang dialokasikan untuk routine yang sudah selesai belum dibebaskan (`mem_free` belum dipanggil otomatis).
- **Risiko:** Memory Leak pada aplikasi long-running yang sering melakukan spawn.

### D. Synchronization Primitives
- **Status:** Future Work.
- **Deskripsi:** Belum ada mekanisme `WAIT`, `NOTIFY`, atau `LOCK` untuk koordinasi antar routine. Saat ini hanya mengandalkan *Cooperative Yield*.
