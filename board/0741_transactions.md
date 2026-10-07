# 0741 — Qualify rollback boundaries and concurrent session execution

Status: in progress | Priority: P0
Depends on: existing PostgreSQL/session/HTTP runtime, not full UT acceptance.
Next: enforce atomic optimistic Modify/Delete/Rename conflicts without rereading the whole
record; then implement BC table/record locking and transaction-type transitions. Integrate
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
targeted clang-tidy with no suppression. `make include-cost HEADERS=runtime/NativeService.h`:
1,151 ms mean frontend, three rounds without PCH; pre-change header at `60f2e40` measured
1,181 ms against the same dependencies. These samples do not establish a speedup.
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
The full native `make test JOBS=2` completed with 179 cases, eight red: four metadata
gates, reflection qualification, header dependencies, builtin reproduction without local
developer docs and toolchain source-binding fixtures. The toolchain reported four failures
among 254 tests, all missing-session fixtures. These are retained failures, not AL UT results.
Write preflight now checks permission → writable provider → trusted TryFunction policy;
set-based DeleteAll shares that boundary. The catalogue gate rejects wrong session errors,
and a compiled ordering defect fails both empty bulk-write diagnostics. `make reflection-metadata`
passes all 79 compiled controls. PageTableField 312, TransactionContract 106, TablePermissions 29,
SqlRowVersion 123 and Temporary 95 checks pass; the four source-binding fixture variants
pass with owned sessions. Targeted clang-tidy reports zero findings in all three changed
C++ units. Header dependency and builtin-reference repairs and a complete rerun remain due.

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
