# 0044 — Record operations will share one correct SQL and temporary contract

Status: open | Priority: P0 | Stage: UT | Reviewed: 2026-10-05
Depends on: 0718 images; 0013 schema metadata; 0034 native declarations.

## Evidence

- Original native runtime package is now source-qualified:
  SystemApp.dll's embedded System 29.0.55365.0/Runtime 18 matches all 398
  verified AL sources byte-for-byte; all 234 compiled tables remain inventoried.
  Missing-source/wrong-version controls refuse. Original `MetaTable` XML creation
  and production `PlatformMetadataProvider::GetMetaTableFromXml` each process
  all 234 originals on CLR 10.0.12: zero refused, no field removal/stubs,
  zero static XML overrides. `NCLMetaTable::AssignFromMetaTable` copies the
  qualified defaults but sets company scope only for non-system Normal tables.
  Native Table Metadata is Normal, independently virtual, and has audit fields;
  its 23 source fields become 33: timestamp 0, SystemId, four audit fields and
  four Runtime-18 user-name/full-name FlowFields. agiru still exposes only 28.
  `/tmp/agiru-table-provider-authority.CyvRCA/native-profile.json` retains full
  populations, original hashes and successful/failed probes. This qualifies the
  native property/creation path, not agiru's complete profile/provider or G1.

- Qualified native property projection is implemented without a second profile
  registry or public header change. `TableMetadata.cpp` shares the proven AL
  string defaults, preserves explicit values/source omissions, and projects
  company scope only for non-platform Normal tables. Production parsing of all
  234 original AL tables feeds the C++ projection: ten properties/2,340 comparisons
  match original NCL assignment exactly. Restoring source-only company scope
  yields 23 native differences; no source/test identity disappears.
  ReflectionMetadataGate retains prior cases: 227 checks/zero red; source gate
  2,917/zero red. Twenty-one compiled controls reject, including restored native
  omission refusal/company scope. Reference/ASan+UBSan gates pass 14,329 checks
  each (gate/identity instrumented, other runtime dependencies normal).
  Targeted runtime/gate analysis has no own findings; 30/30 inherited header
  findings remain unsuppressed. `/tmp/agiru-table-provider-authority.CyvRCA/`
  retains `native-source-comparison.json` and `native-defaults.json`.
  Complete implicit profile, schema activation and shared live read-only provider
  remain open/guarded. No new full UT result or business activation claim.
  `417954e` is pushed; slice-check has 14,225 sources/zero missing, full build
  exits 0/8s, all 25,285 compiler inputs match afterward. Complete local replay
  ends with 146 cases/one red and 233 tooling tests passing. The authored native
  lookup fixture still expected the retired omitted-property refusal; it now
  checks the exact missing-original-App-ID refusal, never a fabricated owner.
  All 61 checks remain; `make table-keys` passes 34/166/61 checks, seven compiled
  and five source controls reject, fixture analysis passes.
  `native-defaults-local.log` and `native-defaults-table-keys{,-lint}.log`.
  Later `cd61d73` replay: all C++ gates/qualifiers show zero red; whole Make test
  ends with 146 cases/one red from two tooling output-isolation errors. Parent
  UT_LOG reproduces them; the ordinary thirteen Milestone cases and all 233
  clean-environment tooling cases pass. Preserve the failed original receipt.
  Lint stays red at 55 silent places/baseline 13; compiler inputs/dependencies
  match afterward. Full AL replay is terminal: 2,173/2,314 passed, 141 failed,
  zero incomplete; 80 codeunits/six workers/1,432s, runner 1/Make 2, no
  infrastructure errors. All identities/statuses/errors/source hashes match
  the preceding full replay: zero gains/losses/missing/added/duplicates.
  `/tmp/agiru-native-defaults-integration.0S8HWX/{result,ut-comparison}.json`.
  Null/unsealed seed is diagnostic, not causal A/B/G1. Fixture output isolation
  now strips parent B/UT_LOG: the strengthened existing case fails before the
  repair, thirteen overridden cases and all 233 tooling cases pass afterward.
  No parent log is created. Whole Make regression needs a new replay; shared
  effective field length is next before integration.

- Complete UT replays through `cd61d73` retain 48 Table Metadata provider
  refusals, unchanged against the preceding full population; 2,173/2,314 pass,
  141 fail, zero incomplete. README retains the source/image/package hashes and
  exact comparison. Step 5 owns the native property/module/implicit-field authority
  and shared read-only provider; do not remove the guard or manufacture defaults.
  Closing this boundary is not a prediction that all 48 business methods pass.

- Table Metadata identity prerequisite: `TableDataProvider` (table 2000000136),
  iterator `MoveNext` RVA 2efed4, constructs `(provider ID, table ID, 0, 0)` and
  calls keyed `VirtualDataProvider::CreateVirtualRecordBuffer`. Original Ncl-29
  `MetadataSystemId` constructors/explicit layout overlay four Int32s at 0/4/8/12
  with a GUID. Executing the original constructors on CLR 10.0.12 gives 14,101
  exact, reversible reference rows; private `MetadataSystemId.{h,cpp}` matches all
  signed bits and GUID byte order without host-endian casts. TableMetadata.cpp
  now projects that stable identity, not an empty/random/company-specific GUID.
  `make reflection-metadata`: 204 checks, 2,917 source checks, twenty controls
  green; reference and ASan/UBSan runs each pass 14,306 checks. Four new mutants
  reject byte/key order, empty identity and wrong provider. Receipt:
  `/tmp/agiru-table-provider-authority.CyvRCA/receipt.json`. Targeted analysis
  reports no own findings; 25/30/30 inherited header findings remain unsuppressed.
  Guard, native omitted properties, audit/rowversion visibility, schema activation
  and shared read-only operations remain open; no new full-UT/provider claim.

  Full integration of this identity prerequisite on `976e479` passes:
  `make all JOBS=6` exit 0/12s; `make test JOBS=2` exit 0,
  146 cases/zero red and 233 tooling tests. All 25,285 compiler inputs match
  afterward. `/tmp/agiru-table-provider-authority.CyvRCA/integration.json`
  retains commands, HEAD, input digest and exits; no new AL UT execution.

- Rename cascade read anchor: `src/rt/Rename.cpp::Rewrite` now writes through an
  independent record, leaving its reader on the old key for dynamic Next resumption.
  The preceding implementation skipped remaining old-parent children when their
  leading primary-key component moved forward. RenameGate retains every original
  case and adds both parent-rename directions, typed/reflected parents, keyed/non-key
  children at 1/64/130 rows, exact aggregates and unrelated-parent controls: 4,886
  checks green, preceding actual runtime 780 red on the identical fixture.
  `/tmp/agiru-rename-anchor.7GvVDQ/{before,after}.log`. Initial twenty-three compiled
  controls reject and input hashes match (`/tmp/agiru-record-order-controls.TqaC8V`).
  Final cleanup reuses one writer and adds direct includes; 4,886 checks stay green.
  Analysis has no own findings, 30/35 inherited runtime/gate header findings remain
  unsuppressed. Final controls pass: 5,872/296/313/7,493/4,886 checks green,
  twenty-three compiled controls reject; hashes match (`/tmp/agiru-record-order-controls.aUoxgz`).
  All three original codeunits complete: 55/57 invoice, 43/44 sales credit memo,
  37/38 purchase credit memo; 135/139 passed, four unchanged Table Metadata failures.
  Three Rename gains, no losses/changed errors/missing/added/duplicates, unchanged
  source denominator (`three-codeunits-comparison.json`). Source/image hashes match
  after execution. Owned diagnostic databases and the disposable image are removed;
  receipts remain. Complete current-tree `make test JOBS=2` on `81c5209` passes:
  141 cases/232 tooling tests, exit 0; input hashes match before/after. All
  twenty-three record controls reject (`/tmp/agiru-record-order-controls.fAdRKi`);
  `/tmp/agiru-rename-anchor.7GvVDQ/all-local.log`. The diagnostic
  image retains all seven non-runtime hashes from 223110, replaces libagiru_rt
  only and does not qualify rebuilt ModifyAll callers or the full UT population.
  Frozen 232014 includes the ModifyAll caller fix, not this cascade fix. Its full
  UT replay is terminal: 2,167/2,314 passed, 147 failed, zero incomplete; against
  223110 no gains/losses/changed errors/missing/added/duplicates, source hashes match.
  Frozen source/System/notice pre/post hashes match (`artifacts/ut-comparison-223110.json`).
  Full cascade-fix replay 004617 is terminal on `24af1d8`: slice/build/local tests
  pass; UT is 2,170/2,314, 144 failed, zero incomplete, 80 codeunits/1,330 seconds.
  All three original Rename tests recover; against 232014 no losses/changed errors/
  missing/added/duplicates, source identities/hashes match. Frozen source/System/
  notice hashes match before/after (`artifacts/ut-comparison-232014.json`). No causal
  A/B or G1 claim; continue the remaining record/provider/UT gaps below.
