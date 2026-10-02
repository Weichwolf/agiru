# 0589 — The toolchain will be reproducible and every gate will fail reliably

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

The review repairs test discovery, redundant builds, generator-check mutation and swallowed analyzer exits. See README for measured make/test/lint outcomes. The full audit checked 167 handwritten translation units: 152 failed and 1,009 unique diagnostics were reported before repairing three missing direct includes. Doxygen emitted 1,444 warning lines after the existing recursive-class exception (baseline 0): 743 are undocumented declarations, 92 report an unclosed `<tt>` tag, and `include/runtime/Table.h` alone has 231 warning lines, including mismatched `\param` and `\return` contracts. The largest groups were missing/direct includes (292), const methods (137), literal origins (90), designated initialization (73) and public state (62). The existing silent-place counter is also 60 against baseline 13; the review adds no suppressions. These counts are findings, not permitted baselines. Default builds still compile the generated slice and all gates; CMake includes all tiers in every gate. The review separates the generated recursion-warning exception from runtime flags.

## Implementation for Sol

1. Keep one serialized build per directory. Use the selected compiler consistently across Make, syntax sweeps and formatter helpers; keep Clang and GCC build directories separate.
2. Keep the new `make gcc` gate (runtime, transpiler and C++ gates in build/gcc); extend it with a standalone generated-source compile without PCH/unity. PCH and unity must not conceal missing includes or app dependencies.
3. Repair lint in bounded batches without raising baselines: direct includes first; initialization and const correctness next; then named constants, complexity and public state. Reconcile the 60 existing suppression sites with their baseline of 13 by removing unjustified directives or fixing the underlying defects; do not raise the allowance. Analyze JSON/XML and SharedRecord lifetime traces with 0035/0718 before treating them as style. Do not run unchecked clang-tidy --fix across public headers: emitted aggregates and AL method signatures are contracts. Count actual compiled consumers and preserve tool failures.
4. Repair Doxygen contracts in include/, starting with the repeated `Table.h` overload blocks. Escape literal AL angle-bracket text that Doxygen parses as `<tt>`, then match each overload's actual parameters and return type; identify any remaining parser defect with a minimal reproducer. Missing-signature and contradictory-parameter diagnostics remain red; do not exclude whole type families.
5. Measure no-op build, one runtime source edit, one generator edit and one public-header edit. Optimize only against these measurements; batch header edits and preserve mtimes for byte-identical output.

## Acceptance

`make`, `make test`, `make lint`, a full lint run and the GCC gate must report actual outcomes. Negative controls now pass for missing gate executables, failed analyzers without diagnostics, stale generated files without source mutation, and changed headers without compiled consumers. Keep these controls green while repairing remaining diagnostics. Never hide failing C++ gates in a baseline.

## References

Repository: Makefile, CMakeLists.txt, test/run.sh, test/lint.sh, test/lint-analysis.py, test/toolchain.py, scripts/gen_builtins.py. No AL semantic is changed by build orchestration.
