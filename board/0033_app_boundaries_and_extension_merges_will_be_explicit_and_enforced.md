# 0033 — App boundaries and extension merges will be explicit and enforced

Status: open | Priority: P1 | Stage: UT app isolation | Reviewed: 2026-09-28
Depends on: 0034 source identities.

## Evidence

- `apps.json` declares dependencies; the slice shares all include roots and does not prove them.
- `Apps.cpp::Holds` includes an include/exclude tie; `Scope.cpp::Contains` excludes it. `scope.json` declares exclusion wins.
- Both matchers reproduced against one tie fixture (`build/review-20260928/scope-probe.log`); consolidate them before widening scope.

## Implementation

1. Unify scope parsing/matching and version the selected population; reject malformed configuration. Identity must distinguish same-named Table/Codeunit and declaring app before C++ spelling.
2. Validate the reaches/app dependency graphs, rejecting cycles and unknown edges. Configure dependencies in topological order and track reaches/apps.json as CMake inputs.
3. Preserve declaring app identity on merged fields, procedures and metadata. Complete each extension/customization kind and deterministic anchor ordering; unresolved anchors must be diagnostic failures or counted refusals.
4. Enforce namespace, Access/local, Extensible, obsolete declarations and preprocessor symbols at generation time. Refuse malformed or unsupported directives rather than silently selecting a branch.
5. Prove normal app linking with undefined-symbol checks appropriate to declared dependencies. Keep slice fallback stubs explicitly out of the full-app correctness claim.
6. Generate sorted immutable catalogues per deployed app composition; reject duplicate installed kind/ID identities. Freeze registration before first session instead of mutating vectors after call_once sorting. Retain app/schema/composition hashes with each native image; table-layout changes require compatible rebuilt consumers, not unchecked hot-loading.

## Acceptance

- Cross-app fixtures cover legal dependency, illegal reverse reference, late extension anchor, missing anchor and duplicate names. Compile without the all-app slice include path. Same inputs produce byte-identical merge order.

## References

Code: `src/gen/Apps.cpp`, `src/gen/Scope.cpp`, `src/gen/Names.cpp`, `src/tc/Main.cpp`.

Platform: devenv-json-files.md, namespace/access/extension and obsolete/preprocessor documentation. AL: apps.json inputs and extension declarations. Predecessor: WI-990 defines scope boundaries; do not widen them accidentally.

Property scope: `access`, `clearactions`, `extensible`, `movedfrom`, `movedto`, `obsoletereason`, `obsoletestate`, `obsoletetag`, `scope-table`.
