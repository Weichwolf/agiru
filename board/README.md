# Delivery board

Review: 2026-10-04. Repository text is English. Open outcomes only; no historical pass count is a current measurement.

Build cleanup (2026-10-02): handwritten sources from 79 source copies are preserved in
`archive/build-20261002/` (relative paths use `--` instead of `/`); standalone probes
are in `archive/build-20261002/probes`. Old `build/` receipts, binaries and downloaded
inputs were moved to the desktop Trash and remain recoverable; references below are
historical, not available current proof. New verification starts from an empty `build/`.
Second requested cleanup: the 14 GB rebuild directory was also moved to Trash;
its four completed verification-worktree registrations were pruned. External worktrees
and archived source branches were preserved. The current receipts below are newly created.
Preserved native prototype: branch `work/native-field-metadata`, worktree
`/home/cosmo/Git/agiru-worktrees/native-field-metadata`; not promoted to `main`.

## Current verification — 2026-10-05

- File/Native-text fixed-tree build passes (compiler inputs `8941708`, documentation
  HEAD `2a75bf1`): six-job resume exits 0 in 1,704 seconds; the preceding two-job
  build was intentionally interrupted after AL workers finished, not a compiler
  failure. Slice-check: 14,225 sources/zero missing; 1,383 unlinked replacement
  bodies remain gaps, not full-app/G1 proof. 25,267 input hashes agree after building.
  `/tmp/agiru-native-text-integration.awrO0A`; local Make tests pass: 142 cases/
  zero red, 233 tooling tests. Original Native declaration checks retain 61 green;
  twenty-three record and nine stream/File controls reject. Source/image hashes
  remain unchanged; verified System package pre/post matches, original notice retained.
  Full UT is terminal on `45f6fd0`: 2,172/2,314 pass, 142 fail, zero incomplete,
  80 codeunits/six workers/1,231 seconds; Make exits 2, runner status 1. Against
  024625, all identities/source hashes agree: two gains, no losses/missing/added/
  duplicates. Empty-doctype XML and OAuth nonce tests recover; code challenge
  advances from Native Base64 refusal to `HashAlgorithm.Create` (0035).
  `ut-comparison-024625.json`; source/image/System hashes match after execution.
  Null/unsealed seed remains diagnostic, not causal A/B/G1.
- Native Base64 transform authority (0034): original `ConvertBuffer` executes
  on CLR 10.0.12, 2,699 calls/2,696 distinct case identities. Whole-buffer Convert
  differs at 104 decode cases, zero encode cases; padded groups, unused bits,
  VT/FF, incomplete tails and chunk error effects require a separate decoder
  policy. `/tmp/agiru-native-base64-blocks.MPyne6`, original binary/source hashes
  retained. Outer I/O loop is authored; no original stream execution or UT gain.
- Native Base64 text bindings (0034): five signatures execute through the common
  emitter; four InStream signatures remain counted refusals. 166 generator and 61
  original-declaration checks green; encoding/terminated-output controls fail 6/2.
  Loader census proves five bound/four unbound and wrong-identity refusal.
  `/tmp/agiru-native-codeunits.h58edG`, hashes match. New binding/refusal/consumer
  analysis passes; 33/1 inherited primitive/gate findings and older compiler findings
  remain unsuppressed. Locale defaults, transform output above the original 10 MiB
  character threshold and original thirteen-test AL suite remain open. Full UT
  replay above recovers OAuth nonce; code challenge reaches HashAlgorithm.Create.
  Root generation
  `/tmp/agiru-transpile.woncg0` retains 35 selected native codeunits, reduces unbound
  methods 72→67 and exits 1 on remaining gaps; package pre/post hashes match. Outside 024625.
- File mode/capacity/position fix (0035): 44 C++ and 25 generated AL checks green;
  all preceding 17/17 retained. Default binary reads preserve newlines, typed
  capacity and zero-byte accounting; text mode returns content length; File.Pos
  is zero-based. Nine compiled controls reject both consumers. Stream 89,
  Base64 79, XML 79/reader 220 and Encoding 87 remain green; all gates rebuild.
  `/tmp/agiru-streams.5Y5ARv`, hashes match; `/tmp/agiru-file-mode.uH1HcI`.
  Runner analysis passes; source/gate retain 33/66 inherited header findings,
  zero own findings, lint exits 2. Empty-doctype reader/replacement/Save/File.Read
  path passes locally. Full UT replay above recovers the original empty-doctype test.
  Encoding, configured bounds, non-text reads, diagnostics and File lifecycle/
  access remain open; nonempty XML subsets are still discarded (0035/0074).
- Stream/XML replay is terminal: `20261005T024625Z-3504499`, frozen HEAD
  `9c3291e`, source
  `12f5e315a376d585ea8c20dbc95ce8dccfb068598fd88d1f82cdac33e648e588`.
  Finished 04:06:14 UTC; runner gone. Slice/build pass; local population is
  142 cases with one tooling failure: DiscoveryGate
  omitted the stream qualifier from its isolated fixture. Fix `079158d` passes its
  missing-binary/per-script removal controls locally; it is outside this snapshot.
  UT exits 2: 2,170/2,314 passed, 144 failed, zero incomplete; 80 codeunits,
  six workers/1,571 seconds. All identities/source manifest/file hashes match
  015114: no gains/losses/changed errors/missing/added/duplicates
  (`artifacts/ut-comparison-015114.json`). Dependencies and frozen source/System/
  original-notice pre/post hashes match. Includes Ignore-header, shared-cursor
  and owned-provider fixes; excludes later File and Native Base64 bindings.
  Null/unsealed seed remains diagnostic repeatability, not causal A/B or G1.
- Owned BLOB providers (0035): 89 C++/17 generated AL checks green, including
  ASan/UBSan; all preceding 73/12 checks retained. Four compiled controls fail
  both consumers, including heap-use-after-free for an unowned provider.
  `/tmp/agiru-streams.D7G8B1`, hashes match; `/tmp/agiru-stream-owner.L3GzVu`.
  All gate consumers rebuilt; Base64 79/XML 220/File 17 remain green.
  Blob/AL runner lint passes; Stream/gate retain 25/47 inherited header findings,
  no own findings, lint exits 2. Terminal 024625 retains all prior statuses/errors.
  File/record/codeunit ownership, AL assignment/Clear/disposal, encoding, bounds,
  nonseekable/BigInteger positions and Native activation remain open.
- Shared InStream cursor (0035): 73 C++ checks green, preceding runtime 16 red;
  12 production-generated AL checks green. Read/reset cursor controls fail both
  consumers (C++ 16/6 red; AL 5/3). All gate consumers rebuilt; Base64 79 and XML
  reader 220 remain green. `/tmp/agiru-stream-alias.hjKWK3`,
  `/tmp/agiru-streams.QdtX0X`, hashes match. No own analysis findings; 25/47 inherited
  source/gate header findings remain (lint exits 2); AL runner lint passes.
  Terminal 024625 retains all prior statuses/errors. Broader provider lifetimes, positions,
  encoding/line handling and Native stream activation remain open.
- XML Ignore header/closing fix (0035): 220 checks green; preceding runtime has
  53 failures on the same expanded gate. Seven compiled controls reject (header
  bypass: 79 failures); fixture-resource tripwire remains qualified. Input hashes
  match: `/tmp/agiru-xml-header.1XX8t9`, `/tmp/agiru-xml-reader.PUZSGA`.
  No own analysis findings, 36/62 inherited header findings remain (lint exits 2).
  Terminal 024625 retains all prior statuses/errors. QName/character validation, other
  encodings, whitespace/error states and Parse/resolver authority remain open.
- XML integration is terminal: `20261005T015114Z-3424473`, frozen HEAD `3f6715e`,
  source `5868281d053a1fc2e797530f440b93f85a8ed23d9afb6c586225de0c19c0e193`;
  finished 02:33:56 UTC, runner gone. Slice/build/local tests pass (141 cases/232
  tooling tests); UT exits 2: 2,170/2,314 passed, 144 failed, zero incomplete,
  80 codeunits/1,354 seconds. Against 004617: no gains/losses/changed errors,
  missing/added/duplicate identities; source manifest/file hashes match
  (`artifacts/ut-comparison-004617.json`). BC/source/System/original-notice hashes
  match; frozen source/System/notice pre/post hashes agree. Null/unsealed seed
  remains diagnostic, not causal A/B or G1. Later Ignore-header/stream changes
  are outside this snapshot; investigate all remaining 144 failures.
- First XML DTD-policy increment (0035): 134 XmlReaderGate checks green; unfiltered
  compiled control has 33 failures. Six policy/cursor/close/Load controls reject;
  the test-only loader tripwire sees no fixture request with policy and is triggered
  by unfiltered input. `/tmp/agiru-xml-reader.3UuLuu`, input hashes match.
  No own analysis findings; 36/62 reader/gate header findings remain, lint exits 2;
  the resource probe analysis passes. Complete local/AL replay is pending, outside
  frozen 004617. Parse/resolver authority, full encoding/Ignore/error-state contracts
  and streaming bounds remain open; no blanket XML safety or all-green-tree claim.
- Rename-fix integration: `20261005T004617Z-3255614`, frozen HEAD `24af1d8`,
  source `f484500aa41dfb2037f5267c3818bebf5227cd7e39656799382d02a3a47deb7a`;
  `slice-check all test ut`, six jobs; terminal at 01:26:10 UTC, runner gone.
  Includes both ModifyAll caller preservation and the Rename cascade read anchor.
  Slice/build/local tests pass (141 cases/232 tooling tests); UT exits 2:
  2,170/2,314 passed, 144 failed, zero incomplete, 80 codeunits/1,330 seconds.
  Against 232014: all three original invoice/credit-memo Rename tests recover;
  no losses/changed errors/missing/added/duplicates, source manifest/file hashes
  match (`artifacts/ut-comparison-232014.json`). Dependencies match 232014;
  frozen source/System/original-notice pre/post hashes match. XML policy changes
  are outside this snapshot. Null/unsealed seed remains diagnostic, not causal A/B
  or G1; investigate the remaining 144 failures.
- Rename cascade regression (0044): the reader must retain the old key while an
  independent record writes the related row. RenameGate retains its original cases
  and adds typed/reflected parent renames in both directions over keyed/non-key
  children at 1/64/130 rows, exact aggregates and unrelated parents: 4,886 checks
  green, preceding actual runtime 780 red on the same fixture.
  `/tmp/agiru-rename-anchor.7GvVDQ`. Initial twenty-three compiled controls reject
  (`/tmp/agiru-record-order-controls.TqaC8V`), input hashes match. Final cleanup
  reuses one writer per cascade and includes only named headers; RenameGate stays
  4,886 green. Analysis has no own findings; 30/35 inherited runtime/gate header
  findings remain unsuppressed. Final control replay passes: 5,872/296/313/7,493/
  4,886 checks green, twenty-three controls reject, all input hashes match
  (`/tmp/agiru-record-order-controls.aUoxgz`). The image retains all seven non-runtime hashes from
  completed 223110 and replaces only libagiru_rt; it does not qualify rebuilt
  generated callers of the new ModifyAll template or the full UT population.
  Three original codeunits are terminal: 55/57 invoice, 43/44 sales credit memo,
  37/38 purchase credit memo; 135/139 passed, four unchanged Table Metadata failures.
  All three Rename tests recover; source count stays 139, no losses/changed errors/
  missing/added/duplicates (`three-codeunits-comparison.json`). Source/image hashes
  match after execution. Owned diagnostic databases and the disposable 903 MiB
  image are removed; hashes/results remain. Complete current-tree `make test JOBS=2`
  on `81c5209` passes: 141 cases/232 tooling tests, exit 0. Source/image hashes match
  before/after; all twenty-three record controls reject (`fAdRKi`).
  `/tmp/agiru-rename-anchor.7GvVDQ/all-local.log`. Full AL qualification is above.
- Caller-fix integration: `20261004T232014Z-3101267`, frozen HEAD `e9be54d`,
  source `74b27d9d9fe48dbdfdc95a03895d9873db1e29bd87d8b2f430c8170ecd984375`;
  `slice-check all test ut`, six jobs; terminal at 00:44:29 UTC, runner gone.
  Slice check/build/local tests pass (141 cases/232 tooling tests); UT exits 2:
  2,167/2,314 passed, 147 failed, zero incomplete, 80 codeunits/1,516 seconds.
  Against 223110: no gains/losses/changed errors/missing/added/duplicates; source
  manifest/file hashes match (`artifacts/ut-comparison-223110.json`). Dependencies
  match 223110; frozen source/System/original-notice pre/post hashes match.
  Includes ModifyAll caller preservation, excludes the later Rename cascade fix.
  Null/unsealed seed remains diagnostic, not causal A/B or G1.
  Current-tree local replay above starts only after this run enters UT; no shared
  gate-DB overlap. Replay the later Rename fix against all 2,314 original identities.
- ModifyAll caller preservation (0044): an independent filtered worker shares temporary
  rows, not caller buffers/cursors/globals. DynamicRecordGate: 7,493 checks green;
  the same fixture against the preceding implementation has 144 failures. SQL and
  temporary walks retain the caller's frame/SystemId/position in all eight orders,
  with live changed successors. New code has no analysis findings; 35 existing
  header findings remain unsuppressed. `/tmp/agiru-modifyall.tQeH01`.
  `make record-order`: 5,872/296/313/7,493 green, twenty-two compiled controls reject;
  input hashes match (`/tmp/agiru-record-order-controls.ZF440l`). Outside frozen 223110; full AL replay,
  sort-field mutation, native trigger/global proof and set-based optimization remain open.
  StorageGate retains 71 green checks, including no-OnValidate. Disposable DB
  `agiru_modifyall_20261005_01` is removed; original gate configuration is restored.
  Default-configuration `make gates JOBS=2` passes on `bc433db`, source hashes match
  (`default-gates-build.log`, `local-source-inputs.sha256`). Complete default-DB
  replay passes: 141 cases/232 tooling tests, exit 0; inputs unchanged before/after
  (`all-local.log`). It starts only after frozen 223110's local tests finish;
  no shared-DB overlap. Full AL with this caller fix is terminal above, without a
  UT status/error change.
- Dynamic-write integration: `20261004T223110Z-2943281`, frozen HEAD `382ecce`,
  source `d6b5367a085aae5be3e0d154ba6caf8660df15047a41945ca6294ab6efa53a0e`;
  `slice-check all test ut`, six jobs; terminal at 23:14:22 UTC, runner gone.
  Slice/build/local tests pass (141 cases/232 tooling tests); UT exits 2:
  2,167/2,314 passed, 147 failed, zero incomplete, 80 codeunits/1,510 seconds.
  Against 210945: zero gains, three losses, no changed errors/missing/added/duplicates;
  source manifests/file hashes match. Original posted invoice/sales credit memo/
  purchase credit memo Rename tests now report roughly tripled total tax amounts.
  Investigate their iteration/write paths; do not waive the regressions.
  Frozen source/System/notice pre/post hashes match.
  Source/System/original-notice dependencies match 210945.
  Compare all 2,314 identities/statuses/errors against 210945. Null/unsealed seed
  remains diagnostic, not causal A/B or G1. ModifyAll's caller fix is outside this run.
- Dynamic SQL reads (0044): successful row/bulk writes change a session/table revision;
  Next discards obsolete buffers before bounded keyset resumption. Tracking retires
  with the final reader; ordinary Next adds no SQL or map lookup. DynamicRecordGate:
  2,335 checks green, including typed/RecordRef mutations, filters, isolation,
  failed/temporary writes and 1,000 retired table visits. Initial business matrix:
  320 checks/48 red before the fix. `make record-order` retains 5,872/296/313 green
  and rejects twenty-one compiled controls. No new analysis findings; existing
  RecordChanges/Navigate/Storage/gate findings remain 25/30/38/34, unsuppressed.
  `/tmp/agiru-dynamic-record.AeNaPT`, `/tmp/agiru-record-order-controls.fEDCql`.
  Full local replay on `ccdd9a7`: 141 cases/232 tooling tests pass, exit 0;
  source hashes match before/after (`all-local.log`, `verified-inputs.sha256`).
  Outside frozen 210945; full AL replay,
  Query boundaries and company qualification remain open. No UT gain or G1 claim.
- Dynamic-write continuation (0044): 5,013 checks green over all eight write kinds,
  typed/RecordRef, uniform/mixed/global-reversed orders and pre-existing backward
  cursors; original cases remain. Own Modify/Delete/Rename retain frame/system ID
  and resume at the current key. Twenty-one compiled controls reject; no own analysis
  findings, 35 existing header findings remain unsuppressed. Disposable gate DB
  `agiru_dynamic_order_20261004_01` isolated the run from frozen AL verification;
  it is removed. Original gate configuration is restored; the default-DB replay
  passes the same 5,013 checks (`default-gate.log`, `default-inputs.sha256`).
  `/tmp/agiru-dynamic-order.1x5qFL`, `/tmp/agiru-record-order-controls.rNkiaB`.
  Expanded matrix is outside the 141-case local replay and frozen 210945; no UT/G1 claim.
- Cursor lifetime (0044): transaction-aware buffers, bounded bidirectional resume
  and released/surviving/absent portal cleanup. CursorLifecycleGate: 313 checks
  green; final coherent predecessor fixture reports 14 failures. Ordering/selection
  remain 5,872/296 green; thirteen compiled controls reject. All 24 traced walks
  execute: 64 steps use three SQL statements, not 64 per-row reads. No own analysis
  findings, 33/25/30 inherited gate/Cursor/Navigate findings remain unsuppressed.
  `/tmp/agiru-cursor-lifecycle.p37EWy`, `/tmp/agiru-record-order-controls.YfDQHq`.
  Full replay is terminal below; dynamic writes are qualified separately above,
  Query transaction contracts remain open.
- Cursor/selection integration: `20261004T210945Z-2749818`, frozen HEAD `d1b4873`,
  source `92a6ba7025e7341a691de8da842698ccb9832d86f3665e6404a84f463cc7901f`;
  `slice-check all test ut`, six jobs; terminal at 22:29:37 UTC, runner gone.
  Slice/build/local tests pass (140 cases/232 tooling tests); UT exits 2:
  2,170/2,314 passed, 144 failed, zero incomplete, 80 codeunits/1,492 seconds.
  Against 193025: one gain, zero losses/missing/added/duplicates; source manifests
  and file hashes match (`artifacts/ut-comparison-193025.json`). The gained original
  SCM Available to Pick UT multi-item summary scenario previously refused item
  tracking at document 106058/line 20000. The other changed error only advances
  generated item/location IDs; tracking quantity remains 0 instead of 10.
  Frozen source/System/notice pre/post hashes and dependencies match.
  Null/unsealed seed remains diagnostic, not causal A/B or G1.
- Selection invalidation (0044): SQL cursors and temporary views follow changed
  filters/keys/directions/active marks; shared temporary Modify is visible.
  SelectionChangeGate: 296 checks green, coherent predecessor 64 red;
  mixed-order matrix remains 5,872 green, nine compiled controls reject.
  `/tmp/agiru-selection-change.li6S1N`, `/tmp/agiru-record-order-controls.sdKg5i`.
  Targeted lint retains existing findings, none in new functions. Outside frozen
  193025; SQL dynamic writes and full AL replay remain open; cursor lifetime is qualified above.
  Complete local replay on HEAD `bae1071` passes: 139 cases/232 tooling tests, exit 0,
  affected source hashes unchanged (`all-local.log`). A read-only Integer probe separately reproduces
  destroyed-cursor FETCH after Commit and savepoint rollback: six checks/two red.
- Mixed-order integration: `20261004T193025Z-2625193`, frozen HEAD `7b7cf45`,
  source `a234a8d7b12a5df02595bc4d9376c8345f454d7566b5652dcc279dc21c499aad`;
  `slice-check all test ut`, six jobs; terminal at 20:42:35 UTC, runner gone.
  Slice/build/local tests pass (138 cases/232 tooling tests); UT exits 2:
  2,169/2,314 passed, 145 failed, zero incomplete, 80 codeunits/1,481 seconds.
  Against 184333: zero gains/losses/changed errors/missing/added/duplicates;
  source manifest/file hashes match (`artifacts/ut-comparison-184333.json`).
  Frozen source/System/notice pre/post hashes match.
  BCApps `bb7111877f`; BC source/System/original-notice hashes match completed
  184333. Null/unsealed seed remains diagnostic repeatability, not causal A/B or G1.
- Record ordering (0044): shared selected-field directions and primary ties;
  SQL mixed-key predicates and temporary views now agree. `make record-order`:
  5,872 checks green, four compiled controls reject; previous actual runtime has
  1,046 failures on the same matrix. `/tmp/agiru-record-order.FQ83WP`.
  Own findings are cleared; inherited header/TempFind findings remain explicit.
  Complete local replay passes: 138 cases/232 tooling tests, exit 0;
  source-input hashes remain unchanged. Full AL replay above is terminal. Outside 184333;
  no live Table Metadata activation, UT gain or G1 claim.
- Codepage integration replay: `20261004T184333Z-2543205`, frozen HEAD `155abe4`,
  source `79960d56653338db08e34dd932ca9d9914ace4454323d51b1578b5ca819d39a1`;
  `slice-check all test ut`, six jobs; terminal at 19:20:49 UTC, runner gone.
  Slice/build/test pass (136 cases/232 tooling tests); UT exits 2:
  2,169/2,314 passed, 145 failed, zero incomplete, 80 codeunits/1,422 seconds.
  Against 175227: zero gains/losses/changed errors/missing/added/duplicates;
  all source manifest/file hashes match. `artifacts/ut-comparison-175227.json`.
  Frozen source/System/notice pre/post hashes match, BCApps `bb7111877f`.
  Null/unsealed seed remains diagnostic repeatability, not causal A/B or G1.
- Codepage foundation (0035): Windows-1252/ASCII/ISO-8859-1, distinct best-fit,
  factory aliases/preambles and explicit unknown-page refusals. `make encoding`
  passes 87 checks and twelve compiled controls; all 192,678 selected native text
  cases agree, 720 outside-profile cases remain visible. Previous actual library
  has 49 failures on the same population. Three complete native codepage corpora
  each retain 65,536 BMP units/256 bytes/1,048,576 supplementary scalars; all C++
  results match. `/tmp/agiru-codepages.g4fWUJ`, `/tmp/agiru-encoding.SaM7UU`.
  CodePage analysis passes; runtime/gate have no own findings, 33 inherited header
  findings each remain unsuppressed. Local replay: 136 cases, one wrong-notice-path
  refusal; 232 tooling tests pass. Corrected layout qualifier passes, without
  rewriting the red receipt. Frozen full replay above is terminal. Other codepages,
  locale defaults/native bindings remain open. Notices: `licenses/`.
