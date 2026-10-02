# 0038 — The complete generated tree will compile and link without slice fallbacks

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

The current build uses test/slice, unity/PCH and scripts/unlinked.py fallbacks. A green slice proves only the listed sources and can translate missing procedures into runtime refusals. Historical fixed blocker counts in the board are obsolete.

The complete frozen UT run found four such refusals, in `LibraryXMLRead.Initialize`, `AgedAccPayableChart.OnOpenPage`, `LibraryGraphMgt.EnsureWebServiceExist` and `NoSeriesCopilotImpl.IsCopilotVisible`. Their generated `.cpp` files exist. A temporary four-source slice expansion failed at generic compile gaps and was removed before it could become a green baseline. `LibraryXMLRead.cpp` needs `XmlDocument.Load(Text)` and `XmlDeclaration.Version/Encoding/Standalone`; `LibraryGraphMgt.cpp` exposes a `Variant` out argument where `JSONManagement.GetStringPropertyValueFromJObjectByName` requires `Text&`; `AgedAccPayableChart.cpp` refers to a missing generated `BusinessChart` control member; `NoSeriesCopilotImpl.cpp` needs a typed response collection `Get(1)` and a nonambiguous error text conversion. Fix these by their AL/.NET contracts in the generator or runtime, compile each Unity root independently, then add each source to the slice only when green. Rerun the four affected codeunits on fresh seed clones and confirm the old refusals disappear. Append new slice entries until WI 0589 supplies stable groups.

Current follow-up: the generic XML overload and declaration accessors now let `LibraryXMLRead.cpp` pass a standalone Clang 19 syntax check, and that source alone was appended to `test/slice`. The other three candidates failed their standalone checks and were removed from the attempted expansion. This is compile evidence only: `LibraryXMLRead.Initialize` still needs a full codeunit A/B. XML declaration navigation remains a separate semantic gap under 0035.

`NoSeriesCopilotImpl.cpp` also now passes standalone Clang 19 and GCC 14 syntax checks and was appended to the slice. The cause was generic: a chained call on an explicitly refused cloud bridge had no `Get` member, and `Error(Refused)` became an ambiguous conversion. `Refused::Get` preserves the member-specific runtime refusal, while the `RaiseOrCollect` refusal overload invokes it directly. `RefusedGate` has two positive controls for those paths. This makes the cloud bridge compilable without pretending Azure OpenAI is implemented; the app's user-facing Copilot path remains explicitly refused until a portable backend exists.

`AgedAccPayableChart.cpp` now also passes standalone Clang 19 and GCC 14 syntax checks and was appended to the slice. Page generation had emitted `usercontrol` only into page metadata, leaving `CurrPage.BusinessChart` with no C++ member. The generator now emits every user control as its declared absent control-add-in member and includes its explicit contract. `GenPageGate` verifies the generated member and include. This preserves the explicit refusal until the control add-in is rebuilt; it does not make chart rendering work. `make transpile` regenerated 18,905 objects from the current BCApps source, preserving the independent source census of 80 UT codeunits and 2,310 methods.

The next frozen slice build exposed the generic completeness hole in that first user-control repair: `Item Tracking Lines` names `CameraBarcodeScannerProviderAddIn`, which was emitted as a page member but never collected into generated `absent/Types.h`. `WritePage` now records every user-control source type in the absent contract; `GenPageGate` verifies both the member and contract. The same compile reached an AL expression with explicit nested equality, `(SourceType = 900) = IsAssembleToOrder`, whose flattened C++ output tripped GCC's parenthesis warning. `BodyWriter` now preserves the AST's left grouping for relational operator chains; `GenCodeunitGate` verifies the exact parenthesized output. After regeneration, the previously failing `ItemTrackingLines.cpp` passes direct Clang 19 and GCC 14 syntax checks. The focused 76-case C++ gate and the GCC runtime/transpiler build pass. The complete validation is pending in a fresh frozen snapshot.

## Implementation for Sol

1. Run a fresh source/header census through make tree and rank current generic root diagnostics. Fix the highest-impact root in src/gen or public/runtime code, then recompile that root before changing the census.
2. Add compiling sources to the slice without removing existing entries. Compare standalone and unity/PCH compilation to expose accidental include dependencies.
3. Measure unresolved procedures by app and forbid fallback symbols in the complete-app build. Link every declared app dependency and verify load before running tests.
4. Record source revision, scope hash, compiler and source counts with the census. Handle duplicate generated class names deterministically from namespace/app identity.

## Acceptance

`make apps` completes under both compilers; a missing implementation makes the full build red. Slice fallbacks remain loud and counted during transition. A source deliberately removed from compilation must be detected by coverage checks.

## References

Repository: CMakeLists.txt, scripts/tree_syntax.sh, first_gap.sh, unlinked.py, test/slice, src/gen. AL: complete selected source population. Historical 36-root/compile-count claims are intentionally removed.
