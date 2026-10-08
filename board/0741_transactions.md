# 0741 — Qualify rollback boundaries and concurrent session execution

Status: in progress | Priority: P0
Depends on: existing PostgreSQL/session/HTTP runtime, not full UT acceptance.
Next: finish the native HTTP regression matrix, rebuild production generated consumers
and execute Customer New/template/save/reopen under 0720;
qualify disabled native callbacks, shutdown rollback, progress execution and the
remaining optimistic-write/isolation contracts. The rebuilt original Customer opens
the real template modal and inserts the explicitly selected template. Expanded workflow
acceptance currently has eight passes and three persistence failures: Address, Country
and Credit Limit stay only in the page buffer. The generic state repair below is gate-
qualified; its production rebuild and original workflow acceptance remain pending.
Repeat counted AL execution after the verified SelectLatestVersion increment below;
retain every startup refusal and test identity. Atomic
optimistic Modify/Delete/Rename, BC locks and transaction-type transitions remain due;
do not defer client construction until all transaction acceptance is complete.

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
  ApplicationArea and random sequences belong to the session, not the executing thread.
  Randomize preserves explicit seeds including zero; its omitted seed uses milliseconds
  since midnight. Random treats negative bounds as positive, zero as one and accepts the
  representable positive/negative bounds without signed overflow; qualify the indirectly
  reachable minimum Integer bound separately. Posting determinism does not override
  these platform contracts; the AL test library explicitly controls its own seed.
- UI callbacks follow AllowSessionCallSuspendWhenWriteTransactionStarted (BC default enabled).
  Trusted server configuration, never a client parameter, owns the policy. A permitted
  write-transaction pause retains its SQL lease and rollback boundary without an implicit
  Commit; disabled policy refuses before displaying a blocking question. Qualify independent
  visibility, explicit Commit, cancellation/rollback and nested modal boundaries under 0720.

## Sources and regression ownership

`PageSession.h` now reconciles source-AL insertion with both pending-new markers.
Native storage probes both primary key and immutable SystemId with bounded `SELECT 1`,
without replacing the live buffer, image, filters or rowversion. Temporary storage uses
its indexed key lookup and typed GUID equality; it never reads/writes PostgreSQL.
Nonblank/copied IDs alone are not insertion receipts; rolled-back inserts remain pending.
`make page-navigation JOBS=2` in the development container: 239 generated navigation,
105 dispatcher and 15 source checks, zero red; 25 execution defects and one compile
refusal reject. The 19 added checks cover source insertion during OnNewRecord,
OnAfterGetCurrRecord/Copy and OnValidate, ordinary field persistence, duplicate prevention,
pending identity/key mismatches, rollback and temporary rows. DelayedInsert regressions
remain green. The changed runner passes targeted clang-tidy without suppressions.
Sources: `test/runtime/page-navigation/{Created.Page.al,Runner.cpp}` and its Bash harness.
References at developer revision `f928288ee840334be73142e5fc0202c0e19b246d`:
`methods-auto/page/page-saverecord-method.md`, `methods-auto/record/record-copy-method.md`,
`devenv-table-system-fields.md`, `properties/devenv-delayedinsert-property.md` and
`triggers-auto/page/devenv-on{insert,modify}record-page-trigger.md`.
BCApps `CustomerCard.Page.al::CreateCustomerFromTemplate` and
`CustomerTemplMgt.Codeunit.al::InsertCustomerFromTemplate` at
`d99152ee35f0ca8cfec43ba6334b7247a0ee6b17` expose the same general lifecycle.
Predecessor WIs 1554/1395 retain trigger/delayed-insert findings. This is not optimistic
write, financial posting or original Customer HTTP acceptance.