- Unicode integration replay: `20261004T175227Z-2443696`, frozen HEAD `c732229`,
  source `7a2621f2a5a8eea448751274f9760ac11011a10a485ed498240149c5b2ed7439`;
  `slice-check all test ut`, six jobs; terminal receipt, runner exited.
  Slice-check/all/test exit 0: 136 local cases/232 tooling tests pass.
  UT exit 2: 2,169/2,314 pass, 145 fail, zero incomplete; 80 codeunits/1,353 seconds.
  Against 164436: zero gains/losses/changed errors/missing/added/duplicate identities;
  source manifest/file hashes match. `artifacts/ut-comparison-164436.json`.
  Frozen source/System/notice hashes match before/after; BC source/System/notice
  dependencies match 164436. Null/unsealed seed remains diagnostic, not causal A/B.
  Codepage changes are outside this snapshot; no G1 claim.
- Unicode foundation (0035): 48 gate checks and five compiled controls pass;
  all 192,332 Unicode cases match original BC29 text cores, 1,066 outside-profile
  cases remain reported. Previous actual library fails 81,373 reference cases.
  `/tmp/agiru-base64-text-reference.pGQWvN`; complete local replay passes 136 cases/
  232 tooling tests, exit 0. Complete AL replay is recorded above, without losses.
  Native binding, other codepages and locale defaults remain open.
- Native integration replay: `20261004T164436Z-2355150`, frozen HEAD `ae9c59a`,
  source `1b5af9f74965a935aa39ab0eda282f8bede0ce4c1f101f7695889fa56efc53c0`;
  `slice-check all test ut`, six jobs; terminal receipt, no live runner.
  Slice-check/all/test exit 0: platform/slice link, 135 local cases/232 tooling tests.
  UT exit 2: 2,169/2,314 pass, 145 fail, zero incomplete; 80 codeunits/1,464 seconds.
  Against 143703: zero gains/losses/missing/added/duplicate identities, matching
  source manifest/file hashes. Two OAuth errors now identify the original native
  Base64 method instead of an absent .NET member; no business pass gain.
  `artifacts/ut-comparison-143703.json`. Frozen source/System/notice hashes remain
  unchanged; BCApps `bb7111877f`. Null/unsealed seed is diagnostic repeatability,
  not causal A/B. Unicode repair is outside this snapshot; no G1/provider claim.
- Native codeunit indexing/output (0034): 35 raw/selected/emitted original identities,
  zero missing/excluded/duplicates; 35 compile without PCH. Linked registry 108 and
  original nine-overload refusal eleven checks green. Bare/qualified/numeric consumers
  execute. All 64 platform units compile/link as one shared library; registry-only
  lookup retains 35 codeunits under `--as-needed`, 108 checks green. Without library
  retention the dependency disappears and 36 checks fail. No native provider proof.
  ID/missing-definition controls fail. Previous actual compiler fails five
  assertions and cannot compile the consumer. Full generation adds 70 files, changes
  103, removes none; `/tmp/agiru-transpile.NfMuFN`, exit 1.
  Root `apps/` regenerated through Make: all 24,534 files match the qualified tree,
  zero root-only/different files, all 14,225 slice sources present;
  `/tmp/agiru-transpile.p0ubXY`, exit 1. Still 72 unbound Native
  methods, 90 inactive other sources, 215 unbound tables and 21 unresolved controls.
  Full local replay: 135 cases/232 tooling tests pass; native tooling 58 checks green.
  Runner analysis passes; Main retains four existing Main/BodyWriter findings,
  no added suppression. `/tmp/agiru-native-codeunit-output.k7vuZx`,
  `/tmp/agiru-native-codeunit-output-{all-local-final,native-tooling-final,lint}.log`.
  Full AL replay and primitive activation remain pending; no UT gain/G1 claim.
- Completed integration replay: `20261004T143703Z-2217132`, frozen HEAD `ad7a917`,
  source `8534a91bf87c10d8fc0ffd85f113dca46381fdda1cacbcccfad778357bfbc87e`;
  `slice-check all test ut`, six jobs; terminal receipt, no live runner.
  slice-check/all pass; test exits 2 (133 cases, one refused layout qualifier,
  227 tooling tests pass). UT exits 2: 2,169/2,314 passed, 145 failed, zero
  incomplete, 80 codeunits/six workers/1,278 seconds. Against 132617: zero gains,
  losses or changed errors. Against 101600: zero gains/losses, eleven changed
  errors. All identities and source hashes match, zero missing/added/duplicates;
  `artifacts/ut-comparison-{132617,101600}.json`. Null/unsealed seed remains
  diagnostic repeatability, not causal A/B. Frozen source/package hashes match
  before/after. Later byte-codec/notice-preflight changes are outside this snapshot.
  The source-only BC input lacks its original notice; matching notice SHA256
  `c2cfccb812fe482101a8f04597dfc5a9991a6b2748266c47ac91b6a5aae15383`
  from 132617 makes the separate frozen layout replay pass, without changing the
  original red target or frozen inputs. `/tmp/agiru-layout-assets-check.z72Dwo`.
  The current freezer rejects this missing-notice input before dependency copying;
  relocated sources require explicit `AGIRU_LAYOUT_SOURCE_NOTICE`. Includes BLOB
  amortization, native codeunit
  admission and corrected Discovery registry. BC input hash matches 132617
  (`af53219b7fc9e68a58293bad9fd178232d6bef47bbab2166b390620fb38497a2`);
  refreezing the source-only snapshot cannot rediscover Git revision, so this
  receipt's revision is null. The matching original receipt records BCApps
  `bb7111877ff786951b86a1a0f80d8b39b8f5dacd`.
- Base64 byte codec (0034): shared string/raw-stream encode/decode, 76-column CRLF,
  strict Convert-profile whitespace/padding and validation before output. Nonalias
  stream output uses 4 KiB scratch; aliased input is preserved before growth.
  `make base64`: 79 checks pass; five compiled mutants fail. Direct original BC29
  `ToBase64Core`/`BytesFromBase64Core` on temporary CLR 10.0.12 supplies 10,051
  cases (4,034 encode/6,017 decode, 2,002 invalid); both C++ output forms agree,
  30,214 checks/zero red. `/tmp/agiru-base64-core.S1ZV6C`; reference SHA256
  `85f6c9f4a6cba8dfcafc23bfe0ccae9871cac943191dd079e558b6a33044d71b`.
  Stream/Encoding retain 51/5 checks; Discovery passes. Three targeted units have
  no own findings, but each retains 25 unsuppressed Char/StringValue header findings.
  Full local replay: 135 cases/227 tooling tests pass, exit 0;
  `/tmp/agiru-base64-all-local.log`. Native production binding, text encodings, stream
  input/cursors, transform-block decoder, AL diagnostics and all thirteen original
  BC tests remain open; no UT gain, performance, multi-user or WASM claim.
- Verification notice preflight (0058): `make verify-check` passes 22 checks;
  previous actual freezer fails the new missing-notice control. The original notice
  override is frozen/hashed and reaches both test/UT targets. Missing notices emit
  an explicit layout refusal; `make layout-assets-check` passes eight registry
  checks, all three assets and integrity/ownership controls. No fallback license
  or weakened notice guard. Complete tooling replay: 228 tests pass.
  `/tmp/agiru-notice-preflight-{verify-check,previous,layout-final,all-tooling}.log`.
- Native codeunit admission (0034): all 35 selected source ASTs retain signatures,
  paths and manifest identity; 72 Native methods remain individually unbound and
  the aggregate 125 inactive-source gap remains. Missing owner/duplicate identities
  refuse; product policy retains original excluded identities. Unbound Native
  emission now refuses before AL locals/body/events, rather than empty success.
  Generator/authored/original-declaration execution passes 49/19/11 checks;
  removing Native fails, previous actual generator fails nine original calls.
  `/tmp/agiru-native-codeunits.tTdgkj`, `/tmp/agiru-native-codeunits-previous.f3GfbR`.
  Native-source/report/enum tooling passes 54 tests. Verified-package generation
  retains all 24,464 files byte-identical and unchanged native/property/control
  gaps; all 14,225 slice files exist. `/tmp/agiru-transpile.vHcXmw`, exit 1;
  `generation-comparison.json` matches all 35 independent raw codeunit identities.
  Discovery's corrected script registry passes its negative controls; current
  full local replay is green above. Frozen replay has only the missing-notice
  refusal above. New loader/refusal/gate/
  runners/emitter analysis passes; three old gate include findings removed.
  `make lint` retains only existing CodeunitWriter/Main/BodyWriter findings over
  thirteen selected units; no suppression/baseline increase.
  `/tmp/agiru-native-codeunits-{all-local,discovery-final,lint}.log`.
  This admission receipt predates production indexing/output above;
  primitive activation remains open; no Base64 binding or UT gain claimed.
- Qualified-table recovery replay: `20261004T132617Z-2130724`, frozen HEAD
  `23d1497`, source `1dee82b25ed072be32d3b5b27f03be68d059c6bdff423b707c0e99f12c947d07`;
  `slice-check all test ut`, six jobs. Terminal: slice-check/all/test exit 0;
  UT exit 2, 2,169/2,314 passed, 145 failed, zero incomplete, 80 codeunits,
  six workers/1,330 seconds. Against 120802: 69 gains, zero losses, 28 changed
  errors. Against 101600: zero gains/losses, eleven changed errors. All 2,314
  identities and source hashes match, with no missing/added/duplicate cases.
  Receipts: run `artifacts/ut-comparison-{120802,101600}.json`. Null/unsealed
  seed: diagnostic repeatability, not causal A/B. Includes installed metadata
  lookup and prepared filters; later BLOB append changes are outside this snapshot.
- BLOB stream append (0035): raw/text/terminated writes reuse owned storage with
  geometric growth; borrowed self-input remains safe across reallocation. Stream
  passes 51 checks; the prior actual library fails four storage-growth controls.
  `/tmp/agiru-stream-append-{gate-final,previous-final}.log`, final image/source hashes
  `/tmp/agiru-stream-append-final.sha256`. Targeted runtime/gate analysis has no own
  findings; 25/47 inherited header findings remain, unsuppressed. Three old gate
  findings are repaired. Full local replay passes 132 cases/223 tooling tests,
  exit 0 (`/tmp/agiru-stream-append-all-local.log`); final source/image hashes
  still match. AL replay for this later batch remains pending;
  encoding, typed binary layouts and the complete BLOB size policy remain gaps.
- Completed native-constant activation `20261004T120802Z-2065848`: frozen HEAD
  `1c01b11`, source `0039282b99790dd762ea620521f5f34d77fa4398eaa0512414f2fc5c66024c25`;
  slice-check/all/test exit 0; UT exit 2: 2,100/2,314 passed, 214 failed,
  zero incomplete, 80 codeunits, six workers/1,258 seconds. Against 101600:
  zero gains, 69 losses, 28 changed errors, no missing/added/duplicate identities;
  manifest/source-file hashes match. Receipt: run `artifacts/ut-comparison-101600.json`.
  Losses reach qualified ordinary table constants, not the later metadata/filter
  helpers. Null/unsealed seed: diagnostic repeatability, not causal A/B.
  Repair adds exact namespace aliases to the existing table index, rejecting
  collisions and wrong namespaces. Generated fixtures pass 25 checks/seven identity
  controls; previous compiler fails the new qualified ordinary constant.
  `/tmp/agiru-native-table-ids.0ENfDa`, `/tmp/agiru-qualified-table-previous.w040rh`.
  Runner analysis passes; Main retains five existing findings and no new findings.
  Verified-package regeneration changes 22 files; the diagnosed qualified refusals
  disappear. `/tmp/agiru-transpile.bafUwI`: exit 1, unchanged package identities
  and native/property/control gaps; all 14,225 slice sources remain. Full Make
  replay and compiled activation recover all 69 UT losses in 132617 above.
- Compiled record filters (0044): one owned predicate for temporary operations
  and computed rows. ReflectionMetadata passes 194 checks/sixteen controls;
  Filter/FilterGroup/Temporary retain 125/137/80 green checks. Same FilterGroup
  bodies: parser calls decrease 97→25, not a workload performance claim.
  `/tmp/agiru-record-filter-parsing.EklR47`, `/tmp/agiru-reflection-metadata.hiyxPP`.
  No new lint findings; inherited headers/TempFind remain red. Complete local Make
  passes 132 cases/223 tooling tests, exit 0; parsing receipt `local-verification.json`
  and `/tmp/agiru-compiled-record-filter-all-local.log`. Outside frozen 120802;
  not live-provider activation or a UT gain.
- Installed metadata lookup (0044): existing catalogue, on-demand projection by
  original ID; absent rows and unqualified declarations remain distinct. Generated
  replay passes 61 checks and two new runtime controls; runner lint passes, runtime
  retains 30 inherited header findings and no own findings. Receipt:
  `/tmp/agiru-table-keys.rUwABv`. No live-provider/UT claim; outside frozen 120802.
- Native table constants (0034): selected native AST identities are independent
  of record bindings/providers. Native-binding/parser gates pass 154/139 checks;
  compiled codeunit/table/page/report fixture passes 13 checks and six identity
  controls. Previous compiler refuses its first constant; ID mutation fails eight
  values. `/tmp/agiru-native-table-ids.{gOASon,2WAIqY}` and
  `/tmp/agiru-native-table-ids-previous.zcfHaD`. Parser/runner analysis passes;
  existing generator/header findings remain, without suppressions. Full verified-
  package regeneration changes 40 files; the original RapidStart filter now emits
  IDs 2000000004/5. Native 233 selected/18 bound/215 unbound, 125 other inactive
  sources and 21 unresolved controls remain; all 14,225 slice sources exist.
  `/tmp/agiru-transpile.vohqhQ`: exit 1, unchanged package hashes. Unchanged
  2,314-method UT replay completed in `20261004T120802Z-2065848`, frozen HEAD
  `1c01b11`, source `0039282b99790dd762ea620521f5f34d77fa4398eaa0512414f2fc5c66024c25`,
  targets `slice-check all test ut`, six jobs. Build/local targets pass; UT regresses
  as recorded above. Independent census retains 113,013 raw/112,998 required
  methods, zero unmeasured files and seven refused variants in three sources
  (exit 2). Receipt: run `artifacts/scope-inventory-development.json` and
  `/tmp/agiru-native-table-ids-census.log`. No provider, full-link or UT gain claimed.
- Extension controls (0034): one page/report splice implementation; unresolved
  anchors refuse and preserve previous output. Compiled metadata 62 checks green;
  missing/cyclic anchors and wrong-order controls reject. Complete local Make:
  131 cases/223 tooling tests green, exit 0. Runner analysis passes; Main/header
  findings decrease six→five, Scan complexity 99→98; no suppression increase.
  Full regeneration exits 1 with unchanged native/missing/property counters and
  21 unresolved controls now explicitly refusing. All 14,225 slice sources remain;
  all emitted files match current root files (321 root-only stale tree entries
  are separate). `/tmp/agiru-transpile.i5mrtz`,
  `/tmp/agiru-control-extensions-all-local.log`. Outside frozen 101600; not full
  extension semantics, app linking or G1 proof.
- Boolean lowering (0073): owned eager left-to-right operands; 26 primitive/34
  generated checks green, old short-circuit compiler 15 red, source-order control
  four red. Complete local Make replay: 130 cases/223 tooling checks green.
  `/tmp/agiru-boolean-final-local.log`; new gate/runner analysis passes, generator
  inherited findings remain. Regeneration retains prior gap counters and all
  14,225 slice sources. Activation snapshot `20261004T101600Z-1902967`
  freezes HEAD `5b7072c`, source `8af4d25d73e72d881a5df4ed1289e7fe928240c10d8f419cd0d5e1c0a9da86b3`;
  slice-check/all/test exit 0; UT exits 2: 2,169/2,314 passed, 145 failed,
  zero incomplete, 80 codeunits, six workers/1,430 seconds. All identities and
  manifest/source-file hashes match 080905: seven gains, zero losses/missing/added
  cases, thirteen changed errors. Gains: six serial-tracking pick/shipment cases
  and the no-breakbulk summary scenario. Eleven RapidStart cases now reach the
  missing Permission Set identity; 37 live Table Metadata refusals remain.
  Comparison: snapshot `artifacts/ut-comparison-080905.json`. The separate AL
  numeric helper is compiled but not emitted. Null/unsealed seed: diagnostic
  repeatability, not causal A/B, complete-app linking or G1 proof.
- Native qualifier/output repair (0034): all 29 platform sources compile in app/
  slice modes; dropped compilation refuses. Original/loader report checks pass
  562 each; native enum declarations pass 28/28. Analysis-only writes no files
  (old compiler: 18); explicit generation is byte-identical to the compiled proof.
  Specialist receipts now exist; six inherited Main/header findings keep lint red.
  `/tmp/agiru-native-no-output-final.4hBhBG`, `/tmp/agiru-native-report-layouts.XcbXKD`.
  This source-only guard/recipe batch is outside snapshot 101600.
- Earlier frozen baseline `20261004T080905Z-1761326`: slice-check/all/test exit 0;
  UT exit 2, 2,162/2,314 passed, 152 failed, zero incomplete, 80 codeunits,
  six workers/1,256 seconds. All 2,314 identities compared with 064830: no gains,
  losses, missing/added identities or changed errors. Receipt:
  `/tmp/agiru-al-decimal.S7CPdN/ut-final-comparison.json`; source
  `9243953607b10e22e34c722a13ba0c1b3416f70e1ee1ced0d2397a04c49e9b0c`.
  Null/unsealed seed: repeatability, not causal A/B or G1/full-app proof.
- Separate AL arithmetic foundation (0066/0073): 34 gate checks and 37,532 original
  BC29 Decimal18 reference cases pass exact values/scales/order/error categories.
  Three compiled controls reject; targeted runtime/gate analysis passes.
  Complete local replay: 128 cases/223 tooling tests green, exit 0.
  `/tmp/agiru-al-decimal.S7CPdN/receipt.json`; not active in generated AL or frozen 080905.
  Qualify every typed conversion/evaluation boundary before full-tree activation;
  this does not close field/SQL/format/version gaps or make G1 green.
- Later Decimal batch (0066): CLR scale 28/96-bit arithmetic, nearest-even fitting
  and exact-division/zero scale contracts; 84 Decimal checks green, three compiled
  controls reject, 31,536 deterministic .NET 8.0.31 cases match exact text/scale.
  Ordinary SQL gate proves scale-28 buffers versus scale-20 persisted values and
  PostgreSQL ties-away rounding (71 checks green). Decimal runtime/gate analysis
  passes without suppressions. `/tmp/agiru-decimal-clr28-*.log`, reference source
  and results `/tmp/agiru-decimal-clr-oracle.cIgWYH/`.
  Complete local gate build and eight affected read-only gates pass; StorageGate
  own findings decrease by eleven to zero, with 31 existing header findings retained.
  Outside completed 064830; full AL activation and UOM-loss investigation remain due.
  Initial local replay: 127 cases/one obsolete FilterGate expectation red;
  223 tooling tests pass. Correct scale-28 filter contract: 125 green, old-core
  control three red; local replay passes 127 cases/223 tooling tests. Later Round
  rejects negative precision: 90 checks green, six red before repair; runtime/gate
  targeted lint pass. Filter cleanup keeps 125 checks, removes sixteen own findings
  and its overlong function; thirty inherited header findings remain unsuppressed.
  Final local replay passes 127 cases/223 tooling tests, exit 0
  (`/tmp/agiru-round-final-all-local.log`). Frozen `20261004T080905Z-1761326` completed
  slice-check/all/test/ut with the unchanged 2,162/2,314 result, source
  `9243953607b10e22e34c722a13ba0c1b3416f70e1ee1ced0d2397a04c49e9b0c`,
  HEAD `108944f`, BCApps `bb7111877f`; later FilterGate cleanup is outside it.
  Compare every original method with 064830. Original UOM default runs retain
  19/19 but only exercise 1/4. Explicit diagnostic seed 4 exposes nine losses
  (9/19 → 0/19), scale-28 setup versus scale-20 SQL read-back; not regular UT counts.
  Evidence and boundary work: 0066; native RNG reset coverage: 0039.
  Executed original BC29 Decimal18 shows an AL-specific eighteen-significant-digit
  wrapper distinct from the CLR core; native 1/9 × 100 has zero remainder against
  its basis precision. Version-qualified AL lowering/conversions remain open
  (0066/0073), not a reason to reduce the CLR core globally.
- Later page batch (0030): system Edit follows a list's declared CardPageId and
  selected row through common Page.Run; explicit actions keep precedence.
  Card opening triggers, standalone modes, false ModifyAllowed and empty selection
  execute. Unbound TestField.AsInteger now raises rather than dereferencing null.
  PageSource 15 and generated AL 15 checks green; four compiled controls reject.
  `make test JOBS=2`: 127 cases/223 tooling tests green
  (`/tmp/agiru-page-navigation-all-local.log`). Include-only runner cleanup repeats
  the focused checks/controls and passes targeted analysis; runtime/gate retain
  44/49 unchanged header findings and zero own findings. Frozen activation
  `20261004T064830Z-1661382` completed `slice-check all test ut`, six jobs;
  source `76f14f5fc6f16069f0bb4619fdc4adcf91257edb46341db981d3efafc28bfdd8`,
  HEAD `108944f`, BCApps `bb7111877f`, original System hash unchanged.
  Includes ordinary metadata defaults: slice-check/all/test=0, ut=2;
  2,162/2,314, 152 failed, zero incomplete, 80 codeunits, 1,390 seconds.
  Compared with 044804: one gain (TestProcessAskUserPermission), zero losses,
  zero missing/added identities. Three other Incoming Document errors now reach
  the explicit live Table Metadata provider refusal instead of an unopened page.
  `/tmp/agiru-page-metadata-ut-comparison.json`; legacy null/unsealed seed,
  diagnostic repeatability rather than causal A/B. Decimal batch is not included.
