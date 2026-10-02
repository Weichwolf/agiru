# 0038 — The complete generated tree will compile and link without slice fallbacks

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

The current build uses test/slice, unity/PCH and scripts/unlinked.py fallbacks. A green slice proves only the listed sources and can translate missing procedures into runtime refusals. Historical fixed blocker counts in the board are obsolete.

## Implementation for Sol

1. Run a fresh source/header census through make tree and rank current generic root diagnostics. Fix the highest-impact root in src/gen or public/runtime code, then recompile that root before changing the census.
2. Add compiling sources to the slice without removing existing entries. Compare standalone and unity/PCH compilation to expose accidental include dependencies.
3. Measure unresolved procedures by app and forbid fallback symbols in the complete-app build. Link every declared app dependency and verify load before running tests.
4. Record source revision, scope hash, compiler and source counts with the census. Handle duplicate generated class names deterministically from namespace/app identity.

## Acceptance

`make apps` completes under both compilers; a missing implementation makes the full build red. Slice fallbacks remain loud and counted during transition. A source deliberately removed from compilation must be detected by coverage checks.

## References

Repository: CMakeLists.txt, scripts/tree_syntax.sh, first_gap.sh, unlinked.py, test/slice, src/gen. AL: complete selected source population. Historical 36-root/compile-count claims are intentionally removed.
