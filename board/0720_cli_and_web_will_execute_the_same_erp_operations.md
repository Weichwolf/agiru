# 0720 — Deliver equivalent web, agent CMD and MCP clients (G2)

Status: in progress | Priority: P0
Depends on: existing generated page declarations, typed record/session primitives
and database access, not full 0058 acceptance. Include client/workflow-blocking runtime
repairs in coherent client increments; preserve existing tests and counted UT failures.
Next: fix the current integration compile failure, then execute Customer List → Card → Validate → Save using external
CMD/MCP and representative htmx browser checks. Preserve the generic generated-page
HTTP/SQL contract below; do not replace permission enforcement with permissive stubs.

Development packaging: 0726 owns one server/web/PostgreSQL Podman container;
Node CMD/MCP runs outside over HTTP. Queued process families 0727–0740 own BC
sandbox reference execution and agiru replication. Their agiru prerequisites are
specific working client contracts, not this WI's full acceptance; no dependency cycle.
BC capture can proceed while client construction is underway. Keep one WI in progress.

## Existing foundation and refreshed implementation review

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
  `make native-storage JOBS=2 AGIRU_SYSTEM_SYMBOLS=<verified-package>`: four original
  sources, 53 C++/SQL checks and two rejected source mutations (InitValue/Code width).
  Original R/I/M/D/X ordinals, Unicode filter roundtrip/reset, actual denied SQL writes,
  indirect rights, company isolation and revocation pass in an owned disposable database.
  Nonempty security filters still refuse explicitly. `GenNativeBindingGate`: 183 checks;
  `make native-table-ids`: 159 parser/25 executed writer checks and seven identity controls.
  CMake's existing platform library owns these sources; do not also register them in the slice.
  Full translation: 234 raw/233 selected System tables, 22 bound/211 unbound; zero table
  source refusals. Existing product exclusion remains separately counted. Translation stays
  red (5,683 property refusals and other retained gaps), not a green subset. Slice check:
  14,225 sources, zero missing. The actual platform library compiles/links all four new tables;
  complete integration build failed after 740/1,064 steps with a generated C++ type
  conversion error; diagnosis/fix and a successful rerun remain required. Thirty native-source tooling tests pass;
  changed-code clang-tidy: 6/323 units checked, zero findings, no new suppression.
  This is not seed provisioning, virtual permission metadata, generated system roles,
  native Customer/browser execution, complete System inventory or AL UT acceptance.
  References: System `src/Tenant Database Tables/{AccessControl,TenantPermissionSet,
  TenantPermission,TenantPermissionSetRel}.Table.al`, package 29.0.55365.0 SHA-256
  `f59a4e4200af2b819670655302ce4ba4bfdd51e133cae6d4faf7896e5bba6b44`;
  developer `properties/devenv-{datapercompany,tabletype,replicatedata,initvalue}-property.md`
  and `methods-auto/record/record-init-method.md` at
  `f928288ee840334be73142e5fc0202c0e19b246d` (TableFilter defaults to empty).
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
  connections; think time retains neither a worker nor a connection. Identical completed
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
  Thirty-five tests pass: exact Decimal/Int64 and enum/temporal metadata, ordering,
  Unicode/entity/terminal framing, strict profile/schema bounds, shell CMD and actual
  MCP stdio discovery/read/set/action, stale/disabled/forged-command refusals,
  redirect/body/UTF-8/timeout handling, private authentication files and uncertain
  write identity without retries. Oversized ASCII explicitly refuses its presentation
  while JSON/MCP retains full structured values. Four executable scalar-rounding,
  disabled-fence, stale-revision and duplicate-POST mutants reject. C++ HTML still
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
   No connection or transaction during user think time; modal suspension is explicit.
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
