# N0 - Go Compiler Architecture Documentation

## Metadata
- **Commit Hash**: f2e51c4
- **Date Extracted**: 2025-12-27
- **Status**: RETIRED (Preserved for resurrection)
- **Purpose**: Bootstrap compiler untuk Morph language (Go → C transpiler)

---

## Overview

N0 adalah compiler Morph generasi pertama yang ditulis dalam Go. Compiler ini bertanggung jawab untuk:
1. **Lexical Analysis** - Tokenization dari source code Morph
2. **Parsing** - Build Abstract Syntax Tree (AST)
3. **Semantic Analysis** - Type checking dan validation
4. **Code Generation** - Transpile ke C code
5. **Runtime Support** - Memory management & GC

---

## Statistics

- **Total Size**: 552KB
- **Lines of Code**: 13,028 lines
- **Go Files**: 66 files (19 source + 47 tests)
- **Go Version**: 1.24.3
- **Package Structure**: 7 packages

---

## Package Architecture

```
pkg/
├── lexer/          # Lexical Analysis (3 files)
│   ├── lexer.go    # Main lexer with state machine
│   ├── token.go    # Token definitions
│   └── utils.go    # Helper utilities
│
├── parser/         # Parser & AST (2 files)
│   ├── ast.go      # AST node definitions
│   └── parser.go   # Recursive descent parser
│
├── types/          # Type System (1 file)
│   └── types.go    # Type definitions & interfaces
│
├── checker/        # Semantic Analysis (4 files)
│   ├── checker.go      # Main type checker
│   ├── scope.go        # Scope management
│   ├── errors.go       # Error reporting
│   └── types_alias.go  # Type aliases
│
├── compiler/       # Code Generator (2 files)
│   ├── compiler.go         # C transpiler
│   └── runtime/
│       └── assets.go       # Embedded runtime template
│
└── evaluator/      # Interpreter (7 files)
    ├── evaluator.go        # Main interpreter
    ├── object.go           # Runtime objects
    ├── env.go              # Environment/scope
    ├── builtins.go         # Built-in functions
    ├── builtins_conv.go    # Conversion builtins
    ├── builtins_io.go      # I/O builtins
    └── builtins_time.go    # Time builtins
```

---

## Core Components

### 1. Lexer (`pkg/lexer/`)

**Purpose**: Tokenize Morph source code into stream of tokens

**Features**:
- State machine dengan support untuk:
  - `STATE_CODE`: Normal code mode
  - `STATE_STRING`: String interpolation mode
- Brace counting untuk nested structures
- Line & column tracking untuk error reporting
- Support untuk keywords bahasa Indonesia (jika, untuk, fungsi, dll)

**Key Files**:
- `lexer.go` - Main lexer implementation dengan state tracking
- `token.go` - Token type definitions dan keywords
- `utils.go` - Helper functions

---

### 2. Parser (`pkg/parser/`)

**Purpose**: Build Abstract Syntax Tree dari token stream

**Architecture**: Recursive Descent Parser dengan Pratt parsing untuk expressions

**AST Node Types**:
- **Statements**: VarStatement, ReturnStatement, IfStatement, WhileStatement, dll
- **Expressions**: InfixExpression, PrefixExpression, CallExpression, dll
- **Literals**: IntegerLiteral, StringLiteral, ArrayLiteral, StructLiteral, dll
- **Advanced**: InterfaceExpression, GenericType, MethodDeclaration

**Features**:
- Precedence-based expression parsing
- Support untuk generic types
- Interface definitions
- Method declarations
- Import system (`ambil` keyword)

---

### 3. Type System (`pkg/types/`)

**Purpose**: Define type representations

**Type Hierarchy**:
```
Type (interface)
├── BasicType       # int, string, bool, dll
├── ArrayType       # []T
├── MapType         # map[K]V
├── FunctionType    # fungsi(params) -> return
├── StructType      # struct { fields }
├── InterfaceType   # interface { methods }
├── GenericType     # T, U (type parameters)
└── PointerType     # *T
```

**Type Operations**:
- Type equality checking
- Type inference
- Generic instantiation
- Interface satisfaction

---

### 4. Checker (`pkg/checker/`)

**Purpose**: Semantic analysis & type checking

**Responsibilities**:
- Variable scope management
- Type inference dan checking
- Dead code detection
- Cyclic dependency detection
- Interface implementation verification
- Generic type resolution

**Architecture**:
- `Checker` - Main type checker dengan scope stack
- `Scope` - Symbol table untuk each lexical scope
- `Errors` - Structured error reporting
- `Importer` - Module import system

**Type Checking Flow**:
1. Walk AST nodes
2. Build scope hierarchy
3. Resolve types untuk each expression
4. Verify type compatibility
5. Check interface implementations
6. Detect unused variables & dead code

---

### 5. Compiler (`pkg/compiler/`)

**Purpose**: Transpile Morph AST → C code

**Compilation Strategy**:
- **Target**: Standard C (C99/C11)
- **Memory Model**: Garbage collected heap dengan reference counting
- **Concurrency**: Morphroutines → pthreads (planned)

**Code Generation**:
1. **Header Generation**:
   - Forward declarations
   - Struct type definitions
   - Function signatures

2. **Body Generation**:
   - Variable declarations → C locals
   - Expressions → C expressions
   - Control flow → C control structures
   - Function calls → C calls

3. **Runtime Integration**:
   - GC rooting untuk temporaries
   - Memory allocation tracking
   - String/Array/Map constructors
   - Error handling

**GC Integration**:
- Temporary rooting (`mph_gc_push_root`)
- Reference tracking
- Cycle detection (planned)
- Threshold-based collection

