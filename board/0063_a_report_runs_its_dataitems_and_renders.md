Type: root
State: open
Area: al, gen, rt
Tags: target

# A report walks its dataitems and renders a layout, and 668 objects stop being a hole

`Report` is the largest untranslated object kind. There is no parser, no writer, no door header and
no `CurrReport` -- **`methods-auto/report/` holds 62 signature pages and the door carries none of
them**, and 38 of those pages are `reportinstance-*`, which the completeness counter does not even
look at (board:0059).

## The population, measured 2026-09-04 over the read roots `apps.json` names

| kind | files | where |
|---|---:|---|
| **`.Report.al`** | **668** | base 659, system 1, tests 8 |
| `.ReportExt.al` | 14 | base |
| query files | 164 | board:0064 |
| xmlport files | 51 | board:0065 |

CLAUDE.md counts 1 508 objects in scope with no generator and 668 of them reports -- **44 % of the
whole hole is this one kind.**

**What it does NOT block is phase 1**, and that is worth stating because it decides the rank: of the
2 305 `[Test]` procedures in the milestone (board:0058), **12 declare a `Report` variable, in 5
codeunits**; board:0030 counts 18 UT files that name the type anywhere. So this is a TARGET item --
"THE COMPLETE BC BUSINESS FUNCTIONALITY, NOT A SUBSET" -- and it ranks behind board:0057 and
board:0030, not ahead of them.

## What the platform documents

**The object is three parts and the first two are already solved shapes.**

- `devenv-report-object.md` and `devenv-report-dataset.md`: `dataset { dataitem(name; Table) {
  column(name; Expression) } }`, indented dataitems joined by `DataItemLink`, ordered by
  `DataItemTableView`, `CalcFields` per dataitem, and since recently a QUERY as a dataitem source.
  board:0034 already found that this is the SAME grammar the page layout uses -- `<kind>(<name>[;
  <source>]) { properties triggers children }` -- so the parser is a header and a writer rather than
  a grammar.
- `devenv-report-triggers.md` gives the ORDER completely, and it is not guessable:

  | # | what runs |
  |---|---|
  | 1 | `OnInitReport` |
  | 2 | the request page, if enabled, with its own page triggers |
  | 3 | `OnPreReport` |
  | 4 | per dataitem: `OnPreDataItem`, then per record `OnAfterGetRecord` (indented dataitems run inside it), then `OnPostDataItem` |
  | 5 | `OnPostReport` |

  `triggers-auto/` carries 4 report triggers, 3 dataitem triggers, 12 request-page triggers, 3
  reportextension and 6 reportextensiondatasetmodify -- 28 of the 152.
- **The runtime surface is `Report` plus `CurrReport`**, 62 documented signatures: `Run`, `RunModal`,
  `Execute`, `Print`, `Preview`, `SaveAs`, `SaveAsPdf`, `SaveAsExcel`, `SaveAsWord`, `SaveAsHtml`,
  `SaveAsXml`, `RunRequestPage`, `SetTableView`, `UseRequestPage`, `Break`, `Skip`, `Quit`,
  `NewPage`, `NewPagePerRecord`, `CreateTotals`, `TotalsCausedBy`, `PageNo`, `Language`,
  `FormatRegion`, `RdlcLayout`, `WordLayout`, `ExcelLayout`, `DefaultLayout`, `TargetFormat`,
  `ValidateAndPrepareLayout`.

**THE LAYOUT IS THE THIRD PART AND CLAUDE.md HAS ALREADY DECIDED IT**: "Reporting is XSL-FO through
Apache FOP to PDF, the route the predecessor takes and the one BC's own RDL layouts translate into
most directly." So the open question is not the renderer but WHERE THE DATASET STOPS: the dataset is
AL and belongs in the transpiler; the layout is RDL/Word and belongs in a converter; and the seam
between them is the report's XML dataset, which BC itself materialises
(`devenv-report-dataset.md`, "Testing the dataset").

## What the predecessor paid for, and every one of these is a trap in the dataset half

`~/Git/openerp` has `report_gen` (995 lines) and its board carries the rounds:

| item | finding |
|---|---|
| **WI-1068** | `CurrReport.Break` was only a FLAG, so a dataitem never stopped -- **infinite loops**; 9 of 24 scattered timeout tests were this, across 6 codeunits |
| **WI-1066** | `REPORT.PRINT` was not implemented, so the report never ran at all and the failure surfaced as a layout error |
| **WI-1083** | `Report.Run` in the NAME form discarded the record it was handed |
| **WI-1064** | `Report.SaveAs` swallowed errors raised inside the report |
| **WI-1099**, **WI-1073** | `SaveAs` with `ReportFormat::Html` wrote the XML dataset instead of HTML, and rendered no layout |
| **WI-1032** | a dataset column name was missing from the embedded XML schema |
| **WI-1126** | report datasets landed as GUID-named files in the working directory -- CLAUDE.md's "artefacts go to `build/`" from the other side |
| **WI-1031** | the report-handler dispatch swallowed errors raised in the report's own dataset |