- ModifyAll caller preservation: `Table.h` uses an independent filtered worker,
  copies NewValue before writes and borrows shared temporary rows. The caller's
  field buffer/SystemId/position survive; Next observes changed stored successors.
  DynamicRecordGate retains every prior case and adds SQL/temporary own bulk walks
  in all eight orders at steps 1/64: 7,493 checks green, preceding implementation
  144 red on the same fixture. New source has no analysis findings; 35 existing
  header findings remain unsuppressed. `/tmp/agiru-modifyall.tQeH01`.
  `make record-order`: 5,872/296/313/7,493 green, twenty-two compiled controls reject;
  the restored caller-traversing header fails the same 144 checks. Input hashes match
  (`/tmp/agiru-record-order-controls.ZF440l`). Outside frozen 223110; original full
  AL execution, sort-field mutation, native global/trigger proof and set-based SQL
  remain open. StorageGate retains 71 green checks, including no-OnValidate.
  Disposable DB `agiru_modifyall_20261005_01` is removed; original gate configuration
  is restored. Default `make gates JOBS=2` passes on `bc433db`; source hashes match
  (`default-gates-build.log`, `local-source-inputs.sha256`). Complete default-DB
  replay passes: 141 cases/232 tooling tests, exit 0; source/control hashes match
  before/after (`all-local.log`, `/tmp/agiru-record-order-controls.QWFUBk`). It starts
  after frozen 223110's local run passes, without shared database overlap.
  Full AL on frozen 223110 is terminal and excludes this caller fix: 2,167/2,314
  passed, 147 failed, zero incomplete; slice/build/local tests pass. Against 210945:
  three losses, zero gains/changed errors/missing/added/duplicates; source manifests
  and file hashes match. Original TestRenamePostedInvoice (134396) and
  TestRenamePostedCrMemo (134397/134416) report roughly tripled total tax amounts.
  Investigate original iteration/write paths and replay the caller fix without
  waiving these regressions. Frozen source/System/notice pre/post hashes match.
- Expanded dynamic writes: 5,013 checks green retain every original case and add
  uniform/mixed/global-reversed orders, already-open backward cursors and own
  Modify/Delete/Rename through typed/RecordRef. Own writes preserve the frame and
  SystemId; Rename resumes from its new key, not the old buffered ordinal.
  Independent fixture ordering uses numeric groups/code identities, not RecordOrder.
  Twenty-one compiled controls reject; final analysis has no own findings and 35
  existing header findings, no suppressions. Disposable DB
  `agiru_dynamic_order_20261004_01` prevents overlap with frozen gate/AL databases.
  The disposable DB is removed and the original gate configuration restored;
  replay on the default database retains 5,013 green checks (`default-gate.log`).
  `/tmp/agiru-dynamic-order.1x5qFL`, `/tmp/agiru-record-order-controls.rNkiaB`.
  Outside the 141-case local replay and frozen 210945; full AL replay remains pending.
- Dynamic SQL reads: private `RecordChanges.{h,cpp}` tracks only active readers,
  with session-owned table revisions and O(1) read checks. Successful Storage row
  writes and RuntimeDeleteAll advance the matching revision; failed, temporary,
  unrelated-table and foreign-connection writes do not. RuntimeNext reuses the
  bounded keyset primitive, never stale buffers. Shared ownership permits token
  teardown after its session; the final reader retires the table counter.
  DynamicRecordGate: 2,335 green across typed/RecordRef Modify/Insert/Delete/Rename,
  ModifyAll/DeleteAll, filter admission/exclusion, fetch boundaries, isolation and
  1,000 retired table visits. Initial business matrix before implementation:
  320 checks/48 red; not an identical replay of the expanded final population.
  `make record-order`: 5,872/296/313/2,335 green; twenty-one controls reject,
  including each write hook, revision checks, table/connection scope and retirement.
  Final analysis has no new findings; RecordChanges/Navigate/Storage/gate retain
  25/30/38/34 existing findings, no suppression increase. Storage's four existing
  source findings and Selection's declaration mismatch remain included.
  `/tmp/agiru-dynamic-record.AeNaPT`, `/tmp/agiru-record-order-controls.fEDCql`.
  Full local replay on `ccdd9a7`: 141 cases/232 tooling tests green, exit 0;
  changed inputs match before/after (`all-local.log`, `verified-inputs.sha256`).
  Outside frozen 210945; full AL replay remains open. Order/own-frame continuation
  is qualified above. No BC throughput/resource parity or UT gain claimed.
- Cursor transactions: `Cursor::Current` checks session/connection/epoch before
  RuntimeNext uses a buffer. One owned OpenSelection primitive resumes either
  direction with the shared keyset order and bounded NO SCROLL fetching, not
  per-row SQL. Cleanup queries `pg_catalog.pg_cursors` after generation changes;
  surviving portals close, absent portals do not poison transactions. Remove the
  incorrect depth stamp, which leaked cursors after savepoint release.
  CursorLifecycleGate: 313 checks green across typed/RecordRef, Commit/rollback,
  fresh buffered values, block boundaries, partial/exhausted reversal and cleanup.
  The final identical fixture against coherent original headers/library has 14
  failures (209 checks; exceptions prevent later checks), not a mixed-ABI replay.
  `make record-order JOBS=2`: 5,872/296/313 ordering/selection/lifecycle checks
  green; thirteen compiled controls reject, including a functional-green one-row
  fetch whose SQL count exceeds the bound. All 24 traced walks retain their cases:
  64 steps use three statements; 129 requested steps use five. This is a native
  micro-workload, not BC throughput/resource parity. Final targeted analysis has
  no own findings; gate/Cursor/Navigate retain 33/25/30 inherited findings, no
  suppressions. Affected NextZero/Cursor/Find/RecordRef/Query/Temporary/FilterGroup
  gates pass 112/230/43/89/21/80/137 checks. `/tmp/agiru-cursor-lifecycle.p37EWy`,
  `/tmp/agiru-record-order-controls.YfDQHq`; the prior read-only red probe stays
  under `/tmp/agiru-selection-change.li6S1N/cursor-epoch-*`.
  Full local/AL replay `20261004T210945Z-2749818` is terminal: HEAD `d1b4873`,
  source `92a6ba7025e7341a691de8da842698ccb9832d86f3665e6404a84f463cc7901f`,
  six jobs, slice-check/all/test/ut; dependency identities match completed 193025.
  Slice/build/local tests pass (140 cases/232 tooling); UT exits 2 at 22:29:37 UTC:
  2,170/2,314 pass, 144 failed, zero incomplete, 80 codeunits/1,492 seconds.
  All identities remain; one gain, zero losses/duplicates, matching manifests/files.
  Original SCM Available to Pick UT multi-item summary now passes; its previous
  item-tracking refusal is gone. The remaining changed diagnostic only advances
  generated item/location IDs, not the failed tracking quantity.
  `artifacts/ut-comparison-193025.json`; frozen source/System/notice hashes match.
  Null/unsealed seed is diagnostic, not causal A/B or G1. Dynamic writes are outside
  this snapshot and still require full replay.
