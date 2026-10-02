# 0718 — Record images and temporary handles will survive copies without dangling references

Status: open | Priority: P0 | Reviewed: 2026-09-22

## Current evidence

Record images now retain their address across Copy, Insert, Get and Modify for temporary and PostgreSQL records. RecordImageGate passes 28 checks, including AddressSanitizer. The original address control failed before the lifetime repair; four additional allocation-failure controls exposed an incoming-record leak, destruction of an image before a failed clone, and loss of the destination image owner/filter state during failed Copy.

HeldImage now owns incoming records immediately, completes replacement cloning before ownership changes, and CopyStateFrom allocates destination state before moving the live image. Failed allocation/assignment preserves image lifetime and filter-state ownership; arbitrary throwing field assignment may still partially update image fields. All 71 local gates pass. This exception-path repair is newer than snapshot 20260922T114239Z-133899 and needs the next integration A/B. EventGate now passes 34 normal/ASan checks, including retained outer xRec references across nested Modify and Validate calls. UBSan and the analyzer's SharedRecord/RecordRef findings remain open.

The previous full run reached all 2,310 methods without a crash, but SCM Available to Pick UT failed 21 methods. Seven share a tracking mismatch where Qty. to Handle (Base) stays 4 while 10 or 125 is expected. Trace the first wrong value on a fresh seed clone through validation, events, temporary Copy and Commit boundaries. Keep test isolation and image lifetime as separate hypotheses; the historical -7 regression was traced to serial rows leaking between tests.

## Implementation for Sol

1. Repeat RecordImageGate and EventGate under UBSan. Copy, Insert, Get, Modify and allocation failure have 28 passing normal/ASan image controls; EventGate has 34, including retained-reference assertions inside nested validation and modification events. Extend the lifetime audit to the remaining bridges below.
2. Keep the destination image owner when copying filter/cursor state and update existing images in place. The type-erased image operations now live in immutable static data, with an image pointer and one operations pointer per populated state. Retain the failing-before/green-after allocation controls when changing this ownership model. Specify separately when temporary table storage is shared and when record fields are copied.
3. Coordinate with 0039: test-isolation rollback must include explicit commits while preserving a commit across an inner Codeunit.Run rollback. Do not install CommitScope(Ignore) around every test; the historical full run lost 15 cases with that shortcut.
4. Recheck Variant, RecordRef and Instance lifetime bridges against the same fixture. Variant already owns record snapshots; the old claim that it always stores a bare address is obsolete.

## Acceptance

ASan fixture fails before the ownership repair and passes after it. Compare the same complete SCM Available to Pick UT codeunit, Payment Export Validation UT and Price Worksheet Line UT before/after, then the whole milestone. Report crashes and method identities, not only aggregate wins.

## References

Platform: devenv-system-defined-variables.md, methods-auto/record/record-copy-method.md. AL: TrackingSpecification.Table.al and SCMAvailabletoPickUT.Codeunit.al. Predecessor: WI-781, WI-1078, WI-1137, WI-1156; Variant findings WI-1095 and WI-1241.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `testisolation`.
