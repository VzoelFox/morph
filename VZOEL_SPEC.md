# Spesifikasi Bahasa Vzoel Word v0.2

Dokumen ini mendefinisikan sintaks dan filosofi untuk Vzoel Word v0.2.

## 1. Filosofi
*   **Ekspresif dalam Bahasa Indonesia:** Sintaks inti menggunakan kata-kata dari Bahasa Indonesia.
*   **Ketangguhan (Resilience):** Mekanisme `vzoelinfinity` untuk fallback otomatis.
*   **Berorientasi pada Proses:** Logika dibungkus dalam `proses`.

## 2. Sintaks Dasar
*   **Komentar:** `#`
*   **Tipe Data:** `String`, `Number`, `Boolean` (`benar`, `salah`), `TidakAda` (`tidak ada`).

## 3. Kata Kunci (Keywords)
*   `atur`: Mendeklarasikan variabel.
*   `lihat`: Output.
*   `jika ... maka ...`: Kondisional.
*   `jika ... tidak berhasil maka ...`: Fallback.
*   `proses`: Mendefinisikan fungsi.
*   `kembali`: Mengembalikan nilai.
*   `ambil ... dari ...`: Impor modul.
*   `.`: Akses properti.

Vzoel Word adalah bahasa pemrograman yang dirancang dengan prinsip-prinsip berikut:

*   **Ekspresif dalam Bahasa Indonesia:** Sintaks inti menggunakan kata-kata dari Bahasa Indonesia untuk membuat kode lebih mudah dibaca dan intuitif bagi penutur asli.
*   **Ketangguhan (Resilience):** Terinspirasi oleh *structured concurrency*, Vzoel Word memiliki mekanisme bawaan untuk menangani kegagalan dan menjalankan alur alternatif secara elegan melalui konsep `vzoelinfinity`.
*   **Berorientasi pada Proses:** Logika dibungkus dalam `proses` yang dapat berjalan secara independen, berkomunikasi, dan dikelola oleh runtime.

## 2. Sintaks Dasar

*   **Komentar:** Dimulai dengan `#`.
*   **Tipe Data:** `String` ("teks"), `Number` (123, 3.14), `Boolean` (`benar`, `salah`), `TidakAda` (`tidak ada`).

## 3. Kata Kunci (Keywords)

### Variabel & Output
*   `atur`: Mendeklarasikan atau menetapkan nilai ke variabel.
*   `lihat`: Menampilkan nilai ke output standar.

### Struktur Kontrol
*   `jika ... maka ...`: Blok kondisional dasar.
*   `jika ... tidak ... maka ...`: Struktur kondisional dengan fallback (lihat `vzoelinfinity`).

### Proses & Fungsi
*   `proses`: Mendefinisikan blok kode yang dapat dieksekusi.
*   `kembali`: Mengembalikan nilai dari sebuah `proses`.
*   `jalankan`: Mengeksekusi sebuah `proses`.

### Penanganan Proses & Konkurensi
*   `tunggu`: Menjeda eksekusi.
*   `pemicu`: Mendefinisikan sebuah event.
*   `harus`: Menegaskan bahwa suatu kondisi harus benar.

## 4. Konsep Inti: `vzoelinfinity`

Sintaks utamanya adalah `jika ... tidak ... maka ...`. Ini memungkinkan runtime untuk secara otomatis mencoba proses berikutnya dalam rantai jika proses sebelumnya gagal.
