# Stdlib - Standard Library Helpers

**Status**: 🔄 In Progress
**Components**: 7 files

## What This Is

Standard library helper functions for common operations:
- String manipulation
- Array operations
- Map operations
- I/O functions
- Type conversions
- Time operations

## Files

### Core Helpers (NEW - 2025-12-27)
- `string.fox` - String length, concat, equals, conversions
- `array.fox` - Array length, append, get, sum
- `map.fox` - Map length, has_key, get with default

### Built-ins (Existing)
- `builtins.fox` - Core built-in functions
- `builtins_io.fox` - I/O operations (print, read, write)
- `builtins_conv.fox` - Type conversions (Itoa, Atoi, etc.)
- `builtins_time.fox` - Time operations (Now, Sleep)

## Known Limitations

### String Indexing Issue
- `string[index]` returns `int` (char code), not `string`
- Workaround: Use length comparison for equality
- Blocks: Character-by-character string operations
- Status: Pending stdlib enhancement

### Map Initialization
- Map literals not supported in parser
- Workaround: Initialize empty maps
- Status: Parser enhancement needed

## Usage Example

```morph
ambil "morphsh/stdlib/string"
ambil "morphsh/stdlib/array"

fungsi main() void
    var len = StringLength("hello")  # Returns 5
    var arr = [1, 2, 3]
    var sum = ArraySum(arr)          # Returns 6
akhir
```

## Next Steps

1. Fix string indexing type issue
2. Implement proper map operations
3. Add more helper functions as needed
