# Tests - Validation & Verification

**Status**: Organized
**Components**: 9 test files

## What This Is

Test files for validating all MorphSH components:
- Bootstrap type system tests
- Evaluator tests
- Environment tests
- Integration tests

## Files

- `test_bootstrap.fox` - Bootstrap type system validation
- `test_env.fox` - Environment tests
- `test_env_minimal.fox` - Minimal environment test
- `test_env_ultra.fox` - Ultra minimal environment test
- `test_eval_minimal.fox` - Minimal evaluator test
- `test_interpreter.fox` - Interpreter integration test
- `test_lexer.fox` - Lexer tests
- `test_types_eval.fox` - Type evaluation tests
- `test_types_minimal.fox` - Minimal type tests

## Running Tests

```bash
# Run individual test
./morph morphsh/tests/test_bootstrap.fox

# Run all tests (when test runner ready)
# ./morph morphsh/tests/run_all.fox
```

## Test Status

Most tests are from bootstrap phase (Phase 1-5).
Need to update for new organized structure.

## Next Steps

1. Update import paths after reorganization
2. Create test_tree_walker.fox for evaluator
3. Create test_stdlib.fox for standard library
4. Build test runner for automated testing
