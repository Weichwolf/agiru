# 0063 — Reports will execute datasets and render declared layouts

Status: open | Priority: P0 | Stage: UT compile/datasets; All rendering | Reviewed: 2026-10-02
Depends on: 0030 request lifecycle; 0019 aggregates; 0074 streams.

## Evidence

- Report dataset/request-page paths exist. Remittance UT failures already require dataset work before clients.
- PDF/Print/Preview have no complete renderer; page-shaped report metadata needs separation.
- `ReportDataset` retains every formatted row, then `Xml()` builds a second whole-document string. `WriteReportFile` checks opening but not write/flush completion.
- Frozen `bd455e1` first fails at root 085 on retired UpgradeCompositeReportParts.cpp (104067), after PEPPOL root 095 compiles. BCApps `6261b1c458`, `Foundation/Reporting/CompositeLayout.ReportExt.al`, replaces that codeunit with reportextension 9666 and fourteen Word layouts (three Theme/eleven HeaderFooter). Parser/AST/extension merge currently discard them. Preserve the slice refusal until generic successor behaviour is proved (0589); no stale or empty seeder.
- CompositeLayoutLookupHelper.Codeunit.al::GetTenantReportDefaultsReportID explicitly returns 2000000001 for Tenant Report Defaults. This is an AL reference, not its complete native declaration/dataset/provider contract; System symbols contain zero reports/report extensions (0034). The helper separately resolves header/theme through six precedence levels and requires Approved parts; retained layout declarations alone do not implement those behaviours.

## Implementation

1. Close the measured compilation blocker first: preserve all named report/extension layouts and owned assets, merge them by report/app/extension identity, obtain the source-backed native target contract (0034), then prove the actual successor before retiring its missing slice entry (0589). Unsupported native binding must refuse explicitly. Follow with missing/wrong dataset rows and exact dataitem/trigger traces; rendering follows dataset correctness.
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

Layout references: developer `devenv-report-{ext-object,layout-declaration}.md` at `ff5939a46`; BCApps `src/Layers/W1/BaseApp/Foundation/Reporting/{CompositeLayout.ReportExt.al,CompositeLayoutLookupHelper.Codeunit.al}` at `6261b1c458`; predecessor board 938, report-rendering architecture (generic dataset tables are not layout fidelity). BCApps AL-Go repoVersion 30/Sandbox 30.0.55525.0 and Runtime-18 guarantees differ from pinned System 28/Runtime 17 and demo 28.4; make compatibility explicit. User intent must be read before implementation.

Property scope: `allowscheduling`, `clearlayout`, `dataitemlink`, `dataitemlink-reports`, `dataitemlinkreference`, `dataitemtableview`, `defaultlayout`, `defaultrenderinglayout`, `enableexternalimages`, `enablehyperlinks`, `excellayout`, `excellayoutmultipledatasheets`, `executiontimeout`, `formatevaluate`, `id`, `includecaption`, `ispreview`, `layoutfile`, `maximumdatasetsize`, `maximumdocumentcount`, `mimetype`, `multiplenewlines`, `optionmembers`, `optionmembers-report`, `previewmode`, `printonlyifdetail`, `processingonly`, `promptmode`, `rdlclayout`, `requestfilterfields`, `requestfilterheading`, `savevalues`, `sharedlayout`, `showprintstatus`, `summary`, `tableno`, `testhttprequestpolicy`, `topnumberofrows`, `type-report`, `userequestpage`, `usesystemprinter`, `version`, `wordlayout`, `wordmergedataitem`.