- Selection changes now invalidate SQL cursors and lazily rebuild temporary views
  through `SelectionChanged`: filters/copies, keys/directions/views and active marks.
  Same-cardinality mark replacement is detected; unchanged predicates/directions and
  inactive marks retain the cursor. Temporary Modify advances the shared row version;
  excluded origins use a binary insertion anchor without skipping the first successor.
  SelectionChangeGate: 296 checks green; the coherent original runtime/headers fail
  64 of the same checks. `make record-order JOBS=2` retains 5,872 ordering checks
  and now rejects nine compiled controls. Receipts: `/tmp/agiru-selection-change.li6S1N`,
  `/tmp/agiru-record-order-controls.sdKg5i`. Clang/Linux x86_64 RecordState sizeof
  is 272 → 264 bytes, not a session/throughput benchmark. Targeted lint remains red:
  gate/RecordState/Temporary/RecordRef/Report have 33/1/31/40/44 existing findings;
  none in the new functions, no suppressions added. Affected NextZero/Cursor/Temporary/
  Find/FilterGroup/Report/TestReport gates pass 112/230/80/43/137/18/15 checks.
  Complete local replay on HEAD `bae1071`: 139 cases/232 tooling tests pass, exit 0;
  affected source hashes match before/after (`all-local.log`, `verified-inputs.sha256`).
  SQL dynamic writes and original full AL execution remain unproved; cursor
  transaction recovery is qualified above. This selection increment
  is outside frozen 193025; no UT gain or G1 claim.
- Mixed record ordering: private `src/rt/RecordOrder.{h,cpp}` compiles selected
  directions and complete primary-key tie-breakers once per comparison operation.
  SQL ORDER BY, reverse/keyset search and temporary views share the same order.
  Mixed predicates use direction-aware lexicographic prefixes and single binds;
  uniform directions retain the direct tuple predicate. No live-provider guard
  is removed. `make record-order JOBS=2`: 5,872 checks green across SQL/temporary,
  typed/RecordRef, mixed/uniform and global reverse orders, ties, filters,
  relative/combined searches and partial/exhausted/zero steps. Four compiled
  controls reject; the previous actual runtime fails 1,046 of the same checks.
  `/tmp/agiru-record-order.FQ83WP`, `/tmp/agiru-record-order-controls.mkHPht`.
  New order/gate units have no own analysis findings; 30/33 inherited header
  findings remain, no suppressions added. Navigate/Selection also have no own
  findings (30/27 inherited); Temporary retains 30 inherited findings and its
  existing TempFind complexity, reduced from 39 to 31. Selection's marked-row
  clause composition is split without changing bind/filter semantics.
  DiscoveryGate passes, including the
  new qualifier's missing-script control. Full local replay passes: 138 cases/
  232 tooling tests, exit 0 (`all-local.log`); source-input hashes match before/
  after. AL replay `20261004T193025Z-2625193` finishes at 20:42:35 UTC on HEAD `7b7cf45`,
  source `a234a8d7b12a5df02595bc4d9376c8345f454d7566b5652dcc279dc21c499aad`;
  slice-check/build/test pass; UT is 2,169/2,314, 145 failed, zero incomplete,
  80 codeunits/1,481 seconds. All identities/statuses/errors match 184333, with no
  duplicates/missing/added cases; source manifests/files and frozen dependencies match.
  `artifacts/ut-comparison-184333.json`; null/unsealed seed is diagnostic, not causal A/B.
  Outside frozen `20261004T184333Z-2543205`. No UT gain, native
  differential oracle, page-presentation or performance claim.
- Shared compiled predicate: `src/rt/RecordFilter.{h,cpp}` owns parsed expressions
  and borrows immutable field declarations. Temporary Build/Count/DeleteAll/CalcSum
  parse once per operation, not once per row; computed metadata uses the same API.
  Group intersections, cross-column OR, skipped FlowFilters and changed/destroyed
  source filter containers execute in ReflectionMetadata: 194 checks green, sixteen
  controls reject. Filter/FilterGroup/Temporary retain 125/137/80 green checks.
  `/tmp/agiru-reflection-metadata.hiyxPP`; focused Make logs
  `/tmp/agiru-compiled-record-filter-{scalar,groups,temporary,final-replay}.log`.
  Identical FilterGroup bodies against current/preceding frozen runtime: 25 versus
  97 ParseFilter calls, both 137/137 green. Instrumentation/source/image receipts:
  `/tmp/agiru-record-filter-parsing.EklR47/`; not a throughput/resource benchmark.
  Targeted analysis: no new source findings; 30 inherited header findings remain,
  plus Temporary's existing TempFind complexity 39. No suppressions added.
  Complete `make test JOBS=2`: 132 cases/223 tooling tests green, exit 0;
  `/tmp/agiru-compiled-record-filter-all-local.log` and parsing receipt
  `local-verification.json`. Outside frozen 120802; live providers remain guarded.
- Installed metadata lookup: `TableMetadata.cpp::InstalledTableMetadata` uses
  the existing sorted catalogue and projects only the requested original ID.
  Missing IDs return no row; missing owners/native default authority still refuse.
  Production-generated table-keys replay: 61 checks green; missing-entry and
  wrong-ID runtime controls reject, with existing key/owner controls retained.
  `/tmp/agiru-table-keys.rUwABv`, `/tmp/agiru-installed-metadata-focused.log`.
  Runner analysis passes; runtime retains 30 inherited public-header findings,
  zero own findings. No second registry, live Record provider or UT gain; outside
  frozen `20261004T120802Z-2065848`.
- Frozen SetCurrentKey integration `20261004T023609Z-1410666` finishes at
  04:01:07 UTC: slice/build/local tests pass; UT is 2,161/2,314, 153 failed,
  zero incomplete. All 2,314 unique identities remain, zero gains/losses.
  Four SCM - Planning UT methods change from wrong quantity to missing Inventory
  Profile entry 3: ErrorOpenWorksheetOnRequisitionLine, OpenPlanningWorksheetOnRequisitionLine,
  OpenReqWorksheetOnRequisitionLine and VSTF325404.
  `/tmp/agiru-current-key-ut-comparison.json`. Trace the changed temporary-key/
  cursor path before calling this behavioural improvement; no quantity repair proved.
