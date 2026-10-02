# 0720 — CLI and web will execute the same ERP operations

Status: open | Priority: P2 | Stage: Clients; start only after G1 | Reviewed: 2026-10-02
Depends on: G1 all UT green; 0030 page dispatcher; 0006 session ownership; 0062 authorization; 0722 exact JSON; 0018/0019/0064 analysis filters/aggregates; 0035 chart contracts; 0063 chart/export rendering.

## Evidence

- `src/cli/Main.cpp` exposes run-tests/version only. No HTTP server or test/ui directory exists. TestPage success is not client parity.
- Predecessor page smoke reached 2,517/2,653 open/render cases, not business parity. Its thin-client experiment exposed two transferable requirements: modal AL execution needs an answer/resume protocol, and lookup targets must come from runtime metadata rather than hard-coded field-name maps. Do not copy its thread/ContextVar machinery or start clients before G1.
- Business Charts and interactive ledger-page analysis are explicit product requirements, not PDF-only presentation. `AnalysisModeEnabled` defaults true; analysis retains page filters and private saved views, grouping/pivots, totals, date hierarchies and authorized drilldown/related fields. No implementation/parity proof exists. BC large-dataset/browser limitations are reference behaviour, not permission to silently drop fields or totals in agiru.

## Implementation

1. Define one typed operation registry in rt: discover/open/read/filter/select/edit/action/answer/close, lookup/drilldown, parts, upload/download and report outputs. Requests identify session, page/control, command ID and observed revision; results contain typed values, rows, capabilities, dialogs, errors and resulting revision.
2. Generate page factories and control accessors from PageDef/ControlDef. Use stable app/page/control identities, not translated captions. Preserve Decimal/BigInteger as exact typed values in JSON; no floating-point conversion.
   Wire proposal: `{"type":"Decimal","value":"123.4500"}` and tagged Int64 strings; enum ordinal/identity separate from caption. Parse htmx field text on the server using declared culture/type. Bypass framework JSON-number helpers if they coerce to double; browser code never uses Number for exact AL values.
3. Implement HTTP hosting as an adapter reaching rt. Both JSON commands for `agiru client --json` and htmx form actions dispatch through the same registry and lifecycle. CLI discovers valid actions and maintains page/dialog handles; it supports real ERP operation and scripted workflows.
   Proposed stack: Drogon routing/filters/HTTP only, no ORM; existing PostgreSQL transaction authority. Run blocking AL/libpq work on a bounded executor, never the event loop; session activation must survive worker changes safely under 0006. Generate htmx HTML with daisyUI/Tailwind CSS compiled at build time, no production Node.js dependency. Implement ERP grid navigation/focus explicitly; CSS components do not provide page semantics. Oat++ is the DTO/OpenAPI-oriented alternative, not an additional dependency.
4. Authenticate both clients identically. Serialize each session's commands; validate permissions, modes and record versions server-side. PostgreSQL owns session ownership/fencing and durable command receipts; process-local AL page state is private, bounded and expires. Lost owner means explicit session recovery, never silent state reuse.
5. Prevent stale/duplicate writes with revision checks and command receipts. Commit receipts atomically where one transaction permits; after ambiguous disconnect across AL Commit boundaries report uncertain outcome and reconcile before retrying a posting.
6. Place exhaustive operation/codec/authorization parity fixtures and workflow scenarios in test/ui/. Replay identical commands through CLI JSON and htmx HTTP endpoints on equivalent fresh clones; compare normalized transcripts and independently queried ledger effects.
7. Browser sampling only: list/card/document+part, modal lookup, failed validation, posting, upload/download, report, keyboard/focus and reconnect. Retest affected samples on renderer changes; all business regression runs use the CLI.
   Reject stale/out-of-order DOM patches by page revision; preserve focus and unsaved control text across partial refreshes. Use stable control/row IDs in HTML, not caption selectors.
