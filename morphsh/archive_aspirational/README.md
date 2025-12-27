# Aspirational Optimizer Archive

**Status**: ARCHIVED - Never Successfully Compiled
**Date Archived**: 2025-12-27
**Reason**: OOM at 31GB RAM for 283-line files

## Contents

1. `lexer_optimized_ASPIRATIONAL.fox` (10KB)
2. `parser_optimized_ASPIRATIONAL.fox` (11KB)
3. `compiler_optimized_ASPIRATIONAL.fox` (11KB)

**Total**: ~32KB of aspirational code

## History

These files were created with ambitious optimization goals:
- Switch case flattening for O(1) token/node type matching
- Multi-case grouping
- Expected 20-40x performance improvement

**Problem**: N0 (Go compiler) has fundamental memory limitations:
- Parser/TypeChecker creates 100K+ objects for large files
- No object pooling or reuse
- OOM at 31GB RAM for 283-line file
- NOT solvable by increasing RAM or GC thresholds

## Test Results

All attempts to compile these files resulted in OOM:
```bash
$ ./morph build lexer_optimized.fox
# Process killed at 31GB RAM usage

$ /tmp/n0-workspace/morph_n0 build lexer_optimized.fox
# Process killed at 31GB RAM usage

$ /root/morph/morph_working build lexer_optimized.fox
# Process killed at 31GB RAM usage (from commit ab756bc)
```

**Conclusion**: Issue is NOT in binary, but in N0 compiler algorithm itself.

## Architectural Issues Found

1. **Over-Engineering**: Switch case flattening too aggressive
2. **Lost Features**: Recursive functions removed during optimization
3. **No Validation**: Never successfully compiled or tested
4. **Unrealistic Benchmarks**: SWITCH_CASE_OPTIMIZATION.md claims "COMPLETE" with no proof

## Lessons Learned

1. **Self-hosting != Self-extending**: N1 can compile itself but cannot create new language features
2. **Test as you build**: Large refactors without incremental testing are risky
3. **Memory matters**: Algorithm complexity > raw RAM size
4. **Be honest**: "Complete" should mean "tested and working"

## Replacement Strategy

See `/root/morph/morphsh/lexer_minimal.fox` (NEW) for realistic, incremental approach:
- Start with single optimization
- Test after each change
- Validate memory usage
- Prove performance improvement with benchmarks

## References

- Investigation: Commit `d0cddd5` (2025-12-27)
- Analysis: `.vzoel.jules/session-2025-12-27-memory-investigation.md`
- Root Cause: N0 parser creates exponential objects for large files

---
**Note**: These files are preserved for historical reference and learning.
DO NOT attempt to compile without fixing N0's fundamental memory issues first.