Borrowed modal lifecycle and native transport are qualified in 0720: 220 generated
navigation, 105 dispatcher and 15 source checks pass; 21 execution defects and one
compile defect reject. The caller's original AL page/filters survive; false/error close
attempts can retry; disabled callbacks refuse before opening. Actual HTTP proof has
154 cases, zero red, across limits 40/7/80 and both TryFunction policies after the
delayed-error repair. Eighteen compiled defect controls reject at named assertions;
`make page-host-test JOBS=2` exits zero with verified input hashes. Failed close inputs preserve
their own receipt identity after the HTTP worker returns, never masquerade as successful
root polling. Independent
SQL verifies nested modal/question suspension, explicit cancellation/timeout rollback,
no implicit modal commit, and durability of a prior explicit Commit after caller failure.
Child-input receipts do not establish root durability. The production ABI rebuild
at `d31bb87` passes slice-check/all (14,225 slice sources, 1,526 seconds); the new
source-insertion repair still needs a rebuilt image and original workflow proof.
No finance acceptance.

Blocking Confirm/StrMenu now enforce trusted
`transactions.allow_session_call_suspend_when_write_transaction_started`, default true,
before native or explicit AL test callbacks. Disabled write-phase interaction refuses
`UiWriteTransaction` without publishing, discarding or changing caller SQL/boundaries;
read-only callbacks and nonblocking messages/progress remain allowed. Explicit Commit
reenables callbacks until the next write; later unwind preserves that Commit. Worker
migration retains the session policy. `make ui-host JOBS=2`: 154 UI checks, 205 configuration
checks and eleven compiled defects rejected, including policy bypass, test-handler bypass,
lost configuration and implicit Commit. Sources: `include/runtime/{SessionOptions,UiHost}.h`,
`src/rt/{UiHost,NativeServiceConfig,written/BuiltinsWritten}.cpp`, `deploy/dev/agiru.json`,
`test/gate/{UiHost,NativeServiceConfig}Gate.cpp`, `test/runtime/ui-host.sh`.
Five changed C++ units pass clang-tidy without suppressions. SessionCommand 49,
SessionParallel 14, TransactionContract 110, CommitDurability 19 and RequiredTestIsolation
430 checks pass. `make page-host-test JOBS=2` retains 78 generated-fixture HTTP cases,
limits 7/40/80, external CMD/MCP and Chromium, plus six compiled defects: zero red.
Reference: developer `administration/server-instance-settings.md` and
`methods-auto/dialog/dialog-{confirm,strmenu}-method.md` at the pinned revision below;
BCApps `Finance/GeneralLedger/Posting/GenJnlPost.Codeunit.al` calls Confirm Management
before posting. Predecessor `openerp/runtime/ui_host.py` has no corresponding policy;
its implicit headless answers and session-per-thread transport are not adopted.
HTTP Confirm/StrMenu suspension and deferred messages now use a real session-owned
endpoint; native page modals now use the same retained AL worker/SQL lease. Report and
request-page callbacks, automatic modal CurrPage.Close and live progress remain gaps.
Production generated consumers have been rebuilt after the expanded SessionOptions
value ABI: `make slice-check all B=/workspace/build/podman JOBS=6` exits zero,
14,225 slice sources, 2,901 seconds. The 1,902 unlinked AL procedures remain gaps;
this is not complete-app compilation, full AL execution or a working client dialog.
Latest frozen native baseline: clean `280917453b8cc12549872a50c4a9b1bfcd8cb436`;
`make verify-start B=/workspace/build/podman JOBS=2 VERIFY_TARGETS=test`:
185 manifest cases, zero red, exit 0, 1,930 seconds including configuration/build.
The run includes the bounded asynchronous executor, not later question/message work.
Source, selected PostgreSQL-5432/AL/slice configuration and system symbols retain their
SHA-256 before/after: respectively
`5e68d7a21dbd3d5eca34ddccef7baa90413e10dcb40245625e4890c8d4be301b`,
`681c0520e14721d4b0875729c614c0c39e1e65ed4ffbc31b8f991225d5db05c0`,
`34c40f0dcc839eb4d244398715e11a801194bfcea69a21257c5de20a9833a955`.
`scripts/verify_snapshot.py` pins private selected-build settings and configures its
own lane before targets, including reused builds; inherited B cannot redirect it.
The earlier wrong-DSN/source configuration defect is repaired in `1a07829`;
SnapshotGate 21, verify-check 29 and tooling 261 checks pass. XMLport fixtures use
their generated AL binding rather than a guessed Row member; all four import profiles
and constructor/execution controls pass. This is native regression evidence, not
source-counted AL UT, complete-app, interactive client or financial posting acceptance.