- Shared `RecordState.cpp::SetCurrentKey` now accepts sortable unindexed fields,
  selects the first active prefix's complete fields and ignores IncludedFields.
  Typed Record and RecordRef.SetView use this primitive; successful selection closes
  the old SQL cursor, while rejected selections preserve the previous key.
  CurrentKeyGate: 54 checks green, with actual SQL/temporary tied-row ordering,
  original native All Profile, disabled/included keys and invalid/unsortable fields.
  FindGate: 43 checks green; its prefix expectation now follows the full source key.
  Receipts: `/tmp/agiru-current-key-{sql,find}-gate.log`.
  Generated AL consumption/statement proof: 48 checks, six call-context mutants,
  three shared key-selection mutants and one source-expression mutant reject
  (`/tmp/agiru-current-key-local-tests.log`, `/tmp/agiru-test-contexts.VFwyFa`).
  The AL fixture's field 4 Sort Order is genuinely unindexed; its implicit PK is
  field 1 Value. Complete local replay: 124 cases/223 tooling tests green.
  Runner targeted analysis passes; new selection/view/call helpers and gate have
  no own findings. RecordState retains one inherited header finding, CurrentKeyGate
  33 inherited findings and BodyWriter 22 existing findings; no suppression increase.
  Full regeneration `/tmp/agiru-transpile.cvtGhD` retains 80/2,314 UT and
  14,225 slice inputs, zero missing. Eight generated files change only consumed
  SetCurrentKey calls; zero added/missing paths. Original package pre/post identities
  match. Translation remains exit 1 for counted platform/AL gaps, not a G1 pass.
  Receipts: `/tmp/agiru-current-key-{generated-changes.patch,ut-identity-comparison.json}`
  and `/tmp/agiru-current-key-{RecordState,Runner}-final-findings.log`.
  FlowField sorting still explicitly refuses; sortable FlowFields, disabled-key
  CurrentKeyIndex identity and failed whole-view atomicity remain unproved. This
  development fix is not in frozen integration `20261004T011421Z-1315211`.
- Shared declaration emitter: `TableWriter.cpp::TableDeclarationProperties`
  serves ordinary TableDef initialization and native source qualification.
  Explicit Boolean, table kind, scalar properties and page IDs share validation;
  native caption arrays use original field numbers/order/repetitions with static
  lifetime. Invalid values, missing fields and unresolved page references refuse.
  GenNativeBinding 128, GenTable 78 and source replay 46 checks green; previous
  frozen compiler fails 25 new declaration checks. Receipts:
  `/tmp/agiru-shared-table-properties-{gate,source-replay,old-compiler-control}.log`.
  Original-package replay `/tmp/agiru-native-bindings.IJI5DF` retains 234 raw/
  233 selected tables: 18 qualification candidates compile, 221 runtime checks,
  215 selected unbound remain red. This receipt precedes only the unused-array
  include optimization. Full generation retains 18,998 objects and identical
  codeunit files; 274 declarations change, no file paths disappear or appear
  (`/tmp/agiru-shared-table-properties-generated-diff.log`; empty directory
  differences are not missing files). Independent source census retains 80/2,314
  identities. Frozen 011421 build/local replay passes 123 cases/223 tooling tests;
  terminal UT is 2,161/2,314, 153 failed, zero incomplete. No pass/identity changes;
  one Incoming Document diagnostic advances to an unopened TestPage (0061/0030).
  Source replay lint passes; TableWriter retains six existing source and inherited
  header findings. The new emitter has no reported findings; no suppressions added.
- Development Table Metadata projection: `src/rt/TableMetadata.{h,cpp}` projects
  the 23 declared source fields with native option identities and a validated
  original module GUID. Caption fields retain numeric IDs/order/repetitions; an
  absent declaration stays empty so the AL caller chooses its primary-key fallback.
  Missing fields/owners/properties, invalid GUIDs and CDS-as-Query refuse. No copied
  catalogue/session state. ReflectionMetadata 165 checks, eleven meaningful
  controls green (`/tmp/agiru-table-metadata-projection-controls.log`). Actual
  production transpiler/definition/primitive replay: 46 checks, retained key/owner/
  classification controls reject (`/tmp/agiru-table-keys.AGKBzR`).
  The live-provider guard remains: absent classification/default authority,
  implicit-field semantics, deployed/schema qualification and shared read-only
  filtering/count/navigation/write refusal are not proved. This projection is not
  a live provider or UT gain, and is not in frozen run `20261003T233302Z-1193251`.
  Targeted analysis exposes 30 existing public-header findings in RecordState,
  Char/StringValue, Duration and Variant; the new source's include findings were
  removed without suppressions. Receipt:
  `/tmp/agiru-table-metadata-projection-header-findings.log`.
- Default-authority counterexample: original 29.0.54011.55407 platform Types
  `MetaTable::.ctor` (RVA 8aa54) defaults `isDataPerCompany` to false, unlike the
  documented AL default true. Ncl LoadMetadata/AssignFromMetaTable copy compiled
  properties; CLR constructor ordinals/defaults are not an AL omission oracle.
  Retain `/tmp/agiru-table-metadata-authority.gCa1Do/provenance.json` and its IL
  receipts. Ordinary AL omission now has compiler-18/Runtime-18 authority:
  CustomerContent, Public and Unspecified are emitted for both OnPrem/Cloud targets;
  documented No and XML creation's Personalization→Cloud complete the projection.
  Source omissions remain immutable. Native omissions still refuse: this authored
  extension probe is not native creation authority. ReflectionMetadata 181 checks
  and thirteen compiled controls pass (`/tmp/agiru-metadata-defaults-local.log`).
  Production-transpiler replay: 52 checks and existing ownership/key controls pass
  (`/tmp/agiru-metadata-defaults-generated.log`). Runner analysis passes; runtime
  and gate retain 30 inherited header findings each and no own findings;
  the gate's two existing initializer findings are repaired without suppression. Complete
  `make test JOBS=2`: 126 cases/223 tooling tests pass
  (`/tmp/agiru-metadata-defaults-all-local.log`); source/tool/image identities:
  `/tmp/agiru-metadata-defaults-receipt.json`. This batch is not in frozen
  `20261004T044804Z-1552154`.
  Oracle `/tmp/agiru-metadata-defaults.Y411KR/compiled-metadata.json` retains the
  explicit/temporary controls and target-independent properties; compiler 17 refuses
  original System Runtime 18, and an extension's Scope=OnPrem fails AL0850.
  Keep demo 28.4, artifact 29.0 and original System 29.0.55365.0 separate.
- Native source qualification is implemented in the existing frozen catalogue:
  exact ABI/source pairs yield one canonical declaration; conflicting compositions
  refuse. `TableDefinition.h` caches immutable metadata once per native type; typed
  Record and RecordRef use the same definition. Runtime targeted analysis and the
  narrow-header negative control pass. Final original package proof
  `/tmp/agiru-native-bindings.WvH4lc`: all 18 emitted qualification
  candidates compile; 221 DSO/record checks green, dropped-library/wrong-namespace
  controls red. All 234 raw native tables remain: 233 selected (18 pass/215 unbound),
  one canonical commercial source exclusion; the unused native licence adapter and
  generator binding are removed (0725). No licence repair, live-provider activation, complete
  field/property/default coverage or current full-app/UT claim; 0725 owns mixed
  commercial caller separation. Production generation retains all 80/2,314 UT;
  qualifications are emitted into apps/platform. Latest local replay: 123 cases/223
  tooling green (`/tmp/agiru-native-product-scope-local-tests.log`). Prior local header/pointer replay: 123 cases/219 tooling green
  (`/tmp/agiru-native-source-local-tests.log`); concurrent native first reads and all
  five controls pass (`/tmp/agiru-native-source-final-catalogue.log`). Full catalogue
  freeze UT replay `20261003T213424Z-1064496` remains 2,161/2,314 with no
  identity/status/diagnostic changes; it does not contain native qualification.
