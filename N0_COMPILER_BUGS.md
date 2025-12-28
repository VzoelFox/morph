# N0 Compiler Bugs & Workarounds

**Date**: 2025-12-28
**Version**: N0 compiler (morph) built from source
**Context**: N1 Self-Hosting Compiler Development

---

## Critical Bugs Discovered

### 1. Struct Return with Assignment Pattern - "multi-var not supported"

**Severity**: ⚠️ CRITICAL
**Impact**: Blocks all functions that return structs using assignment pattern

#### Bug Description:
When returning a struct from a function using assignment pattern, the compiler fails with misleading error: `multi-var not supported`

#### Failing Code:
```fox
struktur Type
    kind int
    name string
akhir

fungsi make_type(k int, n string) Type
    var t Type
    t.kind = k
    t.name = n
    kembalikan t  # ❌ FAILS: "multi-var not supported"
akhir

fungsi main() int
    var mytype = make_type(1, "test")  # This triggers the error
    kembalikan mytype.kind
akhir
```

#### Working Workaround:
Use struct literal syntax instead:
```fox
fungsi make_type(k int, n string) Type
    kembalikan Type{kind: k, name: n}  # ✅ WORKS
akhir
```

#### Root Cause:
In `pkg/compiler/compiler.go`, the check:
```go
if len(s.Names) != 1 || len(s.Values) != 1 {
    return fmt.Errorf("multi-var not supported")
}
```

Incorrectly triggers when parsing struct assignment patterns, even though they're single-variable declarations.

---

### 2. Empty String in Struct Literals - Compilation Timeout

**Severity**: ⚠️ CRITICAL
**Impact**: Any struct with string fields that might be empty cannot use literals

#### Bug Description:
Using empty string `""` in struct literal causes infinite loop/timeout during compilation

#### Failing Code:
```fox
struktur TypeResult
    result int
    error_msg string
    has_error bool
akhir

fungsi ok_result(t int) TypeResult
    kembalikan TypeResult{result: t, error_msg: "", has_error: salah}
    # ❌ TIMEOUT - compilation never completes
akhir
```

#### Working Workaround:
Use sentinel value instead of empty string:
```fox
fungsi ok_result(t int) TypeResult
    kembalikan TypeResult{result: t, error_msg: "ok", has_error: salah}
    # ✅ WORKS - use "ok" or any non-empty string
akhir
```

#### Alternative Workaround (DOESN'T WORK):
Cannot omit fields - N0 requires all struct fields in literals:
```fox
kembalikan TypeResult{result: t, has_error: salah}
# ❌ ERROR: Missing fields in struct literal
```

---

### 3. Logical Operator "dan" Not Transpiled to C

**Severity**: ⚠️ HIGH
**Impact**: Cannot use Fox logical operators; must use workarounds

#### Bug Description:
Fox keyword `dan` (and) is recognized by lexer but not transpiled to C's `&&` operator

#### Failing Code:
```fox
fungsi check_types(left Type, right Type) bool
    jika left.kind == KIND_INT dan right.kind == KIND_INT
        kembalikan benar  # ❌ C compilation error: "before 'dan'"
    akhir
    kembalikan salah
akhir
```

#### Generated C Code:
```c
if ((...) == mph_KIND_INT) dan ((...) == mph_KIND_INT) {
    // C compiler error: expected ')' before 'dan'
}
```

#### Working Workaround:
Use nested if statements:
```fox
fungsi check_types(left Type, right Type) bool
    jika left.kind == KIND_INT
        jika right.kind == KIND_INT
            kembalikan benar  # ✅ WORKS
        akhir
    akhir
    kembalikan salah
akhir
```

#### Root Cause:
In `pkg/compiler/compiler.go`, function `compileInfix`:
```go
return fmt.Sprintf("(%s %s %s)", left, ie.Operator, right), nil
```

Outputs the operator AS-IS without translation. Fox keywords like `dan`, `atau` are not mapped to C operators `&&`, `||`.

