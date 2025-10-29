# Roadmap Proyek: Vzoel Word

Dokumen ini menguraikan status saat ini dan rencana pengembangan untuk bahasa pemrograman Vzoel Word.

## Status Saat Ini: v0.3 (Stabil)

Fitur-fitur inti bahasa, termasuk struktur data dan kontrol alur dasar, telah stabil. Semua tes yang ada berhasil, memvalidasi fungsionalitas yang ada.

---

## Fitur yang Sudah Selesai

### Arsitektur Inti
- **[x] Filosofi & Desain Bahasa:** Mendesain bahasa yang intuitif dan berbasis Bahasa Indonesia, termasuk konsep `vzoelinfinity`.
- **[x] Lexer:** Mengubah kode sumber menjadi token.
- **[x] Parser:** Mengubah token menjadi *Abstract Syntax Tree* (AST).
- **[x] Interpreter:** Mengeksekusi AST menggunakan *Visitor Pattern*.
- **[x] Variabel & Lingkup:** Mendukung *lexical scoping* melalui sistem `Environment`.
- **[x] Sistem Modul:** Membangun sistem impor (`ambil...dari...`) yang terisolasi dan tangguh.
- **[x] Pengujian:** Menulis tes unit untuk memvalidasi semua komponen inti.

### Fitur Bahasa & Sintaks
- **[x] Operator:** Mendukung operator aritmatika dan perbandingan dasar.
- **[x] Fungsi Buatan Pengguna (`proses`):** Mendukung definisi fungsi dengan argumen dan *closure*.
- **[x] Kontrol Alur (`ulangi`):** Mengimplementasikan sintaks perulangan `ulangi ... sebanyak ... kali`.

### Tipe Data & Fungsi Bawaan
- **[x] Tipe Data Dasar:**
  - **[x] `Daftar` (List):** Mendukung literal `[...]` dan akses indeks.
  - **[x] `Peta` (Map):** Mendukung literal `peta{}` dengan kunci string/angka.
  - **[x] `String`:** Mendukung literal `"..."` dan operasi dasar.
- **[x] Fungsi Bawaan:**
  - `lihat()`: Untuk output.
  - `panjang()`: Untuk mengukur panjang `Daftar`.
  - `tambah()`: Untuk menambahkan elemen ke `Daftar`.
  - `potong()`: Untuk memotong `String`.

---

## Rencana Pengembangan (Fase Berikutnya)

### v0.4: Peningkatan Lanjutan
- **[ ] Penanganan Kesalahan:** Mengembangkan sistem penanganan error yang lebih informatif di seluruh interpreter.
- **[ ] Perluasan Pustaka Standar:** Menambahkan lebih banyak fungsi bawaan untuk `Daftar`, `Peta`, dan `String`.

### v0.5: Konkurensi
- **[ ] Desain Konkurensi:** Merancang dan mengimplementasikan model konkurensi kooperatif (konsep `management`).

### Jangka Panjang
- **[ ] Standard Library:** Memperluas pustaka standar secara signifikan.
- **[ ] Framework Backend:** Memulai pengembangan framework backend seperti yang dicita-citakan.

---

## Proyek Lainnya

- **Library Telegram VBase:** Pengembangan proyek ini ditunda untuk fokus pada Vzoel Word.
