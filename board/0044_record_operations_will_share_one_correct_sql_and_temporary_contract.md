# 0044 — Share correct records and live metadata providers

Status: in progress | Priority: P0
Depends on: 0013's effective field/schema profile for catalogue activation;
existing record ownership and declaration bindings. Other record repairs can proceed independently.
Next: replay qualified Integer and live Field repairs with 0013/0058 on every UT
identity; implement installed Table Metadata filters/order/navigation through shared
primitives. Complete Field classification, SQLDataType, package provenance, permissions
and RecordRef.Get result consumption. Investigate StoredImage.

## Implementation

1. Project AllObj/Field/Table Metadata/Page Metadata from one immutable installed
   registry in `src/rt/{ReflectionMetadata,Storage,Selection,Navigate}.cpp`,
   `FieldMetadata.h` and `written/PlatformField.cpp`. Share typed/RecordRef predicates for filters,
   order/count/navigation; keep only cursor indices per handle. Missing metadata
   refuses; no guessed values, copied session catalogue or competing registry.
2. Preserve source app/version/extension property ownership, Name versus Caption,
   read-only live versus writable temporary behavior, exact field/type/length codes
   and permissions. Source CDS → native CRM; provider kind differs from TableType.
   Field provisioning no longer creates SQL copies; live readers ignore existing copies.
   No physical rows/tables were deleted. Qualify any later migration independently.
   Keep storage guards on providers whose navigation is not implemented.
   Field customization defaults belong to the declaring table/extension: an extension's
   AllowInCustomizations affects its new fields, not base fields; field overrides win.
   Retain this origin rather than applying the merged table default to every field.
3. Preserve SQL/temporary Record and RecordRef parity: Init/Clear, assignment/Copy/
   ShareTable, Get/filter/marks and trigger-aware ModifyAll/DeleteAll. Keep alias
   ownership and temporary storage distinct from filter/cursor/record state.
4. Fix only reproduced key/cursor faults: full-prefix first-key choice, disabled/
   duplicate keys, mixed directions, negative Next, dynamic own writes and sort-field
   changes. Retain Next(0), current field-length and filter-group regression tests.
   Trace SCMPlanningUT's four Inventory Profile paths; don't restore ignored keys.
5. One typed filter AST handles quote escapes, GUID values, same-field intersection,
   group -1 cross-column OR, marks and SQL/temporary behavior. CalcFormula resolves
   every predicate; query ON/WHERE/HAVING and NULL/default/ReverseSign are distinct.
6. Preserve bounded dynamic cursors across writes/Commit, transaction generations,
   partial-field/JIT observed-version checks and lazy BLOB loading. Page display
   windows use stable keyset/tiebreaks, not a complete ledger scan or deep OFFSET.
   Batch eligible displayed FlowFields without changing trigger/current-row semantics.
7. PostgreSQL owns database-wide rowversion and returned identity/audit values.
   Preserve supplied SystemId uniqueness/Rename, observed-version DML and LockTable
   wait/version policies. Commit durability differs from test isolation/virtual buffers.
   Use `runtime/RowVersionStorage.h` / `src/rt/RowVersionStorage.cpp` for provisioning;
   Retain the qualified Insert/Modify/Rename allocator and returned-buffer paths.
   SqlTimestamp aliases map to one physical column, never writable duplicate columns.
   Complete generated/native profile coverage and protected observations for stale writes;
   a mutable FieldRef/buffer value is not a concurrency token. Same-transaction writes
   without Commit must not produce the documented post-Commit stale-record error.
   Activate LastUsed/MinimumActive only after every production DML path is qualified.

## Useful implementation details

- Direct sandbox metadata (2026-10-05, CH BC 28.5/platform 28.0.54688.0;
  not the frozen SDK/demo version): table viewer 2000000041 exposes
  `FieldName=$systemId`, but `FieldName=SystemCreatedAt`; the missing-index page's
  SQL include list instead uses `$systemCreatedAt`. Do not normalize AL catalogue
  names to SQL storage names. Table Metadata 2000000136 exposes virtual rowversion
  1 and deterministic SystemIds in sampled rows; this is not storage/DML proof.
  Table Information 8700 includes company/global rows, counts, sizes and compression;
  Database Locks 9511 was empty, Wait Statistics 9520 had 19 categories, Missing
  Indexes 9521 had one recommendation. PostgreSQL providers need their own qualified
  metrics; do not fake Azure SQL waits/recommendations. Plain `table=<id>` works;
  adding the sampled `filter` URL parameter refuses with unknown RunTable parameters.
