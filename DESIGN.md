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

### Entry Point
Program Morph bersifat skrip linear. Tidak ada fungsi `utama` yang dipanggil secara implisit. Kode di level teratas akan dieksekusi. Konvensi umum adalah mendefinisikan fungsi `main` dan memanggilnya di akhir file.

### Contoh Hello World
```fox
ambil "io"

fungsi main() Void
    io.Write(io.Stdout, "Halo, Dunia!\n")
akhir

main()
```

## 2. Sistem Tipe (Type System)

### 2.1 Filosofi
- **Statically Typed**: Semua tipe diketahui pada compile time.
- **Strongly Typed**: Tidak ada implicit coercion yang tidak aman.
- **Explicit Types**: Tipe primitif menggunakan PascalCase (`Int`, `String`) sesuai implementasi saat ini.

### 2.2 Tipe Primitif

| Tipe   | Ukuran  | Deskripsi          | Contoh             |
|--------|---------|--------------------|-------------------|
| Int    | 64-bit  | Bilangan bulat     | 42, -10, 0        |
| Float  | 64-bit  | Bilangan desimal   | 3.14, -0.5, 1.0   |
| String | Dynamic | Teks (Immutable)   | "hello", "dunia"  |
| Bool   | 1 byte  | Boolean            | benar, salah      |
| Void   | 0 byte  | Tipe kosong        | (return type)     |

**Catatan:**
- `String` bersifat immutable. Indexing `str[i]` mengembalikan `Int` (byte value).
- `kosong` adalah nilai null untuk tipe referensi (Array, Map, Function, Interface, Pointer, String).

### 2.3 Tipe Komposit

#### Array (Homogeneous)
```fox
var numbers []Int = [1, 2, 3]
var names []String = ["Alice", "Bob"]
```
- Ukuran dinamis.
- Tipe elemen harus seragam (homogen).

#### Map (Hash Table)
```fox
var ages map[String]Int = {"Alice": 30, "Bob": 25}
```
- Key dan Value harus bertipe konsisten.

#### Struct (Named Fields)
```fox
struktur User
    ID   Int
    Name String
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
var s String = kosong      # Valid
var arr []Int = kosong     # Valid
var i Int = kosong         # Error (Primitif tidak bisa kosong)
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
Rencana menggunakan model **Green Threads** (mirip Goroutine) dengan keyword `jalankan`.
```fox
# Syntax Proposal
fungsi worker(id Int) Void
    ...
akhir

jalankan worker(1)
```
Status: *Planned (Post-Self-Host)*. Saat ini Morph berjalan single-threaded.

### B. Manajemen Memori
Rencana menggunakan pendekatan semi-manual atau region-based. Saat ini manajemen memori ditangani sepenuhnya oleh Garbage Collector dari host language (Go).

### C. Refactoring Tipe
Rencana untuk memigrasikan nama tipe primitif menjadi lowercase (`int`, `string`) agar sesuai dengan konvensi umum. Saat ini menggunakan PascalCase (`Int`, `String`) sebagai batasan teknis sementara.
