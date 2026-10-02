# 0061 — Consumed and discarded calls will keep AL error semantics

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

BodyWriter.cpp has IsTriedCall/Tried support, restricted to bare or simple member shapes. The old statement that TryFunction never appears in the generator is obsolete. Chained receivers and table-local resolution remain paths to verify.

## Implementation for Sol

1. Represent value consumption explicitly through assignments, conditions, arguments, exit, case selectors and nested expressions; do not infer it from an isolated token spelling.
2. Resolve TryFunction attributes from the callee symbol/type, including table-local and chained calls. Consumed calls catch and set last error; discarded calls propagate normally.
3. Do not insert a savepoint around TryFunction: documented write behaviour differs from Codeunit.Run and asserterror. Keep the three mechanisms separate.
4. Extend the same consumption model to Boolean Record/File/XML operations, preserving only their documented failure-to-false cases.

## Acceptance

Generate and execute the same throwing function in every consumption context. Include a write before error that survives a consumed TryFunction, a discarded call that raises and last-error replacement. Run Incoming Doc. To Data Exch.UT as an activation A/B.

## References

Platform: devenv-handling-errors-using-try-methods.md and attributes/devenv-tryfunction-attribute.md. AL: incoming-document conversion and table-local TryFunctions. Predecessor: WI-1141 and prior value-context findings; obsolete rollback advice from 0226 is rejected.
