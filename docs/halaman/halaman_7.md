# Halaman 7: Runtime Intelligence, Daemon Integration & Math Safety

Dokumen ini merangkum evolusi besar pada infrastruktur runtime MorphFox, fokus pada integrasi sistem (OS), keamanan memori tingkat lanjut, dan aritmatika aman.

## 1. Allocator Intelligence (V2)

Morph Allocator telah ditingkatkan untuk mendukung fitur-fitur "pintar" yang memungkinkan manajemen memori jangka panjang.

### Header Halaman (48 Bytes)
Struktur header halaman diperluas dari 32 byte menjadi 48 byte untuk menyimpan metadata krusial:
- **Timestamp (Last Access Time):** Disimpan di offset `0x10`. Memungkinkan algoritma LRU (Least Recently Used) untuk membuang halaman yang jarang dipakai.
- **Page Size:** Disimpan di offset `0x18`. Memungkinkan `munmap` yang aman untuk halaman berukuran variabel (Big Alloc).
- **Magic Number:** Digeser ke offset `0x20` untuk validasi integritas ("VZOELFOX").

### System Reset (Soft Restart)
Fitur `mem_reset` memungkinkan runtime untuk "me-reset" diri sendiri tanpa mematikan proses OS.
- **Mekanisme:** Menelusuri linked list halaman memori dari belakang, melakukan `munmap` (dealokasi ke OS) pada semua halaman kecuali halaman pertama.
- **Manfaat:** Memungkinkan penggunaan kembali proses (Process Reuse) untuk sandbox yang berumur pendek, mengurangi overhead `execve`.

## 2. Daemon Integration

Runtime kini memiliki integrasi erat dengan **Daemon Cleaner** eksternal (`morph_cleaner.sh`).

### Handshake & Monitoring
Saat startup, Runtime menjalankan `__mf_runtime_init` yang melakukan:
1.  Membuat direktori monitoring (`/tmp/morph_swap_monitor`).
2.  Membuat file "Heartbeat": `sandbox_<PID>`.
3.  Menulis timestamp ke file tersebut.

Ini memungkinkan Daemon Cleaner memantau umur dan status proses secara real-time.

### Signal Handling (Linux)
Runtime merespons sinyal POSIX untuk kontrol eksternal:
- **SIGUSR1 (Snapshot):** Memicu dump memori ke disk (`dump_<PID>.bin`) dan update timestamp.
- **SIGUSR2 (Reset):** Memicu `mem_reset` untuk membersihkan memori kembali ke kondisi awal.

## 3. Math Safety (Overflow Protection)

Untuk mencegah bug aritmatika diam-diam, MorphFox mengimplementasikan **Checked Arithmetic** di level assembly.

### Safe Builtins
Operasi berikut mengembalikan status error (RDX=1) jika terjadi overflow atau error:
- `__mf_add_checked`: Penjumlahan dengan deteksi Overflow.
- `__mf_sub_checked`: Pengurangan dengan deteksi Overflow.
- `__mf_mul_checked`: Perkalian dengan deteksi Overflow.
- `__mf_div_checked`: Pembagian dengan deteksi Div-By-Zero dan Overflow.

### Complex Math
Fungsi matematika integer lanjutan:
- `pow(base, exp)`: Pangkat (Exponentiation by squaring).
- `sqrt(val)`: Akar kuadrat (Binary Search).
- `abs(val)`: Nilai mutlak.

## 4. Control Flow Infrastructure

Persiapan infrastruktur untuk Parser masa depan:
- **SwitchContext:** Struktur stack-based untuk menangani `Switch Case` bertingkat dan optimasi Jump Table.
- **LoopContext:** Menyimpan label `Start` (continue) dan `End` (break).
- **Circuit Breaker:** Global `vm_cycle_budget` dan opcode `OP_CYCLE` untuk mencegah infinite loop (Turing Halting Problem mitigation).