Native command failures now require affected-row proof for durable page invalidation
and the failed SQL receipt before returning `failed`. Cleanup uncertainty returns
`unknown`; prior explicit Commit remains durable. `PageCommandHost.cpp` and shared
`src/client/{profile,http,errors,cmd,mcp,web}.mts` preserve exact diagnostics/command IDs
over HTTP instead of treating every server-side AL failure as WriteUncertain.
`test/ui/page-host.mjs` independently checks rollback, Commit, receipts and external
CMD/MCP/Chromium error parity; `page-host.sh` adds a compiled missing-receipt control.
`make page-host-test`: 94 native HTTP cases, limits 40/7/80 and both TryFunction
policies, eight compiled defects rejected; generated navigation 183, PageSource 15,
PageDispatcher 105 and sixteen execution controls/one compile refusal pass.
41 client cases/eight executable defects and thirteen browser cases/three compiled
defects pass. The bounded AL executor keeps HTTP polling available with one HTTP
worker; delayed field writes retain ownership, one SQL effect, rollback and durable
Commit across external CMD/MCP/htmx. Trusted execution/queue/response-wait defaults
are explicit in the server JSON; configuration retains 232 checks, zero red.
Four affected C++ units pass clang-tidy; slice-check/all pass without removing the
1,902 unlinked diagnostic-slice gaps. The real endpoint in `src/rt/PageInteraction.cpp`
now enables native GuiAllowed without a capability flag or forced control visibility.
The shared profile and SQL-fenced `/answers` channel preserve one executing AL stack,
independent pending-write visibility, explicit Commit and rollback on decline/timeout.
HTML refusal precedes question publication. `test/ui/page-host.mjs` has 114 positive
HTTP cases across limits/policies and external CMD/MCP/Chromium, with eleven compiled
defects rejected; native configuration
242, agent 43 and browser fourteen plus Caddy cases pass. Five affected C++ units have
zero clang-tidy findings. Live progress explicitly refuses `UiProgressUnsupported`;
disabled native callback policy, shutdown rollback, modal/ErrorInfo, financial posting
and complete AL acceptance remain open. Client contract/evidence owner: 0720.

`make refresh-records JOBS=2`: 149 C++ checks, 80 generator checks and six compiled
defects rejected. `runtime/RecordRefresh.h` owns both official overloads;
`RecordChanges` invalidates active non-locked observations for the exact table/current
session or all tables. `Navigate` then reopens its bounded SQL cursor from the unchanged
key, retaining SQL ordering, filters and loaded values. Independent committed SQL
updates prove fresh typed/reflected reads in both directions. Own writes still invalidate
all readers, including cache-lock metadata. No historical table counters accumulate.
Physical repeatable/serializable snapshots refuse before invalidation, never commit to
manufacture freshness. Pending writes, nested rollback and prior explicit Commit survive
the operation. CaptionClassTranslate remains unsupported and has no implemented cache.
Cache-lock preservation is not SQL-lock qualification: LockTable/FindSet(forUpdate),
per-table escalation and instance-level SQL locking remain gaps above.
Tests: `test/gate/{RecordRefresh,GenReceiver}Gate.cpp`,
`test/runtime/record-refresh.sh`; the full `make test` manifest includes the new checks.
Existing DynamicRecord 7,493, CursorLifecycle 313, SelectionChange 364, Transaction 12
and RequiredTestIsolation 430 checks pass. Six affected compiled units pass targeted
clang-tidy without suppressions; builtin regeneration reproduces exactly. Full native
build at `f232cfb` exits zero: 14,225 slice sources, 2,717 seconds. Its full test run
retains 183 cases and reports two harness failures: the old RecordRead write-revision
mutation anchor and the missing record-refresh script in the Discovery fixture.
Both are repaired in `test/runtime/record-order.sh` and `test/tooling/toolchain.py`.
`make record-order JOBS=2` passes all 37 controls, including the named matching-write
refusal; cleanup removes mutant inputs/binaries on failure too. All 257 tooling tests
pass with the native compiler/database environment. The full manifest rerun, full lint
and unchanged 2,314-test AL diagnostic remain due; these checks do not qualify session
migration or financial integrity.

