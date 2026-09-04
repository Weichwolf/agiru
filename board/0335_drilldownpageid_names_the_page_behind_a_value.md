Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-drilldownpageid-property.md
Verdict:  fehlt
Class:    activation

# `DrillDownPageId` names the page behind a value

> Sets the ID of the page to use as a drill-down.
>
> Drill-downs are a system-wide feature of fields (**normal fields and FlowFields**) that let you see
> the underlying transactions that make up the information shown in the field. The `DrillDownID`
> property is typically used to create a link from a **Cue** to an underlying page.

"Normal fields and FlowFields" is the correction to make here: the `DrillDown` page property
(board:0337) describes drill-down as a FlowField feature, and this page says it is not. A drill-down
on an ordinary field is legal.

**And the Cue is the real consumer.** A role centre's cue is a FlowField count whose drill-down opens
the filtered list behind the number -- that is the whole interaction, and CLAUDE.md names role
centres with cues as a core expectation.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DrillDownPageId =`: **1 822 declarations**, counted case-insensitively.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`), not on the table.

## The choice

A `PageId` on the table and on the field, exactly as board:0334's, and the same generator-side
resolution. The two properties are siblings and should be one piece of metadata work; they are two
items because their consumers differ -- a lookup writes a value back into the field, a drill-down does
not.

**The filter is the part to get right.** Opening the drill-down page is not enough: it opens FILTERED
to the rows behind this value, and for a FlowField those rows are the `CalcFormula`'s. So this item
is behind board:0047, which computes the FlowField, and not merely behind board:0030.

## Ordering

Behind board:0047 for a FlowField's rows; the metadata half goes with board:0334.

## Gate, and its negative control

A cue's drill-down opens its page filtered to the same rows the FlowField counted, and the count on
the cue equals the row count on the page.

**The negative control is the equality** -- a drill-down that opens the page unfiltered still opens a
page, and only comparing the two numbers catches it.
