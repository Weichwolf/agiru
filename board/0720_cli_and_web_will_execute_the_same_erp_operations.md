# 0720 — Deliver equivalent web, agent CMD and MCP clients (G2)

Status: in progress | Priority: P0
Depends on: existing generated page declarations, typed record/session primitives
and database access, not full 0058 acceptance. Include client/workflow-blocking runtime
repairs in coherent client increments; preserve existing tests and counted UT failures.
Next: extract the production page dispatcher and deliver one real
list → card → validate → save slice through HTTP, Node CMD and MCP.

Development packaging: 0726 owns one server/web/PostgreSQL Podman container;
Node CMD/MCP runs outside over HTTP. Queued process families 0727–0740 own BC
sandbox reference execution and agiru replication. Their agiru prerequisites are
specific working client contracts, not this WI's full acceptance; no dependency cycle.
BC capture can proceed while client construction is underway. Keep one WI in progress.

## Existing foundation and refreshed implementation review

- Preserve agiru's generated `PageDef`/control tree, typed bindings, `PageCore`,
  TestPage lifecycle, validated record primitives and regression gates.
  `src/cli/Main.cpp` currently provides the test runner, not an ERP agent client.
- Refreshed archive SHA256:
  `f654cb6576768cb90fff2e0fb701043139ab4d36723e7427a498132a2e2ee6a3`.
  Inspect `~/Git/openerp/openerp/web/client/{protocol,screen,page_model,ui,session,cli_api,request_page}.py`
  and `~/Git/openerp/openerp/cli/terminal.py` before porting contracts.
  Both clients share screen/page/dialog operations; HTTP text commands retain a
  server session between calls, emit keyframes/diffs and stop on dialogs/errors.
- Tests exist for real HTTP commands/files, ten shared posting/payment/customer
  scenarios and Playwright sales flows. These are review evidence, not newly run
  results. Scenario parity uses in-process CLI; screen parity intercepts the model
  passed to the HTML renderer. Neither proves exhaustive actual Node/MCP/DOM parity.
- Board records 2,542/2,578 for CI 89 on 2026-10-03 and 6,106 runtime /
  318 client gates on 2026-10-05. Denominators differ from agiru; no archived
  method-level terminal receipt was established by this review. No 99%-ERP claim.
- Do not transplant one actor thread per user, Python runtime/test architecture
  or Java report rendering. Persistent connections are released between calls
  in the refreshed session code; modal/Commit ownership still needs native proof.
- Import regression scenarios, not unresolved defects: lock-wait posting can
  leave partial results (1897); a shown-disabled action still executes (1903);
  list reads have an OFFSET/LIMIT first step, with deep keyset/FlowField batching
  still open (1868). Old non-TTY CLI auto-confirms defaults; agent CMD must not.

## Concrete ports (read alongside local BC guarantees)

Paths below are relative to `~/Git/openerp/`; reuse contracts, not Python product code.

| Existing mechanism | Reference | agiru implementation |
|---|---|---|
| Shared operation result: done/dialog/error, messages/windows/files | `openerp/web/client/protocol.py::_response`, `screen.py` | One typed C++ dispatcher result consumed by TestPage/HTML/CMD/MCP |
| Field edit/save versus delayed new row; parts refresh | `protocol.py::set_field`, `page_model.py::_collect_rows` | Existing validation/row-leave primitive; invalid text separate, bounded windows+1 |
| Keyframes on context changes, otherwise keyed control/row diffs | `openerp/cli/terminal.py::{lines,keyframe,diff}` | Node semantic-HTML renderer with stable IDs/revisions; always emit dialogs/errors fully |
| Cross-process sessions, stop command batch at a question/error, multipart files | `openerp/web/client/cli_api.py`, HTTP agent client | Typed HTTP handles, explicit answers, local files as bytes; no server filesystem paths |
| Nested modal lookup editable while caller waits | `openerp/web/client/session.py::wait_for_answer` | Explicit suspended command/page state; bounded executor, no dedicated user thread |
| Data-entry arithmetic/trailing sign/date shorthand | `openerp/web/client/entry.py`, `test_client_entry.py` | Server-owned culture-aware typed parser; don't copy hard-coded en-US rules |
| Analysis SQL groups, accumulators and view import/export | `analysis.py`, `analysis_definition.py`, `analysis_store.py` | Shared query plan, exact aggregates, bounded pivots; views keyed by user/company/page |
| Inspect and report request capabilities | `page_inspection.py`, `request_page.py` | Generated app/field/type/filter provenance and 0063's request model |

