# 0012 — Production commits will be durable and concurrent writes will be checked

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

The review found `Boundaries::Commit` renewing savepoints without SQL COMMIT. `CommitDurabilityGate` first failed 3 of 4 checks: a second connection could not see a committed row, a later rollback removed it, and closing the writer lost it. Production Commit now issues PostgreSQL COMMIT, opens a new transaction only when logical boundaries remain, and advances the cursor epoch. A distinct test-isolation floor retains a rollback boundary for the translated Codeunit runner. The expanded 15-check gate also covers aborted transactions, deferred constraint failure, nested scope unwinding and reuse of the same session. Frozen runs `20260922T103145Z-88633` and `20260922T120756Z-158412` both passed all/test/GCC and reported the same 2,197/2,310 AL methods with zero incomplete cases and no per-method change. A new allocation-failure control found that `Scope::Discard` set its closed flag before allocating the last-error text; if allocation failed, its destructor could no longer roll back the method's write. The new `ScopeDiscardGate` failed before the fix and now passes four checks for plain and coded errors under normal and combined ASan/UBSan builds. The complete local `make test JOBS=2` passes 74 cases. A frozen full integration of this latest change remains to be completed. A separate failing-before control found that rolling back an inner savepoint cleared an outer `Record.Consistent(false)` mark, allowing an inconsistent posting to commit. Boundaries now retain the consistency set at each savepoint and restore it on rollback; the 19-check CommitDurability gate passes in normal and combined ASan/UBSan builds, and all 74 local test programs pass. `make gcc JOBS=2` also passes with the changed transaction header. This latest change still needs the full AL integration. The TestIsolation gate additionally proves that Commit inside a nested error boundary retains earlier writes under the codeunit floor, removes later failed writes, and the codeunit floor ultimately removes both methods' retained writes. `Session` still owns a connection for its whole lifetime. `Table.h::LockTable` and `ReadIsolation` store state without enforcing SQL locks; `Storage.cpp::Updated` checks the primary key without an observed rowversion.

## Implementation for Sol

1. Keep the new two-connection gate: fixture creation outside the tested transaction, uncommitted-write control, visibility through B, a write-after-Commit rollback and visibility after A closes. Extend it to nested boundaries and synchronous durability after a process restart.
2. Keep production transaction completion separate from nested error boundaries and test isolation. Verify pending-write flush and failure during new-boundary setup; a scope destructor must not terminate the process if that setup fails. The test floor retains only its own savepoint, and AutoRollback refuses Commit.
3. Represent isolation per session/table, with record overrides. Implement UpdLock with PostgreSQL row locks and bounded wait policy; name PostgreSQL READ UNCOMMITTED divergence. Check update/delete against the version originally read and report conflicts.
4. Introduce a transaction-owned connection lease after correctness is gated. Handle rollback/destructor failures explicitly; a throwing Scope destructor during unwinding must not terminate the process.

## Acceptance

Two independent connections prove visibility, rollback, conflicts and lock release. Exercise Commit at depth zero and inside nested boundaries. Production durability gates must stay green when 0039 adds isolation. Compare the full UT population before/after activation.

## References

Platform: methods-auto/database/database-commit-method.md, devenv-tri-state-locking.md, devenv-read-isolation.md. AL: posting codeunits and their Commit calls. Predecessor: read WI-1226 through comment 7, which retracts the earlier interpretation of four Payment Registration losses: the discarded temporary Get failed to raise, invalidating that counterexample. It orders the missing Get refusal before revisiting AssertError rollback scope. Preserve discriminating tests and a full A/B before changing that boundary; the present repair changes only SQL Commit failure handling. 0718 records why globally ignoring Commit lost error logs.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `datapercompany`, `readstate`, `transactiontype`.
