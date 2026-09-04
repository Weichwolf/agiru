Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-tooltip-property.md
Verdict:  fehlt
Class:    activation

# A tooltip is declared metadata, and the renderer reads it

> Sets the string used for the tooltip of an action, a field, a FactBox, or an activity button. **In
> the client, tooltips appear when you point to the caption of the control.**
>
> Applies to: Page Label, Page Field, Page Part, Page System Part, Page Chart Part, Page Action Area,
> Page Action, Page Action Group, Page Custom Action, Page System Action, Page File Upload Action,
> Page Analysis View, **Report**.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ToolTip =`: **159 993 declarations.**

**The second-largest population in this sweep**, behind `Caption`'s 288 491 and four times
`TableRelation`'s. Together the two account for 448 484 declarations -- roughly one caption or tooltip
for every six lines of AL in the BaseApp.

That number decides the representation before anything else does: 160 000 strings that are known at
translation time belong in `.rodata` as `constexpr` `string_view`s, demand-paged and shared. Built at
startup they would be the gigabyte per process CLAUDE.md left Python to avoid.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no control metadata, so no tooltip.

## The choice

A `string_view` on each control's `constexpr` descriptor, beside its caption -- one array per page in
`.rodata`, nothing per session, nothing at startup. The renderer emits it as the control's `title`
attribute.

**Not a lookup table keyed by control name.** 160 000 entries in a map is the shape this tree
refuses; the control's descriptor holds its own string and the array index IS the key.

## Ordering

With board:0030's control metadata. It is one member on a struct that has to exist anyway, so it
costs nothing to include from the start and a migration to add later.

## Gate, and its negative control

A control declaring a tooltip renders it; a control declaring none renders no tooltip attribute.

**The negative control is the absent tooltip** -- an implementation that falls back to the caption
produces a tooltip on every control, which looks right in a screenshot and is not what BC does.

## THE TWO LEVELS, MEASURED

`devenv-adding-tooltips.md` (read 2026-09-04, routed here) states the fallback and dates it:

> "Starting in 2024 release wave 1, you can define tooltips on TABLE FIELDS. **When a tooltip is
> defined on a table field, ANY PAGE THAT USES THE FIELD AUTOMATICALLY INHERITS THE TOOLTIP** ... but
> if you define a tooltip on the page field, then THIS version of the tooltip will be displayed."

So the resolution is: the control's own `ToolTip`, else the source field's, else nothing. board:0545
recorded the chain; **this is its size.** Measured 2026-09-04 over `~/Git/BCApps/src` by file
extension:

| where | count |
|---|---:|
| `*.Table.al` | **50 325** |
| `*.Page.al` | 92 903 |
| everywhere (the number this item was filed with) | 159 993 |

The remaining 16 765 are in extensions, reports and XMLports, which the extension split does not
separate -- **stated rather than apportioned**.

**50 325 table-field tooltips are the FALLBACK SOURCE**, and a renderer that reads only the control's
own property shows nothing for every page field that relies on inheritance. Before 2024 wave 1 the
inheritance did not exist and "you need to DUPLICATE the code for the definition of the tooltip" on
every page -- so the 92 903 page-level tooltips are partly that older duplication, and the two numbers
are not disjoint in meaning even though they are disjoint in count.