- `~/Git/openerp/openerp/runtime/base/table/{_table,_temp_table}.py`:
  distinguish AL dynamic navigation from client `_display_window`.
  Refreshed display uses OFFSET/LIMIT; predecessor 1868 still requires keyset,
  current-bookmark positioning and batched FlowFields. Don't copy full-list materialization.
- `runtime/base/test_page.py` and `web/client/page_model.py::_collect_rows`:
  fetch rows+1 to expose continuation, preserve current row and untouched new input.
  Port behavior via existing C++ metadata, not Python reflection flags.
- `web/client/analysis.py`: SQL GROUP BY/SUM/COUNT and separate temporary evaluator;
  reuse typed aggregate/filter primitives. Bound pivots/FlowField fallback and keep
  complete totals; no second SQL builder.
- Predecessor 1897 is an open partial-posting/lock-timeout report, not proven BC
  transaction authority. Inspect original AL Commit boundaries before claiming rollback.
- `runtime/builtins/_system.py::_row_version` uses XID/xmin and returns zero on errors;
  reject this approach: transaction IDs are not distinct row-write versions.
- Rowversion foundation: one logged bigint sequence, CACHE 1 / NO CYCLE / OWNED BY NONE.
  Negative bigint advisory keys retain the first allocation of each active transaction;
  an ASCII AGRV two-integer gate serializes initial publication and minimum reads.
  Transaction-local fence state rolls back with savepoints; later row allocations reuse
  the fence without per-row locks or pg_locks scans. This is not complete ERP DML coverage.

## Acceptance

- Integer SQL projection now preserves the selected Runtime-18 profile: original
  BC29 virtual timestamp 1, physical timestamp alias, blank identity/audit values
  and nonstored user lookups. Unknown/mistyped stored fields still refuse.
  Cursor/Filter gates pass 255/129 checks; typed Record and RecordRef preserve
  bounded counts, filters, forward/reverse navigation and exact implicit values.
  Selection.cpp and both gates pass focused tidy; seven unchecked test optional
  accesses were repaired without weakening comparisons or suppressing diagnostics.
  `make record-order JOBS=2` retains the 26 prior controls and rejects zero-version
  and source-alias mutants (seven/one failed checks). Original authority, artifact
  hash and call-chain RVAs are in 0013. No PostgreSQL allocator or native profile
  rollback is used. This is not wide-domain/cardinality, security or full UT proof;
  the existing million-row cap still requires replacement, not acceptance.
