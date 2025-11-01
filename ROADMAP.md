# Roadmap Proyek: Bahasa Morph

Dokumen ini menguraikan status saat ini dan rencana pengembangan untuk bahasa pemrograman Morph.

## Status Saat Ini: v0.3 (Stabil)

Inti dari Morph telah stabil. Semua tes yang ada telah berhasil, memvalidasi fitur-fitur dasar bahasa.

---

## Fitur yang Sudah Selesai (Core System)

### v0.1: Fondasi & Interpreter Dasar
- **[x] Filosofi Bahasa:** Mendesain bahasa yang intuitif dan berbasis Bahasa Indonesia.
- **[x] Lexer & Parser:** Mengimplementasikan lexer dan parser untuk memproses kode sumber menjadi *Abstract Syntax Tree* (AST).
- **[x] Interpreter Dasar:** Mengimplementasikan interpreter dengan *Visitor Pattern* untuk mengekusi AST.
- **[x] Variabel & Lingkup:** Menciptakan sistem lingkungan (`Environment`) yang mendukung *lexical scoping*.
- **[x] Operator:** Mendukung operator aritmatika dan perbandingan dasar.
- **[x] Fungsi Bawaan Dasar:** Menyediakan fungsi inti seperti `lihat`.

### v0.2: Struktur Data & Fungsi
- **[x] Struktur Data:** Mengimplementasikan tipe data `Daftar` (List), `Peta` (Map), dan `String`.
- **[x] Fungsi Buatan Pengguna:** Mengizinkan definisi fungsi (`proses`) dengan dukungan *closure*.
- **[x] Fungsi Bawaan Lanjutan:** Menambahkan `panjang`, `tambah`, `potong`, `ke_kecil`, `ke_besar`.
- **[x] Konsep `vzoelinfinity`:** Merancang mekanisme fallback (`jika...tidak berhasil maka...`) untuk menangani operasi yang mungkin gagal.

### v0.3: Kontrol Alur & Modul
- **[x] Kontrol Alur:** Mengimplementasikan sintaks perulangan (`ulangi...sebanyak...kali`).
- **[x] Sistem Modul:** Membangun sistem impor (`ambil...dari...`) yang terisolasi.
- **[x] Peluncur Baris Perintah:** Membuat `morph.py` untuk eksekusi file `.fox`.

---

## Rencana Pengembangan (Fase Berikutnya)

### v0.4: Peningkatan Inti Bahasa
- **[ ] Penanganan Kesalahan:** Mengembangkan sistem penanganan error runtime yang lebih informatif (saat ini hanya `print`).
- **[ ] Pustaka Standar:** Memulai desain pustaka standar untuk operasi file, jaringan, dll.

### v0.5: Konkurensi
- **[ ] Desain Konkurensi:** Merancang dan mengimplementasikan model konkurensi (konsep `management`).

---

## Visi Jangka Panjang: Menuju Compiler (Lihat `roadmapnext.py`)

Ini adalah inisiatif besar untuk mengubah Morph dari bahasa interpretasi menjadi bahasa kompilasi.

### Fase 1: Transpiler & Analisis Lanjutan
- **[x] Desain Analisis Statis:** Merancang dan mengimplementasikan sistem inferensi tipe statis sebagai langkah pertama menuju kompilasi.
- **[ ] Implementasi *Transpiler*:** Membangun *transpiler* yang dapat mengubah kode Morph (.fox) menjadi kode Python yang dioptimalkan.

---

## Proyek Lainnya

- **Library Telegram VBase:** Pengembangan proyek ini ditunda untuk fokus pada Morph.
