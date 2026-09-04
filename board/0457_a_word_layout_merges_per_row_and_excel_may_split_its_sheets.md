Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/properties/devenv-wordmergedataitem-property.md, developer/properties/devenv-excellayoutmultipledatasheets-property.md
Verdict:  fehlt
Class:    activation

# A Word layout merges per row, and an Excel layout may split its sheets

**Two pages, one item**: both are per-layout-type structure switches on the report, both change how
one dataset becomes one document, and neither belongs to the RDL path this tree actually needs first.

> **WordMergeDataItem**: the **root-level data item used to generate SEPARATE reports for multiple
> records.** "Only applied when rendering a report using a Word layout." The server does a **mail
> merge** between that data item and the layout.
>
> **Before 2024 wave 2** the server generated one merged Word document and inserted Word sections to
> reset headers, footers and page numbers -- **so Word sections in the layout were not allowed** and
> the report failed at run time. **From 2024 wave 2** it generates one Word document PER ROW,
> converts each to PDF and merges the PDFs -- **and the restriction is gone.**
>
> **ExcelLayoutMultipleDataSheets** (runtime 12.0, Report and -- from runtime 15 -- Report Layout):
> render to **multiple data sheets** named `Data_DataItemName`, one per root data item, instead of a
> single sheet named `Data`. **The layout-level property OVERRIDES the report-level one**, so a new
> layout can use the feature without breaking existing layouts.

**The Word history is worth keeping even though the new behaviour is the one to build**: one document
per row converted to PDF and concatenated is a different pipeline from one merged document, and it is
the one that matches agiru's route -- FOP produces PDFs, and concatenating PDFs is a thing FOP's
neighbours do.

**And the Excel override is the ordinary direction** (more specific wins), unlike board:0374's
`DataCaptionFields`, where the page's declaration is dead. Three properties in this sweep with
explicit precedence, each stating its own.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`WordMergeDataItem =` **299** · `ExcelLayoutMultipleDataSheets =` **17**.

**299 against board:0452's 2 Word layouts.** That contradiction was this item's first task.

**SETTLED by board:0576, and neither count was wrong.** The 2 is the LEGACY `WordLayout` property; the
current form is a `rendering` section of named layouts, and **`Type = Word` is declared 182 times**
there. So there are 184 Word layouts against 299 `WordMergeDataItem` declarations, and the remaining
gap is the property doing its job -- a report may name several data items for one layout.

**The legacy `WordLayout` property is effectively dead** (2 declarations) while `RDLCLayout` is still
768 against 641 in `rendering` sections, so the migration away from the legacy form is half done and
split by TYPE. board:0576 carries that measurement.

## The IST-state

Reports have no generator (board:0063, board:0034).

## The choice

A `DataItemId` and a bit on the report descriptor, and the bit again on the layout descriptor with the
layout's winning. Both feed board:0063's rendering path and neither affects the dataset.

**Behind the RDL route**, which is 768 of 771 layouts.

## Ordering

Inside board:0063, after the RDL path works.

## Gate, and its negative control

A report with `WordMergeDataItem` set to its header data item produces one document section per
header; an Excel layout declaring multiple data sheets produces one sheet per root data item.

**The negative control is the Excel report-level `false` with a layout-level `true`** -- the layout
must win, and an implementation reading only the report property produces a single `Data` sheet that
looks correct until the layout is opened.
