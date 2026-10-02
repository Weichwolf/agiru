# 0018 — SQL and temporary records will evaluate the same filter grammar

Status: open | Priority: P1 | Stage: UT | Reviewed: 2026-10-01
Depends on: 0044 record state.

## Evidence

- `Filter.cpp` parses filters; `RecordState.cpp` separately splits views without tracking quoted delimiters.
- SQL and temporary evaluation are separate implementations. Parity must include filter-group composition, not only matching leaf predicates.
- Current own GUID defect: `FilterText(Guid)` uses bare storage text; `Temporary.cpp::Passes` takes braced `FieldText`; `Filter.cpp::Satisfies` does not canonicalize GUIDs. Same-value typed SetRange therefore rejects its own temporary row. Clang/GCC direct diagnostic: two checks/one red. Original AL privacy fixture stops at check 16/18; checks 17/18 remain unexecuted, not removed. `build/privacy-binding-20261001/{Privacy.Codeunit.al,GuidFilter.cpp,artifacts/{generated-*,guid-filter-*}.json}`. SQL parity is unmeasured.

## Implementation

1. Compile immutable declaration filters once; bind only session/record values per execution. Keep typed SQL parameters and bounded temporary scans.
2. Define one typed filter AST and quote-aware view grammar. Cover escaped quotes, commas/parentheses inside literals, OR/AND precedence, wildcard modifiers, blank values, open ranges and field types.
3. First repair the shared typed GUID comparison/boundary; preserve display text and ordinary Text semantics. Cover typed SetRange, SetFilter substitution, braced/bare/mixed-case/null GUIDs and unequal GUIDs through record and RecordRef. Then run the same fixtures through SQL and Temporary.cpp, including filter groups, SetRange replacement, CopyFilter, marks, GetRangeMin/Max and RecordId values. Never patch the privacy business objects.
4. Separate UI date/token expansion from SetFilter parsing; resolve metadata filter tokens through declared platform hooks rather than guessing.
5. Reject malformed input with a named diagnostic. Avoid repeatedly parsing immutable CalcFormula/TableRelation text on hot reads.

## Acceptance

- Differential fixtures return the same ordered keys for SQL and temporary tables. Include quoted delimiter characters and closing dates; remove quoting or a filter-group term as negative controls. Preserve benchmark bounds for large filtered reads.
- All eighteen original AL privacy checks execute and pass; the pre-repair matcher fails the typed-GUID control. SQL proof uses a disposable database, not a demo/master source.

## References

Code: `src/rt/Filter.cpp`, `src/rt/Where.cpp`, `src/rt/Selection.cpp`, `src/rt/Temporary.cpp`.

Platform: devenv-entering-criteria-in-filters.md and methods-auto/record filter overloads. AL: No. Series temporary lookups and declared CalcFormula filters. Predecessor: WI-870/889/1063/1136 and the filter findings referenced in the old 0044.

GUID: platform `methods-auto/{guid/guid-data-type,record/record-setrange-method,record/record-setfilter-method}.md`; BCApps current main `System Application/App/Privacy Notice/src/PrivacyNoticeApproval.Codeunit.al::ResetApproval` filters User SID. User intent `business-central/privacy-notices-status.md`. Predecessor 1211 confines GUID-key normalization to typed GUIDs; 1727 requires braced nonblank null-GUID display and rejects 1112's empty-string assumption. No Python storage transplant.

Property scope: `sourcetableview`, `sourcetableview-pages`, `sourcetableview-xmlports`.
