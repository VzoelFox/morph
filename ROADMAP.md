# Roadmap Proyek: Vzoel Word

Dokumen ini menguraikan status saat ini dan rencana pengembangan untuk bahasa pemrograman Vzoel Word.

## Status Saat Ini: v0.3 (Stabil & Dipoles)

Inti dari Vzoel Word telah stabil dan mengalami pemolesan. Semua tes yang ada telah berhasil, memvalidasi fitur-fitur dasar bahasa, dan dependensi eksternal pada pengujian telah dihilangkan.

---

## Fitur yang Sudah Selesai (Core System)

### Fase 1: Desain & Spesifikasi (Selesai)
- **[x] Filosofi Bahasa:** Mendesain bahasa yang intuitif dan berbasis Bahasa Indonesia.
- **[x] Spesifikasi Formal:** Membuat dokumen `VZOEL_SPEC.md` yang mendefinisikan sintaks dan semantik.
- **[x] Konsep `vzoelinfinity`:** Merancang mekanisme fallback untuk menangani operasi yang mungkin gagal.

### Fase 2: Lexer & Parser (Selesai & Dipoles)
- **[x] Lexer:** Mengimplementasikan lexer untuk mengubah kode sumber menjadi token.
- **[x] Parser:** Membangun parser untuk mengubah token menjadi *Abstract Syntax Tree* (AST).
- **[x] Pengujian:** Menulis tes unit untuk memvalidasi Lexer dan Parser.
- **[x] Pemolesan:** Melakukan refactoring pada parser untuk mengurangi duplikasi kode.

### Fase 3: Interpreter & Fitur Inti (Selesai & Dipoles)
- **[x] Interpreter Dasar:** Mengimplementasikan interpreter dengan *Visitor Pattern* untuk mengeksekusi AST.
- **[x] Variabel & Lingkup:** Menciptakan sistem lingkungan (`Environment`) yang mendukung *lexical scoping*.
- **[x] Fungsi Bawaan:** Menyediakan fungsi inti (`lihat`, `panjang`, `tambah`, `potong`, `kekecil`, `pangkat`).
- **[x] Struktur Data:** Mengimplementasikan tipe data `Daftar` (List), `String`, dan `Peta` (Map).
- **[x] Fungsi Buatan Pengguna:** Mengizinkan definisi fungsi (`proses`) dengan dukungan *closure*.
- **[x] Sistem Modul:** Membangun sistem impor (`ambil...dari...`) yang terisolasi.
- **[x] Operator:** Mendukung operator aritmatika dan perbandingan dasar.
- **[x] Kontrol Alur:** Mengimplementasikan sintaks perulangan (`ulangi`).
- **[x] Penanganan Kesalahan:** Mengembangkan sistem penanganan error yang informatif.
- **[x] Pemolesan:** Memperbaiki isolasi tes dengan menghilangkan dependensi eksternal.

### Fase 4: Konkurensi (Dasar)
- **[x] Management System:** Mengimplementasikan `management` sebagai fondasi untuk proses konkurensi dan fallback.

---

## Rencana Pengembangan (Fase Berikutnya)

### v0.4: Konkurensi Lanjutan & Pustaka Standar
- **[ ] Desain Konkurensi Lanjutan:** Merancang dan mengimplementasikan model konkurensi kooperatif di atas Management System.
- **[ ] Standard Library:** Memperluas pustaka standar dengan lebih banyak fungsi untuk I/O, file, dan jaringan.

### Jangka Panjang
- **[ ] Framework Backend:** Memulai pengembangan framework backend seperti yang dicita-citakan.
- **[ ] Kompiler & Transpiler:** Melanjutkan pengembangan kompiler dan transpiler untuk meningkatkan performa dan portabilitas.

---

## Status Komponen Lainnya

- **Transpiler (ke Python):** Sebagian diimplementasikan. Mampu mentranspilasi ekspresi dasar tetapi belum mendukung semua jenis pernyataan atau fitur bahasa. (Status: **Alpha**)
- **Compiler (ke IR):** Sebagian diimplementasikan. Dapat mengkompilasi beberapa ekspresi menjadi *Intermediate Representation* (IR) tetapi belum memiliki implementasi *Virtual Machine* (VM) untuk eksekusi. (Status: **Eksperimental**)
- **Library Telegram VBase:** Pengembangan proyek ini ditunda untuk fokus pada Vzoel Word.
