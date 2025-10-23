# Catatan Patch v0.3.0

Versi ini memperkenalkan sistem konkurensi kooperatif `management` dan mencakup sejumlah perbaikan bug penting untuk meningkatkan stabilitas dan fungsionalitas interpreter.

## Fitur Baru
*   **Sistem Konkurensi `management`**:
    *   Memperkenalkan blok `management` yang berfungsi sebagai event loop untuk tugas-tugas konkuren.
    *   Menambahkan ekspresi `jalankan` untuk memulai sebuah `proses` secara asinkron.
    *   Menambahkan ekspresi `tunggu` untuk menjeda eksekusi sebuah `proses` selama durasi tertentu, memungkinkan tugas lain untuk berjalan.

## Perbaikan Bug
*   **Stabilitas Parser**: Memperbaiki serangkaian `IndentationError` dan `TypeError` yang disebabkan oleh duplikasi kode dan kesalahan logika dalam parser, lexer, dan node AST.
*   **Lexical Scoping**: Memperbaiki `TypeError` di konstruktor `Environment` untuk memastikan `enclosing scope` berfungsi dengan benar, yang sangat penting untuk fungsi dan proses.
*   **Sistem Modul `vzoelinfinity`**: Memperbaiki logika `visit_AturStatement` untuk menangani `VzoelModuleNotFound`, memungkinkan fallback impor modul berfungsi seperti yang dirancang.
*   **Penanganan Tipe**: Menambahkan fungsi bawaan `ubahKeString()` untuk mengatasi `TypeError` saat menggabungkan string dan angka.

## Perubahan Lainnya
*   **Peningkatan Kode**: Membersihkan dan memfaktorkan ulang beberapa file inti interpreter (`lexer.py`, `parser.py`, `ast_nodes.py`, `environment.py`) untuk meningkatkan keterbacaan dan pemeliharaan.
*   **Peningkatan Tes**: Memperbaiki dan menyederhanakan beberapa file tes untuk memastikan pengujian yang akurat dan andal.
