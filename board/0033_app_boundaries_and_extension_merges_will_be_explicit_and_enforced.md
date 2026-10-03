# 0033 — App boundaries and extension merges will be explicit and enforced

Status: open | Priority: P0 | Stage: UT symbol identity and app isolation | Reviewed: 2026-10-02
Depends on: 0034 source identities.

## Evidence

- UT bootstrap repair (2026-10-03): `Main::TakeFields` no longer keeps a moved source
  ahead of its destination. Match field ID/name/type/length and reciprocal
  MovedTo/MovedFrom against original app IDs, in either extension order. The three
  original Business Foundation legacy extensions contain 113 moved fields: 82
  takeovers resolve; 31 have no W1 destination and are logged individually as
  inactive sources, not mapped into Field.ObsoleteState's three SDK options.
  Existing SQL columns are not dropped. Raw source inventory and 80/2,314 UT remain
  unchanged. Complete regeneration changes 16 files/removes two obsolete outputs;
  slice remains 14,222/zero missing. Existing NativeSourceCompilerGate passes 14
  tests; the previous frozen compiler fails all three takeover controls. This is
  not complete app ownership, localization, migration or G1 proof.
- Ordinary tables now bind once after extension merging, using the writer's field/procedure allocator. Four original generated AL cases prove post-merge procedure renaming, extension procedures and numeric/name aliases; the old transpiler fails all four. App/namespace ownership, duplicate identities and other object kinds remain open; README indexes current receipts.
- `apps.json` declares dependencies; the slice shares all include roots and does not prove them.
- Own Chart/header proof: `src/gen/{CodeunitWriter,PageWriter}.cpp` shares native alias dependency ownership; locals stay in sources. Door adds only missing owned directives. Six added native includes name their types; redundant directives fall 70,298→66,718, not zero. GenCodeunit retains all 37 checks; old owner control one red. `build/chart-20261001/artifacts/{declaration-proof,dependencies,native-headers,bodies,codeunit-owner-control}.json`; no PCH or full-app/G1 claim.
- Own `build/xmlport-names-20261001/`: one allocator/binding for tables and XMLports; tables bind once after extension merge. Clang/GCC generated AL: tables 14/14, XMLports 15/15 in each single/dependent-app context; seven controls green, archived emitter six independent assertions red. Real TestTableC 132512/139063 compile and execute 20/20 temporary-storage checks each; no SQL/FlowField execution claim.
- Full generation: 24,359 files, twelve added/nineteen changed/nine obsolete paths removed. Full repeat preserves every byte/mtime. Slice 14,213 → 14,219: six same-ID path renames, six appended entries; unrelated order preserved. Code Coverage XMLports 9991/130471 retain IDs/metadata but refuse their unbound System table (0034). Source UT 80/2,310 unchanged/unexecuted. Toolchain 119/119; same 92 local cases/26 connection failures; five-unit findings 67 → 66, Scan complexity 88 → 86. No integration/G1 or complete scoped binding claim. Receipts, exact mappings and patch in prototype artifacts; predecessor naming/guard images preserved.
- PermissionSetBuffer is not a cosmetic duplicate: Role ID is Code[30] versus Code[20]; App Name is a lookup FlowField versus Normal. System primary is Scope/App ID/Role ID; Base primary is Type/Role ID. Preserve each declaration and its bound methods/metadata, not the first or richer header.
- Own AllObj ABI probe: current headers with current libraries give NativeObject 193/193; complete old image gives 193 checks/13 red without a crash. Mixing old headers with new libraries crashed (139). `build/native-catalogue-20261001/source/build/native-catalogue-proof/{old-headers-gate-clang.log,old-image-gate-clang-final.log,old-image-gate-gcc-final.log}`. No compatible-hot-loading claim; rebuild/fingerprint every consumer before native-layout activation.
- `Parser.cpp::ReadSubtypeName` discards qualifiers: source `Record System.Reflection.AllObjWithCaption` becomes subtype `AllObjWithCaption`. Own `build/system-table-proof/QualifiedSubtypeProbe.cpp` reproduces the loss (exit 1). Qualified page SourceTable aliases in the own 0034 prototype do not repair variable/subtype resolution.

## Implementation

1. Namespace matching is shared; unify the remaining JSON readers, version the selected population and reject malformed configuration. Resolve the measured referenced-interface dependency closure without broad fallback conversions or silent scope changes. Identity must distinguish same-named Table/Codeunit and declaring app before C++ spelling.
   Extend `ObjectIdentifiers`/`TableRef::outputIdentifier` to the seven remaining guarded kinds from parsed declarations, not `DeclarationOf`'s line heuristic. Start at `Main::{IndexPages,IndexReports,IndexCodeunits}` and the corresponding writers. Preserve original AL identities and the ownership guard; no output-only suffix or mutable global map. Keep query-facade self types explicit. Activate the reviewed bindings only with frozen proof.
   Preserve full declared subtype qualification in `src/al/Parser.cpp::ReadSubtypeName`; resolve explicit namespace, current namespace and using scope against declaring-app identity. Gate same-named records/codeunits in two namespaces and nested/shadowed using scopes; no final-component-only fallback.
