# 0063 — Execute report datasets and render real layouts

Status: queued | Priority: P1
Depends on: 0013/0044's declared metadata and record/filter/aggregate contracts.
Finish dataset/native successor faults required by UT; request/layout work follows
concrete client/report prerequisites, not a fully green G1 milestone.
Client request presentation uses 0720's dispatcher; no dependency on complete G3.
Next: qualify installed native layouts/dataset rows, then bind one request-page model.

## Implementation and useful refreshed details

1. Preserve `src/gen/{ReportAssets,ReportLayoutsWriter,CodeunitWriter}.cpp`,
   `src/rt/Report.cpp` and registry-only `include/runtime/ReportRegistry.h`.
   Source/package/link fixtures do not prove installed/approved/selected assets.
   Resolve original native consumer losses and versioned owner/default selection.
2. Qualify nested dataitem ordering, links/views/filters, temporary sources, columns,
   Skip/Break/Quit and report/extension/substitution triggers. One bounded typed XML
   dataset pipeline serves all output; errors remain errors, not empty PDF success.
3. `~/Git/openerp/openerp/web/client/request_page.py::{model,apply,schedule}`:
   request options read their control values/captions/choices; recursively flatten
   dataitems and expose RequestFilterFields including FlowFilters. Answer applies
   options and filters before execution. CLI reqset/reqfilter and web use one model.
   Reuse agiru's generated request controls; unknown supplied fields must refuse.
4. `runtime/base/report.py` and predecessor 1806: SaveValues differs for Print/Send
   versus Preview/Cancel; Schedule serializes request parameters into Job Queue/
   Report Inbox. Reuse generic background primitives, not UI-specific scheduling.
   Keep PDF, genuine Word/Excel layouts and XML/data-only exports distinct.
5. Inventory DOCX/RDLC/Excel/custom assets, owners/versions/hashes/notices. Install
   catalogue approval/company/private defaults and upgrade/rollback independently.
   Translate each DOCX/RDLC version once into typed layout + documented HTML/CSS
   print profile, bind XML at execution; preserve expressions/groups/nested data.
6. Implement C++ layout/pagination/shaped text/packaged fonts and positioned scenes
   for Cairo PDF/SVG. Reuse exact Business Chart measures/scenes; interactive
   selection/drilldown remains 0720. No FOP/Office/Python renderer or generic table fallback.
7. Pin WPT wptrunner/reftests, same-engine match/mismatch/fuzzy oracle, fonts/viewport
   and declared profile. Count failures/crashes/unsupported/unexecuted; independently
   compare BC datasets/layouts/paginated PDFs/workbooks. Bound spool/concurrency/memory.
   Prove the shared Cairo/shaping build in WASM; don't impose demo constraints on Linux.

## Acceptance and references

- Refreshed predecessor 1958/1971/1983/1985: the request page is a live retained report
  instance, not a form applied only at OK. Immediate validation refreshes dependent
  options; collision-free control IDs, read-only/visible predicates, TableRelation,
  lookup/assist/drilldown and exact Option/Enum identity/caption are required.
  Option edits and dataitem filters are distinct operations; unknown controls refuse.
  ProcessingOnly exposes OK/Cancel, not Print/Preview. Exercise SaveValues and
  OnQueryClosePage outcomes separately, with CMD/MCP/web and independent SQL.
- Predecessor 1986's nine refused RDLC layouts identify needed generic features:
  column-group tablix (AccountSchedule/Portrait), String.Format, Rnd, PageNumber
  expressions and charts (CustomerTop10List/BOM/timeline). Inventory and qualify them
  in the C++ typed-layout/Cairo path; no XSL-FO/FOP or raw dataset-table PDF fallback.
  Genuine financial column groups precede optional visual polish; charts remain required.
- Sources: `~/Git/openerp/test/openerp/runtime/test_{client_request_page_live,
  request_page_control_keys,request_page_close_query,client_processing_only_report,
  rdlc_layout}.py`, `openerp/web/client/request_page.py`, `scripts/transpiler/rdlc_gen.py`.
  Their renderer startup/warm-up measurements are not native performance evidence.

