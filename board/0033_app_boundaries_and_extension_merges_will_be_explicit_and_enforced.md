# 0033 — App boundaries and extension merges will be explicit and enforced

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

Extensions are merged in src/tc/Main.cpp and apps.json describes dependencies. The slice target includes all app directories and links one library, so it does not prove full-app dependency direction. CMake uses private includes but does not by itself prevent public headers from exposing higher-tier types or shared libraries retaining unresolved references.

## Implementation for Sol

1. Validate the reaches/app dependency graphs, rejecting cycles and unknown edges. Configure dependencies in topological order and track reaches/apps.json as CMake inputs.
2. Preserve declaring app identity on merged fields, procedures and metadata. Complete each extension/customization kind and deterministic anchor ordering; unresolved anchors must be diagnostic failures or counted refusals.
3. Enforce namespace, Access/local, Extensible, obsolete declarations and preprocessor symbols at generation time. Refuse malformed or unsupported directives rather than silently selecting a branch.
4. Prove normal app linking with undefined-symbol checks appropriate to declared dependencies. Keep slice fallback stubs explicitly out of the full-app correctness claim.

## Acceptance

Cross-app fixtures cover legal dependency, illegal reverse reference, late extension anchor, missing anchor and duplicate names. Compile without the all-app slice include path. Same inputs produce byte-identical merge order.

## References

Platform: devenv-json-files.md, namespace/access/extension and obsolete/preprocessor documentation. AL: apps.json inputs and extension declarations. Predecessor: WI-990 defines scope boundaries; do not widen them accidentally.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `access`, `clearactions`, `extensible`, `movedfrom`, `movedto`, `obsoletereason`, `obsoletestate`, `obsoletetag`, `scope-table`.
