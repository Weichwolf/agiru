Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-drilldown-property.md
Verdict:  fehlt
Class:    activation

# A page field with `DrillDown = false` refuses the drill-down its field offers

> Sets a drill-down for a field on a page. **True** if a drill-down for the field is provided;
> otherwise false. **The default value is false.**
>
> Drill-downs are a system-wide feature of **FlowFields** that let you see the underlying
> transactions that make up the information shown in the FlowField. For example, if the FlowField
> shows an account balance, then providing a drill-down lets the user quickly see the various
> transactions that make up the balance.

**This page and `DrillDownPageId`'s disagree about the scope**, and the disagreement is recorded
rather than resolved by preference: this one says drill-down is "a system-wide feature of FlowFields",
`devenv-drilldownpageid-property.md` says "of fields (normal fields and FlowFields)". Where the
documentation describes and the source declares, the source declares -- so the answer is in the AL,
and 1 530 declarations are enough to see whether any sits on a non-FlowField.

`DrillDown` is the control-side switch to `DrillDownPageId`'s target, the same pairing `Lookup` and
`LookupPageId` have (board:0336, board:0334).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DrillDown =`: **1 530 declarations**; the anchoring keeps `DrillDownPageID` out of the count.

## The IST-state

No control properties in the page metadata; nothing consumes it.

## The choice

The tri-state of board:0336, for the same reason: absent, declared `true` and declared `false` are
three different instructions and a `bool` can carry two.

## Ordering

With board:0336 and board:0335.

## Gate, and its negative control

A FlowField control with `DrillDown = false` renders no drill-down although its field declares a
`DrillDownPageId`; the same control without the property renders one.

**The negative control is the declared `false` over a field that offers one** -- an implementation
reading only the field's property renders the drill-down and passes every gate that does not declare
the refusal.
