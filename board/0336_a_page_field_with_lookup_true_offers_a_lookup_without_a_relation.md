Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-lookup-property.md
Verdict:  fehlt
Class:    activation

# A page field with `Lookup = true` offers a lookup where the field declares no relation

> Specifies if a page field has a lookup window. **True** if you want a lookup for the field;
> otherwise false. **The default value is false.**
>
> By default, a lookup provides a list of records in the table.

The property is on a PAGE FIELD only -- there is no table half -- and it is the switch, where
`LookupPageId` (board:0334) is the target. A field whose source has a `TableRelation` already gets its
lookup from the relation; `Lookup = true` is how a control gets one anyway, and `Lookup = false` is
how a control with a relation refuses one.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Lookup =`: **497 declarations**; the anchoring keeps `LookupPageID` out of the count.

## The IST-state

The page metadata carries no control properties beyond what board:0030 has built; the generator
consumes `SourceTable` on a page (`src/gen/PageWriter.cpp`) and nothing on a control.

## The choice

One tri-state on the control -- declared `true`, declared `false`, absent -- and not a `bool`.
Absent means "whatever the relation says", and a `bool` defaulting to `false` would silence 40 221
relations' lookups; a `bool` defaulting to `true` would give one to every field. The distinction
between "absent" and "declared false" is the property, exactly as it is for `initValue`
(`include/meta/TableDef.h:97`, "empty is not absent").

## Ordering

Behind board:0030's control metadata; with board:0334.

## Gate, and its negative control

A control with `Lookup = true` over a field with no `TableRelation` offers a lookup; a control with
`Lookup = false` over a field WITH a relation does not; a control declaring neither follows the
relation.

**The negative control is the third case** -- a two-state implementation gets one of the first two
right and the third wrong, and only a gate with all three sees it.
