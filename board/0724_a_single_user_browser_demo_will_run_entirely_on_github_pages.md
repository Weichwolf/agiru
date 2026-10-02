# 0724 — A single-user browser demo will run entirely on GitHub Pages

Status: open | Priority: P3 | Stage: browser demo after G2 | Reviewed: 2026-10-02
Depends on: G1 all UT green; G2 production clients/parity; 0006 session ownership; 0012 transaction boundaries; 0722 exact typed JSON.

## Evidence

- Requested project goal: agiru business runtime and PostgreSQL in the browser, one user, entirely static GitHub Pages hosting. No demo implementation exists. Production remains Linux/Podman x86_64/aarch64.
- Architecture priority: native Linux/container multi-user throughput and bounded resources. Demo retains functionality, not production scale guarantees; platform adapters may differ without forking business/layout semantics. Rendering must build for the browser, but production must not execute through a WASM VM or serialize work to satisfy demo restrictions (0063).
- [PGlite](https://pglite.dev/docs/about) is PostgreSQL compiled to WASM, exposed through JS/TypeScript; [browser persistence](https://pglite.dev/docs/) uses IndexedDB. It has one exclusive database connection. Its [socket server](https://pglite.dev/docs/pglite-socket) is a Node TCP facility, not browser-native libpq compatibility.
- [GitHub Pages](https://docs.github.com/en/pages/getting-started-with-github-pages/what-is-github-pages) serves static assets, not native server processes. [Emscripten pthreads](https://emscripten.org/docs/porting/pthreads.html) require COOP/COEP; deployment support must be measured, not assumed.

## Implementation

1. Compile the same generated business runtime with Emscripten, initially single-threaded in a Worker. Dispatch production typed commands through an in-browser adapter; reuse the web presentation. No business-rule fork or native HTTP-listener emulation.
2. Implement a narrow async PGlite database bridge. Keep SQL ownership in db; preserve exact Decimal/Int64 text, ordered rows, database diagnostics, transaction/Commit/error semantics and one private session. Explicitly refuse unsupported extensions/native facilities; inventory XML/report/library dependencies before promising full demo coverage.
3. Package a bounded, redistributable demo seed and versioned assets; pin compiler/PGlite/seed/runtime identities. No secrets or production data. Provide persistence, export/reset and an explicit destructive-reset confirmation. Measure download/startup, memory and browser storage limits.
4. Deploy only static assets under the actual Pages repository base path. Verify Worker/WASM MIME, loading, offline-after-load scope and any CSP/header requirements. Do not assume raw sockets, server hosting or pthreads.
5. Build the shared C++ layout/chart engine and Cairo PDF backend with packaged fonts/resources. Replay representative XML-dataset/layout/PDF and interactive ledger-analysis/chart commands against Linux. Keep native throughput/concurrency decisions intact; browser/WASM/backend support must be proved, not inferred from C/C++ source portability.

## Acceptance

- Fresh browser reaches a usable demo with no external agiru/PostgreSQL server. Reload preserves declared local data; confirmed reset restores the seed. Storage quota/errors are visible, not silent success.
- Replay representative sales/purchase, journal and inventory workflows against identical fresh Linux/CLI and browser demo seeds. Compare messages, typed values, ordered rows and independent SQL effects; test validation rollback and Commit followed by error. Missing demo capabilities stay counted.
- Disconnect/malformed bridge response, Decimal scale loss, duplicate posting and reset failure are detected. Browser-specific presentation remains sampled; shared business checks reuse the CLI suite.
- Report demo coverage and limitations separately. Single-user local storage cannot qualify production durability, multi-user isolation, 2 TB or 10,000 users.
- Match representative paginated reports, exact chart measures, drilldown and ledger group/pivot totals with Linux; verify actual browser PDF download/SVG interaction. Missing shaping/fonts, truncated totals or renderer failures remain visible demo gaps.

## References

Code: `src/db/`, `src/rt/`, `src/cli/Main.cpp`, `CMakeLists.txt`, `test/ui/` once G2 creates it. Shared client contract: 0720. ID: all-history maximum 0719 and current open maximum 0723 checked on 2026-10-01.
