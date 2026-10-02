# 0034 — Every object kind will have a truthful translation and runtime census

Status: open | Priority: P1 | Stage: UT census; All object coverage | Reviewed: 2026-09-30
Depends on: none.

## Evidence

- `src/tc/Main.cpp::UntranslatedKinds` counts filename suffixes; unconventional names/declarations can escape the missing-kind inventory.
- `apps.json` omits test-app roots outside W1 Tests; `scope.json` excludes integration/cloud namespaces. These are visible remaining ERP scope, not evidence of full BC coverage.
- System declarations already exist in `work/symbols/`: Base64Convert codeunit 2000000024, Entity Text table 2000000132 and Web Service table 2000000076. Their absence is not a missing .NET library. Base64Convert's nine `[Native]` methods have deliberately empty symbol bodies.
- Current frozen run `20260930T134502Z-209116` activates computed page sources and exposes missing `Field."Type Name"`: slice build fails in FieldDataClassification; all 2,310 UT methods remain missing, 80 codeunits incomplete, runner not started. This is a platform declaration gap, not permission to suppress the source expression.
- Field census after integrated TypeName repair: 24 declared / 18 runtime fields; missing IDs 10, 23, 60–64. Runtime ExternalName 29 should be 10/Text[100], not Text[2047]. Type/DataClassification members and ObsoleteState inventory differ. Independent probe exits 1; no schema/seed mutation. Own `build/field-native-proof/field-surface-{comparison.json,tsv}` retains every mismatch.
- Typed Get and legacy provisioning share `detail::LoadFieldMetadata`; Field/FieldRef share `FieldOptionMembers`. Classes, enabled state, simple relations, captions, obsolete/external properties and UTF-16-bounded names use declarations. Unknown obsolete states refuse. Native navigation remains incomplete (0044).
- Integrated TypeName proof: System field 9/Text[30]; primitive names plus unseparated Code/Text length (`Code20`, `Text100`), Enum reports `Option`. Existing metadata suffices; no new type-identity structure or production .NET dependency. Unknown types refuse before changing the row. PlatformField 106/106 Clang/GCC; old headers/runtime 46 red. Real generated FieldDataClassification body compiles and executes 6/6 checks under both compilers. Own local suite 85 cases / toolchain 65 tests green. Lint 174/185 units, 167 failed; new fixture diagnostics repaired, no mapper/gate finding or suppression increase. Header-only probe 1.512→1.508 seconds, 191,892→192,032 KiB; single samples, not a general performance claim. Full-UT activation pending.
- Pinned native inspection confirms special Field.Type ordinals, not ordinary external-option semantics: Code is 31489 in Field.Type, 31490 in FieldRef.Type; current internal tag 33 is wrong at both public boundaries. NCLOptionMetadataNavTypeField stores 21 coded indexes; FieldDataProvider subtracts one. The probe checks these against System symbols. Do not expand 37,376 blank members or relax ordinary option density globally.

## Implementation

1. Join source census → parsed AST → emitted file → compiled symbol → registered runtime capability by stable declaration identity. Report absent AL/platform objects separately from .NET types.
2. Count top-level object declarations independently from successful parsing; include every extension and namespace-less source selected by scope.json. Report parsed, emitted, compiled, linked and runnable as different stages.
3. Give each kind a typed AST and explicit writer/runtime registration. Remove page-shaped encoding of unrelated kinds incrementally behind generator fixtures rather than spreading more Boolean mode flags.
4. Wire permission kinds to 0062, reports to 0063, queries to 0064, XMLports to 0065 and control add-ins/profiles to 0030. Keep a refused kind visible until a representative runtime case passes.
5. Replace filename-derived missing-kind guesses with parser dispatch accounting and validate the census against source declarations.
6. Ingest pinned System.app declarations through `scripts/fetch_symbols.py` / SymbolReference.json, retaining package identity/hash and native signatures. Bind Native methods to explicit platform primitives or named refusals; never transpile symbol-only empty bodies into successful no-ops. Generate platform tables from their declared fields/keys/properties.
   Own Field's native declaration and immutable metadata; 0044 owns navigation, 0033 declaring-app identity. Keep internal FieldDef tags distinct from native Field.Type and FieldRef.Type values. Declare one constexpr native map and explicit boundary conversions; ordinary options retain dense ordinals. Route generator Field.Type bindings in BodyWriter/CodeunitWriter through one declaration instead of duplicating special cases. Preserve generic external OptionOrdinalValues independently. Complete field numbers/lengths/properties without inventing blank success values.

