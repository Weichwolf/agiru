Type:     task
Status:   open
Parent:   0047
Area:     rt, gen
Source:   developer/properties/devenv-autocalcfield-property.md
Verdict:  fehlt
Class:    activation

# `AutoCalcField` calculates a FlowField before it is read out

> Sets whether FlowFields should be automatically calculated. **True** if the FlowField is
> automatically calculated; otherwise false. **The default is true.**
>
> Applies to: **Xml Port Field Attribute**, **Xml Port Field Element**, **Report Column**.

A FlowField is empty until something calls `CalcFields` (board:0047). On a page the platform does it;
on an XMLport and in a report the property decides, and **its default is `true`** -- so an export or a
report reads the computed value unless someone switched it off.

`AutoCalcField = false` is therefore a PERFORMANCE declaration: 238 places in the BaseApp say "do not
run this query for every row of this export". Under board:0045's 100-million-row table that is the
difference between a report and a table scan per line.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AutoCalcField =`: **238 declarations**, every one of them a deliberate `false`, since `true` is the
default and needs no writing.

## The IST-state

Neither XMLports nor reports are generated (board:0065, board:0063, board:0034), and `CalcFields`
itself has no body (board:0047). There is nothing to switch.

## The choice

One bit per XMLport field and report column, defaulting to `true`, resolved by the generator together
with `CalcFields` (board:0342) -- the port's list and the field's switch are read once and what the
writer emits per field is the answer.

## Ordering

Inside board:0063 and board:0065; behind board:0047, which has to be able to calculate at all.

## Gate, and its negative control

An export of a table with a `Sum` FlowField writes the computed total by default and the empty value
when the field declares `AutoCalcField = false`.

**The negative control is the default** -- an implementation that only acts on the declared `false`
leaves every other FlowField uncalculated and exports zeros, which looks like a formula defect.
