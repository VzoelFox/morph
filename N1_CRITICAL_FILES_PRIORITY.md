# 🎯 N1 CRITICAL FILES - PRIORITY ORDER
## File Mana yang PALING DIBUTUHKAN Pertama Kali?

**Date**: 2025-12-28
**Status**: 🔥 **ACTION PLAN**
**Goal**: Path tercepat ke working compiler

---

## 🔥 PRIORITY 1: TRANSPILER (CRITICAL!)

### File: `n1/transpiler_minimal.fox`
**Lines**: ~300 lines
**Status**: 🚫 **MISSING - BLOCKING EVERYTHING!**
**Impact**: **TANPA INI, TIDAK ADA OUTPUT!**

**Kenapa ini PALING CRITICAL?**
```
✅ Kita SUDAH punya type checker (checker_integrated.c)
✅ Kita SUDAH punya symbol table (symtab_id.c)
✅ Kita SUDAH punya stdlib (std_array, std_map, std_string)
🚫 TAPI TIDAK ADA cara untuk generate C code!

TRANSPILER adalah SATU-SATUNYA file yang blocking semua!
```

### Apa yang harus dilakukan Transpiler?

#### Input: Simple IR (Intermediate Representation)
```fox
# Contoh IR sederhana:
struktur Expr
    expr_type int  # 0=int_literal, 1=binary_op, 2=var_ref
    int_value int
    op string
    left_idx int
    right_idx int
    var_name string
akhir

struktur Function
    name string
    param_count int
    p0_name string
    p0_type int
    return_type int
    expr_count int
    # ... expressions array (inline, max 20)
akhir
```

#### Output: C Code
```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 10;
    int y = 20;
    int result = add(x, y);
    return result;
}
```

### Implementasi Minimal (300 lines):

```fox
# n1/transpiler_minimal.fox

# Expression types
var EXPR_INT_LIT = 0
var EXPR_VAR_REF = 1
var EXPR_BINARY = 2
var EXPR_CALL = 3
var EXPR_RETURN = 4

# Simple IR
struktur Expr
    expr_type int
    int_val int
    str_val string
    op string
akhir

struktur Func
    name string
    ret_type int
    param_count int
    p0_name string
    p0_type int
    p1_name string
    p1_type int
    expr_count int
    e0 Expr
    e1 Expr
    e2 Expr
akhir

# Codegen functions
fungsi type_to_c(t int) string
    jika t == 0
        kembalikan "int"
    akhir
    jika t == 2
        kembalikan "char*"
    akhir
    jika t == 4
        kembalikan "void"
    akhir
    kembalikan "int"
akhir

fungsi gen_expr(e Expr) string
    jika e.expr_type == EXPR_INT_LIT
        kembalikan int_to_string(e.int_val)
    akhir
    jika e.expr_type == EXPR_VAR_REF
        kembalikan e.str_val
    akhir
    jika e.expr_type == EXPR_BINARY
        # Simplified: assume str_val has "left op right"
        kembalikan e.str_val
    akhir
    kembalikan "0"
akhir

fungsi gen_function(f Func) void
    # Return type
    native_print(type_to_c(f.ret_type))
    native_print(" ")
    native_print(f.name)
    native_print("(")

    # Parameters
    jika f.param_count > 0
        native_print(type_to_c(f.p0_type))
        native_print(" ")
        native_print(f.p0_name)
    akhir
    jika f.param_count > 1
        native_print(", ")
        native_print(type_to_c(f.p1_type))
        native_print(" ")
        native_print(f.p1_name)
    akhir

    native_print(") {\n")

    # Body expressions
    var i = 0
    selama i < f.expr_count
        native_print("    ")
        jika i == 0
            native_print(gen_expr(f.e0))
        akhir
        jika i == 1
            native_print(gen_expr(f.e1))
        akhir
        jika i == 2
            native_print(gen_expr(f.e2))
        akhir
        native_print(";\n")
        i = i + 1
    akhir

    native_print("}\n\n")
akhir

fungsi main() void
    # Test: Generate simple function
    var add_func = Func{
        name: "add",
        ret_type: 0,
        param_count: 2,
        p0_name: "a",
        p0_type: 0,
        p1_name: "b",
        p1_type: 0,
        expr_count: 1,
        e0: Expr{expr_type: EXPR_RETURN, str_val: "return a + b"},
        e1: Expr{},
        e2: Expr{}
    }

    gen_function(add_func)
akhir
```