Port controls: the reference action path checks name membership but not Enabled;
enforce the latter server-side. Chart models convert measures to float; retain exact
typed measures until coordinate projection. Analysis import maps captions and drops
unknown fields; use qualified identities and explicit version/mapping refusals.

## Implementation order

- First-release presentation follows BC's page structure, navigation, controls and
  dialogs closely; defer an independent agiru redesign. Implement with agiru-owned
  code, not copied proprietary client sources. Retain the shared HTML/ASCII contract.
- Adopt BC-compatible URL semantics using agiru deployment origins: `company`,
  `page|query|report|table`, `mode`, `profile`, `bookmark`, `filter`, layout and
  documented presentation flags. Keep authentication/cloud routing separate.
  Use one typed parser/builder for AL `GetUrl`, browser history and CMD/MCP open.
  Preserve bookmark record identity, parameter escaping, field-name filter syntax,
  company isolation and permission checks. Never treat a deep link as authority.
  Observed list → card navigation retains the same bookmark but changes page 22 → 21;
  creating a sales order removes `mode=Create` and gains a bookmark after validation.
  Encode spaces as `%20`: the observed BC entrypoint treats `company=CRONUS+CH`
  as a literal plus and refuses the company. Do not use form-encoded URL builders.
  `node`/`dc` are observed navigation hints, not yet established portable contracts.
  Test malformed/duplicate/conflicting selectors, Unicode/quoted filters, stale
  bookmarks, reload/back/forward and links across permitted/forbidden companies.
  Reference: local developer `devenv-web-client-urls.md` and
  `methods-auto/system/system-geturl-clienttype-string-objecttype-integer-recordref-boolean-string-method.md`.
  Direct sandbox: 2026-10-05, CH BC 28.5, platform `28.0.54688.0`, application
  `28.5.54151.54951` (Help & Support). Keep this host distinct from frozen SDK/demo
  versions. Credentials/raw captures stay outside Git; private archive:
  `~/.local/share/agiru/bc-reference/2026-10-05/manifest.json`.

1. Extract a C++ `PageDispatcher`/`PageSession` below the TestPage adapter.
   Move production control contracts out of `runtime/test/`; reuse generated
   factories/control accessors. One model owns modes, current key/version,
   accepted values versus invalid edit text, parts, dirty/new state and dialogs.
2. Preserve open/fetch/current-row/validate/save/action/close trigger order,
   delayed insertion, header-before-part saves, SetRecords, SubPageLink and
   UpdatePropagation. SaveRecord defaults come from source metadata.
   Enforce inherited mode/Visible/Editable/Enabled and permissions on the server.
3. Add one typed command registry: discover/open/read/set/select/invoke/answer/close,
   filters, navigation, lookup/drilldown, parts, files, reports and analysis.
   Stable app/page/control/row identities, session/page/dialog handles, revisions
   and command IDs; no caption selectors or client-local business tables.
4. Add a thin C++ HTTP adapter and semantic HTML/htmx rendering over that registry.
   Run blocking AL/libpq on a bounded executor, not the event loop. Sessions remain
   private; PostgreSQL owns shared permission revisions, fencing and receipts.
   No connection or transaction during user think time; modal suspension is explicit.
   Drogon and daisyUI/Tailwind are proposals, not adopted dependencies.
