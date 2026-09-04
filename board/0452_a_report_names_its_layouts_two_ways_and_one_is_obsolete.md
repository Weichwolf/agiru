Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/properties/devenv-rdlclayout-property.md, developer/properties/devenv-wordlayout-property.md, developer/properties/devenv-excellayout-property.md, developer/properties/devenv-layoutfile-property.md, developer/properties/devenv-defaultlayout-property.md, developer/properties/devenv-defaultrenderinglayout-property.md, developer/properties/devenv-sharedlayout-property.md, developer/properties/devenv-clearlayout-property.md
Verdict:  fehlt
Class:    activation

# A report names its layouts two ways, and the older way is obsolete

**Eight pages, one item**: they are one mechanism -- how a report finds its layout files -- expressed
in two generations, and the pages cross-reference each other. Splitting them would hide the fact that
three of them are the OLD form of the other five.

**The old form**, one property per layout type on the report itself:

> **RDLCLayout / WordLayout / ExcelLayout**: sets the layout used on a report and returns it as a data
> stream. **"The RDL file must be in the same folder as the AL object."** Each page carries the shared
> note `include-single-layout-obsolete.md`.
>
> **DefaultLayout**: `RDLC`, `Word` or `Excel` -- which built-in layout is used by default.

**The new form**, a `rendering` section with several named layouts:

> **LayoutFile** (Report Layout): the filename of the layout file that should be imported with this
> layout.
>
> **DefaultRenderingLayout**: the default layout for this report, used **together with a `rendering`
> section**.
>
> **SharedLayout**, **ClearLayout**: layout composition and removal in a report extension.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`RDLCLayout =` **768** · `LayoutFile =` **884** · `DefaultLayout =` **730** ·
`DefaultRenderingLayout =` **646** · `ClearLayout =` **33** · `WordLayout =` **2** ·
`ExcelLayout =` **1** · `ExcelLayoutMultipleDataSheets =` **17** · `SharedLayout =` **0**.

**Both generations are live at once**: 768 old-form RDLC declarations against 884 new-form
`LayoutFile`s, so a report generator must read both. And **the BaseApp is RDLC** -- 2 Word layouts and
1 Excel in 668 reports.

That last number decides board:0063's order: CLAUDE.md already commits to XSL-FO through Apache FOP,
and 768 of 771 layouts are RDL, so the RDL-to-XSL-FO route is the whole job and Word and Excel are
three files.

`SharedLayout` measures **0**.

## The IST-state

Reports have no generator (board:0063, board:0034).

## The choice

One layout list per report, built by the generator from BOTH forms -- old-form properties become
single-entry renderings -- so the report descriptor has one shape and the two generations are
reconciled at translation time.

**The layout FILES are an artefact question this item names and does not answer**: the `.rdl` sits
beside the `.al` in BCApps, and whether the transpiler copies it, references it, or converts it at
translation time is board:0063's decision. What this item delivers is that every report knows which
files are its own.

`SharedLayout` is refused on its zero.

## Ordering

Inside board:0063, before any rendering: a report cannot render without knowing its layout.

## Gate, and its negative control

A report declaring `RDLCLayout` and one declaring a `rendering` section with a `LayoutFile` both
produce a descriptor naming one layout of type RDL.

**The negative control is a report declaring BOTH** -- a report extension adding a rendering to a
report that already has an old-form layout. The two must compose rather than one replacing the other,
and an implementation that reads only the newer form silently drops 768 layouts.
