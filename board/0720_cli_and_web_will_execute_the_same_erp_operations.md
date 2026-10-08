# 0720 — Deliver equivalent web, agent CMD and MCP clients (G2)

Status: queued | Priority: P0
Depends on: existing generated page declarations, typed record/session primitives
and database access, not full 0058 acceptance. Include client/workflow-blocking runtime
repairs in coherent client increments; preserve existing tests and counted UT failures.
Next: implement native Page.RunModal suspension for the original Customer template
list (1380), including explicit selection/cancel and GetRecord writeback for 0727.
Preserve the AL page variable, caller stack/transaction and generic HTTP/SQL authority;
never substitute a default template, UT handler or client-side business rule.

Development packaging: 0726 owns one server/web/PostgreSQL Podman container;
Node CMD/MCP runs outside over HTTP. Queued process families 0727–0740 own BC
sandbox reference execution and agiru replication. Their agiru prerequisites are
specific working client contracts, not this WI's full acceptance; no dependency cycle.
BC capture can proceed while client construction is underway. Keep one WI in progress.

## Typed variable bindings

- Generated control getters read the original declared scalar storage through
  `ReadPageVariable`, sharing the record-field transport. Decimal scale, Int64 digits,
  Unicode, temporal flags and declared Option/Enum members remain exact. Domains identify
  the object kind, ID and control; display text is not a type oracle. Unsupported storage
  refuses explicitly. Computed-expression transport remains unqualified.
- Sources: `include/runtime/{PageVariableValue,PageValue,Page,PageSession}.h`,
  `src/rt/PageValue.cpp`, `src/gen/{PageWriter,RuntimeSurface}.cpp` and
  `test/gate/PageValueGate.cpp`. PageValue has 43 checks, zero red; affected scalar/generator
  units pass targeted clang-tidy. Generated navigation has 220 checks, zero red;
  the Card's six direct variable bindings now have exact values rather than scalar gaps.
  This is not original Customer, full generated-tree or source-counted UT acceptance.
- Reference: developer `properties/properties/devenv-sourceexpr-property.md` at
  `f928288ee840334be73142e5fc0202c0e19b246d`; BCApps `CustomerCard.Page.al`
  (LastPaymentAmount) and `SalesOrderStatistics.Page.al` (indexed totals) at
  `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`. Predecessor WIs 1799 and 1894 retain
  Decimal/array lessons; no predecessor implementation copied.
- Three no-PCH rounds measured Page 1948.9 ms, PageSession 2070.8 ms and the new
  PageVariableValue helper 839.3 ms during concurrent fixture compilation. No before/after
  build-speed or ERP performance claim. Rebuild generated consumers before use.

## Borrowed modal prerequisite

- `Page.RunModal` now gives the trusted host a closed production adapter over the original
  AL object, preserving its variables, record, filters and LookupMode. Numbered calls retain
  writable record-argument writeback. Explicit AL test handlers keep precedence; missing
  declared handlers cannot fall back to native interaction.
- `PageInstance::CloseModal` uses an explicit action. Query-close false/errors leave the
  page open and retryable; reopening resets lifecycle flags, not AL variables. Callback
  policy refuses before opening during a prohibited write phase.
- `make page-navigation JOBS=2` with the container gate DSN on port 5432: 219 navigation,
  105 dispatcher and 15 source checks, zero red; twenty compiled execution defects and
  one AL-control-shadow compile defect rejected. Four new controls cover object replacement,
  lost veto retries, default close actions and callback-policy bypass.
  `make ui-host JOBS=2` retains 154 UI/242 configuration checks and eleven compiled
  defects, zero red. Three affected compiled C++ consumers pass targeted clang-tidy.
  Three-round no-PCH header frontend means, before/after in milliseconds: UiHost
  1016.9/1018.4, Page 1689.2/1723.8, PageSession 1956.9/1961.6; not throughput evidence.
- Sources: `include/runtime/{Page,PageSession,PageInstance,UiHost}.h`,
  `src/rt/{PageInstance,UiHost}.cpp`, `test/runtime/page-navigation/{Modal.Page.al,Runner.cpp}`
  and its Bash harness. This is a generated-factory/trusted-host gate, not HTTP parity.
  The real native host still refuses `UiModalUnsupported`; original Customer New remains
  unaccepted. Active-modal HTTP authority, receipts, nested dialogs, child-close order and
  BC's modal-only query-close rule remain to qualify. Rebuild generated consumers after
  the PageInstance virtual-interface change before rerunning original Customer.
- Developer `methods-auto/page/page-{runmodal-,getrecord,lookupmode}-method.md` and
  `triggers-auto/page/devenv-onqueryclosepage-page-trigger.md` at
  `f928288ee840334be73142e5fc0202c0e19b246d`.
  Predecessor findings: `openerp/board/1705_lookupmode-bestimmt-schliessaktion.md` and
  `1231_onqueryclosepage-wurde-nie-gefeuert-currpage-lookupmode-war.md`; no Python
  threading or implicit modal commits adopted.

## Native command diagnostics

- `src/rt/PageCommandHost.cpp` emits bounded error HTML with exact AL text/classification,
  command identity and explicit outcome. `failed` requires durable page invalidation and
  a failed PostgreSQL receipt; prior explicit Commit is not undone. Cleanup/transport
  uncertainty stays `unknown`; refusal describes only the current submission.
- `src/client/{profile,http,errors,cmd,mcp,web}.mts` share strict error decoding.
  CMD/MCP retain structured diagnostics; htmx uses text, keeps the last accepted state and
  never retries. A mismatched command or malformed response remains WriteUncertain.
  The empty-code fallback `AlError` is transport classification, not changed AL state.
- `make client-test`: 41 cases, eight executable defects rejected; `make web-test`:
  thirteen Chromium cases, three compiled defects and the actual Caddy asset case pass.
  `make lint-one UNIT=src/rt/PageCommandHost.cpp JOBS=2`: zero findings. Native rollback,
  durable Commit and external CMD/MCP/browser error parity are exercised by
  `test/ui/page-host.mjs`; `make page-host-test` passes 94 native HTTP cases at limits
  40/7/80 under both TryFunction policies and rejects eight compiled defects,
  including a lost failed receipt and blocking HTTP wait. Generated navigation retains 183 checks,
  PageSource 15, PageDispatcher 105 and all sixteen execution defects/one compile refusal.
- Full ErrorInfo/actionable-error dialogs and modal execution remain required;
  this contract is not working business-workflow acceptance.

## Asynchronous native calls

- `PageCommandHost.cpp` owns a bounded AL executor separate from HTTP workers.
  Trusted `pages.execution_workers`, `execution_queue` and `response_wait_ms` are
  explicit in `deploy/dev/agiru.json`; no client parameter selects them. One context
  admits one call, retaining its AL session and transaction boundary on the AL worker.
- Shared HTML profile 3 describes `working`, an opaque call handle and original
  command. Authenticated `GET /calls/<handle>` rechecks PostgreSQL ownership,
  company/host/lifetime and page permissions; completed command snapshots reauthorize
  controls. CMD/MCP/htmx poll the same call without reopening or reposting. Polling
  is bounded; timeout leaves the write uncertain, never cancelled or retried implicitly.
- `test/ui/page-host.mjs` exercises delayed writes with one HTTP worker, independent
  uncommitted SQL visibility, foreign-call refusal, single write effects, rollback,
  durable Commit and actual external CMD/MCP/Chromium completion. `page-host.sh`
  requires a compiled synchronous-execution defect to fail that responsiveness case.
  `test/ui/{agent-client,web-client}.mjs` cover strict profile and polling/error identity.
- The real session-owned endpoint in `src/rt/PageInteraction.{h,cpp}` provides
  Confirm/StrMenu and deferred messages over the same AL call/SQL transaction.
  PostgreSQL owns explicit-answer fencing; `/answers` rechecks owner/company,
  original-action permissions, CSRF, revision and nonce. Identical answers reconcile
  without reexecution; changed, foreign and replaced answers refuse.
  `pages.dialog_timeout_seconds` defaults to 300. Timeout/shutdown unwind, never
  consent; presentation defaults are not answers. HTML byte refusal precedes publication.
  Messages appear at a question or completion, retaining Unicode and stable identities.
  Web/CMD/MCP consume one strict profile; only `working` polls automatically.
  Confirmed asynchronous failures restore the browser's previous inspected business page;
  that snapshot does not revive an invalidated server context or grant retry authority.
- `test/ui/page-host.mjs`: 114 positive native HTTP cases at limits 40/7/80 and
  both TryFunction policies, independently checking pending writes, rollback,
  prior Commit, nested questions, menu cancel/selection, messages and duplicate effects.
  `make page-host-test` rejects eleven compiled defects, including automatic default
  consent, implicit dialog Commit and changed-answer replay; source hashes remain unchanged.
  `make client-test`: 43 cases/eight executable defects; `make web-test`:
  fourteen Chromium cases/three compiled defects plus the actual Caddy asset case.
  Native configuration retains 242 checks; five affected C++ units pass clang-tidy.
  Progress explicitly refuses `UiProgressUnsupported`; live progress, modal/error
  dialogs, original Customer and business workflows remain due. Native disabled
  callback-policy, queue saturation, shutdown rollback and multi-user scale still need qualification.
