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
  behaviour remains a gap, not an approved exclusion (board 0725).
- Root `scope.json` is the single selection policy for the transpiler and independent
  inventory. Product exclusions name bounded source paths and approved reasons;
  namespace/area reachability is not a complete product classification.

## Delivery order

1. Compile the complete in-scope AL tree and make the UT milestone pass through
   `agiru run-tests`. Count the denominator from AL source text, independently of parsing
   and linking. Report missing, refused and crashed cases; they never disappear from totals.
2. After every UT is green, build the ERP CLI and real htmx HTTP UI over one production
   page/command runtime and generated metadata. Prove operation parity under `test/ui/`:
   messages, rows, typed values, permissions and database effects. Use the CLI for exhaustive
   business workflows; verify the actual web client through representative browser samples.
3. Run the complete AL test suite and CLI workflows; prove multi-user behaviour and complete
   BC ERP functionality. Measure performance and resource use against equivalent BC workloads.
4. After G2, deliver a browser-only, single-user Emscripten/WASM demo of agiru with embedded
   PostgreSQL, served entirely as static assets from GitHub Pages. Reuse the production
   business runtime and prove representative workflow/SQL parity; keep its bounds separate
   from production multi-user and scale guarantees.

Historical pass counts are not current measurements. `board/README.md` gives the reviewed
state and execution order. Do not trade the full target for a green subset.

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

Emit immutable declarations as `constexpr` metadata; use `static_assert` for compile-time
facts. Generated headers contain declarations and sources contain bodies. Include only
what a file names; no master header or macros. Measure build cost before widening headers.
`cmake/Precompiled.h` is a build optimization, not an implicit source dependency.

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
  and portability. XML uses libxml2. The reporting target is XSL-FO/Apache FOP to PDF.
- Artefacts go to `build/` or a temporary directory. Golden files in `test/target/` are
  specifications: edit them deliberately, never regenerate them from observed output.

## Build and verification

`make` is the entry point. Use `make help` for the current target list.

- `make census` inventories the raw BCApps AL tree independently of the transpiler's
  parser/linker, without filtering by namespace/app membership. `build/scope-inventory.json` retains source hashes,
  object/test identities, conditional variants, omitted app roots and unmeasured files.
  Namespace selection is diagnostic only; this is not an executable test manifest.
  An incomplete census fails while preserving the report. Use 0725 for product exclusions.
- Hot loop in the development tree: `make gate GATE=RecordRefGate JOBS=2`
  builds and runs one affected C++ gate. Use `B=build/sanitizers` for an already
  configured sanitizer build. Run `make test JOBS=2` for the complete local gates,
  `make tc JOBS=2` for generator edits, then `make gap` for one generated root.
  `make lint-one UNIT=src/rt/Transaction.cpp` analyses one configured unit without compiling objects and writes
  `build/lint/targeted.log`; `make lint` checks all changed handwritten code before
  integration. These targets do not rebuild the slice. Generated execution fixtures
  record their actual successful compiler commands under the configured build's
  `fixture-commands/`; run `make test` before analysing those consumers. Analysis
  includes every handwritten test `.cpp` outside `test/target/`; a missing command
  remains a refusal. Intentionally invalid compile fixtures use `.cpp.in` templates.
- Integration: `make verify-start JOBS=6` freezes the current tracked, untracked and
  generated inputs into `build/verify/<id>/source`, then runs `all test` in a
  serialized reusable lane under `build/verify/lane/source`. Only changed source
  files replace lane inputs, so unchanged build objects retain their timestamps.
  `make verify-status` reports the latest result; `verify.log` holds diagnostics. Raw census reports, AL manifests, provenance and method results stay in
  the snapshot's `artifacts/` directory, independently of lane reuse.
  Use `VERIFY_TARGETS='all test ut'` when the AL milestone and its database are ready.
  The snapshot and lane content hashes must match; the snapshot records Git HEAD.
  Edits in the active tree may
  continue after snapshot creation. A changed source during copying refuses the run.
- `make verify` runs the same frozen verification in the foreground. Bare `make`
  still builds the full slice in the current tree; use it only for a deliberate
  integration check when that tree will remain unchanged until the build ends.
- `make transpile` regenerates `apps/`; `make tree` and `make apps` check the
  complete generated tree. `FULL=1 make lint` checks the whole handwritten surface.

Append new entries to `test/slice` without sorting existing entries. Unity groups are
content-addressed into 896 stable roots capped at 32 sources; only an overfull root
splits. An insertion therefore changes its own Unity file without renumbering unrelated
objects. Keep the slice monotonic so its history remains reviewable (WI 0589).
Make exports the installed ccache PCH settings; Clang slice/app builds disable PCH
timestamps. Keep compile-time date/time macros out of cached sources, and measure
per-run cache hits before attributing a build-time change to the cache.

Use one editable development tree. Bundle coherent fixes and features, freeze the
batch, compile/test it, inspect the results and repeat. Separate development worktrees
are optional and require a concrete isolation need; keep them outside `build/`.
Source copies under `build/` are frozen verification inputs, never development trees.
Before a requested clean rebuild, stop or finish verification, preserve any unmerged
source outside `build/`, then clear build artefacts and run the build and UT through
Make. Record build refusals and every unexecuted UT; a clean directory is not a pass.
Build/test results identify the frozen Git HEAD and content hashes; development may
continue in the editable tree after freezing. On this six-core host, run at
most one six-job integration build at a time; use two jobs for local gates while
one is active. Do not edit compiler inputs of a live build's source snapshot.
Inspect `build/times.log` and snapshot results rather than historical timings.
Preserve exit statuses through pipelines. Keep patches anchored and fail on a missing match.

No test denominator may shrink unnoticed. Defect/suppression baselines may only decrease;
coverage and the compiling slice may only grow. Never raise a baseline to make a gate pass.
Prove new gates with a meaningful negative control. For changes that activate previously
unreachable AL, compare the same full UT population before and after and investigate losses.

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

`board/` holds open work only. Keep one concise WI per coherent outcome: evidence, priority,
dependencies, source references, implementation steps and acceptance tests. Prefer short lists;
remove narratives, repeated history and completed steps. Sol must be able to start from the
named files without reconstructing a session transcript.

Keep existing IDs when consolidating. Allocate new IDs from all Git history, never from
open files alone. Delete completed or superseded items; Git preserves their history. Record
consolidation mappings in the board index. Do not claim ownership through stale `active` text.
Keep only current build recipes in the working tree; Git preserves superseded instructions.
Commit only when requested or otherwise authorized; this review does not require a commit.
