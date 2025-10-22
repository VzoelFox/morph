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
