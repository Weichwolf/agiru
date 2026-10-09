# 0720 — Deliver equivalent web, agent CMD and MCP clients (G2)

Status: in progress | Priority: P0
Depends on: existing generated page declarations, typed record/session primitives
and database access, not full 0058 acceptance. Include client/workflow-blocking runtime
repairs in coherent client increments; preserve existing tests and counted UT failures.
Next: complete shared-client regressions for active-call expiry/revocation
cancellation, then expand invalid/duplicate/lookup/dimension and vendor/item/setup
processes under 0727.
Production enum declarations are regenerated; source-origin and slice checks pass.
The complete native ABI-consumer rebuild exits 0 (2702 seconds). Regeneration still
refuses 5708 properties and exits nonzero; this is not full-app or G1 acceptance.
The current original-client regression passes all twenty-three Customer cases. Preserve
explicit product exclusions; do not implement Microsoft-cloud services, substitute
constant feature answers, silence unselected controls or patch generated apps.
The native diagnostic-slice image compiles and links with `make dev-exec
COMMAND='make all B=/workspace/build/podman JOBS=6 KEEP=1'`, exit 0. Preserve the
1846 known unlinked-source refusals; complete generated-app compilation is not proven.
`make erp-client-test JOBS=2` passes preparation 22/22 and client 23/23,
with zero failures/skips/cancellations; outer exit 0. Original Customer List,
Card, template selection, creation, exact edits and independent reopening succeed
over external CMD/MCP and actual Chromium. All three stale-page cases now refuse
after a peer commits: independent full-row SQL and ledger fingerprints stay unchanged,
and the native diagnostic/command ID and durable failed receipt match each adapter.
Three further cases normalize ` ch ` to `CH`, refuse an absent Country/Region,
preserve the complete Customer and four ledger populations and independently reopen.
Three blocking cases save/reopen Ship, Invoice, All and the exact blank member through
CMD/MCP/Chromium. Independent SQL qualifies ordinals, audit/rowversion and original
OnModify timestamps while preserving all other fields/Customers/ledgers (0727).
Original dropdowns expose 0/space, 1/Ship, 2/Invoice and 3/All in source order across
all adapters. Three privacy cases execute the original Customer triggers: true sets
Blocked=All; changing to Ship suspends for an explicit Confirm with default No.
SQL proves no save while waiting; No preserves the entire row/audit/rowversion and
records the failed original command plus closed answer 0. The uncoded Error('') retains
empty text and production PageSessionDiagnostics classifies it as PageValidation.
Explicit Yes clears privacy blocking and saves Ship; an independent card reopens
false/1. Remaining business fields, other Customers and four ledger populations stay
unchanged. Browser setter/action waits share revision-or-dialog-state detection;
the real Chromium confirmation and released card are captured. Same-card continuation
after root validation failure, posting enforcement and BC visual parity are not qualified.
Enum/Option choices now flow from immutable table/type declarations through borrowed
PageValue spans into native semantic HTML selects and the shared TypeScript parser.
Values, AL member names and captions stay separate; enum display order follows the
declaration array, while Option uses its member sequence. Unknown saved ordinals remain
disabled selected placeholders, not fabricated writable choices. Missing enum order
or members is counted unsupported before emitting a partial field form. Read-only/list
scalars do not duplicate choice lists. ASCII escapes untrusted caption framing characters.
`make gate GATE=PageValueGate JOBS=2` passes 49 checks and PageHtmlGate passes 174.
`make client-test` passes 48 cases and all ten compiled defects reject, including choice
ordering and unsafe ASCII caption escaping. External CMD and real MCP stdio discover
the native choices and each submits one exact ordinal over HTTP. `make web-test` passes
15 Chromium cases plus the Caddy asset/routing case; all three compiled browser defects
reject. The browser selects the exact blank AL member and sends the same ordinal form
envelope as the agent library. This is native HTML/transport fixture qualification,
not original ERP SQL proof. Four affected C++ units pass targeted clang-tidy, zero
failures; not FULL lint. The PageValue standalone header comparison is 326.6 ms before
and 349.6 ms after (three no-PCH rounds each); no performance improvement is claimed.
Fresh BrowserHttp/BrowserSession/SessionIdentity gates pass 37/61/75 checks after
the complete native consumer rebuild. Active-call UI/commit cancellation is qualified
below; next-statement/blocked-SQL cancellation and cryptographic stolen-cookie binding
remain unqualified.
Page-level OptionCaption/ML overrides remain unqualified; no claim that sales/journal
posting enforces the chosen block.
The declaration-order prerequisite is implemented in `src/gen/EnumWriter.cpp`:
`EnumTraits::kDisplayOrdinals` retains source/base-before-extension order separately
from sorted `kValues`; it adds immutable ordinal metadata, not per-session copies.
GenEnumGate passes 28 checks, including the original Flushing Method Filter's
50-before-5 declaration and an empty enum. `make native-enums` passes 19 execution
checks; wrong ordinal, caption and sorted-display controls reject, as do the altered
interface signature and missing platform-owner module. Both app/slice source checks
retain 3/3 sources, zero omissions/errors; their CMake consumers compile.
The specialist now supplies its actual scope and verifies source provenance before
consumer compilation, then checks source/compiler hashes remained unchanged.
`make native-enum-package` qualifies all 28 enums of the verified system package,
including independently source-derived display-order assertions. Four changed C++
units pass targeted clang-tidy, zero failures; not FULL lint, ERP or dropdown acceptance.
Contracts: developer `f928288ee840334be73142e5fc0202c0e19b246d`,
`devenv-extensible-enums.md` and `properties/devenv-optioncaption-property.md`;
BCApps main `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`,
`Foundation/Enums/FlushingMethodFilter.Enum.al`; predecessor 1576 separates typed
Option values from captions. Reproduce using `make dev-exec` with
`make native-enums B=/workspace/build/podman JOBS=2`; package qualification needs
the explicit verified `AGIRU_SYSTEM_SYMBOLS` (SHA-256
`f59a4e4200af2b819670655302ce4ba4bfdd51e133cae6d4faf7896e5bba6b44`).
Sources: `test/gate/GenEnumGate.cpp`, `test/transpiler/native-enums{.sh,/}` and
`test/transpiler/native-enum-package.sh`. Production declarations and native consumers
are rebuilt, and original Customer choice/SQL parity passes above. These presentation
gates do not qualify every runtime enum-reflection operation.
The owned clone, binary/auth copies and native process are drained and removed;
an independent SQL check finds no remaining ERP fixture database. This proves these
workflows, not complete client/ERP parity, tenant matching, visual BC parity or scale.
The generic runtime repair now retains SQL observations independently of mutable
AL timestamp aliases, including assignment, Copy, Reset, Init and RecordRef.
Modify/Rename/Delete compare the observation atomically in SQL. A private UUID
column permits stale aliases only within their own uncommitted PostgreSQL transaction;
Commit/rollback end that authority. Successful writes require no full-row reread.
Pages no longer refresh away their stale observation immediately before input.
An actual blocked competing UPDATE rejects after the writer commits, preserving the
independent full row. Speculative sequence gaps are allowed; stored versions remain exact.
`make dev-exec COMMAND='env AGIRU_TEST_DSN=postgresql://agiru:agiru@127.0.0.1:5432/agiru_gate make rowversions B=/workspace/build/podman JOBS=2'`
exits 0: 136 allocator and 185
SQL checks, plus 25 compiled negative controls. All native consumers were rebuilt
before the successful original-client replay: the protected observation changes
the heap-owned RecordState layout.
Trusted `client-init` installs private ownership metadata on registered existing SQL
tables transactionally; it never creates missing ERP tables or changes existing
business/audit/rowversion values. The ERP qualifier migrates only its owned clone.
Contract: developer revision below, `devenv-table-system-fields.md` and
`methods-auto/record/record-{rename,modify,delete,copy,reset}-method.md`;
Rename explicitly distinguishes own uncommitted aliases from committed stale buffers.
Predecessor `~/Git/openerp/openerp/runtime/base/table/_table.py` carries cache versions,
not an optimistic-write specification;
its board has no more specific guarantee. PostgreSQL 17 `transaction-iso.html`
documents Read Committed predicate rechecking after a competing writer.
Sources: `include/runtime/{RecordState,Table,PageSession,Storage,NativeService}.h`,
`src/rt/{Storage,Table,Navigate,Temporary,SqlColumn,NativeService}.cpp`,
`src/rt/{Rows,SqlColumn}.h`, `test/gate/SqlRowVersionGate.cpp`,
`test/runtime/rowversions.sh`, `test/ui/erp-fixture.sh`.
Key-only/unobserved committed writes, broader API-page regressions and complete UT
acceptance remain unqualified; do not infer complete BC semantics from these gates.
All seven changed C++ units pass individual `make lint-one` checks, zero failures,
among 347 configured handwritten units; this is targeted, not FULL clang-tidy.
`make page-navigation` passes 15 Source, 270 generated and 122 Dispatcher checks;
all 34 execution controls and one control-name compile refusal reject.
Affected `make gate` runs are green: RecordImage 35, Temporary 95, RecordRef 161,
SelectionChange 364, Transaction 12 and TransactionContract 110 checks.
The generic missing-AL diagnostic still incorrectly labels AL members as .NET;
retain that gap separately. Temporary clones and private binary/auth copies were removed.
The pure O365 credentials page remains bounded-product-excluded (0058).
Expand cookie-mode question/modal and CMD/MCP write parity, then setup/master-data
workflows under 0727, retaining the accepted original
Customer template (1380) → create → edit → independent reopen regression. Regenerate
production variable setters and qualify original invalid-input/lookup/dimension cases.
Never substitute a default template, UT handler or client rule.
`Page.ObjectId([UseNames])` now uses declared metadata before page opening: exact
`Page <ID>`, current caption or AL-name fallback; missing metadata refuses. The
`make page-navigation B=/workspace/build/podman JOBS=2` in the development container
passes 15 Source, 270 generated navigation and 122 Dispatcher checks, zero red;
all 34 execution controls and the control-name compile refusal reject, including five
new identity controls. This does not implement excluded Power BI. Native integration
and original Customer replay pass the conflict checks above. The changed
Dispatcher consumer passes targeted clang-tidy: 1/344 units, zero failures, not FULL.
Contract: developer `f928288ee840334be73142e5fc0202c0e19b246d`,
`methods-auto/page/page-objectid-method.md`; BCApps main
`d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`,
`Layers/W1/BaseApp/Sales/Customer/CustomerList.Page.al` OnInit and
`Modules/System/PowerBI/Embedding/PowerBIEmbeddedReportPart.Page.al`;
predecessor board 1356 distinguishes CurrPage metadata from static object resolution.
Sources: `include/runtime/Page.h`, `test/gate/PageDispatcherGate.cpp`,
`test/runtime/page-navigation.sh`. Original-client rowversion acceptance passes on the
rebuilt image; the previous primary-key-only write/input reread defect is repaired
in the shared runtime and qualified by the SQL gates and client replay above.

