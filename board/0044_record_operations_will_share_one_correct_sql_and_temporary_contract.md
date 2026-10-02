# 0044 — Record operations will share one correct SQL and temporary contract

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

Find/Get/Init/Copy/DeleteAll and temporary storage are implemented; old blanket refusal inventories are obsolete. The review fixed platform Field.Get ignoring temporary rows by routing temporary instances through Table<Field>::Get; PlatformFieldGate is green. `Table.h` and RecordRef expose parallel paths that can diverge in discarded-return semantics, filter state and ownership.

## Implementation for Sol

1. Write a small operation matrix over typed Record, RecordRef and temporary records: Init versus Clear, assignment versus Copy, Copy(ShareTable), Get versus filters, Find directions, marks and ModifyAll/DeleteAll triggers.
2. Centralize primitives below the typed wrappers while retaining typed field access. Preserve table variables and temporary ownership according to operation, not a general C++ copy rule.
3. Implement documented Boolean result versus statement-raises behaviour consistently; only recognized not-found/duplicate conditions may become false. Preserve database faults.
4. Complete computed platform tables from system symbols and requested ranges, avoiding fixed-date population as the authoritative implementation.

## Acceptance

The same fixture yields identical keys, field values, filters and events through all three paths. Include no-match, duplicate, negative Next, absent key and malformed typed key controls. Handle lifetime tests belong to 0718.

## References

Platform: methods-auto/record and recordref overloads, devenv-temporary-tables.md and virtual-table pages. AL: No. Series temporary filters and platform table users. Predecessor: WI-1063/1136/1173/1206/1229; retain source usage as a fixture, never a hardcoded runtime branch.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `enableexternalassemblies`, `externalaccess`, `externaltype`, `initvalue`, `iscontroladdin`, `optionordinalvalues`, `provider`, `publickeytoken`, `tabletype`, `usetemporary`, `usetemporary-report`, `usetemporary-xmlport`.
