# Git Workflow & CI/CD

Dokumen ini menjelaskan alur kerja Git dan standar CI/CD untuk proyek Morph.

## 1. Branching Strategy
- **main**: Branch utama yang stabil.
- **feature/*** : Branch untuk fitur baru.
- **bugfix/*** : Branch untuk perbaikan bug.

## 2. Commit Convention
Gunakan format pesan commit yang jelas:
```
<type>: <subject>

<body>
```
Contoh:
```
feat: add basic http server
fix: memory leak in runtime
docs: update AGENTS.md
```

## 3. Ignore Policy (Whitelist)
Proyek ini menggunakan strategi `.gitignore` Whitelist.
- Secara default, **SEMUA** file di-ignore (`*`).
- Hanya file source code dan dokumentasi yang di-whitelist secara eksplisit.
- **Tujuannya**: Mencegah artifact build (binary, object files, temporary files) masuk ke repository secara tidak sengaja.

## 4. AGENTS.md Protocol
Setiap commit yang mengubah kode harus disertai dengan update pada `AGENTS.md`:
1. Generate checksum baru untuk file yang berubah.
2. Update versi di `AGENTS.md`.
3. Catat perubahan di bagian "Riwayat Perubahan".

## 5. CI/CD (GitHub Actions)
Pipeline CI berjalan otomatis pada setiap Push dan Pull Request:
- **Build**: Memastikan kode Go dapat dikompilasi.
- **Test**: Menjalankan `go test ./...` untuk memverifikasi logika.