- Ordinal dataset text: `src/rt/Report.cpp::ReportDataset::Add` selects display
  Format 0 only for held Option/Enum values; other scalars retain XML Format 9.
  It reuses `OrdinalInVariant::ToText` without changing held types, numeric Format
  2/9 or metadata. `ReportGate` covers sparse/unknown ordinals, caption/name fallback,
  blank members, Unicode/XML escaping, unchanged metadata, exact integers,
  XML Booleans and Decimal scale 28. `make variant-text JOBS=2`: 335 assertions
  and five compiled defects across Variant/Format/RecordRef/Report; numeric report
  ordinals and display-formatted Boolean XML must fail. No separate test script.
  `GenReportGate` retains 27 checks. Focused Report.cpp/ReportGate tidy each retain
  ten findings in unchanged Codeunit/Page/Report/Table declarations; no suppression.
  BCApps `d99152ee35f0`: RemittanceAdviceEntries.Report.al column
  DocType_VendLedgEntry2 and RemittanceREPCheckUT.Codeunit.al method
  RefundSharingPaymentDocShownOnRemittanceAdviceEntries independently require
  `Refund`, not `6`. Fresh-clone diagnostics fail before and pass 1/1 after;
  only libagiru_rt's hash changes in the mixed image. This is neither full UT gain
  nor causal A/B proof against a sealed native seed. The owned diagnostic clone
  was removed; the unsealed/null seed was untouched. Reproduce with
  `AGIRU_TEST_PROCEDURE=RefundSharingPaymentDocShownOnRemittanceAdviceEntries`
  and `agiru run-tests --codeunit 'Remittance REP Check UT'` on an owned fresh clone.
  Developer `f928288ee840`: devenv-format-report-field-data.md and
  devenv-report-dataset.md; user docs `bf5ffffa9b026`: ui-work-report.md;
  predecessor 1643 and runtime/base/report.py's separate preview/schema writer.
  BC29 NCL SHA256 `277e35cbdfb8`: ReportProcessorXmlDatasetGenerator WriteValue
  RVA 159700 / FormatOjectInstance 15975c formats CLR scalars separately and
  otherwise uses ToString; this server-XML path does not prove all preview,
  localization or schema guarantees. BigInteger/schema types, preview versus
  server-XML shapes, regional/language formatting and bounded spooling remain gaps.

- Direct sandbox reference (2026-10-05, CH BC 28.5/application 28.5.54151.54951):
  report 1306 exposes eleven installed
  layouts across multiple declaring extensions, including RDLC, Word, QR-enabled,
  theme and email-body variants. Preserve owner-qualified identity, status/default
  selection and subtype; report ID alone cannot select a layout.
  Exported RDLC and standard Word inputs, a single-record A4 PDF and its XML
  `ReportDataSet` are retained privately outside Git. Real request page exposes
  options, posted-invoice filters, printer/layout choice and PDF/XML/Word/data-only
  Excel outputs. Reopening after Send to restores the record filter in this sample;
  Preview/Cancel/scheduling persistence remains to qualify separately.
  Sources: user `ui-manage-report-layouts.md`, `ui-work-report.md`; AL
  `Foundation/Reporting/ReportLayouts.page.al` and report 1306 declarations.
  New composite Word body/theme/header-footer overrides are documented requirements,
  not proven by the captured stand-alone Word sample. Do not copy an obsolete
  custom-layout export restriction onto the modern extension-layout catalogue.

- Existing `make native-report-layouts`, `native-bindings`, `layout-assets-check`
  and `test/reporting/` controls remain; qualified original package retains
  sixteen assets and ordinary/source-bound 562+562 fixture checks.
  These checks are not rendered business reports.
- Request options/filters/SaveValues/scheduling and genuine output effects match
  CLI/web/SQL on disposable clones; malformed or unsupported layouts refuse by identity.
- Reviewed reusable tests:
  `~/Git/openerp/test/openerp/runtime/test_client_request_page.py`,
  `test_text_client.py`, `test_client_e2e_sales.py`;
  `test/specs/client/{sales_order_preview,sales_order_post}.json`.
- Developer report/request-page properties and user `ui-work-report.md` first;
  `~/Git/openerp/openerp/runtime/report_render.py` only for layout findings,
  never its Apache FOP architecture.
- Preserve upstream asset/dependency notices; agiru ownership is not relicensing.

Previous absorbed IDs/matrices:
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
