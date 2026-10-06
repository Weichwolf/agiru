# 0058 — Complete the source-counted UT milestone (G1)

Status: queued | Priority: P0
Depends on: 0013 effective profiles → 0044 live providers/record contracts;
0073 call lowering; required native/report dataset contracts from 0063.
Next: integrate each qualified repair and replay every configured UT identity;
close remaining source/seed/runner/safety gaps. Clients remain queued in 0720.

## Current evidence

- Latest completed frozen integration: `61344f7` / content `0642312188b5`:
  slice-check/all pass; C++ test is 147/150. Cursor/Filter fail on Integer rowversion;
  DynamicRecord fails on missing Resource Cost. A local record-order qualifier briefly
  overlapped its gate database; interference is possible, not proven as the cause.
  All 235 tooling tests pass. AL execution: 2171/2314 passed, 143 failed, eighty codeunits,
  zero incomplete/duplicate identities, 2473 seconds with six workers.
  Against `00c187c`'s 2135/2314: 36 observed gains, zero losses/added/missing identities,
  six changed failed errors. Gains span VAT-log pages, table-field metadata, workflow,
  document aggregates, payment search, assembly and Data Exch. to RapidStart.
  Seed identity remains null/unsealed: diagnostic comparison, not causal A/B or G1.
  Sorted `[codeunit_id, method, status, error]` TSV SHA-256:
  `9f94d7f42bbeee973653314d238a8f092bbd34854840f7a59c844daa75757382`.
  Previous result projections/remaining losses against `9dca232`: recovery `a554715`.
- Running frozen integration: `a554715` / content `56a0866bb13c`, same source/package
  pins and full configured population. Includes shared SQL GetBySystemId, original
  Integer virtual timestamp/SQL alias and native positive-key Field Get/Find/Next/Count.
  Wait for terminal target/AL receipts; do not infer gains from local gates.
  Serialized DynamicRecord/Storage/Temporary/Cursor/Filter pass 7493/71/95/255/129;
  native Field/FieldCatalogue pass 418/61 checks and 51 compiled reflection controls.
  Four Field attributes, authorization and full current-tree AL acceptance remain open.
  Current Parser/AlParserGate focused tidy passes after removing three findings without
  suppressions; the Parser gate retains previous checks and adds seventeen (159 total).
  TableWriter.cpp also passes focused tidy after six repairs; GenTable passes 105 checks
  with unchanged golden files and four rejected compiled mutants. GenTableGate still
  reports the existing BodyWriter.h adjacent-parameter finding. These are local receipts,
  not current-tree full build/lint or AL execution proof.
- Shared native Table Metadata.Get now passes 294 reflection checks, 115 generated
  checks and 43 compiled reflection controls plus the header control (0044).
  Typed/RecordRef agreement covers all 29 stored fields; missing results, projection
  errors, unchanged filters and writable temporary isolation are qualified.
  Navigation remains refused. TableMetadata.cpp/generated Runner focused tidy pass;
  Table.cpp has sixteen diagnostics and the expanded gate exposes an uninitialized-ID
  StoredImage diagnostic absent from the previous gate. No full tidy/UT gain claim.
- Shared SQL GetBySystemId passes 114 record checks and all seventeen compiled
  rowversion/SystemId controls (0044). Typed/RecordRef missing-result consumption,
  searched diagnostics, exact values, unchanged filters, provider errors and cursor
  positioning are qualified. Generated AL value/statement forms compile in all four
  source-binding contexts; these are not executed AL workflow or UT-gain receipts.
  RecordRef/RecordImage retain 147/28 checks; focused SQL gate/Table.cpp tidy retain
  three/sixteen findings. Temporary/virtual SystemId providers remain refused.
- Current tree: `make verify-check VERIFY_CHECKS='' JOBS=2` passes all 235 tooling
  tests. Discovery independently includes system-profile.sh and refuses each missing
  script/binary without shrinking totals. Attribute-census positives use legal Normal
  TryFunction methods; two malformed declarations remain counted and exit 1.
  Developer `f928288ee840`: attributes/devenv-{tryfunction,normal}-attribute.md.
  `test/tooling/toolchain.py` owns DiscoveryGate/TranspilerAttributeCensusGate;
  the frozen failures are not suppressed and still need current-tree integration.
- Integer projection's local repair passes Cursor/Filter 255/129 checks, shared
  record-order controls and focused tidy (0013/0044). It addresses the observed
  refusal path, but the full unchanged 2314-method replay must establish actual
  gains/losses. Original BC29 Field catalogue starts at positive field numbers;
  live native navigation now excludes timestamp zero without removing FieldRef(0).
  The aggregate UT must establish the expected count; gate success is not that replay.
- Latest AL failure concentrations: seventy-two Integer rowversion paths,
  five Page Metadata and two Table Metadata provider refusals, four Inventory Profile
  missing temporary rows and four WorkbookWriter.Create refusals. The Table Metadata
  cases are Incoming Doc. To Data Exch.UT's TestProcessWithDataExchSucceeds and
  TestProcessWithDataExchWithInvalidNamespaceFails. Fix shared contracts, not callers.
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