- Completed scoped-Record/XML integration `20261004T044804Z-1552154`:
  `slice-check/all/test=0`, `ut=2`; 2,161/2,314, 153 failed, zero incomplete,
  80 codeunits, 1,378 seconds. All 2,314 unique identities and pass statuses match
  023609; two Incoming Document diagnostics now reach the Table Metadata refusal.
  `/tmp/agiru-scoped-xml-ut-comparison.json`. Original SQL/debugger replay proves
  post-rollback error restoration and the first trapped error list; the remaining
  unopened-page trap is the card's ErrorMessagesPart (0030/0061).
  Metadata defaults and the later Edit-navigation fix are outside this run.
  Legacy null/unsealed seed: diagnostic repeatability, not causal A/B proof.
- Later Table Metadata batch (0044): ordinary AL defaults are resolved without
  overwriting source omissions; native omissions still refuse. Compiler-18/Runtime-18
  generated metadata qualifies the defaults, not CLR optional arguments.
  ReflectionMetadata 181 checks/thirteen controls and generated AL 52 checks pass.
  Complete `make test JOBS=2` passes 126 cases/223 tooling tests
  (`/tmp/agiru-metadata-defaults-all-local.log`);
  `/tmp/agiru-metadata-defaults-receipt.json` retains source/tool/image identities.
  Runner analysis passes; runtime/gate each retain 30 inherited header findings,
  zero own findings. Two gate findings are repaired without suppression.
  This batch is not in frozen integration `20261004T044804Z-1552154`; the live
  provider guard remains, so no metadata UT gain is claimed.
- Subsequent var-Record Codeunit.Run batch (0061): scoped, lazy table-global identity;
  ordinary copies stay independent and callee state restores after success/error.
  InstanceGate 49 and generated AL 60 checks pass, including post-rollback SQL
  restoration, nested/typed/static/dynamic/statement/handle/cursor cases; three
  compiled controls reject. Complete `make test JOBS=2` passes 126 cases and
  223 tooling tests (`/tmp/agiru-scoped-globals-local-tests.log`, proof
  `/tmp/agiru-codeunit-record.rr4usJ`). New runner targeted analysis passes;
  gate analysis still reports 38 header findings and zero own gate findings,
  including the unchanged no-op Globals assignment; no suppression was added.
  A Globals handle adds one
  pointer, preserving standard layout and allocation laziness; no resource/throughput
  improvement claimed. Completed integration `20261004T044804Z-1552154` has
  build/local targets green and the UT result above. Its targets are
  `slice-check all test ut`, six jobs; source
  `a2852e3b3e7e273f9538ba57cd5b10bd87124ea8ff7fc95eda1859bbbd7ebfbb`,
  System package `34c40f0dcc839eb4d244398715e11a801194bfcea69a21257c5de20a9833a955`,
  HEAD `108944f`, BCApps `bb7111877f`. It also includes the XML-reader batch;
  all 2,314 identities/statuses/diagnostics are compared above.
  `/tmp/agiru-scoped-globals-receipt.json` retains local source/image/proof hashes.
  Original saved-error SQL restoration now executes; no UT pass gain is claimed.
- Subsequent XML-reader batch (0035): shared cursor/Close plus consuming DOM Load;
  positioned/ended readers do not reload old roots. XmlReaderGate 71 checks green;
  three compiled separate-state/raw-load controls reject
  (`/tmp/agiru-xml-reader-consuming-load-{gate,controls}.log`). Reader and gate
  have no own targeted lint findings; inherited findings remain unsuppressed.
  Initial full local Make run ended with signal status 143, without a completion
  count (`/tmp/agiru-xml-reader-consuming-load-local-tests.log`); no child remained.
  Fresh `make test JOBS=2` replay passes all 125 cases and 223 tooling tests, exit 0
  (`/tmp/agiru-xml-reader-consuming-load-local-replay.log`); source/image hashes
  and bounded proof scope: `/tmp/agiru-xml-reader-consuming-load-receipt.json`.
  DTD/resolver enforcement,
  encoded declarations and bounded input remain open. Outside frozen 023609;
  no full UT effect claimed.
- Completed SetCurrentKey integration `20261004T023609Z-1410666`: HEAD `108944f`,
  source `9484879790ef1e208093a33283f1074fa34c4835038df641983ee3b92919796a`,
  original System dependency `34c40f0dcc839eb4d244398715e11a801194bfcea69a21257c5de20a9833a955`,
  frozen BCApps `bb7111877f`. Targets `slice-check all test ut`, six jobs;
  finished 04:01:07 UTC, 5,064 seconds. `slice-check/all/test=0`, `ut=2`;
  all 14,225 slice inputs, 124 local cases and 223 tooling tests retained/green.
  UT: 2,161/2,314, 153 failures, zero incomplete, 80 codeunits, 1,603 seconds.
  Source/package/notice hashes remain unchanged. Versus 011421: all 2,314 unique
  identities retained, zero gains/losses; four SCM - Planning UT diagnostics change
  from wrong quantity to missing Inventory Profile 3. Investigate them in 0044;
  `/tmp/agiru-current-key-ut-comparison.json`. Unsealed seed is repeatability,
  not sealed causal A/B. Subsequent XML-reader changes are excluded.
- Development SetCurrentKey batch (0044/0061): shared typed/reflected first-active
  full-prefix selection, valid unindexed ordering and consumed/statement errors.
  CurrentKeyGate 54 and FindGate 43 checks green; actual SQL/temporary tied rows and
  native All Profile execute. Generated AL 48 checks green; six call-context,
  three key-selection and one source-expression controls reject
  (`/tmp/agiru-current-key-local-tests.log`, `/tmp/agiru-test-contexts.VFwyFa`).
  Complete local replay: 124 cases/223 tooling green. Runner analysis passes;
  new functions/gate have no own findings, inherited findings remain unsuppressed.
  Regeneration `/tmp/agiru-transpile.cvtGhD`: all 80/2,314 UT identities and
  14,225 slice inputs retained; eight changed files contain only 44 consumed-call
  rewrites, zero path losses. Package pre/post identities match; translation still
  exits 1 for counted native/AL gaps. FlowField sorting remains explicitly refused.
  In completed 023609, not 011421; terminal effects are above. No UT pass gain.
- Completed metadata/call-context integration `20261004T011421Z-1315211`: HEAD
  `108944f`, source
  `fa6e53d85c1ad8fb8a68af60cf32d63680167875cda9babe5ae3d9ed4ea8bbe8`,
  BCApps `bb7111877f`, original System dependency
  `34c40f0dcc839eb4d244398715e11a801194bfcea69a21257c5de20a9833a955`.
  Finished 02:21:23 UTC in 3,987 seconds: `slice-check/all/test=0`, `ut=2`.
  Local replay passes 123 cases/223 tooling tests; UT passes 2,161/2,314,
  153 failures, zero incomplete, 80 codeunits, six workers, 1,550 seconds.
  All identities/statuses are unchanged; one diagnostic changes:
  Incoming Doc. To Data Exch.UT::TestProcessWithDataExchSucceeds now reaches
  "The TestPage is not open." (`/tmp/agiru-call-context-ut-comparison.json`).
  Diagnostic replay remains 12/32 (`/tmp/agiru-incoming-doc-trace-after.{log,jsonl}`).
  The unsealed seed is repeatability evidence, not a sealed causal A/B. This snapshot
  includes the projection/shared emitter and TryFunction/single-evaluation case fixes.
  Production regeneration `/tmp/agiru-transpile.foLQAE`: 18,998 objects emitted,
  39,974 full-tree test methods, exit 1 for retained gaps (including 215 selected
  unbound native tables, 625 unsupported object kinds and 5,616 refused properties).
  Native package pre/post identities match; all 14,225 slice inputs remain present.
  Versus the prior frozen apps: 2,218 changed files, zero added/missing paths
  (`/tmp/agiru-call-context-generated-diff.log`).
- Development call-context batch (0061): shared own-method/Rec/this TryFunction
  attribute lookup and explicit per-expression ValueUse; nested arguments no longer
  inherit the outer discarded-call flag. Case selectors are bound once, including
  ranges/else; generated AL execution passes 40 checks. Four compiled call-context
  mutants and the existing source-expression mutant fail
  (`/tmp/agiru-call-context-final-controls.log`). Runner targeted analysis passes;
  the split call/argument helpers have no own findings, inherited findings remain.
  Database write policy, overload/chained
  resolution remain unproved. Frozen incoming-document trace
  reproduces 12/32; do not attribute conversion assertions to optional-field catches.
  Complete local replay passes 123 cases/223 tooling
  (`/tmp/agiru-call-context-local-tests.log`). Independent source census retains
  all 80/2,314 identities (`/tmp/agiru-call-context-ut-identity-comparison.json`).
- Development metadata batch (0044): private 23-field Table Metadata projection;
  one native/app declaration-property emitter, validated Boolean/TableType/page
  references and static source-number caption arrays. GenNativeBinding 128,
  GenTable 78, ReflectionMetadata 165 and generated table-keys 46 checks green;
  previous compiler fails the new declaration gate (25 red). Source replay lint
  is green; existing public-header/generator findings remain unsuppressed.
  Original-package audit `/tmp/agiru-native-bindings.IJI5DF`: 234 raw tables,
  233 selected, 18 qualified/compiled and 221 runtime checks; 215 selected unbound
  remain red. Receipt precedes the final unused-include-only optimization.
  Full generation `/tmp/agiru-transpile.uPdL8p` writes 18,998 objects, exit 1 for
  retained gaps. Output `/tmp/agiru-shared-table-properties-apps.FzGbg1` has no
  added/missing file paths versus apps; 274 declaration files change and all
  codeunit files remain identical. Independent UT census: unchanged 80/2,314,
  zero gained/lost identities (`/tmp/agiru-shared-table-properties-ut-identity-comparison.json`).
  Complete projection-only replay: 123 cases/223 tooling green
  (`/tmp/agiru-table-metadata-projection-local-tests.log`). Final shared-emitter
  replay also passes 123 cases/223 tooling
  (`/tmp/agiru-shared-table-properties-final-local-tests.log`).
  Live-provider guards remain; this batch is in the completed snapshot above,
  not the completed native integration below.
- Completed native qualification/scope integration `20261003T233302Z-1193251`:
  HEAD `108944f`, source
  `8616a740d520103752dd626837f0763ac5be9456d3966c25c1c9a5cf7ce6961c`,
  original System dependency
  `34c40f0dcc839eb4d244398715e11a801194bfcea69a21257c5de20a9833a955`;
  frozen BCApps `bb7111877f`. Finished 00:45:38 UTC in 4,321 seconds:
  `slice-check/all/test=0`, `ut=2`; 123 cases/223 tooling green.
  UT: 2,161 passed/153 failed, zero incomplete, 80 codeunits, six workers,
  1,667 seconds. All 2,314 identities/statuses/diagnostics match the preceding run;
  no gains/losses/missing cases (`/tmp/agiru-native-qualification-ut-comparison.json`).
  Source/package/notice pre/post hashes agree. Null/unsealed seed: diagnostic
  repeatability, not sealed A/B proof. Minimum 2,204/G1 remains unmet.
  Final regeneration `/tmp/agiru-transpile.zsOsIb` remains
  exit 1 for counted native/app gaps, with 18 qualified tables, 80/2,314 UT and
  14,225 slice inputs/zero missing. Generated comparison adds only platform/native
  and changes the absent types/DataClassificationEvalData pair; no old path losses
  (`/tmp/agiru-native-product-scope-generated-diff-final.log`).
- Completed catalogue integration `20261003T213424Z-1064496`: frozen HEAD
  `108944f`, source
  `0990a42f68e1219c180e9f2d9f34267ed4f21555e4d1412cefd62c0f7a6d3820`,
  explicit original System package hash `34c40f0dcc839eb4d244398715e11a801194bfcea69a21257c5de20a9833a955`.
  Finished 22:40:57 UTC in 3,957 seconds: `slice-check/all/test=0`, `ut=2`;
  123 local cases/219 tooling green. UT: 2,161 passed/153 failed, zero incomplete,
  1,567 seconds/six workers. All 80/2,314 identities, statuses and diagnostics match
  the preceding run; no gains/losses/missing cases
  (`/tmp/agiru-catalogue-freeze-ut-comparison.json`). Source/package/notice pre/post
  hashes agree. Null/unsealed seed: diagnostic repeatability, not causal A/B proof.
  Minimum 2,204/G1 unmet. The following native qualification batch is not included.
- Development native qualification: one explicit matching source/ABI pair installs a
  canonical declaration consumed by typed Record and RecordRef. Duplicate bases,
  qualifiers and conflicts refuse; nine freeze paths/five controls pass. Catalogue
  54/GenNativeBinding 85 checks green. `make test JOBS=2`: 123 cases/219 tooling green
  (`/tmp/agiru-native-source-local-tests.log`); additional concurrent native-cache
  replay is green (`/tmp/agiru-native-source-final-catalogue.log`). Catalogue targeted
  analysis and eight-header dependency controls pass; the new narrow header avoids
  record/session dependencies. Final original-package receipt
  `/tmp/agiru-native-bindings.WvH4lc`: 234 raw tables retained; 233 selected
  (18 pass/215 unbound), one original commercial licence table explicitly excluded
  by root scope.json with a separate system-symbols source domain (0725).
  The unused licence adapter/registration/generator binding is removed, without DB
  changes or a success stub. All 18 qualification candidates compile
  without PCH; their separate DSO passes 221 checks. Dropped-library/wrong-namespace
  controls reject; the actual SourceRunner targeted analysis is green. No full-app,
  live-provider or native-qualification UT proof. Production regeneration
  `/tmp/agiru-transpile.zB9cfB` retains all 80/2,314 UT and 14,225 slice sources,
  zero missing. GenScope 304 checks, 44 focused tooling tests and final
  `make test JOBS=2` (123 cases/223 tooling) green
  (`/tmp/agiru-native-product-scope-local-tests.log`). Targeted Apps/NativeSource
  analysis passes. Independent source identity comparison retains all 80/2,314,
  zero added/removed (`/tmp/agiru-native-product-scope-ut-identities.json`). Mixed
  DataClassificationEvalData still refuses its commercial classification branch;
  preserve required privacy/user classifications and finish declared separation in
  0725. Rebuild every changed TableEntry ABI consumer before execution.
  Next: selected native integration, then complete read-only metadata
  providers with authoritative absent-property semantics (0033/0044).
- Completed property/native-owner run `20261003T205744Z-997579`: HEAD `108944f`, source
  `ae81e3835223a22e250af3de395a1b6fb6164ace0eb20b962f02baad4615fd62`,
  explicit original System dependency
  `34c40f0dcc839eb4d244398715e11a801194bfcea69a21257c5de20a9833a955`.
  Finished 21:31:12 UTC in 1,973 seconds: `slice-check/all/test=0`, `ut=2`;
  121 cases/219 tooling green. UT: 2,161 passed/153 failed, zero incomplete,
  1,607 seconds/six disposable clones. Every 80/2,314 source/method identity,
  status and diagnostic matches the preceding run: no gains/losses/missing cases
  (`/tmp/agiru-property-native-owner-ut-comparison.json`). Source/package/notice
  pre/post hashes agree. The following catalogue-freeze batch is not included.
  Seed identity remains null/unsealed, not causal A/B proof. Minimum 2,204/G1 unmet.
- Development catalogue batch: one table/page/codeunit/profile registration boundary
  freezes on first read; duplicate numbered identities, null declarations and late
  registrations refuse. CatalogueGate 36 checks, nine first-reader paths and three
  negative controls pass (`/tmp/agiru-catalogue-freeze-controls.log`). Targeted
  runtime/gate analysis is green. Final `make test JOBS=2`: 123 cases/219 tooling
  green, including missing-script refusal after updating its independent fixture
  (`/tmp/agiru-catalogue-freeze-local-tests-final.log`). Native ownership,
  profile composition and other object-kind registries remain open (0033/0044).
- New property/native-owner batch: 120 reflection checks and seven negative controls
  green; 18 native-source tests retain present table-only manifest ownership and
  reject invalid identities/DTDs/symlinks. The old compiler refuses the legal takeover.
  `make test JOBS=2`: 121 cases/219 tooling tests, zero red
  (`/tmp/agiru-metadata-property-native-owner-alias-final-tests.log`). Full regeneration
  `/tmp/agiru-transpile.6k2ojo` preserves 80/2,314 UT and 14,225 slice inputs/zero missing;
  generated apps remain byte-identical to the completed snapshot below. Original
  package provenance is verified; translation still refuses the existing native/app
  gaps. No live provider or UT gain claimed (0033/0044).

- Completed table-owner integration `20261003T200855Z-894902`: frozen HEAD
  `108944f` plus source hash
  `4930e0d8a040849e6b8ba55b9007e352e26e16af01b7bd2151081f4287b19108`.
  Finished 20:39:59 UTC in 1,828 seconds: `slice-check/all/test=0`, `ut=2`;
  121 local cases/215 tooling tests green. UT: 2,161 passed/153 failed,
  zero incomplete, 1,310 seconds/six workers. All 80/2,314 source/method identities,
  statuses and diagnostics match the preceding run exactly; zero gains/losses
  (`/tmp/agiru-table-owner-ut-comparison.json`). Frozen source/notice hashes agree.
  This snapshot has no separate original-System-package dependency receipt;
  `/tmp/agiru-transpile.uxlEhy` supplies the earlier verified generation provenance,
  not a package-frozen A/B claim. The null/unsealed seed remains diagnostic only.
  Subsequent property mappings/table-only native identity changes are not included.

- Subsequent table-owner batch: root compilation-unit manifests take precedence over
  component manifests; unmanifested groups retain bounded source owners. Private
  yyjson reads/validates original root identities; native manifests share GUID checks.
  GenTableGate 78/zero red; generated table-keys 36/zero red; meaningful source and
  metadata controls reject, including the previous compiler's legal-takeover refusal.
  `/tmp/agiru-table-owner-composition.log`, `/tmp/agiru-table-keys.4mIYcy`.
  Full regeneration `/tmp/agiru-transpile.uxlEhy` preserves 80/2,314 UT and
  14,225 slice inputs/zero missing. Versus the preceding frozen generated tree:
  six new module headers, 63 table definitions changed, no removals
  (`/tmp/agiru-table-owner-generated-diff.log`). Regeneration remains exit 1:
  215 unbound native tables/125 other inactive native sources and existing app gaps.
  Complete `make test JOBS=2`: 121 local cases/215 tooling tests green
  (`/tmp/agiru-table-owner-composition-local-tests.log`). Targeted AppManifest,
  NativeManifest and generated consumer analysis is green; Main/GenTable retain
  inherited findings, no new suppression/baseline. Native table owners, deployed
  composition and live Table Metadata projection remain open (0033/0044).
  Full integration above retains the complete 2,161 baseline without losses.

- Completed text/Decimal integration: `20261003T184820Z-806432`; frozen HEAD
  `108944f` plus source hash
  `a5273ffc0fc217bbb636b0d03a0a095b01cfdd315a9ff4096dfc2082728f9d1a`.
  Finished 19:47:42 UTC in 3,527 seconds: `slice-check/all/test=0`, `ut=2`;
  121 local cases/215 tooling tests green. UT: 2,161 passed/153 failed,
  zero incomplete, 1,439 seconds, six disposable database clones. The same
  80 codeunits/2,314 unique method identities remain: one gain,
  `Data Exch. Exp. Latin Char UT::TestPreMappingExportDataJnlPreserveFALSE`,
  zero losses/missing/added cases (`/tmp/agiru-text-decimal-ut-comparison.json`).
  Source/package/notice pre/post hashes match. The null/unsealed seed still
  precludes sealed-seed causal A/B claims. Minimum 2,204 and G1 remain unmet.
  The subsequent table-source-identity batch is not included in this run.

- Decimal batch (0066), not included in `20261003T172807Z-701995`: exact wide-integer
  modulo no longer requires a representable Decimal quotient; smaller dividends
  retain their scale. Parsing rejects both signs of 2^96. DecimalGate: 59/zero red;
  negative controls reproduce quotient overflow, two accepted out-of-range values
  and two changed-scale results. `/tmp/agiru-decimal-{mod-before,parse-limit-before,
  mod-scale-before,mod-scale-after}.log`. Scale-28 arithmetic remains open.
  Final runtime/gate targeted analysis is green, without a baseline increase;
  combined text/Decimal `make test JOBS=2`: 121 cases/215 tooling tests green
  (`/tmp/agiru-text-decimal-local-tests.log`). Full replay above retains
  every UT identity without a loss; scale-28 Decimal conformance remains open.

- Subsequent text-position activation (0066), not included in frozen run
  `20261003T172807Z-701995`: generated `At`/`CharAt` now reads and replaces UTF-16
  positions instead of UTF-8 bytes, encodes integer assignments and copies same-type
  proxy values. `TextGate` 64/zero red; old code reproduces damaged UTF-8 and refuses
  a Unicode literal. Ten generated AL checks pass; changing the source index fails
  (`/tmp/agiru-text-position-{before,after,generated}.log`). Half-surrogate operations
  still refuse explicitly. Targeted analysis has inherited header findings, no new
  primitive finding. Final `make test JOBS=2`: 121 local cases/215 tooling tests
  green (`/tmp/agiru-text-position-local-tests-final2.log`); the new fixture remains
  counted/refused if its script is missing. Full replay above gains one Latin-export
  case without a loss; half-surrogate parity and other encoding errors remain open.

