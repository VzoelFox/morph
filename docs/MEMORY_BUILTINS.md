# Memory Builtins - Bootstrap Implementation

## Overview

Memory management builtins untuk MorphFox, diimplementasikan dalam x86_64 assembly.

## Arsitektur

```
builtins.s          - Entry points (__mf_*)
    │
    ├── alloc.s     - Page-based allocator (mem_alloc, mem_free)
    ├── arena.s     - Arena/bump allocator
    ├── pool.s      - Fixed-size pool allocator
    └── daemon_cleaner.s - Background memory cleanup
```

## Builtins API

### Memory Operations

| Fungsi | Input | Output | Deskripsi |
|--------|-------|--------|-----------|
| `__mf_mem_alloc(size)` | RDI=size | RAX=ptr | Alokasi memori |
| `__mf_mem_free(ptr, size)` | RDI=ptr, RSI=size | - | Dealokasi |
| `__mf_load_i64(addr)` | RDI=addr | RAX=value | Load 64-bit |
| `__mf_poke_i64(addr, val)` | RDI=addr, RSI=val | - | Store 64-bit |
| `__mf_load_byte(addr)` | RDI=addr | RAX=byte | Load byte |
| `__mf_poke_byte(addr, val)` | RDI=addr, RSI=val | - | Store byte |
| `__mf_memcpy(dst, src, n)` | RDI,RSI,RDX | RAX=dst | Copy memory |

### Arena Allocator

| Fungsi | Deskripsi |
|--------|-----------|
| `arena_create(size)` | Buat arena baru |
| `arena_alloc(arena, size)` | Alokasi dari arena |
| `arena_reset(arena)` | Reset bump pointer |
| `arena_get_usage(arena)` | Bytes terpakai |
| `arena_get_capacity(arena)` | Total kapasitas |

### Pool Allocator

| Fungsi | Deskripsi |
|--------|-----------|
| `pool_create(obj_size, cap)` | Buat pool (obj_size >= 8) |
| `pool_alloc(pool)` | Ambil objek dari pool |
| `pool_free(pool, obj)` | Kembalikan objek ke pool |

### Daemon Cleaner

| Fungsi | Deskripsi |
|--------|-----------|
| `daemon_start()` | Fork daemon process |
| `daemon_clean_snapshots()` | Bersihkan snapshot expired |
| `daemon_clean_sandboxes()` | Bersihkan sandbox expired |
| `daemon_monitor_memory()` | Monitor /proc/meminfo |

## Memory Layout

### Page Header (48 bytes)
```
[0x00] Next Page Ptr
[0x08] Prev Page Ptr
[0x10] Timestamp
[0x18] Page Size
[0x20] Magic "VZOELFOX"
[0x28] Reserved
```

### Arena Header (32 bytes)
```
[0x00] Start Ptr
[0x08] Current Ptr
[0x10] End Ptr
[0x18] ID
```

### Pool Header (48 bytes)
```
[0x00] Start Ptr
[0x08] Current Ptr
[0x10] End Ptr
[0x18] Object Size
[0x20] Free List Head
[0x28] Reserved
```

## Build

```bash
cd bootstrap/asm
as -o builtins.o builtins.s
as -o alloc.o alloc.s
as -o arena.o arena.s
as -o pool.o pool.s
as -o daemon_cleaner.o daemon_cleaner.s
```

## Error Codes

| Code | Deskripsi |
|------|-----------|
| 110 | NULL pointer dereference |
| 111 | Out-of-bounds access |
| 112 | Stack overflow |
