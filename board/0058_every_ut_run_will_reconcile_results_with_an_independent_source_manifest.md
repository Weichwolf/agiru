# 0058 — Complete the source-counted UT milestone (G1)

Status: queued | Priority: P1
Depends on: 0013 effective profiles → 0044 live providers/record contracts;
0073 call lowering; required native/report dataset contracts from 0063.
Next: integrate each qualified repair and replay every configured UT identity;
close remaining source/seed/runner/safety gaps. Client-first delivery and documented
business workflows are active in 0720; full G1 acceptance does not block their implementation.

## Current evidence

- Latest completed integration: `b2a8131` / content `3ab8c23d656f`:
  slice-check/all pass; C++/specialist test is 154/155, one red RowVersion disconnect
  observation. `0cf1488` repairs and qualifies that gate without changing production
  rowversion or connection semantics; its full integration replay is pending.
  AL: 2218/2314 passed, 96 failed, eighty codeunits, zero incomplete/duplicate
  identities; six workers, 1516 seconds. Target exits: 0/0/2/2; G1 remains open.
  Compared with `51831ed`: no gains, losses, added/missing identities or changed errors.
  The complete normalized result digest is unchanged.
  Original BCApps/System pins and all source/input hashes remain unchanged.
  Seed identity is null/unsealed: diagnostic comparison, not causal A/B or G1.
  Result digest from `jq -sc 'sort_by(.codeunit_id,.method)|map({codeunit_id,method,status,error})'
  followed by `sha256sum`: `f036244cb3acdb0e0a31a520bc773497b3a91cdc4f12efc64a87501262097ab1`.
- This completed snapshot includes native Table/Page Metadata navigation,
  source-owned page IDs, RecordRef.Get consumption/diagnostics, scalar catalogue
  CalcFields (`b221e0d`), Unicode caption fallback (`48fcd03`) and shared positions
  (`65d3ade`). It excludes Evaluate dispatch (`6f8c9ce`), XML diagnostics (`4dda6f5`),
  captured loop bounds (`30293c8`), ordinal Variant text (`9985dc7`) and the RowVersion
  gate repair (`0cf1488`); replay these together. Runtime contract evidence belongs
  in 0013/0044/0073; superseded results are recoverable at `0cf1488`.
- Latest AL failure concentrations: thirteen incoming-document conversion failures,
  seven Nothing-to-handle paths,
  four Inventory Profile missing temporary rows and four WorkbookWriter.Create refusals.
  Three EntityText.ReadPermission refusals remain; Page Metadata's canonical views
  and DataCaptionFields still refuse rather than project defaults.
  No former Field/Table Metadata storage refusal remains in this measured population.
  Keep provider write guards: catalogue calculations must not provision SQL copies.
  Correlated native predicates, invalid-content diagnostics and full providers remain gaps.
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
