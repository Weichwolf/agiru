# 0058 — Complete the source-counted UT milestone (G1)

Status: queued | Priority: P0
Depends on: 0013 effective profiles → 0044 live providers/record contracts;
0073 call lowering; required native/report dataset contracts from 0063.
Next: integrate each qualified repair and replay every configured UT identity;
close remaining source/seed/runner/safety gaps. Clients remain queued in 0720.

## Current evidence

- Latest frozen attempt: `9fd1e13` / content `774f78b70460`, slice-check exits 0;
  all/test/ut exit 2 after PageTableFieldGate.cpp's legacy five-field static assertion
  disagrees with the selected native Runtime-18 profile. Inputs remain unchanged.
  UT execution never starts: all 2314 configured methods are unexecuted across
  eighty incomplete codeunits. Repair native fixture assertions, not production
  profiles or test totals, before repeating `make verify-start JOBS=6
  VERIFY_TARGETS='slice-check all test ut'`. Field.Get/property repairs landed later
  and are not covered by this snapshot.
- Latest completed frozen replay: `9dca232` / content `fde18ee95496`, slice check
  and complete slice build pass. Local test remains red: the native-codeunit
  qualifier lacks a public-header include and three tooling checks have stale fixtures.
  AL execution: 2173/2314 passed, 141 failed, eighty codeunits, zero incomplete
  or duplicates (1719 seconds, six workers). Source manifest and result identities
  match; every identity/status/error matches the preceding `d3e767` replay.
  Canonical result hash: `b754b24c37ece93879414fdfc8061754d6238a5d1c2daf6d2c746334f6b4a358`.
  These inputs do not contain the later native Runtime-18 migration, API repairs
  or shared temporary-array storage fix. They are not current-tree G1 proof.
- Current tree: `make verify-check VERIFY_CHECKS='' JOBS=2` passes all 235 tooling
  tests. Discovery independently includes system-profile.sh and refuses each missing
  script/binary without shrinking totals. Attribute-census positives use legal Normal
  TryFunction methods; two malformed declarations remain counted and exit 1.
  Developer `f928288ee840`: attributes/devenv-{tryfunction,normal}-attribute.md.
  `test/tooling/toolchain.py` owns DiscoveryGate/TranspilerAttributeCensusGate;
  the frozen failures are not suppressed and still need current-tree integration.
- Last completed frozen AL replay: `d3e7671` / content `15e116850819`,
  2,173/2,314 passed, 141 failed, zero incomplete; eighty codeunits,
  BCApps `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`. Legacy null/unsealed seed:
  diagnostic repeatability only, not causal A/B or G1. Reproduce through `make ut`;
  compare method identities/statuses, not only totals.
- Frozen slice/build and local 146 cases / 233 tooling tests passed; UT exits
  nonzero (1,701 seconds, six workers). Against `ae71f8d`, all 2,314 identities,
  statuses and errors are unchanged: zero gains/losses/missing/added/duplicates.
  This replay covers the earlier reflection-name/profile-selection changes,
  not `b9f35e9` materialization/current User lookups, `5a4c3be` source-write refusal
  or subsequent rowversion allocation/SQL integration.
- Last measured AL failure concentrations: 48 Table Metadata provider refusals, thirteen
  incoming-document conversion assertions, four Inventory Profile missing temporary rows
  and four WorkbookWriter.Create refusals. Fix their shared contracts, not callers.
- Raw census: 36,883 AL files, 36,792 objects, 4,171 test codeunits,
  113,111 methods; fifteen approved exclusions leave 113,096 required.
  Zero unmeasured files; seven conditional assignments still refuse.
  Reproduce with `make census` against BCApps `d99152ee35f0` and the pinned policy.
  This is not the configured 2,314-method executable milestone.
- Native inventory retains 234 raw tables, 233 selected, eighteen bound and
  215 unbound; 35 selected native codeunits have 67 unimplemented methods.
  Other unactivated objects/properties and omitted app roots remain gaps.
- Preserve existing primitive fixes, generated bindings and negative controls.
  Package/declaration qualifiers are not full-app linking or business execution.
