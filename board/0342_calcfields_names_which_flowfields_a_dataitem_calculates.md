Type:     task
Status:   open
Parent:   0047
Area:     rt, gen
Source:   developer/properties/devenv-calcfields-property.md
Verdict:  fehlt
Class:    activation

# `CalcFields` names which FlowFields a data item calculates

> Sets a list of FlowFields to automatically calculate.
>
> Applies to: **Xml Port Table Element**, **Report Data Item**.
>
> The list of the FlowFields is the list of CalcFields that you can calculate. For that, the
> **AutoCalcField** property must be enabled.

So the pair works the other way round from board:0338's: the DATA ITEM lists candidates and the FIELD
switches each one on. Neither alone calculates anything -- a field with `AutoCalcField = true` whose
data item does not list it is not calculated, and a listed field with `AutoCalcField = false` is not
either.

That reading comes from the page's own sentence ("for that, the `AutoCalcField` property must be
enabled") and from nowhere else, and it is the kind of statement the AL source can confirm. **115
declarations is small enough to read all of them**, and this item does that before it builds.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`CalcFields =`: **115 declarations**, against 238 `AutoCalcField`.

## The IST-state

Nothing: no reports, no XMLports, no `CalcFields` body.

## The choice

A span of `FieldNo` on the data item, resolved by the generator, intersected with the fields'
`AutoCalcField` at translation time so the writer emits one list of field numbers to calculate per
row. Nothing about this needs to be decided while a report is running.

**A name in the list that is not a FlowField of the source table is a `static_assert`**, not a
run-time miss.

## Ordering

With board:0341; the two resolve into one emitted list.

## Gate, and its negative control

A report data item listing one of two FlowFields renders the listed one computed and the other empty.

**The negative control is the unlisted FlowField** -- an implementation that calculates every
FlowField of the source table passes the positive half and pays board:0045's cost on every row.
