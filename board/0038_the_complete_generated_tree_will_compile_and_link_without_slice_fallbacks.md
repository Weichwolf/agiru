# 0038 — The complete generated tree will compile and link without slice fallbacks

Status: open | Priority: P1 | Stage: UT complete in-scope tree | Reviewed: 2026-10-01
Depends on: 0033 app graph; 0034 declaration census; 0073 expression lowering.

## Evidence

- `test/slice` and `scripts/unlinked.py` permit a linked subset with counted runtime refusals. A green slice is not a complete app build.
- Complete pre-repair key-image header census: 9,868/9,873; one PowerBIServiceProvider and four GraphAuthorization failures. All inputs tallied and unchanged-input guards passed. `build/key-contract-20261001/artifacts/{header-census,generated-gap.log}`; later native headers need their own census.
- Main first-gap remains 1 passed + 1 failed + 14,462 unmeasured = 14,464 bodies (AddHeader ambiguity). Own Text prototype: 41 + 1 + 14,423 = 14,465; all file paths retained. InviteExternalAccountant compiles without PCH under both compilers. Next `APIWebhookNotificationSend.cpp:1006` fails on absent System table fields; 0034 owns declarations, 0044 providers. Inventories reconcile with every generated cpp, including support directories. `build/text-result-20261001/artifacts/{body-gap-current.log,body-files-current,invite-regenerated-clang.log,invite-regenerated-gcc.log,webhook-gap-clang.log,webhook-gap-gcc.log}`; main receipts remain in `build/body-gate-20261001/artifacts/`. Prefix only; complete bodies/link closure unmeasured.
- LibraryXMLRead, AgedAccPayableChart, NoSeriesCopilotImpl and LibraryGraphMgt now compile through the slice; remaining runtime refusals still count.
- `Base64ConvertImpl.h` holds absent platform codeunit `Base64Convert`, distinct from generated AL `Base64 Convert`. EntityText table and WebService are also absent AL objects, despite the shared '.NET member' diagnostic.
- Frozen run `20260930T114935Z-65182`: tree 9,867/9,872; five headers fail on absent PowerBIServiceProvider/GraphAuthorization. Apps stop at 62/14,719 scheduled build edges on `AzureADTenantImpl.cpp` calling missing `NavTenantSettingsHelper.TryGetStringTenantSetting`. Both targets are nonzero; remaining body/link failures unmeasured. Complete diagnostics: snapshot `verify.log`, lane `build/tree-syntax/{failed,roots}`.

## Implementation

1. Run `make tree` and `make gap SOURCE=1 SWEEP=1`; persist both inventories and diagnostics by app/source identity. Activate 0073's Text repair only with its UT proof; close the next System-table root in 0034, then repeat the complete body inventory. Verify standalone Clang without PCH; append newly compiling sources to the slice without reordering it.
2. Resolve every referenced AL symbol by app + namespace + kind + ID. Separate excluded AL, unresolved platform symbols, missing linked bodies and genuine .NET members; keep actual AL source declarations authoritative.
3. Compile/link one library per app with only public headers and declared dependency roots. Full-app targets reject unresolved symbols and every fallback stub.
4. Bind generated outputs to BCApps content, scope/apps manifests and generator hash; reject stale output even when method names and counts match. Do not declare completion from the slice census.

## Acceptance

- `make apps` completes under Clang 19 on Linux x86_64/aarch64 for the complete in-scope tree; all source/parse/emit/compile/link populations reconcile.
- Removing a source or swapping same-named stale output fails the gate. Runtime refusals remain failures until their real contracts execute.

## References

Code: `CMakeLists.txt`, `src/tc/Main.cpp`, `src/gen/{Names,Scope,BodyWriter,PageWriter}.cpp`, `scripts/{tree_syntax.sh,first_gap.sh,unlinked.py}`, `test/slice`. AL: System Application Base64 Convert/Entity Text and system-symbol Web Service declarations. Predecessor: WI-990 scope boundary; retain exclusions as visible ERP gaps.
