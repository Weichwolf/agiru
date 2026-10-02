# 0055 — Errors and labels will keep BC text, codes and navigation context

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

Error codes and RecordErrorGate exist; old claims that GetLastErrorCode is missing are stale. Several public contracts retain refusal descriptions alongside implemented bodies. ErrorInfo collection, translated labels and UI actions need distinct behaviour proofs.

## Implementation for Sol

1. Map PostgreSQL failure classes to typed runtime errors without exposing a different error for the same AL operation. Preserve inner codes when table/page validation adds context.
2. Parse label attributes and carry translation metadata; format diagnostic parameters using AL Format rather than C++ approximations.
3. Implement collected-error continuation, clearing and actionable navigation without adding an implicit transaction rollback. Store record/control/action context as typed metadata.
4. Audit handler kinds and dialog wording through both TestPage and HTTP. Diagnose the specific error-log/drilldown path under 0719 after 0718 and 0061.

## Acceptance

Exact-message/code fixtures cover FieldError/TestField forms, duplicate/not-found, nested validation, collected errors and ClearLastError. UI error actions address the intended record after a failed transaction.

## References

Platform: devenv-error-collection.md, devenv-actionable-errors.md, label properties and methods-auto/errorinfo. AL: Assert.ExpectedError/ExpectedErrorCode consumers. Predecessor: WI-1141 for last-error replacement.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `caption`, `captionclass`.
