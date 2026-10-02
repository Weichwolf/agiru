# 0718 — Record images and temporary handles will survive copies without dangling references

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

Record images now retain their address across Copy, Insert, Get and Modify for temporary and PostgreSQL records. RecordImageGate passes 28 checks, including AddressSanitizer. The original address control failed before the lifetime repair; four additional allocation-failure controls exposed an incoming-record leak, destruction of an image before a failed clone, and loss of the destination image owner/filter state during failed Copy.

HeldImage now owns incoming records immediately, clones replacements before changing ownership, and prepares CopyStateFrom before moving the live image. Failed allocation/assignment preserves image lifetime and filter-state ownership; throwing field assignment may still partially update image fields.

Two further lifetime defects are repaired: SharedRecord leaked its incoming record when allocating its reference count failed (6 checks, 1 red before); Variant stored the address of a caller-local RecordRef (ASan reproduced stack-use-after-scope). A boxed RecordRef now owns a separate handle sharing the original AL reference state. Copies preserve identity without aliasing the Variant box itself; no dependency from the net tier back to rt was introduced. The combined ASan/UBSan build passes RecordImage 28, Event 34, SharedRecord 6 and RecordRef 81 checks. All 72 local programs pass with those assertions included. The separate `build/sanitizers` cache uses Clang 19 with `-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -g1 -O1`; run individual cases through `make gate GATE=RecordRefGate B=build/sanitizers JOBS=2`. Final logs: `build/review-sanitizers-final.log` and `build/review-sanitizers-final-rest.log`.

Snapshot 20260922T120756Z-158412 includes the earlier image/allocation repair, but not the new SharedRecord/Variant repairs. Both need the next full AL A/B before this item can close. The initialization reports were reproduced in an independent aggregate with default member initializers. Clang 19 reports garbage for `new State{}` although its ASan/UBSan executable confirms initialization; an explicit defaulted constructor removes that analyzer false positive. RecordRefState now uses that form, and the targeted analysis has no initialization reports. Other lint debt remains. FieldRef.Record and KeyRef.Record still produce borrowed RecordRef buffers.

The previous full run reached all 2,310 methods without a crash, but SCM Available to Pick UT failed 21 methods. Seven share a tracking mismatch where Qty. to Handle (Base) stays 4 while 10 or 125 is expected. Trace the first wrong value on a fresh seed clone through validation, events, temporary Copy and Commit boundaries. Keep test isolation and image lifetime as separate hypotheses; the historical -7 regression was traced to serial rows leaking between tests.

## Implementation for Sol

1. Keep the combined ASan/UBSan gates above green; they now cover Copy, Insert, Get, Modify, allocation failure, boxed RecordRef lifetime and nested validation/modification events. Extend the lifetime audit to the remaining bridges below.
2. Keep the destination image owner when copying filter/cursor state and update existing images in place. The type-erased image operations now live in immutable static data, with an image pointer and one operations pointer per populated state. Retain the failing-before/green-after allocation controls when changing this ownership model. Specify separately when temporary table storage is shared and when record fields are copied.
3. Coordinate with 0039: test-isolation rollback must include explicit commits while preserving a commit across an inner Codeunit.Run rollback. Do not install CommitScope(Ignore) around every test; the historical full run lost 15 cases with that shortcut.
4. Recheck FieldRef.Record, KeyRef.Record and Instance lifetime bridges. Their borrowed buffers remain distinct from the repaired Variant-to-RecordRef handle. Variant owns record snapshots and now also retains RecordRef handles; never describe all Variant alternatives as borrowed.

## Acceptance

ASan fixture fails before the ownership repair and passes after it. Compare the same complete SCM Available to Pick UT codeunit, Payment Export Validation UT and Price Worksheet Line UT before/after, then the whole milestone. Report crashes and method identities, not only aggregate wins.

## References

Platform: devenv-system-defined-variables.md, methods-auto/record/record-copy-method.md, recordref-gettable-method.md, variant-data-type.md. AL: TrackingSpecification.Table.al and SCMAvailabletoPickUT.Codeunit.al, ApprovalEntryOverview.Page.al (boxes and unboxes RecordRef for Page.RunModal). Predecessor: WI-781, WI-1078, WI-1137, WI-1156; Variant findings WI-1095 and WI-1241.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `testisolation`.
