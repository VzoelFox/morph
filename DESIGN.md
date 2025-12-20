# Spesifikasi Desain Bahasa Morph (Draft)

## Filosofi
- **Explicit over Implicit**: Tidak ada "magic". Kode melakukan apa yang tertulis.
- **Error as Value**: Error adalah warga kelas satu. Tidak ada exception.
- **Context-Aware**: Integrasi mendalam dengan tooling/AI via context file.
- **Bahasa Indonesia**: Menggunakan kata kunci Bahasa Indonesia untuk kemudahan pemahaman lokal.

## 1. Tata Bahasa (Grammar) & Sintaks

Menggunakan gaya blok dengan keyword `akhir` (mirip Ruby/Lua) untuk mengurangi noise visual kurung kurawal `{}`.

### Kata Kunci (Keywords)
- `fungsi`: Mendefinisikan fungsi.
- `variabel` / `var`: Mendefinisikan variabel mutable.
- `tetapan` / `const`: Mendefinisikan konstanta.
- `jika`, `tapi`, `lainnya`: Kontrol alur (if-else).
- `untuk`: Perulangan (for).
- `kembalikan`: Return value.
- `akhir`: Menutup blok kode.
- `struktur`: Mendefinisikan tipe data komposit (struct).
- `jalankan`*: Concurrency (Planned).
- `tunda`*: Defer execution (Planned).

### Contoh Hello World
```fox
fungsi utama()
    cetak("Halo, Dunia!")
akhir
```

## 2. Sistem Tipe (Type System)

Statis dan Kuat (Strongly Typed).

### Tipe Dasar
- `int`, `float`, `bool`, `string`, `byte`

### Tipe Komposit
```fox
struktur Pengguna
    nama string
    umur int
akhir

fungsi buatPengguna(n string, u int) -> Pengguna
    kembalikan Pengguna{nama: n, umur: u}
akhir
```

## 3. Penanganan Error (Error Handling)

Menggunakan pola "Error as Value". Fungsi yang bisa gagal harus mengembalikan `error` sebagai nilai terakhir.

```fox
fungsi bagi(a int, b int) -> (int, error)
    jika b == 0
        kembalikan 0, error.Baru("pembagian dengan nol")
    akhir
    kembalikan a / b, nil
akhir

fungsi utama()
    hasil, err = bagi(10, 0)
    jika err != nil
        cetak("Terjadi kesalahan: ", err)
        kembalikan
    akhir
    cetak("Hasil: ", hasil)
akhir
```

### Pemicu Error Detail
Setiap error harus menyertakan konteks (file, baris, op code) secara otomatis di level compiler/runtime (tanpa overhead runtime besar jika di-strip).

## 4. Integrasi AI & Context (Morph Unique Feature)

Compiler menghasilkan file `.morph.vz` yang berisi AST dump, Symbol Table, dan Type Info setiap kali kompilasi, memudahkan AI Agent untuk memahami struktur kode tanpa parsing ulang.

---

## Roadmap: Post-Self-Host

Fitur-fitur berikut akan didiskusikan dan diimplementasikan secara detail **setelah** bahasa mencapai tahap self-hosting (bootstrap compiler selesai).

### A. Manajemen Memori (Semi-Manual)
*Status: Proposal (Ditunda)*
Rencana awal mengadopsi pendekatan **Region-based Memory Management (Arenas)** dengan keyword `wilayah` dan `alokasi`. Detail akan difinalisasi nanti.

### B. Konkurensi (Morphroutines)
*Status: Proposal (Ditunda)*
Rencana awal menggunakan model **Green Threads** (`jalankan`) dan **Channels**. Detail akan difinalisasi nanti.