`CurrReport.Break` is the one to carry forward first: it is not a flag, it is control flow out of the
current dataitem, and getting it wrong costs a hang rather than a wrong answer.

## 280 OF THE 676 REPORTS NEED NO LAYOUT AT ALL, and that re-ranks the whole item

`devenv-report-object.md` names three scenarios, and the third has no output:

> **processing-only reports, where there's no output.** In this case, the report object is typically
> used with a request page to let the user set filters/options for the operation.

Measured over `Layers/W1` on 2026-09-04:

| | |
|---|---:|
| `.Report.al` files | 676 |
| declaring **`ProcessingOnly = true`** | **280 -- 41 %** |
| naming a layout (`RDLCLayout` or a `rendering` section) | 395 |

**So two fifths of the reports are batch jobs wearing a report's clothes**: a dataset, a request
page and code, and nothing to render. `Adjust Exchange Rates`, `Date Compress`, `Suggest Vendor
Payments` -- the BaseApp's processing is written as reports. They need the dataitem walk, the
triggers and the request page; the XSL-FO route is not on their path at all.

That splits this item cleanly and puts the cheap half first: **the dataset engine and the trigger
order deliver 280 objects with no renderer**, and the remaining 395 wait for the layout work.

## The object's shape, from the same page

Five sections in a MANDATORY ORDER -- properties, `dataset`, `requestpage` (optional), `rendering`
(optional), then code -- plus a `labels` block whose entries carry `Comment`, `MaxLength` and
`Locked` exactly as a `Label` does (board:0067's truncation applies here too). Three layout types,
and **Excel does not support printing from the request page** while RDL and Word do.

## The choice

- **The dataitem walk is the record layer, not a second reader.** A dataitem is a filtered, ordered
  read over a table -- board:0044's navigation and board:0045's cursor -- with `DataItemLink`
  becoming a filter on the child from the parent's current row. Nothing new belongs in `src/db` for
  this.
- **`CurrReport` is a session-scoped object with the report's frame**, and `Break`/`Skip`/`Quit` are
  control flow the generator emits as such, never a boolean somebody checks later.
- **The request page is a page** (board:0030) and a `RequestPageHandler` answers it (board:0054);
  a `ReportHandler` REPLACES the whole run including the request page, so the two must not both fire.
- **The dataset is emitted before any layout exists**, and it is what the first gate asserts. A
  report whose dataset is right and whose layout is absent is a report whose remaining work is a
  converter; the reverse is not true.

## Gate

A report with two dataitems, the second indented and linked: the trigger order is the tabulated one,
the child runs inside the parent's `OnAfterGetRecord`, and the dataset carries the rows the same
filters would give through a `Record`. `CurrReport.Break` in the child leaves the child and continues
the parent -- **and the negative control is a report where `Break` is ignored, which must TIME OUT
rather than pass** (WI-1068). `Report.Run` by name and by id hand the same record through
(WI-1083). An error raised in `OnAfterGetRecord` reaches the caller of `SaveAs` (WI-1064).

## THE REQUEST PAGE HAS STATE THAT OUTLIVES THE RUN, read 2026-09-04 (board:0071)

`ui-work-report.md` documents three things about running a report that are the PLATFORM's and not
the report's, and each is a rule with an order rather than a preference.

**"Last used options and filters" is ALWAYS available.** The request page's options and filters are
persisted per user per report, and a named set ("predefined settings") can be selected instead --
"The changes you make aren't saved to the predefined settings entry you select, but they're saved to
the **Last used options and filters** entry." So there are two stores with different write rules,
and the anonymous one is written on every run. That is a system table, not a report's own field, and
it is what a `RequestPageHandler` (board:0054) stands in front of: a test that runs a report twice
sees the first run's filters unless the handler sets them.

**THE LANGUAGE AND FORMAT REGION RESOLVE IN A DECLARED ORDER, and the page states it as one:**

1. the settings given when the report is generated;
2. the settings on the DOCUMENT, which come from the customer's or vendor's card;
3. the settings on the Report AL object.

That is `Format`'s culture (board:0066) decided three levels up, and it means a report cannot format
anything through a session-global. The value has to reach `Format` as a parameter, which is the same
conclusion board:0066 reaches from the other side.

**The `Filter` FastTab's filters are optional AND some reports ignore them**, with no list of which:
"It's not possible to provide a list of which fields are ignored in which reports." So a generic
filter reaching a dataitem is the report's own business, and the platform owes only that it is
offered -- which is a relief rather than a gap, because it removes a class of expected behaviour
this item would otherwise have to reproduce.

**The output kinds are PDF, Word, Excel, `Excel (data only)` and XML**, and the LAYOUT KIND decides
which actions exist: "When a report uses an Excel layout, you don't have the **Printer** field or
**Print** or **Preview** buttons. Instead, there's a **Download** option." So the request page's
action set is derived from the selected layout, not fixed.

**Scheduling is the Job Queue and needs one platform capability**: a report scheduled with a
`Next Run Date Formula` (board:0082) runs in a background session and lands in the Report Inbox.
BaseApp function on a platform primitive agiru does not have, named here so the dependency is
visible rather than discovered.

## THE LAYOUTS ARE FOUR KINDS AND ONE OF THEM IS 72 % OF THEM

`ui-manage-report-layouts.md` names the four layout TYPES -- **Word** (`.docx`), **RDLC**
(`.rdl`/`.rdlc`), **Excel** (`.xlsx`) and **External** (a partner's own renderer) -- and the three
layout SOURCES: from an extension (the AL object's own), user-defined (a table row), and the
built-in ones. Measured 2026-09-04 over `~/Git/BCApps/src`, all `.al`:

| declaration | count |
|---|---:|
| `LayoutFile = '*.rdlc'` in a `rendering` block | 608 |
| `LayoutFile = '*.rdl'` | 33 |
| `LayoutFile = '*.docx'` | 182 |
| `LayoutFile = '*.xlsx'` | 61 |
| **declared layout files** | **884** |
| `DefaultRenderingLayout` | 646 |
| the OLD form, `RDLCLayout = ` on the report object | 768 |
| `WordLayout = ` / `ExcelLayout = ` (the old forms) | 2 / 1 |

**641 of 884 are RDLC -- 72 %** -- which is the measurement behind CLAUDE.md's "reporting is XSL-FO
through Apache FOP", and it says the same thing this item's own ranking does: the RDLC route is the
one that pays, Word (182) is second, and Excel (61) and External are last. The two forms overlap --
a report may declare `RDLCLayout` and a `rendering` block both -- so 884 and 768 are not added; what
is decidable is that **RDLC dominates under either form.**

`External` is the escape hatch and it has a documented extension point
(`devenv-report-custom-render.md`), so it is a subscriber rather than a renderer here.

**A layout also renders EMAIL bodies, and only Word can**: "To use custom report layouts with email,
the file type for the layout must be Word. You can't use the RDLC file type." That is a constraint
on the renderer's output, not on the report: a Word layout must be renderable to HTML as well as to
PDF.

**WHICH LAYOUT RUNS IS A THREE-LEVEL RESOLUTION, and the middle level is COMPANY-SCOPED.**
`ui-set-report-layout.md`: "When there are multiple companies in the application, the layouts are
set on a per-company basis. So the same report in one company can have a different layout in
another." The order:

1. the request page's **Report Layout** field -- "After you run the report, the layout will revert
   to the default layout again", so it is per RUN and stored nowhere;
2. the `Report Layout Selection` row for THIS COMPANY (board:0060);
3. the object's own `DefaultRenderingLayout`.

Same shape as the language and format-region order above, and the same consequence: nothing about
rendering may read a process-wide default.

**AN EXCEL LAYOUT NEEDS NO RENDERER.** `ui-excel-report-layouts.md`: the layout file "contains the
required **Data** sheet and table, and a **Report Metadata** sheet", and every visual in it --
PivotTables, charts, slicers, formulas -- is Excel's own, computed by Excel over that sheet. So the
runtime's whole obligation for the 61 Excel layouts is to write the DATASET into a workbook beside
the layout's other sheets. Together with the 280 `ProcessingOnly` reports this item already counts,
that is 341 of 676 reachable without any page-description output at all -- and the `Excel Document
(data only)` send option is the same code path, which is why it is also how a user CREATES an Excel
layout.

**A WORD LAYOUT BINDS THROUGH A CUSTOM XML PART.** `ui-how-add-fields-word-report-layout.md`: "You
add fields by using the Word custom XML part for the report and adding content controls that map to
the fields of the report dataset." So rendering one is not a renderer either -- it is writing the
dataset as an OOXML custom XML part into the `.docx` and letting its content controls bind. The
expensive half is `.docx` -> PDF, and it is the same half the email route does NOT need, since an
email body is HTML.

That re-reads the layout census one more time:

| | what the runtime actually owes |
|---|---|
| 280 `ProcessingOnly` reports | no output at all |
| 61 Excel layouts | the dataset into a `Data` sheet |
| 182 Word layouts | the dataset into a custom XML part, then `.docx` -> PDF |
| **641 RDLC layouts** | **the report description translated -- the XSL-FO route CLAUDE.md names** |

**And the fonts are a portability finding rather than a feature.** `ui-fonts.md` lists the fonts
preinstalled on the SaaS servers and forbids uploading others; a layout names one of them by name.
agiru renders through FOP on whatever the host has, so a layout naming a font the host lacks either
substitutes or fails -- and a substitution makes the same report over the same data produce
different bytes on two machines, which is the determinism rule this tree does not trade. The font
set therefore has to be a declared, shipped input to the renderer, not the host's.

## THE RENDER PIPELINE HAS DOCUMENTED STAGES, AND EACH IS AN EVENT ON CODEUNIT 44

Eight root pages describe the extension points of the report output pipeline, all published by
**codeunit 44 `ReportManagement`** (read 2026-09-04, board:0071):

| event | raised when |
|---|---|
| `OnAfterIntermediateDocumentReady` | an INTERMEDIATE artefact exists that will be removed once the final one is made -- "typically Xml or Word files" |
| `OnAfterDocumentReady` | an output artefact exists that can be persisted |
| `OnAfterDocumentPrintReady` | the artefact is ready to be printed |
| `OnAfterDocumentDownload` | the artefact is being downloaded |
| `OnCustomDocumentMerger` / `...Ex` | a CUSTOM renderer takes a dataset and a layout -- the `External` layout type's entry point |
| `OnGetFilename` | the suggested export filename |
| `OnAfterSetupPrinters` | the printer list is being assembled |
| **`OnAfterSubstituteReport`** | **before the run** -- a subscriber may name a DIFFERENT report to run instead (`devenv-substituting-reports.md`) |

**Every one has the same shape**, and it is the shape agiru's renderer has to expose:

```al
local procedure OnAfterDocumentReady(ObjectId: Integer; ObjectPayload: JsonObject;
                                     DocumentStream: InStream; var TargetStream: OutStream;
                                     var Success: Boolean)
```

-- the report id, a **JsonObject payload of runtime information**, the produced document as an
`InStream`, a `TargetStream` to write a REPLACEMENT into, and a `Success` flag. "The content in the
`TargetStream` will be discarded if the `Success` parameter is `false`", and **"changing the content
type isn't allowed"**.

**That settles three design questions at once:**

- **The pipeline is a sequence of STREAMS**, not a single call. So the XSL-FO route CLAUDE.md names
  produces an artefact that is handed to a subscriber before it is persisted or printed, and the
  dataset -> layout -> intermediate -> final path has named boundaries rather than being one
  function.
- **`documenttype` in the payload identifies what is in the stream**, so one event serves PDF, Word,
  Excel and XML -- the renderer does not need an event per format.
- **`OnCustomDocumentMerger` is how the `External` layout type is implemented**, which board:0063
  already recorded as "a subscriber rather than a renderer here". This is the subscription.

All eight need board:0057 before they need a renderer: they are `[IntegrationEvent]` publishers on a
platform codeunit, so the runtime must be able to raise an event from C++ into transpiled AL and take
a `var OutStream` back. That is the same `var`-parameter path board:0066's AutoFormat needs and
board:0089's non-short-circuit `and` exposes.

**THE PREVIEW MODE IS DERIVED FROM TWO PROPERTIES, AND THE DEFAULT SPAWNS A SECOND REPORT INSTANCE.**
`devenv-report-triggers.md`, re-read in full 2026-09-04 (board:0071):

| mode | what happens |
|---|---|
| **preview & close** | the preview runs inside the CURRENT report instance and the request page CLOSES; the user reruns the report to change anything. The only mode before version 17 |
| **multiple-preview** -- **the default** | the request page's visible control values and filters are CAPTURED, a **child report instance** is invoked to render, and the original request page stays open |

and "the mode is determined by two report properties: `SaveValues` and `AllowScheduling`".

**That is a second instance of the object running while the first is suspended**, so a report's
triggers can be re-entered -- which matters for anything the runtime keeps per report rather than per
run. It also explains why the request page's state (board:0063's "Last used options and filters") has
to be capturable as a VALUE: the child instance is constructed from it.

Each of `OnInitReport`, the request page and `OnPreReport` can END the run before the next stage --
"If the OnInitReport doesn't end the processing of the report, then the request page ... is run" --
so the sequence is a chain of gates and not a fixed pipeline, and a cancelled request page is an
ordinary outcome rather than an error.


**`OnAfterSubstituteReport` is the first stage and it is not part of rendering.** Before extensions
could extend reports at all, "you can override the base report with your own customized version by
subscribing to the **OnAfterSubstituteReport** event published by **Codeunit 44 - ReportManagement**",
and the mechanism survives alongside `reportextension`. So **running a report is: raise the
substitution event, then run whatever object comes back** -- which means `Report.Run(50100)` may
execute object 50123, and any dispatch that resolves the id at translation time is wrong. The id is
resolved at RUN time, through an event, every time.