- `make slice-check` retains 14,225 sources, zero missing; `make all JOBS=2` succeeds,
  retaining 1,902 diagnostic-slice unlinked gaps. Native configuration passes 232 checks.
  Targeted clang-tidy is clear for PageCommandHost, NativeServiceConfig, its gate and
  the generated HTTP fixture runner. Full AL execution and original Customer remain due.

## Linked-card creation

- `src/rt/PageCommandHost.cpp` advertises `$agiru.new` from the linked Card's
  InsertAllowed, not the noneditable List's flag. It opens that installed Card in New
  mode through the existing AL lifecycle; no selected key is copied. Unknown property
  expressions refuse. False also refuses forged commands and direct Create admission.
- `test/ui/page-host.mjs`: New, AL OnNewRecord initialization, exact saved SQL values,
  authenticated creator, one-row population increase, identical command replay and
  return to the retained list agree across external CMD/MCP/Chromium. Unedited New
  does not insert. Existing private state and SQL transaction/receipt boundaries remain.
- `make page-host-test JOBS=2`: 122 positive HTTP cases at 7/40/80 under both policies,
  zero red; thirteen compiled defects rejected, including wrong creation mode and
  ignored Card InsertAllowed. Navigation 183, PageSource 15, PageDispatcher 105 and
  sixteen execution controls/one compile refusal remain green. Native build and two
  affected C++ units pass clang-tidy. No production/generated header was widened.
- Actual Customer acceptance now retains eight cases: seven existing cases pass;
  New fails with `AlError: Unhandled UI: ModalPage Select Customer Templ. List`,
  outcome failed and the exact original command identity. Independent SQL retains all
  68 customers; the unchanged seed has three templates and Default Nos.=true.
  Native modal execution, template creation, view/filter transfer, empty-list creation,
  editable lists without CardPageId and post-creation list refresh remain unqualified.
- References at developer `f928288ee840334be73142e5fc0202c0e19b246d`:
  `properties/devenv-{cardpageid,insertallowed}-property.md`,
  `triggers-auto/page/devenv-onnewrecord-page-trigger.md`,
  `methods-auto/page/page-runmodal--method.md`. BCApps/user revisions below are unchanged.
  Predecessor `openerp/test/openerp/runtime/test_client_list_new_opens_card.py` and
  `test_client_untouched_new_line.py`, plus board 1730 comment 5, identify the linked-card,
  explicit-template and untouched-row contracts; their implementation is not transplanted.

## Bounded lists and collation

- SQL kernel implemented: `include/runtime/RecordWindow.h`, `src/rt/RecordWindow.cpp`
  and private `RecordSeek.h` share the actual Navigate seek/order logic. One SQL query
  reads at most the requested limit plus one continuation probe; no offset, client
  comparison, unbounded result trimming, source-row/cursor mutation or implicit Commit.
  Loaded values recheck permissions; reverse windows return declared forward order.
  `make record-windows JOBS=2`: 23,511 checks and five compiled bound/seek/reverse/
  permission/continuation defects rejected through the existing `record-order.sh`.
  Its full 37 navigation controls and the six refresh controls also pass. Three affected
  units pass clang-tidy; the standalone public header measures 223.7 ms frontend,
  three no-PCH rounds, not a build-speed or ERP-performance comparison.
- Native HTTP lists now consume `pages.list_rows` from `deploy/dev/agiru.json`, default
  40, through `NativeServiceConfig.cpp` and the generated SQL window adapter. Web,
  CMD and MCP receive profile-2 read-only rows, exact typed values and opaque selection
  commands from one bounded `PageHtml.cpp` renderer; cards retain profile 1. Selection
  retains SQL continuation boundaries. The current-row panel remains alongside rows;
  this is not final BC grid/UI acceptance. No URL/CLI limit or client-side sorting.
  Native HTTP acceptance is counted above: 19 fixture-host cases and 25 actual-entry
  cases per limit 40/7/80, zero red; both TryFunction policies and eight compiled
  ownership/revision/replay/policy/duplicate/list-bound/receipt/wait defects reject.
  Actual Chromium DOM rows/typed values/selection agree with external CMD/MCP;
  UTF-8 owned fixture SQL independently confirms population and unchanged row values.
  Native configuration has 232 checks, zero red. This is authored generated-page proof,
  not original Customer/workflow, production-collation, full ERP or UT acceptance.
  Seven affected native/configuration/generated-consumer units pass clang-tidy without
  additional suppressions; existing current-row HTML/scalar gates retain 164/34 checks
  and nine compiled refusal controls, zero red.
  `make client-test` and `make web-test` remain green, including the five agent and
  three browser execution defects; these preserve profile-1 transport regressions.
- Generated loading: `include/runtime/{PageWindow,PageInstance,PageSession}.h` separates
  loaded-row and selected-row triggers through `Page.h`. The production-only adapter
  retains raw SQL boundaries and selected post-trigger record fields, not a copied page,
  cloned codeunits/parts, client comparison or a reread that erases calculated values.
  One selected trigger follows all row callbacks; an unchanged selection and exhausted
  continuation do not make each displayed row current. Loading errors close the page;
  rollback remains the authenticated caller's boundary, not an implicit close/save.
  Authored generated AL fixtures under `test/runtime/page-navigation/` cover row
  calculations, selected identity, original stored images, 0/1/39/40/41/80 rows and
  limits 7/40/80. Five compiled trigger/selection/image defects supplement the existing
  eleven execution controls. Loading reuses Table's automatic FlowField/image read
  completion before AL triggers and restores only the chosen record's image.
  `make page-navigation JOBS=2`: 183 generated checks, PageSource 15 and PageDispatcher
  105, zero red; all sixteen execution defects and the AL-control-shadowing compile
  refusal reject. Three affected units pass clang-tidy; 120/121 function-length control
  passes. Three no-PCH header rounds measure PageWindow 6.8 ms, PageInstance 202.3 ms,
  PageSession 1,926.2 ms (before: 1,922.0 ms); not an application/performance comparison.
  Ordinary SQL List pages are the initial provider. Custom navigation, new/empty
  editable rows, temporary/virtual providers, inclusive refresh, current-row xRec,
  selected-page-variable/part state and lazy BLOB/byte budgets remain unqualified.
  HTTP now uses this API through the private `src/rt/PageListHtml.h` projection.
  Unchanged GETs retain row handles/projections without replaying row triggers; reads
  reauthorize cached controls before returning values. Byte/control/depth budgets bound
  presentation, not SQL BLOB read volume or query scan cost.
  Rebuild generated production factories before calling the expanded PageInstance API;
  the focused generated fixtures are current, not the full ERP image or AL UT population.
- Preserve server permissions/company and all filter groups. Continue in the declared
  key's SQL order with a unique primary-key tie-breaker; forwards/backwards boundaries
  must use the same collation/comparisons as ORDER BY and indexes. No client-side sort.
- Qualify the actual source database collation: case/accent sensitivity, Unicode,
  whitespace and digit-only Code values. PostgreSQL defaults and bytewise temporary
  comparisons are not BC parity. Keep equality, uniqueness, ranges, wildcard/`@` filters,
  temporary records and SQL ordering consistent; unsupported profiles remain gaps.
- Current native seed `agiru_client_seed_20261007b`, gate database and template1 use
  SQL_ASCII with libc `C`/`C`; Customer
  No./Name/Search Name inherit deterministic database-default collation. BC collation
  parity is unproven; `scripts/cronus_to_pg.py` already encounters mixed SQL Server
  column collations but the transfer does not qualify their preservation. Inspect
  `pg_database`, `pg_attribute`/`pg_collation` and original SQL Server column metadata.
- The window gate explicitly creates UTF-8 storage and tests both C and an ICU
  `und-u-ks-level1` nondeterministic column collation, independently confirming case/
  accent equality. Text equality/ranges, wildcard Code filters, OR filter group -1,
  mixed directions, primary ties, digit-only Codes, deleted anchors and pending-write
  rollback are covered. Existing native data remains unchanged; qualify a validated
  UTF-8 clone and source collation mapping before BC/Unicode claims. PostgreSQL 17's
  nondeterministic wildcard behaviour, temporary/virtual/sequence window adapters,
  projected/lazy BLOB reads, byte budgets and indexed scan cost remain unqualified.
- Acceptance in `test/ui/` and focused C++ gates: 0/1/39/40/41 rows, configured smaller
  and larger windows, equal secondary keys, filtered next/previous, concurrent changes,
  exact Unicode/typed values and identical web/CMD/MCP windows. Count queries/rows read;
  removing the bound or changing continuation comparison must fail named controls.

