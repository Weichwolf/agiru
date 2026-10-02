# 0034 — Every object kind will have a truthful translation and runtime census

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

Tables, codeunits, enums, pages, interfaces, queries, reports, XMLports and profiles have implementation paths. `src/tc/Main.cpp::UntranslatedKinds` is a hardcoded filename-suffix list; it still lists reportext and can miss actual declaration spellings. Permissions, entitlements and control add-ins remain material gaps.

## Implementation for Sol

1. Count top-level object declarations independently from successful parsing; include every extension and namespace-less source selected by scope.json. Report parsed, emitted, compiled, linked and runnable as different stages.
2. Give each kind a typed AST and explicit writer/runtime registration. Remove page-shaped encoding of unrelated kinds incrementally behind generator fixtures rather than spreading more Boolean mode flags.
3. Wire permission kinds to 0062, reports to 0063, queries to 0064, XMLports to 0065 and control add-ins/profiles to 0030. Keep a refused kind visible until a representative runtime case passes.
4. Replace filename-derived missing-kind guesses with parser dispatch accounting and validate the census against source declarations.

## Acceptance

One fixture per object/extension kind, including mixed-case unconventional filenames and a deliberately unsupported declaration. A lost parse cannot lower totals; a header-only stub cannot count as a runnable kind.

## References

Platform: object declaration documentation. AL: apps.json and src/gen/scope.json populations. Predecessor: object writers are useful findings, not authority for the C++ representation.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `apigroup`, `apigroup-page`, `apigroup-query`, `apipublisher`, `apipublisher-page`, `apipublisher-query`, `apiversion`, `apiversion-page`, `apiversion-query`, `definitionfile`, `externalname`, `externalschema`, `horizontalshrink`, `horizontalstretch`, `maximumheight`, `maximumwidth`, `minimumheight`, `minimumwidth`, `odataedmtype`, `odatakeyfields`, `recreatescript`, `refreshscript`, `requestedheight`, `requestedwidth`, `scripts`, `startupscript`, `stylesheets`, `verticalshrink`, `verticalstretch`.