- JSON/XPath batch on uncommitted HEAD `108944f`: system yyjson replaces the vendored
  header; `third_party/` and its fixture-copy dependencies are removed. Exact raw
  number tokens and stable retained nodes are shared by AL/.NET wrappers (0722).
  JSON 231/.NET JSON 43 checks pass both normally and under ASan/UBSan with leak
  detection; original-HEAD control reproduces heap-use-after-free and large-number
  conversion refusal. `/tmp/agiru-yyjson-{local-tests,asan-al-retry,asan-dotnet,
  negative-old,negative-alias}.log`. `make test JOBS=2`: 120 local cases/215 tooling
  tests green. Subsequent XPath helper-only split preserves 79/zero red checks;
  the original engine has 14 red (0035). Private JSON engine targeted analysis is
  green; existing wrapper/XML/header findings remain red, with no baseline increase.
  Full lint formats cleanly but refuses five missing specialist compile commands:
  reporting `{NativeRunner,RegistryRunner}`, native-binding `{Emit,PageRunner}` and
  native-enums `EmitContracts` (`/tmp/agiru-json-batch-lint.log`).
  Regeneration `/tmp/agiru-transpile.JCMJlS` retains native refusals (exit 1),
  14,225 slice inputs/zero missing. Frozen replay `20261003T172807Z-701995`
  finished 18:30:10 UTC in 3,688 seconds: `slice-check/all/test=0`, `ut=2`;
  local 120 cases/215 tooling green. UT: 2,160 passed/154 failed, zero
  incomplete/crashed, 1,380 seconds, six disposable database clones.
  HEAD `108944f` plus source hash
  `fd2359d669ae7a2654750a394feb92af59ddc5fe1be0f59babb0f82582b48239`.
  Independent 80/2,314 manifest identities match the confirmed baseline exactly
  (`/tmp/agiru-json-xpath-ut-{current,baseline}-identities.json`).
  Comparison: one gain, `XML DOM Management UT::CheckElementTextWithEmptyNamespace`,
  zero losses; source/package/notice hashes match before/after
  (`/tmp/agiru-json-xpath-ut-comparison.json`). The null/unsealed seed still limits
  causal A/B claims. The preceding 2,159/2,314 result below is historical.
  JSON parent/insertion/Path and unimplemented
  AL API contracts, high-scale typed JSON conversion through the scale-20 Decimal
  core (0066), XML DTD/cursor policy, native resources and WASM remain open.

- Preceding UT baseline: BCApps `bb7111877f`, System 29.0.55365.0;
  80 codeunits/2,314 source-counted methods. Frozen HEAD `49ce9c0` plus source
  `ae8dfcc314d84c62ae046f9f0079b732b4f63ba1e49ba97464ea51afcc72d176`,
  `/tmp/agiru-verify/b3fb41b94d2994ba/20261003T135330Z-494713/result.json`,
  finished 2026-10-03 14:48:19 UTC, 3,254 seconds. Target exits:
  `slice-check/all/test=0`, `ut=2`; 14,225 slice inputs/zero missing,
  local 120 cases/215 tooling tests green. UT: 2,159 passed/155 failed,
  zero incomplete/crashed, 1,200 seconds. Every identity and status matches
  `20261003T132242Z-466167`; zero gains/losses after removing the unused Storage.h
  PCH include. Source/package/notice pre/post hashes agree. Legacy seed identity
  is null: diagnostic baseline, not sealed-seed causal A/B proof (0718).
  Earlier 2,062→2,159 recovery gained 97 without losses. Minimum 2,204 and G1
  remain unmet. Table Metadata refuses 43 methods across eleven codeunits (0044);
  zero aborted SQL transactions after the NumberSequence fix (0723).
  VIES remains 41/43 with two missing audit consumers; no successful no-op accepted.
  Previous `20261003T111104Z-333469`: build/link green;
  UT 0/2,314, all 80 incomplete before methods
  because moved source fields reach the native metadata option mapper. All C++
  gates pass; local test total is 120/one red in a snapshot fixture inheriting the
  production notice override. The fixture now isolates that environment; all 16
  snapshot tests pass with an inherited override. Moved-field merging now validates
  actual source/destination app IDs: 82 takeovers resolve, 31 unavailable W1
  destinations stay individually reported; no SDK option invented (0033).
  NativeSourceCompilerGate passes 14 tests; old compiler fails the three new
  controls. Regeneration preserves 80/2,314 UT and 14,222 slice inputs/zero missing.
  Previous `20261003T105744Z-318804` builds/links successfully,
  but all 80 runners refuse before executing a method: legacy All Profile.Description
  is varchar(250), original metadata declares Text[2048]. Current schema bootstrap
  widens existing bounded Text/Code columns generically; never narrows or truncates.
  StorageGate passes 63 checks; the source notice is now independently frozen/hashed
  for the layout qualifier. Existing snapshot controls pass all 16 tests.
  Previous `20261003T102008Z-256835` refuses at linking
  after compilation: Environment Information table 3703
  and Users in Plans query 774 lack their generated metadata units in the slice.
  Both original units are now appended; no current executed-UT pass count yet.
- Previous recovery attempts `20261003T100138Z-227433` and `20261003T100605Z-230255`
  refuse on privacy permission metadata and initializer order respectively. Every
  one of the 2,314 methods remains incomplete; neither is an executed-test result.
  Current root PlatformSourceGate passes 2,917 checks; ObjectCatalogueGate 345;
  NativeSourceCompilerGate 11 tests. Original System-29 definitions repair source
  guards; no suppression or licensing SDK repair.
- `20261003T100816Z-231331` refuses on a generic native-field/record-method collision
  in the unchanged CRMNotesSynchJob source: RecordId property syntax called the
  Record ID value. `BodyWriter::Link` now preserves method identity for explicit/
  implicit calls independently of quoted field access. GenCodeunitGate passes 41
  checks, including four controls; GenScopeGate passes 300. Regeneration retains
  80/2,314 UT. Targeted BodyWriter analysis remains nonzero on existing findings;
  Link complexity increases to 41 and needs simplification in 0073. No baseline raised.
  Full local replay finds unnecessary base qualification of an ordinary TableCaption
  property; the refined call/value distinction preserves its existing regression.
  GenSourceBinding passes 159 checks and GenCodeunit 41. The complete root replay
  `/tmp/agiru-ut-recovery-local-tests-final.log` passes all 120 local cases and 212
  toolchain tests, including verified original System-29 definitions.
- Test organization: `test/{gate,runtime,transpiler,reporting,tooling}`, with original
  AL fixtures and C++ golden specifications retained. Frozen `20261003T094423Z-212143`
  (`1b1c63a87f525737a9b32ee7fd51b3ec47c3161a0ef8e2f7ddd0bcdb5c702b43`)
  passes `tc test`: 121 local cases/212 toolchain tests, zero red/skips, 271 seconds.
  The redundant PlatformSource shell wrapper is subsequently removed; its gate runs
  once with verified package arguments when supplied. Expected local total is now
  120, with the same 105 C++ gate sources and 212 toolchain tests. Discovery controls
  pass. The unused `scripts/dropped_properties.py` heuristic is also removed;
  production property diagnostics and their controls remain. The disconnected historical
  `scripts/compile_cost.sh` recipe is removed; `make include-cost`, compiler time traces
  and current build receipts remain the measurement entry points. No AL UT identity is removed.
- Slice reconciliation: 14,222 → 14,220 → 14,222 explicit inputs, zero missing. One upstream
  retired upgrade codeunit is replaced by source-generated platform layout declarations
  (0063); one pure commercial implementation is explicitly excluded by `scope.json`
  (0725). Original identities/reasons remain slice comments. Original generated
  Environment Information metadata and Users in Plans query are appended to satisfy
  actual link dependencies; no provider/cloud/licensing functionality is fabricated.
  Original UT population
  and mixed callers remain selected; runtime commercial/service calls still refuse.

- Previous native-signature dependency batch: frozen `49ce9c0` plus source hash
  `fce9a2314e1c35a87f1644ee2ac18169c18276e2603ce8cb7b526fc7c97e3e2a`,
  `/tmp/agiru-verify/b3fb41b94d2994ba/20261003T082641Z-59227/result.json`:
  `tc test` passes in 242 seconds; 119 local cases/210 toolchain tests, zero red/skips.
  GenInterface passes 35 checks; its targeted analysis is clean. Writer analysis retains
  exactly eight identical before/after diagnostics; no baseline/suppression increase.
- `/tmp/agiru-native-interface-package.Wya3h7/result.json`: independent original
  interface population 10/10 headers/six default-body files compile without PCH;
  missing-include control rejects. Full-generated native interface replay improves
  13/16 to 16/16 files (`/tmp/agiru-original-interface-{before.sp23BC,after.gnCGsT}/result.json`).
  Unknown parameters, returns and nested arguments now retain declaration dependencies
  and refusal counts. Context types/providers/hooks/data-driven execution remain unimplemented
  (0034/0039); compilation is not native business execution.
- `/tmp/agiru-native-signatures-full/comparison.json`: full generation retains all
  24,444 paths, zero additions/losses; only ITestHandler/ITestDataSource headers change.
  Compiler 1/Make 2 remain: 215 unbound native tables, 125 other unactivated sources,
  221 absent AL types/1,864 members and 399 .NET types/1,379 members.
  `/tmp/agiru-native-signatures-census/scope-inventory.json`: fresh independent census
  retains 113,013 raw/112,998 required methods, zero unmeasured files; the same three
  errors/seven conditional variants still refuse. Source revision/hash, scope, populations
  and errors match the previous census. Configured UT retains 80/2,314 methods,
  unexecuted; root apps/scope/slice stay unchanged. Full linking/UT/G1 remain open.

- Previous native enum/interface batch: frozen `49ce9c0` plus source hash
  `9160608ae427f0afb0331d28baf432516010044227d3a45219dacd25acc2a178`,
  `/tmp/agiru-verify/b3fb41b94d2994ba/20261003T080855Z-26079/result.json`:
  `tc test` passes in 231 seconds; 119 local cases/210 toolchain tests, zero skips/red.
  Authored native interface inheritance/AL dispatch/var effects pass 15 runtime checks;
  changed argument mode, sparse ordinal and caption controls reject. Actual app/slice
  objects compile. These are authored-consumer proofs, not native provider/business/G1 proof.
- `/tmp/agiru-native-enum-package.DJ63ew/result.json`: all 28 original System-29 enum
  contracts compile; wrong-ID control rejects, source/compiler/package hashes agree.
  Independent raw inventory retains 398 AL files/368 objects/10 interfaces/28 enums.
  Full generation in `/tmp/agiru-native-interface-full/generated` writes 24,444 paths;
  native interfaces emit ten headers/six default-body sources, native enums 28 headers/three
  sources. Tables retain 19 bound/215 unbound, other unactivated AL files are 125; full
  translation remains compiler 1/Make 2. Method-type/implementor activation remains
  open; original interface declaration/body compilation is proved above, not business execution.
- `/tmp/agiru-native-interface-census/scope-inventory.json`: fresh independent census
  retains 113,013 raw/112,998 required methods, zero unmeasured files; three errors/seven
  conditional variants still refuse. Independent `scripts/ut_manifest.py` and full
  translation both retain 80/2,314 UT; root apps/scope/
  slice are unchanged. Targeted analysis now checks an explicitly configured unit without
  requiring unrelated fixture commands; full analysis still refuses missing handwritten
  commands. Its two new controls and existing tooling pass 35 tests. NativeSource analysis
  is clean; Main retains seven diagnostics (including oversized Scan), none names the
  new interface helpers. This is not a full analysis pass; no suppression/baseline increase.
  Next: 0034 native types/
  original-consumer closure and full AL compile/link/UT, not client work.

- Requested cleanup (2026-10-03): removed 45 GB `build/` and 11 GB `build-asan/`;
  all `build/` receipts referenced below are historical and no longer present.
  Handwritten sources, uncommitted changes, generated `apps/`, downloaded `work/`
  inputs and external development worktrees are preserved. New temporary fixtures and
  frozen verification sources use `/tmp` (inspect its current mount/space); incremental objects
  may use `build/`. `make verify-check` passes all 19 snapshot/tooling checks after
  relocation; shell syntax and patch whitespace checks pass. No full rebuild or AL
  milestone pass follows from this cleanup.
- Before cleanup, the current native-enum batch passed 119 local cases and 200 toolchain
  tests, zero red; original System-28/29 enum contracts passed 16/16 and 28/28 with
  wrong-ID controls. The full raw/required population remains 113,013/112,998 and exact
  configured UT identities remain 80/2,314, business-unexecuted. Full translation and
  changed-code lint remain nonzero; native interface execution/full linking/G1 are open.

- Current code `cb5d127`: fifteen native classes inherit the common base system field numbers; AllObj/AllObjWithCaption/Feature Key metadata now retain the five already-present typed members. Original declared fields/keys/indexed FieldCount stay unchanged. Native source contracts check implicit names/types/numbers/member offsets and refuse explicitly declared reserved IDs. Commercial Tenant License State is untouched. Runtime-18's additional Normal/Temporary FlowFields, complete native SDK/providers/schema and G1 remain open (0034/0013).
- `build/platform-system-fields-{before,after,gate-qualified,generator,catalogue}.log`: base-system gate 828 checks, actual predecessor 195 red/current zero; native binder 77 and catalogue 294 green. `platform-system-fields-tests-committed.log`: 118 local cases/188 toolchain tests green, zero skips. Native matrices `build/native-bindings.{W7K9m3,08I5l6}/result.json` retain all System-28/29 identities/statuses: 18/14 pass, 1/5 fail, 204/215 unbound. Original package and source/library hashes verify; wrong implicit number/type/offset controls reject. Receipts were frozen from `83931af` plus the byte-identical committed source batch; no provider/business-execution claim.
- `build/native-consumers.{iMqCyA,sEZXNe}/result.json`: every original eight-unit result is unchanged in all three variants. System 28: ordinary 7/8, extra contracts 7/8, source-bound 8/8; System 29: 7/8, 5/8, 6/8. Exact 80/2,314 UT identities and full 113,013 raw/112,998 required population match predecessors; all AL methods remain unexecuted. `platform-system-fields-population-comparison.json` records equality. Both replay targets remain nonzero. `platform-system-fields-slice.log` retains all 14,222 inputs and the missing retired report-upgrade source; no removal/stub.
- Changed-code lint checks 53/222 units, 52 fail on retained diagnostics; it is not green. The writer's 33 before/after diagnostics match exactly; the new gate's own nine findings are fixed, inherited header findings remain. No touched native header/new gate/binder diagnostic and no baseline/suppression increase. Receipts: `platform-system-fields-{lint.log,lint-tidy.log,lint-units.json,writer-diagnostics-{before,after}.txt,gate-qualified-tidy.log}`.
- Previous code `eb8aa06`: named-layout manifests and content-addressed asset bundles preserve declaring app/version/source, original bytes and third-party notices. Nested source projects own their layouts; portable separators/dot components normalize without allowing traversal or symlinks. No runtime installer/default/approval or renderer is implied.
- `build/report-assets-full-committed.vhjvEf/`: verified full translation writes 24,397 paths, one new manifest/no lost path. Exactly four report definitions change to correct seven previously ownerless test layouts; all other existing generated files match. All 423 named assets package/verify (306 RDLC, 85 Word, 32 Excel), including sixteen native/extension parts. Original Microsoft license is retained. All 423 immutable declarations from 327 reports compile; ownerless control rejects (`metadata.log`). Legacy asset coverage remains open; this is not an independent full-asset census.
- `build/native-report-layouts.UTnQxW/result.json`, frozen `eb8aa06`: both original/source-bound variants pass 562 checks; actual app/fixture-slice links and registry/drop controls remain green. Sixteen assets package/verify with original ownership/notices. Installed/approved/selected/rendered: zero; all 368 native objects remain business-unexecuted.
- `build/report-assets-tests-committed.log`: 117 local cases/188 toolchain tests green, zero skips. Targeted writer/gate analysis is clean; Main retains the original eight diagnostics after the new parameter-name warning is fixed. Changed-only lint after commit checks zero/221 units, then refuses the existing 55 silent places/baseline 13; it is not an analysis pass. Before/after directive lists are identical; no baseline/suppression increase. Receipts: `report-assets-{lint-committed.log,writer-tidy-committed.log,gate-tidy.log,main-tidy-qualified.log,main-diagnostics-{before,qualified}.txt,silent-{before,after}.txt}`.
- Raw census is canonically unchanged: 113,013 raw/112,998 required methods, zero unmeasured files; three errors/seven conditional variants still refuse. Exact 80/2,314 configured UT identities match and remain unexecuted. Root apps/scope/slice are unchanged. Full translation still exits 1/Make 2 on native gaps; installation/successor proof, complete-tree/link and G1 remain open.

Previous registry code `91190e8`; receipts use the matching dirty batch at frozen HEAD `4cbe2a8`, not a new
full integration. Native report loading shares the original AST/binder and XML module/layout
owners. `build/native-report-layouts.RBEaVo/result.json`: original/source-loaded variants each
pass 562 checks for sixteen layouts. Frozen CMake builds the platform library and actual CLI
in app and authored two-source slice modes. Registry-only lookup passes three checks per mode;
Linux `--as-needed` library-drop and ownership/property/asset/missing-module controls reject.
Static archive retention also passes its negative control. Direct generated-library foundation
dependencies are fixed; slice refusal generation subtracts platform definitions and emits no
helper for zero missing procedures. Compiler/source/package hashes agree. Root apps/scope/slice
are unchanged; this is not full-tree linking. Installed assets/rendered documents: zero;
all 368 native objects remain business-unexecuted. Log: `build/report-registry-native-link-proof-final.log`.

Full source replay: `build/native-consumers.wSWttg/result.json`, System 29 retains all
three eight-unit variants: ordinary 7/8, extra assertions 5/8, source-bound 6/8, no losses.
Both Objects units still reject AllObjWithCaption drift. Native parsing retains 234 tables,
nineteen bound/215 unbound, plus one bound report; 163 other AL files remain unactivated.
The raw table matrix remains 14 pass/five fail/215 unbound
(`build/native-bindings.wlGUVy/result.json`). Native translation exits 1; Make exits 2.
Full generation retains 24,389→24,396 paths, zero lost; seven additions are the platform
module/report/reaches and query 774 header/body (0725 classification still open).
Layouts grow 407→423 immutable declarations; all fourteen extension layouts are now bound.
Raw census remains 113,013 raw/112,998 required methods; exact 80/2,314 UT identities match,
all unexecuted. Previous System-28 eight-unit qualification: `native-consumers.vWqQvV`, 8/8
source-bound; this batch's full replay used System 29, without changing demo 28.4.

ReportRegistry.h now owns the existing entry/lookup declarations without dataset/page
dependencies. Signature, size, alignment and field-offset controls pass; forced full-report
inclusion fails the dependency profile. One registry/unchanged ABI, not new execution semantics.
Local final replay: 115 cases/188 toolchain tests, zero red/skips
(`build/report-registry-tests-final.log`). Both registry/native consumers analyse cleanly;
changed lint checks five of 219 handwritten units, three still fail on unchanged public
headers and XmlPort findings. No own finding remains in changed Report.cpp/ReportGate code.
Formatting and new public documentation pass; no suppression/baseline increase or full-analysis pass.
Receipts: `build/report-registry-{lint-final.log,tidy-final.log,lint-units-final.json,targeted-tidy.log,doc.log}`.
Three-round/no-PCH header frontend sample: Report.h 1,794.5 ms, ReportRegistry.h 213.8 ms
(`build/include-cost.WqMlqU/results.tsv`); consumer syntax-only wall time averages 1.561→0.385 s
(`build/report-registry-header-{before,after}-bash.log`). Concurrent probes; not an ERP performance claim.
Earlier Manifest/SourceReader/NativeRunner analysis is clean; Main still has eight findings,
including Scan complexity 95/382 lines (`build/native-report-loader-main-tidy-qualified.log`).
Next: qualify full platform/tree linking, install owned layout assets and prove the retired
report-upgrade successor before slice reconciliation. Providers, native kinds/methods,
namespace/app identity, SymbolReference joins and G1 remain open (0034/0038/0063/0589).

Latest metadata source batch: `2d3f2de`; snapshot override fix: `599597c`. Receipts
were collected from the matching dirty batch at predecessor HEAD `8d65ae9`, not a
new full frozen integration.

- Page/Table Metadata (0034/0044/0013): original 32/23 fields, Caption Text[80], exact option vocabularies and `pk`/implicit `ID` keys; thirteen source families/2,917 checks/33 rejected source mutants. Named property projections, temporary records and unavailable-live-provider boundaries: 59 checks/four rejected implementation controls. Both named/numeric AL consumers execute all seven option vocabularies and Insert/Get/Count with independent Name/Caption. Partial physical seeding is removed; existing rows remain untouched. Provider/schema migration remain open, not empty-snapshot success.
- Terminal same-population native replay: System 28 18 pass/one fail/204 unbound of 223 tables; System 29 14/five/215 of 234. Exactly Page/Table Metadata gain, zero losses; both original-page primitive/negative controls retained. `build/native-bindings.{dP2MsX,mbyRj1}/result.json`, `build/reflection-metadata-matrix-final-comparison.json`; exit 2, source/library/package hashes agree. All 333/368 native objects stay business-unexecuted; commercial retirement, full properties, providers, eight-consumer replay, generated rebuild, production loader and G1 remain open.
- Metadata batch: 115 local cases/155 toolchain tests green, zero skips, with verified original System 28 (`build/reflection-metadata-tests-original28-final.log`); source gate 2,917/zero red, runtime controls `build/reflection-metadata.v8Bn7g/`. An actual two-test snapshot failure exposed inherited Make overrides bypassing frozen symbol paths; runner now clears all three override channels and strengthened controls pass. Earlier `/tmp` exhaustion and concurrent-link attempts remain failed receipts, not passes; final temp work uses `build/tmp`. Census byte-identical: 113,013 raw/112,998 required methods, seven conditional refusals/exit 2, zero unmeasured (`build/reflection-metadata-census-{before.json,log}`). Apps/scope/slice unchanged; no additional AL business execution.
- Analysis: seven/216 units checked, seven fail; formatting passes, no finding in changed metadata headers or gates. Mapper retains three include-cleaner errors reproduced by a standalone `<expected>` input; other findings are inherited. `build/reflection-metadata-{lint-final.log,tidy-final.log,lint-units-final.json,expected-probe.log}`; no suppression/baseline increase or lint-green claim. ProvisionInstalled/ProvisionDates now fit 120 lines; retained Date-seed body is unchanged apart from the equivalent early-return guard. Three-round/no-PCH header samples under concurrent load: Page 1,541.3→1,566.5 ms, Table 1,542.9→1,580.7 ms; narrow ReflectionTypes 355.4 ms (`build/reflection-metadata-header-{before,after}.log`), not a performance improvement claim.

