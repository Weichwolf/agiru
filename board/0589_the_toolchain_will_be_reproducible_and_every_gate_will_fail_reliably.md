# 0589 — The toolchain will be reproducible and every gate will fail reliably

Status: open | Priority: P0 | Stage: UT → All quality gate | Reviewed: 2026-09-28
Depends on: none.

## Evidence

- Frozen verification and a reusable serialized lane exist; source hashes and artifacts are retained per run. Stable unity groups use 896 roots, split only above 32 sources.
- Current full analysis: 175/175 units, 161 failed, 1,042 unique diagnostics. Doxygen: 1,502 warnings; suppression directives: 55 versus 13 allowed. Artifacts are in README; no baseline increase.
- `make gcc` covers runtime/transpiler/gates, not the complete generated apps. Ordinary `make test` does not establish complete ERP behavior.
- `net/reaches` declares no runtime dependency, but `ALConfigSettings.cpp`, `NavTenantSettingsHelper.cpp` and `NavTestExecution.cpp` call rt Session/HandlerTable. A net-only probe fails to link on those symbols. The common public include root permits the back-edge.

## Implementation

1. Keep focused two-job gates and one six-job integration lane. Verify source/BCApps/generated-image provenance and preserve all tool/process exits.
2. Add complete generated-app GCC and standalone no-PCH/no-unity checks with 0038. Check reaches/app cycles and actual header dependencies.
   Move session/test-context bridge method bodies into rt; net retains only context-free values. Extract a foundation error payload from runtime transaction helpers. Link each foundation independently with unresolved-symbol rejection; audit actual symbol/header edges, not reaches declarations alone.
3. Repair analysis in bounded batches: lifetime traces first (0035/0718), direct includes, initialization/constness, then complexity/state. Remove silent suppressions; never increase baselines or auto-fix public AL signatures indiscriminately.
4. Repair Doxygen overload contracts and malformed tags in include/, beginning with Table.h. Remove obsolete implementation claims.
5. Measure clean/no-op/runtime-edit/generator-edit/public-header-edit costs and per-run ccache hits. Keep output mtimes for byte-identical files; avoid time macros.

## Acceptance

- `make`, `make test`, changed/full `make lint`, Doxygen and complete GCC checks report actual outcomes; no missing executable, empty analyzer result or failed tool can pass.
- Negative controls retain stale-image refusal, source immutability, unique artifacts, detached-result publication, unknown lint unit and local unity insertion/split behavior.

## References

Code: `Makefile`, `CMakeLists.txt`, `scripts/{verify_snapshot.py,unity_groups.py}`, `test/{run.sh,lint.sh,lint-analysis.py,toolchain.py}`. Logs/artifacts: latest frozen run in README. AL semantics are outside this WI.
