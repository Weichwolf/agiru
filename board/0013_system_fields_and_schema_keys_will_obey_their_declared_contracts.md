# 0013 — System fields and schema keys will obey their declared contracts

Status: open | Priority: P1 | Stage: UT → Clients | Reviewed: 2026-10-05
Depends on: 0033 declaring-app identity. Activation: 0034/0058 unchanged-population proof.

## Evidence

- App/native manifest readers and emitted `ModuleDef.minimumRuntime` now preserve
  the explicit AL runtime separately from application version. Omission stays empty;
  present empty/nonstring/duplicate JSON and empty XML declarations refuse.
  `make tc` passes; GenTableGate retains all cases and passes 85 checks;
  all eleven NativeReportSourceCompilerGate methods pass, including compiled
  ordinary/native declarations and a wrong-runtime compile control.
  Original System 29/Runtime 18 and BaseApp 30/no-runtime modules also compile
  against separate runtime/version assertions. Receipts:
  `/tmp/agiru-system-field-write.UYh59R/` and
  `/tmp/agiru-native-report-layouts.Le25iF/`. Both readers pass targeted lint;
  The complete original-package layout qualifier exits 0: both generation variants
  pass 562 checks each, retaining sixteen layouts and ownership/property controls.
  App/slice libraries and named assets remain qualified, not rendered/activated.
  gate/Main analysis remains red on inherited BodyWriter and existing Main
  complexity/size findings. Host selection, omitted-runtime default resolution,
  runtime-string compatibility and complete implicit-profile activation remain open.

- Original reflected writes are not a blanket readonly boundary. Ncl SHA256
  `277e35cbdfb87f17b979813e46fb73c2e84f5b04b85ed806c40367acf72b48b7`:
  `NavFieldRef.set_ALValue` RVA 33c00 reaches `NavRecord.SetFieldValue` RVA 4b89c;
  field index 0 reaches `SetRecordTimestamp` RVA 8ec8f and mutable buffer storage.
  Nonzero writes reach `StoreFieldValue` RVA 910e0; neither chain checks Editable.
  ValidateAsync RVA 296ce8 sets a value only when a new argument exists.
  Static IL receipts in `/tmp/agiru-system-field-write.UYh59R/original-*.il`,
  not executed original business/SQL or cross-version proof. Keep typed source
  assignment, reflected buffer changes, UI Editable and persisted platform values
  separate; do not invent a common FieldRef readonly ban from UI metadata.

- `meta/SystemFields.h` now owns base identities and reserved/implicit predicates
  without typed record helpers. RecordRef's declared index excludes timestamp 0;
  132 checks retain all previous cases and exercise an authored ten-field Runtime-18
  profile, primary-first order and by-number values. Restored range-only filtering
  fails four checks on the same population. `make reflection-metadata` passes
  229/2,917/132 checks; 23 compiled controls and the typed-header dependency control
  reject. `/tmp/agiru-implicit-field-index.xIGT1A/` and
  `/tmp/agiru-reflection-metadata.rrYA7W/` retain receipts. This does not generate
  the complete profile, calculate user lookups, activate providers or prove rowversion.
  Gate analysis has no own findings; 36 header findings remain. Full integration
  of this increment is pending; the live CDS replay predates it.

- Exact original native creation is now executed for all 234 tables (0044).
  Original Table Metadata has 33 effective fields: 23 declared plus timestamp 0,
  SystemId, four audit fields and four Runtime-18 user-name/full-name FlowFields;
  NCL assignment confirms virtual kind and audit applicability. agiru has 28,
  so timestamp/FlowField implementation and row values remain open, not an absent
  authority blocker. `/tmp/agiru-table-provider-authority.CyvRCA/native-profile.json`.
  Original logical field getters now execute twice for all 234 tables/2,340
  implicit fields, zero refused, identical bytes and exact independently inventoried
  IDs. `native-implicit-fields.json` retains hashes and bounds. Original
  `NavRecord.get_ALFieldCount` RVA 47562 subtracts all ten implicit fields: Table
  Metadata's AL count is 23, not its internal 33. Preserve primary-first declared
  indexing; `RecordRef.cpp::IndexedFields` must exclude timestamp 0 when added.
  Original Types XML binds the four read-only Text[50]/Text[80] FlowFields to
  User 2000000120 fields 2/3, filtered on User field 1 by audit GUID 2000000002/4.
  User lookup execution and SQL identifier resolution remain unproved; failed
  NavEnvironment probes are retained, not counted as passes. Raw `$systemId`,
  native byte lengths and classification getters are distinct from AL API contracts.
  Applicability is now executed across all seven original internal table kinds
  and both LinkedObject values: fourteen authored original Types-creation cases
  match twice. Timestamp/SystemId are always present; both audit families require
  `!IsLinked && (Normal || Temporary)`. Ignoring LinkedObject fails two cases;
  universally appending audit fields fails twelve. Original CLR enums prove
  compiler CDS=5 versus native Query=5: numeric casts are invalid. Exact emitter
  IL qualifies CDS→CRM; the shared C++ mapping and controls now pass (0044/README).
  `implicit-profile-boundaries.json` retains all hashes
  and failed attempts. Ordinary NCL creation still refuses without NavEnvironment;
  no ordinary business, SQL lookup or Runtime-17 equivalence is claimed.

