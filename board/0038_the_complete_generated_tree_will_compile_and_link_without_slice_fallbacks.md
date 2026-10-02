# 0038 — The complete generated tree will compile and link without slice fallbacks

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

The current build uses test/slice, unity/PCH and scripts/unlinked.py fallbacks. A green slice proves only the listed sources and can translate missing procedures into runtime refusals. Historical fixed blocker counts in the board are obsolete.

The complete frozen UT run found four such refusals, in `LibraryXMLRead.Initialize`, `AgedAccPayableChart.OnOpenPage`, `LibraryGraphMgt.EnsureWebServiceExist` and `NoSeriesCopilotImpl.IsCopilotVisible`. Their generated `.cpp` files exist. A temporary four-source slice expansion failed at generic compile gaps and was removed before it could become a green baseline. `LibraryXMLRead.cpp` needs `XmlDocument.Load(Text)` and `XmlDeclaration.Version/Encoding/Standalone`; `LibraryGraphMgt.cpp` exposes a `Variant` out argument where `JSONManagement.GetStringPropertyValueFromJObjectByName` requires `Text&`; `AgedAccPayableChart.cpp` refers to a missing generated `BusinessChart` control member; `NoSeriesCopilotImpl.cpp` needs a typed response collection `Get(1)` and a nonambiguous error text conversion. Fix these by their AL/.NET contracts in the generator or runtime, compile each Unity root independently, then add each source to the slice only when green. Rerun the four affected codeunits on fresh seed clones and confirm the old refusals disappear. Append new slice entries until WI 0589 supplies stable groups.

Current follow-up: the generic XML overload and declaration accessors now let `LibraryXMLRead.cpp` pass a standalone Clang 19 syntax check, and that source alone was appended to `test/slice`. The other three candidates failed their standalone checks and were removed from the attempted expansion. This is compile evidence only: `LibraryXMLRead.Initialize` still needs a full codeunit A/B. XML declaration navigation remains a separate semantic gap under 0035.

## Implementation for Sol

1. Run a fresh source/header census through make tree and rank current generic root diagnostics. Fix the highest-impact root in src/gen or public/runtime code, then recompile that root before changing the census.
2. Add compiling sources to the slice without removing existing entries. Compare standalone and unity/PCH compilation to expose accidental include dependencies.
3. Measure unresolved procedures by app and forbid fallback symbols in the complete-app build. Link every declared app dependency and verify load before running tests.
4. Record source revision, scope hash, compiler and source counts with the census. Handle duplicate generated class names deterministically from namespace/app identity.

## Acceptance

`make apps` completes under both compilers; a missing implementation makes the full build red. Slice fallbacks remain loud and counted during transition. A source deliberately removed from compilation must be detected by coverage checks.

## References

Repository: CMakeLists.txt, scripts/tree_syntax.sh, first_gap.sh, unlinked.py, test/slice, src/gen. AL: complete selected source population. Historical 36-root/compile-count claims are intentionally removed.
