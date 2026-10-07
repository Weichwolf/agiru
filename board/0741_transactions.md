# 0741 — Qualify rollback boundaries and concurrent session execution

Status: in progress | Priority: P0
Depends on: existing PostgreSQL/session/HTTP runtime, not full UT acceptance.
Next: enforce atomic optimistic Modify/Delete/Rename conflicts without rereading the whole
record, followed by BC table/record locking and transaction-type transitions. Integrate
with the full native build and counted AL runs; preserve every refusal and failure.

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
- UI callbacks follow AllowSessionCallSuspendWhenWriteTransactionStarted (BC default enabled).
  Trusted server configuration, never a client parameter, owns the policy. A permitted
  write-transaction pause retains its SQL lease and rollback boundary without an implicit
  Commit; disabled policy refuses before displaying a blocking question. Qualify independent
  visibility, explicit Commit, cancellation/rollback and nested modal boundaries under 0720.

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
`methods-auto/record/record-{locktable,readisolation,rename,init,reset}-method.md`,
`methods-auto/recordref/recordref-init-method.md`.
`administration/server-instance-settings.md` defines the callback-in-write-transaction
policy (enabled by default), still unimplemented, and the try-write switch and on-premises
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
Startup: `deploy/dev/agiru.json`, `src/rt/NativeServiceConfig.cpp`,
`test/gate/NativeServiceConfigGate.cpp`, `test/ui/server-config.mjs`.
The complete versioned JSON profile declares every default. `serve --config` is the only
server CLI option; trusted operator credential/migration commands remain separate.
Regular input is bounded to 64 KiB; final-component symlinks/devices/FIFOs refuse.
Unknown/missing/duplicate keys, invalid types/numbers and malformed DSNs refuse with
sanitized diagnostics before connecting. Native/page validation is shared, not copied.
Config gate: 183 checks; AL JSON regression: 250; HTTP gate: 11; zero red.
`make page-host-test JOBS=2`: 12 fixture cases and 18 native application cases for each
TryFunction policy, zero red. Ignoring the policy must fail independent SQL checks;
removing duplicate rejection must fail five named schema checks. Existing three
ownership/revision/replay controls remain required. All eight affected C++ units pass
targeted clang-tidy with no suppression. `runtime/{HttpServerOptions,PageHostOptions}.h`
own the same single options types without transport/execution state. Thirty-one standalone
header probes pass; forced HTTP/page execution headers must fail the configuration boundary.
The current native `make all` and Config/HTTP/SessionParallel gates pass. Before/after
`make include-cost HEADERS=runtime/NativeService.h` samples measure 1,129/986 ms mean
frontend, three rounds without PCH. These samples are not ERP throughput or general build-speed proof.
New gates: `test/gate/{TransactionContract,SessionParallel}Gate.cpp`.
`test/ui/http-server.sh` runs native SessionParallel with a compiled worker-default
defect from `test/gate/HttpWorkerControl.cpp.in`; it must fail the affinity/ceiling check.
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
scope leakage) fail their named checks. Generated Report/XMLport classes now declare their
own default constructor, preventing aggregate construction from bypassing private CRTP
base access. GenReport/GenXmlPort and executable XMLport gates pass; removing the
generated constructor is rejected by compilation. Targeted generator/fixture lint passes.
The replacement Debian-Caddy container passes HTTP/authentication and generated-page
CMD/MCP fixtures. SessionParallel observes six concurrent workers with six available CPUs;
CPU placement is left to Linux, not manually pinned. Session policy is copied from trusted
NativeService/PageHost options loaded once from JSON, never from client input.
Full native transpilation still exits 1 for counted unsupported/missing source declarations;
regenerating the complete tree is not full compilation or a green AL suite.
These results are not
current whole-tree, full AL UT or finance acceptance.
The UT driver now uses the selected `B` for nested builds, freshness checks, execution
and image/library hashes; absent selected images cannot fall back to root build outputs.
`make ut B=<native-build> UT_MASTER_DSN=<verified-seed>` preserves the original default
when no seed override is supplied. Sixteen existing/extended MilestoneGate tooling tests
pass, including missing/stale image, timeout/interruption and source-population controls;
these mocked orchestration cases are not AL UT execution.
Latest direct native integration: `make slice-check B=/workspace/build/podman JOBS=6`
counts 14,225 sources with none missing; `make all` exits zero. C++ inputs stayed unchanged
through the client/tooling/deployment-only increments. All frozen AL file hashes match
clean BCApps `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`. The diagnostic slice still links
1,902 explicit unlinked-procedure refusals: build success is not full-app or G1 acceptance.
The complete native `make test JOBS=2` rerun passes 179 cases with zero red, including
257 tooling tests. This precedes the Init repair below and is not an AL UT result.
Write preflight checks permission → writable provider → trusted TryFunction policy;
set-based DeleteAll shares that boundary and compiled ordering defects refuse. Builtin
reproduction uses the explicit verified developer root; all 1,904 frozen method documents
match the pinned revision. `IsInWriteTransaction` belongs to `BuiltinsWritten`, not a
generated refusing body. Source-binding fixtures retain owned sessions; header dependency
and all 79 reflection controls pass. Full-app and counted AL acceptance remain open.

