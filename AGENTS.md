# agiru

AL-to-C++23 transpiler and runtime for a standalone Business Central ERP backed by
PostgreSQL. The target is complete BC business functionality, including all AL object
kinds and extensions within the product scope below. Repository text, source,
documentation and commits are English.

## Product scope

- agiru-owned code is MIT licensed. All implemented ERP features are available without
  license keys, subscriptions, trials, paid tiers or license-based feature restrictions.
  Preserve third-party notices and licenses; they are not relicensed by this policy.
- BC licensing and entitlements as commercial feature gates, O365/Microsoft 365 and
  integrations with other Microsoft cloud services are not product requirements.
- User/company permissions, authentication, session isolation and generic protocols remain
  required. Do not remove core ERP merely because an AL namespace starts with Microsoft
  or a declaration has Scope=Cloud. Do not substitute always-success licensing stubs.
- Inventory source objects and tests before filtering. Report raw population, selected
  population and explicit excluded identities/reasons separately. Other unsupported
  behaviour remains a gap, not an approved exclusion (WI 0058).
- Root `scope.json` is the single selection policy for the transpiler and independent
  inventory. Product exclusions name bounded source paths and approved reasons;
  namespace/area reachability is not a complete product classification.

## Delivery order

1. First build the agent-only Node.js/TypeScript HTML-to-ASCII client
   with CMD/MCP adapters and the htmx web UI over one production page/command runtime.
   Prove CMD/MCP/web operation parity under `test/ui/`: messages, rows, exact typed values,
   dialogs, permissions and database effects. A fully green UT milestone is not a prerequisite.
2. Inventory and execute every product-scope business process from the local BC user
   documentation through the agent client; independently verify ledger, stock and SQL effects.
   Prioritize setup/master data, sales, purchases, finance, inventory/warehouse, then remaining
   ERP areas. Verify the actual web client through representative browser samples. Fix runtime
   defects that block clients or workflows; keep existing regressions and every known gap visible.
3. Compile the complete in-scope AL tree and run the source-counted UT and complete AL
   test suites; retain missing, refused and crashed identities. Prove multi-user behaviour and complete
   BC ERP functionality. Measure performance and resource use against equivalent BC workloads.
4. After G2, deliver a browser-only, single-user Emscripten/WASM demo of agiru with embedded
   PostgreSQL, served entirely as static assets from GitHub Pages. Reuse the production
   business runtime and prove representative workflow/SQL parity; keep its bounds separate
   from production multi-user and scale guarantees.

Historical pass counts are not current measurements. 0720 owns client-first delivery and
documented business workflows (G2); 0058 retains full UT acceptance (G1) without blocking
client implementation. 0721 owns production qualification (G3); 0724 depends on G2.
Do not trade the full target for a green subset.

## References before semantic changes

Use the local BC source and developer/user documentation repositories under `~/Git/`
before web search. Update their clean tracking branches with `git pull --ff-only` when
current upstream evidence is needed; preserve local edits and never reset or force a
merge. Record the revisions used, and keep frozen verification inputs unchanged.
Use web search only when the required reference is missing locally or local access fails.

Read the relevant overload, trigger or property in this order:

1. `~/Git/dynamics365smb-devitpro-pb/dev-itpro/developer/`: platform guarantees.
2. `~/Git/BCApps/`: AL declarations and usage; read its current `main` directly.
3. `~/Git/dynamics365smb-docs/`: user-facing intent.
4. `~/Git/openerp/board/`: earlier findings and rejected approaches. Search it before
   implementing an AL semantic. Its Python implementation is not the specification.

Method overloads have separate `methods-auto/<type>/*-method.md` files. Documentation
wins on guarantees; AL declarations establish actual names and signatures. Record relevant
paths and predecessor findings in the WI. Do not transplant Python threading machinery.

## Invariants

- Posting is atomic. Errors roll back their boundary; production `Commit()` must be
  durable and survive a later error. Test isolation is a separate, explicit policy.
- Amounts use `agiru::Decimal` with .NET decimal precision and scale, never binary floats.
- Fix generated code in `src/gen/` or its runtime primitive. Never hand-edit `apps/`.
- Runtime and generator contain no business-object-specific fixes. Platform objects are
  declared from system symbols, not guessed from individual test failures.