References: developer `devenv-table-field-text-search.md` and
`dev-itpro/cside/cside-change-database-collation.md` at
`f928288ee840334be73142e5fc0202c0e19b246d`; predecessor
`openerp/board/1742_bc-web-client-defects-reported-on-the-live-client-2026-09-30.md`,
comment 3, identifies skipped rows from numeric Code comparisons against SQL text order.
Implement through `src/rt/{PageCommandHost,PageHtml,Navigate,Selection,RecordFilter}.cpp`
and `include/runtime/{PageHostOptions,PageInstance}.h`, not separate client masks.
Page loading separates `OnAfterGetRecord` for loaded rows from one selected-row
`OnAfterGetCurrRecord` after that block; existing single-row/TestPage navigation still
uses both phases. Preserve source filters, calculated values and AL trigger effects.
Reference: `triggers-auto/page/devenv-onafterget{record,currrecord}-page-trigger.md`,
`methods-auto/decimal/decimal-data-type.md` at the developer revision above. The gate
checks exact persisted scale-20 Decimal values and a separately authored numeric
column at scale 28; this does not qualify changing BC's storage/assignment limits.
Predecessor `openerp/board/1713_arc-headless-client-protocol.md`, comment 3,
and `openerp/runtime/base/test_page.py::{display_rows,_display_pass}` identify accidental
current-row changes and shallow restoration of page globals. Do not transplant its
offset/thread/context machinery or clone mutable codeunit/part authority to restore rows.
`openerp/board/1768_custom_source_record_restored_through_variant.md` records calculated
values lost on a reread; its temporary provider remains a separate qualification.
Developer `devenv-system-defined-variables.md` and
`methods-auto/record/record-setautocalcfields-method.md` require original modification
values and automatic calculations on retrieval. Predecessor 1707 reports previous-row
`xRec` during selected-row changes, with an explicitly unproven initial-row rule;
that separate page-trigger behaviour is not established by stored-image regression proof.

## Production UI suspension contract

- Authorize and durably identify the opening operation before company login/OnOpenPage
  can ask a question. `PageCommandHost::Open` currently registers its public context only
  after both execute; opening admission and uncertain-write reconciliation remain missing.
- Bind the real UI host before AL executes. GuiAllowed is true only with working
  capabilities; preserve separate UT handlers and false for background execution.
- Preserve the suspended AL stack, variables and rollback boundaries. Confirm/StrMenu
  defaults are presentation, never answers; modal pages accept commands only on the active
  child and return the exact Action/selected record. Never replay AL up to a question.
- Queue Message until completion or the next user-interaction pause. Progress Open/Update/
  Close is separate, not a swallowed Message or an automatic answer.
- Completed idle commands release workers/connections. A callback inside an open write
  transaction retains that transaction/lease; pause itself never commits or rolls back.
  Bound suspended execution, deadlines and leases without a dedicated thread per user.
  Abandonment unwinds and rolls back unfinished work, preserving any prior explicit Commit.
- Qualify opening-time questions, nested modal selection/cancel, deferred messages and
  write-before-question visibility/rollback through HTTP/CMD/MCP/browser and independent
  SQL. Reject foreign/stale answers; reconcile identical retries without resuming twice.
  Errors preserve typed AL identity safely.

Developer revision `f928288ee840334be73142e5fc0202c0e19b246d`:
`methods-auto/system/system-guiallowed-method.md`, `methods-auto/dialog/dialog-{message,
confirm,strmenu,open}-method.md`, `methods-auto/page/page-runmodal--method.md`,
`methods-auto/database/database-commit-method.md` and
`administration/server-instance-settings.md::AllowSessionCallSuspendWhenWriteTransactionStarted`
(enabled by default). Predecessor `openerp/board/1713_arc-headless-client-protocol.md`
and `openerp/web/client/session.py::wait_for_answer` identify nested execution; their
actor thread and nested-call commits are not evidence of BC transaction boundaries.
Implementation: `src/rt/{PageCommandHost,SessionCommand,written/BuiltinsWritten}.cpp`,
`include/{runtime/PageSession,type/Dialog}.h`; acceptance belongs in `test/ui/`.

## Existing foundation and refreshed implementation review

- Browser entry: `src/client/web.mts`, `src/client/web/`, `scripts/build_web.sh` and
  `make {web,web-test,dev-web}` use local locked htmx and the shared HTML profile,
  response-effects check and exact command envelope. Development bearer credentials
  stay in tab memory; production sign-in remains pending. Caddy serves static assets;
  browser-document negotiation at `/` requires applying the updated container config.
  `make web-test JOBS=2`: ten Chromium/native-HTML cases and one actual Caddy
  document/asset/header/routing case pass; three compiled defective bundles fail named
  controls. `make client-test JOBS=2`: 37 cases, seven defects rejected. MCP discovery
  marks both operations as potentially destructive and non-idempotent: opening runs
  AL triggers that can write; the name `read` does not authorize automatic retries.
  `src/client/mcp.mts` and `test/ui/agent-client.{mjs,sh}` prove actual stdio hints
  and reject a compiled read-only/idempotent-hint defect. Reference:
  `triggers-auto/page/devenv-onopenpage-page-trigger.md` at the developer revision
  above and `src/rt/PageCommandHost.cpp::Open`. Agent preflight in
  `src/client/http.mts` requires the matching retained `/?handle=<page>` route;
  CMD/MCP refuse opening, mismatched, duplicate and foreign paths before any HTTP call.
  The opening-replay mutant must fail that named test. Updated
  `test/ui/http-server.mjs` command paths retain twelve passing Caddy/native HTTP/SQL
  transport cases using unchanged prebuilt producers; not original ERP workflow proof.
  Opening receipt recovery remains pending.
  Producer
  `PageHtmlGate`: 164 checks. These fixtures are separate from actual ERP HTTP/SQL
  parity. Current-row fragments are not forty-row lists;
  multiline input display and independent invalid-UTF-8 browser refusal remain unqualified.
  Predecessor `openerp/board/1833_client_assets_from_a_cdn.md`: pin/serve local assets,
  preserve licenses; no CDN or copied predecessor implementation.

- Session-owned native dialog bridge: `include/runtime/UiHost.h`, `src/rt/UiHost.cpp`,
  `src/rt/written/BuiltinsWritten.cpp` and `include/type/Dialog.h`. `make ui-host JOBS=2`:
  60 C++ checks, six compiled defects rejected. Background GuiAllowed returns false;
  a real installed endpoint routes Message/Confirm/StrMenu and live progress bindings.
  Defaults never answer; explicit UT adapters never fall back to production callbacks.
  Independent SQL proves no implicit Commit, rollback of unfinished question execution
  and durability of prior Commit. Nested/detached sessions and worker migration preserve
  host ownership. The native question/message endpoint and its current proof are
  described above; modal pages and automatic progress teardown remain pending.
  References above plus
  `methods-auto/dialog/dialog-{update,close}-method.md`; predecessor threading is not adopted.
  The earlier native `make test JOBS=2` rerun passed 181 cases, zero red, including
  257 tooling tests. Changed-code `make lint JOBS=2`: 28/330 units, zero findings,
  unchanged suppression count. Frozen native baseline at `2809174`: 185 cases, zero red,
  recorded in `e67438a`/0741; it predates native questions and linked-card creation.
  These gates/fixtures do not execute the source-counted AL UT.

- Original Customer regression: `make erp-client-test JOBS=2`, eight cases,
  **seven pass/one failure**, none skipped/cancelled. New/template remains red above;
  all seven existing list/card/edit/permission/receipt cases still pass.
  `test/ui/erp-client.mjs` follows the existing asynchronous opening call without
  reopening; `browser-client.mjs` recognizes typed server errors and compares their
  exact text/outcome. Real NativePermissions denial is `Permission/refused` before
  retaining a context or executing AL; CMD exits 2, MCP and Chromium agree, SQL unchanged.
- The original 40-row Customer List matches independent SQL primary-key order and
  every Code/Text value. External CMD/MCP and actual Chromium retain identical state.
  Card OnOpenPage now exposes the exact No. through the real session UI host, not
  forced GuiAllowed/visibility or object-specific runtime code. CMD/MCP/htmx each save
  Unicode Name values; SQL verifies modifier, increased version, all 68 customers,
  durable command receipts and released idle connections. Native build and targeted
  PageCommandHost clang-tidy pass; 1,902 diagnostic-slice unlinked gaps remain.
  `make web-test JOBS=2` passes fourteen Chromium cases, the actual Caddy asset case
  and all three compiled refusal controls; PageHtml retains 164 checks, zero red.
- Seed `agiru_client_seed_20261007b` and source revisions are unchanged; SQL_ASCII/C
  is not source-BC collation or complete Unicode qualification. New/template selection,
  lookups, posting, full business families and source-counted AL acceptance remain due.
  Owned test clone, copied binaries and credentials are removed after verification.
  Earlier Customer failures are recoverable from `ba3190a`; none were filtered out.
  References: developer `methods-auto/system/system-guiallowed-method.md` at
  `f928288ee840334be73142e5fc0202c0e19b246d`; BCApps
  `Layers/W1/BaseApp/{Sales/Customer/CustomerList.Page,CRM/Outlook/OfficeHostProvider.Codeunit,
  Sales/Customer/Customer.Table}.al` at `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`;
  user `sales-how-register-new-customers.md` and `includes/create_new_customer.md` at
  `bf5ffffa9b026e146d29f13a242daa5334ddf0d8`. Predecessor
  `openerp/board/1713_arc-headless-client-protocol.md`: real UI-host GuiAllowed and dialogs
  before a page handle exists; its Python threading/default-confirm machinery is not adopted.