## Optimistic-write implementation boundary

- `src/rt/Table.cpp::Defaulted` retains timestamp fields for Record/RecordRef.Init;
  Clear still resets them. `test/gate/SqlRowVersionGate.cpp` passes 141 SQL checks,
  including key/filter retention, ordinary defaults and independent unchanged storage.
  Before the repair the same gate has five red checks. `make rowversions JOBS=2`
  passes nineteen compiled controls; removing timestamp retention fails both typed and
  reflected checks. The post-repair native `make all` passes; targeted clang-tidy reports
  zero findings for Table.cpp and SqlRowVersionGate.cpp. Private optimistic stamps
  remain unimplemented; retaining public timestamps alone does not enforce stale writes.
- Capture expected SystemId/version privately at SQL load/write boundaries, not from
  mutable AL fields. Assignment/Copy/RecordRef, Reset/Init/Clear and temporary records
  need explicit lifecycle tests; unloaded-record write behaviour still needs qualification.
- CAS predicates match key + logical identity + (expected version or own transaction token).
  A SQL-owned UUID token stays stable across released children and changes after Commit;
  do not use tuple xmin, subtransaction status or a copied numeric transaction ID as authority.
  A temporary native SQL candidate passes 43 checks; five guard/identity/fence defects fail
  named checks, including independent waiting writers that commit or roll back. Numeric-ID
  collision is synthetic, not backup/restore acceptance. No Record API or ERP pass is claimed.
- Persist the private token without changing AL field declarations; measure its 16-byte
  scalar plus actual row/index/write costs. Schema provisioning/import/restore and every
  write path must qualify before enabling the guard. Keep successful writes one statement;
  missing-row versus stale diagnostics must not weaken the atomic predicate.

PostgreSQL 17 backend references (local provider docs unavailable):
[transaction IDs](https://www.postgresql.org/docs/17/functions-info.html#FUNCTIONS-PG-SNAPSHOT),
[Read Committed update rechecking](https://www.postgresql.org/docs/17/transaction-iso.html#XACT-READ-COMMITTED),
[UUID generation](https://www.postgresql.org/docs/17/functions-uuid.html).
These backend guarantees implement the pinned BC contracts above, not SQL Server equivalence.

## Remaining documented cases — no completion claim

`devenv-task-scheduler.md`: rollback before retries/failure sessions;
`methods-auto/session/session-stopsession-method.md`: statement-boundary cancellation;
`devenv-upgrading-extensions.md`: atomic lifecycle and normal-event fallback during
install/uninstall/upgrade. Providers currently refuse or are absent; refusal is not
implementation. Executable AL report business fixtures and full integration/UT remain due.
External HTTP/files/printing and variable/temporary/SingleInstance state are not reversible
SQL effects. CurrentTransactionType/ReadIsolation currently carry values without SQL
enforcement; LockTable only changes record state and discards overload flags. These are
correctness gaps, not completed BC compatibility. PostgreSQL MVCC repeatable reads do
not implement BC's retained shared locks. Complete isolation, optimistic concurrency
and successful rollback/cancellation on connection failures remain acceptance work.