- Previous implicit-key batch (0034): common completion before extensions/system fields, exact lowest-ID name/default clustering and strict ordinary/native flags. 34 generator/61 binding/14 generated temporary checks; old emitter fails four checks, two metadata controls reject. 113 local cases/155 toolchain tests green (`build/primary-key-tests-final.log`); native baseline `build/native-bindings.{AGsjCQ,U8JB0L}/result.json`. Census unchanged; two inherited failing units of six/214 analysed, new normalizer/gate/runner clean. Current metadata declaration proofs supersede its two contract failures; all eight original consumers remain next.

- Native-family declarations (0034): Company/User/Date/Integer global; User email Text[250]; Record Link Text[132] users, removed URL Text[250]/Memo/Company relation/three original keys; All Profile seventeen fields/Text[2048] Description/sole PK. Eleven source families/2,004 checks green, twenty-five mutants reject. Full same-population matrix gains six contracts/loses none in each package: System 28 16 pass/3 fail/204 unbound of 223 tables; System 29 12/7/215 of 234. Both original-page primitives/mutants retained; all native objects business-unexecuted. `build/native-bindings.{bqtDwn,TB7yAe}/result.json`, `build/native-families-matrix{28,29}-comparison.json`; terminal exit 2. Page/Table Metadata, implicit key/system-field/property completeness, providers, eight-consumer replay, rebuilt generated apps and production loader/G1 remain open. Do not repair commercial licensing (0725) to turn the raw audit green.
- Native-family batch: 111 cases/155 toolchain tests green, no skips, offline and with verified original System 28; actual original-source gate 2,004/zero red (`build/native-families-tests{,-original28}.log`). Census byte-identical: 113,013 raw/112,998 required methods, same seven conditional refusals/exit 2, zero unmeasured (`build/native-families-census-{before.json,log}`). Runtime ABI/gates rebuilt together; root apps/scope/slice/baselines unchanged, no extra AL business execution. Lint seven/211 units/seven red on inherited findings, none in the changed headers/gate (`build/native-families-{lint,tidy}.log`). Header frontend samples under concurrent load are recorded in 0034, not a performance claim. Existing populated schemas still require explicit migration/provenance.
- Curated native binding API (0034): 59 generator checks green; source-owned fields/options, explicit native-header ownership and strict compile-time contract emission/duplicate-field refusal. Full independent package matrix: System 28, 333 objects/223 tables, ten contracts pass/nine fail/204 unbound; System 29, 368/234, six pass/thirteen fail/215 unbound. Every bound candidate compiled without PCH; original field-number mutants fail; source/package/library hashes remain unchanged. Original Page Fields Selection List body/definition compile under both packages: six primitive checks green; wrong source expression fails four checks. `build/native-bindings.{MP5Cxr,1I0U2X}/result.json`, `build/native-bindings-{28,29}-final.log`; both terminal exit 2. All objects remain business-unexecuted; full properties/system-field population, eight-consumer replay, production loader and G1 remain open. Commercial-family defects are raw evidence, not repair requirements (0725).
- Native-binding batch: 111 local cases/155 toolchain tests green, zero skips (`build/native-binding-tests-final.log`). Final lint formats cleanly, analyses 24/211 units/twenty failing units; new gate/emitter/page fixture each exits zero (`build/native-binding-lint-final.log`, `build/lint/tidy.log`). No suppression/baseline increase. Independent full BCApps census is byte-identical: 113,013 raw/112,998 required methods, same seven conditional refusals/exit 2, zero unmeasured (`build/native-binding-census-{before.json,log}`). Root apps, scope and slice unchanged; no additional AL business execution or G1 claim.
- Main Field contract (0034): 24 source fields, correct ExternalName 10/Text[100], compact native Type codes and separate classification/SQL/access options. PlatformField 261/PlatformSource 820 checks green; fourteen source mutants fail. Both generated AL aliases execute all 21 native Type members/new options without PCH. Verified System 28 passes; its Field source matches verified 29, which retains two privacy inherent-permission mismatches. `build/field-{declaration-tests-final,package28-verified-source,package29-source,page-binding}.log`. Native loader, full metadata/provider, eight-consumer replay, generated rebuild and G1 remain open.
- Field batch: complete local replay 110 cases/152 toolchain tests green, zero skips. Independent census byte-identical: 113,013 raw/112,998 required methods, same seven conditional refusals; exact configured UT 80/2,314 unchanged (`build/field-{native-census-before.json,native-census.log,ut-current-identities.json,ut-frozen-identities.json}`). No additional AL method execution claim. Standalone Field header frontend: 1,704→1,671 ms, three rounds/no PCH under concurrent load; not a performance improvement claim (`build/field-header-{original,after}.log`).
- Field analysis: runtime and both declaration gates each retain 33 findings in unchanged dependencies, none in the changed Field code/headers after direct-include corrections. Final targeted gate remains 261/0 red. Changed-code lint: formatted, 50/208 units analysed/all fifty fail on outstanding findings; not lint-green and no suppression/baseline increase. `build/field-{runtime-tidy-details,source-tidy-details,gate-tidy-final-details,declaration-gate-final,declaration-lint}.log`, `build/lint/tidy.log`.
- Native report contract fixture `5698734`/build prerequisite `f9f62a5` (0034/0063): verified System `29.0.55365.0`/Runtime 18 from public OnPrem `29.0.54011.55407` supplies original Tenant Report Defaults and two Word assets. Full native raw inventory: 398 files/368 objects/one report, zero unmeasured; SymbolReference still has zero reports. Production compiler fixture merges all fourteen Composite layouts, compiles/registers the original report and preserves sixteen immutable declarations/separate native/BaseApp ownership. 562 checks green; owner/property/asset controls fail, missing target retains all fourteen unresolved layouts. `build/native-report-layouts.T1cFzn/{result.json,raw-inventory.json,generation.log,runner.log}`. All 368 native objects remain business-unexecuted; no production loader, installation, selection, rendering, full-app or G1 proof. Demo pin 28.4 remains unchanged; BCApps app 30 is separately recorded.
- Terminal native-fixture batch: 110 local cases/152 toolchain tests green, zero skips; `build/native-report-layouts-tests-final.log`. Make explicitly builds the runtime before linking the fixture; fresh-directory dry-run regression protects that prerequisite, not a cold-build timing claim. Corrected new consumer analysis: 1/208 units, zero findings (`build/native-report-layouts-tidy-final.log`); changed-code lint formats/analyses cleanly but remains red at inherited 55 silent places/baseline 13, unchanged (`build/native-report-layouts-lint.log`). No suppression increase. Replayed complete BCApps raw census byte-identical to before; same seven conditional refusals/exit 2, no identity/hash loss (`build/native-report-layouts-census-{before.json,log}`). Root `apps/`, scope and slice unchanged.
- Latest completed frozen integration: `build/verify/20261002T190210Z-3462647`, HEAD `7d22864`, failed/2,030 seconds; all/UT/census exit 2, test exit 0 (110 local cases/149 toolchain tests green). Source/pre-post SHA256 `6db7d6a21df8b1eff0a57a3e665541e45115e8025d4c6bce8de41685cbd330a9` agrees; BCApps `bb7111877f`, frozen input SHA256 `af53219b7fc9e68a58293bad9fd178232d6bef47bbab2166b390620fb38497a2`. First build refusal remains root 085/missing UpgradeCompositeReportParts.cpp. UT retains exact 80/2,314 ID/name/method identities, zero executed/all 2,314 missing; canonical raw census matches current BCApps after removing only its absolute source root. `artifacts/ut.log{,.manifest.json,.results.jsonl,.run.json}`, `artifacts/census/scope-inventory.json`; comparisons `build/native-report-{ut-{before,frozen},census-{current,frozen}}.json`. No live integration or G1 claim; the newer native fixture is not part of this frozen batch.
- Atomic NumberSequence primitive (0723): 376 direct DB checks, 300 cross-process ranges and constant one/billion-value RPC counts pass; removed serialization/per-value/unbound-name mutants fail, controlled post-lock cancellation cleans up. Terminal local replay: 110 cases/149 toolchain tests green, zero skips (`build/number-sequence-tests-final.log`). Numeric registry IDs and bound qualified identities replace persisted hashes/interpolation; posting rollback retains consumption, lifecycle changes remain transactional, Exists does not block creation. Public header narrows (330 ms frontend/three no-PCH rounds, not a before/after speed claim). Changed-code lint analyses 4/207 units, three fail on inherited findings; new storage unit clean, no own sequence/gate additions, no suppression/baseline increase (`build/number-sequence-lint.log`). Legacy migration/company rename/isolation/permission/architecture/scale proofs remain open; no full AL execution or G1 claim.
- Reference refresh: BCApps `6261b1c458`→`bb7111877f`, developer docs `ff5939a46e` unchanged, user docs `634710c42`→`0ff62b2266`. Independent raw identities: 113,005→113,013 methods, eight additions/zero losses, fifteen exclusions leave 112,998 required; 36,874 files/36,783 objects, same seven conditional refusals, zero unmeasured. Configured UT identities remain exactly 80/2,314. `build/number-sequence-{raw-methods-{added,lost}.tsv,census-before.log,ut-{before,after}.json}`. Generated apps still describe the preceding source batch; current-source regeneration/link qualification remains 0038/0058. Slice remains 14,222 inputs with missing retired UpgradeCompositeReportParts.cpp; no compatibility stub or entry deletion.

Latest immutable layout/capability code: `c62879f`; receipts below describe this source batch.

- Immutable layout declarations (0063): 416 emitted layouts from 326 actual reports compile; fourteen native-target layouts remain unresolved/not emitted. Two-app execution retains typed object IDs, app/source ownership, localized tokens, obsoletion metadata and worksheet overrides. Unknown/refused properties and ownership/discard controls fail; source-only invocations still audit layouts and report zero emitted declarations. Terminal replay: 108 local cases/149 toolchain tests green, 27 fixture/73 actual-source/39 compiled checks. All 24,389 paths retained; only 652 report headers/definitions change, business bodies and exact 80/2,314 raw UT identities unchanged. Capability census counts 325,464 properties, including 88 previously missed declarations; installation/rendering/obsoletion/workbook behaviour remains explicitly partial. Analysis widened to 122/205 units, initially 119 red; final new writer/runner and layout gate clean, callers retain inherited findings only (Scan 100→89, WritePage 118). Raw census unchanged: 113,005 methods/fifteen exclusions, same seven conditional refusals/exit 2. `build/report-layouts-metadata-{tests-source-only.log,generation-source-only.log,actual.log,lint.log,targeted-status.txt,census.log}`. No denominator loss, baseline increase, complete-app or G1 claim.
- Runtime naming follow-up: `src/rt/TypeMethods.cpp` names the type-method implementation unit; `RefuseUnimplemented` replaces the opaque refusal helper without aliases. Generators/reproduction controls/public Doxygen agree. Previous source, mechanically renamed/formatted, compares byte-identical; no include widening or changed refusal text. `build/type-methods-tests-final.log`: 106 cases/149 toolchain tests green, zero skips. Changed-code lint analyses 3/202 units, all three red; `build/type-methods-lint.log`. No suppression/baseline increase or full-analysis pass. Existing RuntimeSurface/ProcedureNames/InterfaceOutput names remain.
- Previous completed frozen integration: `build/verify/20261002T151251Z-2972704`, HEAD `bd455e1`, failed/1,319 seconds; slice-check/all/UT/census exit 2, test exit 0 (106 local cases/149 toolchain tests green). Source SHA256 `8d33bf18d5091dd81b38312a09ee6ae92c0ff1873ded09ec4d2a60e44dfd7666` and original System SHA256 `b11a9e8a0a376dfa429768c5e951a304b64d14f76761e61093438a2489fa9d7d` agree before/after; BCApps `6261b1c458` unchanged. Real PEPPOL consumer root 095 compiles; first build failure is missing UpgradeCompositeReportParts.cpp in root 085. All 14,222 slice inputs remain counted. No successful integration or G1 claim; snapshot result/artifacts are authoritative, not mutable lane logs.
- Interface defaults: 25 generator/fifteen production-generated execution checks green; overload/var/override/inheritance/Options/body-only dependencies and body/API/self-call controls covered. Actual old emitter stays abstract and mishandles parse/sweep failure; current translation returns nonzero and preserves outputs. Full local replay: 106 cases/149 toolchain tests green, zero skips. Fixture receipts grow analysis to 202 units; changed-code lint fails 25/28, new gate/runner clean. Regeneration exits zero: 24,387→24,389 files including twelve reaches files, two providers/four headers changed/zero lost paths; all pre-existing C++ bodies unchanged. Slice grows 14,220→14,222. `build/interface-default-{tests.log,lint-final.log,generation-comparison.json}`. Actual frozen consumer compilation is proved; complete tree/UT remains blocked. Typed dispatch/RequiredPending/required-only identity remain 0034, not G1 closure.
- Current build blocker: 0063/0034/0589 own the generic report-extension/layout successor to retired codeunit 104067. Native declaration authority is now verified above; production native binding, asset installation/migration and selection are still required. Preserve both native parts and all fourteen extension layouts/assets; no empty compatibility codeunit or silent slice removal.
- Helper/include follow-up: 104 local cases/147 toolchain tests, zero red/skipped (`build/runtime-naming-final-tests.log`); 24,375 generated outputs remain byte-identical. RuntimeMemberSpelling/YieldsRuntimeType and direct ObjectKind includes; canonical two-unit analysis 32→30 displayed findings, zero additions, both units still red. `build/runtime-naming-final-lint.log` and `build/runtime-naming-lint-{added,removed}.txt`; no full-surface clean-analysis claim. `make slice-check` has missing/duplicate/empty negative controls; `make help` lists hyphenated targets.
- RuntimeSurface/include batch: 103 local cases/147 toolchain tests, zero red/skipped (`build/runtime-surface-tests.log`). Generator files/functions renamed without aliases; ObjectKind detached from Scope; Regex backend hidden, copy/move/lifetime covered. Four standalone dependency checks and filesystem/regex negative controls pass. Fresh generation retains all 24,375 paths/bytes, no golden edits. Three-run no-PCH frontend averages: RuntimeSurface 946.1→410.8 ms, Regex 1,208.1→1,107.6 ms. `make include-cost`; 0589. Not a full-build/runtime speed proof.
- RequiredTestIsolation: one immutable CodeunitDef owns identity and requirements; validation precedes factory/OnRun and counts all refused methods. 430 runtime/declaration checks plus nine generated AL checks; removed-metadata/bypassed-policy controls fail four/130 checks. Generation exits zero; 1,315 catalogue sources change only their metadata constructor, all headers retained. `build/isolation-{tests-final,generation-final}.log`; 0039. Translated-runner activation and G1 remain open.
- Changed-code lint: 50/199 units analysed, 48 fail; formatting passes. Regex implementation findings six→five, corrected Regex gate zero own findings; initial test-only move-use/constness issues repaired, not suppressed. Inherited debt remains. `build/runtime-surface-lint.log`, `build/lint/{units.json,tidy.log}`; no full-surface clean-analysis claim or raised baseline.
- Source declaration regression now includes five families; the current Field batch above supersedes the earlier 453-check/four-family replay. Compiler/runtime aliases retain original IDs. 0034 owns complete native binding; 0013 owns populated-schema activation.
- Fresh old/new generators against the same BCApps `6261b1c458` produce 24,375 byte-identical output files, zero gains/losses (`build/platform-source-comparison.json`). Both retain 80 UT codeunits/2,314 methods and refuse one unclassified RequiredTestIsolation declaration. Existing CodeunitDef retention is not runner-policy enforcement (0039); no baseline raised. Generated-tree changes versus the earlier image belong to the upstream refresh, not this compiler patch.
- AL UT: 80 codeunits/2,314 methods, zero executed/all 80 incomplete. Exact IDs/names/methods match frozen `94e4a44`; `build/interface-default-ut-{before,after}.json` compare equal. Latest frozen `artifacts/ut.log{,.manifest.json,.run.json}` retains every identity/build refusal; runner/seed acceptance was not reached. Four upstream additions remain named in 0058.
- Fresh raw inventory: 36,873 files/36,782 objects/4,171 test codeunits/113,005 Test methods; fifteen explicit exclusions, 112,990 required raw methods. Zero unmeasured files; seven CLEANSCHEMA assignments in three files red. Latest snapshot `artifacts/census/scope-inventory.json`; canonical files/objects/errors/source hashes compare equal to frozen `94e4a44` (`build/interface-default-census-{before,after}.sha256`, `6022d58336d344bf0c27688de9be54726b453729f1fc506f8aa1a900d11076ee`). Upstream branches leave unmatched field braces; classify declared compiler/localization profiles in 0058, without suppressing raw refusals.

## Release gates

| Gate | Required outcome | Owner |
|---|---|---|
| G1 — UT first | Complete in-scope generated tree compiles/links; every source-counted UT method passes through `agiru run-tests`; no missing/refused/crashed/skipped/incomplete cases. Reproducible seed, runner and image identities. | 0038, 0039, 0058, 0004 |
| G2 — usable clients | After G1, build the agent-only Node.js/TypeScript HTML/ASCII client with shared CMD/MCP adapters and htmx web UI over one production page/command runtime. Prove operation parity, database effects and errors; browser checks are representative samples. | 0030, 0720, 0006, 0062 |
| G3 — complete ERP | After G2, execute the complete in-scope AL test population and CLI workflows; close all in-scope object/extension/integration gaps. Prove multi-user correctness and BC-relative performance/resource targets at scale. | 0058, 0034, 0721 |
| Browser demo | After G2, ship single-user agiru/WASM with embedded PostgreSQL entirely on GitHub Pages; shared runtime and proved demo workflow/SQL parity. Not a production scale/durability proof. | 0724 |

- Existing `run-tests` CLI and TestPage repairs belong to G1. Business CLI/HTTP construction starts only after G1.
- Agent CMD/MCP becomes the exhaustive business-operation/testing client. Sampling applies to browser rendering/interaction, never to the UT or full AL denominator.
- User scope decision (2026-10-01): agiru-owned code is MIT; no license keys, trials, subscriptions or commercial feature locks; no O365/Microsoft 365 or other Microsoft cloud integrations. Permissions, isolation, generic protocols and complete core ERP remain required (0725).
- Report raw, selected and explicitly excluded source/test identities with reasons. Only the approved product exclusions above may leave the selected denominator; other namespace exclusions, unlinked fallbacks and unsupported providers remain coverage gaps. A green subset is not complete core ERP.
- P0: unsafe state or unreliable proof. P1: UT/semantic blockers. P2: clients and their bounds. P3: complete-suite/ERP/scale work. Promote any later task when a measured UT failure requires it.
- WI dependencies name prerequisite contracts, not completion of unrelated later-stage work in the same WI. Local fixtures/review can start earlier; activation requires their stated gates.

## Next implementation order

| Order | Deliverable | WIs / first action |
|---|---|---|
| 1 | Trustworthy source/seed/image baseline | 0725 explicit product-scope inventory + 0058 diagnostic census; 0589 execution; 0004 scratch guards and fresh sealed seed; 0013 required schema/provenance |
| 2 | Correct execution boundaries | 0718 record ownership; 0722 JSON; 0035 XML safety; 0012 transactions; 0006 sessions; 0723 atomic sequence ranges; 0039 runner hooks/isolation |
| 3 | Compile/semantic closure | First: consume verified native report authority without 0034's native-contract/consumer losses; 0063 installs owned layouts/proves successor; then 0033 app/type identity → 0073 lowering → 0038 full-app link; 0035 only genuine native/.NET contracts |
| 4 | Remaining UT failures | 0044 navigation/company; 0018 filters; 0043 validation/events; 0061 original errors; 0030 page lifecycle; 0063 datasets; 0065 encoding/import; 0719 context drilldown |
| 5 | Clients after G1 | 0062 enforced identity/permissions + 0006 session ownership → 0030 production dispatcher → 0720 agent CMD/MCP + HTML/ASCII/htmx parity |
| 6 | Complete suite and ERP qualification | 0058 full manifest; remaining object/extension/lifecycle/media/background work; 0721 multi-user and matched BC benchmarks |
| After G2 | Browser-only single-user demo | 0724 Emscripten/PGlite, static Pages deployment, exact values and transaction/workflow parity |

## Architecture decisions

- Native source-binding activation remains blocked by measured declaration/consumer losses. Offline package verification, common page field/options and native record method/property context are promoted; the native loader/contracts and failed mtime-idempotence prototype stay isolated (0034/0589).
- Production: Linux/Podman on x86_64/aarch64; Clang 19 + libc++/libc++abi + compiler-rt + LLVM libunwind + LLD. Only current recipes remain supported. Runtime and every app need a consistent fresh C++ ABI build. Browser-only Emscripten/PGlite on GitHub Pages is a separate single-user project goal after G2 (0724), not production multi-user or scale qualification.
- Native Linux/container throughput and bounded multi-user resources drive all architecture choices. WASM retains functional semantics as a single-user demo; platform adapters may differ, never production through a WASM VM. Selected reporting architecture: versioned DOCX/RDLC → typed HTML/CSS print layout; XML dataset → own C++ pagination/vector scene → Cairo PDF/SVG preview (0063). Reuse pinned official WPT reftests/wptrunner through an adapter; BC fidelity/resource/browser proofs remain open. No Java/Office/Typst/browser engine dependency.
- Business Charts and interactive ledger analysis remain required: AL/.NET data contracts 0035 → shared static chart scenes/export 0063; filters/aggregates/query plans 0018/0019/0064 → typed CLI/web analysis, pivots, saved private views and drilldown 0720 → browser demo 0724. Preserve exact values, permissions/company context and bounded server-side work; do not replace interaction with PDF or browser-local full-ledger aggregation. Client construction still waits for G1.
- KISS/DRY: one owner per contract and one declaration/binder for shared metadata; adapters contain no duplicated semantics. Separate source identity, C++ ABI and runtime capability; smaller WIs retain only open outcomes and reproducible receipts.
- Licensing and Microsoft-service integration are removed requirements, not always-success runtime stubs. Retire native bridges only after auditing the selected dependency closure; never disable user/company authorization. Required third-party notices remain.
- AL default keys complete before extension merging/system fields; ordinary/native metadata share the rule. Translation diagnostics and exit status have one owner (0013/0589).
- `SessionState` owns mutable AL state; TLS only selects an active session. PostgreSQL owns shared authority, permissions/version state, durable writes and worker claims.
- One transaction lease holds the connection and live cursors. No connection or open transaction is retained during user think time.
- Production Commit is durable. Test isolation floors and TryFunction/Codeunit.Run/asserterror boundaries remain distinct.
- One C++ production page dispatcher executes TestPage, agent CMD/MCP and htmx commands. Generated immutable metadata supplies one typed page/action model, semantic HTML and exact values; adapters contain no business rules. The agent-only Node.js/TypeScript client renders the declared HTML profile as ASCII and shares a client library between CMD/MCP; no full browser/htmx implementation or Node.js ERP-server dependency (0720).
- Development: one editable tree, coherent fix batches → Make build/gates/test/clang-tidy/UT → inspect and repeat. Frozen verification is optional for continued editing or reproducibility, not a prerequisite to each repair cycle; AGENTS.md holds the short workflow and make help/owning WIs hold specialist targets.
- One library per BC app; public headers plus declared dependency roots. PCH/unity/slice remain build optimizations, not dependency or completeness proof.
- Exact Decimal values, bounded blocks/streams and observed-version writes precede optimization. Proposed performance thresholds and matched-BC proof live in 0721.

