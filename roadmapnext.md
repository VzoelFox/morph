# Visi Jangka Panjang: Evolusi Morph Menuju Compiler

Dokumen ini menguraikan visi jangka panjang untuk evolusi Bahasa Morph dari bahasa yang diinterpretasi menjadi bahasa yang sepenuhnya dikompilasi, menghilangkan ketergantungan pada interpreter Python saat runtime.

---

### Fase 1: Transpiler & Analisis Lanjutan (Target: v0.6)

**Tujuan:** Membangun fondasi untuk kompilasi dengan memperkenalkan analisis sebelum eksekusi.

**Langkah-langkah:**
- **[x] Desain Analisis Statis:** Merancang dan mengimplementasikan sistem untuk analisis tipe statis (inferensi tipe) dan pemeriksaan error sebelum program dijalankan.
- **[x] Implementasi *Transpiler* (Dasar):** Membangun fondasi *transpiler* yang dapat mengubah sintaks dasar Morph (.fox) menjadi kode Python yang setara. Fitur yang didukung saat ini: variabel, literal, operator, dan beberapa fungsi bawaan.

---

### Fase 2: *Intermediate Representation* (IR) (Target: v0.7)

**Tujuan:** Menciptakan jembatan antara bahasa tingkat tinggi Morph dan bahasa mesin tingkat rendah.

**Langkah-langkah:**
- **[ ] Desain IR:** Merancang *Intermediate Representation* (representasi perantara) khusus untuk Bahasa Morph.
- **[ ] Kompiler ke IR:** Membangun komponen compiler (frontend) yang dapat mengubah kode Morph menjadi IR yang telah dirancang.

---

### Fase 3: *Code Generation* & *Backend* LLVM (Target: v0.8)

**Tujuan:** Menghasilkan file executable mandiri dari IR.

**Langkah-langkah:**
- **[ ] Integrasi LLVM:** Mempelajari dan mengintegrasikan LLVM toolchain ke dalam proses build.
- **[ ] Backend Compiler:** Membangun *backend* yang dapat mengubah IR menjadi kode mesin asli (native executable) untuk platform target (misalnya, Linux, Windows) menggunakan LLVM.
