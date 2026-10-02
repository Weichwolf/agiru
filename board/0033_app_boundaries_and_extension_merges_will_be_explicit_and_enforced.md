# 0033 — App boundaries and extension merges will be explicit and enforced

Status: open | Priority: P1 | Stage: UT app isolation | Reviewed: 2026-09-28
Depends on: 0034 source identities.

## Evidence

- `apps.json` declares dependencies; the slice shares all include roots and does not prove them.
- Integrated `NamespaceInScope` centralizes the longest case-insensitive, dot-boundary match for Holds and Contains; exclusion wins equal-length ties. GenScopeGate: 229 checks green under Clang 19/GCC 14; frozen pre-fix generator: 3 red. Complete local test: 80 cases, 0 red; tc succeeds. Both existing scope configurations contain zero equal include/exclude prefixes, so this tie repair changes no currently selected namespace; no scope entry was removed or widened.
- Matching no longer constructs lowercase namespace/prefix strings. AllocationProbe: 2,000 identical admitted checks, 0 new allocations versus 50,000 on the frozen pre-fix generator. This is a matcher measurement, not ERP-scale proof. Scope.cpp, Apps.cpp and GenScopeGate targeted analysis all pass. Sources/logs: `build/development-scope/`.
- Frozen snapshot 20260928T155053Z-49374 completed: 9,866/9,872 headers pass; six fail (one subsequently repaired page/procedure collision plus five absent-interface headers). Tree incorrectly exits 0; repaired separately in 0589. Full-app build stops on missing NavTenantSettingsHelper.TryGetStringTenantSetting; additional app failures remain unmeasured. Source-counted UT milestone: 2,199/2,310, 111 failed, 0 incomplete; not the complete AL test population. Latest scope/navigation/scratch/name-allocation/session patches are not in that snapshot.

## Implementation

1. Namespace matching is shared; unify the remaining JSON readers, version the selected population and reject malformed configuration. Resolve the measured referenced-interface dependency closure without broad fallback conversions or silent scope changes. Identity must distinguish same-named Table/Codeunit and declaring app before C++ spelling.
2. Validate the reaches/app dependency graphs, rejecting cycles and unknown edges. Configure dependencies in topological order and track reaches/apps.json as CMake inputs.
3. Preserve declaring app identity on merged fields, procedures and metadata. Complete each extension/customization kind and deterministic anchor ordering; unresolved anchors must be diagnostic failures or counted refusals.
4. Enforce namespace, Access/local, Extensible, obsolete declarations and preprocessor symbols at generation time. Refuse malformed or unsupported directives rather than silently selecting a branch.
5. Prove normal app linking with undefined-symbol checks appropriate to declared dependencies. Keep slice fallback stubs explicitly out of the full-app correctness claim.
6. Generate sorted immutable catalogues per deployed app composition; reject duplicate installed kind/ID identities. Freeze registration before first session instead of mutating vectors after call_once sorting. Retain app/schema/composition hashes with each native image; table-layout changes require compatible rebuilt consumers, not unchecked hot-loading.

## Acceptance

- Cross-app fixtures cover legal dependency, illegal reverse reference, late extension anchor, missing anchor and duplicate names. Compile without the all-app slice include path. Same inputs produce byte-identical merge order.

## References

Code: `src/gen/Apps.cpp`, `src/gen/Scope.cpp`, `src/gen/Names.cpp`, `src/tc/Main.cpp`.

Matcher contract: existing `scope.json`/`src/gen/scope.json` plus predecessor `openerp/board/990_mem-scope-whitelist.md`; platform `devenv-namespaces-overview.md` establishes namespace hierarchy and distinct declaring identities, not agiru's selection policy. BCApps current main's System.Integration.PowerBI and System.Integration.Graph declarations remain explicit excluded-type dependency gaps; selection was not widened in this patch.

Platform: devenv-json-files.md, namespace/access/extension and obsolete/preprocessor documentation. AL: apps.json inputs and extension declarations. Predecessor: WI-990 defines scope boundaries; do not widen them accidentally.

Property scope: `access`, `clearactions`, `extensible`, `movedfrom`, `movedto`, `obsoletereason`, `obsoletestate`, `obsoletetag`, `scope-table`.
