# Halaman 4: Struktur Userland & Integrasi Lexer

**Status:** Selesai (Integrasi Lexer-Vector & Symbol Table)
**Fokus:** Transisi dari Manajemen Memori ke Logika Bahasa (Userland).

---

## 1. Pendahuluan
Sesi ini menandai tonggak penting di mana kita mulai membangun struktur data tingkat tinggi (Userland) di atas fondasi memori yang telah matang. Kita bergerak dari sekadar "mengalokasikan byte" menjadi "memahami token dan simbol".

## 2. Pematangan Memori & Platform
Sebelum masuk ke Userland, kita melakukan hardening pada layer terbawah:

### A. Implementasi `mem_free` (Soft Release)
Kita mengganti No-Op dengan logika pembebasan memori yang aman namun sederhana:
- **Validasi:** Cek Magic Number (`0xDEADBEEF`) di Page Header.
- **Marking:** Menegasikan ukuran blok (`Size = -Size`) untuk menandainya sebagai bebas.
- **Safety:** Mencegah *Double Free* dengan pengecekan apakah ukuran sudah negatif.

### B. Konsistensi Windows (Dynamic Page Size)
Kita menghapus *hardcoded value* `4096` di Windows.
- **Solusi:** Runtime sekarang memanggil `GetSystemInfo` saat startup untuk mendeteksi ukuran halaman sistem secara dinamis, menyamakan perilaku dengan Linux (yang menggunakan AUXV).

### C. Daemon Cleaner (Ops)
Kita memperkenalkan `scripts/morph_cleaner.sh`, sebuah daemon eksternal untuk menjaga *hygiene* lingkungan pengembangan (membersihkan snapshot/sandbox lama dan memantau penggunaan RAM).

## 3. Struktur Data Inti (SSOT)
Kita mendefinisikan Single Source of Truth baru di `corelib/core/structures.fox` dan `corelib/core/token.fox`.

### A. String (Fat Pointer)
String di MorphFox bukanlah C-String (null-terminated), melainkan Slice:
- **Struct:** `[Length: i64] [Data Ptr: ptr]`
- **Hashing:** Algoritma **FNV-1a (64-bit)** diimplementasikan di assembly untuk kinerja tinggi.
- **Equality:** Cek panjang dulu, baru *byte-per-byte comparison*.

### B. Symbol Table (Hash Map)
Wadah untuk menyimpan variabel dan fungsi.
- **Strategi:** Hash Map dengan resolusi tabrakan **Chaining** (Linked List).
- **Node:** Disimpan di Memory Pool untuk efisiensi. Struktur: `[Next] [Hash] [Key] [Value]`.
- **Zero-Copy Keys:** Symbol Table menyimpan pointer ke struktur String, meminimalkan penyalinan data yang tidak perlu.

## 4. Infrastruktur Lexer
Kita membangun "mata" pertama bagi compiler:
- **Token Struct:** `[Type] [Value] [Line] [Col]`.
- **Lexer State:** Menyimpan posisi kursor pada buffer input.
- **Kemampuan:**
    - Skip Whitespace.
    - Parse Integer (Desimal).
    - Parse Identifier (Alphanumeric).
    - Parse Operator Tunggal.

## 5. Integrasi: Loading Phase
Pencapaian terbesar sesi ini adalah integrasi **Lexer -> Vector**.
- **Alur Kerja:** Lexer membaca input mentah -> Menghasilkan pointer Token -> Pointer disimpan ke dalam Dynamic Vector.
- **Hasil:** Sebuah `Vector<Token*>` yang siap diproses oleh Parser (tahap selanjutnya).

---

**Kesimpulan:**
Dengan selesainya Halaman 4, kita memiliki "bahan baku" yang lengkap: Memori yang aman, Struktur Data (String/Symbol/Vector), dan Token Stream. Langkah selanjutnya adalah menyusun token-token ini menjadi logika eksekusi (Parser/RPN).
