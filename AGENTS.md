# agents.md - Source of Truth untuk AI Agent

## Metadata Dokumen
- **Versi**: 1.0.0
- **Tanggal Dibuat**: 2025-12-20 06.10 WIB
- **Checksum**: SHA256:INITIAL
- **Status**: Active

---

## Tujuan Dokumen
Dokumen ini adalah **single source of truth** untuk AI Agent dalam pengembangan bahasa pemrograman Fox. Setiap perubahan pada codebase akan tercatat di sini dengan checksum, timestamp, dan konteks untuk meminimalisir halusinasi dan menjaga konsistensi.

---

## Struktur Project

```
project-root/
├── agents.md                 # File ini (source of truth)
├── .fox.vz/                  # Direktori konteks dan snapshot
│   ├── snapshots/           # Snapshot state per sesi
│   ├── checksums/           # Checksum file per versi
│   └── context/             # Konteks sesi development
└── .vzoel.jules/            # Catatan hutang teknis & rekomendasi
    ├── technical-debt.fox.vz
    ├── next-improvements.fox.vz
    └── session-notes/
```

---

## Spesifikasi Bahasa Fox

### Status: Dalam Perencanaan
**Target**: Bahasa pemrograman yang bisa self-host dengan compiler bootstrap

### Fitur Core (Planned)
- [ ] Lexer & Parser
- [ ] AST Generator
- [ ] Type Checker
- [ ] Code Generator
- [ ] Self-hosting capability
- [ ] AI-assisted development dengan context tracking

---

## Riwayat Perubahan

### Version 1.0.0 - 2025-12-20 06.10 WIB
**Checksum**: SHA256:INITIAL  
**Perubahan**:
- Initial creation of agents.md
- Mendefinisikan struktur project
- Menetapkan workflow untuk tracking perubahan

**Konteks Sesi**:
- User ingin membuat bahasa pemrograman dengan AI agent
- Fokus pada minimalisir halusinasi dengan tracking ketat
- Menggunakan checksum dan timestamp untuk setiap perubahan
- File konteks disimpan di `.fox.vz`
- Technical debt tracking di `.vzoel.jules`

**File Terkait**:
- `agents.md` (file ini)

---

## Protokol Update

### Setiap Kali Ada Perubahan File:

1. **Generate Checksum**
   ```bash
   sha256sum <filename> > .fox.vz/checksums/<filename>.sha256
   ```

2. **Catat di agents.md**
   - Versi baru (semantic versioning)
   - Timestamp ISO 8601
   - Checksum file yang berubah
   - Catatan perubahan detail
   - Konteks sesi (apa yang sedang dikerjakan)

3. **Simpan Snapshot Konteks**
   ```bash
   # Simpan state conversation di .fox.vz/context/
   session-YYYY-MM-DD-HHMMSS.md
   ```

4. **Update Technical Debt**
   ```bash
   # Catat di .vzoel.jules/
   - Apa yang belum sempurna
   - Rekomendasi perbaikan
   - TODO items
   ```

---

## Template Entry Perubahan

```markdown
### Version X.Y.Z - YYYY-MM-DD HH:MM:SS UTC
**Checksum**: SHA256:<hash>  
**Perubahan**:
- [Daftar perubahan detail]

**Konteks Sesi**:
- [Apa yang sedang dikerjakan]
- [Keputusan design yang diambil]
- [Alasan di balik perubahan]

**File Terkait**:
- `path/to/file1.ext` (SHA256:hash)
- `path/to/file2.ext` (SHA256:hash)

**Input User**:
```
[Copy paste input user jika relevan]
```

**Output Agent**:
```
[Summary output/code yang dihasilkan]
```
```

---

## Aturan untuk AI Agent

1. **SELALU** cek `agents.md` sebelum membuat atau memodifikasi file
2. **JANGAN UBAH AGENTS.md** karena ini sumber kebenaran
3. **SELALU** generate checksum untuk file yang berubah
4. **SELALU** catat konteks lengkap di setiap entry
5. **JANGAN PERNAH** membuat file tanpa mencatatnya di sini
6. **JANGAN PERNAH** mengklaim mengingat sesuatu tanpa cek agents.md
7. **RUJUK** agents.md sebagai satu-satunya sumber kebenaran

---

## Definisi Status File

- **Active**: File sedang aktif digunakan
- **Deprecated**: File lama, diganti file baru
- **Archived**: File disimpan untuk referensi historis
- **Planned**: File yang akan dibuat

---

## Kontrak dengan AI Agent

Saya, Claude AI, berkomitmen untuk:
- Menggunakan `agents.md` sebagai satu-satunya sumber kebenaran
- Tidak mengklaim ingat sesuatu yang tidak tercatat di sini
- Selalu update dokumen ini setelah setiap perubahan
- Mencatat semua input/output di `.vzoel.jules` untuk tracking
- Menjaga konsistensi dan akurasi informasi

---

## Next Steps

1. Diskusi spesifikasi bahasa Fox
2. Tentukan syntax dan semantics
3. Pilih bahasa untuk bootstrap compiler
4. Mulai implementasi lexer
5. Setup tooling untuk auto-checksum

---

**Catatan**: File ini akan terus di-update seiring development. Selalu cek versi terbaru dan checksum sebelum melanjutkan pekerjaan.



**Founder** : Vzoel Fox's 
