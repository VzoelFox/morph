# Spesifikasi Desain Bahasa Morph

## Filosofi
- **Explicit over Implicit**: Tidak ada "magic". Kode melakukan apa yang tertulis.
- **Error as Value**: Error adalah warga kelas satu. Tidak ada exception.
- **Context-Aware**: Integrasi mendalam dengan tooling/AI via context file (`.morph.vz`).
- **Bahasa Indonesia**: Menggunakan kata kunci Bahasa Indonesia untuk kemudahan pemahaman lokal.

## 1. Tata Bahasa (Grammar) & Sintaks

Menggunakan gaya blok dengan keyword `akhir` (mirip Ruby/Lua) untuk mengurangi noise visual kurung kurawal `{}`.

### Kata Kunci (Keywords)
- `fungsi`: Mendefinisikan fungsi.
- `variabel` / `var`: Mendefinisikan variabel mutable.
- `tetapan` / `const`: Mendefinisikan konstanta.
- `struktur`: Mendefinisikan tipe data komposit (struct).
- `antarmuka`: Mendefinisikan interface.
- `jika`, `atau_jika`, `lainnya`: Kontrol alur (if-else-if-else).
- `selama`: Perulangan (while).
- `kembalikan`: Return value.
- `akhir`: Menutup blok kode.
- `native`: Mendeklarasikan fungsi eksternal (binding host).
- `ambil`: Mengimpor modul (`import`).
- `kosong`: Nilai null (`nil`).
- `benar` / `salah`: Boolean literal.

### Tanda Baca
- Titik koma (`;`) bersifat **opsional**. Parser akan mengabaikannya jika ada di akhir baris. Disarankan untuk tidak menggunakannya (Go-style) kecuali memisahkan multiple statement dalam satu baris.

### Interpolasi String
Mendukung penyisipan ekspresi dalam string menggunakan sintaks `#{}`.
```fox
var nama = "Dunia"
var pesan = "Halo, #{nama}!"
```

### Entry Point
Program Morph bersifat skrip linear. Tidak ada fungsi `utama` yang dipanggil secara implisit. Kode di level teratas akan dieksekusi. Konvensi umum adalah mendefinisikan fungsi `main` dan memanggilnya di akhir file.

### Contoh Hello World
```fox
ambil "io"

fungsi main() void
    io.Write(io.Stdout, "Halo, Dunia!\n")
akhir

main()
```

## 2. Sistem Tipe (Type System)

### 2.1 Filosofi
- **Statically Typed**: Semua tipe diketahui pada compile time.
- **Strongly Typed**: Tidak ada implicit coercion yang tidak aman.
- **Explicit Types**: Tipe primitif menggunakan lowercase (`int`, `string`) sesuai konvensi umum (sejak v1.28.0).

### 2.2 Tipe Primitif

| Tipe   | Ukuran  | Deskripsi          | Contoh             |
|--------|---------|--------------------|-------------------|
| int    | 64-bit  | Bilangan bulat     | 42, -10, 0        |
| float  | 64-bit  | Bilangan desimal   | 3.14, -0.5, 1.0   |
| string | Dynamic | Teks (Immutable)   | "hello", "dunia"  |
| bool   | 1 byte  | Boolean            | benar, salah      |
| void   | 0 byte  | Tipe kosong        | (return type)     |

**Catatan:**
- `string` bersifat immutable. Indexing `str[i]` mengembalikan `int` (byte value).
- `kosong` adalah nilai null untuk tipe referensi (Array, Map, Function, Interface, Pointer, String).

### 2.3 Tipe Komposit

#### Array (Homogeneous)
```fox
var numbers []int = [1, 2, 3]
var names []string = ["Alice", "Bob"]
```
- Ukuran dinamis.
- Tipe elemen harus seragam (homogen).

#### Map (Hash Table)
```fox
var ages map[string]int = {"Alice": 30, "Bob": 25}
```
- Key dan Value harus bertipe konsisten.

#### Struct (Named Fields)
```fox
struktur User
    ID   int
    Name string
akhir

# Inisialisasi Literal
var u = User{ID: 1, Name: "Alice"}

# Inisialisasi Constructor (Positional)
var u2 = User(2, "Bob")
```
- Field struct menggunakan tipe eksplisit.
- Mendukung dua gaya inisialisasi: Literal `{}` dan Constructor `()`.

### 2.4 Operator

- **Aritmatika**: `+`, `-`, `*`, `/`, `%` (Modulo).
- **Bitwise**: `&`, `|`, `^` (XOR), `~` (NOT), `<<`, `>>`.
- **Logika**: `&&`, `||`, `!`.
- **Perbandingan**: `==`, `!=`, `<`, `>`, `<=`, `>=`.

### 2.5 Null Safety
```fox
var s string = kosong      # Valid
var arr []int = kosong     # Valid
var i int = kosong         # Error (Primitif tidak bisa kosong)
```

## 3. Standard Library

Morph menyediakan pustaka standar melalui perintah `ambil`. Modul yang tersedia saat ini:

- **io**: Input/Output dasar.
    - `Open(path String) File`
    - `Write(f File, s String) Int`
    - `Close(f File) Int`
    - `Stdout` (Variable)
- **math**: Fungsi matematika (`Sin`, `Cos`, `Sqrt`, `Abs`, `PI`).
- **string**: Manipulasi string (`Len`, `Concat`).
- **time**: Waktu dan durasi (`Now`, `Sleep`).
- **conv**: Konversi tipe (`Atoi`, `Itoa`).

Contoh penggunaan:
```fox
ambil "math"
ambil "conv"

var s = "123"
var i, err = conv.Atoi(s)
jika err == kosong
    var root = math.Sqrt(i)
akhir
```

## 4. Roadmap & Fitur Masa Depan (Planned)

Fitur berikut adalah bagian dari rencana jangka panjang dan belum tersedia di implementasi saat ini.

### A. Konkurensi (Concurrency)
Rencana menggunakan model **Green Threads** (mirip Goroutine) dengan fungsi bawaan `luncurkan`.
```fox
# Syntax Proposal
fungsi worker(id int) void
    ...
akhir

luncurkan(worker, 1)
```
Status: *Implemented (MVP)*.
- Mendukung `luncurkan`, `saluran_baru`, `kirim`, `terima` (C Runtime via Pthreads).
- Channel hanya mendukung tipe `int` (int64) untuk saat ini.

### B. Manajemen Memori
Rencana menggunakan pendekatan semi-manual atau region-based. Saat ini manajemen memori ditangani sepenuhnya oleh Garbage Collector dari host language (Go).

### C. Refactoring Tipe (Resolved)
Nama tipe primitif telah dimigrasikan menjadi lowercase (`int`, `string`, `bool`, `void`) pada v1.28.0.
