# 0589 — The toolchain will be reproducible and every gate will fail reliably

Status: open | Priority: P0 | Stage: UT → All quality gate | Reviewed: 2026-10-01
Depends on: none.

## Evidence

- LLVM-only policy (2026-10-01): Clang 19, libc++/libc++abi, compiler-rt, LLVM libunwind and LLD; no GCC target/install/integration recipe. CMake verifies a C++23 compile/link probe; syntax/cost scripts select the same standard library. Existing libstdc++ binaries cannot be ABI proof. This host lacks libc++-19-dev, libc++abi-19-dev and libunwind-19-dev; compiler-rt/LLD are installed. Install via `scripts/install.sh`, configure a fresh B directory, rebuild runtime/apps and run complete frozen verification. Do not edit the two live Chart inputs.
- Own-only checked-output image: Clang/GCC toolchain 112/112; three actual-transpiler controls green, archived emitter five assertions red each. Same 92 local cases/26 connection failures. All 24,350 generated files byte-identical; UT 80/2,310 unchanged/unexecuted. `build/write-status-20261001/{README.md,source-identity.json,proof-exits.json,artifacts/}`; prior semantic proofs remain indexed in README/0013.
- Reproduced false success: a directory replacing a header returned 0, claimed emission and lost its owned sentinel to Sweep. Shared WriteFile now checks close/status for objects/options/absent/reaches; ReportOutput cleans only after successful translation and retains support headers. Failed controls preserve prior bodies/sentinels and source counts. No atomic whole-tree publication claim.
- Main targeted findings 7 → 7; no new failure mode, Scan complexity 92 → 91. No baseline/suppression increase; full lint remains red. Prior 182-unit baseline remains independently preserved.
- Latest completed frozen run `20260930T210740Z-797012`: test/GCC pass; all/UT/tree/apps fail; source/post-source hashes agree. G1 is not proved. PostgreSQL localhost:5433 gives no response; container access is restricted. No demo/master mutation.
- Integrated controls cover failed/cached/warning/empty header tallies, SOURCE/SWEEP body entry, independent populations, source freezing, original System-package provenance and foundation link direction. Current body-tool receipts: `build/body-gate-20261001/`; later runtime/generator prototypes remain own-only. Earlier controls/results are indexed in README, not repeated here.
- Full analysis, Doxygen and suppressions remain red. Local gates cannot establish ERP completeness. Only the current LLVM recipe is supported; superseded instructions belong in Git history.
- Remaining ownership gaps: `Formatted` shares `/tmp/agiru-format-in` (no observed race claimed). Own naming prototype now preserves all 24,359 files' bytes/mtimes on full repeat; slice 14,213 → 14,219 retains original identities/order. Frozen activation remains 0033/0013. Transpiler UT layout heuristic still differs from the independent manifest (0058).

## Implementation

1. Preserve actual nonzero statuses and complete inventories in frozen integration; keep one six-job lane and focused two-job gates. Check core/BCApps/System/generated hashes and every missing/refused/crashed result.
2. Finish compiler-input ownership: core files()/digest() still treat directory/file links differently from the strict dependency freezer. Current src/include/cmake have no links. Serialize lint with command-database reconfiguration; alternate B directories retarget the shared symlink.
3. Add full generated-app Clang and standalone no-PCH/no-unity checks with 0038 on Linux x86_64/aarch64. Check actual symbol/header edges and reaches/app cycles; retain independent foundation links.
4. Isolate formatter input per invocation or stream it to the process; check formatter input/output/status. Exercise two worktrees without touching user data. Activate the checked writer only with frozen verification; keep all failure/no-sweep controls.
5. Repair analysis in bounded batches: lifetime, direct includes, initialization/constness, complexity/state. Remove suppressions; repair public overload Doxygen, starting at Table.h. Never raise baselines.
6. Measure clean/no-op/runtime/generator/header edits and per-run ccache hits. Retain byte-identical output mtimes, stable bounded unity roots and date/time-macro exclusion.

## Acceptance

- Make/tests/analysis/Doxygen/full Clang builds report actual outcomes; missing executables, empty results, tool failures and printed translation refusals cannot pass.
- Retain negative controls for frozen-source immutability, stale images, unique artifacts, detached publication, unknown lint units and local unity insertion/splitting.
- SOURCE/SWEEP enter complete inventories without header preflight. A first-gap prefix is not full compilation/link proof.
- Formatting two distinct inputs concurrently returns each caller's own bytes. Failed output writes return nonzero, remain counted and do not masquerade as completed emission.
- Compare unchanged populations/images and investigate losses before semantic activation (0058).

## References

Code: `Makefile`, `CMakeLists.txt`, `src/tc/Main.cpp::{ReportFailures,WriteFile}`, `src/gen/Format.cpp`, `scripts/{verify_snapshot.py,unity_groups.py}`, `test/{run.sh,lint.sh,lint-analysis.py,toolchain.py}`. Current frozen logs: README. AL semantics: owning WI, not this item.
