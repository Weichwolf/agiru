# 0012 — Production commits will be durable and concurrent writes will be checked

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

The review found `Boundaries::Commit` renewing savepoints without SQL COMMIT. The new `CommitDurabilityGate` first failed 3 of 4 checks against that implementation: a second connection could not see a committed row, a later rollback removed it, and closing the writer lost it. Production Commit now issues PostgreSQL COMMIT, opens a new transaction only when logical boundaries remain, and advances the cursor epoch. The same gate and all 70 local C++/toolchain cases then passed. A distinct test-isolation floor retains a rollback boundary for the translated Codeunit runner. The complete AL A/B and frozen snapshot integration remain pending. `Session` still owns a connection for its whole lifetime. `Table.h::LockTable` and `ReadIsolation` store state without enforcing SQL locks; `Storage.cpp::Updated` checks the primary key without an observed rowversion.

## Implementation for Sol

1. Keep the new two-connection gate: fixture creation outside the tested transaction, uncommitted-write control, visibility through B, a write-after-Commit rollback and visibility after A closes. Extend it to nested boundaries and synchronous durability after a process restart.
2. Keep production transaction completion separate from nested error boundaries and test isolation. Verify pending-write flush and failure during new-boundary setup; a scope destructor must not terminate the process if that setup fails. The test floor retains only its own savepoint, and AutoRollback refuses Commit.
3. Represent isolation per session/table, with record overrides. Implement UpdLock with PostgreSQL row locks and bounded wait policy; name PostgreSQL READ UNCOMMITTED divergence. Check update/delete against the version originally read and report conflicts.
4. Introduce a transaction-owned connection lease after correctness is gated. Handle rollback/destructor failures explicitly; a throwing Scope destructor during unwinding must not terminate the process.

## Acceptance

Two independent connections prove visibility, rollback, conflicts and lock release. Exercise Commit at depth zero and inside nested boundaries. Production durability gates must stay green when 0039 adds isolation. Compare the full UT population before/after activation.

## References

Platform: methods-auto/database/database-commit-method.md, devenv-tri-state-locking.md, devenv-read-isolation.md. AL: posting codeunits and their Commit calls. Predecessor: WI-1226 rejects a guessed AssertError rollback-to-last-Commit rule after an A/B lost four Payment Registration cases; its tested `Assert.RecordIsEmpty` and temporary-record path require a discriminating gate before changing AssertError boundaries. 0718 records why globally ignoring Commit lost error logs.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `datapercompany`, `readstate`, `transactiontype`.
