# 0039 — The AL runner will control test lifecycle and isolation explicitly

Status: open | Priority: P0 | Stage: UT | Reviewed: 2026-10-03
Depends on: 0012 boundaries; 0058 comparison; 0718 images; 0006 session state.

## Evidence

- RequiredTestIsolation preflight now reads the same immutable `CodeunitDef` as the generated catalogue. None/omitted permit every valid policy; Disabled/Codeunit/Function require an exact match. Invalid/mismatched declarations produce one failed result per source method before construction/OnRun. Gate: 430 checks, including two-connection no-effect proof; actual generated AL: nine checks. Removed metadata and bypassed policy controls fail four/130 checks. `test/runtime/required-isolation.sh`, `build/isolation-tests-final.log`: 102 local cases/147 toolchain tests, zero red/skipped. This does not activate the translated TestRunner.
- BCApps `6261b1c458` generation now exits zero, retaining 80 UT codeunits/2,314 methods and all 24,375 output paths. Exactly 1,315 test-catalogue sources change; normalization of their constructor migrations leaves zero additional byte changes, and all headers are unchanged. `build/isolation-output-comparison.json`. Repeated generation is byte-identical, but six rewrites/two sweeps remain 0589. The new `Create Company Tests` declaration (139326, Disabled) is counted, not excluded or forced into Codeunit isolation. `build/isolation-{generation,generation-final}.log`, `build/isolation-*-generation.sha256`; no executed-UT claim.
- `TestRunner.cpp` invokes test methods directly; generated TestRunner before/after hooks and PermissionTestHelper reset chain are not driving execution.
- Codeunit/Function/Disabled policies and handler/trap cleanup have focused gates. CLI defaults to Codeunit; platform default is Disabled.
- `TransactionModel::None` currently avoids success Commit but does not itself refuse direct writes or give page interactions separate transactions.
- Runtime-18 context primitives retain provider/test-app identities, owned callback
  metadata and phase-limited Skip authority across AL value copies. TestContextGate
  passes 56 checks; generated AL passes seven with an altered-provider negative control.
  `/tmp/agiru-verify/b3fb41b94d2994ba/20261003T090518Z-178436/result.json` proves
  these primitives, not production hook selection/dispatch/transactions or data-driven
  execution. Private controller: `src/rt/TestContextScope.h`; unbound variables refuse
  conservatively, not as a measured BC default. No source test identity leaves the total.

## Implementation

1. Generate runner metadata and callback registration; select a translated TestRunner explicitly. Dispatch empty-function codeunit callbacks, skips, OnRun, OnBeforeTestRun and OnAfterTestRun.
2. Run each before/after hook in its documented own transaction outside test isolation floors. Implement hook-error outcomes and durable reports separately from failed test data.
3. Complete the TestIsolation × TransactionModel matrix and runner selection; retain the proved RequiredTestIsolation preflight. Codeunit may retain successful cross-method state; Function discards each method; AutoRollback rejects explicit Commit; None rejects direct writes but permits documented page transactions.
4. Implement PermissionTestHelper and the translated reset chain, then retire native duplicates. Restore handlers/traps/current instance on setup, execution, callback and reporting failures.
5. With 0034's Runtime-18 context binding, implement declared TestHandlers and native
   before/after codeunit/procedure/case dispatch. Context belongs to the active test/session;
   before-procedure/case Skip prevents execution and reports its reason; after-hook Skip
   has no effect. For ITestDataSource, enumerate stable nonblank, bracket-free, distinct
   identifiers before executing any case; materialize only executing cases in order.
   Keep source method identities and discovered case totals separate and fully reported.

## Acceptance

- Two-connection gates prove hook transactions, explicit Commit, nested rollback, successful cross-method state, False before-hook, failing after-hook and None page/direct-write distinction.
- Same complete codeunits and sealed seed before/after; unchanged population and no unexplained losses. Full UT goes through the translated runner.
- Context getters, permitted/prohibited Skip phases, lazy retrieval, invalid/duplicate
  identifiers and concurrent-session isolation have generated AL execution/negative controls;
  skipped/unmaterialized cases remain counted, never treated as passed methods.

## References

Code: `src/rt/TestRunner.cpp`, `include/runtime/TestRunner.h`, `src/gen/CodeunitWriter.cpp`, `src/cli/Main.cpp`; gates: `TestIsolationGate`, `HandlerLifetimeGate`. Platform: TestIsolation property, TransactionModel/HandlerFunctions attributes, TestRunner triggers. AL: `Tools/Test Framework/Test Runner/src`. Predecessor: WI-963/1088/1316; method sampling changes isolation evidence.

RequiredTestIsolation: developer `properties/devenv-requiredtestisolation-property.md` (runtime 16); BCApps `Layers/W1/Tests/Company Creation/CreateCompanyTests.Codeunit.al` and `Tools/Test Framework/Test Runner/src/TestSuiteMgt.Codeunit.al::{SelectTestMethodsByRange,SelectTestMethodsByExtensionAndTestCategorization}`; generator coverage `src/tc/Main.cpp::{kTranslatedProperties,kPartlyTranslatedProperties}` and carrier `include/meta/CodeunitDef.h::requiredTestIsolation`.
Revisions: developer docs `ff5939a46e`, BCApps `6261b1c458`, user docs `634710c42e` (no RequiredTestIsolation reference found). Predecessor 963/1088 identifies cross-method state losses; do not adopt isolation merely to maximize green counts.

Property scope: `requiredtestisolation`, `subtype`, `subtype-blob`, `subtype-codeunit`, `testisolation`, `testtype`.

Native contexts: developer `ff5939a46e`, `methods-auto/{testhandlercontext,datasourcecontext}/`
and `testhandlercontext-skip-method.md`; original verified System-29 `src/System Interfaces/
{ITestHandler.Interface.al,TestFramework/ITestDataSource.interface.al}` supplies signatures,
default bodies and stable-case/lazy-retrieval guarantees. BCApps main `bb7111877f`, user docs
`0ff62b2266` and predecessor board have no context-type usage/implementation finding.
Native source/type binding is 0034; test lifecycle/execution remains here.
