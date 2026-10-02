# 0719 — Error-message drilldown will retrieve the logged record context

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-09-28
Depends on: 0061 original error; 0055 typed context; 0030 traps; 0718 ownership.

## Evidence

- Incoming Doc. To Data Exch.UT has conversion, missing-error and unopened-page failures. Their common root is not established.

## Implementation

1. Run the complete codeunit on a fresh clone with `AGIRU_TRACE_ERRORS=1`; preserve the first caught error and initialization order.
2. Trace ErrorMessage insert → RecordId serialization → SetContextFilter → DrillDown/trap. Distinguish no log row, wrong context filter and unopened page.
3. Fix only the demonstrated generic primitive; add one reduced context round trip and one trapped DrillDown case. Feed the scenario into 0720 parity.

## Acceptance

- Correct context retrieves the logged row/message and opens its page; different context returns none. Full UT comparison reports no unexplained losses.

## References

AL: `IncomingDocToDataExchUT.Codeunit.al`, `ErrorMessage.Table.al::{SetContext,SetContextFilter,ShowErrorMessages}`. Code: `src/rt/{RecordRef,TestPage,Transaction}.cpp`. Platform: RecordId and TestPage DrillDown overloads. Predecessor: WI-1141/1235.
