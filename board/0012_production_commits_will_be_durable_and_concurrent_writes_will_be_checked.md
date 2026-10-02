# 0012 — Production commits will be durable and concurrent writes will be checked

Status: open | Priority: P0 | Stage: UT → Clients → All | Reviewed: 2026-09-28
Depends on: 0013 observed versions; 0718 record ownership.

## Evidence

- `Boundaries::Commit` now issues SQL COMMIT outside explicit test floors; independent-connection gates cover visibility, later rollback, failed commits and consistency marks.
- `Table.h::LockTable` discards Wait/VersionCheck; reads do not consume stored isolation. `Storage.cpp::Updated/DeleteRow` use key-only predicates.
- `Scope::~Scope` can throw during stack unwinding; Session still owns the connection.
- `CommitBehaviorGate` proves monotonic nested restrictions and restoration after normal return/error: Clang 19 and GCC 14 each 11 checks green; unchanged runtime negative control: 2 red. Patch integrated; frozen AL verification pending.

## Implementation

1. Preserve production COMMIT and the separate Codeunit/Function isolation floors. Add process-restart durability and fault injection for rollback, release and renewed-savepoint setup.
2. Implement table/session lock state and record overrides in the shared SQL read path. Bind lock timeout per transaction; enforce Wait and VersionCheck. Document PostgreSQL's lack of dirty reads.
3. Carry observed rowversion with each record image; update/delete/rename use key plus observed version and check affected rows. Distinguish conflict from absent key and database fault.
4. Make unwinding cleanup nonthrowing without swallowing diagnostics: invalidate the connection/session on failed rollback, retain the primary error plus cleanup failure. Return only clean leases to the pool.
5. Prove the separate implicit Codeunit.Run commit contract; it must not inherit explicit CommitBehavior restrictions. Session ownership of restriction state belongs to 0006.

## Acceptance

- Two connections prove durable Commit, failed-write rollback, lock wait/refusal, stale-write conflict and lock release; restart confirms committed rows.
- Nested scopes and failed cleanup cannot terminate the process or reuse an aborted connection. Full UT method comparison has no unexplained losses.
- Outer Error/inner Ignore refuses explicit Commit; reverse nesting also refuses, unwind restores the outer scope, and implicit Codeunit.Run retains its separate contract.

## References

Code: `src/rt/{Transaction,Scopes,Storage,Navigate,Selection}.cpp`, `include/runtime/Table.h`; gates: `CommitBehaviorGate`, `CommitDurabilityGate`, `ScopeDiscardGate`, `TransactionGate`. Platform: `methods-auto/database/database-commit-method.md`, `attributes/devenv-commitbehavior-attribute.md`, `methods-auto/commitbehavior/commitbehavior-option.md`, `devenv-tri-state-locking.md`, `devenv-read-isolation.md`. BCApps main: `src/Layers/W1/BaseApp/Finance/GeneralLedger/Preview/GenJnlPostPreview.Codeunit.al::PreviewStart`. User intent: `business-central/ui-how-preview-post-results.md`. Predecessor: WI-1306 forbids weaker nested restrictions; WI-993; WI-1226 comment 7 retracts the discarded-Get counterexample. Globally ignoring Commit is rejected.

Property scope: `datapercompany`, `readstate`, `transactiontype`.