- Property/native-owner batch: `ReflectionMetadata` resolves all 18 original native
  property members by their verified names/ordinals, case-insensitively; documented
  legacy Scope aliases normalize to Cloud/OnPrem. Missing/unknown properties,
  moved obsolete states and field-only access values refuse, without invented defaults.
  ReflectionMetadataGate: 120 checks/zero red; seven mapping/guard mutants reject
  (`/tmp/agiru-reflection-metadata.l6YAW5`). Table-only native packages now validate
  a present original NavxManifest and retain its owner for field takeovers; malformed,
  DTD and symlink manifests refuse. Raw manifest-absent fixtures stay unqualified.
  NativeSourceCompilerGate: 18 tests green; preceding frozen compiler refuses the
  legal native takeover (`/tmp/agiru-native-table-owner-source-tests-final.log`).
  Complete local replay: 121 cases/219 tooling tests green
  (`/tmp/agiru-metadata-property-native-owner-alias-final-tests.log`). Regeneration
  `/tmp/agiru-transpile.6k2ojo` retains 80/2,314 UT and 14,225 slice inputs/zero missing;
  generated bytes match the completed table-owner snapshot exactly. Native TableDef
  owners, absent-property semantics, immutable installed composition and the live
  provider remain open. Full frozen replay `20261003T205744Z-997579` retains
  2,161/2,314 passed, zero gains/losses/missing/diagnostic changes, matching
  source/package/notice hashes (`/tmp/agiru-property-native-owner-ut-comparison.json`).
  The later catalogue-freeze batch is not included in that UT replay.
- Table-owner batch: `AppManifest.cpp` uses private system yyjson to read the root
  identity, not a dependency's first `id`; validates required strings/GUIDs and
  duplicate identity keys. Native XML manifests share GUID validation. Table/field
  merging and immutable table declarations use the configured compilation-unit
  manifest; unmanifested groups use a bounded source manifest. Missing owners stay
  null; symlink manifests refuse. Original Business Foundation AuditCodes is a
  counterexample to unconditional nearest-manifest ownership (0033).
  GenTableGate 78/zero red; generated table-keys 36/zero red. Root/component,
  nested/shared/missing owners, decoded Unicode and moved-field takeovers execute;
  wrong owners, duplicate IDs, missing/root-vs-dependency identities, symlinks and
  wrong takeovers refuse. The previous frozen compiler refuses the legal grouped
  takeover (`/tmp/agiru-table-owner-composition.log`, `/tmp/agiru-table-keys.4mIYcy`).
  Full regeneration `/tmp/agiru-transpile.uxlEhy` preserves 80/2,314 UT and
  14,225 slice inputs/zero missing; six source-module headers and 63 table-definition
  changes versus the preceding frozen image (`/tmp/agiru-table-owner-generated-diff.log`).
  Native declaration ownership, installed composition/cross-app collisions and the
  live-provider guard remain open. Full local replay: 121 cases/215 tooling tests
  green (`/tmp/agiru-table-owner-composition-local-tests.log`). AppManifest,
  NativeManifest and generated consumer targeted analysis is green; inherited
  Main/GenTable findings remain, without a baseline increase. Full replay
  `20261003T200855Z-894902`: 2,161/2,314 passed, zero gains/losses/missing cases
  or diagnostic changes (`/tmp/agiru-table-owner-ut-comparison.json`).
- Latest frozen UT `20261004T064830Z-1661382`: 48 explicit Table Metadata
  provider refusals across twelve codeunits; 2,162/2,314, 152 failed, zero incomplete.
  Page navigation gains one case without losses; three more Incoming Document
  paths now reach this provider boundary. This is a counted blocker population,
  not a prediction that implementing the provider will pass all 48 methods.
  Original System-29 TableMetadata declares 23 fields plus
  implicit system fields. The live-provider guard remains; no UT gain claimed.
- Declaration batch `2458f19`: generated `TableDef` retains available source app identity, original AL
  namespace, Scope, ObsoleteReason, table DataClassification and LinkedObject.
  `meta/ModuleDef.h` shares immutable app declarations without AL state/includes.
  Missing owners/properties are explicit null/empty, not fabricated defaults.
  Bound native definitions still need source-owned declaration metadata;
  deployment target is not Scope and extensions must not replace the base owner.
  Live projection/filtering/read-only execution remain unimplemented.
  GenTableGate 64/zero red; actual generated table-keys fixture 26/zero red,
  four wrong-key/clustering/owner/classification controls reject. LinkedObject
  storage remains refused. Receipts: `/tmp/agiru-table-metadata-commit-controls.log`
  and `/tmp/agiru-table-metadata-gen-final.log`. The subsequent generated slice
  rebuild and full UT replay include this declaration batch; no metadata UT gain.
  Complete local replay: 120 cases/215 tooling tests green, zero red/skips
  (`/tmp/agiru-table-metadata-commit-tests.log`).
- Predecessor findings 1017/1080/1417: direct-ID lookup from the shared registry,
  Name separate from Caption, and typed table-kind mapping. Its
  `runtime/base/virtual_metadata.py` has only nine fields, different source field
  numbers and unverified option ordinals; do not copy it as the System-29 contract.
  Extend `src/gen/TableWriter.cpp::TableDefinitions` and the shared declaration
  metadata, using `src/tc/Main.cpp::EmitModule`/`NativeManifest` for real owners.
  Keep immutable definitions shared; direct Get must not materialize every table.
  Reuse bounded filtering/navigation/count primitives, reject persistence, and prove
  all source properties/owners plus unsupported mappings before removing the guard.

- Integrated one FilterGroup/HasFilter primitive below typed Record and RecordRef. Getters do not allocate/reset state; >255 is ignored; HasFilter reads only the selected group instead of all groups/refusing. Main FilterGroupGate: 137 green; getter-reset/unbounded-setter/group-blind controls: 21/34/9 red. Two generated AL alias pages execute free-group search and preserve independent filters. All 139 no-PCH consumers retain results (135 green/four red); 140 Python identities and every old C++ count/status retained, same 26 DB failures. Receipts: build/filter-group-integration-20261002/artifacts/{main-proof,stable-controls,consumers,stable-targeted-lint,proof}.json. SQL/full UT remain unproved.
- Consumed setter return remains unproved: current platform examples describe the new group, while BCApps WorkflowResponseFactBox.OnFindRecord saves/restores the returned group. Existing prior-group return is unchanged; neither example nor source usage is a BC runtime oracle. Group -1 FlowField behaviour also needs its own refusal/SQL proof.
- Main's generic RequireTableProvider boundary now refuses unavailable Page Table Field reads/navigation/SQL writes before session access or record stamping. Schema provisioning reports and skips it rather than creating an empty physical replacement. Explicit temporary storage remains independent. This is declaration/refusal proof, not a live provider; receipts are in README.
- Page/Table Metadata now use the same named live-provider/schema guard. Partial physical seeding is removed without deleting existing data. `ReflectionMetadataGate` retains 59 projection/refusal/temporary checks; ordinal-cast, CDS→Query, unknown→Normal and removed-guard controls reject. `src/rt/ReflectionMetadata.{h,cpp}` maps verified property identities, not ordinal casts; absent kinds remain refusals. This is not live projection or SQL/provider acceptance.
- Integrated unfiltered `GetRangeMin/Max` refusal in the common `Filter.cpp::RangeBoundText`. RangeBoundGate: 32/32 Clang/GCC; old runtime: 20 red. Covers regular/temporary records, borrowed/owned FieldRef, cleared/unrelated filters, exact Decimal scale and explicit blank equality. FilterGate 122/122, RecordRefGate 89/89; 84 local cases green. Direct includes repaired; targeted analysis has only inherited-header findings, with no new gate findings. Frozen full UT retains all 2,310 identities with no status/diagnostic changes; diagnostic legacy-seed evidence, not causal A/B (README). Logs: `/home/cosmo/Git/agiru-worktrees/goal-20260928/build/range-bound-*.log`. Internal empty-bound parsing is unchanged; local docs do not supply exact BC diagnostic wording.

