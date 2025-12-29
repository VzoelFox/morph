# CI/CD Architecture - N1 Compiler Development

## 🎯 Tujuan: Robust, Automated, Honest Testing

**Prinsip**:
- TELITI - Test semua aspects sebelum claim success
- HATI-HATI - Safety checks prevent breaking changes
- JUJUR - Report actual status, not wishful thinking

---

## 📋 Components

### 1. Test Runner (`scripts/test_runner.sh`)

**Purpose**: Run all N1 tests automatically dengan clear reporting

**Features**:
- Auto-discover test files (pattern: `n1/test_*.fox`)
- Compile each test dengan timeout (60s)
- Run each test binary dengan timeout (30s)
- Collect pass/fail results
- Generate summary report
- Exit code: 0 (all pass) or 1 (any fail)

**Output Format**:
```
╔════════════════════════════════════════╗
║  N1 Test Runner - Automated Testing   ║
╚════════════════════════════════════════╝

[1/5] Testing n1/test_codegen_literals.fox
  ✅ Compile: SUCCESS (2.3s)
  ✅ Run: SUCCESS (0.1s)

[2/5] Testing n1/test_module_import.fox
  ✅ Compile: SUCCESS (3.1s)
  ✅ Run: SUCCESS (0.2s)

════════════════════════════════════════
Summary: 5/5 PASSED (100%)
Total time: 12.4s
════════════════════════════════════════
```

**Error Handling**:
- Timeout detection
- Compilation errors captured
- Runtime errors captured
- Detailed error reporting

---

### 2. Regression Test Suite (`n1/regression_suite.fox`)

**Purpose**: Verify all previously working features still work

**Test Categories**:

1. **Module System** (6 modules):
   - token, ast, types, lexer, parser, checker imports
   - Export/import verification
   - Module prefix correctness

2. **Literal Compilation** (3 types):
   - IntegerLiteral: 42, 0, -123, 9876
   - StringLiteral: "hello", "hello\nworld", "", "quotes"
   - BooleanLiteral: benar, salah

3. **Type System** (types.fox):
   - 25/25 type operation tests
   - Type equality, assignability
   - Binary/prefix type checking

4. **Helper Utilities** (stdlib_codegen):
   - int_to_string()
   - string_escape()
   - module_to_prefix()
   - is_valid_c_ident()

**Expected**: ALL tests must pass. Any failure = regression detected.

---

### 3. Checksum Validator (`scripts/validate_checksums.sh`)

**Purpose**: Verify file integrity and detect unauthorized changes

**Features**:
- Read checksums from `.morph.vz/checksums/`
- Compute current checksums for all tracked files
- Compare expected vs actual
- Report mismatches with filename and location
- Support for N0 frozen files (should NEVER change)

**N0 Frozen Files** (CRITICAL - must not change):
```bash
pkg/lexer/lexer.go
pkg/compiler/compiler.go
pkg/parser/parser.go
pkg/checker/checker.go
cmd/morph/main.go
morph (binary)
```

**Output Format**:
```
Validating checksums...

✅ n1/token.fox - OK
✅ n1/ast.fox - OK
✅ n1/types.fox - OK
⚠️  n1/codegen.fox - MISMATCH
    Expected: a3ff44e55ff033bb1681043c863aeb3f858488c1c0b907b68edd000754f24788
    Actual:   b4ee55f66ff144cc2792154d974cbe4g969599d2c0c018c79fee111865g35899

❌ pkg/compiler/compiler.go - N0 FROZEN FILE MODIFIED!
    This is a CRITICAL violation!

Summary: 58/60 OK, 1 modified, 1 FROZEN violation
```

**Exit Codes**:
- 0: All checksums valid
- 1: Mismatch in N1 files (warning)
- 2: N0 frozen file modified (CRITICAL ERROR)

---

### 4. Pre-commit Hook (`.git/hooks/pre-commit`)

**Purpose**: Safety checks BEFORE commit is allowed

**Checks** (in order):
1. **Checksum validation** - Ensure checksums updated
2. **Test execution** - Run regression suite
3. **Build verification** - All modified .fox files compile
4. **AGENTS.md check** - Version number incremented
5. **Frozen file check** - No N0 files modified

**Behavior**:
- ✅ All checks pass → Allow commit
- ❌ Any check fails → Block commit + show error message
- User can override with `git commit --no-verify` (NOT recommended)

**Output Format**:
```
Running pre-commit checks...

[1/5] Checksum validation... ✅ PASS
[2/5] Regression tests... ✅ PASS (15.2s)
[3/5] Build verification... ✅ PASS (8.3s)
[4/5] AGENTS.md version... ✅ PASS (v1.80.0 → v1.81.0)
[5/5] Frozen file check... ✅ PASS

All checks passed! Proceeding with commit.
```

