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

### 2.1 Filosofi
- **Statically Typed**: Semua tipe diketahui pada compile time
- **Strongly Typed**: Tidak ada implicit coercion (kecuali Int→Float via cast)
- **Inferred When Possible**: Type annotation optional jika compiler bisa infer
- **Explicit When Ambiguous**: Empty literals dan no-initializer butuh type

### 2.2 Tipe Primitif

| Tipe   | Size    | C Equivalent | Contoh             |
|--------|---------|--------------|-------------------|
| int    | 8 bytes | int64_t      | 42, -10, 0        |
| float  | 8 bytes | double       | 3.14, -0.5, 1.0   |
| string | 16 bytes| morph_string*| "hello", "dunia"  |
| bool   | 1 byte  | bool         | benar, salah      |

**Special:**
- `nil`: Null value untuk reference types (pointer, string, arrays, maps)

### 2.3 Tipe Komposit

#### Array (Dynamic, Homogeneous)
```fox
var numbers []int = [1, 2, 3];
var names []string = ["Alice", "Bob"];
var empty []float = [];  # Type annotation required
```

**Karakteristik:**
- Dynamic size (grows automatically)
- Homogeneous (semua elemen tipe sama)
- Zero-indexed
- Pass by reference (pointer semantics)

#### Map (Hash Table)
```fox
var ages map[string]int = {"Alice": 30, "Bob": 25};
var empty map[int]string = {};  # Type annotation required
```

**Karakteristik:**
- Key-value pairs
- Keys must be comparable (int, string, bool)
- Values can be any type
- Pass by reference

#### Struct (Named Fields)
```fox
struktur User
    id int
    name string
    active bool
akhir

var u User = User{id: 1, name: "Alice", active: benar};
var u2 = User{id: 2, name: "Bob", active: salah};  # Type inferred
```

**Karakteristik:**
- Named fields dengan tipe
- Struct literal initialization
- Pass by value (default), tapi bisa by reference via pointer

### 2.4 Function Types

```fox
# Function declaration
fungsi add(a int, b int) int
    kembalikan a + b;
akhir

# Function type variable
var operation fungsi(int, int) int;
operation = add;

# Multiple returns
fungsi divide(a int, b int) (int, error)
    jika b == 0
        kembalikan 0, error.Baru("division by zero");
    akhir
    kembalikan a / b, nil;
akhir
```

**Karakteristik:**
- First-class functions (dapat di-assign ke variable)
- Multiple return values
- Named parameters (untuk readability)

### 2.5 Interface Types (v1.0 - Untuk Self-Hosting)

```fox
antarmuka Node
    fungsi TokenLiteral() string
    fungsi String() string
akhir

struktur IntegerLiteral
    value int
akhir

# Method receiver (implements interface)
fungsi (i IntegerLiteral) TokenLiteral() string
    kembalikan "INT";
akhir

fungsi (i IntegerLiteral) String() string
    kembalikan int_to_string(i.value);
akhir
```

**Karakteristik:**
- Implicit implementation (duck typing)
- Method receivers untuk implement interface
- Runtime dispatch via vtable (di C codegen)

### 2.6 Type Inference Rules

#### Rule 1: Literal Inference
```fox
var x = 10;        # int
var f = 3.14;      # float
var s = "hello";   # string
var b = benar;     # bool
```

#### Rule 2: Composite Inference
```fox
var arr = [1, 2, 3];              # []int
var m = {"key": 42};              # map[string]int
var u = User{id: 1, name: "A"};   # User
```

#### Rule 3: Function Return Inference
```fox
fungsi getNum() int { kembalikan 42; akhir
var x = getNum();  # int (dari return type)
```

#### Rule 4: Multiple Assignment Inference
```fox
fungsi split() (int, string)
    kembalikan 1, "error";
akhir

var num, err = split();  # num: int, err: string
```

#### Rule 5: Ambiguous Cases (NEED ANNOTATION)
```fox
var arr = [];           # ❌ ERROR: Cannot infer element type
var arr []int = [];     # ✅ OK

var m = {};             # ❌ ERROR: Cannot infer key/value type
var m map[int]int = {}; # ✅ OK

var x;                  # ❌ ERROR: Type required
var x int;              # ✅ OK
```

### 2.7 Type Conversions

#### Explicit Casting (Only)
```fox
var i = 10;
var f = float(i);    # int → float (lossless)
var i2 = int(3.14);  # float → int (lossy, warning issued)

# String conversions via stdlib
var s = int_to_string(42);
var n, err = string_to_int("123");
```

**Rules:**
- NO implicit conversion (berbeda dari Go/C)
- Hanya int↔float yang bisa di-cast
- String conversions via stdlib functions (return error)
- bool↔int forbidden (explicit: `if b then 1 else 0`)

### 2.8 Mutability

#### Variable (var)
```fox
var x = 10;
x = 20;  # ✅ OK
```

#### Constant (tetapan)
```fox
tetapan x = 10;
x = 20;  # ❌ ERROR: Cannot reassign constant

tetapan arr = [1, 2, 3];
arr = [4, 5];     # ❌ ERROR: Cannot reassign
arr[0] = 10;      # ✅ OK: Dapat mutate content (v1.0)
```

**Note:** Deep immutability (freeze content) akan ditambahkan nanti sebagai `immutable` keyword.

### 2.9 Null Safety (Minimal v1.0)

```fox
var s string = nil;      # ✅ OK: string bisa nil
var x int = nil;         # ❌ ERROR: Primitive tidak bisa nil

var arr []int = nil;     # ✅ OK: Reference type
jika arr == nil
    cetak("array kosong");
akhir
```

**Note:** Full Option<T> type untuk null safety akan di v2.0+.

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