- Native ERP client preparation: `make erp-fixture JOBS=2`,
  `test/ui/erp-fixture.sh` and `test/ui/erp/Prepare.cpp`: 26 C++ checks, exact independent
  read-back of all eleven original Company columns (including Id/audit/rowversion),
  68 Customers/2,820 G/L Entries/149 Items/44 Sales Headers preserved in an owned seed clone.
  Two explicit fixture users, one assignable tenant role with nine documented wildcard
  object kinds and one company-specific assignment; the other user has no assignments.
  Real native authority grants/denies Customer R/I/M/D and Customer List Execute, refuses
  another company and prevents an actual AL write (independent SQL remains unchanged).
  Non-fixture DSNs refuse before connecting; original Company and seed provenance are
  unchanged; clone/binaries/private credentials are cleaned up. Targeted tidy: 1/324 units,
  zero findings. Broad administrator grants are fixture data, never production defaults,
  implicit SUPER or exemptions. Generated system roles, complete seed/version parity,
  actual Customer HTTP/browser execution and workflow acceptance remain open.
  References: developer `devenv-permissions-on-database-objects.md` (explicit wildcard
  permissions) at `f928288ee840334be73142e5fc0202c0e19b246d`; original System
  `src/Tenant Database Tables/Company.Table.al`, package/revision pinned below;
  predecessor `openerp/board/982_mem-project-user-table-seeded-empty.md` identifies the
  missing User setup, while WI 1700's implicit SUPER/system-table shortcuts are rejected.

- Stored System permission tables: `src/gen/CodeunitWriter.cpp` binds the original
  Access Control (2000000053), Tenant Permission Set (2000000165), Tenant Permission
  (2000000166) and Tenant Permission Set Rel. (2000000253) by ID/name/namespace.
  `src/tc/Main.cpp` emits their ordinary typed headers/bodies/definitions and options,
  retaining original System module ownership; no duplicate runtime field dictionaries.
  `src/rt/Table.cpp` now resets and reads TableFilter as its exact stored expression;
  this does not implement row-security enforcement or FieldRef value coercion.
  `include/type/TableFilter.h::ToText` and `src/rt/Record.cpp::IsBlank` fix typed TestField
  diagnostics/blank checks without widening includes. Empty/nonempty equality, missing values,
  mismatches with both exact Unicode expressions, error identity and Init are executed;
  the original failing `TestEditingPermissions.cpp` instantiation remains subject to the
  complete integration rerun. Targeted tidy for runtime/runner: zero findings (324 available units).
  `make native-storage JOBS=2 AGIRU_SYSTEM_SYMBOLS=<verified-package>`: four original
  sources, 62 C++/SQL checks and two rejected source mutations (InitValue/Code width).
  Original R/I/M/D/X ordinals, Unicode filter roundtrip/reset, actual denied SQL writes,
  indirect rights, company isolation and revocation pass in an owned disposable database.
  Nonempty security filters still refuse explicitly. `GenNativeBindingGate`: 183 checks;
  `make native-table-ids`: 159 parser/25 executed writer checks and seven identity controls.
  CMake's existing platform library owns these sources; do not also register them in the slice.
  Full translation: 234 raw/233 selected System tables, 22 bound/211 unbound; zero table
  source refusals. Existing product exclusion remains separately counted. Translation stays
  red (5,683 property refusals and other retained gaps), not a green subset. Slice check:
  14,225 sources, zero missing. The actual platform library compiles/links all four new tables;
  complete native integration build now exits zero, including the repaired TableFilter
  diagnostics. Diagnostic slice linking retains 1,902 explicit unlinked-procedure refusals;
  successful compilation is not complete app or AL execution. Thirty native-source tooling tests pass;
  changed-code clang-tidy: 6/323 units checked, zero findings, no new suppression.
  This is not seed provisioning, virtual permission metadata, generated system roles,
  native Customer/browser execution, complete System inventory or AL UT acceptance.
  References: System `src/Tenant Database Tables/{AccessControl,TenantPermissionSet,
  TenantPermission,TenantPermissionSetRel}.Table.al`, package 29.0.55365.0 SHA-256
  `f59a4e4200af2b819670655302ce4ba4bfdd51e133cae6d4faf7896e5bba6b44`;
  developer `properties/devenv-{datapercompany,tabletype,replicatedata,initvalue}-property.md`
  and `methods-auto/record/record-{init,testfield-joker,testfield-joker-joker}-method.md` at
  `f928288ee840334be73142e5fc0202c0e19b246d` (TableFilter defaults to empty).
  BCApps `Layers/W1/Tests/Permissions/TestEditingPermissions.Codeunit.al`,
  `AssertTenantPermissionSetupEqualsTenantPermissionSetup`, at
  `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17` compares the original Security Filter fields.
  Predecessor `openerp/board/1810_permission_set_relation_tables.md` identifies missing
  relation/metadata providers; WI 1700's implicit SUPER/system-table exemptions are rejected.

- Native CRONUS transfer: `scripts/seed_demo.py` supports separate container/database/user
  endpoints, stdin SQL, checked bytea-to-Int64 rowversions, monotonic allocator reconciliation
  and complete sorted typed read-back. `--verify` rechecks every shared table after building/failed
  imports without copying or changing the original provenance nonce; complete/changed identities refuse.
  Native `agiru_client_seed_20261007b`: 1,864 source tables, 1,726 target tables, 1,363 shared;
  555 populated/808 empty tables, 49,893 rows, zero refused tables, `typed_readback=true` and
  `status=complete`. Rowversion allocator: 163,961. Original `agiru-pg/cronus` remains unchanged.
  Retain all 501 source-only/363 target-only table and 915 dropped/67 defaulted column identities
  in `agiru_seed_provenance`; this is shared-transfer proof, not a version-equivalent full ERP seed
  or sealed A/B template. Original Company/User and four permission tables are not transferred yet.
  `make verify-check VERIFY_CHECKS=SeedTransferGate`: 19 checks; full tooling: 251 checks;
  `make gate GATE=SqlRowVersionGate`: 123 checks, all green. Exact numbers/Unicode/row multiplicity,
  milliseconds and DateTime dates remain checked; no tolerance or uncounted normalization.
  Time uses the target SQL type to discard BC's documented 1754-01-01 carrier date only for Time.
  References: developer `methods-auto/time/time-data-type.md` at `f928288ee840334be73142e5fc0202c0e19b246d`;
  BCApps `Layers/W1/BaseApp/Inventory/Item/Item.Table.al`, fields 61/63, at
  `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`; predecessor
  `openerp/scripts/setup/cronus_bak_loader.py::_coerce` distinguishes Time from DateTime.
  Reproduce with `python3 scripts/seed_demo.py --verify --source-container agiru-pg
  --source-database cronus --target-container agiru-dev --target-exec-user agiru --into <interrupted-seed>`.

- Native entry: `src/cli/{Main,Services}.cpp`, `include/runtime/NativeService.h` and
  `src/rt/NativeService.cpp` provide `serve`, trusted `client-init` and one-time `client-token`.
  HTTP uses `NativePermissions` for Page Execute/source Read and transitive TableData access.
  Startup requires one exact original Company in the initial flat profile; multi-company
  schema routing is not implemented. No implicit provisioning, users, grants or SUPER.
  `make page-host-test JOBS=2`: 12 fixture-host and 17 actual-entry CMD/MCP/HTTP cases,
  independent SQL values/modifier/rollback/Commit/revocation checks, clean SIGTERM and
  three compiled ownership/revision/replay defects. Pages/data are authored fixtures,
  not Customer ERP, actual-browser acceptance or full object-execution permission proof.
  Native build, changed-code clang-tidy and 29 standalone header checks pass.

- Installed permission metadata: `include/runtime/PermissionSetRegistry.h` and
  `src/rt/PermissionSetRegistry.cpp` freeze exact system identities with logarithmic lookup;
  duplicate/late/tenant registrations refuse. `make permission-sets JOBS=2`: 127 checks,
  eleven compiled defects and ASan/UBSan. Real generated declarations/extensions still need
  emission and installation; absent system assignments refuse, never synthesize grants.

- Permission composition: `include/meta/PermissionSetDef.h`,
  `include/runtime/PermissionSets.h`, `src/rt/PermissionSets.cpp` resolve exact app/role/scope
  identities, operation-level direct/indirect rights, wildcards, target-bound extensions and
  role-local recursive exclusions. Missing sets, cycles, malformed policy and resource excess
  refuse; included security filters refuse until row enforcement exists. No synthetic SUPER or
  cross-user authority cache. `make permission-sets JOBS=2` owns the C++ gate and compiled
  counterprobes, not native SQL assignments, AL execution contexts or live client authorization.
  References at the pinned revisions below: developer `devenv-permissionset-composing.md`,
  `devenv-permissions-on-database-objects.md`, `properties/devenv-{included,excluded}permissionsets-property.md`;
  original System `Tenant Database Tables/{AccessControl,TenantPermission,TenantPermissionSetRel}.Table.al`
  and `System Enums/PermissionObjectType.Enum.al`. Predecessor WI 1700 and
  `openerp/runtime/permissions.py`: reject global whole-object exclusions and implicit SUPER.

