# Morph Abstraction Layer Design

## Principle: Language Independent of Implementation

Morph is designed to be retargetable to different backends
without changing user code or compiler code (v2+).

## Abstraction Levels

### Level 1: Morph Language (User-Facing)
- High-level semantics (arrays, maps, structs)
- No implementation details exposed
- Defined in: DESIGN.md

### Level 2: Runtime Interface (Compiler-Facing)
- Abstract operations (array_new, array_append)
- Function signatures, not implementations
- Defined in: RUNTIME_API.md

### Level 3: Implementation (Backend-Specific)
- C structs, LLVM IR, bytecode, etc.
- Replaceable without affecting Level 1 & 2
- Defined in: MEMORY_LAYOUT.md (C-specific)

## Backend Independence Test

Valid Morph code must work with ANY backend:

```fox
var arr []int = [1, 2, 3];
arr.append(4);
```

This should compile to:
- C backend: morph_array_append()
- LLVM backend: @morph.array.append()
- VM backend: ARRAY_APPEND instruction
- Future: Native vector instructions

**If backend change requires Morph code changes: BAD DESIGN**