- Integrated Next(0) preserves SQL/temporary typed Record and RecordRef positions and pending field values; omitted Steps remains one. `NextZeroGate`: Clang 19 and GCC 14 each 32 checks green; unchanged runtime negative control: 16 red. Logs: `build/review-20260928/next-zero-{negative,positive}.log`.
- Integrated partial/exhausted Next preserves the last reached row and permits reversal; INT_MIN uses a widened magnitude. Typed SQL/temporary and RecordRef paths, including multiple fetch blocks: `NextZeroGate` 112 checks green under Clang 19/GCC 14; frozen pre-fix runtime 44 red. `CursorGate` 217 checks green under both compilers, including exact full-block exhaustion; retains one bounded block, not the full set. Complete local test: 80 cases, 0 red. Logs: `build/review-20260928/next-partial-{final-gates,gcc,frozen-negative}.log`, `cursor-endpoint-gcc.log`. Current frozen verification includes the patch; full UT identities/statuses/diagnostics match September 28 (README), without sealed-seed causal proof.
- Targeted analysis of Navigate/Temporary/Cursor still refuses existing public-header findings and TempFind complexity; no finding in the changed functions, no suppression or baseline increase. Logs: `build/review-20260928/next-{Navigate,Temporary,Cursor}-tidy.log`.
- SQL keyset comparison builds a uniform tuple comparison even when selected key fields descend. SQL/temporary/RecordRef need one operation matrix.
- `ChangeCompany` refuses; SQL names use only TableDef.name. CompanyName alone does not isolate company data.
- `Selection.cpp::Series` silently caps the virtual Integer population at 1,000,000 admitted rows. The documented domain is ±1,000,000,000; a fetch bound must not change Count or omit records.

## Implementation

0. Replay the four changed SCM - Planning UT methods from the frozen receipt;
   pinpoint the first missing Inventory Profile Get and preceding temporary-row
   key/cursor operations. Original source:
   `~/Git/BCApps/src/Layers/W1/Tests/SCM-Planning/SCMPlanningUT.Codeunit.al`.
   Preserve documented complete-prefix selection; do not restore ignored keys
   just to recover old diagnostics. Compare the full population after any fix.
1. Replay the expanded dynamic-write matrix above through the full AL population.
   Keep bounded reads
   and measure write-loop overhead, including ModifyAll's current per-row traversal.
   Qualify sort-field changes, trigger globals and subscribed events before introducing
   a set-based fast path; caller-preserving traversal alone is not that contract.
   Qualify Query.cpp's transaction-boundary
   contract separately: Query.Read does not use RuntimeNext. Route table identity
   through explicit company context; preserve lifecycle/mixed-order/step gates.
   Reject predecessor 0889's unsupported claim that a PostgreSQL snapshot proves
   BC own-insert blindness; platform documentation requires dynamic sets.
2. Write a small operation matrix over typed Record, RecordRef and temporary records: Init versus Clear, assignment versus Copy, Copy(ShareTable), Get versus filters, Find directions, marks and ModifyAll/DeleteAll triggers.
   Extend the retained FilterGroupGate matrix to SQL and full sealed-seed UT A/B. Verify consumed setter return against the platform rather than assuming the example proves it; prove group -1 FlowField refusal. Keep independent per-group filters, same-field intersection, cross-column OR and every former item-tracking identity.
3. Centralize primitives below the typed wrappers while retaining typed field access. Preserve table variables and temporary ownership according to operation, not a general C++ copy rule.
   Shared `FieldRef.Length`/`Field.Len` primitive is implemented in
   `src/rt/{RecordRef.cpp,FieldMetadata.h,written/PlatformField.cpp}`; public
   contract: `include/runtime/RecordRef.h`. Declared-only `FieldDef.length`,
   UTF-16 capacity and source assertions remain unchanged; unknown types refuse.
   Retained/expanded gates pass 98/305 checks; restored-zero control fails 7/19.
   All 4,477 original lengths match the projection (zero control: 2,754 differences).
   `field-length-implementation.json` in the native authority receipt retains
   hashes and remaining lint findings. Rebuild all generated consumers and compare
   unchanged full local/AL populations; no new UT result yet. Qualified original 4,477-field,
   separately authored DateFormula and current three-error receipts:
   `/tmp/agiru-table-provider-authority.CyvRCA/native-field-length-contract.json`;
   developer `ff5939a46e`, `methods-auto/fieldref/fieldref-length-method.md`, BCApps
   `bb7111877f`, `TestLibraries/LibraryTablesUT.Codeunit.al` and
   `ApplicationTestLibrary/LibraryUtility.Codeunit.al`. Predecessor docaudit cites
   the guarantee but `_recordref.py::_field_declared_length` still returns zero;
   do not inherit that shortcut.
4. Extend documented Boolean result versus statement-raises behaviour; preserve database faults. SetCurrentKey's shared selection/consumption fix is implemented above. Classify sortable/unsortable FlowFields from their actual formulas/table domains; implement supported SQL/temporary ordering rather than a blanket refusal. Prove CurrentKeyIndex against disabled/duplicate key identities without guessing filtered index numbering. 0061 owns consumed/discarded lowering.
5. Complete computed platform tables from system symbols and requested ranges, avoiding fixed-date population as the authoritative implementation.
   Qualify each provider's creation path against 0013's original virtual-buffer
   evidence: virtual timestamp 1, conditional audit defaults and supplied versus
   default SystemId. Table Metadata's keyed creation/identity path is qualified
   above; native original omitted properties and field visibility are qualified
   by `native-profile.json`; agiru's complete effective profile remains open.
   Do not substitute persisted-row SQL defaults.
   Share Page/Table Metadata, Field and AllObj live projections over immutable installed metadata. Populate every represented source property from qualified declarations/app identity, keep Name separate from Caption, and refuse missing authority rather than default values. Reuse `ReflectionMetadata` mappings; CDS is not Query. Prove typed/reflected filtering, count/navigation, permissions, read-only writes and temporary independence before removing the guards. 0013 owns explicit legacy-snapshot/schema activation.
   Native ownership: start at `Main::WriteNativeObjects`, `NativeSources::app` and
   `TableWriter::{NativeTableAssertions,TableReflectionProperties}`. Emit original
   namespace/property/module declarations with verified bound-record ABI contracts
   into the platform library. Install one canonical TableEntry per identity before
   catalogue freeze; no competing metadata registry or silent duplicate replacement.
   Preserve the implemented typed Record/RecordRef canonical binding and qualify
   the complete selected production image after 0725 retirement; no compiling-only
   subset or stale ABI consumer is integration proof. The qualified native
   effective defaults and non-system Normal company predicate are implemented;
   complete timestamp and four Runtime-18 FlowFields before enabling the live
   provider. Provider kind
   is separate from TableType; folder names are not runtime proof.
   `implicit-profile-boundaries.json` qualifies every original internal kind ×
   LinkedObject pair through Types creation: fourteen cases/replay match; ignored
   LinkedObject/universal audit controls fail two/twelve. Keep timestamp/SystemId
   for every kind; audit families require unlinked Normal/Temporary. Original
   compiler CDS and native Query both have ordinal 5 but are different contracts;
   qualify emitter conversion before replacing the existing CDS refusal.
   Ordinary NCL creation remains unexecuted after a dependency/finalizer refusal;
   do not promote the Types declaration matrix to live business proof.
   Validate table Scope eligibility at source admission: the reference compiler
   rejects an ordinary extension's Scope=OnPrem (AL0850), irrespective of its
   OnPrem deployment target. Preserve permitted original platform declarations;
   property-name projection alone does not implement this restriction.
   Types-29 `MetaTable::AppendSystemFields` (RVA 8be38) inserts field 0
   SystemRowVersion and uses linked/table-kind-dependent audit fields; retained
   `/tmp/agiru-metadata-defaults.Y411KR/table-system-fields.il` is creation evidence,
   not native row-value authority. Preserve 0013/0034's complete version-profile gaps.
   Page Table Field: project the source-declared field/type/length/caption/kind/scope metadata for installed pages and table extensions. Share filter/order/count/navigation with typed Record and RecordRef; preserve pending obsolete fields, exact native codes and permissions. Replace the named refusal only with a tested read-only provider, not seeded placeholders. Temporary writes remain legal; virtual writes refuse. Source classification and generated binding remain 0034/0033.
   Keep the Integer domain intact; bound transfer through cursors, not a truncated relation. Gate filtered Count and navigation across the former million-row cutoff.
   For Field, use `src/rt/FieldMetadata.h` over immutable installed declarations for typed/RecordRef Get, Find and Next. Keep only cursor indices and existing record filters per handle, not a copied catalogue per session. One predicate serves Count/navigation; live virtual writes refuse, temporary writes remain legal. Remove legacy SQL Field copies only on disposable/schema-qualified databases (0004). 0034 owns schema/value contracts, not a second navigation implementation.

