# 0039 — The AL runner will control test lifecycle and isolation explicitly

Status: open | Priority: P0 | Stage: UT | Reviewed: 2026-09-28
Depends on: 0012 boundaries; 0058 comparison; 0718 images; 0006 session state.

## Evidence

- `TestRunner.cpp` invokes test methods directly; generated TestRunner before/after hooks and PermissionTestHelper reset chain are not driving execution.
- Codeunit/Function/Disabled policies and handler/trap cleanup have focused gates. CLI defaults to Codeunit; platform default is Disabled.
- `TransactionModel::None` currently avoids success Commit but does not itself refuse direct writes or give page interactions separate transactions.

## Implementation

1. Generate runner metadata and callback registration; select a translated TestRunner explicitly. Dispatch empty-function codeunit callbacks, skips, OnRun, OnBeforeTestRun and OnAfterTestRun.
2. Run each before/after hook in its documented own transaction outside test isolation floors. Implement hook-error outcomes and durable reports separately from failed test data.
3. Implement the complete TestIsolation × TransactionModel matrix. Codeunit may retain successful cross-method state; Function discards each method; AutoRollback rejects explicit Commit; None rejects direct writes but permits documented page transactions.
4. Implement PermissionTestHelper and the translated reset chain, then retire native duplicates. Restore handlers/traps/current instance on setup, execution, callback and reporting failures.

## Acceptance

- Two-connection gates prove hook transactions, explicit Commit, nested rollback, successful cross-method state, False before-hook, failing after-hook and None page/direct-write distinction.
- Same complete codeunits and sealed seed before/after; unchanged population and no unexplained losses. Full UT goes through the translated runner.

## References

Code: `src/rt/TestRunner.cpp`, `include/runtime/TestRunner.h`, `src/gen/CodeunitWriter.cpp`, `src/cli/Main.cpp`; gates: `TestIsolationGate`, `HandlerLifetimeGate`. Platform: TestIsolation property, TransactionModel/HandlerFunctions attributes, TestRunner triggers. AL: `Tools/Test Framework/Test Runner/src`. Predecessor: WI-963/1088/1316; method sampling changes isolation evidence.

Property scope: `requiredtestisolation`, `subtype`, `subtype-blob`, `subtype-codeunit`, `testisolation`, `testtype`.