**Output yang diharapkan:**
```c
int add(int a, int b) {
    return a + b;
}
```

**Timeline**: 2-3 hari untuk basic version

---

## 🟠 PRIORITY 2: PARSER MINIMAL (HIGH)

### File: `n1/parser_minimal.fox`
**Lines**: ~200 lines
**Status**: 🚫 **MISSING**
**Impact**: **Need to convert source → IR**

**Kenapa Priority 2?**
```
Kita perlu convert Fox source code → IR yang bisa ditranspile
Bisa start dengan SANGAT SEDERHANA (hanya fungsi + return)
```

### Implementasi Minimal:

```fox
# n1/parser_minimal.fox
# Parse ONLY: fungsi, return, binary ops

struktur Token
    type int
    value string
akhir

# Simple recursive descent parser
fungsi parse_function(tokens []Token) Func
    # Skip "fungsi"
    # Get name
    # Parse params
    # Parse body
    # Return Func struct
akhir

fungsi parse_return(tokens []Token) Expr
    # Skip "kembalikan"
    # Parse expression
    # Return Expr struct
akhir
```

**Timeline**: 3-4 hari untuk basic parser

---

## 🟡 PRIORITY 3: LEXER MINIMAL (MEDIUM)

### File: `n1/lexer_minimal.fox`
**Lines**: ~150 lines
**Status**: 🚫 **MISSING**
**Impact**: **Need to tokenize source**

**Kenapa Priority 3?**
```
Perlu tokenize input, tapi bisa mulai dengan SANGAT MINIMAL
Hanya support: identifier, int, keywords (fungsi, kembalikan)
```

### Implementasi Minimal:

```fox
# n1/lexer_minimal.fox
# Tokenize ONLY: fungsi, identifier, int, kembalikan, operators

fungsi tokenize(input string) []Token
    # Very simple character-by-character
    # Return array of tokens
akhir
```

**Timeline**: 2-3 hari untuk basic lexer

---

## 🎯 RECOMMENDED ACTION PLAN

### Week 1: Transpiler Foundation
```
Day 1-2: Create transpiler_minimal.fox
         - IR structures (Expr, Func)
         - gen_function() untuk simple case
         - Test: Generate "int add(int a, int b) { return a + b; }"

Day 3-4: Extend transpiler
         - Support variables (var x = 10)
         - Support binary ops (a + b, a * b)
         - Test: Generate more complex functions

Day 5-7: Integration with checker
         - Use checker_integrated untuk type info
         - Connect types → C types
         - Test: Type-checked transpilation
```

### Week 2: Parser Foundation
```
Day 8-10: Create parser_minimal.fox
          - Parse function declaration
          - Parse return statement
          - Parse binary expressions
          - Test: Parse simple function

Day 11-12: Connect Parser → Transpiler
           - Parser output → IR
           - IR → Transpiler
           - Test: End-to-end (source → C code)

Day 13-14: Bug fixes + polish
```

### Week 3: Lexer + Integration
```
Day 15-17: Create lexer_minimal.fox
           - Tokenize keywords, identifiers, numbers
           - Test: Tokenize simple program

Day 18-19: Full pipeline
           - Source → Lexer → Parser → Checker → Transpiler → C
           - Test: Compile hello world

Day 20-21: End-to-end testing
```

### Week 4: Polish + Expand
```
Day 22-24: Add more features
           - If/else support
           - While loops
           - More operators

Day 25-28: Production ready
           - Error handling
           - Better error messages
           - Documentation
           - Self-hosting test (compile N1 dengan N1!)
```

---

## 📋 CONCRETE FIRST STEPS (TODAY!)

### Step 1: Create `n1/transpiler_minimal.fox` (RIGHT NOW!)