Product-page selection: `scope.json` explicitly excludes only
`Layers/W1/BaseApp/Modules/System/PowerBI/` as Microsoft-cloud integration. The independent
raw census at the BCApps revision above retains all 47 module objects: fourteen codeunits,
fourteen tables, nine pages, six enums and four interfaces. Total raw population remains
36,792 objects / 113,111 test methods; explicit excluded objects rise from 205 to 252,
with excluded test methods unchanged at 3,744 and selected methods unchanged at 38,420.
The census still exits nonzero for the existing three sources/seven conditional variants;
it is not G1 acceptance. Generic visualization and mixed callers remain required.
`PageSelection.{h,cpp}` indexes only bounded policy targets, rejects ambiguous identities
and removes their parts after extension composition. Discarded direct
`CurrPage.<part>.Page.<method>` calls retain argument evaluation once, in AL source order.
Consumed values, non-CurrPage receivers, unknown ERP parts and generic chart add-ins
continue to refuse explicitly. `generation-product-parts.tsv` records owning source/page,
original part alias, target source/reason and raw statement location/member; cells escape
backslash, tab, CR and LF. Original procedure bodies/identities remain inventoried.
`make product-pages B=/workspace/build/podman JOBS=2` passes 22 generator and ten native
execution checks; all five compiled removal/domain/namespace/value/order defects reject.
Its independent fixture census retains five raw objects: one explicit exclusion, three
selected and one namespace omission. Existing GenPage 58 and GenCodeunit 83 checks pass.
GenScope now passes 297 checks after correcting stale assertions from before the reference
policy adoption (`70cdccd`), preserving namespace-only omissions as unclassified gaps and
testing bounded O365 suite classification separately from name-only matching.
Sources: `src/gen/{PageSelection,BodyWriter,CodeunitWriter}.{h,cpp}`, `src/tc/Main.cpp`,
`test/gate/{GenPageSelection,GenScope}Gate.cpp`, `test/transpiler/product-pages{.sh,/}`.
Changed-code `make lint B=/workspace/build/podman JOBS=2` covers 27/347 handwritten
units with zero failures; the twelve-suppression baseline is unchanged, not FULL lint.
References at the pinned revisions above: developer `properties/devenv-subpagelink-property.md`,
BCApps `Modules/System/PowerBI/README.md` and the embedded part's SetPageContext/
SetFilterToMultipleValues declarations, user `across-how-use-financials-data-source-powerbi.md`;
predecessor `scripts/transpiler/scope.py` and the retained Customer part/calls are evidence,
not permission to introduce successful missing-part stubs.
Production regeneration writes 18,378 objects (45 changed, zero swept), including the
original Customer List's preserved ObjectId/record/FieldNo argument evaluations. Its
receipt records nineteen excluded parts and fourteen discarded calls with ten valid
columns per row; generic controls and consumed values are not selected away.
The generator still exits 1: 21 unresolved extension anchors, 618 objects whose kinds
have no generator and 122 refused property declarations remain counted, not a green
generation claim.
The parser retains 77 UT codeunits / 2,298 methods and 38,421 total test methods;
the one-method discrepancy from the independent census remains open under 0058.
`make slice-check B=/workspace/build/podman JOBS=2` exits 0: 14,225 raw slice identities,
13,592 selected, 102 product-excluded, 531 omitted and zero missing.
Native integration exits 0 with the unchanged 1,761 unlinked-source refusals. Original
client execution reaches the next opening blocker above, not workflow acceptance.
CRM Integration Management 5330 was not explicitly product-excluded: the earlier
namespace omission was a required-dependency gap. The pinned original
`Integration/Dataverse/CRMIntegrationManagement.Codeunit.al` checks actual read permission,
stored connection enablement and session-owned state; IsCDSIntegrationEnabled invokes
an event. `CRM/Outlook/OfficeManagement.Codeunit.al` delegates IsAvailable to Office Host
Management's event, not a client-side constant. The root policy and independent census
must agree on any dependency activation. Predecessor board search has no specific
replacement contract; its broad namespace omission is not permission to invent one.

