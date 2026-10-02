# 0718 — Record images and temporary handles will survive copies without dangling references

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

`include/runtime/Table.h::CaptureImage` previously replaced the owned image; `RecordState.h::CopyStateFrom` swapped out the state that owned it, while `xRec` had been handed out as a reference. The new `RecordImageGate` failed on the old implementation because `Copy` changed the image address. `HeldImage` now stores a pointer to immutable type operations and updates a live image in place; `CopyStateFrom` preserves the destination's owner while copying source state. The expanded gate proves Insert updates the same image and Copy retains the destination address while copying a nonblank source image: 71 local gates passed. AddressSanitizer passed with the held reference dereferenced (6 checks). Get/Modify/nested triggers and complete AL A/B remain pending. The full analysis additionally flags uninitialized-state paths in `SharedRecord::Reset` and several RecordRef methods; reproduce them under sanitizers before classifying them. Its historical -7 regression was traced to committed serial rows leaking between tests, not evidence that freed memory was correct.

## Implementation for Sol

1. Extend the failing-before/green-after `RecordImageGate` through Get, Modify and nested triggers, then repeat under ASan/UBSan. The first version covers Copy; the expanded version adds Insert and source-to-destination image content.
2. Keep the destination image owner when copying filter/cursor state and update existing images in place. The type-erased image operations now live in immutable static data, with an image pointer and one operations pointer per populated state. Verify allocation failure leaves the old owner intact. Specify separately when temporary table storage is shared and when record fields are copied.
3. Coordinate with 0039: test-isolation rollback must include explicit commits while preserving a commit across an inner Codeunit.Run rollback. Do not install CommitScope(Ignore) around every test; the historical full run lost 15 cases with that shortcut.
4. Recheck Variant, RecordRef and Instance lifetime bridges against the same fixture. Variant already owns record snapshots; the old claim that it always stores a bare address is obsolete.

## Acceptance

ASan fixture fails before the ownership repair and passes after it. Compare the same complete SCM Available to Pick UT codeunit, Payment Export Validation UT and Price Worksheet Line UT before/after, then the whole milestone. Report crashes and method identities, not only aggregate wins.

## References

Platform: devenv-system-defined-variables.md, methods-auto/record/record-copy-method.md. AL: TrackingSpecification.Table.al and SCMAvailabletoPickUT.Codeunit.al. Predecessor: WI-781, WI-1078, WI-1137, WI-1156; Variant findings WI-1095 and WI-1241.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `testisolation`.