**Embedded Runtime**:
- `runtime/assets.go` - Contains C runtime template
- Template includes: GC, memory management, builtins
- Injected during compilation

---

### 6. Evaluator (`pkg/evaluator/`)

**Purpose**: Interpret Morph code (untuk REPL & testing)

**Object System**:
```
Object (interface)
├── Integer
├── String
├── Boolean
├── Array
├── Map
├── Struct
├── Function
├── Error
└── Nil
```

**Features**:
- Environment-based variable storage
- Function closures
- Built-in functions
- Error propagation
- Module system

**Built-in Categories**:
- **Conversion**: `int()`, `string()`, `bool()`
- **I/O**: `print()`, `println()`, `input()`
- **Time**: `now()`, `sleep()`
- **Core**: `panjang()`, `tambah()`, dll

---

## Compilation Pipeline

```
┌─────────────┐
│ Source Code │
│   (.fox)    │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Lexer     │ → Tokenization
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Parser    │ → AST Building
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Checker   │ → Type Checking & Semantic Analysis
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Compiler   │ → C Code Generation
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  GCC/Clang  │ → Native Binary
└─────────────┘
```

---

## Entry Point

**File**: `cmd/morph/main.go`

**Command Structure**:
```go
morph <command> <file>

Commands:
- compile  : Compile .fox file to C then native binary
- run      : Compile and execute
- check    : Type check only
- eval     : Interpret mode
```

**Importer System**:
- `FileImporter` - Loads modules from disk
- `EvalImporter` - Wraps FileImporter untuk evaluator
- Search paths: current dir, stdlib, relative paths

---

## Integration with Modern Morph

### Why N0 was Retired

N0 berhasil mencapai self-hosting milestone. Morph compiler sekarang bisa dikompilasi menggunakan compiler Morph itu sendiri (MorphSH - N1). Go compiler tidak lagi diperlukan untuk bootstrap.

**Timeline**:
- ✅ Phase 1-5: Parser, Type System, Generics, Interfaces
- ✅ Phase 6: Optimization (Switch case flattening)
- ✅ Phase 7: Full self-hosting achieved (commit 1ea7457)
- 🗄️ N0 archived but preserved at commit f2e51c4

### Current State (Post-Retirement)

- **N1** (MorphSH): Production compiler written in Morph
- **N3**: Pure Morph compilation target (in development)
- **N4**: FoxVM bytecode with JIT (planned)

---

## Resurrection Strategy

### Why Resurrect N0?

1. **Bootstrap Independence**: Bisa rebuild dari source tanpa dependency ke existing binary
2. **Cross-Platform**: Go compiler lebih portable daripada C toolchain
3. **Development Speed**: Faster iteration untuk language features
4. **Reference Implementation**: Source of truth untuk semantic behavior
5. **Testing**: Comprehensive test suite (47 test files)

### Resurrection Plan

```
Step 1: Preserve Current State
├── Create branch: n0-resurrection-backup
├── Generate checksums untuk all pkg/ files
└── Document architecture (this file)

Step 2: Restore Build System
├── Verify go.mod dependencies
├── Test build: go build ./cmd/morph
└── Run test suite: go test ./pkg/...

Step 3: Integration Testing
├── Compile stdlib modules
├── Run morphsh compiler dengan N0
└── Verify output compatibility dengan N1

Step 4: Parallel Development Model
├── Keep N0 for bootstrap & reference
├── Use N1 (MorphSH) for production
├── Sync language features between both
└── Use N0 tests as conformance suite
```

---

## File Checksums (SHA256)

Generate dengan: `go run checksum_gen.go`

Checksums disimpan di: `.morph.vz/checksums/pkg/`

---

## Dependencies

**External**: None (Pure Go stdlib)

**Go Stdlib Packages Used**:
- `fmt` - Formatting
- `os` - File I/O
- `path/filepath` - Path handling
- `strings` - String operations
- `crypto/sha256` - Checksums (checksum_gen.go)

---

## Testing Infrastructure

**Test Files**: 47 test files across all packages

**Test Categories**:
1. **Lexer Tests**: Token recognition, string interpolation
2. **Parser Tests**: AST building, error recovery
3. **Checker Tests**: Type checking, scope management, error detection
4. **Compiler Tests**: C code generation
5. **Evaluator Tests**: Runtime behavior

**Test Coverage Areas**:
- Basic types (int, string, bool)
- Collections (array, map)
- Control flow (if, while, for)
- Functions & closures
- Structs & methods
- Interfaces & duck typing
- Generics
- Error handling
- Dead code detection
- Cyclic dependencies

---

## Future Enhancements (If Resurrected)

1. **Performance**:
   - Parallel compilation
   - Incremental builds
   - Better GC algorithms

2. **Diagnostics**:
   - Better error messages
   - Warning system
   - Code suggestions

3. **Tooling**:
   - LSP server
   - Debugger integration
   - Profiler

4. **Language Features**:
   - Pattern matching
   - Async/await
   - Better generic constraints

---

## Contact & Maintenance

- **Original Repository**: https://github.com/VzoelFox/morph
- **Resurrection Commit**: f2e51c4
- **Documentation Date**: 2025-12-27
- **Status**: PRESERVED FOR RESURRECTION

---

## Conclusion

N0 adalah foundation solid yang membawa Morph dari concept ke self-hosted reality. 13,028 lines of carefully crafted Go code yang telah membuktikan design language Morph bisa di-implement dan di-bootstrap.

Preservation N0 memastikan kita bisa selalu rebuild language dari scratch, maintain reference implementation, dan validate new features against proven baseline.

**N0 lives on as the eternal bootstrap seed.** 🌱
