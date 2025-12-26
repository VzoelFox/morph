# MorphFox N3 - Pure Morph Compiler

✅ **CREATED**: N3 adalah compiler Morph yang ditulis sepenuhnya dalam bahasa Morph, tanpa dependency C untuk syscall dan I/O.

## Status Implementasi

### ✅ COMPLETED
- **Project Structure**: Clean architecture dengan separation of concerns
- **Runtime System**: Pure Morph syscall interface tanpa C dependency
- **Basic Testing**: Minimal test cases berfungsi
- **Build System**: Script untuk compile N3 dengan morph existing

### 🚧 IN PROGRESS
- **Lexer**: Token parsing foundation (basic implementation)
- **Parser**: AST generation (minimal structure)
- **Type System**: Type checking framework (skeleton)
- **Code Generation**: Pure Morph output generator

### 📋 TODO
- **Inline Assembly**: Direct syscall implementation
- **File I/O**: Pure Morph file operations
- **Self-Compilation**: N3 compiles itself
- **Memory Management**: Pure Morph heap management

## Struktur

```
morphfox/
├── core/           # Core compiler components
│   ├── lexer.fox   # ✅ Tokenizer (minimal)
│   ├── parser.fox  # ✅ AST parser (skeleton)
│   └── types.fox   # ✅ Type system (framework)
├── backend/        # Code generation
│   ├── morph.fox   # ✅ Pure Morph output
│   └── native.fox  # ✅ Native syscall interface
├── runtime/        # Pure Morph runtime
│   ├── memory.fox  # ✅ Memory management (basic)
│   ├── io.fox      # ✅ I/O without C dependency
│   └── syscall.fox # ✅ Direct syscall interface
├── main.fox        # ✅ Compiler entry point
└── test_*.fox      # ✅ Test cases
```

## Keunggulan N3

1. **Zero C Dependency**: Semua I/O dan syscall menggunakan pure Morph
2. **Clean Architecture**: Struktur rapi, tidak berantakan seperti morphsh
3. **Self-Bootstrapping**: Target untuk compile dirinya sendiri
4. **Direct Syscalls**: Bypass C runtime sepenuhnya

## Testing

```bash
# Test minimal functionality
cd morphfox
../morph test_minimal.fox

# Test syscall interface
../morph test_syscall.fox

# Build N3 (when ready)
../build_n3.sh
```

**Next Priority**: Implement inline assembly untuk direct syscall Linux.