- Native SQL authority: `include/runtime/NativePermissions.h`,
  `src/rt/NativePermissionSnapshot.{h,cpp}`, `src/rt/NativePermissions.cpp` resolve the
  active authenticated user/company from original Access Control/Tenant Permission rows
  in one bounded SQL snapshot. No cross-user cache, role-name grant or system-table exemption.
  `make native-permissions JOBS=2`: 41 checks, eight compiled defects and ASan/UBSan;
  independent SQL verifies denied writes. `RequirePage` rechecks Page Execute/source Read.
  Pre-service integration: 14,225 slice sources and 176/176 C++/tooling cases;
  not a current AL UT or ERP milestone.
  Original Code[20]/Code[30] role widths require explicit text casts in the recursive CTE.
  System catalogue installation, storage/schema migration, indirect AL contexts, row filters,
  in-flight revocation fencing and full object-execution authority remain unqualified. Source references:
  original `TenantPermissionSet.Table.al`; user `ui-define-granular-permissions.md` and
  BCApps `System Application/Test/Permission Sets/src/PermissionRelationTests.Codeunit.al`
  (`TestReduceToIndirectPermissionFromPermissionSet`), at the revisions pinned below.

- Selected transport: unmodified Caddy → private loopback libmicrohttpd → C++ handler;
  no ERP module, custom HTTP/TLS parser or Node ERP server.
  System libmicrohttpd supplies HTTP framing/polling/suspend-resume through a private
  C API, without a C++ ABI dependency. Library LGPL-2.1+ and Caddy Apache-2.0 notices
  remain installed under the libmicrohttpd Debian copyright and `/usr/share/doc/caddy/LICENSE`
  paths respectively; owned adapter stays MIT.
  Caddy serves static assets, replaces forwarding headers and disables upstream retries/cache. Development is
  loopback HTTP; local TLS/certificate persistence is qualified by `make dev-check`, while
  public ACME and production load remain unqualified. Native transport
  uses fixed workers, bounded queue/connections and per-request/aggregate body limits;
  AL/SQL never execute on its network event loop. Stop drains suspended requests.
  `make http-test`: twelve actual HTTP cases pass, including external shell CMD/SDK
  MCP, original encoded BC URI, exact Unicode/form/binary bytes, independent SQL
  transport records, static assets, forged headers, ambiguous framing, error/limit
  refusals, body release and bounded admission while workers block.
  `HttpServerGate`: eleven checks; existing `PageHtmlGate`: 164, zero red.
  Twenty-two standalone-header/forced-dependency controls pass; HTTP source and gate
  targeted tidy pass without new suppressions. Public transport header has no native
  backend/DB/session/thread includes; measured frontend cost is 764.9 ms over three
  no-PCH rounds, not a build-performance improvement or generated-app requirement.
  Handler SQL is a fixture receipt, not AL Validate/Save/posting or production
  authentication/session/permission parity. `Session(dsn, authenticatedUser)` now resolves
  the typed GUID/name from PostgreSQL's system User table and refuses blank/missing,
  unnamed, disabled/unknown-state or expired users, without a license gate. The host
  must authenticate first; this constructor does not verify credentials or permissions.
  `make session-identity`: 75 identity, 49 command and 32 native credential checks;
  twelve compiled identity/command/credential/provider defects reject;
  independent SQL verifies committed creator/modifier ownership. Nested failures and
  worker reuse restore private identity/language; the SYSTEM/blank harness stays compatible.
  Detached `Session(Guid)` retains private AL state without a DB connection or worker;
  `SessionCommand` borrows an exclusive connection, rechecks the system User account,
  commits successful completion and rolls back unfinished work. Explicit AL Commit
  survives a later failure. Five hundred idle contexts add no PostgreSQL connections;
  SingleInstances, temporary rows, language and workdate survive worker/lease changes.
  Session/epoch-bound cursors cannot poison another user's reused connection. Closed
  connections and nested-scope cleanup preserve the original AL error; normal cleanup
  failure propagates instead of terminating the process. DSN constructors remain harness
  adapters. Pool/admission/settings reset, operator provisioning, browser/password sign-in, in-flight revocation,
  company-close invalidation and page/table authorization remain pending.
  Eleven existing session/transaction/cursor gates retain 1,107 passing checks.
  Session/command/cursor/SingleInstance and new fixture targeted tidy pass. Connection
  retains three existing Execute complexity/trace findings; Transaction retains one
  existing unsafe trace-environment finding. No baseline or suppression was raised.
  Narrow SessionCommand/SingleInstance headers measure 0.8/40.7 ms frontend over three
  standalone no-PCH rounds; no build/runtime performance or scale claim.
  Unicode data is preserved; existing Code uppercasing is ASCII-only and remains a gap.
  Blocking-handler cancellation, durable command reconciliation and WASM transport
  remain unqualified. Preserve these limits in the next real page/SQL increment.

- Native agent authentication: `ClientCredentials` stores random bearer verifiers in
  PostgreSQL, bound by cascading foreign key to the original system User GUID; account
  deletion removes credentials instead of blocking the core user lifecycle. Only SHA-256
  verifiers reach SQL; expiry uses database time and revocation retains an audit row.
  `SecureToken` uses the existing private system OpenSSL dependency, never UUID/MT
  randomness or password-style fast hashing. Issuance is trusted-operator-only, not an
  anonymous endpoint. Credential lookup must precede SessionCommand's account-state
  check; neither grants page/table/company permission. The command host below supplies
  page handles/revisions; independent durable reconciliation remains pending.
  `make http-test` passes eleven transport and nine authentication cases through
  actual Caddy/private C++/PostgreSQL, with external CMD and official SDK MCP.
  Independent SQL checks two identities, accepted-call receipts, disable/expiry/
  revocation refusals and no idle DB connection; private fixture credentials are removed.
  Entropy/digest provider failures refuse; compiled counterprobes bypassing expiry,
  revocation, GUID ownership and crypto-failure guards fail their named checks.
  This fixture authenticates static semantic HTML, not production page authorization,
  AL saves/posting, browser login or complete ERP parity. Imported-seed/auth-schema
  migration, TLS deployment and concurrent revocation fencing remain unqualified.
  Public headers exclude SQL/session/page/native crypto implementations; ClientCredentials
  still inherits Guid's existing StringValue/vector dependency. No suppression was widened.
  Standalone three-round no-PCH frontend cost: credentials 1,230.8 ms, token 356.0 ms;
  measured during qualification, not a build/runtime performance improvement.

- Shared native command host: `PageCommandHost` executes installed production page
  factories over authenticated HTTP, not authored static HTML. PostgreSQL owns exact
  user/company/host identity, expiry, revisions and started/complete/failed command
  receipts. Bounded private AL state and list/card stacks survive request-local
  connections; idle time after completed commands retains neither worker nor connection.
  UI suspension inside an unfinished transaction is not implemented. Identical completed
  bodies replay after snapshot reauthorization; changed bodies, stale revisions, forged
  CSRF/Origin and foreign handles refuse. Failed writes invalidate private pages; a failed
  receipt never implies rollback of explicit AL Commit. Back rereads the selected list row.
  Restarted hosts refuse stale handles instead of guessing recovered AL state.
  Navigation/Save and AL fields/actions share the same semantic HTML forms and unchanged
  CMD/MCP adapters. Anonymous AL area/actions containers receive presentation-only IDs,
  not invented AL control names; host/AL action collisions explicitly refuse.
  `make page-host-test JOBS=2`: twelve actual Caddy/C++/PostgreSQL cases with external
  shell CMD and official SDK MCP. Independent SQL checks exact values, modifier GUID,
  update-trigger counts, receipt/revision ownership, revocation, rollback and Commit followed
  by an error. Three compiled ownership/revision/replay defects fail their named HTTP checks.
  Generated navigation retains 82 checks, dispatcher 105 and source 15; prior eleven
  execution/control-name counterprobes remain active. HTML retains 164 checks.
  Host, HTML producer/private escaping and changed C++ qualifier/gates pass targeted
  tidy without added suppressions; the host header compiles without runtime/SQL/native
  transport implementation dependencies. Shared escaping/button generation avoids
  separate web/agent markup paths. Complete generated apps must rebuild for PageInstance's
  new Save operation; this fixture qualification is not a full-app or UT result.
  Authorization is mandatory; the qualifier supplies SQL-backed fixture grants, not a
  full BC permission-set/security-filter/indirect-access provider. Only one explicitly
  configured company/database is accepted; its imported-schema binding remains unqualified.
  Session-owned TableData checks now cover typed/reflected reads/writes, buffered record/query
  reads, relations and FlowFields. ReadPermission/WritePermission use the same authority;
  WritePermission requires every Insert/Modify/Delete right. Authenticated sessions without
  a provider refuse; trusted account resolution uses the original User declaration below
  the AL access boundary, not a system-table exemption. Actual temporary buffers retain
  their documented no-SQL-rights policy. The host requires both page and table authorities;
  authorized fixture actions cannot read/insert a denied second table or expose its values.
  `make table-permissions JOBS=2`: 29 checks and two compiled permission-bypass/partial-write
  defects; independent SQL verifies refusal effects and exact user/company grant ownership.
  Eleven affected runtime gates retain 8,862 checks; identity/command/credentials retain
  75/49/32 checks and twelve compiled defects. Twenty-three standalone-header/dependency
  controls pass, including the forced-session authority control. Targeted tidy passes for
  the authority, Session, host, navigation, identity and HTTP qualifiers. RecordRef retains
  four existing findings; Table retains thirteen; Query retains its two existing findings
  (Build complexity 55 and QueryDef::Groups). The new permission gate inherits the latter
  header finding. No baseline/suppression was raised; full lint/apps/UT remain pending.
  These fixture providers are not native BC role composition: indirect/inherent rights,
  object Execute and security
  predicates remain unimplemented; filtered policies must refuse rather than grant all rows.
  Do not expose this as a fully authorized ERP server. Native entrypoint/provisioning,
  browser login/assets,
  full URLs/bookmarks/filters/parts/dialogs, typed errors/messages, reconciliation endpoints
  and multi-context record concurrency remain pending. Context/navigation/lifetime and
  receipt count/bytes have explicit initial bounds, not production scale guarantees.
  Full page property/mode policy, retention/cleanup and read-side AL error-state recovery
  remain unqualified.