## Imported implementation evidence (2026-09-30)

- The refreshed predecessor has a fixed-list CI result of 2,539/2,578 and a separate corpus result of 1,853/2,157 in-scope cases; 126 more IDs were explicitly out of scope. These are not agiru UT results, not the same denominator, and not proof of complete BC functionality. Keep agiru's independent source census and explicit exclusions (0058).
- Its page-open smoke run reached 2,517/2,653 non-API pages; open/render success is not operation parity. A single stateful page protocol, modal answers, runtime-derived lookup metadata and separate AL-error/runtime-error classifications inform 0030/0720, only after G1.
- Static test metadata reduced predecessor discovery memory from 615 to 418 MB; lazy object loading reduced its Python image from 812 to 405 MB. These are hypotheses for measuring agiru's immutable C++ metadata and per-session cost, not transferable performance claims (0006/0721).
- A simultaneous CI run, development server and client gate exhausted memory; keep heavyweight integration serialized. Audits must detect stateful mutable objects as well as containers (0006/0589).

## Review findings

| Priority | Reproducible finding | WI |
|---|---|---|
| P0 | One canonical policy now excludes one license/SaaS test object explicitly; all raw identities remain. Mixed core/cloud modules and omitted app roots still need product classification, dependency closure and mandatory runner partitioning. | 0725, 0058 |
| P0 | SingleInstance and Manual bindings now use per-Session ownership; other mutable TLS state still requires isolation. | 0006 |
| P0 | LockTable arguments/state are not enforced by SQL reads; update/delete predicates omit observed version. | 0012 |
| P0 | JSON parent/replacement/Root/Path contracts remain incomplete; high-scale typed conversion/equality still reaches the scale-20 Decimal core despite exact raw lexemes. | 0722, 0066 |
| P0 | XmlReader ignores Prohibit and reads a local external entity; demonstrated with a review-owned fixture. | 0035 |
| P0 | Atomic number-sequence reservations/name binding now pass concurrency and rollback gates; legacy identity migration, populated AL execution and scale qualification remain open. | 0723 |
| P1 | Descending SQL Next(-1) moves 2 → 1 instead of 3; Next(0) is now fixed through all four typed/RecordRef SQL/temporary paths. | 0044 |
| P1 | Isolated subscriber failure is swallowed inside an active write transaction. | 0057 |
| P0 | Shared table/XMLport naming and post-merge binding verified own-only; other kinds, scoped lookup and app/interface closure remain open. | 0033 |
| P1 | Base64Convert, EntityText and WebService absence is reported as '.NET member', obscuring platform/AL work. | 0034, 0038 |
| P1 | Page.Update(false) returns immediately; substantial production behavior remains inside TestPage. | 0030 |
| P1 | Decimal calculation is globally rounded to SQL scale 20 instead of documented CLR scale 28; Format/field/input limits need separate boundaries. | 0066 |
| P1 | SetEncrypted stores plaintext and isolated-storage keys omit extension identity. | 0062 |
| P1 | Session bridges now belong to rt; all three foundations independently link and reject reverse edges. Public-header/runtime-error coupling still needs an audit. | 0589 |

Diagnostic sources/logs: `build/review-20260928/`: RuntimeProbe, ScopeProbe, JsonProbe and BoundaryProbe. Initial review was read-only; subsequent fixes and negative controls are recorded in the owning WIs and verification below. No commits or demo/master mutations.

## Verification