## Acceptance

- FilterGroup SQL paths retain the gate's repeated getter, groups -1/0/2/255/256, current-group HasFilter, independent handles, copy/reset and same-field/cross-column intersections. Keep all three negative controls. Full UT A/B retains the same population; no former item-tracking loss is ignored. Consumed setter return needs platform runtime evidence.
- The same fixture yields identical keys, field values, filters and events through all three paths. Include no-match, duplicate, negative Next, absent key and malformed typed key controls. Handle lifetime tests belong to 0718.
- Unindexed sortable fields return true and sort correctly; active prefix selection uses the first full key. Disabled keys, IncludedFields and unsortable fields have separate controls. Genuine failure returns false in value context and raises in statement context; keep both checks in the retained unindexed probe.

## References

Mixed record order: developer `ff5939a46e05`,
`methods-auto/record/record-{setascending,ascending,find}-method.md`;
BCApps `bb7111877ff7`, `System Application/App/Table Information/src/TableInformationCacheImpl.Codeunit.al::SetBiggestTablesFilter`;
user `0ff62b2266fd`, `archive/WorkingWithDynamics/sorting.md`.
Predecessor board search found no SetAscending/mixed-direction finding; 1102
separately covers cursor invalidation. Record-search order follows the documented
current-key path with primary ties; SetAscending is not a client-page sort policy.

Compiled predicates: developer `ff5939a46e05`, `methods-auto/record/record-{filtergroup,count}-method.md`
and `devenv-flowfilter-overview.md`; BCApps `bb7111877ff7`,
`Foundation/Reporting/CustomLayoutReporting.Codeunit.al::{SetGroupFilter,FindNextEmptyFilterGroup}`;
user `0ff62b2266fd`, `business-central/ui-enter-criteria-filters.md`.
Predecessor 1063/1103 require every reader to retain group intersections; do not
replace grouped source filters with a field-keyed dictionary or parallel state.