- Preserve agiru's generated `PageDef`/control tree, typed bindings, `PageCore`,
  TestPage lifecycle, validated record primitives and regression gates.
  `src/cli/Main.cpp` currently provides the test runner, not an ERP agent client.
- `runtime/PageCore.h` now owns the presentation-neutral control interface;
  `runtime/test/PageTraps.h` isolates test trapping. `PageDispatcher` borrows that
  adapter and its declaration, requires authorization on every operation and rejects
  wrong identities/kinds and current hidden/disabled/read-only client operations.
  It reuses existing field/trigger bindings, not separate business rules. Display
  text/Option ordinals stay separate. `ReadValue` carries exact bound scalar values:
  Decimal scale, Int64 digits, Boolean tokens, temporal undefined/closing flags and
  Option/Enum ordinals/member names with qualified table/field domains. Global Enum
  object identities and variable/computed-expression bindings remain unqualified.
  RecordId storage bytes use explicit Base64; binary/media/filter values refuse rather
  than fabricate scalars. FlowFilters need the session filter model, not raw storage.
  `PageSession<P>` now owns the common lifecycle/validation/save/trigger/part kernel;
  `TestPage<P>` adds generated test controls, traps and explicit row-error collection.
  Production row-save errors propagate; production handles are not publicly copyable.
  `PageInstance` owns a production adapter around that kernel, not a TestPage base;
  `MakeInstalledPage` uses the same frozen catalogue and refuses missing/null/mismatched
  factories. Creation stays closed; open/move/RecordId selection preserve existing AL
  trigger paths. Composition keeps AL controls named Open/Move/Declaration unambiguous.
  Only registration definitions include the typed factory. All 2,836 generated page
  definitions now emit it; complete compilation/linking remains unproven.
  `make transpile` still exits 1: 21 unresolved extension operations and 5,683 refused properties
  remain counted, not a green-subset claim. Full SQL-backed authorization, modal
  suspension and complete production HTTP remain pending. The external
  Node CMD/MCP adapters now share one bounded semantic-HTML/HTTP agent library;
  fixture transport qualification does not establish ERP parity.
  `RenderPageHtml` renders one current row through ReadValue/Inspect, not a second
  execution model: ordered controls, exact machine attributes, display text and shared
  htmx forms. Handles/revisions/receipt prefixes/CSRF come from the future server;
  rendering them is not validation or persistence. It never saves/navigates/invokes.
  Authorization precedes dynamic visibility; hidden leaves disappear and disabled
  actions remain discoverable. Unsupported controls and scalar bindings remain
  counted alerts. Bounded bytes/declarations/depth refuse
  atomically; unsafe controls or malformed UTF-8 never silently normalize. The narrow
  UTF-8 validator reuses the existing codec without importing its Array/Regex headers.
  This is not a list window, part/dialog implementation, BC-styled UI or HTTP parity.
  Failed-new-row retry recovery, non-delayed primary-key insertion and complete lifecycle
  semantics are not qualified by extraction.
  Container verification: 105 dispatcher checks and 82 generated navigation/lifecycle/
  handle/request-page checks pass. Generated production list/card instances now retain
  selection across separate command leases; typed field validation/save is committed
  before the next command. Independent SQL confirms exact value and modifier GUID;
  later list navigation reopens the committed cursor safely. This authored fixture's
  allowlist is not SQL-backed authorization or actual HTTP/client ERP parity.
  Direct SQL also confirms missing refused inserts. Eleven execution mutants and the
  AL-control shadowing compile control reject. Scalar 34, semantic HTML 164 and codec 102 checks pass; nine additional
  scalar/HTML execution mutants reject. Generated controls also retain ControlValue
  without colliding with the new typed-value primitive. Generator page 42/report 27,
  catalogue 54 and isolation 17 checks passed on the preceding factory increment.
  Targeted dispatcher,
  scalar, HTML, default-adapter sources, both new gates and generated-fixture runner
  tidy pass; the dispatcher gate retains 16 findings in existing Page/PageSession/
  Table/Codeunit implementations. Codec-source tidy retains four existing Encoding/
  Regex/TimeSpan header findings; BodyWriter retains 20 findings including existing
  complexity/function-size debt, without suppression changes. Standalone include cost:
  PageInstance 205 ms, PageSession 1,788 ms (three frontend rounds, no PCH); not a runtime
  performance claim. Full integration/UT on this increment remains pending; 0058
  keeps the previous counted result.
- External agent implementation: `src/client/{profile,ascii,http,command,cmd,mcp}.mts`.
  `make client-test` builds the locked TypeScript package, consumes actual C++
  `PageHtmlGate --html` and checks a declared Node HTTP transport fixture on the host.
  Thirty-six tests pass: exact Decimal/Int64 and enum/temporal metadata, ordering,
  Unicode/entity/terminal framing, strict profile/schema bounds, shell CMD and actual
  MCP stdio discovery/read/set/action, stale/disabled/forged-command refusals,
  redirect/body/UTF-8/timeout handling, private authentication files and uncertain
  write identity without retries. Oversized ASCII explicitly refuses its presentation
  while JSON/MCP retains full structured values. Authentication-file opens are nonblocking;
  FIFOs refuse without a writer. A bounded child-process regression fails without this guard.
  Five executable scalar-rounding, disabled-fence, stale-revision, duplicate-POST and
  blocking-authentication mutants reject. C++ HTML still
  passes 164 checks; its changed producer's targeted tidy passes. Fixtures make no
  SQL/browser/production-authentication claim.
  parse5 8.0.1 supplies HTML tree/entity semantics; official MCP SDK 1.32.1 and
  zod 3.25.76 supply protocol/strict shared schemas, not another ERP implementation.
  Locked dependencies/notices stay client-only; Node remains outside `agiru-dev`.
  `~/Git/openerp/board/{1771,1772,1791,1903}_*.md` informed stable identities,
  compact/noninteractive output and disabled-action guards; no Python port.
  Current-row-only limits remain explicit: no production server, stored sessions,
  modal/list/part/BC navigation support or command-receipt reconciliation endpoint yet.
- Refreshed archive SHA256:
  `f654cb6576768cb90fff2e0fb701043139ab4d36723e7427a498132a2e2ee6a3`.
  Inspect `~/Git/openerp/openerp/web/client/{protocol,screen,page_model,ui,session,cli_api,request_page}.py`
  and `~/Git/openerp/openerp/cli/terminal.py` before porting contracts.
  Both clients share screen/page/dialog operations; HTTP text commands retain a
  server session between calls, emit keyframes/diffs and stop on dialogs/errors.
- Tests exist for real HTTP commands/files, ten shared posting/payment/customer
  scenarios and Playwright sales flows. These are review evidence, not newly run
  results. Scenario parity uses in-process CLI; screen parity intercepts the model
  passed to the HTML renderer. Neither proves exhaustive actual Node/MCP/DOM parity.
- Board records 2,542/2,578 for CI 89 on 2026-10-03 and 6,106 runtime /
  318 client gates on 2026-10-05. Denominators differ from agiru; no archived
  method-level terminal receipt was established by this review. No 99%-ERP claim.
- Do not transplant one actor thread per user, Python runtime/test architecture
  or Java report rendering. Persistent connections are released between calls
  in the refreshed session code; modal/Commit ownership still needs native proof.
- Import regression scenarios, not unresolved defects: lock-wait posting can
  leave partial results (1897); a shown-disabled action still executes (1903);
  list reads have an OFFSET/LIMIT first step, with deep keyset/FlowField batching
  still open (1868). Old non-TTY CLI auto-confirms defaults; agent CMD must not.

## Concrete ports (read alongside local BC guarantees)

Paths below are relative to `~/Git/openerp/`; reuse contracts, not Python product code.

| Existing mechanism | Reference | agiru implementation |
|---|---|---|
| Shared operation result: done/dialog/error, messages/windows/files | `openerp/web/client/protocol.py::_response`, `screen.py` | One typed C++ dispatcher result consumed by TestPage/HTML/CMD/MCP |
| Field edit/save versus delayed new row; parts refresh | `protocol.py::set_field`, `page_model.py::_collect_rows` | Existing validation/row-leave primitive; invalid text separate, bounded windows+1 |
| Keyframes on context changes, otherwise keyed control/row diffs | `openerp/cli/terminal.py::{lines,keyframe,diff}` | Node semantic-HTML renderer with stable IDs/revisions; always emit dialogs/errors fully |
| Cross-process sessions, stop command batch at a question/error, multipart files | `openerp/web/client/cli_api.py`, HTTP agent client | Typed HTTP handles, explicit answers, local files as bytes; no server filesystem paths |
| Nested modal lookup editable while caller waits | `openerp/web/client/session.py::wait_for_answer` | Explicit suspended command/page state; bounded executor, no dedicated user thread |
| Data-entry arithmetic/trailing sign/date shorthand | `openerp/web/client/entry.py`, `test_client_entry.py` | Server-owned culture-aware typed parser; don't copy hard-coded en-US rules |
| Analysis SQL groups, accumulators and view import/export | `analysis.py`, `analysis_definition.py`, `analysis_store.py` | Shared query plan, exact aggregates, bounded pivots; views keyed by user/company/page |
| Inspect and report request capabilities | `page_inspection.py`, `request_page.py` | Generated app/field/type/filter provenance and 0063's request model |