- Field length authority now covers every original native field: 234 tables/4,477
  fields, zero refused. Original FieldRef.Length and virtual Field.Len both use
  NCL FieldDefinedLength, not raw declared capacity. The shared agiru primitive
  now reports Integer/Boolean 4 and GUID 16; all four real C++ caller cases match.
  Decimal is 12, BLOB 8, RecordID 448 and TableFilter 504,
  never `sizeof` host wrappers. Original native sources contain no DateFormula;
  a separately authored original-creation probe confirms 32. Receipt:
  `/tmp/agiru-table-provider-authority.CyvRCA/native-field-length-contract.json`.
  `field-length-implementation.json`: 98/305 gate checks pass, restored zero
  lengths fail 7/19; all 4,477 original lengths match, old zero control differs
  2,754 times. Source `FieldDef.length` stays declared-only. Full generated/local/AL
  replay and native profile/provider remain open (0044); lint remains red.

- Native virtual-buffer authority: original BC 29.0.54011.55407 Ncl SHA256
  `277e35cbdfb87f17b979813e46fb73c2e84f5b04b85ed806c40367acf72b48b7`:
  `VirtualDataProvider::.cctor` RVA addf3 sets VirtualTimeStamp to 1, not 0.
  `AddSystemFieldValues` RVA adcb0 writes it to buffer slot 0; audit fields use
  NavDateTime/NavGuid defaults only when NCLMetaTable.HasAuditFields is true.
  SystemId always receives the supplied argument: unkeyed creation passes
  NavGuid.Default, while the keyed overload passes its SystemId member.
  IL/hash receipts: `/tmp/agiru-record-order.FQ83WP/native-virtual-system-{fields,population}.il`;
  original extraction provenance: `/tmp/agiru-table-metadata-authority.gCa1Do`.
  Static creation-path evidence, not original BC execution,
  native AL omission defaults or cross-version equivalence
  with System 29.0.55365.0/Runtime 18. Do not apply SQL insertion/audit defaults
  or unconditional zero rowversions to computed providers without qualification.
  Table Metadata's actual keyed iterator is now traced and its SystemId core
  measured against the original constructors (0044); audit applicability,
  timestamp visibility and the complete native field profile remain open.
- Executed UT recovery (2026-10-03): the Integer sequence provider emitted only
  Number while the cursor selected all declared fields, including system fields.
  `Selection::SeriesColumns` now supplies the complete typed projection with the
  same SQL defaults as schema creation; unpersisted virtual rows have no insertion
  identity/audit stamp. Unexpected additional stored fields refuse rather than
  becoming invented data. CursorGate passes 230 checks, including actual SQL and
  removal of the identity projection as a failing control; FilterGate remains
  122/zero red. Public headers, declared fields and range policy are unchanged.
  Full 80/2,314 replay remains required; this does not close virtual write policy,
  audit applicability or Integer's existing range-cap/report-loop gaps.
- Actual UT bootstrap refuses all 80 codeunits before their 2,314 methods: imported
  All Profile.Description is varchar(250), while original System declares Text[2048].
  `Storage.cpp::EnsureColumns` now widens bounded Text/Code columns to generated
  declarations, preserving rows and never narrowing larger columns. StorageGate has
  63 checks, including the old-width rejection, retained values and repeated provisioning.
  No seed/master is changed; AL runs adapt disposable clones. Other type migrations
  remain unsupported by this bootstrap; full production migration is not proved.

