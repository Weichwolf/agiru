# 0063 — Reports will execute datasets and render declared layouts

Status: open | Priority: P2 | Reviewed: 2026-09-22

## Current evidence

Report datasets and request pages have generator/runtime paths in PageWriter.cpp and Report.h/.cpp. SaveAsPdf/Print/Preview still require a renderer. Two old files used ID 0063; they are consolidated here, retaining the implemented dataset work.

## Implementation for Sol

1. Separate report dataset/lifecycle metadata from page request controls without duplicating the page engine. Verify nested dataitem ordering, link/view filters, column expressions, temporary items, Skip/Break/Quit and all report/extension triggers.
2. Resolve request filters per dataitem/table identity; preserve page-owned members over control-name collisions. Wire report substitution and declared platform events through normal event dispatch.
3. Read rendering and legacy layout declarations with explicit precedence, language/format region, limits and timeouts. Translate supported RDL layouts to XSL-FO and render through Apache FOP.
4. Implement request-page handlers, preview lifecycle, streams/files and scheduling using the same dataset pipeline. Unsupported layout features must refuse with names/counts.

## Acceptance

Golden dataset and lifecycle fixtures precede PDF comparison. Render a representative invoice and compare rows/totals/captions; preview must follow its documented second-run behaviour. Prove output failure rolls back only the intended boundary.

## References

Platform: devenv-report-object.md, report/dataitem triggers, rendering/layout properties and reportinstance overloads. AL: report declarations and extensions. Predecessor: WI-1082 (request-page name precedence) and report view/layout findings.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `allowscheduling`, `clearlayout`, `dataitemlink`, `dataitemlink-reports`, `dataitemlinkreference`, `dataitemtableview`, `defaultlayout`, `defaultrenderinglayout`, `enableexternalimages`, `enablehyperlinks`, `excellayout`, `excellayoutmultipledatasheets`, `executiontimeout`, `formatevaluate`, `id`, `includecaption`, `ispreview`, `layoutfile`, `maximumdatasetsize`, `maximumdocumentcount`, `mimetype`, `multiplenewlines`, `optionmembers`, `optionmembers-report`, `previewmode`, `printonlyifdetail`, `processingonly`, `promptmode`, `rdlclayout`, `requestfilterfields`, `requestfilterheading`, `savevalues`, `sharedlayout`, `showprintstatus`, `summary`, `tableno`, `testhttprequestpolicy`, `topnumberofrows`, `type-report`, `userequestpage`, `usesystemprinter`, `version`, `wordlayout`, `wordmergedataitem`.