| Evidence | Current result |
|---|---|
| Overload binding counterexamples, no fix | Main source aa917af7c462b80c90b51d905ca1fa28b0f081090a070cd56a81adb682a0a6fe unchanged during probes. Four corrected fixtures pass the Microsoft AL compiler, then fail generated C++ without PCH: internal case plus different arity/same-arity types/var Text, and wrong first-declaration Guid coercion for a Text literal. Three first-letter-only controls already compile/execute and remain recorded; do not claim all case variants are broken. build/overload-binding-20261002/artifacts/{internal-case,checked}/probe.json. AL0440 from the illegal draft Codeunit.Run is preserved; no draft acceptance claim. 0073 names the one-declaration call-binding architecture and complete receiver/type/mode matrix. No semantic/runtime fix, BC runtime, AL milestone or G1 claim. |
| Current frozen integration, terminal failed | Snapshot build/verify-cache-20261002/source/build/verify/20261002T064256Z-4: 1,247 seconds, all/test exit 2. Snapshot/lane source SHA256 3fb68a3d78df971196abf77310857fd8003e0324f880162eb6fd5e462a6fe2d7 and frozen System hash b11a9e8a0a376dfa429768c5e951a304b64d14f76761e61093438a2489fa9d7d remain identical before/after. Root 785 retains PageFieldsSelectionList.cpp:19 Format(Caption), missing native Page Table Field SourceTable binding; not repaired by a guessed Caption() call. Snapshot repeats 137 FilterGroup checks/140 Python tests green, 97 local cases/same 26 DB failures, all main counts/statuses retained. All 2,310 configured AL methods remain unexecuted/missing. No measured per-run cache-hit comparison or full-tree/link/G1 pass. result.json/verify.log and build/filter-group-integration-20261002/artifacts/{integration-origin,integration-result}.json. Only boards follow the frozen main image. |
| Filter-group runtime, promoted | Nine curated files; typed Record/RecordRef share pure getters, >255 ignore and current-group HasFilter. Main LLVM make tc test: 137 FilterGroup checks/140 unchanged Python identities green, 97 cases/same 26 DB failures; all old C++ counts/statuses retained. Tested complete image SHA256 93842a7fe732b74ce1e482b89854d61987583f4b71936efc3f2a92069d294f72. Two generated AL alias pages execute getters, limit checks, clearing and free-group search. Getter-reset/unbounded-setter/group-blind controls: 21/34/9 red. All 54 emitted parameterless call-site bodies plus definitions/setter/regression consumers: 139 units, 135→135 green without PCH/no loss/new diagnosis. Four old failing bodies stay counted; SCMProductionOrdersII overload-spelling gap is recorded in 0073. All 24,346 generated files, slice 14,212 and configured source UT 80/2,310 unchanged; zero AL methods executed/all missing. Raw census reused only after frozen AL/scanner/policy hash equivalence; one unmeasured GB report remains red. Runtime analysis 20→20/27→27; new gate has only 21 inherited-header findings, no new finding or suppression. Normal lint retains the same eight-header abort. build/filter-group-integration-20261002/artifacts/{main-proof,main-tests,proof,consumers,stable-controls,stable-targeted-lint,full-lint,ut-manifest,ut-statuses}.json. Consumed setter return, SQL, complete-tree/link/full UT/G1 remain unproved (0044/0058). |
| Native record context, promoted | Four curated files; SourceTable/TableRef owns Rec/xRec/named-record properties and native alias spelling; exact quoted fields precede methods. Main make tc test: 159 binding checks/140 unchanged Python identities green; same 96 cases/26 DB failures, zero skips. Reviewed/regenerated SHA256 e4a301d8feda89c48eb475a72cbe9b7a913cb9c6343c98520c527607fe6cd8be. Two generated AL alias pages execute without PCH: property getters/setters, selected-group filters, named field-argument ownership and instance isolation. Old compiler: 41 checks red/both alias cases fail; normalized-field mutant: one red. All 58 changed/regression body/definition units: 54→55 green, only IncomingDocumentApprovers gains; no loss/new diagnosis, UserCard retains IsWSKeyAllowed. Draft DefaultDimensionsMultiple loss repaired and retained. Generation: 28 changed bodies, 24,346 files/none added or removed; slice 14,212/UT 80/2,310 unchanged, all AL methods missing. Raw census byte-identical; later corrections change no frozen AL/scanner/policy input. BodyWriter analysis 22→21/no new findings; normal lint retains eight-header abort. build/native-record-context-integration-20261002/artifacts/{main-proof,main-tests,main-generation,proof,consumers,census-comparison,final-controls,normalized-field-mutant,targeted-lint-current,full-lint}.json. The original no-DB filter-group-probe.json exposed getter-reset/>255 runtime gaps; the promoted repair is indexed above (0044). No native System input, full Field/provider, full-tree/link/SQL/G1 proof. |
| Shared page record binding, promoted | Nine curated code/test files; Rec/xRec/bare fields and options share declared bindings, preserving local shadowing, ordinary collisions and report dataitems. Fresh main: 89 binding checks/140 Python tests green; 96 local cases/same 26 DB failures, no previous test/body/status loss. Tested/regenerated SHA256 08b40363d8e2a3591c5d8db39d768eff0dab2a15fc5c34e8588291769ea00fcb. Old compiler: 36 checks red/both generated AL alias cases fail. Eight bodies change; all 24,346 files match the reviewed image, no path loss. All sixteen changed body/definition units: 14→14 green without PCH/no added diagnostics; two failing bodies retained. RefusedOption 1,278→1,253/no additions. Complete raw census bytes match; UT 80/2,310 and slice 14,212 unchanged. Targeted analysis adds none; normal lint retains eight-header abort. build/page-record-binding-integration-20261002/artifacts/{main-proof,main-tests,main-generation,proof,consumers,census-comparison,current-controls,full-lint}.json; targeted-lint-current.json. No native System loader/contracts, Field layout/provider, AL runner, SQL, full-tree or G1 proof. |
| Offline System package verifier, promoted | Only scripts/fetch_symbols.py and its regression in test/tooling/toolchain.py: --verify reuses the canonical ledger/hash/identity verification without network or writes; changed packages refuse. Fresh main make tc test: 139 Python tests, 96 local cases/same 26 DB failures, all prior bodies/counts/statuses retained. Unchanged source SHA256 7ce5032858af8b517a81a53c2381b7b38260ac73b4b7beedbf7a5a5df039a90f. No binder, Make/Main or generated output promoted. build/native-source-binding-integration-20261002/artifacts/{main-tests,main-proof}.json. |
| Current-origin native source binder, own only | Source at build/native-source-binding-integration-20261002/source; stable SHA256 7e3d0acb2c17cea02cc9eb7b5971327b7fc7b73b804697c9fe40ce010d214a7c. Explicit System input, common BindTable, native ABI includes and source contracts; Make verifies the original package. Forty new C++ checks/143 Python tests green; 97 cases/same 26 DB failures, all old bodies/counts/statuses retained. Original tooling page compiles/executes; nine declaration mutants refuse; same five predecessor controls have seventeen failures/one error, current zero. Independent raw System: 362 files/333 objects/223 tables, zero unmeasured; fifteen candidates (five contract-green/ten red), four wrong-ID refusals plus 204 unsupported. Actual eight-consumer comparison 7→5: PageFieldsSelectionList gains, three Field/source-binding losses retained. Full generation 24,346→24,348, 1,942 changed/none removed; slice 14,212 and configured UT 80/2,310 unchanged, no AL methods executed. UsersInPlans query requires 0725 classification. Five-unit corrected analysis adds no findings; normal lint retains the eight-header abort. artifacts/{proof,stable-tests,generation,system-raw-inventory,native-contracts,consumers,controls,full-lint}.json; stable-analysis/targeted-lint.json. Field declarations, common Rec/option binding, other native defects and licensing retirement precede promotion. No live provider/direct compiler package provenance/full-tree/G1 proof. |
| Ordinary source binding, promoted to main | Thirteen curated code/test files; one table binding after extension merge, declaration-only signatures and name/number aliases. Fresh main make tc test: 36 new binding checks and all 138 Python tests green/zero skipped; 96 local cases, same 26 DB failures, no prior test/body/status loss. Tested source SHA256 17262c1f42bda5ab4d11cddbcc543d5d4e71a8e8f13ba28d9938bf325dd092a0, unchanged and equal to reviewed image. Four identical generated AL cases compile/execute without PCH; sealed old compiler fails all four. Main regeneration matches all 24,346 reviewed files: eighteen bodies change, none added/removed; all 14,212 slice paths present. Complete changed/tooling consumer comparison 20→22/23, only PurchaseBatchPostMgt/SalesBatchPostMgt gain, zero losses/new diagnostics. Five-unit analysis adds none; Main 24→9, new gate zero; normal lint aborts on the same eight legacy unconsumed headers. Fresh raw census retains 112,055 methods and the unmeasured GB report. Configured W1 UT 80/2,310 unchanged/unexecuted; not a whole-source runnable denominator. Native System input/contracts, root 785 Caption, live metadata providers, SQL posting and G1 remain open. build/source-binding-integration-20261002/artifacts/{main-proof,main-tests,main-generation,generation,consumers,final-analysis,full-lint,source-census}.json; sealed-controls/ and before-source/. |
| Page Table Field runtime family, promoted to main | Ten curated files; no older compiler image copied. Independent source comparison matches all fifteen fields, four options/37 values, original key, Pending reasons, OnPrem and Brick constants. Fresh main `make tc test JOBS=2`: 312 new checks, 136 Python tests/zero skipped; 95 local cases and the identical 26 DB failures, no existing suite count/status loss. Tested source SHA256 `ddf45666f81f08cb6137a4d197890f29606a047789bd6b6f8311be92216e4104`, unchanged and equal to isolated current-main image. Provider no-op mutant fails fourteen checks; twelve option controls retain dense-option refusal and reject malformed native codes. Seven-unit analysis adds no findings; ProvisionInstalled complexity 79→71, full lint still red. All 24,346 generated files and 14,212 slice paths unchanged; W1 runner UT 80/2,310 remains unexecuted, not a whole-source denominator. Six original generated tooling units have identical before/after compiler results/diagnostics: root 785 remains blocked on Caption because source binding is absent. Live projection, common Scope/fieldgroups and G1 remain open. Rebuild all generated consumers for the extended TableDef ABI before AL execution. `build/page-table-field-integration-20261002/artifacts/{proof,main-tests,source-contract,option-controls,provider-control,targeted-lint,population,actual-blocker}.json`. |
| Object catalogue, promoted to main | Four files only: AllObj/AllObjWithCaption/AllObjType headers plus ObjectCatalogueGate; no older generator/native/provenance chain copied. Source-defined six/nine fields, sole pk, rX, original lengths/IDs and all 23 sparse options independently match pinned System declarations. Corrected gate 294 green; same complete old source/runtime image 90 red. All 965 generated quoted-include consumers compiled without PCH: 915→916 green, only original Objects.cpp gains, zero losses; 49 remaining errors retained. Generation changes zero of 24,346 files; all 14,212 active slice sources present; exact raw UT 80/2,310 unchanged. Targeted runtime findings 20/43 unchanged; six initial test-only findings repaired, no new finding over byte-identical inherited headers; default lint still aborts on eight legacy unconsumed headers. Fresh main `make tc test JOBS=2`: 136 Python tests green/zero skipped, 94 local cases/26 DB failures, catalogue 294 green; unchanged SHA256 `1fe8036b958accd0be649fe9ea2eee7bfbaed0ce0e747c24752f5aa9c57d645d`. `build/object-catalogue-integration-20261002/artifacts/{source-contract,consumer-comparison,draft-controls,draft-lint,current-tests,main-tests,generation,before-frozen}.json`. Before source/runtime frozen at `before-source/`; provider/Scope/fieldgroups/migration/full-tree/G1 remain open (0034/0013/0044). |
| Catalogue frozen replay, terminal failed | Normal make verify JOBS=6 in the equivalent writable clone; snapshot build/verify-cache-20261002/source/build/verify/20261002T011922Z-7/. Finished 2026-10-02 01:34:57 UTC, 924 seconds; all/test both exit 2. Source/post-source SHA256 both 1fe8036b958accd0be649fe9ea2eee7bfbaed0ce0e747c24752f5aa9c57d645d; System input hash unchanged. Root 785 fails PageFieldsSelectionList.cpp:19, Format(Caption) resolves to the page method: source Page Table Field was absent from that snapshot's native binding/registration (0034). Snapshot retains 294 catalogue checks and 136 toolchain identities green/zero skipped; 94 cases/26 DB failures. Per-run cache counters: zero direct/preprocessed hits, 469 misses, one compile failure; no cache-speed claim. Main normal regeneration also exits 0 in 146.8 seconds, all 24,346 files unchanged. build/object-catalogue-integration-20261002/artifacts/{verification,verify-source-identity,main-generation}.json and verify-cache-{before,after}.txt; snapshot result.json/verify.log. This terminal image predates the Page Table Field runtime promotion; no live integration or current-tree/full-UT/G1 pass. |
| XML safety investigation, not a fix | Fresh production stream probe: 32 UTF-8/UTF-16LE cases; four prohibited/ignored external-file leaks, eight reported ignored DTDs, two positioned-reader reloads of consumed siblings, six closed-reader root reloads. Source contract consumes the reader; closed readers leave an empty document, not a presumed exception. Native libxml2 2.9.14 experiment: 24 observations block external general/parameter/subset resources through context-owned callbacks while retaining internal entity/default-attribute parsing; FIFO control observes an actual open only in the unsafe case. No global loader or private host data. Parser stops retain a non-null document/wellFormed=true with error 111: pointer/form alone is not success. `build/xml-policy-20261002/{runtime-probe,context-probe,resource-trap,next-policy}.json`. Ignore grammar, bounds, HTTP trap, shared cursor/alias state and production integration remain open (0035); source references and implementation plan stay in the WI. |
| Integration lane | Snapshot `20260930T210740Z-797012` remains terminal/failed. Native/Text/Chart compiler prototypes are own-only; current own runs are recorded below, not production integration. |
| Dictionary/designer family, promoted to main | Main and validated image matched SHA256 `0581bb5dbba3e123038eb63c4d250d514c92bcc86a96e242a16ca0e5636b383c` after normal regeneration. Dictionary 70, Designer 45, GenReceiver 10 green; old receiver three red, old includes one red. All 135 main toolchain identities retained, including five newer provenance tests; 93 local cases/26 DB failures. All thirteen changed bodies/eight original consumers measured without PCH; no new compile loss, Keys and two old report failures remain. Generation changes 1,469 files, removes only approved codeunit 138073 cpp/header: 24,346 files, 14,212 active slice sources/none missing; exact raw UT 80/2,310 unchanged. Six-unit targeted analysis: no new findings; full lint 193/193 units, 165 failed. `build/dictionary-integration-20261002/artifacts/{proof,main-generation,post-generation-tests,controls,body-comparison,consumer-comparison,targeted-lint,lint-full}.json`. No complete-tree/full-UT/SQL/G1 proof. |
| Frozen LLVM integration, terminal failed | Snapshot-cache double deletion repaired: old regression two errors, SnapshotGate 12 green. Fresh main: 136 toolchain identities green/zero skipped; 93 cases/26 DB failures; Dictionary 70, Designer 45, GenReceiver 10 green. Tested main SHA256 `fc47494793ce324005f446bd3bcc4112af50e02dbff2f0c6988310fc94dc5bed`, unchanged; code/generated inputs equal the frozen clone, only board differs. Seven main ELF checks contain no GNU C++/unwind runtime dependency. Original .git is read-only; equivalent writable clone runs normal `make verify JOBS=6`. Snapshot `build/verify-cache-20261002/source/build/verify/20261001T233840Z-31/`: finished 2026-10-01 23:56:17 UTC, 1,044 seconds; all/test both exit 2. Build stops at root 450, Objects.cpp:60 missing AllObjWithCaption.ALNamespace (0034); snapshot tests repeat 136 green/93 cases/26 DB failures. Source/post-source SHA256 both `68e46b20264136659a8e0f8691e2b4d304c11feefc2336c59e8fe436ed73a5c4`. `result.json` and `verify.log`; `build/verify-cache-20261002/artifacts/{source-identity,verification}.json`; `build/dictionary-integration-20261002/artifacts/{main-proof,main-post-cache-tests,snapshot-cache-old,snapshot-cache-new}.json`. No integration/full-UT/G1 pass. |
| Dictionary/designer predecessor, own image | `build/dictionary-20261001/source`, unchanged SHA256 `28ed96a03524c46bc08e590ca7d11561a90bfbdcbb5bec2100f8a9d0cb8689f7`. Dictionary 70, Designer 45 and binder 213 green; old dictionary six red, old receiver three red, all twenty constant mutants and both wrong iterator policies fail. All 149 toolchain tests green/zero skipped; 96 local cases/26 DB failures. Six consumers and sixteen original body comparisons retained; both activation losses repaired, Keys/two old report failures remain. No new targeted findings; repeat generation changes zero files. All 24,357 paths, 14,218 active slice sources and exact raw UT 80/2,310 retained. Parent/frozen inputs rechecked. `artifacts/{proof,dictionary-verified,designer-proof,iterator-controls,receiver-old-replay,receiver-new,actual-consumers-new,changed-bodies-final,lint-final-comparison}.json`; generation identities in `artifacts/dictionary-verified/`. Selected primitive family subsequently promoted above; the whole predecessor compiler image was not copied. Keys/array/boxing/designer lifecycle and G1 remain open. |
| Page source field binding, own only | `build/page-source-binding-20261001/source`, unchanged final SHA256 `6167d4c0faf2acc5b2a001e832d45c42beca0184461edba3c617272c35c17d2d`. Binder 205 green/old eighteen red; NativeObject 1,365, PageTableField 190 and all 149 toolchain tests green, zero skipped. 94 local cases/26 DB failures. Original ChangeLogSetupFieldList body/definition compile without PCH; UserCard definition compiles, body retains one IsWSKeyAllowed error after both State_4 errors disappear. All four units retained. Full generation: 24,357 files, only those two bodies changed, none deleted; all 14,218 active slice sources present; exact UT 80/2,310 unchanged. BodyWriter targeted lint 21→21, no new finding/baseline increase. `artifacts/{proof,old-binding,actual-page-before,actual-page-after,actual-user,final-tests,generation,lint-comparison,designer-contract}.json`. Original UT process handle disappeared without a terminal receipt; its log is retained. Unchanged incremental recheck exits 2 after 136 s: three TenantLicenseStateImpl native declaration mismatches at root 619; consumer bytes equal predecessor. Correct frozen BCApps revision and both input hashes checked; all 2,310 methods missing/80 incomplete, no identity gains/losses. `artifacts/{ut-resume,ut-proof}.json`. Licensing dependency closure remains 0725; no main integration, page execution, SQL or G1 claim. |
| Page Table Field binding, own only | `build/page-table-field-20261001/source`, unchanged final SHA256 `13899bfd7676c8d0f821a4885b36c7ae3d7282887e058d806a4efe7c94ef19ec`. Original fifteen fields/four options and primary key; registered declaration, explicit unavailable-provider boundary, independent temporary storage. PageTableField 190/190, binder 185/185, existing NativeObject 1,365/1,365; 149 toolchain tests, zero skipped. Old binder twelve red; five changed source contracts refuse. 94 local cases/26 DB failures. Fresh generation changes nine files, drops none; all 14,218 active slice sources present, exact raw UT 80/2,310 retained. Five of six actual page/definition units compile without PCH; AddPageFields has eight Designer-property errors. Storage targeted lint remains red with no new unexpected diagnostics; ProvisionInstalled complexity 77→69 after separating schema provisioning. Terminal full UT build: exit 2 after 1,103 s, former root 785 compiles; root 534 fails on three ChangeLogSetupFieldList field names. Consumer bytes equal predecessor; all 2,310 methods missing/80 incomplete, no identity gains/losses. Live projection, integration and G1 remain open. `artifacts/{proof,source-controls,actual-pages,final-tests,generation,lint-storage-final,ut,ut-proof}.json`. |
| Current LLVM core/gate verification, own only | Main-source image `build/llvm-20261001/source`, SHA256 `eb25178280829b7fb2dab5f05ff7a2a408cb0afa2363e40effb002e86c071713`, unchanged during testing. All five core libraries, transpiler and 86 C++ gates build with libc++/compiler-rt/LLVM libunwind/LLD; actual C++23 compile/link probe passes. 89 local cases: 26 PostgreSQL connection failures; all 104 toolchain tests pass, zero skipped. Five new policy cases pass; unchanged old recipe fails all five (seven assertions). Missing direct `<algorithm>` in Table.h fixed. Library ELF dependencies contain libc++/libc++abi/libunwind, not libstdc++/libgcc_s. `build/llvm-20261001/artifacts/{test-2,proof}.json`, `build/clang-policy-20261001/proof.json`. No full slice/apps/UT or production-integration verdict. |
| LLVM compiler consolidation, own only | Preserved Chart/System-symbol/app-identity chain plus current LLVM ABI and canonical product policy. Source SHA256 `1e0e824e772a1a1e611639434f23dc3476095a2fe8d0d68e408a2b6d13077970`, unchanged in post-generation local tests. All 147 toolchain identities retained, zero skipped; 93 local cases/26 DB failures. NativeObject 1,365 and binder 169 green; DataMeasure 49 and DataTable 38 green. Five old numeric-control checks fail; main-source gates also pass 49/38. All fifteen named sparse values independently match current AL declarations. Six core/compiler ELF dependencies use LLVM libraries without GNU C++/unwind runtime dependencies. Actual page/codeunit compilation clears the numeric refusal, but dependency linking remains open and all twelve page checks remain missing. Fresh generation: 24,357 files, 14,218 active slice sources, none missing; exactly one approved license/SaaS source excluded explicitly. Raw UT 80/2,310, zero gains/losses. Frozen BCApps/System hashes `309e83d80e702969df9e7611149e69e422af0f2e2acb1374ea57cc685d46aec4` / `b11a9e8a0a376dfa429768c5e951a304b64d14f76761e61093438a2489fa9d7d`. `build/compiler-llvm-20261001/artifacts/{proof,generation,inputs,main-gates,numeric-controls,actual-chart}.json`. Compiler chain not integrated; terminal UT build failure is recorded below, no G1/client/performance claim. |
| Chart / preserved predecessor image | Original System ID 2000000078, fields 3/6/9, Pending; shared declaration-header ownership, source-owned locals, Door adds missing directives only. Historical Clang/GCC NativeObject 1,365 green; binder 169 green/old 23 red; all 37 GenCodeunit checks retained. AL 21 guards plus driver green; two unsuppressed Pending warnings. Contracts 18/19, only Tenant License State red; 92 local cases/26 DB failures and 119 toolchain tests. Its numeric conversion blocker is superseded by the LLVM row; twelve actual-page checks remain missing. Nine handwritten/4,632 generated paths; same slice/24,359 repeat files. Six new native includes name their types; redundant directives 70,298→66,718. All 14,471 C++ bodies compared: only eight original Chart-binding paths differ beyond includes. Four-unit findings unchanged, no raised baseline. `build/chart-20261001/artifacts/{declaration-proof,actual-chart,bodies,dependencies}.json`; SHA256 `7d8acdccca3fa0f2a9b031199ac6c4522915d5f686b29de15e638c47a0098cb9`. No integration/G1/client/performance claim. |
| OData Edm Type / predecessor, own only | Source ID 2000000179/Blob field 10/UserDefined through the existing binder; no Clear/Refused fallback. Clang/GCC: NativeObject 1,328 green/old twelve red; binder 141 green/old fourteen red. Oracle-valid AL thirteen guards plus one driver; actual generated page twelve temporary-store checks green, not SQL/publishing/permissions/client parity. Four original AL procedure bodies and old gate functions retained. Contracts 17/18, Tenant License State red; same 92 cases/26 DB failures and 119 toolchain tests, no new targeted findings. Four handwritten/eight generated paths; same slice and 24,359 repeat byte/mtime-identical files. Terminal UT build 1,104.9 s clears OData/root 739, then CopyGenericChart/root 875 fails Caption(Text + Refused): original Chart 2000000078 is unbound. Four no-PCH probes reproduce unchanged consumer; same 80/2,310, 0 executed/80 incomplete, all methods missing. `build/odata-edm-20261001/artifacts/{proof,ut,next-diagnostic,actual-odata}.json`; stable SHA256 `783010cd6ee3dc8ff25525f8f475b933793bc29b88f2963d83248333c15e2bb8`. Own only; migration/activation remain 0004/0013/0034/0058. |
| Page Metadata / predecessor, own only | All 32 original fields, Caption Text[80], thirteen options/HeadlinePart=12; source-owned projection explicitly refuses six absent kinds. Clang/GCC NativeObject 1,294 green/old 206 red; AL 27 guards plus driver, actual PageManagement two zero-table guards; projection 20 green/guessed-Card mutant six red. Storage findings 57→56, no generated changes. UT 824.5 s clears PageManagement/root 792; its OData/root 739 blocker is superseded above. Same 80/2,310, 0 executed/80 incomplete. `build/page-metadata-20261001/artifacts/{proof,ut,next-diagnostic}.json`; SHA256 `4ba5aab491d7967b660832ba2f68521c6879d809937bb4fbd97e71b5e27e431a`. Live metadata/provider/activation remain 0013/0034/0044/0058; no integration/G1. |
| Object Options / predecessor, own only | Source ID 2000000196, nine fields/twenty sparse option slots/source key; generic Temporary-name reservation (0073). NativeObject 1,022 green/old 67 red, binder 125 green/old nineteen red; AL nine + driver three green under both compilers. Actual ReportSettings compiles, not saved-settings workflow proof. UT 421.2 s clears root 879, then PageManagement/792 refuses Page Metadata; current isolated declaration fix above supersedes that cause. Same 80/2,310, 0 executed/80 incomplete. `build/object-options-20261001/artifacts/{proof,ut,next-diagnostic}.json`; no integration/G1. |
| Shared field-enum binding / earlier compiler receipt, own only | One existing lookup for Codeunit/Table/Page/Query; nearest declaration owns shadowing. Binder 105 green/predecessor seven red, AL eight checks plus four driver assertions. Actual report compiles; actual feature editability four cases succeeds in both images/compilers. Eight generated page sources change; repeat/slice stable. UT 400.7 s clears report root 639; ObjectOptions→Clear blocker superseded above. Same 80/2,310 incomplete, not passing. `build/field-enum-20261001/{source-identity.json,sequence-exits.json,artifacts/proof.json}`; no integration/G1/workflow proof. |
| Text/Guid / earlier compiler receipt, own only | Existing primitive/formatter reused; Text 75 green, 43 original checks retained, erased-result mutant six red. Generated AL sixteen checks plus destination error; actual consolidation and whole-old-header controls retained. Oracle proves eight Text-result cases; Code+Code remains open. Guid/root 321 clears; former report-option blocker superseded above. `build/text-guid-20261001/{source-identity.json,proof-exits.json,artifacts/}`. No integration/G1 proof. |
| Table Metadata / earlier compiler receipt, own only | Twenty-three original fields/six options/rX/effective ID key; one shared reflection vocabulary. Clang/GCC NativeObject 891, binder 82, lowered fixture 39 green; predecessors 181/five red. Actual Uncouple compiles. Unknown CDS projection refuses: eight green/guessed-mapping control one red. Header-cost samples retained, not ERP performance claims. Findings CodeunitWriter 10→10, Storage 57→57, ProvisionInstalled complexity 80→79. Four generated files changed; repeat stable. Former Guid blocker superseded above. `build/table-metadata-20261001/{source-identity.json,proof-exits.json,artifacts/}`. Fixture Option.AsInteger is not valid AL (0073); no integration/live-provider proof. |
| Record Link / earlier compiler receipt, own only | Fourteen source fields plus system fields, original types/properties/three keys; generic own-versus-related RecordId binding. NativeObject 606, binder 73 and AL 28 green; predecessors 25/16 red, old emitter fails. Actual CRM Notes compiles. Qualified Company relation diagnostic remains four red of eleven (0043). Former Table Metadata blocker is superseded above. `build/record-link-20261001/{source-identity.json,proof-exits.json,artifacts/}`. No integration/G1/provider proof. |
| All Profile / earlier compiler receipt, own only | Seventeen fields, original lengths/slots/obsolete state/PK/inherent rX; shared Scope vocabulary. NativeObject 447 green/old image 74 red; original AL 24/24 and actual ConfigSetup body/metadata compile. Unindexed sorting still returns false despite correct ordering (0044). Its former Record Link/Table Metadata compiler failures are superseded above. `build/all-profile-20261001/{source-identity.json,proof-exits.json,artifacts/}`. No live profile authority or G1. |
| Privacy binding / earlier compiler receipt, own only | Correct source IDs/fields/captions/relations/keys/replication; NativeObject 291, binder 54, real privacy page bodies/metadata compile. Original AL fixture retains all eighteen checks but stops at GUID filtering; direct diagnostic stays red under both compilers (0018). UT's former All Profile blocker is superseded above. `build/privacy-binding-20261001/{README.md,source-identity.json,proof-exits.json,artifacts/}`. No SQL migration/page execution/G1. |
| Native Field / earlier compiler receipt, own only | One Type/Type Name mapper; 24 source fields and 21 coded Type members. Clang/GCC: Field 295, binder 45, boundary 97 green; whole old image 76/97 red. Generated AL executes 13 checks; actual FieldDataClassification metadata compiles, not executes. Native contracts 8/14 → 9/14; production metadata/FieldRef.Type remain 0034, navigation 0044. Same 92 cases/26 connection failures, 119/119 toolchain tests, 77 → 77 targeted findings. Regeneration retains 24,359 files, 188 changed; repeat byte/mtime-identical; slice unchanged. UT retains 80/2,310, 0 executed/80 incomplete. Eleven Field defects cleared; then blocked by absent PrivacyNotice.Link, superseded by the privacy receipt above. `build/ut-compile-20261001/{README.md,source-identity.json,proof-exits.json,artifacts/}`. No integration/G1. |
| Shared output naming, own only | One allocator/binding for tables/XMLports; table binding follows extension merge once. Clang/GCC generated AL: tables 14/14, XMLports 15/15 per same/dependent-app context; seven controls green, archived emitter six independent assertions red. Real TestTableC pair: 20/20 temporary-storage checks both; no SQL/FlowField proof. Toolchain 119/119; same 92 cases/26 connection failures; five-unit findings 67 → 66, Scan 88 → 86. Full generation/repeat exit 0: 24,359 files, twelve added/nineteen changed/nine obsolete paths removed; repeat byte/mtime-identical. Slice 14,213 → 14,219 with explicit same-ID renames and appended entries. Code Coverage XMLports retained but refuse unbound native table 2000000049 (0034). UT 80/2,310 unchanged/unexecuted. `build/xmlport-names-20261001/{README.md,source-identity.json,proof-exits.json,artifacts/}`; predecessors preserved. Other kinds/scoped binding: 0033; custom XMLport var metadata: 0073; activation: 0013. No integration/G1. |
| Checked-output prototype, own only | One checked writer for objects/options/absent/reaches; cleanup only after success. Clang/GCC toolchain 112/112; three controls green, archived emitter five assertions red each. Same 92 local cases/26 connection failures. All 24,350 generated files byte-identical; source UT 80/2,310 unchanged/unexecuted. Main findings 7 → 7, Scan complexity 92 → 91, no new failure mode. Full repeat changes only three TestTableC mtimes: tables 132512/139063 collide and only 132512 survives (0033, now P0); file counts are not object coverage. Inline-header UT subtotal gap stays 0058. `build/write-status-20261001/{README.md,source-identity.json,proof-exits.json,artifacts/}`. No integration/G1. |
| Implicit-key/status prototype, own only | Clang/GCC GenKey 68/68, generated AL 12/12, toolchain 109/109. Archived generator: same twelve execution checks/nine red; three invalid-source controls red. Invalid defaults retain source/UT counts and now fail translation. Serialized suites: 92 cases/26 unchanged connection failures. Regeneration: 24,350 paths retained, 79 changed; source UT 80/2,310 unchanged/unexecuted. Four lint rechecks plus unchanged predecessor Parser establish the same 182-unit selection at 691 → 691; no new failure mode, Scan complexity 93 → 92. Native contracts remain eight matching/six failing over fourteen. `build/implicit-key-20261001/{README.md,source-identity.json,artifacts/}`; 0013 owns activation. No integration/G1. |
| Text-result prototype, own only | Runtime owns Text concatenation results; emitter anchors literals/labels. Clang/GCC Text 43/43, generated AL 13/13; old runtime same 43 checks/ten red. Old generator and runtime independently fail the fixture. Serial local suites 92 cases/26 unchanged connection failures, toolchain 107/107. Full regeneration retains 24,350 paths (1,567 changed); source UT 80/2,310 unchanged/unexecuted. Body prefix 1 → 41 over 14,465; next APIWebhookNotificationSend absent System tables (0034), reproduced both compilers without PCH. Two lint rechecks keep the carried-forward 181-unit baseline at 689; no added findings. `build/text-result-20261001/{source-identity.json,proof-exits.json,artifacts/}`; 0073 owns activation and remaining Code+Code/literal-type proof. |
| Body-gate repair, integrated | SOURCE/SWEEP bypass the unrelated header census; complete inventories include support directories. Werror, missing/empty scope and missing/spent roots refuse. Sixteen real-Make/Clang controls green; old implementation fourteen cases/seventeen assertions red. Own toolchain 106/106 Clang/GCC; main 99/99 Clang and byte-identical compiler-source frozen GCC. Stale main build/gcc remains separately red (ten failures). No generator/runtime integration; `build/body-gate-20261001/{proof-exits.json,artifacts/integration.json}`. |
| Actual full-inventory body entry | Main 1 passed + 1 failed + 14,462 unmeasured = 14,464 cpp files; own 1 + 1 + 14,463 = 14,465. AddHeader Text/SecretText ambiguity in InviteExternalAccountant.cpp:301, reproduced under both compilers without PCH (0073). This is a first-gap prefix, not full compilation/link success. Own 24,350 generated files unchanged; source UT 80/2,310 unchanged/unexecuted. Complete inventories/logs: `build/body-gate-20261001/artifacts/`. |
| Own predecessor header census | Pre-repair tally 9,868 passed + five failed = 9,873; unchanged-input guards passed. PowerBIServiceProvider once, GraphAuthorization four times. Old make gap header preflight prevented bodies. Archived `build/key-contract-20261001/artifacts/{header-census,generated-gap.log}`. Key image only, not a full-tree proof for later scope/AllObj headers. |
| Key/native-contract prototype, own only | Latest working copy/receipts `build/native-catalogue-20261001/source/build/native-catalogue-proof/`; predecessor native-scope/key images retained. 223 source tables: fourteen candidates, 209 unbound, four conflicting IDs refused. Clang/GCC: NativeObject 193/193, GenKey 39/39, binder 34/34; complete old image same 193 checks/thirteen red, archived generator 31 red. Eight of fourteen represented contracts match; six fail. AllObj now retains six source fields and only `pk`. Real ReportManagementHelper compiles both; old header five named failures. Providers/execution unproved. Serial suites: 92 cases/26 connection failures, toolchain 90/90. All 24,350 regenerated files remain byte-identical; UT population 80/2,310 unchanged. No production integration or G1. WIs 0013/0034/0589. |
| Latest completed frozen run | `20260930T210740Z-797012`, finished 2026-09-30 22:00:44 UTC, 3,150 seconds. All/UT/tree/apps 2; test/GCC 0 (89 cases/69 toolchain). Includes error-value separation; AllObjWithCaption.ALNamespace blocks the UT runner. |
| Frozen source identity | agiru HEAD `136a5a77c87aade461e72e5722728525c9294fa2` plus dirty inputs; latest completed source/post-source SHA256 both `3d5e43e538a58948f11eced3ad4ddf45ba5cd3073e549b6ad6dfb592780a2fc0`. |
| Earlier frozen UT proof | Snapshot refuses AllObjWithCaption.ALNamespace: 0/2,310 executed; all methods missing, 80 codeunits incomplete. Declaration defect superseded by later own prototypes above, not integration/G1 proof. Independent population remains 80/2,310; `build/feature-key-proof/ut-identity-comparison.json`. Snapshot artifacts, never historical lane logs, are authoritative. |
| Last executed full UT | Previous snapshot `20260930T114935Z-65182`: 2,199/2,310; 111 failed, 0 incomplete; 80 codeunits, six workers, 1,258 seconds. Same identities/diagnostics as snapshot 105054. Legacy seed lacks complete provenance: diagnostic repeatability, not sealed A/B proof. |
| Complete tree/apps | Latest completed tree 9,867/9,872; five absent-interface failures. All/apps block on AllObjWithCaption.ALNamespace. Last entered full-app build (snapshot 114935): 62/14,719 edges, missing TryGetStringTenantSetting. Pinned native contract inspected; provider absent (0035). No scope reduction. |
| Computed sources, integrated after snapshot | GenPage 25/25 and PageSource 14/14 under Clang 19/GCC 14; main gates 25/25 and 14/14. Generated AL execution fixture 13/13 both; final local suite 85 cases green, toolchain 59/59. Old generator eight red; old reader refuses. One variable lookup replaces four loops; existing complexity finding removed. Main regeneration exits 0; all generated file paths and 9,872 header identities retained; independent census 80/2,310. Two real changed BC page bodies pass Clang syntax. Full UT effects pending. WI 0030; `build/page-source-*` in main and own worktree. |
| Other integrated gate controls | RangeBound 32/32 both, old runtime 20 red; UT preflight 59 toolchain tests, five old-script controls red, old build-cancellation control times out. NextZero 112/112 and Cursor 217/217 both; old runtime 44 red. GenScope 229/229 both; old matcher three red. SessionInstance 12/12, SubscriberLifetime 22/22 and SessionBinding 18/18 both; old runtime five/14/10 red. WIs 0044/0058/0033/0006/0057 retain inputs and logs. |
| Attribute census, integrated after snapshot | Only observed acted-on kinds counted; no unsigned catalogue subtraction. Six actual-transpiler controls pass under Clang/GCC; old binary six red. Own/main local suites 85 cases and toolchain 65 tests green. Changed-code lint 174/185 units, 167 failed; no new census-specific diagnostic or suppression increase. Main `build/attribute-census-main-tests.log`; own `build/attribute-census-*`. WI 0059. Next frozen run still requires the native Field build blocker to be repaired. |
| Native Field metadata | One mapper for Get/provisioning; shared Field/FieldRef option strings. Type Name field 9/Text[30] uses existing primitive metadata and length: Code20/Text100/Option. PlatformField 106/106 Clang/GCC; archived old headers/runtime 46 red. Real generated BC page body compiles and executes 6/6 checks under both compilers; own/main local suites 85 cases / 65 toolchain tests green. Lint 174/185 units, 167 failed; no new mapper/gate findings or suppression increase. Native ordinals/schema and bounded read-only navigation remain open (0034/0044); no full-UT/G1 closure. Own `build/field-native-proof/`; main `build/field-native-main-tests.log`. |
| User Personalization/storage, integrated | All 19 source fields + five system fields; six lookup formulas, shared license vocabulary, correct properties. Independent 19/19 comparison; old surface 9/19. Gate 320/320 Clang/GCC; old headers/runtime 24 red. Native Removed-column contract: current seven green, archived old header/runtime seven red; thirteen Normal columns + five system columns and obsolete-value roundtrip. Real generated readers 7/7 both. Own serialized suites 86 cases / 66 toolchain tests green both; main recheck 86/66 green. Parent-Make override control red before fixture isolation; no runner relaxation. Lint 175/186 units, 168 failed; no new header/gate findings or baseline increase. Own `build/user-personalization-proof/`, main `build/user-personalization-main-tests.log`; 0013/0034. Page binding/provider gaps remain 0030/0019; no functional-page/G1 claim. |
| Source/platform provenance | BCApps main/HEAD `a9ea4d84534cebba852c44bf0f841c2ea149de4e`; BC_VERSION 28.4.53241.0; local System symbols 28.0.53152.0 / Runtime 17.0. Record schema/data mismatches; version text alone is not compatibility proof. |
| Feature Key, integrated after snapshot | All ten source fields; adds 9/10 Text[2048] using existing metadata/runtime. Independent 10/10; old 8/10. Gate 93/93 Clang/GCC; old headers/runtime six red; actual generated readers 3/3 both, old body compilation red. Own suites 87 cases / 66 toolchain tests green both; main 87/66 green (`build/feature-key-main-tests.log`). Lint 176/187, 169 failed; no new touched-header/gate findings or suppression increase. Own `build/feature-key-proof/`. Historical predicate refusal is superseded by own execution above (0073); activation and source/control IDs remain 0030. No functional-page/G1 claim. |
| Foundation ownership, integrated after snapshot | Three bridge bodies moved byte-identically net → rt; Linux no-undefined policy derives from reaches. All al/net/db independently link under Clang/GCC; actual production links reject injected reverse edges, guard-removal controls link. SessionBridge 16/16 on new and archived old runtime. Own suites 88 cases/68 toolchain green both; main 88/68 green. Own `build/foundation-proof/`, main `build/foundation-main-tests.log`; 0589. No new semantic implementation or session-authority claim. |
| Error value, integrated | ErrorValue separates owned text/code from unchanged transaction helpers; 174 scoped files match the own reviewed image. Payload 15/15 old/new, boundary controls red before separation under both compilers. Own Clang/GCC suites 89 cases/69 toolchain green; all 24,348 fresh old/new generated files byte-identical. Builtin generator/direct imports aligned. Lint 178/189, 157 failed; final two-unit rechecks give zero new diagnostics, canonical findings 699 → 692. Own `build/error-payload-proof/{verified-clang-tests.log,verified-gcc-tests.log,final-equivalence.json,lint-audit.json}`; 0589. Frozen 210740 retains 9,867/9,872 headers and both local gates/builds; no G1 closure. Door inventory defects stay in 0034. |
| System symbols, integrated after active snapshot | Explicit AGIRU_SYSTEM_SYMBOLS is separately frozen and hashed; links, changed input and inherited live paths refuse. Own suites 89 cases/75 toolchain green both; archived old verifier five assertions red. Real copy 362/362 AL files plus unchanged manifest/reference. Independent census 333 object declarations + 29 namespace files; 283 parse, fifty unsupported stay visible. Own `build/system-symbol-proof/`; 0034/0589. Production AST ingestion remains open. |
| Original System package, integrated after active snapshot | Fetch preserves original NAVX and provenance under version/SHA256; no existing extraction is deleted. Eight new controls; own Clang/GCC 89 cases/83 toolchain green, main eight green; archived fetcher two assertions red. Actual CDN package SHA256 `5b72ba127cb2221722f02544bfb3f3bd5a50402bf92bfab6d7e584e049b9681d`, 634,691 bytes; 370 extracted files/362 AL files byte-identical to the existing package. Original/ledger survive frozen copying. Own `build/symbol-package-proof/`; main `build/symbol-package-main-tests.log`. 0034 owns AST ingestion/provenance consumption; no compatibility/G1 claim. |
| Wider independent census, corrected 2026-10-01 | W1 Tests: 1,402 codeunits / 41,618 methods. All BCApps/src: 36,697 AL files, 36,605 recognized object declarations, 4,137 test codeunits / 112,055 methods and raw Test attributes. Adds uppercase-extension source and two BOM-hidden codeunits (74 + 20 methods) to the previous census. All 19 UTF-16 sources retained. 25,018 files outside configured app roots remain inventoried. GB SalesShipment.Report.al has unresolved conditional brace depths: one unmeasured source and `make census` exit 2, not a full-object census pass. Localization/conditional variants are not a deduplicated runnable suite. `build/llvm-20261001/{source/build/scope-inventory.json,artifacts/raw-inventory-proof.json}`. |
| Seed policy | `agiru_seeded` remains an unsealed legacy template. Disposable clones only; no demo/master mutations. WI 0004. |
| Static analysis | Full September 28: 175/175 units, 161 failed, 1,042 unique diagnostics. Current 181-unit comparison: 689 → 689, no added/removed diagnostic after AllObj changes; no baseline/suppression increase. Earlier TableDefinitions complexity 49 → 46 retained. Not full-surface proof; lint remains red. `build/native-catalogue-20261001/source/build/native-catalogue-proof/lint-comparison.json`. |
| Suppressions/documentation | 55 suppression/silent-place directives versus unchanged baseline 13; public-header Doxygen 1,502 warnings versus baseline 0. `build/review-20260928/{tidy.log,tidy-units.json,doxygen-warnings.txt}`. Baselines may not rise. |
| Not proved | Complete AL tree/link, G1, clients/parity, full ERP suite, multi-user scale and BC-relative performance. Later stages remain gated. |

