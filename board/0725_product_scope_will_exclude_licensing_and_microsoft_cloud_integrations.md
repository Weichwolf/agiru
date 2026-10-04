# 0725 — Product scope will exclude licensing and Microsoft cloud integrations

Status: open | Priority: P0 | Stage: scope baseline before G1 | Reviewed: 2026-10-04
Depends on: 0033 app/object identity; 0058 independent source manifest.

## Scope

- User decision: MIT agiru; no licensing, trials, commercial entitlements, paid tiers or feature locks. No O365/Microsoft 365 or other Microsoft cloud integrations.
- Preserve full core ERP, extension semantics, permissions, company/session isolation and generic HTTP/SMTP/file interfaces. Microsoft namespaces and Scope=Cloud alone are not exclusion criteria.
- Root LICENSE covers agiru-owned code; preserve required upstream/dependency notices. Inventory redistribution separately; no blanket relicensing.

## Evidence

- Canonical native exclusion: `bc-licensing:system-symbols/src/Tenant Database
  Tables/TenantLicenseState.Table.al`, ID 2000000189. Its original four fields and
  Evaluation/Trial/Paid/Suspended/Deleted/LockedOut states govern commercial periods;
  source inspection, not its namespace or a failing contract, establishes the reason.
  `system-symbols/` selects a separate source domain: the same BCApps relative path
  remains required. Parsing/independent raw inventory precede selection.
- Removed the unused native licence header, registration, generator identity/aliases
  and option mappings; no DB rows/schema removed and no Paid/success stub added.
  Full regeneration `/tmp/agiru-transpile.zB9cfB` retains 234 raw tables, selects 233,
  excludes one with its identity/reason, emits 18 qualification sources and retains
  all 80/2,314 UT. The 215 selected unbound tables and other native gaps remain red.
  Original-package qualifier `/tmp/agiru-native-bindings.WvH4lc`: 18/18 candidates
  compile without PCH; 221 typed/RecordRef/ownership checks green; wrong namespace
  and dropped-library controls reject. Raw audit still retains all 234 table outcomes.
  `make slice-check`: 14,225 sources, zero missing. GenScope: 304 checks; focused
  native/product/inventory tooling: 44 tests green. Final `make test JOBS=2`: 123
  cases/223 tooling green; targeted Apps/NativeSource analysis passes. Independent
  manifest comparison has zero added/removed UT identities
  (`/tmp/agiru-native-product-scope-ut-identities.json`). Final regeneration
  `/tmp/agiru-transpile.zsOsIb` preserves these populations. Full frozen
  `slice-check all test ut` run `20261003T233302Z-1193251` is active, not a pass.
- Mixed core caller remains unresolved: original BCApps
  `Layers/W1/BaseApp/Utilities/DataClassificationEvalData.Codeunit.al`
  unconditionally calls `ClassifyTenantLicenseState`, then other required privacy
  classifications. Generated commercial access now explicitly refuses. Preserve
  the privacy codeunit and User/access-control classifications; separate this
  bounded branch through declared selection semantics, never a runtime object-name
  special case or an always-success placeholder. This is not closure proof.
- UT recovery: two exact private implementations are now product-excluded in
  `scope.json`: System Application/App/{Tenant License State/src/TenantLicenseStateImpl,
  Azure AD Tenant/src/AzureADTenantImpl}.Codeunit.al (2301/3705). The former implements
  commercial trial/paid state; the latter calls Microsoft Entra/Graph and Power Platform.
  BCApps `bb7111877f`: incoming public wrappers and mixed User Settings,
  MyPlatformNotifications and CDSIntTableSubscriber remain selected. Their commercial/
  service dependency calls must refuse, not manufacture Paid/tenant results. No UT
  identity is excluded: generation retains 80 codeunits/2,314 methods. Complete mixed
  caller separation remains open; no licensing SDK repair is made.