- Preserve AL names, types, argument modes, value-context semantics and trigger order.
  Unsupported behaviour must refuse explicitly; a successful no-op is not an implementation.
- Posting results are deterministic. Combine concurrent work in a declared order.
- Shared mutable authority belongs in PostgreSQL. Metadata may be shared read-only;
  session state must not leak through process globals or thread reuse.

## Architecture and scale

Dependency tiers are declared by `src/*/reaches`: `al`, `net` and `db` are foundations;
`gen` reaches `al`; `rt` reaches `net` and `db`; `cli` reaches `rt`. Generated apps see
only the public headers and declared app dependencies. One BC app becomes one library.

Production targets Linux on x86_64/aarch64, primarily deployed with Podman, 2 TB and
10,000 users. The browser/WASM demo is a separate project target, not a production platform. Stream reads in
bounded blocks, index declared keys according to their properties, and borrow connections
for transactions. Measure per-session memory and operation overhead against equivalent SQL.
These are requirements, not claims that the current runtime meets them.

For development, run the agiru server, served web assets and PostgreSQL together in
one Podman container. The agent CLI runs outside it over the same HTTP endpoints
as the browser. Keep database data persistent, use disposable test clones and preserve
existing containers/data during migration. This development packaging does not mandate
a single-container production topology or put Node.js in the ERP server.

Use unmodified Caddy from official Debian repositories at the public edge and system libmicrohttpd for agiru's private
native HTTP listener. Caddy owns ACME/TLS/static delivery; C++ owns authentication and ERP
execution. Keep blocking work off network event loops, bound admission and disable
automatic upstream write retries. Persist certificate state; keep the admin API disabled.
Replace untrusted forwarding headers at the trusted edge; neither proxy headers nor
deep links grant permissions. No custom HTTP/TLS parser or
ERP embedding in nginx/Caddy modules. WASM uses a separate platform transport adapter.

Linux/container multi-user performance and bounded resources drive architecture decisions.
The single-user WASM demo must retain functional behaviour, but does not impose production
throughput or scale guarantees. Share business/layout semantics; platform adapters may differ
for threading, storage, files and rendering integration. Do not route the native production
runtime through a WASM VM or impose browser deployment constraints on Linux scalability.

Emit immutable declarations as `constexpr` metadata; use `static_assert` for compile-time
facts. Generated headers contain declarations and sources contain bodies. Include only
what a file names; no master header or macros. Measure build cost before widening headers.
`cmake/Precompiled.h` is a build optimization, not an implicit source dependency.
Registry-only report consumers use `runtime/ReportRegistry.h`; dataset/request-page
execution stays in `runtime/Report.h`. Keep one registry, preserve its entry ABI and
prove the narrow header's dependency profile with negative controls.

## Client contract

- C++ owns ERP execution, authorization and sessions. Node.js/TypeScript is a thin
  agent-client dependency, not a production ERP-server dependency (0720).
- One generated typed page/action model supplies semantic HTML and exact machine values.
  Web uses htmx; agents render its declared HTML profile as compact ASCII with unchanged
  Unicode data. No separate CLI masks, business rules or full browser/htmx implementation.
- CMD and MCP share one client library and operation contract. Discover valid actions;
  use explicit session/page/dialog handles, revisions and command IDs across calls.
- Output is bounded, deterministic and noninteractive: text/JSON for CMD, structured
  results plus compact text for MCP. No TTY/ANSI requirement or automatic confirmations.
  Preserve Decimal/Int64 exactly, treat content as untrusted, and reconcile uncertain writes
  before retries. HTTP wiring and independent SQL effects are part of parity proof.

## Layouts, charts and analysis

- Translate each versioned DOCX/RDLC asset once into an agiru-owned typed layout and
  documented HTML/CSS print profile. Bind the XML report dataset at execution time.
  Preserve expressions, grouping, nested data, fonts and pagination; unsupported features
  refuse explicitly and remain counted ERP gaps, never silently simplified layouts.
- Implement layout/pagination in C++; use Cairo as the PDF backend, not as the layout
  engine. Share positioned vector/glyph output with SVG previews and static business
  charts. Use proper text shaping and packaged fonts, not Cairo's toy text API.
  Genuine Excel workbook layouts remain a separate format, not PDF or a dataset dump.
