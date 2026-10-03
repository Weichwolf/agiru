# 0013 — System fields and schema keys will obey their declared contracts

Status: open | Priority: P1 | Stage: UT → Clients | Reviewed: 2026-10-03
Depends on: 0033 declaring-app identity. Activation: 0034/0058 unchanged-population proof.

## Evidence

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
   Implement Runtime-18's read-only SystemCreatedBy/SystemModifiedBy user-name/full-name FlowFields for Normal/Temporary tables from the verified declaration profile. Preserve reserved IDs 2000000005–2000000008, Text[50]/Text[80], current User lookup and nonstored/read-only semantics; do not add them to Runtime-17 or infer virtual-table applicability from an absent TableType property.
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

Base field declarations: `include/meta/Declare.h::SystemFieldNumbers`, `test/gate/PlatformSystemFieldsGate.cpp`, `test/transpiler/native-bindings.sh`; developer `ff5939a46e`, `devenv-table-system-fields.md` separates the five base fields from Runtime-18's additional Normal/Temporary FlowFields. Typed-member/metadata availability does not prove SQL audit generation or provider contents.

Default keys: `src/gen/{TableKeys,TableWriter,TableDefinitions,NativeKeyAssertions}.cpp`, `src/tc/Main.cpp::IndexTables`, `test/gate/{GenKeyGate,GenNativeBindingGate}.cpp`, `test/transpiler/table-keys.sh`, `test/transpiler/table-keys/`. Platform: `analyzers/appsourcecop-as{0010,0118,0123}.md`, `diagnostics/diagnostic-al{256,450,464,527}.md`, `methods-auto/recordref/recordref-keyindex-method.md`. BCApps current main: UserSettings, CreatePickParameters, WordTemplatesTestTable4; pinned System TableMetadata. AL0325's explanatory whitelist contradicts actual Code primary keys; do not adopt it. Focused predecessor search found no default-primary finding; 980/1244's emitted-contract guidance applies.

Platform: `devenv-table-keys.md`, `properties/devenv-{clustered,enabled,maintainsiftindex,maintainsqlindex,unique,sumindexfields,includedfields,sqlindex,obsoletestate}-property.md`, system-field/Insert guarantees and `analyzers/appsourcecop-as{0002,0016}.md`. AL: BCApps main `a9ea4d84534cebba852c44bf0f841c2ea149de4e`, `Layers/W1/BaseApp/Finance/GeneralLedger/{Journal/PostedGenJournalLine,Ledger/GLEntry}.Table.al`, ResourceCost and CopilotSettings; pinned UserPersonalization/FeatureKey. User intent: `business-central/database-missing-indexes.md`. Predecessor 980/1244 show property loss and unreliable name-only audits; their correctness-neutral clustering label is not a metadata contract. Fixtures: `test/gate/{GenKeyGate,UserPersonalizationGate}.cpp`; preserved `artifacts/{old-generator-control-final.log,wrong-key-clang.log,wrong-key-gcc.log,transpile-first-control.log,transpile.log}`. Earlier obsolete-column control remains `build/user-personalization-proof/stored-negative-runtime.log`; Moved/merge identity is 0033.

Property scope: `autoincrement`, `sqldatatype`, `sqltimestamp`.