- `Storage.cpp` generates keys and system fields, but writes lack observed-rowversion predicates. Field presence is not a uniqueness/audit/concurrency guarantee.
- Page/Table Metadata declarations now match System 28/29 (0034), but their corrected lengths/options/key must not reinterpret existing persisted snapshots. The old seeding loops are removed; live access/provisioning is guarded until provider and schema identity are qualified. Existing rows were not deleted or migrated. Temporary records retain the source contract; 0044 owns computed read-only authority.
- AllObj/AllObjWithCaption now retain their source sole pk and original field numbers (0034); this is declaration proof only. Retire the old invented name index and wrong caption/package columns through explicit populated-schema activation, not blind ProvisionInstalled. Scope/fieldgroups, system-field visibility and live read-only catalogue authority remain open; no SQL migration was run.
- The known native bridge exposes all five base implicit field numbers; AllObj/AllObjWithCaption/Feature Key metadata and typed reflection now include them. Source contracts check their types/member offsets; 828 checks pass and three source-contract controls reject. No SQL migration/audit-trigger/uniqueness proof. Runtime-18's four user-name/full-name FlowFields remain unimplemented; base metadata is not a complete version profile (0034).
- Integrated `src/gen/TableKeys.cpp::CompletePrimaryKey` completes ordinary/native defaults before extensions/system fields. Use the lowest declared field ID and exact AL name; unsuitable fields refuse, never select a later field. Table Metadata requires `ID`/field 1. `make table-keys`: 34 generator/61 native binding/14 generated temporary-operation checks; previous emitter fails four checks, wrong-name/clustering controls reject. Lower-ID extension fields/secondary keys cannot replace the base default. SQL enforcement/migration/full UT remain unproved; current receipts are in README.
- Existing shared key projection preserves declared flags, sums/order, IncludedFields and text. Duplicate clustered selections, invalid Boolean values and SqlIndex refuse. Integrated `Stored()` retains Removed Normal columns; reference restrictions and FlowFields/FlowFilters stay separate. Earlier controls are indexed in README. Current SQL verification is unavailable; no physical-clustering or production-activation claim.

## Implementation

1. Use one database-wide sequence for rowversion; include the returned version in RecordState and expose it to 0012 without widening every generated field wrapper.
2. Audit SystemId uniqueness, supplied-ID Insert overloads, created/modified stamps and database-wide monotonic SystemRowVersion. Assign versions in PostgreSQL so independent tiers cannot collide.
   Implement Runtime-18's read-only SystemCreatedBy/SystemModifiedBy user-name/full-name FlowFields for unlinked Normal/Temporary tables from the verified declaration profile. Preserve reserved IDs 2000000005–2000000008, Text[50]/Text[80], current User lookup and nonstored/read-only semantics; do not add them to Runtime-17 or infer virtual-table applicability from an absent TableType property.
   Retain one immutable runtime-version field profile, standard-layout member offsets
   and shared typed/reflected calculation primitives. Keep timestamp/implicit fields
   out of declared FieldCount/index loops. Verify actual FieldName → virtual Field
   lookup → FieldRef.Value callers, not expected-name literals (predecessor 1114).
   Resolve the host profile and omitted-runtime compiler default separately from
   the preserved source minimum runtime; never infer either from app version.
   Derive generator system-name recognition (`BodyWriter.cpp::IsSystemFieldName`)
   from the same profile instead of its separate six-name list.
   Extend the narrow `meta/SystemFields.h` identities into the complete versioned
   profile; do not reintroduce typed declaration dependencies or derive the reserved
   range from an array's first member. Header dependency controls reject the old
   `meta/Declare.h` consumer. Three no-PCH rounds measure standalone SystemFields
   474.6 ms versus Declare 1,825.6 ms; `/tmp/agiru-include-cost.veRnns/results.tsv`.
   Concurrent-host header measurements, not a production/build speedup claim.
   Qualify source-language assignment refusal and dynamic FieldRef buffer behaviour
   independently against original callers. Do not gate `FieldRef::SetValue` by
   Editable alone or change the internal `SetFieldText` used by Store/CalcField/SQL.
   Reuse the existing Lookup formula for current User names; prove changed/missing
   users, calculated values and the separately qualified typed/reflected write policies.
3. Activate the shared default/key projection only with 0034/0058 proof. Complete unsupported default eligibility from platform evidence; never select a later field silently. Implement SqlIndex as separate SQL fields, not SetCurrentKey fields; keep its refusal until supported. Enforce Unique, MaintainSqlIndex, IncludedFields, Enabled, AutoIncrement and SqlTimestamp restrictions. No SQL Server physical-clustering promise for PostgreSQL.
4. Return platform-owned fields from writes and preserve identity on Rename. Coordinate version checks with 0012 and company-qualified sequences with its company work.
5. Use typed generated metadata for constraints and migration comparisons; never infer a field number from display order.
   Retain obsolete Normal columns and their data through synchronization; exclude FlowFields/FlowFilters independently. Do not conflate Removed with declaring-app migration/Moved (0033).

## Acceptance

