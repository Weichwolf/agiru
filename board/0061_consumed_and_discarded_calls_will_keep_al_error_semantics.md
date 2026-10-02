# 0061 — Consumed and discarded calls will keep AL error semantics

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-09-28
Depends on: 0073 resolved value context; 0012 separate error boundaries.

## Evidence

- `BodyWriter.cpp` has IsTriedCall/Tried for limited receiver shapes; table-local/chained resolution remains unproved.
- Incoming-document conversion assertions hide the first error; existing `AGIRU_TRACE_ERRORS=1` can expose caught boundaries.

## Implementation

1. Trace one complete failing codeunit before classifying its conversion failures. Keep TryFunction, Boolean Codeunit.Run and asserterror policies distinct.
2. Represent value consumption explicitly through assignments, conditions, arguments, exit, case selectors and nested expressions; do not infer it from an isolated token spelling.
3. Resolve TryFunction attributes from the callee symbol/type, including table-local and chained calls. Consumed calls catch and set last error; discarded calls propagate normally.
4. Do not insert a savepoint around TryFunction: documented write behaviour differs from Codeunit.Run and asserterror. Keep the three mechanisms separate.
   Make DisableWriteInsideTryFunctions an explicit runtime policy: on-premises default rejects writes in consumed try calls, online/explicitly allowed mode does not roll them back. Argument evaluation belongs inside the consumed-call catch/write-policy boundary; discarded calls use ordinary rules.
5. Extend the same consumption model to Boolean Record/File/XML operations, preserving only their documented failure-to-false cases.

## Acceptance

- Generate/execute every consumption context. In write-allowed mode, a write before a caught error survives; in write-disabled mode it refuses before mutation, including argument-side writes. Discarded calls raise normally; last-error replacement remains exact. Run Incoming Doc. To Data Exch.UT as an activation A/B.

## References

Code: `src/gen/BodyWriter.cpp`, `include/runtime/Error.h`, `include/runtime/Codeunit.h`.

Platform: devenv-handling-errors-using-try-methods.md and attributes/devenv-tryfunction-attribute.md. AL: incoming-document conversion and table-local TryFunctions. Predecessor: WI-1141 and prior value-context findings; obsolete rollback advice from 0226 is rejected.