Filter groups: include/runtime/{Record,RecordState,Table,RecordRef}.h, src/rt/{RecordRef,Temporary}.cpp, test/gate/FilterGroupGate.cpp and test/transpiler/page-record-binding/. Platform methods-auto/{record,recordref}/*-{filtergroup,hasfilter}-method.md and record/record-getfilter-method.md; current official FilterGroup pages checked 2026-10-02. BCApps main a9ea4d84534cebba852c44bf0f841c2ea149de4e, src/Layers/W1/BaseApp/Foundation/Reporting/CustomLayoutReporting.Codeunit.al::{FindNextEmptyFilterGroup,SetGroupFilter,GetNextGroupFilters}; System/Workflow/WorkflowResponseFactBox.Page.al::OnFindRecord. User business-central/ui-enter-criteria-filters.md. Earlier 1063/1103 expose shared-group storage and five item-tracking regressions; preserve independent filters and investigate every loss, not the predecessor's one-dictionary compromise. Generator property syntax is 0073; runtime remains this WI.

Sorting authority: developer `ff5939a46e05`,
`methods-auto/record/record-setcurrentkey-method.md`,
`methods-auto/{record,recordref}/*-setview-method.md`,
`methods-auto/recordref/recordref-{currentkeyindex,keycount}-method.md` and
`properties/devenv-includedfields-property.md`. BCApps `bb7111877ff7`,
`System Application/App/User Settings/src/UserSettingsImpl.Codeunit.al::PopulateProfiles`;
original System `Virtual Tables/AllProfile.Table.al` has only Scope/App ID/Profile ID PK.
User intent: `business-central/ui-enter-criteria-filters.md`. Predecessor 1464 separates
optional Boolean contracts; its claim that SetCurrentKey never fails is rejected.
RecordRef has no SetCurrentKey method; its SetView shares the primitive. No filtered
disabled-key index numbering or Python call-context maps are inferred.

Range bounds: platform `methods-auto/{record,fieldref}/*-getrangemin-method.md` and `*-getrangemax-method.md`; BCApps main `a9ea4d84534cebba852c44bf0f841c2ea149de4e`, `src/System Application/App/Email/src/Email/Sent/SentEmails.Query.al` and `src/System Application/App/Extension Management/src/ExtensionSettings.Page.al` guard calls with GetFilter. User docs contain no separate contract. Predecessor `~/Git/openerp/board/1715_getrangemin-without-filter-must-raise.md` distinguishes public errors from internal blank FlowFilter bounds. Preserve `RangeBoundOf` and the existing FlowFilter gate.

Selection-change authority: developer `ff5939a46e05`,
`methods-auto/record/record-{next,setfilter,copyfilter,copyfilters,mark,markedonly,clearmarks}-method.md`
and `methods-auto/database/database-commit-method.md`,
and `dev-itpro/administration/optimize-sql-al-Database-methods-and-performance-on-server.md`
in the same documentation repository.
BCApps `bb7111877ff7`: `Inventory/Tracking/InventoryProfileOffsetting.Codeunit.al::ForecastConsumption`
narrows, Find('+'), widens and Next; `Inventory/Counting/Document/PhysInvtShowDuplicates.Codeunit.al`
marks during an ordinary walk before activating MarkedOnly. Both under `src/Layers/W1/BaseApp/`.
User `0ff62b2266fd`, `business-central/ui-enter-criteria-filters.md`.
Predecessor `openerp/board/1102_persistentes-next-ignoriert-filteraenderungen-im-ergebnissat.md`
identifies the stale-selection failure; its Python implementation is not transplanted.

Cursor lifetime: same developer/BCApps revisions; Record/RecordRef Next and
Database Commit methods above. Original `src/Layers/W1/BaseApp/System/RapidStart/ConfigWorksheet.Page.al::GetRelatedTables`
uses FindSet → Commit → Next directly. User `0ff62b2266fd`,
`business-central/{ui-batch-posting,ui-how-run-batch-jobs}.md` requires complete batches
and visible errors, not a cursor implementation. Predecessor 1102/1485 supplies stale
selection and partial-step counterexamples. PostgreSQL 17 docs, checked 2026-10-04
(no installed local copy): [pg_cursors](https://www.postgresql.org/docs/17/view-pg-cursors.html),
[CLOSE](https://www.postgresql.org/docs/17/sql-close.html),
[DECLARE](https://www.postgresql.org/docs/17/sql-declare.html). Ordinary PostgreSQL
cursors are insensitive; their snapshots are not BC dynamic-result-set proof.

Dynamic writes: developer `ff5939a46e05`, the administration dynamic-result-set
guarantee above and `methods-auto/record/record-{modifyall,deleteall}-method.md`.
BCApps `bb7111877ff7`, `src/Layers/W1/BaseApp/Sales/Document/ItemChargeAssgntSales.Codeunit.al::AssignItemCharges`
does four ModifyAll calls before traversal. User `0ff62b2266fd`,
`business-central/{ui-how-run-batch-jobs,ui-enter-criteria-filters}.md`.
Predecessor 1573 identifies ModifyAll's buffer/position corruption; 1589 records
the documented default-global requirement. Independent workers preserve the caller
and start with default globals; native per-row/subscriber/global-reset proof remains open.
`ItemChargeAssgntSales::AssignEqually` immediately traverses from the retained first row;
0889's own-insert-blindness claim contradicts the platform dynamic-set guarantee.
Own Rename: `methods-auto/{record,recordref}/*-rename-method.md` and
BCApps `bb7111877ff7`, `src/Layers/W1/Tests/ERM/CopyPriceDataTest.Codeunit.al::T066_CopyResourceCostInconsistentData`
renames ResourceCost.Code while retaining Type/Work Type Code. The docs' prohibited
table examples do not establish a blanket refusal of every option-containing key;
retain this original source counterexample without a business-specific runtime branch.

Rename cascade: developer `ff5939a46e05`, `methods-auto/record/record-rename-method.md`
and `devenv-set-relationships-between-tables.md` explicitly require automatic updates
of related key values. BCApps `bb7111877ff7`, original
`src/Layers/W1/Tests/{ERM-Sales/ERMSalesInvoiceAggregateUT,ERM-Sales/ERMSalesCrMemoAggrUT,ERM-Purchase/ERMPurchCrMemoAggrUT}.Codeunit.al`
Rename methods and `BaseApp/Utilities/DocumentTotals.Codeunit.al` compare stored
entity totals with posted-header FlowFields over the related lines. Predecessor
1162 supplies the same three original cases; 0934's rejected interpretation that
only explicit AL triggers cascade contradicts the platform documentation.
Keep generic declared-relation propagation; repair traversal, not business code
or dynamic-read visibility. User batch/filter intent and revisions are cited above.

Partial/extreme steps: platform `methods-auto/{record,recordref}/*-next-method.md`; BCApps current main `src/Layers/W1/Tests/Cost Accounting/ERMCAGLTransfer.Codeunit.al::ValidateTransfer` and `src/Layers/W1/Tests/Dimension/DimensionCorrectionTests.Codeunit.al` use non-unit steps. User intent: `dynamics365smb-docs/archive/WorkingWithDynamics/sorting.md`. Preserve the selection/lifecycle matrix above; full SQL mutation and Query transaction contracts remain open.

Code: `src/rt/{Record,Navigate,Temporary,Selection,RecordRef,PlatformTables}.cpp`, `include/runtime/Table.h`. Platform: `methods-auto/record/record-next-method.md`, `methods-auto/recordref/recordref-next-method.md`, other Record/RecordRef overloads, devenv-temporary-tables.md, devenv-integer-virtual-table.md. AL: `src/Layers/RU/Tests/Local/ERMVATReinstatement.Codeunit.al::SuggestVATSettlement` explicitly calls temporary Next(0); No. Series temporary filters and platform table users. No dedicated user-facing Next(0) contract; platform method documentation governs. Predecessor board searched for Next(0), with no matching finding; WI-1063/1136/1173/1206/1229 cover adjacent record contracts. Retain source usage as a fixture, never a hardcoded runtime branch.

Table declaration authority: developer docs `ff5939a46e`,
`devenv-json-files.md`, `properties/devenv-{scope-table,linkedobject,obsoletereason}-property.md`
and `methods-auto/moduleinfo/moduleinfo-data-type.md`; BCApps `bb7111877f`,
`Business Foundation/App/app.json`,
`Business Foundation/App/NoSeries/src/Setup/NoSeries.Table.al`,
`Layers/W1/BaseApp/Projects/Resources/Pricing/ResourceCost.Table.al`.
Original verified System `29.0.55365.0`: `src/Virtual Tables/TableMetadata.Table.al`.
Stable metadata identity: same revisions, `devenv-{virtual-tables,table-system-fields}.md`;
BCApps `DataClassificationMgtImpl.Codeunit.al::IsSupportedTable`; original Ncl
`TableDataProvider`/`MetadataSystemId`/`VirtualDataProvider` and retained IL/layout
receipts above. The 16-byte permutation in MetadataSystemId.cpp converts the original
little-endian four-Int32 overlay to Guid's canonical byte representation. Keep
original 29.0.54011.55407, System 29.0.55365.0 and demo 28.4 bounds distinct.
Predecessor 830 warns of broad metadata activation and vacuous no-op assertions;
1040/1041 require runtime providers, not silently empty physical copies.
User docs `0ff62b2266`, `business-central/admin-classifying-data-sensitivity.md`:
developer classification is not user-maintained sensitivity. Predecessor 1017/1080/1417
are findings, not schema authority. Local `onprem/classifying-data.md` describes
2018 NAV field defaults and upgrades, not modern omitted table classification.

Source-owner composition: same developer/BCApps revisions, `devenv-json-files.md`
and `properties/devenv-{movedfrom,movedto}-property.md`; original
`Business Foundation/App/{app.json,AuditCodes/app.json,
AuditCodes/src/Legacy/ObsoleteReturnReasonExt.TableExt.al}` and
`Layers/W1/BaseApp/Inventory/Location/ReturnReasonExt.TableExt.al`.
The W1 destination explicitly names Business Foundation, not AuditCodes.

Property mappings: same developer revision, `properties/devenv-{compressiontype,
datacaptionfields,scope-table,access,obsoletestate,inherentpermissions,
inherententitlements}-property.md`; original System TableMetadata source above.
Scope aliases follow the property documentation, not enum-position casts. The
redirected classification pages supply no absent-table classification default.
Ordinary AL defaults use Microsoft Development.Tools 18.0.41.62505
(`https://www.nuget.org/packages/Microsoft.Dynamics.BusinessCentral.Development.Tools/18.0.41.62505`),
SHA256 `592eb1173af4faee59aca91b4624ccbb70c725ffc78b36719e078b63ffc6697b`,
original System 29.0.55365.0 and authored default/explicit/temporary tables with
`/generatecode+`. Runtime-18 compiled XML is identical for OnPrem/Cloud targets.
Types-29 XML constructor RVA 8ac88 leaves absent Scope at Personalization (0),
normalized to Cloud by the documented legacy alias; absent ObsoleteState is No.
The CLR optional-argument constructor is not this XML creation path.
Retain version/tool/source hashes and the compiler/XML receipts under
`/tmp/agiru-metadata-defaults.Y411KR/`; proprietary tools stay outside agiru.
BCApps references: `System Application/App/Data Classification/src/`
`DataClassificationMgtImpl.Codeunit.al::IsSupportedTable` and
`DataPrivacyEntities.Table.al`: table classification is distinct from field
classification; obsolete/type metadata controls supported business tables.

Caption-field serialization: BCApps `bb7111877f`,
`Apps/W1/DataSearch/App/DataSearchInTable.codeunit.al::{GetKeyText,SplitStringToIntegerList}`
consumes comma-separated integer field numbers; an empty declaration selects the
primary-key fallback in AL. The private projection preserves order/repetitions.
Complete the shared native/app property emission in `TableWriter.cpp`; retain
source omissions separately from documented effective defaults. Access's field-level
default and cmdlet compression defaults do not establish every table metadata default.

Property scope: `enableexternalassemblies`, `externalaccess`, `externaltype`, `initvalue`, `iscontroladdin`, `optionordinalvalues`, `provider`, `publickeytoken`, `tabletype`, `usetemporary`, `usetemporary-report`, `usetemporary-xmlport`.
