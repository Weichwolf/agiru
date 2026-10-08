# 0721 — Complete and qualify the ERP at production scale (G3)

Status: queued | Priority: P2
Depends on: [0720](0720_cli_and_web_will_execute_the_same_erp_operations.md)
acceptance (G2); retain 0058 as the mandatory UT regression gate.
Next after G2: reconcile all test apps/build variants, then execute full AL
tests and agent business workflows on sealed disposable databases.

## Remaining implementation

1. Expand the independent manifest beyond UT suffixes to all selected Test objects
   and methods. Preserve handler-only codeunits, localization/conditional variants,
   omitted app roots and raw/selected/excluded identities. Classify declaration,
   refusal, implementation and execution separately; no green-subset completion.
2. Close remaining in-scope object/extension/native overload/property/trigger gaps
   through the shared generator/runtime primitives qualified under 0058.
   Support real app ownership/permission sets, profiles/control add-ins and published
   generic APIs; exclude commercial and Microsoft-service integrations only.
3. Finish bounded dynamic cursors, keyset page windows, partial-record projection/JIT
   with observed versions, lazy BLOB/media and batched eligible FlowFields.
   Preserve AL own-write/Commit semantics, declared keys and index maintenance.
   Reuse typed filter/query ON/WHERE/HAVING plans; inspect realistic SQL plans.
4. Consume [0063](0063_reports_will_execute_datasets_and_render_declared_layouts.md)'s
   real dataset/request/layout/Cairo/workbook/WPT contracts. Qualify installation,
   approval/company selection, report/extension triggers, fidelity and bounded
   resources on complete business workloads; don't duplicate its layout engine.
5. Complete XMLport XML/text/namespace/encoding/occurrence/validation/import boundaries,
   request pages and bounded stream/ZIP/workbook I/O. Persist ordered media sets and
   chunked payloads transactionally across tiers; explicit missing-ID/lifetime errors.
6. Persist app/version/data-version/company state and a migration journal.
   Lock upgrades in PostgreSQL; build/validate immutable composition, drain sessions,
   migrate, then activate matching binaries/schema. Preserve obsolete/Moved data;
   phase failures resume/refuse safely. DataTransfer is upgrade-only.
   Validate packaged paths/app access; binary rollback is not data rollback.
7. Implement DB-claimed background/scheduled work with leases/fencing/attempts,
   bounded workers, cancellation/retries/failure hooks and child-session isolation.
   Read-only page tasks reject writes/locks; stale owners cannot commit effects.
   External effects need idempotency; no unsupported exactly-once claim.
8. Define matched BC workloads for sales/purchases, journals, stock/warehouse,
    navigation, imports and reports. Match versions/features/hardware/durability,
    data, concurrency/think time; measure release Linux/Podman x86_64/aarch64.
    Record p50/p95/p99, throughput, CPU/request, whole-stack RSS/PSS, marginal
    session memory, DB calls/bytes, allocations, WAL, waits and pool occupancy.
    Separate idle users from active transactions; measure cold/warm/skew/hot-key,
    contention/failure/recovery towards 2 TB and 10,000 users.
9. Compare with equivalent SQL to locate overhead; optimize only measured costs.
    Repeat trials with uncertainty/resource limits. Proposed 2× throughput /
    ≤50% CPU-per-request/session-memory and no p95 regression remain proposals,
    not agreed measurements. Without matched BC, superiority stays unproved.

## Evidence and acceptance

- Refreshed predecessor 1945/1948/1957 rejects priority-queue reordering and image
  GC freezing on end-to-end evidence. Do not import interpreter/GIL workarounds.
  Retain ANALYZE after seed restoration, immutable metadata/parameterized plans,
  bounded reusable workers and before/after SQL/RSS/latency measurements. Warm-up
  must not execute business triggers/writes or hide cold-start costs.
- P2 after G2: authenticated `/admin` operational contracts for environment inventory,
  explicit backup/download, restore/copy into a NEW database, capacity, session
  cancellation and operation receipts. Use bounded jobs and private deployment
  credentials; preserve demo/master data, refuse path/database ambiguity, prove restore.
  PITR requires qualified WAL archival, not a pg_dump claim. Source:
  `~/Git/openerp/openerp/web/admin/{auth,backups,store,insights}.py` (1964/1965).
- Generic AL telemetry stays required despite the reference's namespace omission:
  bounded batches, environment/session dimensions, redaction, PostgreSQL retention
  and visible delivery failures; use a separate transaction, never Commit ERP work.
  Do not import process-global mutable registries/mailboxes or per-database endless
  runners (1970/1979). Multi-session tests must see no identity/policy/data leakage.
- Existing app/version migration work also owns safe extension installation:
  validate paths, size, publisher/app/runtime/ABI/dependencies; versioned immutable
  native packages, schema/data migration, session drain and rollback evidence.
  Reference 1999's Python ZIP import is not native loading or a security proof.
  agiru extensions are native C++ source ZIPs, not AL/transpiler inputs; Linux/Podman
  only, with no WASM-demo requirement. Their sole application dependency is the public
  agiru runtime SDK: no internal headers, additional libraries or arbitrary build scripts.
  Fix include/link inputs and audit ELF dependencies against runtime/toolchain dependencies;
  dependency checks do not sandbox native code. Compile only the extension in an isolated
  builder. Versioned module ABI and generation-owned registries must precede activation;
  this contract is not evidence of an implemented loader or an approved hot-unload design.

- No matched BC benchmark, 2 TB / 10,000-user or complete aarch64 qualification exists.
  Existing `make native-report-layouts` proves declaration/link/package controls
  (562+562 checks, sixteen assets), not installed/selected layouts, rendering,
  a complete app tree or business execution; 0063 owns their qualification.
- Complete selected AL suite and agent workflows pass; independently verify ledger
  amounts, atomic boundaries, permissions, users/companies and deterministic outcomes.
- Multi-tier restart/disconnect/stale writer/exhausted pool tests retain durability
  and bounded resources. No gain by removing semantics/work or disabling durability.
- BC comparison covers genuine layouts/workbooks, charts and interactive analysis.
  Unsupported assets/capabilities stay visible; preserve licenses/notices.
- Keep workload specifications/commands in `test/` and durable concise evidence
  with pinned revisions in the owning WI; never depend on disposable receipts.
  Production native architecture remains primary.

## Files and consolidation

`src/rt/{Cursor,Navigate,Storage,Session,Query,Report}.cpp`,
`src/net/Decimal.cpp`, `src/gen/{CodeunitWriter,ReportLayoutsWriter,ReportAssets}.cpp`,
`include/runtime/ReportRegistry.h`, `include/runtime/Report.h`,
`test/{reporting,runtime,ui}/`, `CMakeLists.txt::AGIRU_FAST_LOOP`.
Local developer/user report, XMLport, stream, upgrade, task and ledger documentation;
BCApps originals before predecessor implementation. G1-required dataset/stream fixes
remain executable steps in 0058; this WI does not block those repairs.
Absorbs 0070, 0074, 0090 and prior performance requirements 0008/0009/0596;
0063 remains the focused reporting owner.
Detailed matrices and previous absorbed-ID mappings:
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