- Reuse upstream Web Platform Tests (WPT) CSS reftests and wptrunner through an engine
  adapter. Pin the upstream revision; retain original match/mismatch and fuzzy metadata.
  Render each test/reference pair with the same engine; browser-to-agiru comparisons
  are separate differential checks, not the WPT reftest oracle. Pin fonts and viewport.
  Declare the tested profile and count failures, crashes, unsupported and unexecuted cases.
  WPT conformance does not replace BC layout/dataset/chart and paginated PDF comparisons.
- Business Charts and interactive ledger-page analysis are required. Share exact typed
  measures, filters, grouping/pivots and drilldown semantics across CLI, web and exports;
  interactive UI is not a PDF renderer. Reuse query/filter/aggregate primitives, keep
  permissions/company context on the server and persist private analysis definitions in
  PostgreSQL. Bound scans, pivot cardinality, output blocks and per-session memory.
- Native Linux execution is primary. Prove the same layout/chart code and Cairo PDF
  backend in Emscripten with packaged resources; WASM support is not yet verified.
  Preserve dependency licenses/notices. Client/analysis UI does not depend on G1 acceptance.

## Implementation rules

- C++23, `-Wall -Wextra -Wpedantic -Werror`. Clang 19 is the production compiler;
  use libc++/libc++abi, compiler-rt, LLVM libunwind and LLD. No second compiler gate is
  required. Architecture-specific operations require a portable path.
- Prefer typed IDs, `span`, `string_view`, and explicit ownership. Private state is the
  default. Use exceptions for AL errors and `expected` when refusal is a returned value.
- Public names in `include/` carry Doxygen contracts. `src/` carries no prose comments;
  `make comments` removes them. Explain behaviour in gates, decisions in WIs.
- Diagnostics are part of AL behaviour. Do not swallow errors or add uncounted suppressions.
  Constants need a documented origin where their meaning is not self-evident.
- Dependencies are allowed when the standard library is insufficient; justify their purpose
  and portability. XML uses libxml2; JSON uses system yyjson behind a private adapter,
  preserving exact number tokens and stable node ownership. Byte hashing uses system OpenSSL
  Crypto behind a private adapter; no OpenSSL headers in generated/public interfaces.
  Its WASM backend remains unqualified. Reporting must preserve BC layout semantics and produce
  genuine PDF/workbook output through the layout/Cairo architecture above. Verify native
  Linux performance/resources and browser-only WASM compatibility; no Java/desktop-Office engine.
- Incremental compiler outputs may use `build/`. Temporary fixtures, probes, source
  copies and verification snapshots must use `/tmp`, never accumulate in the repository.
  Inspect the current mount and available space first; `/tmp` may be a bounded tmpfs.
  Remove disposable inputs/binaries after use; retain only current verification receipts.
  Golden files in `test/transpiler/golden/` are specifications: edit them deliberately,
  never regenerate them from observed output.

## Build and verification

Use Make for builds/tests and existing repository scripts; `make help` lists specialist
targets. One editable tree: bundle coherent fixes → build/test → inspect results → repeat.
No mandatory snapshots or development worktrees. Keep compiler inputs unchanged during a
direct build; freeze only when continued editing or reproducible isolation requires it.

1. For generator changes: `make tc JOBS=2`, then `make transpile
   AGIRU_SYSTEM_SYMBOLS=<verified-package>`. Never edit generated `apps/`.
2. Run affected C++ gates: `make gate GATE=RecordRefGate JOBS=2`.
3. Integrate: `make slice-check`, `make all JOBS=6`, `make test JOBS=2`,
   `make lint JOBS=2`, then `make ut JOBS=6`. Inspect every exit status and UT result;
   fix failures and repeat. Local gates do not replace the source-counted AL UT.
4. For focused analysis use `make lint-one UNIT=src/rt/Transaction.cpp JOBS=2`;
   `FULL=1 make lint JOBS=2` covers the complete handwritten surface.
5. Use `make apps JOBS=6` for complete generated-app compilation/linking, independently
   of the diagnostic slice. `make tree` diagnoses coverage; neither replaces AL execution.

