# Desain Intermediate Representation (IR) untuk Bahasa Morph

Dokumen ini menguraikan desain awal dari Intermediate Representation (IR) yang akan digunakan oleh compiler Morph. IR ini berfungsi sebagai jembatan antara *frontend* (parser & analyzer) dan *backend* (code generator).

## 1. Tujuan Desain

- **Independen dari Target:** IR harus abstrak dan tidak bergantung pada arsitektur mesin target tertentu (misalnya, x86, ARM).
- **Mudah Digenerate:** Proses transformasi dari AST (Abstract Syntax Tree) ke IR harus sederhana.
- **Mudah Dioptimasi:** Struktur IR harus memungkinkan untuk penerapan berbagai teknik optimasi di masa depan.
- **Mendukung Fitur Morph:** IR harus dapat merepresentasikan semua fitur bahasa Morph, termasuk tipe data dinamis, fungsi, dan struktur kontrol.

## 2. Format IR

IR akan menggunakan representasi berbasis instruksi linear, mirip dengan *Three-Address Code* (TAC). Setiap instruksi akan terdiri dari:

- `opcode`: Operasi yang akan dilakukan (misalnya, `ADD`, `STORE`, `LOAD`).
- `result`: (Opsional) Variabel/register temporary tempat hasil operasi disimpan.
- `arg1`: (Opsional) Argumen pertama.
- `arg2`: (Opsional) Argumen kedua.

**Contoh:**
```
atur a = 10 + 5
```
Akan ditransformasikan menjadi:
```
t1 = LOAD_CONST 10
t2 = LOAD_CONST 5
t3 = ADD t1, t2
STORE_VAR a, t3
```

## 3. Daftar Instruksi (Opcode) Awal

### Operasi Aritmatika
- `ADD src1, src2 -> dest`
- `SUB src1, src2 -> dest`
- `MUL src1, src2 -> dest`
- `DIV src1, src2 -> dest`

### Operasi Memori
- `LOAD_CONST value -> dest`      (Memuat nilai literal)
- `LOAD_VAR name -> dest`         (Memuat nilai dari variabel)
- `STORE_VAR name, src`         (Menyimpan nilai ke variabel)

### Operasi Kontrol Alur
- `JUMP label`                    (Lompat tanpa syarat)
- `JUMP_IF_FALSE cond, label`     (Lompat jika kondisi salah)
- `LABEL name`                    (Penanda untuk target lompatan)

### Operasi Fungsi
- `CALL func, [args] -> dest`
- `RETURN [value]`

## 4. Representasi Struktur Kontrol

### Pernyataan `jika/maka`
Struktur `jika kondisi maka ... akhir` akan diubah menjadi lompatan kondisional.
**Contoh Morph:**
```
jika a > 5 maka
  lihat "besar"
akhir
```
**Representasi IR:**
```
  t1 = LOAD_VAR a
  t2 = LOAD_CONST 5
  t3 = GREATER_THAN t1, t2  // (membutuhkan opcode perbandingan baru)
  JUMP_IF_FALSE t3, end_if

  t4 = LOAD_CONST "besar"
  CALL print, [t4]

LABEL end_if
```

### Perulangan `ulangi`
Struktur `ulangi ... sebanyak N kali` akan diimplementasikan menggunakan counter dan lompatan.
**Contoh Morph:**
```
ulangi
  lihat "hello"
sebanyak 3 kali
```
**Representasi IR:**
```
  counter = LOAD_CONST 0
  limit = LOAD_CONST 3

LABEL loop_start
  t1 = LESS_THAN counter, limit // (opcode perbandingan)
  JUMP_IF_FALSE t1, loop_end

  // Badan perulangan
  t2 = LOAD_CONST "hello"
  CALL print, [t2]

  // Inkrementasi counter
  one = LOAD_CONST 1
  counter = ADD counter, one
  JUMP loop_start

LABEL loop_end
```

## 5. Representasi Tipe Data Kompleks

### Tipe `Daftar` (List)
`Daftar` akan dialokasikan di *heap* dan direferensikan melalui alamat memori.

**Opcode Baru:**
- `NEW_LIST [elements] -> dest`  (Membuat list baru)
- `LIST_GET src, index -> dest` (Mengambil elemen)
- `LIST_SET dest, index, value` (Mengatur elemen)

**Contoh:** `atur x = [10, 20]`
**Representasi IR:**
```
  t1 = LOAD_CONST 10
  t2 = LOAD_CONST 20
  t3 = NEW_LIST [t1, t2]
  STORE_VAR x, t3
```

### Tipe `Peta` (Map)
`Peta` juga akan dialokasikan di *heap*.

**Opcode Baru:**
- `NEW_MAP -> dest`               (Membuat map kosong)
- `MAP_GET src, key -> dest`    (Mengambil nilai berdasarkan kunci)
- `MAP_SET dest, key, value`    (Mengatur nilai untuk sebuah kunci)

**Contoh:** `atur y = peta{"kunci": "nilai"}`
**Representasi IR:**
```
  t1 = NEW_MAP
  t2 = LOAD_CONST "kunci"
  t3 = LOAD_CONST "nilai"
  MAP_SET t1, t2, t3
  STORE_VAR y, t1
```

---
*Dokumen ini akan terus dikembangkan seiring dengan implementasi compiler.*