---

## Summary of Workarounds Applied to N1

### types.fox (✅ FIXED - 25/25 tests passing)

1. **Changed**: Assignment pattern → Struct literal
   - `var t Type; t.kind = k;` → `kembalikan Type{kind: k}`

2. **Changed**: Empty strings → Sentinel values
   - `error_msg: ""` → `error_msg: "ok"`

3. **Changed**: `dan` operators → Nested ifs
   - Applied to 27 instances using automated Python script
   - Result: File grew from 841 to 896 lines

### parser.fox (⏸️ TIMEOUT - needs empty string fix)
- Status: Compilation timeout
- Cause: Contains `make_var_statement("", "")` calls
- Fix needed: Replace empty strings with sentinel values

### lexer.fox (⏸️ TIMEOUT - needs simplification)
- Status: 543 lines with complex logic
- Cause: Multiple issues (empty strings, complexity)
- Fix needed: Simplify or split into smaller modules

### checker.fox (⏸️ NOT YET TESTED)
- Status: Pending types.fox completion (dependency)
- Expected issues: Similar empty string and `dan` operator usage

---

## Testing Results

### types.fox Test Suite Output:
```
✅ Test 1: Type Creation - PASSED
✅ Test 2: Type Equality - PASSED
✅ Test 3: Null Assignability - PASSED
✅ Test 4: Type Comparability - PASSED
✅ Test 5: Binary Ops - Arithmetic - PASSED
✅ Test 6: Binary Ops - Comparison - PASSED
✅ Test 7: Binary Ops - Logical - PASSED
✅ Test 8: Binary Ops - Bitwise - PASSED
✅ Test 9: Error Detection - PASSED
✅ Test 10: Prefix Operations - PASSED
✅ Test 11: Kind to String - PASSED

TEST SUMMARY: 25 Passed, 0 Failed
```

---

## Recommendations

### For N1 Development:

1. **Avoid empty strings**: Use sentinel values like `"ok"`, `"-"`, or `"none"`
2. **Avoid assignment pattern for structs**: Always use struct literals
3. **Avoid `dan`/`atau` operators**: Use nested if statements
4. **Keep files under 300 lines**: Large files timeout during compilation
5. **Test incrementally**: Compile frequently to catch bugs early

### For N0 Compiler Improvements:

1. **Fix struct assignment pattern handling**: Allow `var x Type; x.field = val; return x`
2. **Fix empty string in literals**: Debug infinite loop in struct literal parsing
3. **Add operator translation**: Map `dan` → `&&`, `atau` → `||` in C output
4. **Improve error messages**: "multi-var not supported" is misleading for struct returns
5. **Add compilation timeout detection**: Warn user instead of hanging indefinitely

---

## Files Modified

- `n1/types.fox`: Fixed and working (896 lines)
- `n1/types.fox.backup`: Original version before fixes
- `n1/types.fox.pre_dan_fix`: Version before `dan` operator fix
- `morph.backup`: Backup of potentially modified N0 binary
- Fresh N0 compiled from source to verify bugs are in original compiler

---

## Honest Assessment

**Previous AI claim**: "N0 doesn't support import statements" - **FALSE**
**Reality**: Import system works perfectly with correct module paths

**Current blockers for full N1 self-hosting**:
1. ✅ types.fox - WORKING (core type system)
2. ⏸️ lexer.fox - TIMEOUT (needs simplification)
3. ⏸️ parser.fox - TIMEOUT (empty string fixes needed)
4. ⏸️ checker.fox - UNTESTED (likely similar issues)

**Estimated completion**:
- With current workarounds: 40-50% complete (types.fox working)
- Full parser+lexer in N1: Realistic if we apply same workarounds systematically
- Timeline: Depends on addressing parser/lexer timeouts

---

**Report generated**: 2025-12-28 12:52 UTC
**Compiler tested**: morph (N0) rebuilt from source
**Test platform**: Linux 5.15.0-164-generic