2. Validate the reaches/app dependency graphs, rejecting cycles and unknown edges. Configure dependencies in topological order and track reaches/apps.json as CMake inputs.
3. Preserve declaring app identity on merged fields, procedures and metadata. Complete each extension/customization kind and deterministic anchor ordering; unresolved anchors must be diagnostic failures or counted refusals.
4. Enforce namespace, Access/local, Extensible, obsolete declarations and preprocessor symbols at generation time. Refuse malformed or unsupported directives rather than silently selecting a branch.
5. Prove normal app linking with undefined-symbol checks appropriate to declared dependencies. Keep slice fallback stubs explicitly out of the full-app correctness claim.
   Finish declaration/body dependency ownership across writers: merge explicit AST and resolved body-header sets once; remove redundant legacy Door type-list ownership only with all-kind controls. `dependencies.json` names 6,718 files with remaining duplicate directives. Do not move local-only types into public headers or add a master/PCH dependency.
6. Generate sorted immutable catalogues per deployed app composition; reject duplicate installed kind/ID identities. Freeze registration before first session instead of mutating vectors after call_once sorting. Retain app/schema/composition hashes with each native image; table-layout changes require compatible rebuilt consumers, not unchecked hot-loading.
   Keep original AL namespace and app/package identities in the same declaration metadata consumed by object reflection (0034), page binding (0030) and typed lowering (0073). Runtime AllObjWithCaption cannot supply ALNamespace from today's caption-only provisioning. No second lookup map or C++-spelling reconstruction; namespace-less source is distinct from missing provenance.

## Acceptance

- Cross-app fixtures cover legal dependency, illegal reverse reference, late extension anchor, missing anchor and duplicate names. Compile without the all-app slice include path. Same inputs produce byte-identical merge order.
- Two distinct AL names that normalize alike retain both IDs, fields and references. Archived emitter is red; conflicting output ownership refuses, never last-writer-wins. Successful repeat keeps bytes and mtimes without declaration loss.

## References

Moved fields: developer `ff5939a46e`, `devenv-move-table-fields-between-extensions.md`
and `properties/devenv-{movedfrom,movedto,obsoletestate}-property.md`; BCApps
`bb7111877f`, Business Foundation `AuditCodes/src/Legacy/ObsoleteSourceCode{,Setup}Ext.TableExt.al`,
`NoSeries/src/Legacy/NoSeriesObsolete.TableExt.al`, W1
`Foundation/AuditCodes/SourceCodeSetupExt.TableExt.al` and original app manifests.
Predecessor board 913 identified duplicate moved-source merging, not a general
permission or No. Series bypass. Receipts:
`/tmp/agiru-ut-recovery-moved-{all-controls,old-control,transpile,slice}.log`.

Code: `src/al/Parser.cpp::ReadSubtypeName`, `src/gen/Apps.cpp`, `src/gen/Scope.cpp`, `src/gen/Names.cpp`, `src/tc/Main.cpp`.
Collision sources: current BCApps main `Layers/W1/Tests/{TestLibraries/TestTableC.Table.al,Monitor Sensitive Fields/TestTableC.table.al}`; output proof above, independent source UT 80/2,310 unchanged and unexecuted.
PermissionSetBuffer: same main, `System Application/App/Permission Sets/src/PermissionSetBuffer.Table.al` (9862) versus `Layers/W1/BaseApp/System/Permissions/PermissionSetBuffer.Table.al` (9009). Platform `devenv-namespaces-overview.md`, `properties/devenv-tabletype-property.md`; user `business-central/across-inspect-page.md`. Earlier 953: file-only suffixes leave registry/references wrong; 1318: silent replacement hides definitions; 1631: locals need their own scoped allocation, not object-name workarounds. Completed matcher/snapshot receipts remain in README, not this open item.

XMLport/table merge: platform `devenv-{xmlport-object,table-ext-object,namespaces-overview}.md`; BCApps main `src/{Layers/W1/BaseApp/Modules/System/DevTools,Tools/Test Framework/Test Runner/src}/CodeCoverage/CodeCoverageDetailed.XmlPort.al`, `src/Apps/W1/{AutomaticAccountCodes/app/src/Tables/AutoAccGLAccount,AMCBanking365Fundamentals/app/Tables/AMCBankBankAccountExt}.TableExt.al`; user `business-central/about-export-data.md`. Earlier 953 rejects file-only suffixes; XMLport custom `var` metadata stays in 0073.

Matcher contract: root `scope.json` plus predecessor `openerp/board/990_mem-scope-whitelist.md`; platform `devenv-namespaces-overview.md` establishes namespace hierarchy and distinct declaring identities, not agiru's selection policy. The stale generator-local policy is removed (0725). System.Integration.PowerBI/Graph remain excluded; legacy Microsoft.Integration.Graph includes local ERP/API helpers, so service retirement requires identity/reference classification rather than a whole-namespace exclusion.

Platform: devenv-json-files.md, namespace/access/extension and obsolete/preprocessor documentation. AL: apps.json inputs and extension declarations. Predecessor: WI-990 defines scope boundaries; do not widen them accidentally.

Property scope: `access`, `clearactions`, `extensible`, `movedfrom`, `movedto`, `obsoletereason`, `obsoletestate`, `obsoletetag`, `scope-table`.
