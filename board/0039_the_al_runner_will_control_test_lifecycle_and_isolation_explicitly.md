# 0039 — The AL runner will control test lifecycle and isolation explicitly

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

`src/rt/TestRunner.cpp::RunOne` invokes methods directly. A new ordered gate first proved that a method with an unused declared handler failed yet retained its row for the next method (`default,missed` instead of `default`). RunOne now discards its method scope before reporting that failure. All 73 local gates pass. Allocation controls exposed three failures in handler installation/deinstallation: installation now prepares replacement state before publishing it; deinstallation detaches state before allocating diagnostics. HandlerLifetimeGate passes 7 normal/ASan/UBSan checks. Three further failing-before controls exposed leaked page traps after a successful final method and handlers/traps after a foreign C++ exception. RunOne now owns an automatic cleanup guard with allocation-free HandlerTable.Reset, and setup is inside its error boundary; TestIsolationGate passes 10 normal/ASan/UBSan checks. Foreign exceptions remain visible and the test instance is released. A Codeunit isolation floor now takes back explicit Commits, and AutoRollback refuses them. RunRegisteredTests still always wraps a codeunit; no generated TestRunner drives before/after hooks, and Function/Disabled policies remain absent. The CLI runner exists, so the old claim that there is no runner is obsolete.

## Implementation for Sol

1. Carry the selected TestRunner, TestIsolation and TransactionModel as explicit runtime metadata. Register OnBeforeTestRun/OnAfterTestRun hooks and drive the translated runner through the CLI, including the empty-function codeunit callbacks and skip result.
2. Implement an isolation floor below nested error scopes, coordinated with 0012. None, Codeunit and Function are distinct policies. Successful methods may share state under Codeunit isolation; do not unconditionally roll back each successful method. The documented `[TransactionModel(AutoRollback)]` refuses an explicit `Commit()`; `AutoCommit` allows it, while the runner's Codeunit/Function isolation still rolls it back at its own floor. `None` starts a transaction only for a page interaction.
3. Keep the new unused-handler rollback gate. Ensure exceptions during installation, invocation, result collection and hook execution always uninstall handlers and release traps; the handler allocation and foreign-exception paths are now covered, but generated runner hooks and their exceptions still need coverage.
4. Implement the PermissionTestHelper bookkeeping needed by the translated reset chain before activating it. Replace native reset duplicates only once the corresponding AL hooks are exercised.

## Acceptance

Gates cover explicit Commit followed by method failure, Commit surviving an inner rollback, successful cross-method state under Codeunit isolation, Function isolation, AutoRollback, unused handlers, false before-hook and throwing after-hook. Run the same full codeunits for A/B; per-method sampling is not an isolation proof.

## References

Platform: properties/devenv-testisolation-property.md, attributes/devenv-transactionmodel-attribute.md, attributes/devenv-handlerfunctions-attribute.md, devenv-testrunner-codeunits.md and both TestRunner triggers. AL: Tools/Test Framework/Test Runner/src. Predecessor: WI-963, WI-1088 (164/190 versus 179/190 under different isolation), WI-1316.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `requiredtestisolation`, `subtype`, `subtype-blob`, `subtype-codeunit`, `testisolation`, `testtype`.
