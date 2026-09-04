Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/devenv-howto-report-layout.md, developer/devenv-howto-rdl-report-layout.md, developer/devenv-howto-excel-report-layout.md, developer/devenv-using-word-to-author-your-report-layout.md, developer/devenv-hyperlinks-in-word-report-layouts.md, developer/word-layout-add-in.md, developer/devenv-format-report-field-data.md
Verdict:  fehlt
Class:    activation

# A report layout is authored outside AL, and two system data items carry metadata

**Seven pages, one item**: the three per-format layout how-tos, the Word authoring guide, the Word
add-in, the hyperlink page and the field-formatting page. They describe one thing from the LAYOUT side
-- what the file outside the `.al` contains and what the dataset must offer it.

board:0452 filed the layout properties and measured **768 RDLC layouts against 2 Word and 1 Excel**.
These pages are what those files are.

## Two system data items exist only in the layout, not in the dataset

> "metadata is available in the **XML Mapping** pane as columns in **two system dataitems:
> `ReportMetadata` and `ReportRequest`**."
>
> **"These two dataitems AREN'T PART OF THE REPORT DATASET but are only present in the LAYOUT XML."**
>
> **ReportMetadata**: `ExtensionID`, `ExtensionName`, `ExtensionPublisher`, `ExtensionVersion`,
> `ReportID`, `ReportName`, **`AboutThisReportTitle`**, **`AboutThisReportText`**, `ReportHelpLink`.
>
> **ReportRequest**: metadata "from the report request (the report invocation that created the
> document)", beginning with `TenantEntraId`.

**So the rendered document has access to fields the dataset does not contain**, supplied by the
platform at render time. That is a second data source into the layout, and an implementation that fed
the layout only the dataset would leave nine named columns empty.

**Three of them come from other items**: `AboutThisReportTitle` and `AboutThisReportText` are
board:0388's teaching-tip properties on the request page; `ReportHelpLink` is board:0393's.
**`ExtensionID`/`Name`/`Publisher`/`Version` come from `app.json`**, which board:0393 also needs and
which no item yet confirms the transpiler reads.

> **"Using metadata columns from system data items lets you SKIP ADDING COMMON FIELDS to the report
> dataset, such as company name and user name."**

## The layout file is generated, then edited outside AL

> To create a Word layout: add a layout entry of type Word, set `LayoutFile`, **"now BUILD THE
> EXTENSION to GENERATE the Word file that includes the CUSTOM XML PART."**

**The build produces the layout file's skeleton** -- a `.docx` carrying a custom XML part that mirrors
the dataset -- and a human then edits it in Word. So the dataset's SHAPE is a published contract in two
directions: board:0396's `IncludeCaption` adds `<column>Caption` entries to it, and the layout binds to
those names.

**That makes the dataset schema an artefact the generator must emit**, not just a runtime structure --
and it is the piece CLAUDE.md's XSL-FO route needs anyway, since an RDL layout is bound to the same
schema.

## What this means for the XSL-FO route

CLAUDE.md commits to **XSL-FO through Apache FOP**. board:0452 measured that **768 of 771 layouts are
RDL**, so the conversion is RDL -> XSL-FO, and these pages say what the RDL is bound to: the dataset
schema plus the two system data items.

**`devenv-format-report-field-data.md`** is board:0491's `AutoFormat` applied to a report column, so the
rendered value's format comes from the same resolver -- **the layout receives formatted text, not raw
values**, which is what makes a report's numbers match a page's.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0452: `RDLCLayout` **768**, `LayoutFile` **884**, `WordLayout` **2**, `ExcelLayout` **1**.
board:0457: `WordMergeDataItem` **299** -- which board:0457 records as contradicting the 2, and this
page's "build generates the Word file" is a possible explanation: a layout may exist as a file without
a `WordLayout` property, added through the `rendering` section instead.

**That is a partial resolution of board:0457's open contradiction and is recorded as one**, not as an
answer -- the AL source still settles it.

## The IST-state

board:0063: no report generator, no dataset, no renderer. Whether the transpiler reads `app.json` for
the four extension metadata columns is unmeasured and is named here.

## The choice

The generator emits the dataset SCHEMA beside the report -- a `constexpr` description of data items,
columns and `IncludeCaption` entries -- and the renderer supplies the two system data items from the
session and `app.json`.

**The layout files themselves are artefacts, not generated code**: they live beside the `.al` in
BCApps and are copied or referenced, which board:0452 names as its open question.

## Ordering

Inside board:0063, after board:0451's data items and board:0436's `ProcessingOnly` path. Behind
board:0491 for the value formatting.

## Gate, and its negative control

A rendered report's `ReportMetadata.ReportName` column carries the report's object name and
`ReportRequest` carries the invocation's data, neither of which is in the dataset; a column with
`IncludeCaption` appears in the schema as `<name>Caption`.

**The negative control is a layout referencing a metadata column** -- it must render, and an
implementation that feeds the layout only the dataset leaves it blank, which no dataset-level assertion
can see.

## THE RENDERING ROUTE IS XSL-FO THROUGH APACHE FOP

**A layout is TRANSLATED, not interpreted.** CLAUDE.md names the route -- *"Reporting is XSL-FO
through Apache FOP to PDF, the route the predecessor takes and the one BC's own RDL layouts translate
into most directly"* -- and it decides what a layout type MEANS here:

| declared `Type` | count | what agiru does with it |
|---|---:|---|
| `RDLC` | 641 + 768 legacy | **translate the RDL to XSL-FO**, then Apache FOP to PDF |
| `Word` | 182 + 2 legacy | **translate the `.docx` to XSL-FO**, then Apache FOP to PDF |
| `Excel` | 61 + 1 legacy | **not a print route at all** -- an `.xlsx` is written, not rendered |
| `Custom` | 0 | board:0575's `OnCustomDocumentMergerEx`, an AL subscriber |

**So there is ONE rendering engine and two translators into it**, and the translation is the work: an
RDL definition and a Word document are two descriptions of a page, and XSL-FO is the third that FOP
can print. The alternative -- one renderer per layout type -- is three page engines instead of one,
which is the shape CLAUDE.md rejects for the same reason it rejects a template beside a model.

**The Excel row is deliberately different and is stated rather than assumed.** An Excel layout's
output is a spreadsheet, not a printed page: the documentation calls Excel *"used for analytical
reports"* against Word for documents. FOP produces PDF and PostScript, not `.xlsx`. **Whether an Excel
layout should ALSO have a PDF route is not settled by the documentation** -- BC's request page offers
`Download` for Excel and `Print`/`Preview` for RDLC and Word (board:0557's button table), which says
Excel is not printed -- and that button table is the evidence, not an inference from the format.

**Apache FOP is a JUSTIFIED dependency in CLAUDE.md's terms**: it replaces writing a page-layout and
PDF engine, PDF is one of the four things this tree does not write from scratch, and FOP is Java --
reachable on every architecture this builds for, which is the second half of the requirement. **The
cost is a JVM in the process picture**, and that is the thing to weigh rather than the library.
