# 0589 — The toolchain will be reproducible and every gate will fail reliably

Status: open | Priority: P0 | Stage: UT → All quality gate | Reviewed: 2026-09-30
Depends on: none.

## Evidence

- Snapshot `20260928T155053Z-49374`: tree recorded 6 failed/9,872 headers but target_exits.tree is 0. Repaired scripts/tree_syntax.sh now exits 1 on failed headers and uses Werror for headers/PCH. Cache identity includes compiler/flags. Six real-compiler controls pass (valid, failed/cached failure, warning, empty, missing, permissive-cache transition); full toolchain suite 54/54. Frozen original script fails invalid-header and warning controls. Artifacts: build/development-toolchain/. New full-tree measurement pending; no old result relabeled green.
- Frozen verification and a reusable serialized lane exist; source hashes and artifacts are retained per run. Stable unity groups use 896 roots, split only above 32 sources.
- Current full analysis: 175/175 units, 161 failed, 1,042 unique diagnostics. Doxygen: 1,502 warnings; suppression directives: 55 versus 13 allowed. Artifacts are in README; no baseline increase.
- `make gcc` covers runtime/transpiler/gates, not the complete generated apps. Ordinary `make test` does not establish complete ERP behavior.
- Integrated MilestoneGate fixture isolation clears inherited MAKEFLAGS/MFLAGS/MAKEOVERRIDES; parent B=build/gcc previously relocated ut.log outside the fixture's asserted evidence path. Explicit parent-output control is red before repair; full toolchain 66/66 both. Serialize Clang/GCC gate execution on their shared dedicated database: concurrent suites reproduced a Resource Cost drop/read race; serialized GCC suite 86/86. This is fixture ownership, not a production runner relaxation.
- Integrated: three session/handler-dependent bridge bodies moved unchanged from net to `src/rt/dotnet/`. Empty reaches lists now derive Linux no-undefined links, without a second tier map. Clang/GCC independently link al/net/db; actual production-link injection rejects a missing runtime edge and succeeds only with its guard removed. SessionBridge 16/16 on new and archived old runtime; own suites 88 cases/68 toolchain green both, main 88/68 green. Lint 177/188, 170 failed; no bridge/gate-owned finding or baseline increase. Own `build/foundation-proof/`, main `build/foundation-main-tests.log`. Public-header dependency audit remains open.

## Implementation

1. Keep compiler/tool-failure controls and nonzero tree tallies in frozen integration. Snapshot 185800 reports five failures/9,872 headers with tree exit 2. Keep focused two-job gates and one six-job integration lane; verify source/BCApps/generated-image provenance and preserve all tool/process exits.
   Serialize lint with compiler-configuration changes in the same worktree: make db/test with B=build/gcc retargets the shared compile_commands.json symlink. A running analysis must retain one configured command database; separate build directories alone do not isolate it.
2. Add complete generated-app GCC and standalone no-PCH/no-unity checks with 0038. Check reaches/app cycles and actual header dependencies.
   Extract a foundation error payload from runtime transaction helpers. Audit actual symbol/header edges, not reaches declarations alone; retain independent foundation links and rejection controls. net must remain context-free.
3. Repair analysis in bounded batches: lifetime traces first (0035/0718), direct includes, initialization/constness, then complexity/state. Remove silent suppressions; never increase baselines or auto-fix public AL signatures indiscriminately.
4. Repair Doxygen overload contracts and malformed tags in include/, beginning with Table.h. Remove obsolete implementation claims.
5. Measure clean/no-op/runtime-edit/generator-edit/public-header-edit costs and per-run ccache hits. Keep output mtimes for byte-identical files; avoid time macros.

## Acceptance

- `make`, `make test`, changed/full `make lint`, Doxygen and complete GCC checks report actual outcomes; no missing executable, empty analyzer result or failed tool can pass.
- Negative controls retain stale-image refusal, source immutability, unique artifacts, detached-result publication, unknown lint unit and local unity insertion/split behavior.

## References

Code: `Makefile`, `CMakeLists.txt`, `scripts/{verify_snapshot.py,unity_groups.py}`, `test/{run.sh,lint.sh,lint-analysis.py,toolchain.py}`. Logs/artifacts: latest frozen run in README. AL semantics are outside this WI.