**Failure Example**:
```
Running pre-commit checks...

[1/5] Checksum validation... ✅ PASS
[2/5] Regression tests... ❌ FAIL

Error: 2/10 regression tests failed:
  - n1/test_codegen_literals.fox: Runtime error
  - n1/test_module_import.fox: Compile error

Commit BLOCKED. Fix tests before committing.
To override (NOT recommended): git commit --no-verify
```

---

### 5. Build Verification (`scripts/verify_build.sh`)

**Purpose**: Ensure all modified files compile successfully

**Features**:
- Detect modified .fox files via `git diff`
- Compile each file dengan N0 compiler
- Timeout: 60s per file
- Report compilation errors with line numbers
- Exit code: 0 (all compile) or 1 (any fail)

**Output Format**:
```
Verifying builds for modified files...

[1/3] n1/codegen.fox... ✅ COMPILED (2.1s)
[2/3] n1/test_codegen_literals.fox... ✅ COMPILED (1.8s)
[3/3] n1/parser.fox... ❌ COMPILE ERROR

Error in n1/parser.fox:
  Line 145: undeclared identifier 'lexer.NextToken'

Build verification FAILED: 2/3 compiled
```

---

### 6. CI/CD Pipeline Script (`scripts/ci_pipeline.sh`)

**Purpose**: Master script yang runs ALL checks in sequence

**Execution Flow**:
```
1. Checksum Validation → STOP if frozen files modified
2. Build Verification  → STOP if any file fails to compile
3. Test Runner         → STOP if any test fails
4. Generate Report     → Save results to .morph.vz/ci_reports/
```

**Report Format** (`.morph.vz/ci_reports/YYYY-MM-DD_HH-MM-SS.md`):
```markdown
# CI/CD Report - 2025-12-29 03:00:15 UTC

## Status: ✅ ALL CHECKS PASSED

### Checksum Validation
- Total files: 60
- Valid: 60
- Modified: 0
- Frozen violations: 0
- Result: ✅ PASS

### Build Verification
- Files tested: 12
- Compiled: 12
- Failed: 0
- Total time: 18.4s
- Result: ✅ PASS

### Test Execution
- Tests run: 5
- Passed: 5
- Failed: 0
- Total time: 15.2s
- Result: ✅ PASS

### Details
- Commit: 43f29ed
- Branch: main
- Author: VzoelFox
- Timestamp: 2025-12-29 03:00:15 UTC

---
Generated by CI/CD Pipeline v1.0.0
```

---

## 🚀 Usage Examples

### Manual Testing:
```bash
# Run all tests
./scripts/test_runner.sh

# Validate checksums
./scripts/validate_checksums.sh

# Verify builds
./scripts/verify_build.sh

# Full CI pipeline
./scripts/ci_pipeline.sh
```

### Git Integration:
```bash
# Install pre-commit hook
./scripts/install_hooks.sh

# Commit (will auto-run checks)
git commit -m "..."

# Override checks (NOT recommended)
git commit --no-verify -m "..."
```

### CI/CD Integration:
```bash
# In GitHub Actions (.github/workflows/ci.yml)
- name: Run CI/CD Pipeline
  run: ./scripts/ci_pipeline.sh
```

---

## 📊 Success Criteria

**CI/CD is "robust" when**:
- ✅ All tests auto-discovered and run
- ✅ Checksums validated on every commit
- ✅ N0 frozen files protected
- ✅ Build errors caught before commit
- ✅ Regression tests prevent breakage
- ✅ Clear, actionable error messages
- ✅ Fast execution (< 30s for typical commit)
- ✅ Reports saved for history

**CI/CD is "production-ready" when**:
- ✅ Zero false positives (no flaky tests)
- ✅ Zero false negatives (catches all regressions)
- ✅ Handles edge cases (timeouts, OOM, etc)
- ✅ Easy to debug failures
- ✅ Documented for new developers

---

## 🎯 Implementation Priority

**Phase 1** (Critical - Do First):
1. Test Runner - Core functionality
2. Checksum Validator - File integrity
3. Pre-commit Hook - Block bad commits

**Phase 2** (Important - Do Second):
4. Regression Suite - Comprehensive tests
5. Build Verification - Compile checks
6. CI Pipeline - Integration

**Phase 3** (Nice to Have):
7. GitHub Actions integration
8. Notification system (email/Slack)
9. Performance benchmarking
10. Code coverage analysis

---

**Next Steps**: Implement Phase 1 scripts dengan TELITI approach.
