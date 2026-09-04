Type:     task
Status:   open
Parent:   0066
Area:     net, rt
Source:   developer/properties/devenv-blankzero-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `BlankZero` renders a zero as nothing, and a `false` with it

> Indicates whether the system displays zeros (0) and No. **True** if zeros and No are not
> displayed; otherwise, **false**.

**"and No"** is the half that is easy to miss: the property covers Boolean `false` as well as
numeric zero, so it is not `BlankNumbers = BlankZero` restricted to numbers. That the two pages
otherwise describe the same effect is what makes the interaction worth stating (board:0323).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`BlankZero =`: **5 104 declarations.** It is the most-declared property in this whole group, ahead of
`MinValue`'s 3 398 -- an amount column that shows nothing instead of `0.00` is BC's default look, and
5 104 fields say so.

The first measurement said 5 918. The extra 814 are `BlankZero="false"` inside RDLC layout XML
embedded in report files -- a layout attribute, not an AL property.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`).

## The choice

One bit on `FieldDef`, consulted on the field's rendering path with `BlankNumbers`. Given the
population, this is the one in the group that pays for itself: 5 104 fields render a visible
difference and a report or a page that shows `0,00` everywhere is wrong on every line.

**What the pages do not settle**: `BlankZero = true` together with `BlankNumbers = BlankPos`. Both
are declarable and the documentation never pairs them; the AL source decides, and if no field
declares both then the answer is that the case does not arise -- which is a measurement, not a
guess, and it belongs in this item when it is pulled.

## Ordering

Ahead of board:0323 on population, behind board:0066's engine.

## Gate, and its negative control

A `BlankZero` Decimal field renders `0` as the empty string and `0.01` as `0.01`; a `BlankZero`
Boolean renders `false` as the empty string.

**The negative control is the Boolean** -- an implementation that reads the page's title and handles
only numbers passes a numeric gate and is wrong on "and No".