5. Implement the Node.js/TypeScript HTML-to-ASCII agent client; CMD and local
   stdio MCP share one client library and the same HTTP business endpoints.
   Parse a bounded, versioned semantic HTML profile, not a general browser/htmx engine.
   No Node ERP server, Python sidecar or second business implementation.
6. Tagged Decimal/Int64 strings preserve exact values/scale; enum identity/ordinal
   stays separate from caption. Server parses submitted display text by culture/type.
   ASCII framing preserves Unicode content and safely escapes terminal controls.
7. CMD defaults to deterministic compact text, optional lossless JSON, stable
   error/exit codes, stderr diagnostics and no TTY/ANSI/blocking prompts.
   MCP uses structured schemas/results, stdout protocol-only and a pinned SDK.
   Cross-process handles continue the same session; explicit modal answers only.
   Bound pages/fields/output, report truncation and expose cursors/full values.
8. Authenticate/re-authorize every command and related-table read. Serialize session
   mutations, expire state and fence stale owners. Protect cookie commands with
   CSRF/origin checks; escape HTML; bound bodies/files and configured origins.
   Untrusted business text is data. Never blindly retry an uncertain posting:
   reconcile command receipts after ambiguous Commit/disconnect.
9. Implement charts and ledger analysis through the same registry: exact typed
   measures, filters, SQL groups/HAVING, sum/count/average/min/max, calendar groups,
   pivots, private saved views, exports and authorized drilldown. Persist definitions
   in PostgreSQL by user/company/page; bound scans/pivots/memory and cancellation.
   Share query/FlowField plans, not browser-local whole-ledger aggregation.
   SVG chart interaction shares scenes with report export under 0721.
   Role-center 9022/part 1392 sandbox sample renders an SVG aged-payables chart:
   Week has 14 bars, Day has 16. Period actions recompute buckets; clicking Older
   opens page 29 with Due Date/Open filters. Preserve AL DataPointClicked/index
   semantics and typed filters; independently reconcile totals before parity claims.
   Sources: `HelpAndChartWrapper.Page.al`, `BusinessChartBuffer.Table.al`.
   Analysis views persist independent column/group/pivot/filter/sort definitions;
   support create/save/rename/duplicate/reorder/delete and definition import/export.
   Copying must not alias the original mutable definition. Share links reopen an
   authorized copy and retain explicit company binding (or an explicit unbound
   choice), not grant data access. Prove persistence across reconnect and isolation
   across users/companies. Reference: user `business-central/analysis-mode.md`.
   Sandbox proof (2026-10-05, page 20): rename with description, account grouping,
   pivot mode and Duplicate work. Adding document-type grouping to the copy leaves
   the original unchanged; both definitions survive a full page reload. Original
   also pivots Document Type into column labels; the copy retains its own row groups.
   Exported
   `.analysis.json` preserves target object, column state, filters, pivot mode and
   dependencies. Share exposes company binding; recipient permissions remain untested.
   Packaged `analysisviews`/`DefinitionFile` on pages/extensions/customizations are
   documented, not yet exercised: immutable shared definitions, editable private
   copies and profile ownership. Read developer `devenv-analysis-view-package.md`.
10. Recreate the reviewed ten scenarios in `test/ui/` for actual shell CMD,
    MCP transport and htmx HTTP on equivalent disposable committed clones;
    use independently queried SQL effects. Sample the real browser/DOM separately.

## Acceptance

- Inventory product-scope business processes from `~/Git/dynamics365smb-docs/`
  (reviewed revision `0ff62b2266fd97265c1be00802b8ac23c0f22b2d`). Prioritize
  setup/master data → sales → purchases → finance → inventory/warehouse →
  remaining ERP areas. Keep every required process visible with commands,
  independent ledger/stock/SQL expectations and its acceptance status.
