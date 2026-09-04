Type:     task
Status:   open
Parent:   0063
Area:     al, gen
Source:   developer/devenv-report-layout-declaration.md, developer/devenv-multiple-report-layouts.md, developer/devenv-report-design-overview.md, developer/devenv-reports.md, developer/devenv-reporting-options-overview.md
Verdict:  fehlt
Class:    activation

# A report declares its layouts twice over, and the legacy form is the majority

**Five pages, one item, and it CLOSES board:0457's open question.**

A report's layouts are declared one of two ways, and the documentation calls the first one legacy:

```AL
// legacy: three properties, at most one RDLC and one Word
report 50000 "Standard Report Layout"
{
    RDLCLayout = './StandardReportLayout.rdlc';
    WordLayout = './StandardReportLayout.docx';
    DefaultLayout = Word;
}

// current: a rendering section of NAMED layouts, and the default is chosen by NAME
report 50000 "Standard Report Layout"
{
    rendering
    {
        layout(LayoutExcelPivot)
        {
            Type = Excel;
            LayoutFile = 'EmpShownAsPivot.xlsx';
            Caption = 'ExcelPivot';
            Summary = 'Employee list shown in a pivot table';
        }
    }
}
```

## The measurement that closes board:0457

board:0457 found `WordMergeDataItem` at **299** against board:0452's **2** `WordLayout` declarations,
called the gap a partial resolution, and named the AL source as what would settle it.

**Measured 2026-09-04 over `~/Git/BCApps/src`:**

| | count |
|---|---:|
| `rendering` sections | **669** |
| `layout(` entries | **884** |
| `LayoutFile =` | 884 |
| layout `Type = RDLC` | **641** |
| layout `Type = Word` | **182** |
| layout `Type = Excel` | **61** |
| layout `Type = Custom` | **0** |

641 + 182 + 61 = 884, exactly the `layout(` count, so the census is complete.

**So there are 184 Word layouts -- 182 in `rendering` sections and 2 in the legacy property -- against
299 `WordMergeDataItem` declarations.** board:0457's hypothesis was right and its number was reading
the wrong half of the mechanism: **the legacy `WordLayout` property is effectively dead and the
`rendering` section carries the Word layouts.** The remaining gap between 184 and 299 is that a report
may declare several data items with `WordMergeDataItem` for one layout, which is what the property is
for.

## Both forms have to be supported, and the legacy one is the bigger half for RDLC

| legacy | count | current | count |
|---|---:|---|---:|
| `RDLCLayout =` | **768** | `Type = RDLC` | 641 |
| `WordLayout =` | **2** | `Type = Word` | 182 |
| `ExcelLayout =` | **1** | `Type = Excel` | 61 |
| `DefaultLayout =` | 730 | `DefaultRenderingLayout =` | 646 |

**The migration is half done and the halves are split by TYPE.** RDLC is still mostly legacy (768
against 641); Word and Excel have moved almost completely (2 and 1 against 182 and 61). **A transpiler
that supported only the recommended syntax would lose 768 RDLC layouts**, and one that supported only
the legacy syntax would lose 243 Word and Excel ones.

`DefaultRenderingLayout` at 646 against 669 `rendering` sections leaves **23 sections with no default**
-- consistent with the documented rule that the property *"can't be set on report extension objects,
only on report objects."*

## Four rules, and two of them are checks

**The `rendering` section's POSITION is fixed**: *"the new layout declaration moves the layouts to a
rendering section, which must be declared JUST AFTER THE REQUEST PAGE SECTION."* board:0549 has the
report's section order and board:0574 the page's; this pins one more edge of it.

**`MimeType` is only supported when `Type = Custom`** -- a conditional property of board:0573's family,
and **measured at 0 against `Custom` at 0**, so the two agree and the pair is currently unexercised.

**`Caption` and `Summary` fall back to the LAYOUT NAME**: *"the application will show the translated
caption and summaries to the end-user, with fallback to the layout name in case the caption is
undefined."* Both go into the XLIFF (board:0566), so a layout name is a user-visible string when the
caption is missing. `Summary` is 855 declarations against 884 layouts -- so 29 layouts fall back.

**A report extension's layout is NOT used automatically.** *"The layout in a report extension will not
automatically be used when the report extension is deployed. To use the report extension layout ... go
to the Report Layout Selection page."* So adding a layout by extension changes nothing until a user
selects it -- which makes the layout list per report a piece of TENANT DATA over the `constexpr` list,
not a replacement for it.

