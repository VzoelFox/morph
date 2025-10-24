# Roadmap Proyek: Vzoel Word

Dokumen ini menguraikan status saat ini dan rencana pengembangan untuk bahasa pemrograman Vzoel Word.

## Status Saat Ini: v0.2 (Stabil)

Inti dari Vzoel Word telah stabil. Semua tes yang ada telah berhasil, memvalidasi fitur-fitur dasar bahasa.

---

## Fitur yang Sudah Selesai (Core System)

### Fase 1: Desain & Spesifikasi
- **[x] Filosofi Bahasa:** Mendesain bahasa yang intuitif dan berbasis Bahasa Indonesia.
- **[x] Spesifikasi Formal:** Membuat dokumen `VZOEL_SPEC.md` yang mendefinisikan sintaks dan semantik.
- **[x] Konsep `vzoelinfinity`:** Merancang mekanisme fallback untuk menangani operasi yang mungkin gagal.

### Fase 2: Lexer & Parser
- **[x] Lexer:** Mengimplementasikan lexer untuk mengubah kode sumber menjadi token.
- **[x] Parser:** Membangun parser untuk mengubah token menjadi *Abstract Syntax Tree* (AST).
- **[x] Pengujian:** Menulis tes unit untuk memvalidasi Lexer dan Parser.

### Fase 3: Interpreter & Fitur Inti
- **[x] Interpreter Dasar:** Mengimplementasikan interpreter dengan *Visitor Pattern* untuk mengeksekusi AST.
- **[x] Variabel & Lingkup:** Menciptakan sistem lingkungan (`Environment`) yang mendukung *lexical scoping*.
- **[x] Fungsi Bawaan:** Menyediakan fungsi inti seperti `lihat`, `panjang`, dan `tambah`.
- **[x] Struktur Data:** Mengimplementasikan tipe data `Daftar` (List).
- **[x] Fungsi Buatan Pengguna:** Mengizinkan definisi fungsi (`proses`) dengan dukungan *closure*.
- **[x] Sistem Modul:** Membangun sistem impor (`ambil...dari...`) yang terisolasi.
- **[x] Operator:** Mendukung operator aritmatika dan perbandingan dasar.

---

## Rencana Pengembangan (Fase Berikutnya)

### v0.3: Peningkatan Struktur Data & Kontrol Alur
- **[x] Tipe Data Lanjutan:** Menambahkan dukungan untuk `String` dan `Map` (kamus).
- **[ ] Operasi String:** Membangun fungsi bawaan untuk manipulasi string (misalnya, penggabungan, pemotongan).
- **[ ] Tipe Data Lanjutan:** Menambahkan dukungan untuk `String` dan `Map` (kamus).
- **[ ] Operasi String:** Membangun fungsi bawaan untuk manipulasi string.
- **[ ] Kontrol Alur:** Mengimplementasikan sintaks perulangan (misalnya, `ulangi`).
- **[ ] Penanganan Kesalahan:** Mengembangkan sistem penanganan error yang lebih informatif.

### v0.4: Konkurensi
- **[ ] Desain Konkurensi:** Merancang dan mengimplementasikan model konkurensi kooperatif (konsep `management`).

### Jangka Panjang
- **[ ] Standard Library:** Memperluas pustaka standar dengan lebih banyak fungsi.
- **[ ] Framework Backend:** Memulai pengembangan framework backend seperti yang dicita-citakan.

---

## Proyek Lainnya

- **Library Telegram VBase:** Pengembangan proyek ini ditunda untuk fokus pada Vzoel Word.
