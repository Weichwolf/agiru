# 0058 — Complete the source-counted UT milestone (G1)

Status: queued | Priority: P1
Depends on: 0013 effective profiles → 0044 live providers/record contracts;
0073 call lowering; required native/report dataset contracts from 0063.
Next: integrate each qualified repair and replay every configured UT identity;
close remaining source/seed/runner/safety gaps. Client-first delivery and documented
business workflows are active in 0720; full G1 acceptance does not block their implementation.

## Current evidence

- Latest completed integration: `c79dd1bfb99d43b7aedaa8805d9f3bf598dadfc1` /
  content `0922a47ce9a41720bdec522b433bc0c481c4d19d40b32de5547a269ca20e5ba7`:
  slice-check/all pass; C++/specialist test is 159/159, zero red.
  AL: 2221/2314 passed, 93 failed, eighty codeunits, zero incomplete/duplicate
  identities; six workers, 1112 seconds. Target exits: 0/0/0/2; G1 remains open.
  Manifest and executed identity-set digest match:
  `86ced2a881afad70c781f23d54fc9a263ff133581736de8b04e830cf54e70827`.
  Compared with `a107d129`'s 2219/2314: two gains, zero losses or missing identities.
  Gains: 132543.DataExhangeDefinitionImportInsertsRuleFromNextTransformationRuleField
  and 133771.RefundSharingPaymentDocShownOnRemittanceAdviceEntries.
  BCApps/System pins match the previous run; frozen inputs remain unchanged during verification.
  Seed identity is null/unsealed: diagnostic comparison, not causal A/B or G1.
  Result digest from `jq -sc 'sort_by(.codeunit_id,.method)|map({codeunit_id,method,status,error})'
  followed by `sha256sum`: `d325737b7bfbdc879ac99cd0a479112342c55ad46ae985d0eea7be068b1ac3f0`.
- This completed snapshot includes native Table/Page Metadata navigation,
  source-owned page IDs, RecordRef.Get consumption/diagnostics, scalar catalogue
  CalcFields (`b221e0d`), Unicode caption fallback (`48fcd03`) and shared positions
  (`65d3ade`), Evaluate dispatch (`6f8c9ce`), XML diagnostics (`4dda6f5`), captured
  loop bounds (`30293c8`), ordinal Variant text (`9985dc7`) and the RowVersion gate
  repair (`0cf1488`), report-ordinal, XMLport field-validation and page-dispatch/factory
  increments. It excludes semantic HTML, Node clients and the HTTP prototype;
  do not label it current-HEAD acceptance.
  Runtime contract evidence belongs in 0013/0044/0073; superseded results are
  recoverable at `c2df724`.
- Latest AL failure concentrations: thirteen incoming-document conversion failures,
  seven Nothing-to-handle paths,
  four Inventory Profile missing temporary rows and four WorkbookWriter.Create refusals.
  Three EntityText.ReadPermission refusals remain; Page Metadata's canonical views
  and DataCaptionFields still refuse rather than project defaults.
  No former Field/Table Metadata storage refusal remains in this measured population.
  Keep provider write guards: catalogue calculations must not provision SQL copies.
  Correlated native predicates, invalid-content diagnostics and full providers remain gaps.
- Raw census, 2026-10-08: 36,883 AL files, 36,792 objects, 4,171 test codeunits,
  113,111 methods. The refreshed scope explicitly excludes 202 test codeunits /
  3,744 methods, leaving 109,367 product-required methods. Configured selection:
  1,194 test codeunits / 38,420 methods; 70,947 required methods remain outside
  selection and visible as coverage gaps, not additional approved exclusions.
  Zero unmeasured files; seven conditional assignments still refuse.
  Reproduce with `make census` against BCApps `d99152ee35f0` and the pinned policy.
  This is inventory, not an executable UT result.
- Bound `Layers/W1/BaseApp/Office365Credentials.Page.al` (page 1312, Office 365
  Credentials) to the approved `microsoft-cloud` exclusion: the original source only
  collects Office 365 credentials to install Outlook Business Inbox add-ins. It is not
  generic agiru authentication; keep User Card, User Details and its mixed test library,
  SecretText and Cryptography Management. Source revision: `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`.
  Raw census remains 36,792 objects with source SHA-256
  `1868aa2837730b4e4b168401b8eab1f0cb1cd87b2144c423c12ac34d62e9e6c5`;
  explicit exclusions become 205 and selection 10,506. Every test-method population
  above is unchanged. The same three sources/seven conditional assignments still
  refuse inventory; `make census` remains nonzero, not a hidden green claim.
  `make verify-check VERIFY_CHECKS=SourceInventoryGate`: 20 tests pass, including
  exact-path/core-neighbor/raw-identity controls and reintroduction of the bounded page.
  `ProductSourceGate`: seven actual-generator selection/refusal controls pass.
  The required constructor/field/TestPage/overload emission repairs remain 0720 work;
  do not remove mixed price or permission codeunits to obtain a build. No direct donor
  board finding names this page. Native production regeneration remains pending.
- Native inventory retains 234 raw tables, 233 selected, eighteen bound and
  215 unbound; 35 selected native codeunits have 67 unimplemented methods.
  Other unactivated objects/properties and omitted app roots remain gaps.
- Preserve existing primitive fixes, generated bindings and negative controls.
  Package/declaration qualifiers are not full-app linking or business execution.
- BCApps `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17` retains byte-identical sources
  for all eighty configured UT codeunits / 2,314 methods versus `bb7111877f`.
  `make transpile` after `0b31295` retains 77 selected UT codeunits / 2,298 methods;
  these are parser counts, not executed results. The verified native package has
  22 bound / 211 unbound tables and 67 unbound methods; 122 refused properties and
  21 unresolved extension anchors keep translation at exit one. All 306,303 property
  declarations are classified; zero silent drops. Generation is not full-tree compilation.
- Build-input selection now uses `scripts/build_sources.py` and
  `cmake/GeneratedSources.cmake` for slice, full apps and native libraries. The generator
  publishes current CPP identities and policy/app-root snapshots even on refusal; the
  transpile wrapper retains original AL identities across successful sweeps. Required
  missing, unverified or stale inputs refuse; handwritten gates/transpiler remain buildable.
  Source selection is not a successful generation or full-app compilation claim.
  After required Privacy activation, `make slice-check`: 14,225 raw / 13,594 selected /
  99 approved product exclusions / 532 selection omissions, zero missing/errors; every
  raw slice identity remains unchanged. All configured apps plus platform: 14,655 raw /
  13,996 selected / 107 product exclusions / 552 omissions, zero errors.
  Native declarations follow bounded product rules, not the
  configured BCApps namespace reachability filter, matching `gen/NativeSource.cpp`.
  Original UTF-8 BOMs previously bypassed namespace selection in eleven slice CPPs;
  `tc/Main.cpp` now handles the BOM. Examples: BCApps `d99152ee35f0` paths
  `Layers/W1/BaseApp/{CRM/Outlook/ContactSyncQueueDialog.Page.al,System/Telemetry/TelemetryManagement.Codeunit.al}`.
  Actual-generator refusal and compiled CMake controls cover BOM bypass, stale/missing
  required inputs, exact exclusions, removed origins, separate omissions and native domain
  selection. Reproduce with `make verify-check VERIFY_CHECKS=BuildSourcesGate`;
  `ProductSourceGate` and `NativeSourceCompilerGate` cover actual generated manifests.
  Native integration exposed selected `CustomerConsentTests` requesting `Accept` from
  untyped `TestPage<>`: required page 1820 was omitted with `System.Privacy`.
  `scope.json` now activates genuine privacy/consent declarations; explicit product
  exclusions are unchanged. Generated handlers bind typed page 1820, its declared Accept
  and Cancel actions, not an invented UnknownPage action or a dropped test. Reference:
  BCApps `Layers/W1/BaseApp/{CustConsentConfirmation.Page,CustomerConsentMgt.Codeunit}.al`
  and `Layers/W1/Tests/Misc/CustomerConsentTests.Codeunit.al` at `d99152ee35f0`;
  developer `methods-auto/testaction/testaction-invoke-method.md` at `f928288ee840`;
  predecessor `openerp/board/1232_deklarierte-aktion-ok-cancel-wurde-vom-eingebauten-schluss.md`
  requires declared actions to outrank built-in closing. Inventory/ProductSource/BuildSources
  gates pass 19/7/10 cases. Regeneration retains 77/2,298 parsed UT identities and classifies
  307,067 properties with zero silent drops. Original-page execution and full native
  compilation remain due; generation still exits one for the known refusals below.
  Current regeneration still refuses 122 properties, 67 native methods and 21 anchors;
  77 UT codeunits / 2,298 methods remain parser inventory, not execution.
  Full tooling run: `make verify-check VERIFY_CHECKS=` passes 279 tests with the explicit
  container gate DSN and pinned source/docs. `make tc` and full-app CMake configuration
  pass; focused `lint-one UNIT=src/tc/Main.cpp` reports zero failures. These are not
  current full native build, aggregate tidy or AL execution results.
- Last aggregate tidy measurement: 388 unique diagnostics, ninety of 267 units
  analysed, all ninety failed. Subsequent shared-header repairs make focused
  PlatformSourceGate analysis pass; the aggregate was not rerun and remains unproven.
  Preserve zero tolerated diagnostics and suppression limits; no baseline increase.

## Focused prerequisites

- [0013](0013_system_fields_and_schema_keys_will_obey_their_declared_contracts.md):
  complete source/host-selected fields, reflection and schema/key metadata.
- [0044](0044_record_operations_will_share_one_correct_sql_and_temporary_contract.md):
  live catalogues and SQL/temporary record, filter, cursor and aggregate contracts.
- [0073](0073_generated_expressions_will_preserve_al_types_and_evaluation_effects.md):
  declaration-owned calls, var/value context and expression lowering.
- [0063](0063_reports_will_execute_datasets_and_render_declared_layouts.md):
  required native report/dataset/request contracts; renderer work follows concrete
  client/report prerequisites, not complete G1 acceptance.
- Qualified prerequisites do not require all future work of an owning WI;
  activation still needs this WI's unchanged-population comparison.

## Remaining G1 contracts, ordered by prerequisite

| Priority | Contract / next implementation | Files / prerequisite |
|---|---|---|
| P0 | Scope partition; sealed seed; mandatory independent manifest and missing/refused/crashed accounting | `scope.json`, `scripts/{scope_inventory,ut_manifest,ut_milestone,ut_results,seed_demo}.py`, `src/rt/RunnerDatabase.cpp`; source/package identity before activation |
| P0 | Session-private state; explicit record/FieldRef/KeyRef ownership; clean transaction leases | `src/rt/{Session,SingleInstance,Events,Scopes}.cpp`, `include/runtime/{Table,RecordRef,Session}.h`; ownership before boundaries |
| P0 | Durable Commit; distinct TryFunction/Codeunit.Run/asserterror and runner isolation; observed-version writes/LockTable | `src/rt/{Transaction,Scopes,Storage,Selection,TestRunner}.cpp`; schema versions and ownership |
| P0 | XML DTD/resolver safety; JSON parent/Root/Path/Replace; exact Decimal core and conversion boundaries | `src/net/{XmlReader,JsonEngine,Json,DotNetJson,Decimal}.cpp`; parser policy before external input, Decimal before exact codecs |
| P1 | App/namespace/dependency identity; extension ownership, Moved/obsolete storage; one declaration-owned overload/var/value-context binder | `src/gen/{Names,NativeSource,BodyWriter,TableWriter}.cpp`, `src/tc/Main.cpp`; identities before lowering/link |
| P1 | Complete selected native table/report/enum/interface/method binding and full app libraries; no slice refusal substitutes | `src/gen/{NativeSource,NativeManifest,TableWriter,CodeunitWriter}.cpp`, `scripts/transpile.sh`; original package provenance, then consumers and unchanged-population replay |
| P1 | SQL/temporary Record parity, dynamic cursors, mixed sort/Next, full-prefix keys, filters/GUIDs, FlowFields and query ON/WHERE/HAVING | `src/rt/{Table,Temporary,Navigate,Selection,Filter,Query}.cpp`; identity/profile → shared record/filter → aggregate/query |
| P1 | Validation/xRec/relation/event order, isolated subscribers and Boolean failure-to-false versus statement errors | `src/rt/{Table,Events,Scopes}.cpp`, `src/gen/BodyWriter.cpp`; ownership and transaction boundaries |
| P1 | Required TestPage, request-page and report dataset/native successor contracts; error-log RecordId/drilldown | `include/runtime/PageCore.h`, `src/rt/{TestPage,Report,Handlers}.cpp`, `src/gen/{PageWriter,CodeunitWriter,ReportLayoutsWriter}.cpp`; validation/filter/lifecycle first; no HTTP/client construction here |
| P1 | Remaining .NET/encoding/stream/XMLport/native Base64 signatures from actual failing callers; exact culture/UTF/calendar semantics | `src/net/`, `src/rt/dotnet/`, `src/gen/CodeunitWriter.cpp`; shared primitives, explicit unsupported signatures |
| P0 | User/company/app authorization, scoped encrypted storage, sequence identity/migration, provider-owned tenant facts | `include/runtime/{Table,RecordRef}.h`, `src/rt/{IsolatedStorage,NumberSequence,NumberSequenceStorage,Storage}.cpp`; context/schema identity before integration |
| P1 | Reliable Make statuses, complete compiler inputs, no-PCH/app dependency controls and decreasing lint/suppression debt | `Makefile`, `scripts/`, `cmake/`, `test/{gate,runtime,transpiler,tooling}/`; never replace UT with tooling proof |

## Scope alignment (2026-10-08)

- O365/Microsoft 365 and other Microsoft cloud integrations are excluded product
  requirements. Retain core ERP, contact CRM, permissions, local APIs and generic
  HTTP/SMTP/SFTP/file/Excel-workbook functionality. An O365 name is not sufficient:
  `Invoicing/O365SalesCancelInvoice.Codeunit.al` uses generic document email.
- Refreshed `~/Git/openerp/` archive SHA256
  `bdd25abe5cff00db252130d247d80ed5d733a30d18d3167b83aad7e2ed7324ce`;
  its only `*scope.json`, `scripts/transpiler/scope.json`, has SHA256
  `7c03ea285e63bd18e2a30649038f43151dca810b450df3c47b7d5f2609fc7776`.
  Namespace arrays and test-area arrays now match; remove former Dataverse/D365Sales/
  SyncEngine, Azure.Identity, Privacy and Telemetry re-inclusions, and restore
  Outlook/Graph carve-outs. Preserve implemented code and generic ERP requirements.
- Resolve the reference's 24 exact test names, O365 prefix and plan suffix into
  bounded `scope.json.product_exclude` source paths, never Python identifier rules
  in C++. At BCApps `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`, the complete
  excluded test-source sets agree: 109 W1 files (72 O365, thirteen plan-suffix,
  24 named) and 93 localized overrides. Keep existing three private/native product
  exclusions. Required permission tests 139400/134612/139004/132903 remain selected.
  Whole excluded suites do not remove generic charts, posting, users or reporting.
- `src/tc/Main.cpp` applies area exclusions to Test codeunits only; libraries and
  ordinary tables remain available. Namespace admission cannot bypass test-area
  exclusion: seven BCPT samples exposed that defect in the independent comparison.
  `scripts/scope_inventory.py` reports raw,
  selected, explicitly excluded and omitted-required populations independently.
  Namespace/area omission is not proof that every omitted object is a cloud service.
- UT raw population remains eighty / 2,314. Canonical selection is 77 / 2,298:
  138002 (two methods) and 139319 (one) have explicit suite exclusions;
  134627 (thirteen) is the reference's Graph-area omission, separately classified.
  `scripts/{ut_manifest,ut_milestone}.py` retain raw/excluded manifests, exact reasons,
  scope hash and population totals; empty selection refuses before execution.
  Previous 2,314-method results above are not renamed as new-scope passes.
- Concrete wrong admission: W1 `CRM/Outlook/O365GraphAuthentication.Codeunit.al`
  (7108) requests Graph/O365 tokens; `O365BidirectionalSync.Codeunit.al`
  (7106) calls graph.microsoft.com. Bound their service source exclusions and
  incoming dependencies explicitly; do not exclude local contact CRM wholesale.
- Audit Exchange/Outlook, Graph/OneDrive/SharePoint, Microsoft 365 email providers,
  Teams/Excel-online/Power BI and Entra/cloud-specific branches from source before
  changing `scope.json.product_exclude`. Preserve generic counterparts and mixed
  settings/privacy/work-date/notification callers. No successful license/cloud stubs.
  The bounded W1 BaseApp `Modules/System/PowerBI/` cloud implementation is now explicitly
  classified: 47 raw objects retained, no test-method loss. Page-part/call selection and
  its native refusal/effect qualifier are owned by 0720; generic charts remain required.
  Full raw census still refuses the existing three conditional sources/seven variants.
- OCR, payment providers and migration-to-BC implementations in mixed namespaces
  remain a source/dependency audit, not permission to drop Incoming Documents,
  generic payment/file exchange or RapidStart. Predecessor 1998/2000 proposes
  AL-level replacement after country composition; reject successful retirement stubs.
  Country conflicts need object/signature/body-hash proof before modifying sources.
- Prove raw = selected + explicitly excluded + visible omissions, with every
  identity and reason;
  compare source-counted UT before/after. Reachability/namespace omissions remain gaps.
  Predecessor 760/1134 rejects dependency-touch and no-op-based test exclusions.
  Before/after source digest and raw identities agree; the same seven conditional
  assignments still refuse in the same three files. `make census` remains exit 1,
  not a green full-tree qualification. Reproduce with the pinned BCApps revision.
- Verification: native `make tc JOBS=2` passes; `make lint-one
  UNIT=src/tc/Main.cpp JOBS=2` has zero diagnostics. `make verify-check
  VERIFY_CHECKS='ProductSourceGate ManifestGate MilestoneGate SourceInventoryGate'`
  passes 57 tests; unrestricted `make verify-check VERIFY_CHECKS=` passes 265, zero
  failures. Select the verified native build/BC source and dedicated gate DSN explicitly.
  Controls retain exact-source boundaries, libraries, both namespace forms, missing
  targets, wrong runner totals and empty-selection refusal with raw receipts.
  Independent configured test selection agrees with the reference: 1,194 codeunits /
  38,420 methods, zero mismatches. These are scope/tooling proofs, not an AL UT run.

## Acceptance

- Complete selected generated tree compiles/links; `agiru run-tests` passes every
  source-counted milestone method. Every missing/duplicate/refused/crashed/timed-out/
  skipped/unexecuted case stays reported; zero unexplained losses.
- Fresh disposable clones have a complete sealed source/seed identity; receipts pin
  Git/content/BCApps/System/compiler/image/work-date/runner/symbol identities.
  Preserve raw variants and resolve seven CLEANSCHEMA assignments against the actual
  build/localization matrix, not theoretical mutually exclusive populations.
- Build, C++ gates, generator fixtures and lint pass without raised baselines.
  New contracts have meaningful negative controls. G1 is not full-suite/G3 proof.
- Scope retirement neither removes core ERP nor bypasses denied permissions.
  [0720](0720_cli_and_web_will_execute_the_same_erp_operations.md) proceeds independently;
  its client-first goal does not relax these full G1 acceptance requirements.

## Consolidation

Absorbs 0004, 0006, 0012, 0033, 0034, 0035, 0038, 0039, 0043,
0055, 0057, 0059, 0061, 0062, 0064, 0065, 0066, 0589, 0718, 0719,
0722, 0723, 0725. Focused owners 0013/0044/0073/0063 remain separate.
Previous detailed receipts, acceptance matrices and transitive absorbed-ID mappings:
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
Consolidation retires duplicate plans, not implementation, tests or requirements.