- Two connections writing different tables receive distinct increasing versions. Duplicate supplied SystemId fails, Rename retains it, audit stamps follow documented trigger order, and disabled/nonmaintained keys generate the intended DDL.
- Default controls cover reordered IDs, original case/spaces, unsuitable/unknown fields and extension keys; generated Get/Insert/duplicate behavior uses the base primary. Retain explicit-key/Clustered controls, including Posted Gen. Journal Line. SQLIndex fields may differ without changing logical uniqueness/navigation; unsupported forms never succeed silently.

## References

Integer projection: developer `ff5939a46e`, `devenv-{integer-virtual-table,virtual-tables,table-system-fields}.md`
(virtual rows are computed; audit values are blank before Insert); verified System
29 `src/Virtual Tables/Integer.Table.al`; `src/rt/{Selection,Storage}.cpp` share
`ColumnZero` through the private selection contract. Predecessor board 936 warns
that changing range/MaxIteration behaviour activates unrelated report defects;
this repair leaves those bounds unchanged. Receipts:
`/tmp/agiru-ut-recovery-integer-{cursor,filter-final,lint}.log`.

Code: `src/rt/Storage.cpp`, `src/gen/TableWriter.cpp`, `include/meta/TableDef.h`.

Base field declarations: `include/meta/SystemFields.h::SystemFieldNumbers`, `test/gate/PlatformSystemFieldsGate.cpp`, `test/transpiler/native-bindings.sh`; developer `ff5939a46e`, `devenv-table-system-fields.md` separates the five base fields from Runtime-18's additional Normal/Temporary FlowFields. Typed-member/metadata availability does not prove SQL audit generation or provider contents.

Runtime selection/write boundaries: developer `ff5939a46e`,
`devenv-{choosing-runtime,json-files,table-system-fields}.md`,
`properties/devenv-editable-property.md`,
`methods-auto/fieldref/fieldref-{value,validate}-method.md`;
BCApps `bb7111877f`, `Layers/W1/BaseApp/app.json`; original System
`NavxManifest.xml` Runtime 18.0. Predecessor 1150 requires local/global/var
resolution before system-name recognition; 867/967 separate UI and AL writes.

Implicit profile: developer `ff5939a46e`, `methods-auto/recordref/recordref-{fieldcount,fieldindex}-method.md`;
BCApps `bb7111877f`, `Layers/W1/BaseApp/Modules/System/ApplicationArea/ApplicationAreaMgmt.Codeunit.al`
and `Apps/W1/{HybridSL/app/src/Migration/History/SLPopulateHistTables.Codeunit.al,ExpenseAgent/test/src/ActivityLog/ExpenseActivityLogTest.Codeunit.al}`.
Original creation/getter and declaration IL receipts: `native-implicit-fields.json` above.

Default keys: `src/gen/TableKeys.cpp::CompletePrimaryKey`, `src/gen/TableWriter.cpp::{NativeKeyAssertions,NativeFieldAssertion}`, `src/tc/Main.cpp::IndexTables`, `test/gate/{GenKeyGate,GenNativeBindingGate}.cpp`, `test/transpiler/table-keys.sh`, `test/transpiler/table-keys/`. Platform: `analyzers/appsourcecop-as{0010,0118,0123}.md`, `diagnostics/diagnostic-al{256,450,464,527}.md`, `methods-auto/recordref/recordref-keyindex-method.md`. BCApps current main: UserSettings, CreatePickParameters, WordTemplatesTestTable4; pinned System TableMetadata. AL0325's explanatory whitelist contradicts actual Code primary keys; do not adopt it. Focused predecessor search found no default-primary finding; 980/1244's emitted-contract guidance applies.

Platform: `devenv-table-keys.md`, `properties/devenv-{clustered,enabled,maintainsiftindex,maintainsqlindex,unique,sumindexfields,includedfields,sqlindex,obsoletestate}-property.md`, system-field/Insert guarantees and `analyzers/appsourcecop-as{0002,0016}.md`. AL: BCApps main `a9ea4d84534cebba852c44bf0f841c2ea149de4e`, `Layers/W1/BaseApp/Finance/GeneralLedger/{Journal/PostedGenJournalLine,Ledger/GLEntry}.Table.al`, ResourceCost and CopilotSettings; pinned UserPersonalization/FeatureKey. User intent: `business-central/database-missing-indexes.md`. Predecessor 980/1244 show property loss and unreliable name-only audits; their correctness-neutral clustering label is not a metadata contract. Fixtures: `test/gate/{GenKeyGate,UserPersonalizationGate}.cpp`; preserved `artifacts/{old-generator-control-final.log,wrong-key-clang.log,wrong-key-gcc.log,transpile-first-control.log,transpile.log}`. Earlier obsolete-column control remains `build/user-personalization-proof/stored-negative-runtime.log`; Moved/merge identity is 0033.

Property scope: `autoincrement`, `sqldatatype`, `sqltimestamp`.
