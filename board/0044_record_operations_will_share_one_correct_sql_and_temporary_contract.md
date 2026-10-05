# 0044 — Share correct records and live metadata providers

Status: queued | Priority: P0
Depends on: 0013's effective field/schema profile for catalogue activation;
existing record ownership and declaration bindings. Other record repairs can proceed independently.
Next: implement Field Get/Find/Next/Count over installed metadata and prove the
actual FieldName → catalogue → FieldRef caller before retiring seed snapshots.

## Implementation

1. Project AllObj/Field/Table Metadata/Page Metadata from one immutable installed
   registry in `src/rt/{ReflectionMetadata,FieldMetadata,Storage,Selection,Navigate}.cpp`
   and `written/PlatformField.cpp`. Share typed/RecordRef predicates for filters,
   order/count/navigation; keep only cursor indices per handle. Missing metadata
   refuses; no guessed values, copied session catalogue or competing registry.
2. Preserve source app/version/extension property ownership, Name versus Caption,
   read-only live versus writable temporary behavior, exact field/type/length codes
   and permissions. Source CDS → native CRM; provider kind differs from TableType.
   Remove legacy SQL Field copies only on disposable schema-qualified databases,
   after proving persisted-name migration/read behavior. Keep guards until qualified.
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
- `SqlRowVersionGate` adds 95 checks through the production typed Record/RecordRef,
  query, FlowField and cursor paths. Insert/Modify/Rename return the single physical
  version to every alias; ModifyAll allocates per affected row. Exact Decimal buffers,
  identity/creation audit, rollback/two-session fences and missing-column backfill
  survive; repeated provisioning does not restamp and incompatible storage refuses.
  Seven additional compiled controls detect reused/stale versions, wrong physical
  column/filter/query/navigation mappings and zero backfill: fifteen controls total.
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