## Acceptance

- One fixture per object/extension kind, including mixed-case unconventional filenames and a deliberately unsupported declaration. A lost parse cannot lower totals; a header-only stub cannot count as a runnable kind.
- Field proof reconciles every declared number/name/type/length/option ordinal, tests scalar/enum/extension metadata and FlowField/FlowFilter classes through typed Get and navigation, then compiles the real page body. Compare the unchanged full UT population after activation; absent/refused fields remain visible.

## References

Code: `src/al/Parser.cpp`, `src/tc/Main.cpp`, `src/gen/Refused.cpp`.

System sources: `work/symbols/src/System Codeunits/Runtime/Base64Convert.Codeunit.al`, `work/symbols/src/Tenant Database Tables/EntityText.Table.al`, `work/symbols/src/Application Database Tables/WebService.Table.al`; manifest version 28.0.53152.0, Runtime 17.0. Record compatibility with BC_VERSION 28.4.53241.0 and current BCApps main; unequal version strings alone are not proof of incompatibility.

Field: `work/symbols/src/Virtual Tables/Field.Table.al`, `include/platform/Field.h`, `src/rt/{FieldMetadata.h,written/PlatformField.cpp,Storage.cpp,RecordRef.cpp}`, `src/gen/{BodyWriter.cpp,CodeunitWriter.cpp,Names.cpp}`, `test/gate/PlatformFieldGate.cpp`; platform `devenv-virtual-tables.md`, `properties/devenv-{fieldclass,enabled,obsoletestate,optionordinalvalues}-property.md`, `methods-auto/fieldref/fieldref-{type,optionmembers,optionstring}-method.md`, `onprem/Field-Virtual-Table.md`. Current BCApps `System Application/App/Data Classification/src/FieldDataClassification.Page.al`; user intent `business-central/admin-classifying-data-sensitivity.md`. Predecessor 1040 rejects duplicates; 985 distinguishes names/captions; 1243's generic external-to-internal substitution is not adopted.

Native proof: official platform artifact 28.4.53241.0, `ServiceTier/PFiles64/Microsoft Dynamics NAV/280/Service/Microsoft.Dynamics.Nav.Ncl.dll`, SHA256 `d37240e842d6e259407f27fc100cccc6fd4d885e9dd059503253b516491b4213`. GetFieldTypeName RVA a1c7c, ScaffoldingHelper.GetTypeString 2056f8; GetTypeOptionValue a1b99, NCLOptionMetadataNavTypeField initializer d7c49; NavFieldRef.get_ALType 327a8/get_FieldType 3301c. Own worktree `build/field-native-proof/` retains identity, extraction/inspection scripts and mappings; analysis-only dnfile/dncil, no BC runtime execution or production dependency. Static inspection is not a live BC workload measurement.

Platform: object declaration documentation. AL: apps.json and src/gen/scope.json populations. Predecessor: object writers are useful findings, not authority for the C++ representation.

Property scope: `apigroup`, `apigroup-page`, `apigroup-query`, `apipublisher`, `apipublisher-page`, `apipublisher-query`, `apiversion`, `apiversion-page`, `apiversion-query`, `definitionfile`, `externalname`, `externalschema`, `horizontalshrink`, `horizontalstretch`, `maximumheight`, `maximumwidth`, `minimumheight`, `minimumwidth`, `odataedmtype`, `odatakeyfields`, `recreatescript`, `refreshscript`, `requestedheight`, `requestedwidth`, `scripts`, `startupscript`, `stylesheets`, `verticalshrink`, `verticalstretch`.