- Compiler/LLVM predecessor UT: exit 2 after 1,091 s, unchanged consolidation image `1e0e824e772a1a1e611639434f23dc3476095a2fe8d0d68e408a2b6d13077970`. Root 785 fails on PageFieldsSelectionList.cpp:19; unbound System Page Table Field (2000000171) makes Caption resolve to the inherited page method. The own-only binding prototype clears that cause, but main still fails it after the runtime-only promotion; OnPrem tooling remains required. Runner not started: 0 executed, all 2,310 methods missing, 80 codeunits incomplete; exact identities unchanged. Receipts: `build/compiler-llvm-20261001/artifacts/{llvm-compiler-ut,ut-proof}.json`. No G1 claim.
- Chart old-ABI runs are closed. UT exits 2 after 1,500 s with unchanged SHA256 `7d8acdccca3fa0f2a9b031199ac6c4522915d5f686b29de15e638c47a0098cb9`; source population 80/2,310, zero executed, 80 incomplete. Its Tenant License State blocker is historical. `build/chart-20261001/artifacts/{ut.json,ut.log,ut-milestone.log}`. Header census explicitly interrupted for LLVM migration (exit 130): 3,515 passed, zero observed failures, 6,361 unmeasured of 9,876; not a complete-tree verdict.
- Current LLVM migration: fresh `build/llvm-20261001/source`, native C++23 libc++/compiler-rt/libunwind/LLD probe passes. Old compiler outputs/caches were removed (49 configured roots, 4,645 paths); sources and unmerged implementations remain. Current build/test results belong under `build/llvm-20261001/artifacts/`, not the old-ABI rows above. No production/G1/client/performance claim.
- Policy-owner correction: production read root scope.json, while earlier scope gates and inventory read a stale generator-local copy. Historical `product-scope-proof.json` proves provisioning removal, but its Graph-removal control affected only that unused policy; it did not change production selection. Removed the duplicate and made gates/inventory read the production policy. Mixed Microsoft.Integration.Graph local ERP/API helpers remain required; real service retirement needs reference classification, not wholesale namespace removal (0725).
- Canonical-scope baseline: SHA256 `789e19e1da86c33aebf56d2582f22ae5b9fd8c18ec34b471cca024eea27b7f87`, unchanged during verification. Scope 294/294 and toolchain 127/127, zero skips; 89 local cases/26 DB failures. Actual-transpiler file/directory/refusal controls pass; old implementations fail five methods/thirteen assertions plus two counting/path controls. The latter caught inline namespace/object identity loss and trailing double-slash rule drift; both corrected. Apps.cpp targeted analysis passes; Main.cpp remains red (Scan complexity 93), no raised baseline or full-lint claim. Latest compiler/primitive work is recorded in the consolidation row above.
- Main source-revision correction: 135/135 toolchain tests, zero skipped; all previous 130 identities retained. Five new controls pass; previous helper fails two. Actual frozen BCApps no longer borrows outer agiru HEAD; without explicit producer revision it reports unknown. Real BCApps checkout still records `a9ea4d84534cebba852c44bf0f841c2ea149de4e`. Exact frozen UT identities remain 80/2,310, zero gains/losses; tested scripts unchanged and historical receipts untouched. `build/ut-revision-20261001/{proof.json,toolchain.log,previous-controls.log,current-controls.log}`. Git ownership is not clean-tree/content provenance or G1 proof.
- Main manifest correction, preceding revision check: 130/130 toolchain tests, zero skipped, all previous 127 identities retained. Three escaped-boundary-quote controls fail against the previous scanner (two assertions/one false-duplicate error); current scanner and independent token inventory agree. Exact frozen W1 UT identities remain 80/2,310, zero gains/losses. PCH cache gate respects configured `B`; first full run exposed its stale root-build path, repeated full run passes. Unchanged tested script hashes and BCApps revision: `build/manifest-boundary-20261001/{proof.json,toolchain.log,previous-controls.log,current-controls.log,manifest.json}`. No AL execution/G1 claim.
- Raw population remains 36,697 files / 36,605 recognized objects / 4,137 test codeunits / 112,055 methods. Exact object/method identities and source hashes are compared, not only totals. One explicit W1 license/SaaS exclusion: codeunit 138073, fifteen methods; post-product raw methods 112,040, not a configured runnable-suite denominator. Raw UT identities remain exactly 80/2,310; zero exclusions, gains or losses. One GB conditional report remains unmeasured, keeping `make census` red. Actual production-policy generation excludes that test while retaining O365TrialBalance and local GraphCollectionMgtItem; generation alone does not prove compilation/linking or runtime semantics. Receipts: `build/llvm-20261001/artifacts/canonical-scope-proof.json`, `canonical-scope-refactored-{tests,census,generation,lint-driver}.{json,log}`. Full product classification, dependency retirement, runner partition and G1 remain open (0058/0725).
- Frozen results/logs: `build/verify/<id>/{result.json,verify.log,artifacts/}`; no live source inputs may be edited.
- Local loop: `make gate GATE=<affected> JOBS=2`; generator: `make tc JOBS=2`; targeted analysis: `make lint-one UNIT=<file>`.
- Integration: one `make verify-start JOBS=6 VERIFY_TARGETS='all test ut tree apps'` at a time; `make verify-status`.
- Activation: retain the raw source-counted inventory and explicitly justify any approved product-scope partition (0725); identical sealed seed, every selected method compared, every loss investigated. Missing/refused/crashed remain failures. Generated-fixture counts prove C++ execution, not automatic platform-compiler acceptance: AL 17 rejects Option.AsInteger. Retain historical checks; use valid Integer assignments in new AL fixtures (0073).

## Open items and consolidation

- Existing IDs and absorbed requirements retained. IDs 0720–0725 follow the maximum across all Git history (0719); 0725 allocated on 2026-10-01 after also checking current uncommitted IDs through 0724.
- 0030 retains page semantics; 0720 owns client delivery/parity. 0006 retains session ownership; 0721 owns performance qualification.
- 0035 retains native/.NET bridges; 0722 isolates shared JSON number/lifetime safety.
- 0723 isolates the newly reproduced number-sequence reservation/identity contract from 0012's record transactions.
- 0724 owns the browser-only demo project goal; implementation stays gated by G2. The provisional demo notes in 0720 are absorbed, not a second implementation plan.
- 0725 owns the approved licensing/O365/Microsoft-cloud exclusions and dependency retirement; supersedes Tenant License State repair in 0034 and commercial entitlement enforcement in 0062. Raw source/test identities remain under 0058.
- Older consolidation source: `git show 136a5a7:board/<old-name>`. The pre-review uncommitted board is also frozen in `build/verify/20260928T142909Z-7923/source/board/`.
- Absorbed IDs record requirement ownership, not implementation completion.

| WI | Priority | Outcome | Absorbed IDs / mapping |
|---|---|---|---|
| [0725](0725_product_scope_will_exclude_licensing_and_microsoft_cloud_integrations.md) | P0 | Explicit product scope and safe licensing/cloud retirement | Scope decision; licensing repair/enforcement removed from 0034/0062 |
| [0004](0004_provisioning_will_record_source_and_seed_provenance_and_produce_usable_test_databases.md) | P0 | Provisioning will record source and seed provenance and produce usable test databases | 0613 |
| [0006](0006_session_ownership_and_performance_will_have_measured_portable_bounds.md) | P0 | Mutable runtime state will belong to the session | 0008, 0009, 0596 |
| [0012](0012_production_commits_will_be_durable_and_concurrent_writes_will_be_checked.md) | P0 | Production commits will be durable and concurrent writes will be checked | 0060, 0077, 0079, 0087, 0267, 0458, 0463, 0490, 0621 |
| [0013](0013_system_fields_and_schema_keys_will_obey_their_declared_contracts.md) | P1 | System fields and schema keys will obey their declared contracts | 0080, 0353, 0371, 0511 |
| [0018](0018_sql_and_temporary_records_will_evaluate_the_same_filter_grammar.md) | P1 | SQL and temporary records will evaluate the same filter grammar | 0432, 0508, 0509, 0543 |
| [0019](0019_flowfields_will_preserve_semantics_and_use_bounded_aggregate_queries.md) | P1 | FlowFields will preserve semantics and use bounded aggregate queries | 0047, 0340, 0341, 0342, 0507, 0510, 0521, 0678 |
| [0030](0030_one_page_lifecycle_will_drive_testpage_and_the_http_ui.md) | P1 | One page lifecycle will serve TestPage and both clients | 0083, 0198, 0206, 0210, 0211, 0212, 0213, 0218, 0220, 0221, 0233, 0278, 0279, 0280, 0281, 0282, 0283, 0284, 0285, 0286, 0287, 0288, 0289, 0292, 0293, 0294, 0295, 0296, 0298, 0300, 0329, 0330, 0334, 0335, 0336, 0337, 0362, 0374, 0375, 0385, 0387, 0388, 0389, 0390, 0393, 0395, 0399, 0400, 0401, 0402, 0403, 0404, 0405, 0406, 0407, 0408, 0409, 0411, 0413, 0414, 0415, 0416, 0417, 0418, 0419, 0420, 0421, 0422, 0423, 0425, 0426, 0428, 0429, 0430, 0431, 0433, 0434, 0460, 0466, 0469, 0474, 0475, 0477, 0478, 0480, 0482, 0485, 0487, 0496, 0529, 0537, 0538, 0539, 0540, 0541, 0542, 0551, 0553, 0554, 0555, 0560, 0561, 0567, 0568, 0574, 0579, 0657, 0665, 0696, 0700, 0703 |
| [0033](0033_app_boundaries_and_extension_merges_will_be_explicit_and_enforced.md) | P0 | App boundaries and extension merges will be explicit and enforced | 0015, 0069, 0192, 0207, 0209, 0355, 0356, 0357, 0359, 0360, 0363, 0427, 0533, 0544, 0545, 0552, 0572 |
| [0034](0034_every_object_kind_will_have_a_truthful_translation_and_runtime_census.md) | P0 | Every object kind will have a truthful translation and runtime census | 0297, 0365, 0424, 0465, 0479, 0565, 0604, 0623 |
| [0035](0035_rebuilt_net_types_will_preserve_reference_and_culture_semantics.md) | P0 | Rebuilt .NET types will preserve reference and culture semantics | 0078, 0215, 0222, 0227, 0497, 0502, 0585, 0618, 0646, 0675, 0714 |
| [0038](0038_the_complete_generated_tree_will_compile_and_link_without_slice_fallbacks.md) | P1 | The complete generated tree will compile and link without slice fallbacks | 0580, 0591, 0595, 0598, 0599, 0612, 0702 |
| [0039](0039_the_al_runner_will_control_test_lifecycle_and_isolation_explicitly.md) | P0 | The AL runner will control test lifecycle and isolation explicitly | 0223, 0225, 0268, 0269, 0470, 0472, 0493, 0630, 0715 |
| [0043](0043_validation_and_relations_will_run_in_the_documented_order.md) | P1 | Validation and relations will run in the documented order | 0029, 0068, 0228, 0229, 0230, 0232, 0234, 0235, 0236, 0237, 0238, 0239, 0240, 0241, 0242, 0243, 0316, 0317, 0318, 0319, 0320, 0321, 0322, 0325, 0328, 0331, 0332, 0530, 0677 |
| [0044](0044_record_operations_will_share_one_correct_sql_and_temporary_contract.md) | P0 | Record operations will share one correct SQL and temporary contract | 0025, 0032, 0056, 0364, 0398, 0449, 0481, 0505, 0522, 0523, 0607, 0620, 0659, 0697 |
| [0045](0045_reads_will_remain_bounded_and_partial_records_will_be_real.md) | P2 | Reads will remain bounded and partial records will be real | 0017, 0048, 0344, 0345, 0348, 0350, 0351, 0370, 0372, 0520, 0660 |
| [0055](0055_errors_and_labels_will_keep_bc_text_codes_and_navigation_context.md) | P1 | Errors and labels will keep BC text, codes and navigation context | 0382, 0384, 0506, 0518, 0519, 0528, 0566 |
| [0057](0057_events_will_preserve_lifetime_permissions_and_isolated_transaction_semantics.md) | P1 | Events will preserve lifetime, permissions and isolated transaction semantics | 0191, 0196, 0197, 0203, 0204, 0244, 0245, 0246, 0247, 0248, 0249, 0251, 0252, 0253, 0254, 0255, 0256, 0257, 0258, 0259, 0260, 0261, 0262, 0263, 0264, 0265, 0266, 0512, 0513, 0514, 0515, 0516, 0706 |
| [0058](0058_every_ut_run_will_reconcile_results_with_an_independent_source_manifest.md) | P0 | Every UT run will reconcile results with an independent source manifest | — |
| [0059](0059_surface_coverage_will_distinguish_declarations_refusals_and_tested_behaviour.md) | P1 | Surface coverage will distinguish declarations, refusals and tested behaviour | 0028, 0040, 0071, 0358, 0484, 0525, 0562, 0563, 0564, 0588, 0593, 0594, 0610 |
| [0061](0061_consumed_and_discarded_calls_will_keep_al_error_semantics.md) | P1 | Consumed and discarded calls will keep AL error semantics | 0073, 0012 |
| [0062](0062_authorization_will_be_enforced_at_data_and_object_boundaries.md) | P1 | Authorization will be enforced at data and object boundaries | 0202, 0214, 0313, 0314, 0315, 0376, 0377, 0378, 0379, 0380, 0381, 0473, 0483, 0492, 0495, 0499, 0559, 0671 |
| [0063](0063_reports_will_execute_datasets_and_render_declared_layouts.md) | P0 | Reports will execute datasets and render declared layouts | 0301, 0302, 0303, 0304, 0305, 0306, 0307, 0308, 0309, 0391, 0396, 0397, 0436, 0450, 0451, 0452, 0454, 0455, 0457, 0459, 0486, 0488, 0489, 0546, 0547, 0549, 0557, 0575, 0576, 0577, 0628 |
| [0064](0064_queries_will_stream_correct_joins_filters_and_typed_aggregates.md) | P2 | Queries will stream correct joins, filters and typed aggregates | 0299, 0352, 0441, 0453, 0461, 0462, 0464, 0550, 0556 |
| [0065](0065_xmlports_will_preserve_schema_encoding_and_import_transaction_semantics.md) | P1 | XMLports will preserve schema, encoding and import transaction semantics | 0310, 0311, 0312, 0338, 0367, 0410, 0412, 0442, 0443, 0444, 0445, 0446, 0447, 0448, 0501, 0548 |
| [0066](0066_formatting_and_text_operations_will_follow_al_culture_and_character_rules.md) | P1 | Formatting and text operations will follow AL culture and character rules | 0007, 0010, 0016, 0041, 0053, 0075, 0082, 0323, 0324, 0326, 0437, 0438, 0440, 0491, 0503, 0527, 0632, 0716 |
| [0070](0070_install_and_upgrade_will_execute_declared_lifecycle_transactions.md) | P3 | Install and upgrade will execute declared lifecycle transactions | 0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277, 0500, 0534 |
| [0073](0073_generated_expressions_will_preserve_al_types_and_evaluation_effects.md) | P1 | Generated expressions will preserve AL types and evaluation effects | 0027, 0049, 0051, 0076, 0081, 0084, 0085, 0086, 0088, 0089, 0467, 0468, 0524, 0531, 0558, 0573, 0578, 0581, 0584, 0587, 0590, 0597, 0608, 0614, 0629, 0633, 0695 |
| [0074](0074_streams_and_media_will_have_explicit_ownership_and_persistent_storage.md) | P1 | Streams and media will have explicit ownership and persistent storage | 0031, 0200, 0494, 0532, 0535 |
| [0090](0090_background_work_will_have_database_backed_ownership_and_session_isolation.md) | P3 | Background work will have database-backed ownership and session isolation | 0290, 0291, 0498, 0536 |
| [0589](0589_the_toolchain_will_be_reproducible_and_every_gate_will_fail_reliably.md) | P0 | The toolchain will be reproducible and every gate will fail reliably | 0005, 0046, 0050, 0616 |
| [0718](0718_record_images_and_temporary_handles_will_survive_copies_without_dangling_references.md) | P0 | Record images and temporary handles will survive copies without dangling references | 0037, 0042, 0526, 0624, 0640 |
| [0719](0719_error_message_drilldown_will_retrieve_the_logged_record_context.md) | P1 | Error-message drilldown will retrieve the logged record context | — |
| [0720](0720_cli_and_web_will_execute_the_same_erp_operations.md) | P2 | Agent CMD/MCP and web will execute the same ERP operations | Shared HTML/ASCII client and operation parity; page semantics remain 0030 |
| [0721](0721_equivalent_erp_workloads_will_prove_lower_latency_and_resource_cost.md) | P3 | Equivalent ERP workloads will prove lower latency and resource cost | Performance qualification split from 0006; 0008/0009/0596 retained |
| [0722](0722_json_numbers_and_aliases_will_preserve_values_and_lifetimes.md) | P0 | JSON numbers and aliases will preserve values and lifetimes | Shared JSON safety split from 0035 |
| [0723](0723_number_sequence_ranges_will_be_atomic_and_portably_addressed.md) | P0 | Number-sequence ranges will be atomic and portably addressed | New concurrency finding; related boundary ownership remains in 0012 |
| [0724](0724_a_single_user_browser_demo_will_run_entirely_on_github_pages.md) | P3 | Single-user agiru/WASM and embedded PostgreSQL will run entirely on GitHub Pages | Demo target split from 0720 |
