Type:     task
Status:   open
Parent:   0068
Area:     rt, gen
Source:   developer/properties/devenv-valuesallowed-property.md
Verdict:  fehlt
Class:    activation

# A `ValuesAllowed` field refuses a value that is not on its list

> Sets the values that are allowed in the field. Separate the values with a comma. For example, if
> you only want the user to enter 1, 3, or 5 in this field, enter 1,3,5 for this property.

The page's own syntax line is `ValuesAllowed = Codeunit, Page, Query;` -- so the list is not
necessarily numeric: on an Option or Enum field it is a list of MEMBER NAMES, and those resolve to
ordinals where the enumeration is in scope, exactly as `initValue` already does
(`include/meta/TableDef.h:86`).

Same UI-only enforcement point as the rest of this group.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ValuesAllowed =`: **46 declarations** -- the smallest population in the group.

## The IST-state

Not in `FieldDef` (`include/meta/TableDef.h:67`).

## The choice

A `std::span<const ...>` of allowed values on `FieldDef`, resolved by the GENERATOR the way
`initValue`'s member name is, and never by a lookup at run time. That is the whole reason it can be
a member-name list at all.

**And the check is a subset test against `values`**, which the field already carries for an Option --
so a `ValuesAllowed` naming a member that is not in the enumeration is a `static_assert`, not a
run-time miss.

## Ordering

With 0317. Behind board:0049 where the field is an Option, because the member resolution is that
item's.

## Gate, and its negative control

Typing an off-list value through a `TestPage` raises; an on-list one does not; an AL assignment of
an off-list value succeeds.

**The negative control is a `ValuesAllowed` naming a member the enum does not have** -- it must fail
to compile, or the list silently allows nothing.