Port controls: the reference action path checks name membership but not Enabled;
enforce the latter server-side. Chart models convert measures to float; retain exact
typed measures until coordinate projection. Analysis import maps captions and drops
unknown fields; use qualified identities and explicit version/mapping refusals.

## Implementation order

- First-release presentation follows BC's page structure, navigation, controls and
  dialogs closely; defer an independent agiru redesign. Implement with agiru-owned
  code, not copied proprietary client sources. Retain the shared HTML/ASCII contract.
- Adopt BC-compatible URL semantics using agiru deployment origins: `company`,
  `page|query|report|table`, `mode`, `profile`, `bookmark`, `filter`, layout and
  documented presentation flags. Keep authentication/cloud routing separate.
  Use one typed parser/builder for AL `GetUrl`, browser history and CMD/MCP open.
  Preserve bookmark record identity, parameter escaping, field-name filter syntax,
  company isolation and permission checks. Never treat a deep link as authority.
  Observed list → card navigation retains the same bookmark but changes page 22 → 21;
  creating a sales order removes `mode=Create` and gains a bookmark after validation.
  Encode spaces as `%20`: the observed BC entrypoint treats `company=CRONUS+CH`
  as a literal plus and refuses the company. Do not use form-encoded URL builders.
  `node`/`dc` are observed navigation hints, not yet established portable contracts.
  Test malformed/duplicate/conflicting selectors, Unicode/quoted filters, stale
  bookmarks, reload/back/forward and links across permitted/forbidden companies.
  Reference: local developer `devenv-web-client-urls.md` and
  `methods-auto/system/system-geturl-clienttype-string-objecttype-integer-recordref-boolean-string-method.md`.
  Direct sandbox: 2026-10-05, CH BC 28.5, platform `28.0.54688.0`, application
  `28.5.54151.54951` (Help & Support). Keep this host distinct from frozen SDK/demo
  versions. Credentials/raw captures stay outside Git; private archive:
  `~/.local/share/agiru/bc-reference/2026-10-05/manifest.json`.

1. Use the shared `PageDispatcher`/`PageSession` and generated production factories.
   Complete one semantic model for modes, current key/version,
   accepted values versus invalid edit text, parts, dirty/new state and dialogs.
2. Preserve open/fetch/current-row/validate/save/action/close trigger order,
   delayed insertion, header-before-part saves, SetRecords, SubPageLink and
   UpdatePropagation. SaveRecord defaults come from source metadata.
   Enforce inherited mode/Visible/Editable/Enabled and permissions on the server.
3. Add one typed command registry: discover/open/read/set/select/invoke/answer/close,
   filters, navigation, lookup/drilldown, parts, files, reports and analysis.
   Stable app/page/control/row identities, session/page/dialog handles, revisions
   and command IDs; no caption selectors or client-local business tables.
4. Add a thin C++ HTTP adapter and semantic HTML/htmx rendering over that registry.
   Run blocking AL/libpq on a bounded executor, not the event loop. Sessions remain
   private; PostgreSQL owns shared permission revisions, fencing and receipts.
   Completed idle commands retain no connection. Suspended write transactions keep their
   lease and boundaries; modal suspension never adds an implicit Commit.
   Native HTTP uses the adopted libmicrohttpd/Caddy boundary; daisyUI/Tailwind remain
   unadopted presentation proposals.
5. Extend the Node.js/TypeScript HTML-to-ASCII agent client to production HTTP;
   CMD and local stdio MCP share one client library and the same business endpoints.
   Parse a bounded, versioned semantic HTML profile, not a general browser/htmx engine.
   No Node ERP server, Python sidecar or second business implementation.
6. Tagged Decimal/Int64 strings preserve exact values/scale; enum identity/ordinal
   stays separate from caption. Server parses submitted display text by culture/type.
   ASCII framing preserves Unicode content and safely escapes terminal controls.
7. CMD defaults to deterministic compact text, optional lossless JSON, stable
   error/exit codes, stderr diagnostics and no TTY/ANSI/blocking prompts.
   MCP uses structured schemas/results, stdout protocol-only and a pinned SDK.
   Cross-process handles continue the same session; explicit modal answers only.
   Bound pages/fields/output, report truncation and expose cursors/full values.
8. Authenticate/re-authorize every command and related-table read. Serialize session
   mutations, expire state and fence stale owners. Protect cookie commands with
   CSRF/origin checks; escape HTML; bound bodies/files and configured origins.
   Untrusted business text is data. Never blindly retry an uncertain posting:
   reconcile command receipts after ambiguous Commit/disconnect.
9. Implement charts and ledger analysis through the same registry: exact typed
   measures, filters, SQL groups/HAVING, sum/count/average/min/max, calendar groups,
   pivots, private saved views, exports and authorized drilldown. Persist definitions
   in PostgreSQL by user/company/page; bound scans/pivots/memory and cancellation.
   Share query/FlowField plans, not browser-local whole-ledger aggregation.
   SVG chart interaction shares scenes with report export under 0721.
   Role-center 9022/part 1392 sandbox sample renders an SVG aged-payables chart:
   Week has 14 bars, Day has 16. Period actions recompute buckets; clicking Older
   opens page 29 with Due Date/Open filters. Preserve AL DataPointClicked/index
   semantics and typed filters; independently reconcile totals before parity claims.
   Sources: `HelpAndChartWrapper.Page.al`, `BusinessChartBuffer.Table.al`.
   Analysis views persist independent column/group/pivot/filter/sort definitions;
   support create/save/rename/duplicate/reorder/delete and definition import/export.
   Copying must not alias the original mutable definition. Share links reopen an
   authorized copy and retain explicit company binding (or an explicit unbound
   choice), not grant data access. Prove persistence across reconnect and isolation
   across users/companies. Reference: user `business-central/analysis-mode.md`.
   Sandbox proof (2026-10-05, page 20): rename with description, account grouping,
   pivot mode and Duplicate work. Adding document-type grouping to the copy leaves
   the original unchanged; both definitions survive a full page reload. Original
   also pivots Document Type into column labels; the copy retains its own row groups.
   Exported
   `.analysis.json` preserves target object, column state, filters, pivot mode and
   dependencies. Share exposes company binding; recipient permissions remain untested.
   Packaged `analysisviews`/`DefinitionFile` on pages/extensions/customizations are
   documented, not yet exercised: immutable shared definitions, editable private
   copies and profile ownership. Read developer `devenv-analysis-view-package.md`.
10. Recreate the reviewed ten scenarios in `test/ui/` for actual shell CMD,
    MCP transport and htmx HTTP on equivalent disposable committed clones;
    use independently queried SQL effects. Sample the real browser/DOM separately.

## Acceptance

- Inventory product-scope business processes from `~/Git/dynamics365smb-docs/`
  (reviewed revision `0ff62b2266fd97265c1be00802b8ac23c0f22b2d`). Prioritize
  setup/master data → sales → purchases → finance → inventory/warehouse →
  remaining ERP areas. Keep every required process visible with commands,
  independent ledger/stock/SQL expectations and its acceptance status.
- Prepare a reproducible sales pitch: resettable demo seed, customer-to-payment
  and vendor-to-payment chains, stock movements, report/analysis samples and
  measured latency/resources. Real browser samples verify presentation;
  agent CMD executes every business step. Disclose gaps rather than claim 99% coverage.
- Every registered business capability has CMD/MCP/web operation coverage:
  messages/codes, typed values, ordered rows, filters/selections, modes, dialogs,
  permissions, artifacts and committed/rolled-back effects. Missing adapters and
  web-only extension/add-in business actions remain counted parity gaps.
- Independent AL/SQL expectations detect a shared wrong implementation. Mutants
  remove a handler/HTML identity, alter scale/order, bypass permissions, duplicate
  posting, use stale revisions or suppress truncation; each must fail.
- Preserve existing tests and G1's independently counted population; a green G1 is
  not a client-start prerequisite. Native durability/lock-timeout/Commit-followed-by-
  error tests are distinct from rollback-only client scenario fixtures.
- Agent workflows cover sales/purchases, journals, inventory/warehouse, recovery
  and report request options/filters/SaveValues/scheduling/downloads.
  Browser samples cover list/card/document+part, modal, validation, posting,
  upload/download, report, keyboard/focus and reconnect/stale DOM updates.
- Ledger/chart parity includes more data than a fetch block, authorized related
  fields and separate users/companies; unsupported expressions explicitly refuse.
  No speed or 99%-business claim from page-open counts or transcript equality.