```bash
cd /root/morph/n1

cat > transpiler_minimal.fox << 'EOF'
# N1 Minimal Transpiler - Day 1
# Generate C code from simple IR

var TYPE_INT = 0
var TYPE_VOID = 4

struktur Expr
    kind int  # 0=return, 1=binary, 2=literal
    value string
akhir

struktur Func
    name string
    ret_type int
    body string
akhir

fungsi type_to_c(t int) string
    jika t == TYPE_INT
        kembalikan "int"
    akhir
    kembalikan "void"
akhir

fungsi gen_func(f Func) void
    native_print(type_to_c(f.ret_type))
    native_print(" ")
    native_print(f.name)
    native_print("() {\n    ")
    native_print(f.body)
    native_print("\n}\n")
akhir

fungsi main() void
    native_print("/* Generated by N1 Transpiler v0.1 */\n\n")

    # Test: Generate simple function
    var test = Func{name: "add", ret_type: TYPE_INT, body: "return 42"}
    gen_func(test)
akhir
EOF
```

### Step 2: Compile & Test

```bash
cd /root/morph
./morph build n1/transpiler_minimal.fox

# Should output:
# /* Generated by N1 Transpiler v0.1 */
#
# int add() {
#     return 42
# }
```

### Step 3: Expand Incrementally

```bash
# Day 1: Simple function generation ✅
# Day 2: Add parameters support
# Day 3: Add multiple statements
# Day 4: Add binary operators
# Day 5: Integration with checker_integrated
```

---

## 🎯 WHY THIS ORDER?

### 1. Transpiler First = Immediate Value
```
✅ See C code output immediately
✅ Can test codegen independently
✅ Motivating - see progress fast!
✅ Unblocks everything else
```

### 2. Parser Second = Connect to Real Source
```
✅ Can parse actual .fox files
✅ Generate IR for transpiler
✅ Starting to look like real compiler
```

### 3. Lexer Last = Complete Pipeline
```
✅ Now have full source → executable pipeline
✅ Can accept raw .fox text
✅ Fully working compiler!
```

---

## ⚠️ ANTI-PATTERNS TO AVOID

### ❌ Don't Do This:
```
❌ "Let me design the perfect AST first" - NO! Too slow
❌ "Let me build complete lexer with all features" - NO! Overkill
❌ "Let me implement full type system" - NO! Already have checker_integrated
❌ "Let me port all N0 features" - NO! Start minimal
```

### ✅ Do This Instead:
```
✅ Build transpiler_minimal.fox TODAY (300 lines)
✅ Test with hardcoded IR
✅ Expand incrementally (one feature per day)
✅ See working C code output ASAP
✅ Keep it SIMPLE and WORKING
```

---

## 📊 SUCCESS METRICS

### Week 1 Success:
```
✅ transpiler_minimal.fox compiles
✅ Generates valid C code for simple function
✅ Can handle: function, return, int literal
✅ Output compiles with gcc
```

### Week 2 Success:
```
✅ parser_minimal.fox works
✅ Can parse simple .fox source
✅ End-to-end: source → IR → C code
✅ Can compile: fungsi add(a int, b int) int { kembalikan a + b }
```

### Week 3 Success:
```
✅ lexer_minimal.fox works
✅ Full pipeline: text → tokens → AST → IR → C → binary
✅ Can compile hello world program
✅ Working compiler!
```

### Week 4 Success:
```
✅ Support if/else, while loops
✅ Better error messages
✅ Can compile N1's own transpiler (SELF-HOSTING!)
✅ Production ready
```

---

## 🎉 BOTTOM LINE

**File yang PALING DIBUTUHKAN:**

### 🔥 #1: `n1/transpiler_minimal.fox` (300 lines)
**START THIS TODAY!**
- Paling critical
- Unblocks everything
- Immediate visible progress
- Timeline: 2-3 hari

### 🟠 #2: `n1/parser_minimal.fox` (200 lines)
**START NEXT WEEK**
- Needed untuk parse source
- Connects to transpiler
- Timeline: 3-4 hari

### 🟡 #3: `n1/lexer_minimal.fox` (150 lines)
**START WEEK 3**
- Completes pipeline
- Timeline: 2-3 hari

**TOTAL TIMELINE**: 4 minggu untuk working compiler dari nol!

---

**Apakah saya boleh membantu create `transpiler_minimal.fox` SEKARANG?** Ini adalah file paling critical dan akan unblock semua progress!
