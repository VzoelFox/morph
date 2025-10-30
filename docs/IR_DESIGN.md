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

---
*Dokumen ini akan terus dikembangkan seiring dengan implementasi compiler.*
