# 0724 — Deliver the browser-only single-user demo

Status: queued | Priority: P3
Depends on: [0720](0720_cli_and_web_will_execute_the_same_erp_operations.md)
acceptance (G2). Report/chart samples additionally need 0721's shared rendering
contract, not completion of its production scale qualification.
Next after G2: prove one native/browser command and SQL transaction on the same seed.

## Implementation

1. Compile the same generated C++ business runtime with Emscripten in a Worker.
   Dispatch the production typed commands through a browser adapter and reuse
   the web presentation; no business fork, native-listener emulator or native
   production runtime inside a WASM VM.
2. Bridge asynchronously to embedded PostgreSQL/PGlite with exact Decimal/Int64,
   ordered SQL results, diagnostics, rollback and durable local Commit boundaries.
   One private user/session; inventory/refuse unavailable native/extensions/dependencies.
   Do not assume browser libpq/raw sockets or pthread header support on Pages.
3. Package a bounded redistributable seed and fonts/layout/assets with pinned
   compiler/engine/runtime/seed identities and preserved notices. No production
   data/secrets. Implement persistence/export and explicitly confirmed reset;
   measure startup/download/memory/storage limits.
4. Serve exclusively static GitHub Pages assets under the real repository base
   path. Check Worker/WASM MIME/CSP/header/loading and offline-after-load bounds.
5. Prove shared C++ layout/chart and Cairo PDF browser builds with packaged shaping
   resources. Reuse 0721's semantics; never constrain Linux multi-user throughput
   to the demo's one-connection/single-user architecture.

## Acceptance

- Fresh browser works without external agiru/PostgreSQL; reload retains declared
  data, confirmed reset restores the seed, quota/bridge errors are explicit.
- Representative sales/purchase/journal/inventory workflows match fresh Linux/CLI
  seeds in messages, exact values, rows and independent SQL effects. Cover validation
  rollback, Commit followed by error, duplicate posting and disconnect/recovery.
- Sample real PDF downloads/SVG interaction, pagination, chart measures, ledger
  group/pivot/drilldown totals against Linux. Missing capabilities remain counted.
- Demo coverage stays separate from production durability, isolation, 2 TB /
  10,000-user and performance claims. No demo implementation is currently proved.

Files: `src/{db,rt,cli}/`, `CMakeLists.txt`, `test/ui/`; existing PGlite/browser
deployment references and previous detail: Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
