# 0718 — Record images and temporary handles will survive copies without dangling references

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

`include/runtime/Table.h::CaptureImage` replaces the owned image; `RecordState.h::CopyStateFrom` swaps out the state that owns it. `xRec` is handed out as a reference. The previous WI recorded ASan use-after-free and a candidate stable-image fix, but that fix is absent from HEAD. The full analysis additionally flags uninitialized-state paths in `SharedRecord::Reset` and several RecordRef methods; reproduce them under sanitizers before classifying them. Its historical -7 regression was traced to committed serial rows leaking between tests, not evidence that freed memory was correct.

## Implementation for Sol

1. Create a small fixture that retains xRec through Copy, Get, Modify and nested triggers. Run it under ASan/UBSan before changing ownership.
2. Preserve the destination image owner when copying filter/cursor state. Update an existing image in place when live references can still name it; avoid recursively copying image state. Specify separately when temporary table storage is shared and when record fields are copied.
3. Coordinate with 0039: test-isolation rollback must include explicit commits while preserving a commit across an inner Codeunit.Run rollback. Do not install CommitScope(Ignore) around every test; the historical full run lost 15 cases with that shortcut.
4. Recheck Variant, RecordRef and Instance lifetime bridges against the same fixture. Variant already owns record snapshots; the old claim that it always stores a bare address is obsolete.

## Acceptance

ASan fixture fails before the ownership repair and passes after it. Compare the same complete SCM Available to Pick UT codeunit, Payment Export Validation UT and Price Worksheet Line UT before/after, then the whole milestone. Report crashes and method identities, not only aggregate wins.

## References

Platform: devenv-system-defined-variables.md, methods-auto/record/record-copy-method.md. AL: TrackingSpecification.Table.al and SCMAvailabletoPickUT.Codeunit.al. Predecessor: WI-781, WI-1078, WI-1137, WI-1156; Variant findings WI-1095 and WI-1241.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `testisolation`.