- BCApps `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17` retains byte-identical sources
  for all eighty configured UT codeunits / 2,314 methods versus `bb7111877f`.
  `make transpile` has regenerated against this revision and the verified native
  package; 215 unbound native tables, 67 native methods and 5,683 refused property
  declarations keep translation nonzero. Generation is not successful full-tree
  compilation or AL execution.
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
  required native report/dataset/request contracts; renderer work follows G1.
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
| P1 | Required TestPage, request-page and report dataset/native successor contracts; error-log RecordId/drilldown | `include/runtime/test/PageCore.h`, `src/rt/{TestPage,Report,Handlers}.cpp`, `src/gen/{PageWriter,CodeunitWriter,ReportLayoutsWriter}.cpp`; validation/filter/lifecycle first; no HTTP/client construction here |
| P1 | Remaining .NET/encoding/stream/XMLport/native Base64 signatures from actual failing callers; exact culture/UTF/calendar semantics | `src/net/`, `src/rt/dotnet/`, `src/gen/CodeunitWriter.cpp`; shared primitives, explicit unsupported signatures |
| P0 | User/company/app authorization, scoped encrypted storage, sequence identity/migration, provider-owned tenant facts | `include/runtime/{Table,RecordRef}.h`, `src/rt/{IsolatedStorage,NumberSequence,NumberSequenceStorage,Storage}.cpp`; context/schema identity before integration |
| P1 | Reliable Make statuses, complete compiler inputs, no-PCH/app dependency controls and decreasing lint/suppression debt | `Makefile`, `scripts/`, `cmake/`, `test/{gate,runtime,transpiler,tooling}/`; never replace UT with tooling proof |

## Scope audit (2026-10-05)

- O365/Microsoft 365 and other Microsoft cloud integrations are excluded product
  requirements. Retain core ERP, contact CRM, permissions, local APIs and generic
  HTTP/SMTP/SFTP/file/Excel-workbook functionality. An O365 name is not sufficient:
  `Invoicing/O365SalesCancelInvoice.Codeunit.al` uses generic document email.
- Compared with the refreshed implementation policy, agiru additionally admits
  `Microsoft.Integration.{Dataverse,D365Sales,SyncEngine}`,
  `System.Azure.Identity`, `System.Privacy` and `System.Telemetry`;
  it no longer excludes `Microsoft.CRM.Outlook` or the namespace-less Graph area.
  These are technical reachability differences, not approved cloud requirements.
- Concrete wrong admission: W1 `CRM/Outlook/O365GraphAuthentication.Codeunit.al`
  (7108) requests Graph/O365 tokens; `O365BidirectionalSync.Codeunit.al`
  (7106) calls graph.microsoft.com. Bound their service source exclusions and
  incoming dependencies explicitly; do not exclude local contact CRM wholesale.
- Audit Exchange/Outlook, Graph/OneDrive/SharePoint, Microsoft 365 email providers,
  Teams/Excel-online/Power BI and Entra/cloud-specific branches from source before
  changing `scope.json.product_exclude`. Preserve generic counterparts and mixed
  settings/privacy/work-date/notification callers. No successful license/cloud stubs.
- Existing four exact product rules exclude two private service/licensing
  implementations, native TenantLicenseState and the fifteen-method notification
  test object. Resolve mixed public callers rather than inventing Paid/tenant facts.
- Prove raw = selected + explicitly excluded, with every identity and reason;
  compare source-counted UT before/after. Reachability/namespace omissions remain gaps.
  Predecessor 760/1134 rejects dependency-touch and no-op-based test exclusions.
  This review did not change the executable policy or any test denominator.

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
  Only then release [0720](0720_cli_and_web_will_execute_the_same_erp_operations.md).

## Consolidation

Absorbs 0004, 0006, 0012, 0033, 0034, 0035, 0038, 0039, 0043,
0055, 0057, 0059, 0061, 0062, 0064, 0065, 0066, 0589, 0718, 0719,
0722, 0723, 0725. Focused owners 0013/0044/0073/0063 remain separate.
Previous detailed receipts, acceptance matrices and transitive absorbed-ID mappings:
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
Consolidation retires duplicate plans, not implementation, tests or requirements.