Session ApplicationArea and the lazy random generator now belong to `SessionState`,
not thread-local values. `make session-identity JOBS=2` passes 48 SessionValues checks,
84 generator checks and the existing identity/command/credential gates. Six new compiled
ownership/seed/bound/clock defects fail named claims; the twelve existing controls remain.
Nested, reused, migrated and simultaneous authenticated sessions retain independent values.
Explicit zero seeds remain zero; omitted Randomize uses milliseconds since midnight,
including a compiled fixed-clock proof. Positive/negative bounds draw identically and
Integer maximum no longer overflows. Seed 11 still produces Random(100) = 48.
The indirectly reachable Integer minimum bound explicitly refuses `RandomBoundRange`:
its positive magnitude is unrepresentable; actual BC behaviour remains unqualified.
Do not count this gap as an approved exclusion. Parameterless generated callers must
be rebuilt for the new overload; previous binaries are not qualification of clock seeding.
Tests: `test/gate/SessionValuesGate.cpp`, `test/runtime/session-values/Clock.cpp` and
the existing `test/runtime/session-identity.sh`; no additional orchestration script.
Seven affected units pass clang-tidy; builtin reproduction retains 109 written overloads.
UiHost 60, RecordRefresh 149, Transaction 12, DynamicRecord 7,493 and SessionParallel 14
checks pass. Full manifest, full lint and counted AL execution remain pending.

Contracts at developer revision `f928288ee840334be73142e5fc0202c0e19b246d`:
`methods-auto/session/session-applicationarea-method.md`,
`methods-auto/system/system-{random,randomize}-method.md`,
`methods-auto/integer/integer-data-type.md` (minimum reached through arithmetic).
BCApps `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`:
`src/Layers/W1/Tests/ApplicationTestLibrary/LibraryRandom.Codeunit.al` explicitly sets
the test seed; `src/Layers/W1/BaseApp/Modules/System/ApplicationArea/ApplicationAreaMgmt.Codeunit.al`
sets/reads the current session area. Predecessor `board/1479_applicationarea-sichtbarkeit.md`
identifies session ownership but retracts inferred TestPage visibility rules; do not
port that visibility hypothesis. Its `_math.py` clock/negative-bound shortcuts contradict
the developer contracts and are not adopted. Runtime owners:
`src/rt/{SessionState.h,SessionRandom.h,SessionRandom.cpp,SingleInstance.cpp,written/BuiltinsWritten.cpp}`.

Post-bridge native rerun at `ea6c922`: `make ut B=/workspace/build/podman JOBS=6
UT_MASTER_DSN=<same-complete-seed>` retains all 2,314 tests/80 codeunits; zero executed,
80 incomplete, 623 seconds. The GuiAllowed startup refusal is gone; every sampled
codeunit now refuses `Database.SelectLatestVersion(Integer)` before test execution.
No assertion population is dropped. Native build exits zero; seed remains an unsealed
shared-transfer diagnostic, not equivalent A/B proof. The browser/Customer rerun is
in 0720. The increment above repairs the recorded overload refusal, but is not a new
AL result. Predecessor `_system.py`'s no-op and
WI 889's proposed implicit commit are not adopted. Reference overloads:
`methods-auto/database/database-selectlatestversion{,-integer,--}-method.md` at the
developer revision below; `src/rt/{RecordChanges,Cursor,Navigate}.cpp` own bounded reads.

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
policy (enabled by default), enforced for Confirm/StrMenu above, and the try-write switch and on-premises
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
  `src/rt/RowVersionStorage.cpp` now provisions `agiru_platform.write_transaction_v1()`;
  `test/gate/RowVersionGate.cpp` passes 136 primitive checks, including transaction-local
  cache cleanup, first writes inside released/rolled-back children and independent connections.
  `make rowversions JOBS=2` passes twenty-two compiled controls, including fixed, regenerated
  and session-persistent tokens. Changed-code `make lint JOBS=2` passes three of 328
  handwritten units with zero findings; native `make all` passes. TransactionContract 110
  and RequiredTestIsolation 430 checks remain green. This primitive does not yet activate
  Record write guards.
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