Dependency activation: root `scope.json` selects Dataverse/D365Sales/SyncEngine/Outlook
declarations, not new cloud requirements or changed method bodies. Independent raw
inventories contain 65/91/40/66 objects; all 262 identities were inspected before selection.
Policy SHA-256 `1ee1354a74089b46c72aa7e0984c41a5534e2885658e36358900082a47f66513`
selects 10,767 objects, with unchanged 252 explicit exclusions, 38,420 selected test
methods, 109,367 required and 70,947 required omissions. Contact Sync Test's 21 methods
remain area-omitted, not product-excluded; helper subscribers remain selected. The
census retains its three-source/seven-variant refusal. GenScope passes 315 checks;
SourceInventory/ProductSource qualifiers pass 20/7 tests with existing controls;
GenScopeGate targeted clang-tidy passes 1/347 units, zero failures.
Regeneration writes 18,917 objects (372 changed, zero swept), with real CRM/Office/table
references in Customer List. Parser UT stays 77/2,298 and total methods 38,421; the census
discrepancy remains open. Translation exits 1: 21 unresolved anchors, 618 untranslated
objects and 5,708 refused properties, including 2,303 ExternalName, 2,250 ExternalType
and 1,014 ExternalAccess declarations. Do not discard these or claim backend support.
Slice-check exits 0: 14,226 raw, 14,023 selected, 102 excluded, 101 omitted, zero missing.
Native integration exits 0 with 1,846 explicit unlinked-source procedure refusals;
original Customer replay after activation passes the eleven existing cases above. The first link correctly
refused missing data `kSynthRelationMappingBufferTable`; `test/slice` now includes its
original `base/core/table/SynthRelationMappingBuffer.def.cpp`, not a data stub.
Its namespace-less original `Integration/Dataverse/SynthRelationMappingBuffer.Table.al`
declares temporary table 5378 with no procedures; source SHA-256
`15ff05308758d1ba1a6bdbbd50553353f376d530f4c19421aebc869024aef99f`
matches the generated provenance. Existing slice identities remain unchanged.
Contracts at the pinned developer revision: `methods-auto/record/record-readpermission-method.md`,
`properties/devenv-{singleinstance,eventsubscriberinstance}-property.md`; original BaseApp
`Integration/{Dataverse,D365Sales,SynchEngine}/` and `CRM/Outlook/`. The sealed client
seed has no CRM/CDS connection rows; do not modify it to force disabled behavior.
User workflow intent and predecessor namespace policy do not replace original events
or permissions. No generator/runtime business-object-specific branch was introduced.

Immediate security prerequisite: qualify browser session transport below before accepting
externally accessible client workflows; retain the credential-specific ownership regression.

Development packaging: 0726 owns one server/web/PostgreSQL Podman container;
Node CMD/MCP runs outside over HTTP. Queued process families 0727–0740 own BC
sandbox reference execution and agiru replication. Their agiru prerequisites are
specific working client contracts, not this WI's full acceptance; no dependency cycle.
BC capture can proceed while client construction is underway. Keep one WI in progress.

## P0 security acceptance

- Login UX may wait; security does not. Preserve bearer/account/page/TableData checks.
  Any no-login development access is loopback-only and cannot become a public profile.
- Browser sessions: server-issued opaque CSPRNG tokens, hashed PostgreSQL verifiers;
  `__Host-agiru` cookie with `Secure; HttpOnly; SameSite=Strict; Path=/`, no Domain.
  No authentication secrets in URLs, HTML, logs or browser storage. Native development
  HTTP remains explicitly local; qualify cookies over actual Caddy HTTPS, not fake flags.
- Bind every page, call, dialog and command receipt to its client-session identity,
  user and company. Two credentials for the SAME user must not adopt each other's handles.
  CLI/MCP keep separate private credentials; sharing an authorized identity is not
  sharing its AL state. A copied bearer/cookie remains replayable: opaque tokens and
  IP/User-Agent checks are not sender-constrained cryptographic device binding.
- Server-enforced configurable idle/absolute deadlines, logout/revocation and account
  disable; rotate on login/privilege change, invalidate the old identity, fence suspended
  operations. Reconciliation of an uncertain write must never repeat its business effects.
- Session-bound CSRF plus exact trusted Origin for writes; Fetch Metadata defense and
  explicit absent-header policy for agents. SameSite alone is insufficient. Opening an
  AL page can execute writing triggers: audit GET/deep-link/prefetch entry points before
  cookie support, reject cross-site execution and keep initial navigation a static shell.
- Reauthorize every operation; qualify same-user/different-client and different-user/
  company/tenant access, stale/forged handles, replay and concurrent revocation. Also
  prove XSS/HTML escaping/CSP, no-store, bounded admission/rate/body/output/storage,
  upload/archive/XML limits, trusted proxy/TLS policy and secret-free diagnostics.
  Preserve atomic SQL effects under denial, timeout, disconnect and exhaustion.
- Current code inspection: `ClientCredentials.cpp` checks bearer expiry/revocation;
  `PageCommandHost.cpp` binds ownership to credential/user/company/host and checks write CSRF/Origin.
  `HttpServer.cpp` emits no-store; Caddy declares CSP. The actual HTTPS htmx client
  exchanges its development source credential once, retaining only CSRF in memory;
  loopback HTTP uses a separate tab-memory bearer adapter. Full SaaS isolation and
  password sign-in remain unqualified.
- Credential identity gates: 36 checks, zero red; `make session-identity` retains
  75 identity, 49 command, 48 value and 84 generator checks, zero red. Thirteen identity/
  command/credential/provider and six value defects reject. A same-user peer has a distinct
  SHA-256 verifier without changing AL UserSecurityId; legacy unbound contexts are
  invalidated without deleting command evidence. The earlier pre-cookie
  `make page-host-test JOBS=2` run passed 170/170 regular cases (38 fixture, 44 native
  for each of limits 40/7/80), without skips or cancellations; all 22 compiled defects
  rejected and input hashes remained unchanged. That historical acceptance is retained
  at e2d7a5f. The active-call increment's matrix retry is pending: its earlier attempt
  completed 38 fixture/44 native cases, then ended with SIGTERM/143 and early auth-file
  cleanup. This interrupted run is not acceptance; its owned service/database were removed.
  Independent SQL proves same-user read/write/replay/poll/question/modal/receipt denial
  without additional effects. Eight affected C++ consumers pass targeted clang-tidy.
- Local contracts at developer revision `f928288ee840334be73142e5fc0202c0e19b246d`:
  `developer/methods-auto/database/database-usersecurityid-method.md` and
  `administration/understanding-session-timeouts.md`. Predecessor 1730, comment 4,
  exposed idle-session connection leakage. Modal answer-idle renewal is separate from
  full BC session timeout/disconnect cancellation, which remains unqualified.
