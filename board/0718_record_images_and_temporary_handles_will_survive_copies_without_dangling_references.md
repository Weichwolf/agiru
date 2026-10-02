# 0718 — Record images and temporary handles will survive copies without dangling references

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

Record images now retain their address across Copy, Insert, Get and Modify for temporary and PostgreSQL records. RecordImageGate passes 28 checks, including AddressSanitizer. The original address control failed before the lifetime repair; four additional allocation-failure controls exposed an incoming-record leak, destruction of an image before a failed clone, and loss of the destination image owner/filter state during failed Copy.

HeldImage now owns incoming records immediately, clones replacements before changing ownership, and prepares CopyStateFrom before moving the live image. Failed allocation/assignment preserves image lifetime and filter-state ownership; throwing field assignment may still partially update image fields.

Two further lifetime defects are repaired: SharedRecord leaked its incoming record when allocating its reference count failed (6 checks, 1 red before); Variant stored the address of a caller-local RecordRef (ASan reproduced stack-use-after-scope). A boxed RecordRef now owns a separate handle sharing the original AL reference state. Copies preserve identity without aliasing the Variant box itself; no dependency from the net tier back to rt was introduced. The combined ASan/UBSan build passes RecordImage 28, Event 34, SharedRecord 6 and RecordRef 89 checks, including same-record mutation through FieldRef.Record and KeyRef.Record. All 74 local programs pass; GCC also builds the tree after the ownership fix. The separate `build/sanitizers` cache uses Clang 19 with `-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -g1 -O1`; run individual cases through `make gate GATE=RecordRefGate B=build/sanitizers JOBS=2`. Latest logs: `build/review-fieldref-negative.log`, `build/review-fieldref-mutation.log`, `build/review-fieldref-test.log`, `build/review-fieldref-gcc.log`.

Snapshot 20260922T125708Z-196091 includes the image, SharedRecord and boxed-RecordRef repairs. Clang and GCC builds and the local gate passed. Its source-counted AL run reached the same 2,310 methods in 80 codeunits as 20260922T120756Z-158412: 2,197 passed, 113 failed, zero incomplete, and all 2,310 method identities, statuses and error texts were identical. The two runs have matching AL source/manifest hashes and work date and the same seed database OID/size hint, but neither has an immutable seed identity. The source revision field on both was incorrectly the agiru checkout; the toolchain fix for future snapshots is in 0058. Do not infer a pass gain from this run.

A further ASan negative control found heap-use-after-free when FieldRef or KeyRef outlived the creating RecordRef. Both now retain the shared RecordRefState, and FieldRef.Record/KeyRef.Record return another handle on that state. Copy, move, same-record mutation and explicit Close behaviour are covered by RecordRefGate: 89 checks, zero red under ASan/UBSan, with the original lifetime control red before the fix. The initial state pointer is retained without a separate allocation. The full AL A/B for this latest repair is pending.

Targeted clang-tidy first reported an analyzer use-after-free path through a temporary FieldRef in `RenameKeys_`. The key now writes through its declared FieldDef and the generic SetFieldText primitive, avoiding that temporary handle. RenameGate passes 14 checks, including a new RecordRef.Rename case that verifies the changed key, the related-row cascade and removal of the old key. The targeted analyzer no longer reports that path; it remains red on unrelated header and existing RecordRef.cpp findings. No suppression or baseline was added.

The previous full run reached all 2,310 methods without a crash, but SCM Available to Pick UT failed 21 methods. Seven share a tracking mismatch where Qty. to Handle (Base) stays 4 while 10 or 125 is expected. Trace the first wrong value on a fresh seed clone through validation, events, temporary Copy and Commit boundaries. Keep test isolation and image lifetime as separate hypotheses; the historical -7 regression was traced to serial rows leaking between tests.

## Implementation for Sol

1. Keep the combined ASan/UBSan gates above green; they now cover Copy, Insert, Get, Modify, allocation failure, boxed RecordRef lifetime and nested validation/modification events. Extend the lifetime audit to the remaining bridges below.
2. Keep the destination image owner when copying filter/cursor state and update existing images in place. The type-erased image operations now live in immutable static data, with an image pointer and one operations pointer per populated state. Retain the failing-before/green-after allocation controls when changing this ownership model. Specify separately when temporary table storage is shared and when record fields are copied.
3. Coordinate with 0039: test-isolation rollback must include explicit commits while preserving a commit across an inner Codeunit.Run rollback. Do not install CommitScope(Ignore) around every test; the historical full run lost 15 cases with that shortcut.
4. Keep FieldRef/KeyRef's shared RecordRefState lifetime and same-record mutation semantics; closing any shared handle invalidates the others safely. Audit the remaining Instance lifetime bridges. BCApps calls FieldRef.Record().SetTable in API pages and stores returned handles in codeunits, so a later move to snapshots must first preserve those call sites' semantics.

## Acceptance

ASan fixture fails before the ownership repair and passes after it. Compare the same complete SCM Available to Pick UT codeunit, Payment Export Validation UT and Price Worksheet Line UT before/after, then the whole milestone. Report crashes and method identities, not only aggregate wins.

## References

Platform: devenv-system-defined-variables.md, methods-auto/record/record-copy-method.md, recordref-gettable-method.md, variant-data-type.md. AL: TrackingSpecification.Table.al and SCMAvailabletoPickUT.Codeunit.al, ApprovalEntryOverview.Page.al (boxes and unboxes RecordRef for Page.RunModal). Predecessor: WI-781, WI-1078, WI-1137, WI-1156; Variant findings WI-1095 and WI-1241.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `testisolation`.