**And a report must have a layout unless it is processing-only**: *"a report that is viewed, printed,
or saved from a client must have a report layout."* `ProcessingOnly = true` is 761 (board:0557),
against 2 135 report objects.

## What each layout file IS

| type | the file |
|---|---|
| RDLC | an RDL definition, authored in Visual Studio Report Designer or Report Builder |
| Word | a `.docx` carrying a **custom XML part that represents the report dataset** |
| Excel | an `.xlsx`, with the dataset on its own sheets |
| Custom | anything, with a `MimeType`, rendered by an AL subscriber (board:0575) |

The rendering route for them is below, and it is one engine rather than three.

**The Word layout's mechanism is stated and is the one worth carrying**: the `.docx` contains a custom
XML part that IS the report dataset, so a Word layout is a merge between that part and the generated
dataset XML -- which is why board:0547 found the dataset schema to be a published artefact and why
`WordMergeDataItem` exists at 299.

`ExcelLayoutMultipleDataSheets` at **17** is a v26 property that puts each data item on its own sheet.

## The IST-state

- **No report is parsed** (board:0063), so neither form is read.
- **board:0452 owns the legacy properties** and measured `WordLayout` 2 without the other half.
- **board:0547 owns the layout files** and board:0572 how a file travels with an app.
- **`DefaultLayout` at 730 and `DefaultRenderingLayout` at 646 are two properties for one concept**,
  which nothing currently reconciles.

## The choice

**One `constexpr` list of layouts per report, and the legacy properties are DESUGARED into it by the
parser.**

```cpp
enum class LayoutType : std::uint8_t { RDLC, Word, Excel, Custom };
struct LayoutDef {
  std::string_view name;      // the legacy form synthesises one
  LayoutType type;
  std::string_view file;
  std::string_view caption;   // empty means: use name
  std::string_view summary;
};
```

**Why desugar rather than carry both shapes:** the two forms differ in surface and not in meaning --
`RDLCLayout = 'x.rdlc'` is `layout(<synthesised>) { Type = RDLC; LayoutFile = 'x.rdlc'; }`. Carrying
both would put the choice in every consumer; desugaring puts it in one place and makes
`DefaultLayout = Word` a lookup by type in the same list `DefaultRenderingLayout` looks up by name.

**The synthesised name has to be chosen and the choice is visible**: the legacy form has no name, and
a caption falling back to a name means the synthesised one can reach a user. **It is derived from the
type** -- `RDLC`, `Word`, `Excel` -- which is what BC's own UI shows for a legacy layout, rather than
from the file name, which would leak a path.

**Two `static_assert`s**: `MimeType` without `Type = Custom`, and `DefaultRenderingLayout` on a
`reportextension`. Both are documented refusals and both have a population of 0 today, **which is the
uncomfortable half** -- board:0573 records the same problem, and the answer is the same: the gate
cases are synthetic and the counter carries its denominator.

## Ordering

**Before board:0547's rendering work** -- there is nothing to render until the layout list exists --
and **after board:0549's report object**, which owns the section grammar this adds an edge to.

**RDLC first at 768 + 641**, Word second at 184, Excel third at 61. And the legacy form is not
optional: it is the majority of the RDLC half.

## Gate, and its negative control

1. a report with `RDLCLayout` and `WordLayout` and `DefaultLayout = Word` emits TWO `LayoutDef`s and
   marks the Word one default
2. a report with a `rendering` section of three named layouts emits three, and
   `DefaultRenderingLayout` picks by name
3. a layout with no `Caption` reports its NAME as the caption
4. `MimeType` without `Type = Custom` fails to transpile
5. `DefaultRenderingLayout` on a `reportextension` fails to transpile
6. a `rendering` section declared BEFORE the request page fails to transpile

**The negative control is case 1 against case 2.** Implement only the `rendering` section -- the
recommended syntax, and the one every current example shows -- and case 2 passes while case 1 emits
nothing. **768 RDLC layouts disappear and no report fails to translate**, because a report with no
layout is legal when it is processing-only and merely renders nothing when it is not. That is the
silent version of this defect and it is why case 1 is first.

## Class

`activation`. No report renders, so nothing regresses. **The measurement is the finding**: the
recommended syntax covers 884 layouts and the legacy syntax 771, and building only the recommended one
loses the larger half of RDLC without any error.

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
