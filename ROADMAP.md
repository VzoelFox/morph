# Project Roadmap

Dokumen ini menguraikan rencana pengembangan untuk Bahasa Pemrograman "Vzoel Word".

## Fase 1: Desain & Spesifikasi (Selesai)

*   [x] Mendefinisikan filosofi bahasa berbasis Bahasa Indonesia.
*   [x] Membuat dokumen spesifikasi formal (`VZOEL_SPEC.md`) untuk v0.2.
*   [x] Mendesain konsep inti `vzoelinfinity`.
*   [x] Membuat contoh kode (`hello.vz`, `infinity_demo.vz`).

## Fase 2: Lexer & Parser (Selesai)
## Fase 2: Lexer & Parser (Sedang Berlangsung)

*   [x] Menginisialisasi proyek interpreter Python.
*   [x] Membangun **Lexer** untuk mengubah kode `.vz` menjadi token.
*   [x] Menulis tes untuk memvalidasi Lexer.
*   [x] Membangun **Parser** untuk mengubah token menjadi *Abstract Syntax Tree* (AST).

## Fase 3: Interpreter (Sedang Berlangsung)

*   [x] Membuat *interpreter* dasar yang dapat mengeksekusi AST.
*   [x] Mengimplementasikan logika untuk variabel dan fungsi dasar (`lihat`).
*   [ ] **(Aktif)** Mengimplementasikan Sistem Modul Lanjutan (`ambil...dari...`) dengan fallback.
*   [ ] **(Aktif)** Membangun **Parser** untuk mengubah token menjadi *Abstract Syntax Tree* (AST).

## Fase 3: Interpreter (Belum Dimulai)

*   [ ] Membuat *interpreter* dasar yang dapat mengeksekusi AST.
*   [ ] Mengimplementasikan logika untuk variabel dan fungsi dasar.

## Proyek Lainnya (Ditunda)

*   **Library Telegram VBase**: Proyek ini ditunda untuk memprioritaskan pengembangan "Vzoel Word".
