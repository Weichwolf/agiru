# 0720 — CLI and web will execute the same ERP operations

Status: open | Priority: P2 | Stage: Clients; start only after G1 | Reviewed: 2026-09-28
Depends on: G1 all UT green; 0030 page dispatcher; 0006 session ownership; 0062 authorization; 0722 exact JSON.

## Evidence

- `src/cli/Main.cpp` exposes run-tests/version only. No HTTP server or test/ui directory exists. TestPage success is not client parity.

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

## Acceptance

- Every registered operation has CLI and web coverage; missing adapter/handler/unsupported capability fails the parity census.
- Extension/control-add-in business actions require the same machine-action/result contract. A web-only business path remains a counted parity gap; presentation-only differences do not require a terminal replica.
- Compare messages/codes, typed values, ordered rows, filters/selection, modes, dialogs, artifacts and committed/rolled-back writes. Normalize only declared nondeterministic IDs/timestamps with preserved identity relationships.
- Independent AL/SQL expectations catch a shared wrong implementation. Negative controls disconnect one adapter, omit a value, change order/scale, bypass permissions and duplicate a posting; each is detected.
- CLI completes sales/purchase posting, journals, stock/warehouse and error recovery as capability coverage grows. Browser samples verify the actual htmx/DOM path; a shared dispatcher alone does not prove UI wiring.
- G2 is required before expanding execution to the complete AL suite; keep the UT regression gate green throughout.

## References

Code: `src/cli/Main.cpp`, `include/meta/PageDef.h`, `include/runtime/test/PageCore.h`, `src/gen/PageWriter.cpp`; extract production lifecycle under 0030. Platform: `devenv-testing-pages.md`, TransactionModel attribute and page/control methods. Predecessor: WI-1113/1401/1411; do not replay load triggers indiscriminately. New ID: all-history maximum 0719 checked on 2026-09-28.

Stack proposals (2026-09-28): [Drogon](https://github.com/drogonframework/drogon), [daisyUI build integration](https://daisyui.com/docs/install/), [Oat++ DTO alternative](https://oatpp.io/docs/components/dto/). No dependency installed by this review; compare dependency footprint, Clang/GCC/aarch64 builds, bounded executor and cancellation before adoption.
