# 0063 — Reports will execute datasets and render declared layouts

Status: open | Priority: P1 | Stage: UT datasets; All rendering | Reviewed: 2026-10-02
Depends on: 0030 request lifecycle; 0019 aggregates; 0074 streams.

## Evidence

- Report dataset/request-page paths exist. Remittance UT failures already require dataset work before clients.
- PDF/Print/Preview have no complete renderer; page-shaped report metadata needs separation.
- `ReportDataset` retains every formatted row, then `Xml()` builds a second whole-document string. `WriteReportFile` checks opening but not write/flush completion.
- Current BCApps `6261b1c458`, `Foundation/Reporting/CompositeLayout.ReportExt.al`: reportextension 9666 adds fourteen Word Theme/HeaderFooter layouts to the native Tenant Report Defaults report, replacing upgrade codeunit 104067 in upstream `bf484e587`. `ParseReport`/`ParseReportExtension` skip rendering blocks; AST/`MergeReportExtensions` retain no named layout metadata. Supplied System symbols contain zero reports/report extensions: native target ID/provider authority remains 0034. Keep the retired slice entry's refusal until successor behaviour is implemented; do not restore a stale or empty seeder (0589).

## Implementation

1. Prioritize missing/wrong dataset rows with exact dataitem/trigger traces. Rendering follows dataset correctness; carry report definitions as report metadata and reuse only the request-page portion.
2. Separate report dataset/lifecycle metadata from page request controls without duplicating the page engine. Verify nested dataitem ordering, link/view filters, column expressions, temporary items, Skip/Break/Quit and all report/extension triggers.
3. Resolve request filters per dataitem/table identity; preserve page-owned members over control-name collisions. Wire report substitution and declared platform events through normal event dispatch.
4. Preserve named rendering layouts in the report/extension AST and immutable report definitions, including extension identity, Type/Subtype, LayoutFile, Caption/Summary and asset ownership. Bind the native target through verified platform declarations (0034), never a guessed ID. Read legacy declarations with explicit precedence, language/format region, limits and timeouts. Translate supported RDL layouts to XSL-FO and render through Apache FOP.
   Inventory RDLC/Word/Excel/custom layouts independently; an unsupported format remains an ERP gap. Feed a typed dataset sink into bounded streaming/spooling; use a bounded FOP worker pool, explicit resource resolver and cancellation. Include renderer memory/CPU in 0721, not just the C++ process.
5. Implement request-page handlers, preview lifecycle, streams/files and scheduling using the same dataset pipeline. Unsupported layout features must refuse with names/counts.

## Acceptance

- Golden dataset and lifecycle fixtures precede PDF comparison. Render a representative invoice and compare rows/totals/captions; preview must follow its documented second-run behaviour. Prove output failure rolls back only the intended boundary.
- Full-disk/broken-output/renderer-timeout controls fail loudly; no successful truncated artifact. Large reports do not duplicate the full dataset in service memory; temporary artifacts have scoped cleanup.
- Count all fourteen Composite Layout declarations/assets independently before and after translation. Removed rendering metadata and unresolved native target controls must refuse; declaring layouts alone does not prove installation, selection or document fidelity. An installed extension layout must not silently replace the selected default.

## References

Code: `src/al/{Parser.cpp,Ast.h}`, `src/tc/Main.cpp::MergeReportExtensions`, `src/rt/Report.cpp`, `src/gen/PageWriter.cpp`, `src/gen/BodyWriter.cpp`, `test/gate/GenReportGate.cpp`.

Platform: devenv-report-object.md, report/dataitem triggers, rendering/layout properties and reportinstance overloads. AL: report declarations and extensions. Predecessor: WI-1082 (request-page name precedence) and report view/layout findings.

Layout references: developer `devenv-report-{ext-object,layout-declaration}.md` at `ff5939a46`; BCApps path above; predecessor board 938, report-rendering architecture (generic dataset tables are not layout fidelity). Runtime-18 layout guarantees require an explicit compatibility decision against the pinned Runtime-17 System input. User intent must be read before implementation.

Property scope: `allowscheduling`, `clearlayout`, `dataitemlink`, `dataitemlink-reports`, `dataitemlinkreference`, `dataitemtableview`, `defaultlayout`, `defaultrenderinglayout`, `enableexternalimages`, `enablehyperlinks`, `excellayout`, `excellayoutmultipledatasheets`, `executiontimeout`, `formatevaluate`, `id`, `includecaption`, `ispreview`, `layoutfile`, `maximumdatasetsize`, `maximumdocumentcount`, `mimetype`, `multiplenewlines`, `optionmembers`, `optionmembers-report`, `previewmode`, `printonlyifdetail`, `processingonly`, `promptmode`, `rdlclayout`, `requestfilterfields`, `requestfilterheading`, `savevalues`, `sharedlayout`, `showprintstatus`, `summary`, `tableno`, `testhttprequestpolicy`, `topnumberofrows`, `type-report`, `userequestpage`, `usesystemprinter`, `version`, `wordlayout`, `wordmergedataitem`.
