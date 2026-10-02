# 0012 — Production commits will be durable and concurrent writes will be checked

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

`src/rt/Transaction.cpp::Boundaries::Commit` releases and recreates savepoints but never issues SQL COMMIT. The platform Commit documentation explicitly says it ends one write transaction and separates it from the next; savepoint renewal cannot satisfy this outside a test isolation floor. `TransactionGate` reads through the writing session, so it cannot prove durability. `Session` owns a connection for its whole lifetime. `Table.h::LockTable` and `ReadIsolation` store state without enforcing SQL locks; `Storage.cpp::Updated` checks the primary key without an observed rowversion.

## Implementation for Sol

1. First add a two-connection test: create the fixture outside the tested transaction, write and Commit through A, observe through B, close A, then observe again. Add an uncommitted-write control and a write-after-Commit rollback case.
2. Separate production transaction completion from nested error boundaries and test isolation. Production Commit must flush pending work, COMMIT synchronously, invalidate transaction-bound cursors, then recreate logical boundaries only when needed. Do not emulate durability by releasing savepoints.
3. Represent isolation per session/table, with record overrides. Implement UpdLock with PostgreSQL row locks and bounded wait policy; name PostgreSQL READ UNCOMMITTED divergence. Check update/delete against the version originally read and report conflicts.
4. Introduce a transaction-owned connection lease after correctness is gated. Handle rollback/destructor failures explicitly; a throwing Scope destructor during unwinding must not terminate the process.

## Acceptance

Two independent connections prove visibility, rollback, conflicts and lock release. Exercise Commit at depth zero and inside nested boundaries. Production durability gates must stay green when 0039 adds isolation. Compare the full UT population before/after activation.

## References

Platform: methods-auto/database/database-commit-method.md, devenv-tri-state-locking.md, devenv-read-isolation.md. AL: posting codeunits and their Commit calls. Predecessor: WI-1226 rejects a guessed AssertError rollback-to-last-Commit rule after an A/B lost four Payment Registration cases; its tested `Assert.RecordIsEmpty` and temporary-record path require a discriminating gate before changing AssertError boundaries. 0718 records why globally ignoring Commit lost error logs.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `datapercompany`, `readstate`, `transactiontype`.
