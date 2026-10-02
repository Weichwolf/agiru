# 0719 — Error-message drilldown will retrieve the logged record context

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

The previous trace plan identifies Incoming Doc. To Data Exch.UT: the conversion failure should be caught, logged under a RecordId context, then exposed by a trapped Error Messages page. It remains a hypothesis until reproduced on the repaired image/runner; no current result was measured in this review.

## Implementation for Sol

1. After 0718 and 0061, run the complete codeunit on a fresh seed and isolate TestProcessWithDataExchSucceeds while preserving its initialization.
2. Trace the error insert and the drilldown select, including RecordId serialization and context filters. Determine whether logging never occurred, the filter differs, or page trapping failed.
3. Fix the generic primitive identified by that trace. Do not special-case the table, test, page name or expected message.
4. Add a reduced error-context round trip and trapped DrillDown fixture, then include it in the HTTP parity suite.

## Acceptance

Logged RecordId context survives write/read/filter and the intended trapped page opens with the same message. An intentionally different context returns no rows; SQL tracing is removed or remains a generic opt-in facility.

## References

AL: IncomingDocToDataExchUT.Codeunit.al, ErrorMessage.Table.al SetContext/SetContextFilter/ShowErrorMessages. Platform: RecordId and TestPage DrillDown methods. Predecessor: search logged-error/context/filter findings after the trace identifies the primitive.
