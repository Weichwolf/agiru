Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-lookuppageid-property.md
Verdict:  fehlt
Class:    activation

# `LookupPageId` names the page a field looks up through

> Sets the ID of the page you want to use as a lookup. By default, a lookup provides a list of
> records in the table.
>
> **NOTE:** Consider creating dedicated lookup pages instead of standard pages. **Default list pages
> run all triggers and fact boxes even if they aren't shown in the lookup.**

Two statements, and the second is the one with teeth. It is on the TABLE as well as on a page field,
so a table declares its own default lookup and a page field may override it -- the same
field-versus-control override `ExtendedDataType` has (board:0329).

And the note is a performance fact this tree can act on rather than advise about: a lookup that opens
a full list page runs `OnOpenPage`, every `OnAfterGetRecord` and every FactBox part. Under
board:0006's per-session budget that is measurable, and the documentation says outright that BC pays
it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`LookupPageId =`: **2 294 declarations**, counted case-insensitively -- AL writes `LookupPageID` far
more often than the page's own `LookupPageId`.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`) and not in the table's own metadata; the generator
consumes no page-id property at all.

## The choice

A `PageId` on the table and on the field, strong-typed, resolved by the generator so a
`LookupPageId` naming a page nobody declared is a translation error. Absent means "the target's list
page", which needs the relation's target from board:0331 -- so this item cannot land before that one.

## Ordering

Behind board:0331 for the default, and behind board:0030 for anything to open.

## Gate, and its negative control

A field with `LookupPageId` opens that page; the same field without it opens the relation target's
list page.

**The negative control is the absent property** -- an implementation that only honours the explicit
id leaves 37 927 relations with no lookup at all.
