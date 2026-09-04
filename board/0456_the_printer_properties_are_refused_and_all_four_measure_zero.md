Type:     task
Status:   open
Parent:   0063
Area:     gen
Source:   developer/properties/devenv-papersourcedefaultpage-property.md, developer/properties/devenv-papersourcefirstpage-property.md, developer/properties/devenv-papersourcelastpage-property.md, developer/properties/devenv-pdffontembedding-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# The printer properties are refused, and all four measure zero

**Four pages, one item**: three paper-source properties with an identical ten-value enumeration, and
one PDF font switch. They are grouped because they take one decision on one piece of evidence -- a
population of zero -- and four files would repeat it.

> **PaperSourceDefaultPage / PaperSourceFirstPage / PaperSourceLastPage** (Report): `Upper`, `Lower`,
> `Middle`, `Manual`, `Envelope`, `ManualFeed`, `AutomaticFeed`, `TractorFeed`, `SmallFormat`,
> `LargeFormat` -- **printer bins.**
>
> **PdfFontEmbedding** (Report): `Default` (uses the server instance's **Report PDF Embedding**
> setting), `Yes`, `No`.

**The paper sources name physical printer trays** -- a tractor feed is a dot-matrix printer's
continuous paper. They belong to a print pipeline that talks to a printer driver, and agiru's
documented route is XSL-FO through Apache FOP to a PDF (CLAUDE.md), which has no bins.

`PdfFontEmbedding` is different in kind -- FOP does embed fonts and could honour it -- but its default
defers to a server-instance setting that does not exist here, and nobody declares it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`PaperSourceDefaultPage =` **0** · `PaperSourceFirstPage =` **0** · `PaperSourceLastPage =` **0** ·
`PdfFontEmbedding =` **0**.

**Four zeros.** Checked with the pattern that measures `Caption` at 288 491 on the same tree.

## The IST-state

Reports have no generator (board:0063, board:0034), and there is no print pipeline of any kind.

## The choice

**Refuse all four**, on the sweep's standing arithmetic for a zero population (board:0327,
board:0333, board:0346, board:0347, board:0361, board:0366, board:0394, board:0435, board:0439,
board:0447, board:0452).

**And record why the zero is not surprising**: BC's own reports do not select printer trays either,
because the paper source is a printer setting rather than a report one in every deployment since the
web client. That is the difference between a property nobody uses and a property nobody should use.

## Ordering

With board:0067's census. No runtime work.

## Gate, and its negative control

A report declaring any of the four fails to transpile.

**The negative control is the whole BaseApp transpiling with all four refusals in place** -- which is
what proves four zeros rather than trusting them, and this item has nothing else in it.
