# 0039 — The AL runner will control test lifecycle and isolation explicitly

Status: open | Priority: P0 | Stage: UT | Reviewed: 2026-10-02
Depends on: 0012 boundaries; 0058 comparison; 0718 images; 0006 session state.

## Evidence

- RequiredTestIsolation preflight now reads the same immutable `CodeunitDef` as the generated catalogue. None/omitted permit every valid policy; Disabled/Codeunit/Function require an exact match. Invalid/mismatched declarations produce one failed result per source method before construction/OnRun. Gate: 430 checks, including two-connection no-effect proof; actual generated AL: nine checks. Removed metadata and bypassed policy controls fail four/130 checks. `test/required-isolation.sh`, `build/isolation-tests-final.log`: 102 local cases/147 toolchain tests, zero red/skipped. This does not activate the translated TestRunner.
- BCApps `6261b1c458` generation now exits zero, retaining 80 UT codeunits/2,314 methods and all 24,375 output paths. Exactly 1,315 test-catalogue sources change; normalization of their constructor migrations leaves zero additional byte changes, and all headers are unchanged. `build/isolation-output-comparison.json`. Repeated generation is byte-identical, but six rewrites/two sweeps remain 0589. The new `Create Company Tests` declaration (139326, Disabled) is counted, not excluded or forced into Codeunit isolation. `build/isolation-{generation,generation-final}.log`, `build/isolation-*-generation.sha256`; no executed-UT claim.
- `TestRunner.cpp` invokes test methods directly; generated TestRunner before/after hooks and PermissionTestHelper reset chain are not driving execution.
- Codeunit/Function/Disabled policies and handler/trap cleanup have focused gates. CLI defaults to Codeunit; platform default is Disabled.
- `TransactionModel::None` currently avoids success Commit but does not itself refuse direct writes or give page interactions separate transactions.

## Implementation

1. Generate runner metadata and callback registration; select a translated TestRunner explicitly. Dispatch empty-function codeunit callbacks, skips, OnRun, OnBeforeTestRun and OnAfterTestRun.
2. Run each before/after hook in its documented own transaction outside test isolation floors. Implement hook-error outcomes and durable reports separately from failed test data.
3. Complete the TestIsolation × TransactionModel matrix and runner selection; retain the proved RequiredTestIsolation preflight. Codeunit may retain successful cross-method state; Function discards each method; AutoRollback rejects explicit Commit; None rejects direct writes but permits documented page transactions.
4. Implement PermissionTestHelper and the translated reset chain, then retire native duplicates. Restore handlers/traps/current instance on setup, execution, callback and reporting failures.

## Acceptance

- Two-connection gates prove hook transactions, explicit Commit, nested rollback, successful cross-method state, False before-hook, failing after-hook and None page/direct-write distinction.
- Same complete codeunits and sealed seed before/after; unchanged population and no unexplained losses. Full UT goes through the translated runner.

## References

Code: `src/rt/TestRunner.cpp`, `include/runtime/TestRunner.h`, `src/gen/CodeunitWriter.cpp`, `src/cli/Main.cpp`; gates: `TestIsolationGate`, `HandlerLifetimeGate`. Platform: TestIsolation property, TransactionModel/HandlerFunctions attributes, TestRunner triggers. AL: `Tools/Test Framework/Test Runner/src`. Predecessor: WI-963/1088/1316; method sampling changes isolation evidence.

RequiredTestIsolation: developer `properties/devenv-requiredtestisolation-property.md` (runtime 16); BCApps `Layers/W1/Tests/Company Creation/CreateCompanyTests.Codeunit.al` and `Tools/Test Framework/Test Runner/src/TestSuiteMgt.Codeunit.al::{SelectTestMethodsByRange,SelectTestMethodsByExtensionAndTestCategorization}`; generator coverage `src/tc/Main.cpp::{kTranslatedProperties,kPartlyTranslatedProperties}` and carrier `include/meta/CodeunitDef.h::requiredTestIsolation`.
Revisions: developer docs `ff5939a46e`, BCApps `6261b1c458`, user docs `634710c42e` (no RequiredTestIsolation reference found). Predecessor 963/1088 identifies cross-method state losses; do not adopt isolation merely to maximize green counts.

Property scope: `requiredtestisolation`, `subtype`, `subtype-blob`, `subtype-codeunit`, `testisolation`, `testtype`.