- SQL `GetBySystemId` now shares one reader and optional-result wrapper for typed
  records and RecordRef. A consumed miss returns false; a discarded miss raises
  the searched SystemId diagnostic instead of allowing work on a stale buffer.
  Storage failures still throw; RecordRef must already be open. Successful reads
  forget the old cursor and preserve a position for subsequent Next; filters remain
  unchanged. Temporary and unqualified virtual providers still explicitly refuse.
  `make gate GATE=SqlRowVersionGate JOBS=2`: 114 checks, including nineteen new
  identity/Decimal/version/filter/missing/error/navigation checks on an owned database.
  RecordRef/RecordImage gates retain 147/28 checks. `make rowversions JOBS=2`
  retains fifteen compiled controls and rejects two new controls: discarded-miss
  suppression fails four checks; lost positioning fails two. Named/numeric Record
  and RecordRef AL value/statement fixtures compile in both table/codeunit contexts;
  `make verify-check VERIFY_CHECKS='' JOBS=2` passes all 235 tooling tests.
  This is compile qualification plus live C++ SQL proof, not AL workflow acceptance.
  SqlRowVersionGate/Table.cpp focused tidy retain three/sixteen existing findings;
  no new test finding or suppression. Full integration replay remains required.
  Developer `f928288ee840`: methods-auto/{record,recordref}/*-getbysystemid-method.md;
  BCApps `d99152ee35f0`: System/Workflow/RecordRestrictionMgt.Codeunit.al,
  PrintGenJnlLineSystemId lookups before Insert/ModifyCheckLedgerEntry.
  Predecessor 1462 documents discarded misses allowing stale Sales/Purchase/Service
  line writes; reuse C++ consumption semantics, not Python statement flags.
- Installed `Table Metadata.Get` now reaches the immutable registry through
  `src/rt/Table.cpp::RuntimeGet`, shared by typed records and RecordRef. It ignores
  ordinary filters without replacing them, preserves optional-missing versus discarded
  errors, projects before changing attributes and refuses unqualified field bindings.
  No SQL snapshot or per-session catalogue; temporary reads still use their own store.
  Frozen provider version 1 and blank audit fields follow the sampled BC 28.5 viewer,
  not Integer or PostgreSQL rowversion. Find/Next/Count/GetBySystemId and live writes
  remain explicitly refused; this is not complete provider or security-filter acceptance.
  ReflectionMetadataGate passes 294 checks, including exact typed/RecordRef agreement
  on all 29 stored fields. Generated source execution passes 115 checks.
  `make reflection-metadata JOBS=2` rejects all 43 compiled controls plus the header
  control, adding false-missing, zero-version and stored-FlowField mutants.
  TableMetadata.cpp and generated Runner focused tidy pass. Table.cpp still reports
  sixteen diagnostics; the expanded gate exposes a StoredImage uninitialized-ID
  diagnostic not reported by the previous gate. No suppression or baseline increase.
  Developer `f928288ee840`: devenv-virtual-tables.md and record-get-method.md;
  BCApps `d99152ee35f0`: System/Workflow/WorkflowEvent.Table.al uses optional Get.
  Predecessor 1229 distinguishes filter-blind Get from filter-aware Find.
  Current-tree AL replay is still required; no UT gains are claimed.
- Native `Field.Get` uses `RuntimeGet` / `GetInstalledFieldMetadata` for typed and
  RecordRef reads, with the original positive-key catalogue domain and checked native
  field-span ABI. It never reads a SQL copy. Native zero/negative keys miss; omitted
  trailing keys still default to zero. Timestamp remains addressable by FieldRef(0),
  positive reserved fields remain catalogued, and temporary zero-key rows stay writable
  and readable through both paths. Typed consumed/discarded misses preserve false versus
  searched-key errors; projection errors throw and filters survive. RecordRef.Get's
  own discarded-result wrapper and security-filter enforcement remain separate gaps.
  `PlatformFieldGate`: 418 checks, including stored-value parity and qualified binding
  refusal. Runtime mapper and gate focused tidy pass after adding the named Record.h
  dependency; Table.cpp retains sixteen findings, without suppressions/baseline increases.
  Reflection qualifier retains its 43 compiled controls and adds catalogue-zero and
  unchecked-binding controls (four/two failed checks). Field Access/search/customization
  policies, unknown-value refusal and owned moved-error keys remain qualified.
  Original BC29 authority: FieldDataProvider iterator RVA `2e888c`, source/hash in 0013.
  Developer `f928288ee840`: record-get, recordref-get/field and devenv-virtual-tables;
  BCApps `d99152ee35f0`: ConfigPackageField uses consumed/discarded typed reads,
  DataTypeManagement.FindFieldByName uses FieldName → Field.FindFirst → FieldRef.Value.
  Predecessor 1114 requires that actual name-derived caller; 1136 rejects hidden misses.
  Native Find/FindSet/Next/Count/IsEmpty now use `src/rt/FieldNavigation.cpp`: one
  shared immutable locator index, table-range narrowing and indexed primary navigation;
  other valid orders use bounded-memory selection scans, not per-session row copies.
  Existing RecordFilter/RecordOrder preserve groups, marks, mixed order and exact values.
  Handles retain independent bookmarks even after buffer edits; changed views re-anchor.
  `FieldCatalogueGate`: 61 checks, including the real FieldName → filtered catalogue →
  FieldRef.Value caller, source-declared/positive implicit counts, typed/RecordRef parity,
  signed/extreme Next and temporary zero-key independence. Live DML, empty ModifyAll
  and empty DeleteAll(true) refuse; temporary bulk writes remain valid. A pre-fix
  replay fails exactly the new empty triggered-delete claim. The qualifier rejects 51 compiled
  controls plus the header control; six new controls detect timestamp population, lost
  bookmarks, ignored filters, zero version/identity and both empty-write guards.
  Original BC29 FieldDataProvider.GetFieldRecordBuffer RVA `a4330` supplies metadata
  identity from `{2000000041, TableNo, No, 0}` and frozen version 1 through
  VirtualDataProvider; original artifact/hash and initialization RVAs are in 0013.
  DataClassification, SQLDataType, AppPackageID and AppRuntimePackageID are still
  unprojected; filtering/sorting them refuses instead of trusting default values.
  Complete projection, authorization, secondary-order performance and full AL replay
  remain open. Do not claim the aggregate UT is repaired without its executed result.
  FieldNavigation.cpp, Navigate.cpp, PlatformField.cpp and the new catalogue gate pass
  focused tidy. Storage.cpp/Table.cpp report eight/sixteen diagnostics outside changed code;
  no suppression or baseline increase. Full current-tree integration remains required.
  Serialized `make gate GATE=<name> JOBS=2` replay passes DynamicRecord/Storage/
  Temporary/Cursor/Filter at 7493/71/95/255/129 checks, including existing SQL and
  temporary bulk-write behavior. `make slice-check`: 14225 sources, none missing.
- `AllowInCustomizations` defaults stay on the declaring fields in `src/al/Ast.h` /
  `Parser.cpp`, before extension merging. Extension properties survive parsing;
  explicit field values override that owner's default in `src/gen/TableWriter.cpp`.
  Existing runtime metadata carries the effective string without widening public
  structures or adding includes. The native Field projection treats Never as unavailable;
  ToBeClassified/AsReadOnly/AsReadWrite and deprecated Always are available, independently
  of Editable. Unknown values refuse before replacing projected attributes.
  `make table-keys JOBS=2`: generated execution passes 115 checks, with separate base,
  extension, override and omitted-extension defaults; both new generated owner/override
  mutants fail two checks, while existing controls remain. The two new runtime mutants
  reject always-available and Editable-derived flags. This qualifies declared policy,
  not customization editability, implicit-field flags or actual client behavior.
  Parser/GenTable/source-binding gates pass 159/105/271 checks; GenCodeunit passes 49.
  Runtime mapper, Field gate, generated fixture, Parser.cpp and AlParserGate.cpp pass
  focused tidy. Separate label reading from variable-block traversal and argument scanning
  from attribute lookup; retain grouped declarations, quote/comma/empty argument handling
  and procedure boundaries. The report fixture remains unchanged outside its assertion
  function. TableWriter.cpp focused tidy now passes: separate declared-member formatting,
  relation handling and dependency emission; six complexity/concatenation findings removed.
  Twenty added GenTable checks preserve empty InitValue, numeric IDs, flags, relation forms
  and narrow header/forward declarations. The previous emitter also passes; four compiled
  mutants lose empty initializers, typed IDs, compound relations or forward declarations
  and are rejected. Golden specifications are unchanged. GenTableGate's focused tidy retains
  the existing BodyWriter.h PartControlSpelling adjacent-parameter finding, not a new test finding.
  The new pointer-conversion and fixture-number findings were repaired, not suppressed.
  Verified-package regeneration still refuses 5683 properties (exit 1); this is not G1.
  Developer `f928288ee840`: `properties/devenv-allowincustomizations-property.md`;
  BCApps `d99152ee35f0`: `Apps/W1/Subcontracting/App/src/Purchase/SubcPurchaseHeader.TableExt.al`
  declares AsReadOnly for its new fields. Predecessor `board/docaudit/03_properties.tsv`
  treats this as UI-only; do not discard the required native metadata flag.
  Predecessor 1310 records lost extension modifications; modification precedence
  remains a separate merger qualification. DataClassification's linked AL property page
  is absent at developer HEAD and redirects online. Git `8789c061b446` retains that page,
  but describes both ToBeClassified as initial and CustomerContent as default, without
  table inheritance guarantees. Current onprem/classifying-data.md and AS0016 establish
  new-field/Flow classification; qualify table/extension and implicit defaults separately.
  Four declared Field attributes remain unprojected:
  DataClassification, SQLDataType, AppPackageID and AppRuntimePackageID.
- Temporary record arrays now share one row store across dimensions, not their
  field buffers or filters; distinct arrays and ordinary/scalar elements stay
  independent. `TemporaryGate` passes 95 checks and `AlArrayGate` seventeen.
  `make record-order JOBS=2` retains all prior controls and adds a compiled
  separate-store mutant: all twenty-six reject, including two restored-noexcept
  overloads rejected by typed assertions. Copying rvalues uses the existing copy
  path; throwing-element checks prove error propagation, not process termination.
  `include/type/AlArray.h` adds no
  include and passes standalone `make include-cost HEADERS=type/AlArray.h`.
  Developer `f928288ee840`: `methods/devenv-array-methods.md` explicitly specifies
  shared temporary array rows; record-copy/get preserve views and primary-key lookup.
  BCApps `d99152ee35f0`: InventoryProfileOffsetting.CalculatePlanFromWorksheet
  populates element one and passes both views to planning. Predecessor board 1079
  identifies the same storage boundary. The older frozen replay still has all four
  SCMPlanningUT missing temporary row `3` failures; this repair needs full AL replay.
  Focused tidy remains red: twelve existing TemporaryGate/header findings and
  fourteen AlArrayGate/header findings; the false noexcept overloads are removed.
  No new helper/test finding, suppression or baseline increase; full tidy is open.
- Record boxes expose borrowing accessors, not writable ownership/table/identity slots.
  `RecordRefGate` qualifies independent snapshots, typed writes, SetTable and clearing
  one copied slot, plus state self-assignment versus explicit Copy; 147 checks pass.
  `reflection-metadata.sh` rejects public-owner and assignment-state mutants.
  This is ownership/API evidence, not live link-storage or complete business proof.
  Developer `f928288ee840`: recordref gettable/settable/copylinks/haslinks and
  duration-data-type; BCApps `d99152ee35f0`: System/Workflow/WorkflowRecordManagement.
  Predecessor 1095/1241: preserve independent Variant boxes and complete typed snapshots.
- Same fixture yields keys/values/filters/events through typed SQL, RecordRef and
  temporary paths; include absent/duplicate keys, malformed values and negative Next.
- Live catalogue exact lookup/name/count/order/permission/write controls fail mutants;
  temporary catalogues alone do not prove production virtual-table navigation.
- Two connections prove distinct database-wide versions, stale-write conflict,
  durable Commit/restart and boundary rollback. Never mutate seed/master templates.
- `make rowversions` uses owned empty databases: 114 checks, 384 concurrent SQL writes,
  savepoints/errors/disconnect, incompatible storage, 32/64-bit bounds and eight
  foundation controls for missing/per-row fences, unsafe minimum/cache, cross-database
  locks, publication races and cancellation leaks. Provider and gate pass focused
  clang-tidy. Reconnect is covered; PostgreSQL server restart/WASM are not.
- `SqlRowVersionGate` passes 114 checks through the production typed Record/RecordRef,
  query, FlowField and cursor paths. Insert/Modify/Rename return the single physical
  version to every alias; ModifyAll allocates per affected row. Exact Decimal buffers,
  identity/creation audit, rollback/two-session fences and missing-column backfill
  survive; repeated provisioning does not restamp and incompatible storage refuses.
  Seven additional compiled controls detect reused/stale versions, wrong physical
  column/filter/query/navigation mappings and zero backfill; two SystemId controls
  qualify optional results and positioning: seventeen controls total.
  `src/rt/SqlColumn.{h,cpp}` owns field-to-column mapping, including relative/mixed-key
  navigation. Source names/types/AutoIncrement/FlowField collisions refuse before DDL.
  This is an authored selected profile, not original-source/full-tree DML proof;
  current generated apps remain unselected. Keep AL rowversion methods refused.
  Shared-header/pre-existing runtime clang-tidy findings remain red; no suppressions
  or baseline increases. Full UT replay after this integration remains required.
- Run `make gate GATE=RecordRefGate`, `PlatformFieldGate`,
  `ReflectionMetadataGate`, cursor/filter/transaction gates and
  `test/runtime/reflection-metadata.sh`; replay all UT under 0058.
  Local docs: record/recordref methods, dynamic results, read-isolation/tri-state locking.
  References: developer `ff5939a46e`, BCApps `bb7111877f`; predecessor 0889/1114/1868.
- Rowversion contract: developer `f928288ee840`:
  `devenv-table-system-fields.md`, `methods-auto/database/database-{lastusedrowversion,minimumactiverowversion}-method.md`.
  Also `properties/devenv-sqltimestamp-property.md` and
  `methods-auto/record/record-{modify,rename,find,next}-method.md`;
  BCApps `d99152ee35f0`: Shopify `Inventory/Tables/ShpfyShopLocation.Table.al`
  declares the BigInteger Version alias (field 3, SqlTimestamp).
  PostgreSQL 17: [sequences](https://www.postgresql.org/docs/17/sql-createsequence.html),
  [advisory-key layout and database scope](https://www.postgresql.org/docs/17/view-pg-locks.html),
  [savepoint lock release](https://www.postgresql.org/docs/17/explicit-locking.html),
  [disjoint advisory key spaces](https://www.postgresql.org/docs/17/functions-admin.html).

Absorbs 0018, 0019, 0045 and their former absorbed IDs. Detailed matrices/mappings:
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