- SQL browser-session authority: `make browser-sessions JOBS=2` has 61 checks,
  zero red; nine compiled deadline/source/CSRF/admission/rotation defects reject.
  HMAC follows [RFC 4231](https://www.rfc-editor.org/rfc/rfc4231.html) vectors;
  provider failure refuses. Actual SQL statement/row tracing contains no cookie,
  source bearer or CSRF secrets. PostgreSQL stores only SHA-256 verifiers, caps
  idle/absolute expiry by source expiry, serializes per-user admission and atomically
  rotates/revokes identities without extending their original deadline. Independent
  connections prove commit/rollback and competing issuance. This is a storage gate,
  not HTTPS cookie, browser adoption, active-stack cancellation or SaaS acceptance.
  Five affected compiled C++ units pass targeted clang-tidy; existing identity,
  command, credential, session-value and generator regressions remain green.
  Sources: `include/runtime/BrowserSession.h`, `src/rt/BrowserSession.cpp`,
  `src/net/SecureToken.cpp`, `test/gate/BrowserSessionGate.cpp` and
  `test/runtime/browser-sessions{.sh,/MacProvider.cpp}`.
- Native cookie transport: `src/rt/BrowserHttp.{h,cpp}` is shared by production
  `PageCommandHost.cpp`. Browser ERP reads and writes require session-bound CSRF,
  same-origin Fetch Metadata, exact configured proxy authority/TLS and write Origin;
  navigation/prefetch, duplicate cookies and cookie/bearer ambiguity refuse before AL.
  Retained AL work strips raw cookies, bearers and forwarding authority. Agent bearer
  requests remain independent. Trusted `browser_sessions` settings are complete in
  `deploy/dev/agiru.json`; activation on HTTP refuses. Development defaults stay disabled.
  Same-origin bootstrap is passive; rotation/logout commit before issuing/deleting cookies.
  Fresh operations request idle renewal; actual cookie-mode SQL tests prove passive
  bootstrap and command replay do not renew it. Active UI waits and commit checkpoints
  now recheck authority below; arbitrary CPU/blocked-SQL cancellation remains unqualified.
  `make browser-auth JOBS=2`: 37 protocol/front-door and 280 configuration checks,
  zero red; nine compiled CSRF/metadata/proxy/origin/cookie/retention/HTTPS-policy
  defects reject. Eleven affected compiled C++ units pass targeted clang-tidy.
  Current SQL/session regressions retain 61 browser-session, 75 identity, 49 command,
  36 credential, 48 value and 84 generator checks, zero red; their negative controls
  still reject. Container runs explicitly select the container-local gate DSN.
- `make browser-https-test` uses the official Debian Caddy, private libmicrohttpd and
  PostgreSQL together in a disposable container, with external Node and Chromium clients.
  All nine TLS/cookie/CSRF/rotation/logout/source-expiry cases pass without disabling
  certificate verification; an untrusted CA is rejected. Forged forwarding headers are
  replaced and denials leave independent SQL probe effects unchanged. Eleven actual
  Chromium cases prove cookie adoption/no retained bearer, exact CMD/MCP list values,
  Validate/Save/GET-followed replay SQL effects, independent tab state, durable logout,
  malformed-grant refusal without bearer fallback, and expiry while AL still awaits
  an explicit answer. Logout now cancels suspended ConfirmWrite, CommittedConfirm and
  ModalNested before the thirty-second dialog timeout. Independent SQL proves closed
  unanswered dialogs/modals, failed original receipts, pending-write rollback, preserved
  earlier Commit and unaffected same-user agent authority. The original command ID
  survives expiry reset; no automatic answer/retry or additional SQL write occurs.
  This is generated-page transport proof,
  not full business workflows or SaaS acceptance. Fixture Runner passes targeted
  clang-tidy with zero failures.
  Existing regressions: `make web-test` passes 14 Chromium cases and one Caddy case;
  three defective bundles reject. `make client-test` passes 44 CMD/MCP cases and
  the 164-check native HTML producer; eight executable client defects reject.
  Durable sources: `src/client/{browser-session,web}.mts`,
  `test/ui/{browser-https.sh,browser-https.mjs,browser-page-https.mjs,trusted-chromium.sh}`,
  `test/gate/BrowserHttpGate.cpp`, `deploy/dev/Caddyfile`.
  Chromium first refuses the certificate, then imports its CA into a private NSS
  profile mounted only in the test process namespace. Personal trust remains unchanged;
  [Chromium Linux certificate management](https://chromium.googlesource.com/chromium/src/+/main/docs/linux/cert_management.md)
  consulted 2026-10-08. No certificate bypass or disposable-receipt WI dependency.
- Active-call authority: `make page-call-authority JOBS=2` passes **75/75** checks;
  seven compiled expiry/idle/sticky/commit/wait/serialization/own-account defects reject.
  One private guard binds each AL activation to its user, credential/browser verifier,
  CSRF, page, host and company. PostgreSQL rechecks run independently of pending ERP
  changes; questions/modals poll every 250 ms without accepting defaults. A sticky
  cancellation cannot be swallowed into a later explicit/implicit commit. Commit-time
  User/credential/browser/context row locks order concurrent revocation; the gate proves
  the actual SQL lock wait, not connection-start latency. Earlier commits remain durable.
  Pending self-account changes are not mistaken for committed external disablement;
  once committed, disablement refuses later commits. This is a primitive gate, not
  original User Card workflow acceptance or arbitrary CPU/blocked-SQL preemption.
  Native `make all` exits 0; the latest increment takes 11 seconds and retains all 1846
  unlinked refusals. Nine changed C++ units pass targeted clang-tidy, no suppressions;
  full lint, UT and complete AL execution are not qualified by these runs.
  Existing identity/command/transaction/durability/browser gates pass
  75/49/12/110/19/61 checks. Sources: `src/rt/{CommandAuthority,PageCallAuthority,
  SessionUser,PageInteraction,PageModal}.{h,cpp}`, `src/rt/{PageCommandHost,Session,
  Transaction}.cpp`, `test/gate/PageCallAuthorityGate.cpp`,
  `test/runtime/page-call-authority.sh`, `test/ui/browser-page-https.mjs`.
  Contracts at the pinned revisions above: developer
  `developer/methods-auto/session/session-stopsession-method.md`,
  `developer/methods-auto/database/database-commit-method.md` and
  `administration/understanding-session-timeouts.md`; BCApps
  `Modules/System/User/{UserCard.Page,User.Codeunit}.al` and
  `System Application/App/User Permissions/src/UserPermissionsImpl.Codeunit.al`;
  user `business-central/ui-how-users-permissions.md`; predecessor 1730/1775.
- HSTS: `deploy/dev/Caddyfile` sets `max-age=31536000` only for actual HTTPS,
  deferred so upstream responses cannot weaken it; no subdomain/preload commitment.
  The official Debian Caddy requires the compatible header block, not the newer `>`
  shorthand; the real TLS test rejected its missing header before correction.
  Fresh 2026-10-09 `make browser-https-test JOBS=2`, at 445ca73 after the native ABI
  consumer rebuild, passes nine protocol and eight
  Chromium cases, including HSTS on shell/assets/grants/denials; outer exit 0 and
  unchanged input/binary hashes. The owned TLS container, native binaries/private
  auth and test trust profile are removed. The earlier HSTS `make web-test JOBS=2`
  passed fourteen browser cases, the HTTP edge case and three compiled controls;
  HTTP and forged `X-Forwarded-Proto: https` do not emit HSTS. Earlier container runs of
  `make browser-auth`, `make browser-sessions` and SessionIdentityGate retained
  317, 61 plus one provider, and 75 passing checks; eighteen security defects reject.
  HSTS follows [OWASP session transport guidance](https://cheatsheetseries.owasp.org/cheatsheets/Session_Management_Cheat_Sheet.html)
  and [Caddy headers](https://caddyserver.com/docs/caddyfile/directives/header),
  consulted 2026-10-09. Active-stack revocation and full SaaS isolation remain unqualified.
- Predecessor `~/Git/openerp/board/1775_a_session_per_tab.md`: do not equate a shared
  browser authentication cookie with AL page state. Retain explicit independent page
  contexts; reject global session managers, URL credentials and unbounded tab sessions.
  Two actual tabs now prove passive shared authentication with independent page handles
  and AL variables; logout invalidates the other tab's next operation without SQL effects.
  Never claim stolen-cookie device binding.
- Current `make page-host-test JOBS=2` passes all 170 regular cases: fixture 38/38
  and production 44/44 for each of limits 40/7/80 and both TryFunction write policies.
  All 22 compiled defects reject; source/input hashes remain unchanged; outer exit 0.
  Source/navigation/dispatcher gates pass 15/270/122 checks, with 34 execution controls
  and one expected control-name compile refusal. The invalid-input case retains every
  Integer/Decimal/Int64/array/Enum family, all three adapters, exact diagnostics,
  independent SQL effects, rejected-command replay and explicit correction. It passes
  in 37.125 seconds in the default native profile without changing its 60-second limit.
  `test/ui/browser-client.mjs::browserFailure` deliberately injects an undeclared option
  into the actual DOM select before submission: this is a browser-tamper negative test,
  not an advertised choice or a replacement HTTP client. The native server must refuse it.
  Preparing input before registering the response waiter also avoids an orphan rejection
  when input preparation fails. All owned page-host databases/auth/binaries are removed.
  The historical cancellation remains recoverable at 2973049; its production latency
  cause is not proved repaired. Five CLI help launches cost 144–166 ms; no HTTP/SQL or
  production performance improvement is claimed. Active-stack revocation and complete
  business-process/BC parity remain unqualified.
- References: [OWASP session management](https://cheatsheetseries.owasp.org/cheatsheets/Session_Management_Cheat_Sheet.html)
  and [CSRF prevention](https://cheatsheetseries.owasp.org/cheatsheets/Cross-Site_Request_Forgery_Prevention_Cheat_Sheet.html),
  consulted 2026-10-08. Sources/tests: `src/rt/{ClientCredentials,PageCommandHost}.cpp`,
  `src/net/HttpServer.cpp`, `src/client/{http,web}.mts`, `deploy/dev/{Caddyfile,agiru.json}`,
  `test/gate/ClientCredentialsGate.cpp`, `test/ui/{page-host,http-server,web-client}.*`.

## Refreshed client contracts (2026-10-08)

- Implicit field arguments: `BodyWriter.cpp` and `CodeunitWriter.cpp` now share
  `RecordFieldArguments` in `RuntimeSurface.{h,cpp}`, replacing drifting method lists.
  Collection retains bare/quoted fields for ModifyAll, LoadFields, range/filter queries,
  CopyFilter, FieldActive, Relation and AreFieldsLoaded; non-field values are not schema.
  The combined native-codeunit qualifier below covers these calls.
  Ten unavailable-record operations compile and throw before subsequent AL
  effects; deleting Enabled from generated refusal declarations fails compilation.
  Native-attribute, source-ID, missing-definition and both Base64 controls still reject.
  This repairs dependency emission, not the excluded integration or core company-copy
  acceptance. Retain mixed cleanup code and its explicit refusals until qualified.
  References: developer `methods-auto/record/record-{modifyall,loadfields}-method.md`
  at `f928288ee840334be73142e5fc0202c0e19b246d`; BCApps `main`
  `src/Layers/W1/BaseApp/EnvironmentCleanupSubs.Codeunit.al` at
  `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`, SHA-256
  `a8af594ed0941128459ba06699d9d5cb15b0f0c7666b90d9c562328a9df510c2`,
  equal to the production source copy. User intent: `business-central/about-new-company.md`.
  Predecessor 1173 retains the cost of successful fallbacks instead of named refusals.
  Verified System manifest is 29.0.55365.0/runtime 18.0; artifact directory
  29.0.54011.55407 is not that manifest version. Package SHA-256 remains
  `f59a4e4200af2b819670655302ce4ba4bfdd51e133cae6d4faf7896e5bba6b44`.

- Indexed unavailable members: `CodeunitWriter.cpp` now collects fields/methods after
  balanced indices, ignoring brackets inside string literals. Five compiled AL cases
  cover direct/nested/multidimensional reads, an index with a side effect and Validate's
  implicit field. Refusals retain member identity, evaluate the index once and stop
  before later effects; removing the matrix-only declaration fails compilation.
  The combined qualifier retains the existing negative controls. Mixed-case unavailable type
  identity and the other production build failures remain open; this is not ERP proof.
  References: developer `methods/devenv-array-methods.md` at `f928288ee840334be73142e5fc0202c0e19b246d`;
  BCApps `main` `src/Layers/W1/Tests/ERM/CopyPriceDataTest.Codeunit.al`, lines 2072/2074,
  at `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`; predecessor 1389 distinguishes
  array element access/extent from collection mutation. Durable fixtures:
  `test/transpiler/native-codeunits/{source/NativeFixture.Codeunit.al,Runner.cpp}`.

- Unavailable scalar operations: `dotnet::Refused` implements explicit refusal for
  +=, -=, *= and /=; `Clear` rejects marked unavailable members/options instead of
  replacing them with a successful default. Six ordinary AL cases retain member
  identity and stop before later effects; removing the operations fails both assertions.
  `make gate GATE=RefusedGate JOBS=2`: 11 checks, zero red. Ordinary Integer Clear remains
  implemented. `make lint JOBS=2` checks 53 affected compiled units with zero failures;
  format passes and the suppression baseline remains 12. Whole absent-record Clear and
  same-type Refused member assignment remain gaps.
  References: developer `devenv-al-operators.md` and
  `methods-auto/system/system-clear-joker-method.md` at the revision above; BCApps
  `main` `src/Layers/W1/Tests/TestLibraries/LibraryCRMIntegration.Codeunit.al` at the
  revision above. Predecessor 1389 requires Clear to preserve array extent rather than
  invent a collection reset. Durable tests: `test/gate/RefusedGate.cpp` and
  `test/transpiler/native-codeunits/{source/NativeFixture.Codeunit.al,Runner.cpp}`.

- FieldNo return shape: `BodyWriter.cpp` preserves the documented Integer result
  before overload resolution, including unavailable Record and indexed receivers.
  Four compiled refusal cases retain member identity and stop before later effects;
  selected Record FieldNo reaches Integer, while a Codeunit FieldNo retains Text.
  Removing the cast restores the Integer/JsonArray ambiguity and must fail compilation.
  The combined native-codeunit qualifier below retains this typing control.
  Side-effecting indexed method
  receivers remain unqualified; constant-index typing is not a once-only evaluation proof.
  References: developer `methods-auto/record/record-fieldno-method.md` at the revision
  above; BCApps `main` `src/System Application/App/Retention Policy/src/Retention Policy
  Allowed Tables/RetenPolAllowedTables.Codeunit.al` at the revision above. Predecessor
  1094 distinguishes intrinsic AL names from similarly spelled fields/procedures.
  Durable fixtures: `test/transpiler/native-codeunits/{source/*.al,Runner.cpp}`.

- Unavailable TestPage parts: `BodyWriter.cpp` and `CodeunitWriter.cpp` refuse
  First, nested Visible/DrillDown and SetValue calls at their actual execution point,
  retaining the original AL path. Compiled fixtures prove argument evaluation once,
  no subsequent effects and an untouched untaken branch; selected parts retain their
  declared field calls. Removing refusals or argument evaluation fails execution.
  `runtime/Page.h` no longer returns successful defaults for absent controls/add-ins.
  Its shared `IsAlRefusal` marker prevents Variant fallback/conversion ambiguity.
  Compiled AL Variant assignment/arguments refuse before callee effects; removing
  the marker must fail compilation with the original ambiguity.
  The former headless no-op and its historical nine-UT justification were not a
  platform guarantee. Keep any resulting AL UT losses visible; implement required
  charts/add-ins rather than restoring successful stubs or expanding exclusions.
  Actual part-entry host writes/triggers remain unqualified, as predecessor 803 warns.
  `make native-codeunits JOBS=2` with the verified System package: generator 74,
  executable refusals 98, source-bound 8 and original Base64 61 checks, zero red;
  all existing negative controls reject. `make gate GATE=PageDispatcherGate JOBS=2`:
  113 checks, zero red. Ten affected compiled units pass clang-tidy; format passes
  and the suppression baseline remains 12. This is not a FULL=1 claim over all 344 units.
  References at the pinned revisions above: developer
  `methods-auto/testpart/testpart-first-method.md` and
  `methods-auto/testfield/testfield-{visible,setvalue}-method.md`; BCApps `main`
  `src/Layers/W1/BaseApp/Modules/System/User/UserCard.Page.al` and
  `src/Layers/W1/BaseApp/Sales/Customer/CustomerCard.Page.al`.
  Local headless/no-op evidence was missing; official
  [control add-in asynchronous considerations](https://learn.microsoft.com/en-us/dynamics365/business-central/dev-itpro/developer/devenv-control-addin-asynchronous-considerations),
  consulted 2026-10-08, specify asynchronous void calls/events, not this fallback's
  correctness. A real add-in transport must preserve that contract.
  Durable tests: `test/gate/{GenCodeunit,PageDispatcher}Gate.cpp` and
  `test/transpiler/native-codeunits/{source/PartHost.Page.al,source/NativeFixture.Codeunit.al,Runner.cpp}`.

- Unavailable AL type identity: `Names.cpp` encodes the case-folded raw name and object
  kind, preserving spaces, punctuation and distinct kinds instead of using a sanitized
  C++ display name as identity. `CodeunitWriter.cpp` merges members across equivalent
  spellings; `Main.cpp` emits deterministic original-name diagnostics. Scalar and indexed
  `var Record` calls now share their type. Whole-object copy assignment refuses before
  later AL effects; it must not silently succeed after case aliases are merged. .NET
  aliases remain separate. Namespace/using resolution, Unicode case folding, absent
  storage and inherited by-value conversions remain unqualified.
  `make native-codeunits JOBS=2` with the verified System package: generator 78,
  executable 107, source-bound 8 and original Base64 61 checks, zero red. All existing
  controls plus default-copy assignment, unrelated `var` type and missing merged-member
  mutations reject. GenNamesGate has 39 checks, GenPageGate 58 and GenInterfaceGate 35,
  zero red. Changed-code lint covers 31 of 344 units with zero failures; format passes
  and suppression baseline stays 12. Original group 526 containing
  `CopyPriceDataTest.cpp` now compiles with production headers/PCH and warning policy:
  `make dev-exec COMMAND='cmake --build /workspace/build/podman -j 2 --target
  CMakeFiles/agiru_slice.dir/Unity/unity_stable/526/root_cxx.cxx.o'`.
  This is original-callsite compile proof, not storage/integration execution or full linking.
  References at the pinned revisions above: developer `devenv-al-variables.md` and
  `devenv-namespaces-structure.md`; BCApps `main`
  `src/Layers/W1/Tests/{ERM/CopyPriceDataTest,TestLibraries/LibraryCRMIntegration}.Codeunit.al`.
  Predecessors 779/850/810 warn against normalized-name collisions, miss-only guesses
  and counting source matches as emitter proof. Durable fixtures:
  `test/transpiler/native-codeunits/{source/AbsentPeer.Codeunit.al,source/NativeFixture.Codeunit.al,Runner.cpp}`.

- Missing selected-schema fields: `BindTable` marks parsed field schemas explicitly;
  incomplete indexes are not evidence of absence. `BodyWriter.cpp` refuses missing
  scalar/indexed field access at its reached branch, retaining the original table/member
  and evaluating the receiver once. Known storage, declared procedures and intrinsic
  Record methods remain unchanged. No column, license default or whole-method refusal
  is invented. The license-plan fields come from the unselected
  `System Application/App/Azure AD Plan/src/User Details/PlanUserDetails.TableExt.al`;
  keep the mixed `UserDetailsTestLibrary` and its core user/SUPER-permission operations.
  `make native-codeunits JOBS=2`: generator 83, executable 114, source-bound 8 and
  original Base64 61 checks, zero red; all existing controls and a successful-default
  mutation reject. GenTableGate has 105 checks and GenPageGate 58, zero red.
  Changed-code lint covers 25 of 344 units with zero failures; format passes and the
  suppression baseline stays 12. This is not full-surface lint acceptance.
  Original group 421 containing `UserDetailsTestLibrary.cpp` now compiles with production
  headers/PCH and warning policy:
  `make dev-exec COMMAND='cmake --build /workspace/build/podman -j 2 --target
  CMakeFiles/agiru_slice.dir/Unity/unity_stable/421/root_cxx.cxx.o'`.
  Its Get/HasSuperPermissionSet bodies and pre-access Get/error checks remain intact.
  Full linking remains pending. Implicit field-method arguments,
  chained results and nonconst var binding to missing fields remain unqualified.
  References at the pinned revisions above: developer `devenv-table-ext-object.md`;
  BCApps `main` the extension above and
  `System Application/Test Library/User Details/src/UserDetailsTestLibrary.Codeunit.al`;
  user intent `business-central/ui-how-users-permissions.md`. Predecessor 1173 warns
  against successful defaults shifting the actual error; donor
  `scripts/transpiler/generator/body_emitter/_emitter.py` keeps unknown receiver types
  conservative. Durable tests: `test/gate/GenCodeunitGate.cpp` and
  `test/transpiler/native-codeunits/{source/NativeFixture.Codeunit.al,Runner.cpp}`.

- Production regeneration retains 21 unresolved control anchors, 618 unsupported
  object kinds and 122 refused properties: translator exit 1, source-origin check 0.
  It writes 18,378 objects, with 37 changed and zero swept after the missing-field repair.
  Canonical unavailable declarations contain 283 AL types/2,492 members; separate
  .NET declarations contain 383 types/1,279 members. These are refusal declarations,
  not implemented objects or accepted tests. Parser test populations
  stay at 38,421 and the 77-codeunit/2,298-method milestone subset; the independent
  0058 source census remains separate, not a parser or executed-test result. Slice-check:
  14,225 raw entries, 13,592 selected, 101 product-excluded, 532 omitted, zero missing.
  Do not confuse these diagnostic source entries with the full AL test population.
  The previous keep-going build failed six groups (526/650/551/028/421/140).
  After regeneration, group 140 including original `RetenPolInstallBaseApp.cpp` compiles
  with the production PCH, warning policy and source-selection checks:
  `make dev-exec COMMAND='cmake --build /workspace/build/podman -j 2 --target
  CMakeFiles/agiru_slice.dir/Unity/unity_stable/140/root_cxx.cxx.o'`.
  Current original `UserCardTest.cpp` and `LibraryCRMIntegration.cpp` groups also compile
  with the production PCH, warning policy and unchanged source-selection counts:
  `make dev-exec COMMAND='cmake --build /workspace/build/podman -j 2 --target
  CMakeFiles/agiru_slice.dir/Unity/unity_stable/551/root_cxx.cxx.o
  CMakeFiles/agiru_slice.dir/Unity/unity_stable/028/root_cxx.cxx.o'`.
  The O365 credentials source is explicitly excluded; remaining generic field/type
  repairs and full linking remain open. This narrow compile is not full native/ERP acceptance.
  Named Page.Run/RunModal now lower through the numbered runtime API, with an explicit
  kind/name refusal before dispatch instead of an invented absent class or object zero.
  Compiled fixtures cover optional records/field numbers, Action results and mixed-case
  calls; known-page execution remains a separate regression.
  Removing all six named-page calls fails both identity and subsequent-effect assertions;
  the combined native-codeunit qualifier above retains these controls.
  `make page-navigation JOBS=2` with the container-local gate DSN: 270 navigation,
  105 dispatcher and 15 source checks, zero red; all 29 execution controls and one
  compile refusal reject. The deliberately removed unbound-integer guard crashes its
  mutant; production checks do not crash. Native integration remains pending.
  The 144-consumer lint run exposed Load_ complexity in three PageSession consumers
  and a fixture operation-count finding. Both are repaired: Load_ now separates the
  source-record guard from Load_Record_Window_ without changing its SQL/trigger order.
  All three compiled header consumers and the fixture pass focused lint; format and
  changed-code checks pass, suppression baseline stays 12. Navigation retains all
  390 regular checks, 29 execution controls and one compile refusal. This is affected
  consumer qualification, not a new FULL=1 claim over all 344 handwritten units.
  References: developer `methods-auto/page/page-{run,runmodal}-integer-table-integer-method.md`
  at the revision above; BCApps `main` `src/Layers/W1/BaseApp/GlobalAdminMessage.Page.al`
  and `src/System Application/App/Azure AD User Management/src/User sync/AzureADUserUpdateWizard.Page.al`
  at the revision above. Predecessor 1713 retains explicit unknown-platform refusals.
  Do not broaden exclusions or install a successful integration placeholder for compilation.

- P0, before sales/purchase replay: each row owns its OnAfterGetRecord-derived
  variables, arrays and totals; rendering a pending input row must not reset them.
  Reposition after posting from AL's current key/view, and initialize a new line only
  when appropriate. Empty temporary editable lists and dynamically editable parts
  are input surfaces too. Qualify original journals, Apply Entries and document totals.
- P0: preserve declared identities, not normalized-name precedence. Distinguish
  LocationCode from "Location Code" and AmountToApply from "Amount to Apply";
  reject forged hidden/noneditable controls. Honor only SubPageLink and SourceTableView,
  never guessed host Document Type. 0073 owns binding, 0044 owns record/filter semantics.
  Sales/purchase entry must address the visible FilteredTypeField when the original
  Type control is hidden; a default Item value must not mask a dropped agent input.
- P1: modal pages retain actions and editable lookup rows; explicit OK chooses a row.
  CurrPage.Close terminates through the declared lifecycle. Scope exit/navigation and
  explicit Close need separate BC qualification; never copy the reference's
  unconditional OnQueryClosePage bypass or default dialog answers.
- P1: request options validate immediately, re-read dependent values and expose
  lookup/assist/drilldown; reject an option name used as a dataitem filter (0063).
  Lists without CardPageId use the enabled Return-shortcut action to open a record;
  related read-only values expose authorized Brick/DropDown previews and card links.
- P1: load visible Role Center/FactBox parts on demand with shared explicit operations
  for agents; collapsed panes do no work. Suggestions use declared relation branches,
  server filters and bounded windows; do not sort or invent values on the client.
- Browser samples: row-key-qualified DOM IDs; discard stale morph listeners;
  preserve cursor/QuickEntry/Tab/arrow/F2/Escape behavior and invalid entered text.
  Keep presentation errors separate from AL TestPage diagnostic framing without
  losing error codes, command receipts or rollback. Agent parity is semantic, not TTY.
- Sources in refreshed `~/Git/openerp/`: `openerp/web/client/{page_model,protocol,request_page}.py`,
  `openerp/web/static/bc.js`, `test/openerp/runtime/test_client_{journal_after_posting,
  row_globals_after_validate,part_link_only,modal_page_actions,lines_become_editable}.py`,
  `scripts/analysis/client_feature_audit.py`, predecessor 1915–1930/1938/1940/1946/
  1954/1956/1958/1962/1971/1982–1985. Inventory source-declared capabilities against
  the shared model, then execute original CMD/MCP/web/SQL cases; static flags alone
  are not proof. These are queued agiru contracts, not imported pass claims.

## Typed variable bindings

- Generated control getters read the original declared scalar storage through
  `ReadPageVariable`, sharing the record-field transport. Decimal scale, Int64 digits,
  Unicode, temporal flags and declared Option/Enum members remain exact. Domains identify
  the object kind, ID and control; display text is not a type oracle. Unsupported storage
  refuses explicitly. Computed-expression transport remains unqualified.
- Generated variable setters check Evaluate and refuse failed conversion as TestValidation
  before OnValidate. The shared Integer reader rejects destination-width overflow, ERANGE
  and embedded-NUL prefixes without changing storage. Untyped Option/Duration share it;
  declared Option/Enum numeric ordinals cannot accept partially parsed digits.
- `make gate GATE=OptionGate JOBS=2`: 75 checks; GenPageGate: 58 checks, zero red.
  Generated navigation: 270 checks, dispatcher: 105 and source: 15, zero red;
  29 execution defects and one control-name compile refusal reject. Three new integer
  defects independently remove width, overflow and complete-input checks; another
  discards the generated setter result. Five affected compiled units pass clang-tidy.
- Native modal TestValidation inputs roll back their child SQL boundary, persist a failed
  child-command receipt and retain the caller/page for explicit correction. Fatal/action
  errors still unwind the caller; input receipts are not root commits. The HTTP proof below
  covers Integer/Decimal/BigInteger/indexed Integer/Option refusal, identical replay,
  unchanged original values, validation exactly once and independently visible SQL effects.
  BC inline invalid-text retention/discard UI, all conversion types, discarded AL Evaluate
  value-context semantics and regenerated original-production setters remain unqualified.
- Sources: `include/BuiltinsWritten.h`, `include/runtime/{PageVariableValue,PageValue,Page,PageSession}.h`,
  `src/rt/PageValue.cpp`, `src/gen/{PageWriter,RuntimeSurface}.cpp` and
  `test/gate/PageValueGate.cpp`. PageValue has 43 checks, zero red; affected scalar/generator
  units pass targeted clang-tidy. Generated navigation has 220 checks, zero red;
  the Card's six direct variable bindings now have exact values rather than scalar gaps.
  This is not original Customer, full generated-tree or source-counted UT acceptance.
- Reference: developer `properties/properties/devenv-sourceexpr-property.md` at
  `f928288ee840334be73142e5fc0202c0e19b246d`; BCApps `CustomerCard.Page.al`
  (LastPaymentAmount) and `SalesOrderStatistics.Page.al` (indexed totals) at
  `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`. The same developer revision supplies
  `methods-auto/system/system-evaluate-method.md`,
  `methods-auto/{integer/integer,biginteger/biginteger}-data-type.md`
  and `triggers-auto/pagefield/devenv-onvalidate-pagefield-trigger.md`. Predecessor
  WIs 1799/1894 retain Decimal/array lessons, 1198 typed writeback and 1946 correctable
  field errors; 1084's wrong-typed text fallback is rejected. No implementation copied.
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
  The native host now uses the transport below; original Customer opens its modal,
  inserts the chosen template and passes ordinary-field persistence/independent reopen.
  Non-modal query-close behaviour remains unqualified.
- Developer `methods-auto/page/page-{runmodal-,getrecord,lookupmode}-method.md` and
  `triggers-auto/page/devenv-onqueryclosepage-page-trigger.md` at
  `f928288ee840334be73142e5fc0202c0e19b246d`.
  Predecessor findings: `openerp/board/1705_lookupmode-bestimmt-schliessaktion.md` and
  `1231_onqueryclosepage-wurde-nie-gefeuert-currpage-lookupmode-war.md`; no Python
  threading or implicit modal commits adopted.

## Native modal transport

- Profile 4 exposes only the active modal through explicit nonce, revision and command
  identities. External CMD/MCP and htmx share the same generated controls, exact values,
  bounded SQL windows and explicit OK/Cancel. No automatic selection or confirmation.
- Original AL page storage, caller stack and SQL lease remain on the bounded AL worker.
  HTTP workers consume immutable snapshots, never AL page pointers. PostgreSQL fences
  ownership, changed-payload replay and receipts; each HTTP operation reauthorizes access.
  Child receipts mean processed input, not a committed caller transaction.
- Authored generated-page HTTP proof: 38 fixture-host cases and 44 actual `agiru serve`
  cases for each of limits 40/7/80, including both TryFunction write policies: 170 pass,
  zero red. Independent SQL covers explicit selection/GetRecord, original-variable edits,
  nested modals/questions, query-close veto/error retries, cancellation/timeout and later
  caller rollback without undoing an earlier explicit Commit. Message replay preserves
  the exact original identities/text; Chromium samples use the same public Caddy endpoint.
- Delayed close errors now retain their exact child-command receipt address through
  working snapshots; authenticated GET polling reads the SQL receipt without executing AL.
  CMD/MCP/web follow that receipt, retain the original error and require explicit retry.
  Fresh reads retain the open modal and updated revision without replaying the failed input.
  Typed input failures also retain their exact child receipt, original values and open
  modal; CMD/MCP/Chromium explicitly correct all nine invalid cases. Tests reuse one root
  page per client rather than widening server context limits or callback deadlines.
  Fresh modal inputs renew the idle answer wait within the absolute context deadline;
  passive reads and identical replay do not. Independent SQL proves no premature commit.
  All twenty-two compiled defect controls reject at named assertions; `make page-host-test
  JOBS=2` exits zero and verifies input hashes. Normal matrices remain complete; each
  mutant executes its affected case and required predecessors, not unrelated timeouts.
  Close-trigger SQL delay uses fixture-only
  PostgreSQL triggers; it does not depend on the unimplemented AL Sleep method.
- `make client-test JOBS=2`: 44 cases and eight executable defects rejected;
  `make web-test JOBS=2`: 14 browser cases, three defects and actual Caddy asset delivery.
  Generated navigation 270, dispatcher 105 and source 15 checks pass; 29 execution
  defects and one compile refusal reject. Native configuration has 242 checks, zero red.
  Five units affected by the typed-input increment pass targeted clang-tidy without suppressions.
- Sources: `src/rt/PageModal.{h,cpp}`, `src/rt/{PageCommandHost,PageInteraction,PageHtml}.cpp`,
  `include/runtime/PageHtml.h`, `src/client/{profile,http,ascii,web}.mts`,
  `test/ui/{page-host.sh,page-host.mjs,browser-client.mjs}` and authored AL fixtures in
  `test/runtime/page-navigation/`. Reproduce with `make page-host-test JOBS=2`.
- Previous production regeneration uses an AL-file-hash-equal copy of BCApps
  `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`, verified System symbols 29.0.54011.55407
  (package `f59a4e4200af2b819670655302ce4ba4bfdd51e133cae6d4faf7896e5bba6b44`) and
  host runtime 18.0. `make transpile` exits 1: 21 unresolved extension anchors,
  5,683 refused properties and 625 untranslated in-scope object kinds remain counted.
  Native declarations select 233/234 tables (one approved licensing exclusion), with
  22 bound/211 unbound; all 35 codeunits are selected but 67 methods remain unbound.
  The resumed `make slice-check all JOBS=6` completes at `d31bb87`: 14,225 slice sources,
  exit zero, 1,526 seconds; 1,902 unlinked procedures remain diagnostic stand-ins.
  The later source-insertion repair also passes production slice-check/all: exit zero,
  14,225 sources, 2,593 seconds; the same 1,902 unlinked gaps remain counted.
  Its fresh native HTTP regression matrix retains all 154 cases and all 18 compiled
  refusal controls, zero red, with unchanged input hashes; focused navigation has 239
  checks and 25 execution controls. Original Customer workflow proof is recorded below.
  Generation/slice linking is not complete AL execution.
- Remaining: wider master-data workflows, computed control values, variable input
  validation/retry, automatic AL CurrPage.Close, dynamic Editable, progress/report callbacks,
  modal error-string quotas and expired-receipt cleanup. This is not source-BC collation,
  full generated-tree/UT execution, financial posting or production-scale acceptance.

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
- Read-only source inventory on SQL Server 2022 CU26 (`16.0.4265.3`): CRONUS and original
  Customer No./Name/Search Name use `Latin1_General_100_CS_AS`. All collated source columns:
  that profile 15,032; `Latin1_General_100_CI_AS` 2; `Latin1_General_BIN` 37;
  `Latin1_General_CI_AS_KS_WS` 16; `SQL_Latin1_General_CP1_CI_AS` 46;
  `SQL_Latin1_General_CP437_CS_AS` 1. A single guessed database-default mapping is insufficient.
  Source equality probes distinguish case/accents and `1`/`01`, but equate composed/
  decomposed accents and trailing spaces; native C text equality does not equate the latter.
  Source ordering for `01,1,10,2,a,A,e,é` differs from C. Preserve existing databases;
  qualify a UTF-8 clone, comparison/uniqueness/filter profile and actual indexed continuation.
  Reproduce source identity/counts with `SELECT name,collation_name FROM sys.databases
  WHERE name=DB_NAME()` and `SELECT collation_name,count(*) FROM sys.columns WHERE
  collation_name IS NOT NULL GROUP BY collation_name` in original CRONUS; inspect the
  Customer entries through `sys.tables t JOIN sys.columns c ON c.object_id=t.object_id`.
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

- Original Customer regression after the source-insertion repair: `make erp-client-test
  JOBS=2`, **eleven pass/zero failures**, none skipped/cancelled. Original New/template
  display and all seven existing cases remain green. CMD/MCP/Chromium each explicitly
  select the second template, receive original Card 21 and create one numbered customer;
  SQL confirms creator and five inherited posting/payment/currency fields. Name, Address,
  Country and Credit Limit persist and match an independent original List/Card reopen.
  Decimal storage scale remains exact. SQL counts and complete row-content fingerprints
  prove no insert/delete/update in Customer/G/L/Item/Value ledgers; the new customer has
  no ledger entries and zero displayed balance. All eight earlier cases remain present.
  The generic state repair and its focused proof belong to 0741. Earlier 8/11 failures
  are recoverable at `ac896f4`; no failing identity was removed.
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
  is not source-BC collation or complete Unicode qualification. Wider template cases,
  lookups, posting, full business families and source-counted AL acceptance remain due.
  Browser samples preserve the actual created/reopened values, but the current long
  flattened control/action presentation is not BC visual/layout parity. Unsupported
  controls remain visible; dynamic layout/visibility, FactBoxes and complete UI remain gaps.
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
