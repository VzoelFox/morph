# MorphFox: Bahasa Pemrograman Mandiri

## Filosofi
**"Mencari Kekayaan Sendiri"**

MorphFox dibangun di atas prinsip kemandirian total. Kita tidak mewarisi "harta" dari bahasa lain (seperti libc C atau runtime Rust), melainkan mendefinisikan kekayaan (fitur & struktur) kita sendiri dari nol.

## Arsitektur
Sistem ini dirancang dengan pendekatan **Bottom-Up**:

1.  **ABI (Single Source of Truth)**
    - Kontrak murni yang mendefinisikan bagaimana sistem bekerja.
    - Lokasi: `corelib/core/*.fox`

2.  **Platform Abstraction**
    - Implementasi fisik dari kontrak ABI menggunakan Assembly murni.
    - Menjembatani perbedaan OS (Linux vs Windows) melalui Macro.
    - Lokasi: `corelib/platform/`

3.  **Morph Allocator**
    - Manajemen memori kustom yang sadar akan struktur Halaman (Page).
    - Tidak bergantung pada `malloc/free` sistem operasi.

## Status Saat Ini
Proyek ini sedang dalam tahap aktif pengembangan fondasi (Runtime & Memory Management).
