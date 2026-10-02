# 0055 — Errors and labels will keep BC text, codes and navigation context

Status: open | Priority: P1 | Stage: UT diagnostics; Clients messages | Reviewed: 2026-09-28
Depends on: 0061 error boundaries; 0066 formatting.

## Evidence

- Error codes exist; ErrorScope currently stores collected messages as thread-local strings, losing richer actionable context.
- Client parity needs codes, parameters and record/control targets in addition to displayed text.

## Implementation

1. Carry a structured error value across runtime and transport; retain BC display text but exclude SQL/DSN internals from user-facing errors. Session ownership belongs to 0006.
2. Map PostgreSQL failure classes to typed runtime errors without exposing a different error for the same AL operation. Preserve inner codes when table/page validation adds context.
3. Parse label attributes and carry translation metadata; format diagnostic parameters using AL Format rather than C++ approximations.
4. Implement collected-error continuation, clearing and actionable navigation without adding an implicit transaction rollback. Store record/control/action context as typed metadata.
5. Audit handler kinds and dialog wording through both TestPage and HTTP. Diagnose the specific error-log/drilldown path under 0719 after 0718 and 0061.

## Acceptance

- Exact-message/code fixtures cover FieldError/TestField forms, duplicate/not-found, nested validation, collected errors and ClearLastError. UI error actions address the intended record after a failed transaction.

## References

Code: `src/rt/ErrorInfo.cpp`, `src/rt/Scopes.cpp`, `src/rt/Transaction.cpp`, `include/runtime/Error.h`.

Platform: devenv-error-collection.md, devenv-actionable-errors.md, label properties and methods-auto/errorinfo. AL: Assert.ExpectedError/ExpectedErrorCode consumers. Predecessor: WI-1141 for last-error replacement.

Property scope: `caption`, `captionclass`.
