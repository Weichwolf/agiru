# 0044 — Record operations will share one correct SQL and temporary contract

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-09-30
Depends on: 0718 images; 0013 schema metadata.

## Evidence

- Integrated unfiltered `GetRangeMin/Max` refusal in the common `Filter.cpp::RangeBoundText`. RangeBoundGate: 32/32 Clang/GCC; old runtime: 20 red. Covers regular/temporary records, borrowed/owned FieldRef, cleared/unrelated filters, exact Decimal scale and explicit blank equality. FilterGate 122/122, RecordRefGate 89/89; 84 local cases green. Direct includes repaired; targeted analysis has only inherited-header findings, with no new gate findings. Frozen full UT retains all 2,310 identities with no status/diagnostic changes; diagnostic legacy-seed evidence, not causal A/B (README). Logs: `/home/cosmo/Git/agiru-worktrees/goal-20260928/build/range-bound-*.log`. Internal empty-bound parsing is unchanged; local docs do not supply exact BC diagnostic wording.

- Integrated Next(0) preserves SQL/temporary typed Record and RecordRef positions and pending field values; omitted Steps remains one. `NextZeroGate`: Clang 19 and GCC 14 each 32 checks green; unchanged runtime negative control: 16 red. Logs: `build/review-20260928/next-zero-{negative,positive}.log`.
- Integrated partial/exhausted Next preserves the last reached row and permits reversal; INT_MIN uses a widened magnitude. Typed SQL/temporary and RecordRef paths, including multiple fetch blocks: `NextZeroGate` 112 checks green under Clang 19/GCC 14; frozen pre-fix runtime 44 red. `CursorGate` 217 checks green under both compilers, including exact full-block exhaustion; retains one bounded block, not the full set. Complete local test: 80 cases, 0 red. Logs: `build/review-20260928/next-partial-{final-gates,gcc,frozen-negative}.log`, `cursor-endpoint-gcc.log`. Current frozen verification includes the patch; full UT identities/statuses/diagnostics match September 28 (README), without sealed-seed causal proof.
- Targeted analysis of Navigate/Temporary/Cursor still refuses existing public-header findings and TempFind complexity; no finding in the changed functions, no suppression or baseline increase. Logs: `build/review-20260928/next-{Navigate,Temporary,Cursor}-tidy.log`.
- SQL keyset comparison builds a uniform tuple comparison even when selected key fields descend. SQL/temporary/RecordRef need one operation matrix.
- `ChangeCompany` refuses; SQL names use only TableDef.name. CompanyName alone does not isolate company data.
- Reproduced on SQL and temporary rows: Next(0) moves 1→2. SQL descending Next(-1) moves 2→1 instead of 2→3 (`build/review-20260928/runtime-probe.log`).
- `Selection.cpp::Series` silently caps the virtual Integer population at 1,000,000 admitted rows. The documented domain is ±1,000,000,000; a fetch bound must not change Count or omit records.

## Implementation

1. Add reverse/mixed key directions and filter/key-change fixtures. Build lexicographic predicates per key direction plus deterministic primary-key tie-breakers; route table identity through explicit company context. Preserve the completed zero/partial/exhausted/extreme-step gates.
2. Write a small operation matrix over typed Record, RecordRef and temporary records: Init versus Clear, assignment versus Copy, Copy(ShareTable), Get versus filters, Find directions, marks and ModifyAll/DeleteAll triggers.
3. Centralize primitives below the typed wrappers while retaining typed field access. Preserve table variables and temporary ownership according to operation, not a general C++ copy rule.
4. Implement documented Boolean result versus statement-raises behaviour consistently; only recognized not-found/duplicate conditions may become false. Preserve database faults.
5. Complete computed platform tables from system symbols and requested ranges, avoiding fixed-date population as the authoritative implementation.
   Keep the Integer domain intact; bound transfer through cursors, not a truncated relation. Gate filtered Count and navigation across the former million-row cutoff.

## Acceptance

- The same fixture yields identical keys, field values, filters and events through all three paths. Include no-match, duplicate, negative Next, absent key and malformed typed key controls. Handle lifetime tests belong to 0718.

## References

Range bounds: platform `methods-auto/{record,fieldref}/*-getrangemin-method.md` and `*-getrangemax-method.md`; BCApps main `a9ea4d84534cebba852c44bf0f841c2ea149de4e`, `src/System Application/App/Email/src/Email/Sent/SentEmails.Query.al` and `src/System Application/App/Extension Management/src/ExtensionSettings.Page.al` guard calls with GetFilter. User docs contain no separate contract. Predecessor `~/Git/openerp/board/1715_getrangemin-without-filter-must-raise.md` distinguishes public errors from internal blank FlowFilter bounds. Preserve `RangeBoundOf` and the existing FlowFilter gate.

Partial/extreme steps: platform `methods-auto/{record,recordref}/*-next-method.md`; BCApps current main `src/Layers/W1/Tests/Cost Accounting/ERMCAGLTransfer.Codeunit.al::ValidateTransfer` and `src/Layers/W1/Tests/Dimension/DimensionCorrectionTests.Codeunit.al` use non-unit steps. User intent: `dynamics365smb-docs/archive/WorkingWithDynamics/sorting.md`. Predecessor `openerp/board/1102_persistentes-next-ignoriert-filteraenderungen-im-ergebnissat.md` identifies filter/key invalidation; that separate gap remains open here.

Code: `src/rt/{Record,Navigate,Temporary,Selection,RecordRef,PlatformTables}.cpp`, `include/runtime/Table.h`. Platform: `methods-auto/record/record-next-method.md`, `methods-auto/recordref/recordref-next-method.md`, other Record/RecordRef overloads, devenv-temporary-tables.md, devenv-integer-virtual-table.md. AL: `src/Layers/RU/Tests/Local/ERMVATReinstatement.Codeunit.al::SuggestVATSettlement` explicitly calls temporary Next(0); No. Series temporary filters and platform table users. No dedicated user-facing Next(0) contract; platform method documentation governs. Predecessor board searched for Next(0), with no matching finding; WI-1063/1136/1173/1206/1229 cover adjacent record contracts. Retain source usage as a fixture, never a hardcoded runtime branch.

Property scope: `enableexternalassemblies`, `externalaccess`, `externaltype`, `initvalue`, `iscontroladdin`, `optionordinalvalues`, `provider`, `publickeytoken`, `tabletype`, `usetemporary`, `usetemporary-report`, `usetemporary-xmlport`.
