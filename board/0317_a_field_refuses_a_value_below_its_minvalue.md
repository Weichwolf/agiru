Type:     task
Status:   open
Parent:   0068
Area:     rt, gen
Source:   developer/properties/devenv-minvalue-property.md
Verdict:  fehlt
Class:    activation

# A field refuses a value below its `MinValue`, but only when the value came from the UI

`MinValue` "sets the minimum numeric value for a field", and the page tabulates the type it applies
to: Integer, Decimal, Date and Time -- with `January 1, 0` and `00:00:00` as the Date and Time
floors, so it is not a numeric-only property.

**And the enforcement point is narrower than it looks.** The sibling pages that share this
property's wording -- `NotBlank` (0319), `Numeric` (0320) -- say it outright: "This setting is
evaluated for controls and fields **during validation**. Validation occurs only if the field or
control value is updated **through the UI** ... **If a field is updated through application code,
then the property is not validated.**"

So `Rec."Line No." := -1` from AL is legal and the same value typed on a page is not. A runtime that
enforced the bound on every assignment would refuse code the BaseApp writes.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`MinValue =`: **3 398 declarations.**

## The IST-state

`include/meta/TableDef.h:67` -- `FieldDef` carries `no`, `name`, `caption`, `type`, `length`,
`offset`, `values` and `initValue`, and nothing else. **`MinValue` is not in the metadata at all**,
so neither the value nor the check exists.

## The choice

`FieldDef` gains an optional bound, emitted by the generator as the column spells it -- the same
shape `initValue` already has, including its "empty is not absent" rule.

The check lives at the UI-input boundary (board:0030) and NOT in `Table<Derived>`'s assignment path.
**That is the item's whole point**: putting it in the assignment is the obvious place and the wrong
one.

**What the page does not settle, and it is said rather than guessed**: whether `Rec.Validate(field,
value)` counts as UI or as application code. `Validate` is what a page calls, so it probably does --
but the page says "through the UI", not "through Validate", and the BaseApp calls `Validate` from
code constantly. The answer decides whether 3 398 bounds fire on 82 344 `Validate` call sites, and
it is looked up in the AL source when this item is pulled.

## Ordering

Behind board:0068's metadata, which is the same field-table extension every sibling in this group
needs. Before board:0030's input path, which is where the check goes.

## Gate, and its negative control

Typing a value below the bound through a `TestPage` raises; assigning the same value in AL does not.

**The negative control is the assignment** -- a check in `Table<Derived>` refuses both and breaks
BaseApp code that legitimately assigns out-of-range intermediates.
