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
- `jalankan`: Concurrency (mirip `go`).
- `tunda`: Defer execution (mirip `defer`).

### Contoh Hello World
```morph
fungsi utama()
    cetak("Halo, Dunia!")
akhir
```

## 2. Sistem Tipe (Type System)

Statis dan Kuat (Strongly Typed).

### Tipe Dasar
- `int`, `float`, `bool`, `string`, `byte`

### Tipe Komposit
```morph
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

```morph
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

## 4. Manajemen Memori (Semi-Manual)

Mengadopsi pendekatan **Region-based Memory Management (Arenas)** dan **Explicit Allocation** untuk kontrol penuh.

- **Stack Allocation**: Default untuk variabel lokal.
- **Heap Allocation**: Eksplisit dengan `alokasi`.
- **Pembersihan**:
    - Otomatis untuk stack.
    - Semi-otomatis menggunakan `wilayah` (region/arena). Semua yang dialokasikan dalam wilayah akan dibersihkan saat wilayah berakhir.

```morph
fungsi prosesData()
    // Membuat wilayah memori baru
    wilayah w
        data = alokasi(w, BesarData) // dialokasikan di heap dalam wilayah w
        // ... proses data ...
    akhir
    // 'data' otomatis dibebaskan di sini saat 'akhir' dicapai
akhir
```

## 5. Konkurensi (Concurrency)

Menggunakan model **Green Threads** (mirip Goroutine) dengan komunikasi via **Saluran** (Channel).

```morph
fungsi pekerja(id int, ch saluran<int>)
    cetak("Pekerja #{id} mulai")
    ch <- id * 2 // Kirim data ke saluran
akhir

fungsi utama()
    ch = buat_saluran<int>()

    untuk i = 0; i < 5; i++
        jalankan pekerja(i, ch)
    akhir

    untuk i = 0; i < 5; i++
        hasil = <-ch
        cetak("Dapat: ", hasil)
    akhir
akhir
```

## 6. Integrasi AI & Context (Morph Unique Feature)

Compiler menghasilkan file `.morph.vz` yang berisi AST dump, Symbol Table, dan Type Info setiap kali kompilasi, memudahkan AI Agent untuk memahami struktur kode tanpa parsing ulang.

---

**Pertanyaan untuk Diskusi:**
1. Apakah sintaks blok (`akhir`) sudah sesuai keinginan atau lebih suka `{}`?
2. Apakah pendekatan `wilayah` (Arena) untuk memori semi-manual cocok dengan visi Anda?
3. Apakah penamaan keyword bahasa Indonesia ini sudah pas?
