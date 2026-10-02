# 0013 — System fields and schema keys will obey their declared contracts

Status: open | Priority: P1 | Stage: UT → Clients | Reviewed: 2026-10-01
Depends on: 0033 declaring-app identity. Activation: 0034/0058 unchanged-population proof.

## Evidence

- `Storage.cpp` generates keys and system fields, but writes lack observed-rowversion predicates. Field presence is not a uniqueness/audit/concurrency guarantee.
- Own-only `build/implicit-key-20261001/source`: one AL completion rule supplies ordinary/native defaults before extension merging and system fields. No source keys means an implicit primary, not zero keys; preserve the lowest eligible field ID and original name. Table Metadata requires `ID`/field 1; never delete its key to satisfy the former checker. Unknown eligibility refuses.
- GenKey 68/68 and generated execution 12/12 Clang/GCC; archived generator nine red over the same twelve execution checks. Lower extension IDs/secondary keys cannot replace the base default. Invalid defaults retain source/UT counts and fail translation (0589). All 24,350 paths retained, 79 changed; source UT 80/2,310 unchanged, unexecuted. Receipts: `build/implicit-key-20261001/{README.md,source-identity.json,artifacts/}`.
- Existing shared key projection preserves declared flags, sums/order, IncludedFields and text. Duplicate clustered selections, invalid Boolean values and SqlIndex refuse. Integrated `Stored()` retains Removed Normal columns; reference restrictions and FlowFields/FlowFilters stay separate. Earlier controls are indexed in README. Current SQL verification is unavailable; no physical-clustering or production-activation claim.

## Implementation

1. Use one database-wide sequence for rowversion; include the returned version in RecordState and expose it to 0012 without widening every generated field wrapper.
2. Audit SystemId uniqueness, supplied-ID Insert overloads, created/modified stamps and database-wide monotonic SystemRowVersion. Assign versions in PostgreSQL so independent tiers cannot collide.
3. Activate the shared default/key projection only with 0034/0058 proof. Complete unsupported default eligibility from platform evidence; never select a later field silently. Implement SqlIndex as separate SQL fields, not SetCurrentKey fields; keep its refusal until supported. Enforce Unique, MaintainSqlIndex, IncludedFields, Enabled, AutoIncrement and SqlTimestamp restrictions. No SQL Server physical-clustering promise for PostgreSQL.
4. Return platform-owned fields from writes and preserve identity on Rename. Coordinate version checks with 0012 and company-qualified sequences with its company work.
5. Use typed generated metadata for constraints and migration comparisons; never infer a field number from display order.
   Retain obsolete Normal columns and their data through synchronization; exclude FlowFields/FlowFilters independently. Do not conflate Removed with declaring-app migration/Moved (0033).

## Acceptance

- Two connections writing different tables receive distinct increasing versions. Duplicate supplied SystemId fails, Rename retains it, audit stamps follow documented trigger order, and disabled/nonmaintained keys generate the intended DDL.
- Default controls cover reordered IDs, original case/spaces, unsuitable/unknown fields and extension keys; generated Get/Insert/duplicate behavior uses the base primary. Retain explicit-key/Clustered controls, including Posted Gen. Journal Line. SQLIndex fields may differ without changing logical uniqueness/navigation; unsupported forms never succeed silently.

## References

Code: `src/rt/Storage.cpp`, `src/gen/TableWriter.cpp`, `include/meta/TableDef.h`.

Default keys: `src/al/Parser.cpp::EnsurePrimaryKey`, `src/tc/Main.cpp::IndexTables`, `test/gate/GenKeyGate.cpp`, `test/toolchain.py::ImplicitPrimaryKeyGate`. Platform: `analyzers/appsourcecop-as{0010,0118,0123}.md`, `diagnostics/diagnostic-al{256,450,464,527}.md`, `methods-auto/recordref/recordref-keyindex-method.md`. BCApps current main: UserSettings, CreatePickParameters, WordTemplatesTestTable4; pinned System TableMetadata. AL0325's explanatory whitelist contradicts actual Code primary keys; do not adopt it. Focused predecessor search found no default-primary finding; 980/1244's emitted-contract guidance applies.

Platform: `devenv-table-keys.md`, `properties/devenv-{clustered,enabled,maintainsiftindex,maintainsqlindex,unique,sumindexfields,includedfields,sqlindex,obsoletestate}-property.md`, system-field/Insert guarantees and `analyzers/appsourcecop-as{0002,0016}.md`. AL: BCApps main `a9ea4d84534cebba852c44bf0f841c2ea149de4e`, `Layers/W1/BaseApp/Finance/GeneralLedger/{Journal/PostedGenJournalLine,Ledger/GLEntry}.Table.al`, ResourceCost and CopilotSettings; pinned UserPersonalization/FeatureKey. User intent: `business-central/database-missing-indexes.md`. Predecessor 980/1244 show property loss and unreliable name-only audits; their correctness-neutral clustering label is not a metadata contract. Fixtures: `test/gate/{GenKeyGate,UserPersonalizationGate}.cpp`; preserved `artifacts/{old-generator-control-final.log,wrong-key-clang.log,wrong-key-gcc.log,transpile-first-control.log,transpile.log}`. Earlier obsolete-column control remains `build/user-personalization-proof/stored-negative-runtime.log`; Moved/merge identity is 0033.

Property scope: `autoincrement`, `sqldatatype`, `sqltimestamp`.
