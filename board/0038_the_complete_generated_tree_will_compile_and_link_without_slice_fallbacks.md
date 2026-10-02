# 0038 — The complete generated tree will compile and link without slice fallbacks

Status: open | Priority: P1 | Stage: UT complete in-scope tree | Reviewed: 2026-09-28
Depends on: 0033 app graph; 0034 declaration census; 0073 expression lowering.

## Evidence

- `test/slice` and `scripts/unlinked.py` permit a linked subset with counted runtime refusals. A green slice is not a complete app build.
- LibraryXMLRead, AgedAccPayableChart, NoSeriesCopilotImpl and LibraryGraphMgt now compile through the slice; remaining runtime refusals still count.
- `Base64ConvertImpl.h` holds absent platform codeunit `Base64Convert`, distinct from generated AL `Base64 Convert`. EntityText table and WebService are also absent AL objects, despite the shared '.NET member' diagnostic.

## Implementation

1. Run `make tree`; persist generic root diagnostics by app and source identity. Fix roots in src/gen/runtime, verify standalone Clang/GCC without PCH, then append compiling sources to the slice.
2. Resolve every referenced AL symbol by app + namespace + kind + ID. Separate excluded AL, unresolved platform symbols, missing linked bodies and genuine .NET members; keep actual AL source declarations authoritative.
3. Compile/link one library per app with only public headers and declared dependency roots. Full-app targets reject unresolved symbols and every fallback stub.
4. Bind generated outputs to BCApps content, scope/apps manifests and generator hash; reject stale output even when method names and counts match. Do not declare completion from the slice census.

## Acceptance

- `make apps` completes under Clang 19 and GCC 14 for the complete in-scope tree; all source/parse/emit/compile/link populations reconcile.
- Removing a source or swapping same-named stale output fails the gate. Runtime refusals remain failures until their real contracts execute.

## References

Code: `CMakeLists.txt`, `src/tc/Main.cpp`, `src/gen/{Names,Scope,BodyWriter,PageWriter}.cpp`, `scripts/{tree_syntax.sh,first_gap.sh,unlinked.py}`, `test/slice`. AL: System Application Base64 Convert/Entity Text and system-symbol Web Service declarations. Predecessor: WI-990 scope boundary; retain exclusions as visible ERP gaps.
