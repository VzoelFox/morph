# Spesifikasi Bahasa Vzoel Word v0.2

Dokumen ini mendefinisikan sintaks dan filosofi untuk Vzoel Word v0.2.

## 1. Filosofi

Vzoel Word adalah bahasa pemrograman yang dirancang dengan prinsip-prinsip berikut:

*   **Ekspresif dalam Bahasa Indonesia:** Sintaks inti menggunakan kata-kata dari Bahasa Indonesia untuk membuat kode lebih mudah dibaca dan intuitif bagi penutur asli.
*   **Ketangguhan (Resilience):** Terinspirasi oleh *structured concurrency*, Vzoel Word memiliki mekanisme bawaan untuk menangani kegagalan dan menjalankan alur alternatif secara elegan melalui konsep `vzoelinfinity`.
*   **Berorientasi pada Proses:** Logika dibungkus dalam `proses` yang dapat berjalan secara independen, berkomunikasi, dan dikelola oleh runtime.

## 2. Sintaks Dasar

*   **Komentar:** Dimulai dengan `#`.
    ```vz
    # Ini adalah komentar
    ```

*   **Tipe Data:** `String` ("teks"), `Number` (123, 3.14), `Boolean` (`benar`, `salah`), `TidakAda` (`tidak ada`).

## 3. Kata Kunci (Keywords)

### Variabel & Output
*   `atur`: Mendeklarasikan atau menetapkan nilai ke variabel.
*   `lihat`: Menampilkan nilai ke output standar.

    ```vz
    atur pesan = "Halo Dunia"
    lihat(pesan)
    ```

### Struktur Kontrol
*   `jika ... maka ...`: Blok kondisional dasar.
*   `jika ... tidak ... maka ...`: Struktur kondisional dengan fallback (lihat `vzoelinfinity`).

    ```vz
    atur umur = 20
    jika umur > 17 maka {
        lihat("Dewasa")
    }
    ```

### Proses & Fungsi
*   `proses`: Mendefinisikan blok kode yang dapat dieksekusi, mirip dengan fungsi.
*   `kembali`: Mengembalikan nilai dari sebuah `proses`.
*   `jalankan`: Mengeksekusi sebuah `proses`.

    ```vz
    proses sapa(nama) {
        kembali "Halo, " + nama
    }

    atur sapaan = jalankan sapa("Vzoel")
    lihat(sapaan)
    ```

### Penanganan Proses & Konkurensi
*   `tunggu`: Menjeda eksekusi hingga sebuah `proses` selesai.
*   `pemicu`: Mendefinisikan sebuah event atau trigger.
*   `jika pemicu ... terjadi maka ...`: Menangani event.
*   `harus`: Menegaskan bahwa suatu kondisi harus benar, jika tidak, proses dianggap gagal.

## 4. Konsep Inti: `vzoelinfinity`

`vzoelinfinity` adalah implementasi dari *structured concurrency* dengan *auto-fallback*. Ini adalah jantung dari ketangguhan Vzoel Word. Sintaks utamanya adalah `jika ... tidak ... maka ...`.

**Ide Utama:**
Daripada menangani *error* secara manual, Anda mendefinisikan serangkaian proses alternatif. Runtime akan mencoba menjalankan proses utama. Jika proses tersebut `gagal` (misalnya karena `harus` tidak terpenuhi atau terjadi error internal), runtime secara otomatis akan beralih ke proses `maka` berikutnya.

**Contoh Konseptual:**

```vz
# Coba ambil data dari API utama, jika gagal, coba API cadangan.
# Jika itu juga gagal, ambil dari cache lokal.

proses ambilDariUtama {
    atur data = http.get("api.utama/data")
    harus data.status == 200 # Jika ini gagal, seluruh proses gagal
    kembali data.json
}

proses ambilDariCadangan {
    lihat("API utama gagal, mencoba cadangan...")
    atur data = http.get("api.cadangan/data")
    harus data.status == 200
    kembali data.json
}

proses ambilDariCache {
    lihat("Semua API gagal, membaca dari cache...")
    kembali cache.read("data.json")
}

# VzoelInfinity in action!
jika hasil = jalankan ambilDariUtama tidak berhasil maka
jika hasil = jalankan ambilDariCadangan tidak berhasil maka
atur hasil = jalankan ambilDariCache

lihat(hasil)
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
