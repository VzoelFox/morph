# Halaman 15: WASM Parity & Virtual Domain
**Versi:** 1.0 (2026-02-15)
**Status:** Stabil
**Fokus:** Penyelarasan WebAssembly dengan Native v1.1 & Arsitektur Virtualisasi

---

## 1. Pendahuluan

Halaman ini mendokumentasikan upgrade besar pada platform WebAssembly (WASM) untuk mencapai paritas fitur dengan Core Native (Linux/Windows) v1.1. Langkah ini krusial sebelum membekukan repository, memastikan bahwa kode yang dikompilasi di Native dapat berjalan mulus di Browser.

## 2. WASM Runtime v1.1

### A. Memory Allocator
`runtime.wat` telah ditulis ulang untuk mendukung struktur memori standar MorphFox:
*   **Header 8-Byte:** Menyimpan ukuran alokasi, kompatibel dengan logika pointer Native.
*   **Alignment 16-Byte:** Menjamin akses memori yang efisien dan aman.
*   **Bump Pointer:** Strategi alokasi linear sederhana namun cepat.

### B. Executor Hardening
`executor.wat` kini memiliki fitur keamanan setara Native:
*   **Stack Safety:** Fungsi `stack_push` dan `stack_pop` menggunakan instruksi `unreachable` untuk menjebak (trap) overflow/underflow, mencegah korupsi memori diam-diam.
*   **OP_HINT Support:** Handler untuk instruksi hint (Code 99) telah ditambahkan sebagai *Safe NOP*.

## 3. Virtual Domain (JS Host)

Untuk mendukung visi "Virtual Domain" (lingkungan terisolasi mirip `.venv`), `loader.js` telah dimutakhirkan.

### Offline Loading Strategy
Alih-alih mengandalkan kompilasi di sisi klien (JS Compiler), loader kini mendukung pemuatan langsung **Binary RPN (`.bin`)**.
1.  **Compile di Native:** Gunakan `tools/dump_rpn` untuk mengubah `.fox` menjadi `.bin`.
2.  **Load di Web:** `MorphLoader` mengambil `.bin` via fetch, menuliskannya ke memori WASM, dan langsung mengeksekusinya.

### Syscall Bridge
Sistem syscall menghubungkan dunia "Virtual" RPN dengan API Browser nyata:
*   `INTENT_WRITE` -> `console.log`
*   `INTENT_DOM_CREATE` -> `document.createElement`
*   `INTENT_DOM_APPEND` -> `appendChild`

## 4. Status Paritas Cross-Platform

| Fitur | Linux (Native) | Windows (Native) | Web (WASM) | Status |
| :--- | :--- | :--- | :--- | :--- |
| **Allocator Header** | 48 Bytes (Page) | 48 Bytes (Page) | 8 Bytes (Block) | Compatible |
| **Stack Safety** | Panic (102) | Panic (102) | Trap (Unreachable) | **Parity** |
| **Global Vars** | Full | Full | Stub/Host | Partial |
| **Offline Load** | ELF/Exec | EXE | Binary RPN Loader | **Resolved** |

---

*Dokumen ini menandai kesiapan ekosistem MorphFox untuk deployment hybrid.*
