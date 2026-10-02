# 0034 — Every object kind will have a truthful translation and runtime census

Status: open | Priority: P1 | Stage: UT census; All object coverage | Reviewed: 2026-09-28
Depends on: none.

## Evidence

- `src/tc/Main.cpp::UntranslatedKinds` counts filename suffixes; unconventional names/declarations can escape the missing-kind inventory.
- `apps.json` omits test-app roots outside W1 Tests; `scope.json` excludes integration/cloud namespaces. These are visible remaining ERP scope, not evidence of full BC coverage.
- System declarations already exist in `work/symbols/`: Base64Convert codeunit 2000000024, Entity Text table 2000000132 and Web Service table 2000000076. Their absence is not a missing .NET library. Base64Convert's nine `[Native]` methods have deliberately empty symbol bodies.

## Implementation

1. Join source census → parsed AST → emitted file → compiled symbol → registered runtime capability by stable declaration identity. Report absent AL/platform objects separately from .NET types.
2. Count top-level object declarations independently from successful parsing; include every extension and namespace-less source selected by scope.json. Report parsed, emitted, compiled, linked and runnable as different stages.
3. Give each kind a typed AST and explicit writer/runtime registration. Remove page-shaped encoding of unrelated kinds incrementally behind generator fixtures rather than spreading more Boolean mode flags.
4. Wire permission kinds to 0062, reports to 0063, queries to 0064, XMLports to 0065 and control add-ins/profiles to 0030. Keep a refused kind visible until a representative runtime case passes.
5. Replace filename-derived missing-kind guesses with parser dispatch accounting and validate the census against source declarations.
6. Ingest pinned System.app declarations through `scripts/fetch_symbols.py` / SymbolReference.json, retaining package identity/hash and native signatures. Bind Native methods to explicit platform primitives or named refusals; never transpile symbol-only empty bodies into successful no-ops. Generate platform tables from their declared fields/keys/properties.

## Acceptance

- One fixture per object/extension kind, including mixed-case unconventional filenames and a deliberately unsupported declaration. A lost parse cannot lower totals; a header-only stub cannot count as a runnable kind.

## References

Code: `src/al/Parser.cpp`, `src/tc/Main.cpp`, `src/gen/Refused.cpp`.

System sources: `work/symbols/src/System Codeunits/Runtime/Base64Convert.Codeunit.al`, `work/symbols/src/Tenant Database Tables/EntityText.Table.al`, `work/symbols/src/Application Database Tables/WebService.Table.al`; manifest version 28.0.53152.0, Runtime 17.0. Record compatibility with BC_VERSION 28.4.53241.0 and current BCApps main; unequal version strings alone are not proof of incompatibility.

Platform: object declaration documentation. AL: apps.json and src/gen/scope.json populations. Predecessor: object writers are useful findings, not authority for the C++ representation.

Property scope: `apigroup`, `apigroup-page`, `apigroup-query`, `apipublisher`, `apipublisher-page`, `apipublisher-query`, `apiversion`, `apiversion-page`, `apiversion-query`, `definitionfile`, `externalname`, `externalschema`, `horizontalshrink`, `horizontalstretch`, `maximumheight`, `maximumwidth`, `minimumheight`, `minimumwidth`, `odataedmtype`, `odatakeyfields`, `recreatescript`, `refreshscript`, `requestedheight`, `requestedwidth`, `scripts`, `startupscript`, `stylesheets`, `verticalshrink`, `verticalstretch`.
