# 0718 — Record images and temporary handles will survive copies without dangling references

Status: open | Priority: P0 | Stage: UT ownership | Reviewed: 2026-09-28
Depends on: none; activation uses 0058.

## Evidence

- HeldImage preserves addresses across Copy/Get/Insert/Modify; allocation, SharedRecord and boxed RecordRef/FieldRef/KeyRef lifetime regressions have ASan/UBSan gates.
- Remaining raw Instance bridges are not fully audited. Historical SCM tracking mismatches are downstream symptoms, not proof of a remaining lifetime defect.

## Implementation

1. Audit `Instance`, HeldImage, SharedRecord and Variant bridges: explicit owner, aliases, Close/move/destruction behavior, exception guarantee. Preserve temporary storage sharing separately from record/filter/cursor copying.
2. Keep existing image owners stable while assigning fields; prepare allocations before publishing state. Do not promise strong field-value rollback for a throwing assignment unless implemented.
3. Trace the first wrong tracking quantity through validation/events/temp Copy under the corrected runner. Compare SCM Available to Pick, Payment Export Validation and Price Worksheet Line as complete codeunits, then all UT.
4. Preserve shared RecordRefState semantics: FieldRef/KeyRef survive the creating handle, Record() mutations affect the same state and Close invalidates aliases safely.

## Acceptance

- Meaningful failing-before controls cover lifetime, aliases and allocation failure under ASan/UBSan; no new suppression.
- Full source population has zero crashes and no unexplained losses. Do not replace commits with Ignore to mask leaked test state.

## References

Code: `include/runtime/{RecordState,RecordRef,Codeunit}.h`, `include/type/KeyRef.h`, `src/rt/RecordRef.cpp`; gates: `RecordImageGate`, `RecordRefGate`, `SharedRecordGate`, `EventGate`, `RenameGate`. Platform: system-defined variables, Record.Copy, RecordRef.GetTable and Variant contracts. AL: TrackingSpecification, SCMAvailabletoPickUT, ApprovalEntryOverview. Predecessor: WI-781/1078/1137/1156/1095/1241.

Property scope: `testisolation`.
