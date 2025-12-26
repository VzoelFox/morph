# README.md - Stdlib Organization untuk N2

## Struktur Stdlib N2

### 📁 stdlib/core/ - Core System Components
- `memory.fox` - Custom allocator interface (replace malloc/free)
- `gc.fox` - GC control API (manual collection, thresholds)
- `strings.fox` - Pure string operations (no libc dependencies)
- `hash.fox` - Hash functions untuk containers

### 📁 stdlib/concurrency/ - Concurrency System
- `morphroutines.fox` - Morphroutines system (replace goroutines)
- `channels.fox` - Channel operations (replace Go channels)
- `workers.fox` - Worker mobility system

### 📁 stdlib/io/ - I/O Operations
- `buffer.fox` - Memory buffer I/O (replace file syscalls)
- `format.fox` - String formatting (replace printf/scanf)

### 📁 stdlib/system/ - System Operations
- `time.fox` - Pure time operations (replace time syscalls)
- `errors.fox` - Error handling system

### 📁 stdlib/ - Legacy (Keep for compatibility)
- `math.fox` - Math operations (existing)
- `conv.fox` - Type conversions (existing)
- `numeric.fox` - Numeric operations (existing)

## N2 Independence Goals

### ❌ Eliminate Dependencies:
- Goroutines → Morphroutines
- Go Channels → Morph Channels  
- File I/O Syscalls → Memory Buffers
- Time Syscalls → Pure Functions
- Libc malloc/free → Custom Allocator
- Printf/Scanf → Format Functions

### ✅ Keep Pure:
- Type System (all 5 phases)
- GC System (enhanced)
- Memory Management (custom)
- String Processing (pure)
- Hash Functions (independent)

## Usage Examples

```fox
# Old N1 way (with dependencies)
luncurkan(task_func)
var ch = saluran_baru(10)

# New N2 way (independent)
var unit = MorphUnit{id: 1, is_active: benar}
morph_spawn(unit, "task_func")
var ch = morph_channel(10)
```

## Implementation Status

- ✅ Core structure created
- ✅ Critical modules implemented
- ⏳ Testing in progress
- ⏳ Integration pending
