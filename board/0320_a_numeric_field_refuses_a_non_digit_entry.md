Type:     task
Status:   open
Parent:   0068
Area:     rt, gen
Source:   developer/properties/devenv-numeric-property.md
Verdict:  fehlt
Class:    activation

# A `Numeric` field refuses a non-digit entry from the UI

> Sets a value that requires that users enter only numbers in the field.

`Numeric = true` on a **Text** or **Code** field, which is the only place it means anything -- an
Integer field is numeric by its type. It is the "this Code field holds digits" declaration, and the
page names `MinValue` and `MaxValue` as its relatives, so it belongs to the same check.

Same enforcement point as 0317-0319: "checked during validation ... If a field is updated through
application code, then the **Numeric** property is not validated."

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Numeric =`: **201 declarations** -- the smallest of the constraint properties by an order of
magnitude, and the reason this is a leaf and not a root.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`).

## The choice

One bit on `FieldDef` and one predicate at the input boundary. **"Numeric" is the AL sense and not
`std::isdigit`'s** -- the property is documented as "only numbers", and whether a sign, a space or a
decimal separator passes is a question the page does not answer. Read the AL source when this is
pulled: 201 declarations are few enough to look at all of them, and a Code field carrying `-` would
settle it.

## Ordering

With 0317; it shares the metadata extension and the boundary.

## Gate, and its negative control

Typing `A1` into a `Numeric` Code field through a `TestPage` raises; assigning it in AL does not.

**The negative control is the AL assignment.**