- Main's new source-bound replay confirms query 774 Users in Plans is newly emitted: exactly two paths added, zero removed, all 80/2,314 UT identities retained (`build/native-source-paths28-{before,after}.txt`, `build/native-consumers.{vWqQvV,dE8PA4}/result.json`). Incoming source remains AzureADPlanImpl and PlanUserDetails; Essential/Premium plan fields are not an agiru feature gate. This reconfirms the prototype classification below; do not remove required User/authentication or the whole namespace. No commercial repair or new scope exclusion applied.
- Generic record context now also compiles IncomingDocumentApprovers' existing SaaS-only License Type filter; UserCard loses its FilterGroup errors but retains IsWSKeyAllowed. Page 192 still owns core incoming-document approver selection. Audit/separate BC commercial/cloud classification before product activation; do not exclude approvals/User/authentication wholesale or invent license values. BCApps src/Layers/W1/BaseApp/eServices/EDocument/IncomingDocumentApprovers.Page.al::HideExternalUsers; build/native-record-context-integration-20261002/artifacts/{generation,consumers}.json. No new scope exclusion or service implementation.
- Transpiler read root scope.json; prior gates/inventory read a stale src/gen copy. Consolidated to the production policy; old Graph-removal proof applied only to the unused copy, not generation. ProvisionInstalled's fabricated Paid/1980–2079 row remains removed, without a replacement stub.
- Microsoft.Integration.Graph mixes local ERP/API data helpers with real Microsoft-service bridges. Keep local helpers and current core dependency reachability; System.Integration.Graph remains excluded. Do not exclude the mixed namespace wholesale.
- Explicit reason/path exclusion now removes W1 O365RoleCenterNotifications (138073): thirteen license/trial/paid notification tests and two license-dependent SaaS sandbox tests. Incoming AL search found no calls to this W1 test object. Its fifteen methods remain in raw inventory; no UT-suffixed method is removed. The IN counterpart remains unclassified.
- Main's slice now retains its former cpp as a reasoned exclusion comment: 14,213→14,212 active sources. Fresh generation removes exactly its cpp/header, no other paths; raw UT remains 80/2,310, no exclusions/gains/losses. The earlier compiler image separately retains six appended app-identity sources and 14,218 active sources; these were not copied into main. Do not treat this retirement as general slice-filter permission. build/dictionary-integration-20261002/artifacts/{generation,main-generation,proof}.json.
- O365TrialBalance and invoice-total tests exercise core ERP. Namespace-less names, Microsoft prefixes and Scope=Cloud are not safe exclusion rules. Raw file/object/test identities and reasons survive generation filtering; namespace/app omissions remain separate gaps.
- UserSettingsImpl.UpdateCurrentUsersSettings mixes company switching with trial messages; MyPlatformNotifications mixes work-date notifications with evaluation state. Permission sets still reference Tenant License State - Read. Separate commercial responsibilities before retiring native licensing bridges; do not remove core settings/notifications or security modules.
- Actual-transpiler controls cover exact files, directory boundaries, malformed/duplicate/missing rules and retained raw tests. Previous implementations fail the controls; final counts, source identity and receipts belong in README. Full classification, selected dependency closure, runner partition and G1 remain open.

## Implementation

1. Classify licensing, O365 and Microsoft-service connectors from the raw inventory with explicit app/kind/ID/source identities and reasons; retain raw counts and all unresolved classifications. Resolve the remaining unmeasured GB report via 0058, not by suppressing it.
2. Extend the canonical reason/path policy only after source classification. Reconcile raw, selected and excluded runnable test identities in the mandatory runner verifier; inventory annotations alone do not prove executable scope. Never infer exclusion from an unresolved dependency.
3. Audit incoming references to each excluded object. Keep ERP responsibilities in their owning module; exclude only service-specific actions/subscribers/providers with visible capability diagnostics. No successful no-op or blanket System.Environment/Security/Email exclusion.
4. Retire unused native licensing/Graph/cloud bridges after the selected dependency closure is proved. Do not rewrite generated apps by hand, migrate populated schemas or discard notices.
5. Run the same raw UT inventory before/after; publish selected and excluded identities/reasons plus every status gain/loss. Rebuild the selected complete tree and run all selected tests through the normal runner.

## Acceptance

- Raw = selected + explicitly excluded, disjoint by full identity; every missing/refused/crashed selected test stays failed. Unexpected exclusions or missing reasons fail the gate.
- Negative controls detect policy-owner drift, service re-inclusion, fabricated Paid provisioning, unidentified namespace-less exclusions and accidental core/permission removal. Mixed local Graph/API helpers stay required.
- Core finance/sales/posting workflows and denied user/company operations remain functional without any license state or Microsoft service. DB execution requires a disposable sealed seed; compiler/static controls are not workflow proof.

## References

Code: scope.json, src/gen/{Scope,Apps,NativeSource,RuntimeSurface,CodeunitWriter}.cpp, src/tc/Main.cpp, scripts/{scope_inventory.py,transpile.sh,ut_manifest.py,ut_milestone.py,ut_results.py}, src/rt/{Storage,PlatformTables}.cpp, test/gate/GenScopeGate.cpp, test/tooling/toolchain.py::{NativeSourceCompilerGate,ProductSourceGate,SourceInventoryGate,ManifestGate}.
AL: BCApps src/Layers/W1/Tests/SMB/O365RoleCenterNotifications.Codeunit.al; src/Layers/W1/BaseApp/{Integration/Graph,System/Notifications/MyPlatformNotifications.Codeunit.al}; src/System Application/App/User Settings/src/UserSettingsImpl.Codeunit.al; src/System Application/{App,Test}/Tenant License State/; System symbol TenantLicenseState.Table.al. Platform: devenv-test-codeunits-and-test-methods.md, devenv-namespaces-overview.md; user intent: admin-extend-trial.md. Predecessor: board/990_mem-scope-whitelist.md, no Tenant/O365 finding; disabled-test subtraction is not adopted. Declaration references remain in 0034; permission semantics remain in 0062.
ID: all-history maximum 0719 and current open maximum 0724 checked on 2026-10-01.
