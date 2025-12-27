# MorphSH - Self-Hosting Compiler Components

**Status**: Organized & In Progress
**Last Updated**: 2025-12-27 20:30 WIB

## Directory Structure

```
morphsh/
├── bootstrap/           # Type system & checker (Phase 1-5 ✅ COMPLETE)
├── stdlib/              # Standard library helpers
├── evaluator/           # Tree walker & AST evaluator
├── tests/               # Test files for all components
├── archive/             # Archived experimental code
└── archive_aspirational/# Optimizer aspirational code (archived)
```

## Reorganization (2025-12-27)

**Before**: 69 .fox files + 48 build artifacts in flat structure
**After**: Organized into 5 logical directories

### Changes Made:
1. **bootstrap/**: All type system, checker, scope management files (26 files)
2. **stdlib/**: Built-in functions & helpers (7 files)
3. **evaluator/**: AST & tree walker implementation (11 files)
4. **tests/**: All test files (9 files)
5. **archive/**: Old experimental code (16 files)

## Current Status

### ✅ Complete
- **Bootstrap Type System** (Phase 1-5):
  - Scope management
  - Type checking (primitives, structs, arrays, maps)
  - Generic types ([]T, map[K]V)
  - Interface checking (duck typing)
  - Parser integration

### 🔄 In Progress
- **Tree Walker**: Simple evaluator working (arithmetic, comparisons)
- **Stdlib**: Basic helpers for string, array, map operations

### ⏸️ Pending
- **Full Tree Walker**: Complete AST traversal & evaluation
- **Self-Compilation**: N3 compile N3 test
- **Bootstrap Chain**: N0 → N1 → N2 → N3 validation

## Usage

### Compile a File
```bash
cd /root/morph
./morph morphsh/evaluator/tree_walker_simple.fox
```

### Run Tests
```bash
./morph morphsh/tests/test_bootstrap.fox
```

## References

- **Design**: /root/morph/DESIGN.md
- **Roadmap**: /root/morph/ROADMAP.md
- **Technical Debt**: .vzoel.jules/technical-debt.morph.vz
- **Sessions**: .vzoel.jules/session-2025-12-27-*.md

## Philosophy

> "Ship working features incrementally > Ship perfect features never"

Build in small, testable increments. Validate assumptions early. Be honest about status.
