# Spesifikasi Bahasa Vzoel Word v0.1

Dokumen ini mendefinisikan sintaks dan semantik dasar untuk versi 0.1 dari bahasa pemrograman Vzoel Word.

## 1. Komentar

Komentar adalah teks yang diabaikan oleh interpreter. Komentar dimulai dengan tanda `#`. Apa pun dari `#` hingga akhir baris dianggap sebagai komentar.

```vz
# Ini adalah komentar.
```

## 2. Tipe Data Dasar

Vzoel Word v0.1 memiliki tiga tipe data dasar:

*   **Number:** Angka, bisa integer atau float (misal: `10`, `3.14`).
*   **String:** Teks, diapit oleh tanda kutip ganda (misal: `"Halo Dunia"`).
*   **Boolean:** Nilai kebenaran, yaitu `true` atau `false`.

## 3. Variabel

Variabel digunakan untuk menyimpan data. Variabel dideklarasikan menggunakan kata kunci `set`, diikuti oleh nama variabel, tanda `=`, dan nilainya.

```vz
set nama = "Vzoel"
set umur = 1
set aktif = true
```

## 4. Fungsi Bawaan (Built-in Functions)

### `show()`

Fungsi `show()` digunakan untuk menampilkan output ke konsol. Fungsi ini dapat menerima satu argumen dari tipe data apa pun.

```vz
# Contoh penggunaan:
set pesan = "Halo dari Vzoel Word!"
show(pesan)

show("Pesan langsung tanpa variabel.")
show(123)
```
