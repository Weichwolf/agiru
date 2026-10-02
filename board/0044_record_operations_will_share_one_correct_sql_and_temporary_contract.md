# 0044 — Record operations will share one correct SQL and temporary contract

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-09-28
Depends on: 0718 images; 0013 schema metadata.

## Evidence

- `Navigate.cpp::RuntimeNext` and `Temporary.cpp::TempNext` convert zero steps into one. The Next overload documentation requires staying on the current row.
- SQL keyset comparison builds a uniform tuple comparison even when selected key fields descend. SQL/temporary/RecordRef need one operation matrix.
- `ChangeCompany` refuses; SQL names use only TableDef.name. CompanyName alone does not isolate company data.
- Reproduced on SQL and temporary rows: Next(0) moves 1→2. SQL descending Next(-1) moves 2→1 instead of 2→3 (`build/review-20260928/runtime-probe.log`).
- `Selection.cpp::Series` silently caps the virtual Integer population at 1,000,000 admitted rows. The documented domain is ±1,000,000,000; a fetch bound must not change Count or omit records.

## Implementation

1. Add Next(0), reverse/mixed key directions and cursor-exhaustion fixtures first. Build lexicographic predicates per key direction plus deterministic primary-key tie-breakers; route table identity through explicit company context.
2. Write a small operation matrix over typed Record, RecordRef and temporary records: Init versus Clear, assignment versus Copy, Copy(ShareTable), Get versus filters, Find directions, marks and ModifyAll/DeleteAll triggers.
3. Centralize primitives below the typed wrappers while retaining typed field access. Preserve table variables and temporary ownership according to operation, not a general C++ copy rule.
4. Implement documented Boolean result versus statement-raises behaviour consistently; only recognized not-found/duplicate conditions may become false. Preserve database faults.
5. Complete computed platform tables from system symbols and requested ranges, avoiding fixed-date population as the authoritative implementation.
   Keep the Integer domain intact; bound transfer through cursors, not a truncated relation. Gate filtered Count and navigation across the former million-row cutoff.

## Acceptance

- The same fixture yields identical keys, field values, filters and events through all three paths. Include no-match, duplicate, negative Next, absent key and malformed typed key controls. Handle lifetime tests belong to 0718.

## References

Code: `src/rt/{Record,Navigate,Temporary,Selection,RecordRef,PlatformTables}.cpp`, `include/runtime/Table.h`. Platform: methods-auto/record and recordref overloads, devenv-temporary-tables.md, devenv-integer-virtual-table.md. AL: No. Series temporary filters and platform table users. Predecessor: WI-1063/1136/1173/1206/1229; retain source usage as a fixture, never a hardcoded runtime branch.

Property scope: `enableexternalassemblies`, `externalaccess`, `externaltype`, `initvalue`, `iscontroladdin`, `optionordinalvalues`, `provider`, `publickeytoken`, `tabletype`, `usetemporary`, `usetemporary-report`, `usetemporary-xmlport`.
