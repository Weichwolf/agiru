Type:     task
Status:   open
Parent:   0066
Area:     net, rt
Source:   developer/properties/devenv-blanknumbers-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `BlankNumbers` clears a number by its sign when the field is formatted

> Indicates whether the system will **clear a range of numbers as it formats them**.

**This is a FORMAT property and not a constraint**, which is why it sits under board:0066 and not
under board:0068 with its neighbours. Nothing is refused; the value stays in the row and the
rendered text is empty.

Six values, and they are a sign predicate rather than a flag:

| value | blanks |
|---|---|
| `DontBlank` | nothing (default) |
| `BlankNeg` | `< 0` |
| `BlankNegAndZero` | `<= 0` |
| `BlankZero` | `= 0` |
| `BlankZeroAndPos` | `>= 0` |
| `BlankPos` | `> 0` |

`BlankZero` is also a property of its own (board:0324), and the two overlap exactly on one of the
six -- the page pair says nothing about which wins when both are declared.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`BlankNumbers =`: **155 declarations**, against 5 104 for `BlankZero`. The general property is rare
and its one special case is 33 times more common.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`), and `Format` has no field-property input at all.

## The choice

An enumerator on `FieldDef` and a predicate consulted where a FIELD is rendered -- not inside
`Format(Decimal)`, which is given a value and not a field. That distinction is the item: the
property belongs to the field's rendering path, and putting it in `Format` would blank a number
whose field never declared it.

## Ordering

Behind board:0066's format engine, which is where the rendering path is.

## Gate, and its negative control

A field with `BlankNumbers = BlankNegAndZero` renders `-1` and `0` as the empty string and `1` as
`1`; the stored value is unchanged and a `Get` reads it back.

**The negative control is the round trip** -- an implementation that blanks the VALUE rather than
its rendering loses data, and the gate has to be able to see the difference.
