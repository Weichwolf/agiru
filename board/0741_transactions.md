# 0741 — Qualify rollback boundaries and concurrent session execution

Status: in progress | Priority: P0
Depends on: existing PostgreSQL/session/HTTP runtime, not full UT acceptance.
Next: qualify existing rollback gates, concurrent HTTP and generated AL call contexts;
repair failures and run targeted clang-tidy before commit/push. Then implement and qualify
BC table/record locking, transaction-type transitions and atomic optimistic write conflicts.

## Acceptance

- Evaluated Codeunit.Run rolls back errors, commits success durably and refuses pending
  production writes before execution. CommitBehavior cannot suppress implicit commits.
  Statement Run propagates errors and never implicitly commits caller writes.
- Explicit Commit survives later failure; nested rollback, consistency marks, cursors,
  deferred constraints and independent connection visibility remain correct.
- DisableWriteInsideTryFunctions is immutable trusted runtime configuration at native startup.
  Test both policies, argument evaluation, nested writes and discarded ordinary calls.
  Allowed failed-try writes survive the catch but remain subject to caller rollback.
- Isolated subscribers independently commit/roll back and retain variable mutations;
  pending writes require normal-event fallback. Collectible errors roll back evaluated Run
  without losing the collection; clearing errors is not rollback. Ordinary errors still raise.
- Report/XMLport Quit and trigger errors roll back execution without undoing prior Commit.
- Isolation follows BC developer contracts, not identically named PostgreSQL levels:
  per-table tri-state escalation, instance overrides, retained shared/update locks,
  read-only transaction types and their allowed transitions. No dirty reads is acceptable
  to the owner; retain this deliberate backend difference, not a claim of SQL Server parity.
  Concurrent stale client writes must refuse atomically without lost updates; prove
  Modify/Delete/Rename and rowversion fences with independent concurrent connections.
  Finance acceptance requires failed and competing posting runs with balanced independent
  ledger/control-account checks; primitive gates alone cannot certify financial integrity.
- HTTP defaults to affinity-available CPU workers within an explicit ceiling. Independent
  sessions execute concurrently; one session never executes two commands simultaneously.
  Worker reuse/migration must not leak identity, policy, errors, restrictions or SQL effects.

## Sources and regression ownership

Developer docs at `f928288ee840334be73142e5fc0202c0e19b246d`:
`methods-auto/codeunit/codeunit-run{,-integer-table,-string-table}-method.md`,
`devenv-handling-errors-using-try-methods.md`, `devenv-events-isolated.md`,
`devenv-error-collection{,-api}.md`, `attributes/devenv-{commitbehavior,transactionmodel}-attribute.md`,
`properties/devenv-testisolation-property.md`, `methods-auto/database/database-{commit,isinwritetransaction}-method.md`,
`triggers-auto/report/devenv-onpostreport-report-trigger.md`, `methods-auto/report/reportinstance-quit-method.md`,
`properties/devenv-autoincrement-property.md`, `devenv-number-sequences.md`;
`devenv-{read-isolation,tri-state-locking,table-system-fields}.md`,
`properties/devenv-{transactiontype,readstate}-property.md`,
`methods-auto/database/database-currenttransactiontype-method.md`,
`methods-auto/record/record-{locktable,readisolation}-method.md`.
`administration/server-instance-settings.md` defines the try-write switch and on-premises
default true (online allows writes). agiru preserves its previous allow-writes default
and exposes both policies. Predecessor `openerp/board/1220_*` identifies pending-write
isolated-event fallback; its Python session/thread machinery is not adopted.
BCApps `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`:
`src/Layers/W1/BaseApp/Finance/GeneralLedger/Posting/GenJnlPost{Batch,Line}.Codeunit.al`
locks G/L Entry before register/entry-number allocation. Predecessor `openerp/board/1868_*`
also identifies default dirty reads, tri-state escalation and missing stale-write checks;
its proposed PostgreSQL snapshot isolation is not evidence of BC lock equivalence.

Runtime: `include/runtime/{Transaction,Codeunit,Error,Report,XmlPort,SessionOptions}.h`,
`src/rt/{Transaction,Scopes,Events,Session,Table,TablePermissions,Navigate,PageCommandHost}.cpp`,
`src/net/HttpServer.cpp`, `src/cli/Services.cpp`.
New gates: `test/gate/{TransactionContract,SessionParallel}Gate.cpp`.
Existing regressions: `test/gate/{Transaction,CommitDurability,CommitBehavior,TestIsolation,
RequiredTestIsolation,SessionCommand,NumberSequence,SqlRowVersion}Gate.cpp`,
`test/runtime/{codeunit-record,test-contexts,xmlport-import}/`.
Current focused result: TransactionContract 106 checks, zero red; SessionParallel 14,
generated Codeunit Record 99, SqlRowVersion 123, SessionCommand 49, NumberSequence 396,
RequiredTestIsolation 430, Transaction 12, CommitDurability 19, CommitBehavior 11,
TestIsolation 17, Report 50, Event 34, Instance 51 and HttpServer 11: zero red.
Generated test contexts passed 48 checks; the Make target now rebuilds the key-control
consumer after the Session ABI change. Three generated global-state defects and four
runtime defects (missing implicit Commit, missing rollback, try-write bypass, worker-local
scope leakage) fail their named checks. Targeted lint repairs include direct headers,
CRTP construction access and fixture constness; final affected checks remain running.
These results are not
current whole-tree, full AL UT or finance acceptance.
The previous direct integration lane was deliberately interrupted after 519/1174 steps
before changing compiler inputs. It is not a build pass or current UT measurement.

## Remaining documented cases — no completion claim

`devenv-task-scheduler.md`: rollback before retries/failure sessions;
`methods-auto/session/session-stopsession-method.md`: statement-boundary cancellation;
`devenv-upgrading-extensions.md`: atomic lifecycle and normal-event fallback during
install/uninstall/upgrade. Providers currently refuse or are absent; refusal is not
implementation. Executable XMLport/AL report fixtures and full integration/UT remain due.
External HTTP/files/printing and variable/temporary/SingleInstance state are not reversible
SQL effects. CurrentTransactionType/ReadIsolation currently carry values without SQL
enforcement; LockTable only changes record state and discards overload flags. These are
correctness gaps, not completed BC compatibility. PostgreSQL MVCC repeatable reads do
not implement BC's retained shared locks. Complete isolation, optimistic concurrency
and successful rollback/cancellation on connection failures remain acceptance work.