- Reference/acceptance matrix includes attachment upload/download/delete and exact
  bytes; list filters/views and analysis grouping/pivots; FactBox selection refresh
  and drilldowns; nested lookup selection/cancel; Option/Enum identity versus caption
  and Code normalization; role-center business charts/periods/legends/drilldowns;
  report request/layout selection; workflows/approvals; and Database*/Table
  Information pages with authorized metadata and bounded live operational providers.
  Track observed, documented-only, refused and unexecuted cases separately.
- Workflow reference: pages 1500/1501/1505 and
  `System/Workflow/WorkflowSubpage.Page.al`. Copied purchase-approval template
  `MS-POAPW` has six conditional steps; its first response chain restricts the
  record, sets Pending Approval, creates and sends approval requests. Event filters
  include header and line tables and open through OnAssistEdit, only when editable.
  Own disabled copy was renamed (Code uppercasing plus related-record confirmation)
  and exported as XML; existing enabled workflows were untouched. Import overwrites
  an existing Code per `across-how-to-export-and-import-workflows.md`: require an
  explicit overwrite decision. Approval execution/delegation/job-queue recovery and
  posting restrictions remain unexecuted; XML export is not workflow execution proof.
- Attachment reference: own marked sales order → Attachments → page 1173;
  upload dialog accepts a local UTF-8 file, stores extension/type/user/time and
  Flow to Sales Trx. Download reproduced all 116 bytes (SHA256
  `99c3ef35b467e1484dd6b508b8eadd80f21cf66c8ee159c97eb6df5a1fd20281`);
  own attachment was deleted with confirmation and its downloaded copy retained.
  OneDrive was not exercised and is excluded. Native attachment/media ownership,
  posting transfer and independent user/company access still need qualification.

## References and consolidation

agiru: `include/meta/PageDef.h`, `include/runtime/{PageCore,PageDispatcher,PageInstance,PageSession,PageValue,PageHtml}.h`,
`include/type/Utf8.h`, `src/gen/{BodyWriter,PageWriter}.cpp`,
`src/rt/{PageInstance,PageValue,PageHtml,TestPage,Session}.cpp`, `src/net/Encoding.cpp`, `src/cli/Main.cpp`.
Command host: `include/runtime/PageCommandHost.h`, `src/rt/PageCommandHost.cpp`,
shared private escaping `src/rt/HtmlText.{h,cpp}`, `test/ui/page-host.{sh,mjs}` and
`test/ui/page-host/Runner.cpp`; generated AL input:
`test/runtime/page-navigation/{List,Card,CommandContract}.Page.al`.
URL subset follows developer `devenv-web-client-urls.md`; commits follow
`methods-auto/database/database-commit-method.md`, at the pinned revisions below.
Control-dispatch references: developer revision
`f928288ee840334be73142e5fc0202c0e19b246d`,
`properties/devenv-{enabled,editable,visible}-property.md`; BCApps revision
`d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`,
`Sales/Document/SalesOrder.Page.al` (`CopyDocument.Enabled`, inherited line state);
user `business-central/ui-enter-data.md` at
`bf5ffffa9b026e146d29f13a242daa5334ddf0d8`;
predecessor `openerp/web/client/protocol.py::{set_field,invoke_action}` and WI 1903.
Session identity: developer `methods-auto/database/database-{userid,usersecurityid}-method.md`;
BCApps `Modules/System/User/UserCard.Page.al` (State/Expiry Date) and
`System Application/App/User Permissions/src/UserPermissionsImpl.Codeunit.al` (enabled users);
user `business-central/ui-how-users-permissions.md` (disable/revoke); predecessor WIs
1449/1792 retain typed-ID/sign-in findings, not its deferred-authentication policy.
Revisions are those above. Implementation: `src/rt/Session.cpp`, `include/runtime/Session.h`;
proof: `test/gate/{SessionIdentity,SessionCommand}Gate.cpp`, `test/runtime/session-identity.sh`.
Native credentials: local developer `administration/users-credential-types.md` and
`administration/authenticating-users-with-navuserpassword.md` distinguish authentication
from Windows/cloud providers; agiru's agent credential is a native host contract, not
BC password-blob compatibility. Original `Tenant Database Tables/User.Table.al` owns the
foreign-key identity. Predecessor WI 1792's deferred authentication policy is not adopted.
OpenSSL private randomness/provider checks: [RAND_priv_bytes](https://docs.openssl.org/3.0/man3/RAND_bytes/),
[EVP_Digest](https://docs.openssl.org/3.0/man3/EVP_DigestInit/); local provider documentation
was unavailable. Source/test paths: `include/runtime/{SecureToken,ClientCredentials}.h`,
`src/net/SecureToken.cpp`, `src/rt/ClientCredentials.cpp`, `test/gate/ClientCredentialsGate.cpp`,
`test/runtime/client-credentials/ProviderFailure.cpp`, `test/ui/client-authentication.mjs`.
TableData boundary: developer `methods-auto/{record,recordref}/*-{readpermission,
writepermission}-method.md`, `devenv-permissions-on-database-objects.md`,
`properties/devenv-permissions-property.md`, `devenv-temporary-tables.md`; user
`business-central/ui-define-granular-permissions.md`, at the revisions above.
Original BC 29.0.54011.55407 System package SHA256
`f59a4e4200af2b819670655302ce4ba4bfdd51e133cae6d4faf7896e5bba6b44`:
`Tenant Database Tables/{AccessControl,TenantPermission}.Table.al` and
`Virtual Tables/AggregatePermissionSet.Table.al` establish identities/signatures,
not implemented providers. Inventory system/tenant compositions before installing one;
set exclusions are scoped to the expanded set, never a global grant deletion.
Predecessor WI 1700 and `openerp/runtime/permissions.py` informed transitive checks;
reject their broad system-table exemption, implicit SUPER/test grants, global exclusions
and Python context/thread machinery. Implementation: `include/runtime/TablePermissions.h`,
`src/rt/{TablePermissions,Table,Navigate,Query,RecordRef,Session}.cpp`;
proof: `test/gate/TablePermissionsGate.cpp`, `test/runtime/table-permissions.sh` and
`test/runtime/page-navigation/RestrictedRow.Table.al` through `test/ui/page-host.mjs`.
Persistent commands: developer `properties/devenv-singleinstance-property.md` and
`methods-auto/database/database-commit-method.md`; user
`business-central/ui-change-basic-settings.md`, at the revisions above. Predecessor
WI 1743 and `openerp/web/client/session.py::{acquire,release,release_after_call}` show
idle-connection exhaustion and per-call release, not an actor/thread architecture to port.
Implementation: `include/runtime/{SessionCommand,SingleInstance}.h`,
`src/rt/{SessionCommand,SingleInstance,Cursor,Transaction}.cpp`;
generated list/card lease proof: `test/runtime/page-navigation/Runner.cpp`.
Shared-kernel extraction: developer `triggers-auto/page/devenv-onopenpage-page-trigger.md`,
`properties/devenv-delayedinsert-property.md`,
`methods-auto/testpage/testpage-getvalidationerror-method.md` and
`devenv-report-triggers.md` at the developer revision above; predecessor WI 1113
retains reread/part-refresh findings. `test/runtime/page-navigation/{Delayed.Page.al,
Request.Report.al,Runner.cpp}` qualifies authored page/AL-test adapter behaviour,
not report rendering, full BC lifecycle or a live HTTP session.
Factory/lifecycle references: developer `methods-auto/testpage/testpage-{openview,
openedit}-method.md`; BCApps `Bank/Ledger/BankAccountLedgerEntries.Page.al` declares
the Open control; predecessor WI 1370 warns against replacing interactive instances
with repeated headless Page.Run calls. Reference revisions are those above.
Reproduce control dispatch with `make gate GATE=PageDispatcherGate JOBS=2`;
navigation/compiled refusal controls with `make page-navigation JOBS=2`.
These authored primitive tests are not live client or ERP parity evidence.
Scalar/HTML references: developer `methods-auto/decimal/decimal-totext--method.md`,
`properties/devenv-fieldclass-property.md`, `devenv-flowfilter-overview.md`;
BCApps `Finance/GeneralLedger/Account/GLAccount.Table.al` fields 28–30;
user `business-central/ui-enter-criteria-filters.md`; predecessor WI 1347 and
`openerp/web/client/page_model.py::std_filters`. Revisions are those above.
`make page-profile JOBS=2` owns the generated current-row/scalar/HTML acceptance and
compiled negative controls under `test/ui/page-profile.sh`; it is not complete parity.
Container reproduction:
`make dev-exec COMMAND='env AGIRU_TEST_DSN=postgresql://agiru:agiru@127.0.0.1:5432/agiru_gate make page-profile JOBS=2 B=/workspace/build/podman'`.
Local developer: page/control methods, `devenv-testing-pages.md`,
`properties/devenv-analysismodeenabled-property.md`; user:
`business-central/analysis-mode.md`. Read current local guarantees before porting.
Reviewed tests: `~/Git/openerp/test/openerp/runtime/test_client_{equivalence,scenario_parity,screen_parity,probes,e2e_sales,swallowed_db_error}.py`,
`~/Git/openerp/test/specs/client/`; predecessor findings 1771/1772/1791/1868/1897/1903.
Absorbs 0030 (including its previous absorbed IDs); detailed recovery:
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
