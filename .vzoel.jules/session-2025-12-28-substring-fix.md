# Session 2025-12-28 - Substring Fix

## Summary
Fixed substring() segfault issue in N0 compiler.

## Root Cause
- `substring` was NOT in `isBuiltin()` function
- `substring` was NOT in `compileCall()` switch cases  
- Compiler treated substring as closure variable instead of builtin

## Fixes Applied
1. **compiler.go** - Added "substring" to isBuiltin() (line ~1901)
2. **compiler.go** - Added case for substring in compileCall() (line ~1615)
3. **runtime.c.tpl** - Improved null check in mph_string_substring
4. **morph.h.tpl** - Added declaration for mph_string_substring

## Checksums
- pkg/compiler/compiler.go: 77e101763be314b6e7369525fc4125b898892e6a522737828130c5b6bf152ecc
- pkg/compiler/runtime/runtime.c.tpl: 055947a565df422e51dea4dbba800703d1942796fb88e40357102888745e0b56
- pkg/compiler/runtime/morph.h.tpl: 741a650c1ee8dcb5d7927b39c55fd7a6b85dc8bd9ba82542f04fe705532dcdbe

## Test Result
```
fungsi main() void
    var s = "hello world"
    var sub = substring(s, 0, 5)
    native_print(sub)
    native_print("\n")
akhir
```
Output: `hello` ✅

## Notes
- Go installed to /usr/local/go for rebuilding morph
- Moved C files from pkg/compiler/runtime/ to c_backup/ to allow Go build
- Moved checksum_gen.go to .bak (was interfering with build)


## N1 Type Checker & Stdlib (2025-12-28 04:01)

### Type Checker v2
- All binary ops: +, -, *, /, ==, !=, <, >, <=, >=
- Unary ops: -, !
- Type rules implemented

### Symbol Table
- ID-based (string comparison too slow/OOM)
- 4 slots using global vars

### Stdlib Modules
- `std_string.fox` - str_len, str_eq
- `std_array.fox` - IntArr struct, arr_new/push/get
- `std_map.fox` - IntMap struct, imap_new/set/get

### Checksums
- n1/checker_v2.fox: ae943e2ca308fb130f21373ba48ef80c64194f088c3a3edcf37c24cb01439f5d
- n1/symtab_id.fox: f0b64fe56612997ad9b0398a53e41decc539d2dc3d93b0910db9543fc43d74eb
- n1/std_string.fox: 50828f1407ad679eb15ecc299dca7fc8660873205c7db4d1e913f3619fb3fe1d
- n1/std_array.fox: 18b0d4b915b3150d0ab43f0ca712d0ce5b02511efa4d7df68a3b7ebf00751489
- n1/std_map.fox: caee0f6e39eed836d716bf3fc429c710b4271d4e37f4b16ec14968c91b01f9a1


## Import System Investigation (2025-12-28 04:12)

### Findings
- Import syntax: `dari "module" ambil Func1, Func2`
- Export: uppercase names only
- Search paths: rootDir, stdlib, n1

### Bug Found
- Imported vars/functions stored in `_env` closure
- `_env` is NULL when calling main
- Causes segfault on any import usage

### Workaround
- Created `checker_integrated.fox` - all-in-one without imports
- Works correctly with symbol table + type checker

### Checksums
- n1/checker_integrated.fox: 87c48a0fcd71fa5480769cd6e1e3c4daa7b0d2ae455e645be3def3e8abcdeace
- cmd/morph/main.go: 2ee59b9f8a7c89e67fbf52b062653f08ac1f08d7ef127d4190f97ba792becccb


## Import Bug Fix (2025-12-28 04:20)

### Root Cause
- Imported identifiers treated as captured vars → `_env->X`
- `_env` is NULL in main → segfault

### Fix Applied (compiler.go)
1. Added `importOrigins map[string]string` to Compiler struct
2. In `collectGlobals`: track imported identifiers and their module prefix
3. In `compileExpression`: check `importOrigins` before `currentGlobals`
4. In `compileCall`: check `importOrigins` for function calls

### Test Results
- `dari "std_types" ambil GetKindInt, GetKindStr` → ✅ Works
- `dari "std_types" ambil KIND_INT, KIND_STR` → ✅ Works

### Checksum
- pkg/compiler/compiler.go: 8c7882c21fc72c2fbd68d7ad1873d1a4d25e59b132e5cecdb87203bc48f8fed9
