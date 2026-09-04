Type:     task
Status:   open
Parent:   0043
Area:     rt, gen
Source:   developer/properties/devenv-validatetablerelation-property.md
Verdict:  fehlt
Class:    activation

# `ValidateTableRelation = false` lets a value through, and changes what the lookup does

> Sets whether to validate a table relationship. **The default is true.**
>
> If you want to let users enter any value without validating the entry, then choose **false**. For
> example, on the **Item** card you can specify the vendor you typically purchase from. Set this to
> **false** to allow users to select a vendor that may not already be in the Vendor table. It will
> then be up to the field's **OnValidate trigger** to process what the user has typed, for example
> create a new vendor with that name.

So the field keeps its `TableRelation` -- the lookup still lists `Vendor` -- and only the REFUSAL is
switched off. A relation with `ValidateTableRelation = false` is a suggestion.

**And the property changes the lookup's behaviour, which the page describes precisely:**

| | `true` | `false` |
|---|---|---|
| as the user types | the best match is auto-selected in the lookup | nothing is selected, focus stays in the field |
| Tab / Enter | saves the selected entry | saves whatever the user typed |
| no match, Tab / Enter | **validation error**, the empty lookup stays open | the lookup closes, free text is kept |

> The behavior of the lookup when set to **false** was changed in Business Central 2020 release
> wave 1 (**runtime 9.0**). In earlier versions ... focus switches to an entry in the lookup, and the
> lookup stays open even if there's no match.

The pre-9.0 behaviour is named so it is not implemented by accident from an older sample.

> If you set `ValidateTableRelation` to false, then you should also set `TestTableRelation` to false
> (board:0333). Otherwise, a database test on the field relations may fail.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ValidateTableRelation =`: **2 240 declarations**, against 40 221 `TableRelation`s. So 5.6 % of
relations are declared non-validating -- and since the default is `true`, every one of those 2 240 is
a deliberate `false`.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`); `CheckRelation` is an empty body
(`src/rt/Table.cpp:350`), so today every relation behaves as `false` by accident.

**That is the trap in this item.** Building board:0331 and board:0043 without this one turns 2 240
fields from working into refusing, and the failure looks like a relation defect rather than a missing
property. It is the reason both are classified `activation` and take an A/B.

## The choice

One bit on `FieldDef`, defaulting to `true`, read by `CheckRelation` before it does anything. The
lookup half belongs to board:0030's renderer and reads the same bit.

## Ordering

**With board:0331, not after it.** A relation check that lands without this property is a regression
on 2 240 fields.

## Gate, and its negative control

A field with `ValidateTableRelation = false` accepts a value the target table does not hold and its
`OnValidate` still runs; the same field with the property absent refuses it.

**The negative control is the absent property** -- an implementation that reads the bit but defaults
it to `false` passes the first half and validates nothing anywhere, which is exactly today's state.