- Prepare a reproducible sales pitch: resettable demo seed, customer-to-payment
  and vendor-to-payment chains, stock movements, report/analysis samples and
  measured latency/resources. Real browser samples verify presentation;
  agent CMD executes every business step. Disclose gaps rather than claim 99% coverage.
- Every registered business capability has CMD/MCP/web operation coverage:
  messages/codes, typed values, ordered rows, filters/selections, modes, dialogs,
  permissions, artifacts and committed/rolled-back effects. Missing adapters and
  web-only extension/add-in business actions remain counted parity gaps.
- Independent AL/SQL expectations detect a shared wrong implementation. Mutants
  remove a handler/HTML identity, alter scale/order, bypass permissions, duplicate
  posting, use stale revisions or suppress truncation; each must fail.
- Preserve existing tests and G1's independently counted population; a green G1 is
  not a client-start prerequisite. Native durability/lock-timeout/Commit-followed-by-
  error tests are distinct from rollback-only client scenario fixtures.
- Agent workflows cover sales/purchases, journals, inventory/warehouse, recovery
  and report request options/filters/SaveValues/scheduling/downloads.
  Browser samples cover list/card/document+part, modal, validation, posting,
  upload/download, report, keyboard/focus and reconnect/stale DOM updates.
- Ledger/chart parity includes more data than a fetch block, authorized related
  fields and separate users/companies; unsupported expressions explicitly refuse.
  No speed or 99%-business claim from page-open counts or transcript equality.
- Reference/acceptance matrix includes attachment upload/download/delete and exact
  bytes; list filters/views and analysis grouping/pivots; FactBox selection refresh
  and drilldowns; nested lookup selection/cancel; Option/Enum identity versus caption
  and Code normalization; role-center business charts/periods/legends/drilldowns;
  report request/layout selection; workflows/approvals; and Database*/Table
  Information pages with authorized metadata and bounded live operational providers.
  Track observed, documented-only, refused and unexecuted cases separately.
- Workflow reference: pages 1500/1501/1505 and
  `System/Workflow/WorkflowSubpage.Page.al`. Copied purchase-approval template
  `MS-POAPW` has six conditional steps; its first response chain restricts the
  record, sets Pending Approval, creates and sends approval requests. Event filters
  include header and line tables and open through OnAssistEdit, only when editable.
  Own disabled copy was renamed (Code uppercasing plus related-record confirmation)
  and exported as XML; existing enabled workflows were untouched. Import overwrites
  an existing Code per `across-how-to-export-and-import-workflows.md`: require an
  explicit overwrite decision. Approval execution/delegation/job-queue recovery and
  posting restrictions remain unexecuted; XML export is not workflow execution proof.
- Attachment reference: own marked sales order → Attachments → page 1173;
  upload dialog accepts a local UTF-8 file, stores extension/type/user/time and
  Flow to Sales Trx. Download reproduced all 116 bytes (SHA256
  `99c3ef35b467e1484dd6b508b8eadd80f21cf66c8ee159c97eb6df5a1fd20281`);
  own attachment was deleted with confirmation and its downloaded copy retained.
  OneDrive was not exercised and is excluded. Native attachment/media ownership,
  posting transfer and independent user/company access still need qualification.

## References and consolidation

agiru: `include/meta/PageDef.h`, `include/runtime/test/PageCore.h`,
`src/gen/PageWriter.cpp`, `src/rt/{TestPage,Session}.cpp`, `src/cli/Main.cpp`.
Local developer: page/control methods, `devenv-testing-pages.md`,
`properties/devenv-analysismodeenabled-property.md`; user:
`business-central/analysis-mode.md`. Read current local guarantees before porting.
Reviewed tests: `~/Git/openerp/test/openerp/runtime/test_client_{equivalence,scenario_parity,screen_parity,probes,e2e_sales,swallowed_db_error}.py`,
`~/Git/openerp/test/specs/client/`; predecessor findings 1771/1772/1791/1868/1897/1903.
Absorbs 0030 (including its previous absorbed IDs); detailed recovery:
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
