# MorphFox

## Filosofi Pengembangan

Proyek ini dibangun dengan pendekatan "Mencari Kekayaan Sendiri", bukan sekadar mewarisi. Kita membangun dari bawah ke atas dengan urutan yang ketat:

1.  **ABI (Application Binary Interface) sebagai SSOT**:
    Kita mendefinisikan kontrak antarmuka biner secara konseptual terlebih dahulu. Ini adalah Single Source of Truth (SSOT). Tidak ada kode, hanya kesepakatan murni tentang bagaimana sistem bekerja (register, syscalls, memory layout).

2.  **Pemetaan Platform (Platform Mapping)**:
    Setiap arsitektur target (x86_64, ARM64, WASM) harus memetakan dirinya ke SSOT ABI tersebut. Ini adalah lapisan implementasi fisik dari kontrak abstrak.

3.  **Compiler & Builtins**:
    Compiler dibangun di atas fondasi ABI yang sudah stabil. Builtins bahasa (seperti tipe data primitif) dibuat di sini.

4.  **High Level Language**:
    Bahasa tingkat tinggi adalah lapisan terakhir yang memberikan sintaks manis di atas infrastruktur yang kokoh.

## Struktur Proyek

```
morphfox/
├── corelib/
│   ├── core/
│   │   ├── prelude.fox          ← SSOT Utama (Kontrak ABI Deskriptif)
│   │   ├── builtins.fox
│   │   └── types.fox
│   │
│   └── platform/                ← Implementasi Kontrak ABI per Arsitektur
│       ├── x86_64/
│       ├── arm64/
│       └── wasm32/
│
└── std/
```

## Status Saat Ini
Tahap: **Pemetaan Platform & Definisi ABI**
Fokus: Membuat kontrak inti (Memori, I/O, Proses) secara deskriptif.

---
*'Dibuat oleh' Vzoel Fox*