- Optional frozen run: `make verify-start JOBS=6
  VERIFY_TARGETS='slice-check all test ut'`; inspect `make verify-status` and its artifacts.
  Relocated BC sources need `AGIRU_LAYOUT_SOURCE_NOTICE=<original-notice>` for `test`;
  missing notices refuse before dependency copying, not after the build.
  Snapshots/lane sources under `/tmp` are immutable inputs, never development trees.
  Receipts identify Git HEAD, content/dependency hashes, target exits and test population.
- At most one six-job integration lane. Local gates use two jobs; never overlap mutating
  tests on the same gate database. Serialize lint with build-database reconfiguration.
  Before a requested clean rebuild, finish/stop verification and preserve any unmerged
  source outside `build/`; clearing artifacts is not a test pass.
- Keep tests grouped under `test/{gate,runtime,transpiler,reporting,tooling}`;
  client parity belongs in `test/ui/`. Boundaries: `test/README.md`.
  Bash orchestrates tests; Python tooling and AL transpiler fixtures are allowed.
  Fixture consumers need successful compile-command receipts before lint; missing
  commands refuse. Deliberately invalid compile inputs use `.cpp.in`.
- `make census` counts raw AL independently; `scope.json` owns approved exclusions.
  Retain missing/refused/crashed/skipped/unexecuted identities. Compare the same full UT
  population after semantic activation and investigate losses. Never raise a defect or
  suppression baseline to obtain green; prove new gates with meaningful negative controls.
- Append to `test/slice` without sorting/deleting identities. Retain stable bounded unity
  groups. Slice compilation is not full-app or G1 proof.
- Include only named dependencies; measure header/consumer cost with `make include-cost`.
  Keep Clang PCH/ccache settings and timestamp-free PCH builds; avoid compile-time date/time
  macros. Measure per-run hits/timings before claiming cache or performance improvements.
- Native package/layout qualifiers use original verified sources, owners and notices.
  Keep raw inventories, provenance hashes and explicit refusals; declaration/fixture
  success is not a live provider, layout installation/rendering or ERP milestone.
  Specialist acceptance belongs in WIs 0058/0721, not this workflow.
- Preserve pipeline exit statuses; patches must fail on missing anchors. Use
  `build/times.log` and current receipts, not historical timing/pass claims.

## Database environment

Inspect the local environment instead of assuming resources or container state. The usual
containers are `agiru-pg` and `agiru-mssql`; `podman start agiru-pg` starts the existing database.
Use the dedicated gate database for C++ tests and disposable clones for AL runs. Never run
mutating tests against the demo source or a master template.

`BC_VERSION` pins the demo artefact, not the BCApps source revision. Record both revisions
and report schema/data mismatches. The imported CRONUS database uses company/system schemas;
the current runner also has a flattened seeded database. Do not confuse those layouts.
A seed used as A/B proof needs a `complete` provenance row and a sealed template; a legacy
`null` identity or database size is only a hint.

## Work items

`board/` contains only WIs, no README, index or activity diary. Keep one WI in progress
and concrete outcome-sized WIs; maintain separate queued business-process families
for BC sandbox execution and agiru replication, not one WI per field or test step.
Each WI owns its priority, dependencies, next action, evidence, source files and acceptance.
Keep later work queued behind explicit prerequisite contracts, without dependency cycles.
Use durable source/test paths, pinned revisions and reproducible commands; never make a
WI depend on disposable `/tmp` receipts. Search the refreshed `~/Git/openerp/` code and
board for reusable implementation details and rejected approaches. Prefer short lists;
remove repeated history and completed steps. Preserve implemented code and regression tests.

Keep existing IDs when consolidating. Allocate new IDs from all Git history, never from
open files alone. Delete completed or superseded items; Git preserves their history. Record
absorbed IDs and a recovery revision in the receiving WI. Do not claim stale ownership.
Keep only current build recipes in the working tree; Git preserves superseded instructions.

## Commits and pushes

- Commit and push after every verified coherent increment (code, tests or documentation),
  before starting the next increment. No separate request is required.
- Split commits by outcome; leave unrelated unfinished changes out.
- Report failed pushes, pending checks and known failures explicitly; never force-push.
  A pushed commit is not an ERP milestone.
