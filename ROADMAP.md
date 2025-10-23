# Project Roadmap

Dokumen ini menguraikan rencana pengembangan untuk Bahasa Pemrograman "Vzoel Word".

## Fase 1: Desain & Spesifikasi (Selesai)
*   [x] Mendefinisikan filosofi bahasa berbasis Bahasa Indonesia.
*   [x] Membuat dokumen spesifikasi formal (`VZOEL_SPEC.md`).
*   [x] Mendesain konsep inti `vzoelinfinity`.

## Fase 2: Lexer & Parser (Selesai)
*   [x] Membangun Lexer dan Parser untuk menghasilkan *Abstract Syntax Tree* (AST).
*   [x] Menulis tes untuk memvalidasi Lexer dan Parser.

## Fase 3: Interpreter & Fitur Inti (Selesai)
*   [x] Membuat interpreter dasar yang dapat mengeksekusi AST.
*   [x] Mengimplementasikan variabel, lingkup (scoping), dan fungsi (`proses`).
*   [x] Mengimplementasikan Sistem Modul (`ambil...dari...`) dengan fallback.
*   [x] Mengimplementasikan Sistem Konkurensi Kooperatif (`management`).

## Fase 4: Struktur Data & Kontrol Alur (Sedang Berlangsung)
*   [ ] **(Aktif)** Implementasi Tipe Data `Daftar` (List).
*   [ ] Implementasi struktur perulangan (`untuk` atau `selama`).
*   [ ] Implementasi Tipe Data `Kamus` (Map/Dict).

## Proyek Lainnya (Ditunda)
*   **Library Telegram VBase**: Proyek ini ditunda untuk memprioritaskan pengembangan "Vzoel Word".
