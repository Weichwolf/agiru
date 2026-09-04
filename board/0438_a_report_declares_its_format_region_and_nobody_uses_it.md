Type:     task
Status:   open
Parent:   0066
Area:     gen, rt
Source:   developer/properties/devenv-formatregion-property.md, developer/properties/devenv-culture-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# A report declares the region its numbers and dates are formatted in

**Two pages, one item**: both name a locale for a report's rendering, one as a format region and one
as a culture, and they are the same decision made twice on the same object kind.

> **FormatRegion** (runtime 11.0, **Report**): Sets the format region used when formatting **numbers
> and date/time values**. **Based on RFC 4646**, `languagecode2-country/regioncode2` -- `ja-JP`,
> `en-US`. Where a two-letter language code is not available, **a three-letter ISO 639-3 code** is
> used.
>
> **Culture**: the culture for the report.

**This collides with a rule the tree already holds.** board:0041 fixes case conversion to the
invariant culture, and CLAUDE.md's determinism invariant says the same posting produces the same
entries twice. A per-report format region is a per-report locale, which is fine for RENDERING and
would be a defect anywhere near a stored value -- so the boundary between the two has to be explicit,
and this item is where it is stated.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`FormatRegion =` **0** · `Culture =` **47**.

**`FormatRegion` is never declared** in the BaseApp, and `Culture` 47 times. So BC's own reports run
in the session's region, and the override is nearly unused.

## The IST-state

Reports have no generator (board:0063, board:0034). board:0066's format engine has no locale input at
all; board:0007's Decimal formatting is one format.

## The choice

`FormatRegion` is **refused**, on its zero -- the sweep's standing arithmetic (board:0327,
board:0333, board:0346, board:0347, board:0361, board:0366, board:0394, board:0435). `Culture` is
carried on the report descriptor at 47 declarations and consulted by the renderer only.

**And the boundary is stated in the item rather than left to whoever implements it**: a format region
reaches board:0066's RENDERING path and never `Evaluate`, never a stored Decimal, never a key. A
locale that reached a comparison would make a sort order depend on a report property.

## Ordering

Behind board:0063's report generator and board:0066's format engine.

## Gate, and its negative control

A report declaring `Culture = 'ja-JP'` renders its dates in that culture; a table declaring
`FormatRegion` fails to transpile.

**The negative control is a stored value** -- the same posting run under two cultures must produce
byte-identical entries, which is CLAUDE.md's determinism invariant and the thing a per-report locale
could break.