8. Classify page-open failures into expected AL refusals, runtime defects and transport/render defects; opening a page is not an operation-parity pass. Test modal answer/resume while its caller remains suspended and reject client-local table/field lookup maps.
9. Add typed chart read/series-selection/drilldown and analysis enter/leave/filter/sort/group/pivot/aggregate/view/export commands to the same registry. CLI consumes exact chart data and drives every business interaction; web renders SVG and dispatches the same commands. Reuse the chart scene for static PDF exports (0063), not an independent calculation or PDF-based interactive UI.
10. Derive ledger analysis from authorized page/query metadata and filter state. Reuse typed SQL/filter/aggregate plans (0018/0019/0064), including FlowFields and supported page expressions without bypassing AL semantics. PostgreSQL performs eligible grouping/HAVING; bound pivot cardinality, row blocks, cancellation and per-session memory. Never load the complete ledger into the browser or report a page-sized subset as the full total. Persist versioned private view definitions in PostgreSQL with user/company/page identity; shared links recheck recipient permissions.

## Acceptance

- Every registered operation has CLI and web coverage; missing adapter/handler/unsupported capability fails the parity census.
- Extension/control-add-in business actions require the same machine-action/result contract. A web-only business path remains a counted parity gap; presentation-only differences do not require a terminal replica.
- Compare messages/codes, typed values, ordered rows, filters/selection, modes, dialogs, artifacts and committed/rolled-back writes. Normalize only declared nondeterministic IDs/timestamps with preserved identity relationships.
- Independent AL/SQL expectations catch a shared wrong implementation. Negative controls disconnect one adapter, omit a value, change order/scale, bypass permissions and duplicate a posting; each is detected.
- CLI completes sales/purchase posting, journals, stock/warehouse and error recovery as capability coverage grows. Browser samples verify the actual htmx/DOM path; a shared dispatcher alone does not prove UI wiring.
- G2 is required before expanding execution to the complete AL suite; keep the UT regression gate green throughout.
- Chart fixtures compare series/types, labels, exact measures, selection and drilldown results through CLI/web/export; browser samples verify real SVG wiring. Negative controls swap series, truncate data and bypass permissions.
- G/L, customer, vendor and item ledger analysis fixtures cover page+analysis filters, group subtotals, sum/count/min/max/average, calendar date hierarchies, pivots, saved/reopened views, related-field authorization and exact exports. Compare against independent SQL including more rows than one fetch block; analysis never mutates ledger entries. Two users/companies cannot leak views/data. Disabled analysis and unsupported expressions refuse explicitly; all missing operations remain parity gaps.

## References

Code: `src/cli/Main.cpp`, `include/meta/PageDef.h`, `include/runtime/test/PageCore.h`, `src/gen/PageWriter.cpp`; extract production lifecycle under 0030. Platform: `devenv-testing-pages.md`, TransactionModel attribute and page/control methods. Predecessor: WI-1113/1401/1411; do not replay load triggers indiscriminately. New ID: all-history maximum 0719 checked on 2026-09-28.

Stack proposals (2026-09-28): [Drogon](https://github.com/drogonframework/drogon), [daisyUI build integration](https://daisyui.com/docs/install/), [Oat++ DTO alternative](https://oatpp.io/docs/components/dto/). No dependency installed by this review; compare dependency footprint, Clang/Linux x86_64/aarch64 builds, bounded executor and cancellation before adoption.

Browser-only demo delivery is owned by 0724; both targets share the production dispatcher and typed operation contract above.

Analysis/chart references: developer `properties/devenv-analysismodeenabled-property.md` at `ff5939a46`; BCApps `System Application/App/Business Chart/src/{BusinessChart.Codeunit,BusinessChartType.Enum}.al` at `6261b1c458`; user `business-central/analysis-mode.md` at `634710c42` (page filters, private views, pivots, statistics, related fields, permissions and large-dataset limits). Predecessor 1016 rejects empty chart measures/phantom dimensions; property audit classifies analysis as client-phase work. Generic AL chart data/bridge contracts remain 0035, not a UI-only replacement.
