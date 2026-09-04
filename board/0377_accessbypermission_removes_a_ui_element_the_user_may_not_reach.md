Type:     task
Status:   open
Parent:   0062
Area:     rt, gen
Source:   developer/properties/devenv-accessbypermission-property.md
Verdict:  fehlt
Class:    activation

# `AccessByPermission` removes a UI element the user may not reach

> Sets a value for a table field or UI element that determines the permission mask for an object that
> a user must have to see and access the related page fields or UI element. **The UI element will be
> removed at runtime** if the user does not have permissions to a certain object.
>
> All types of UI elements will be removed: **fields on pages, including FactBoxes; actions on pages,
> including ToolBars and navigation panes; page parts, such as Lines FastTabs.**
>
> Applies to: Table field, Page Field, Page Part, Page System Part, Page Chart Part, Page Action,
> Page Custom Action, Page File Upload Action, Page, Report.

**Removed, not disabled.** The element is absent from the rendered page, which for an htmx renderer
(CLAUDE.md) means it is never in the fragment -- and that is the right shape, because an element
merely hidden is still in the DOM and still reachable.

Three rules the page states and an implementation would otherwise get wrong:

1. **`X` is not valid for `tabledata`**, and `R`/`I`/`M`/`D` are not valid for anything else. "For
   other objects including Table, Page, Query, Report, Codeunit, or Xmlport, it can only be Execute."
   Both halves are decidable from the declaration, so both are `static_assert`s.
2. **On a field from a VIRTUAL table the property is ignored** -- "users will see this field on the
   page in the client even if they do not have the permissions". A documented exception, and
   board:0032 owns which tables those are.
3. **It only takes effect if the server's UI Elements Removal setting is `LicenseFile` or
   `LicenseFileAndUserPermissions`.** So it is configuration-dependent in BC, and agiru has to decide
   which setting it behaves as -- a decision, not a translation.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AccessByPermission =`: **10 211 declarations.**

**That is the second-largest property population in this sweep**, behind `TableRelation`'s 40 221 and
ahead of `DecimalPlaces`. Ten thousand UI elements in the BaseApp are conditional on a permission, so
a renderer that ignores the property shows every one of them to every user.

## The IST-state

board:0062: no permission check exists. Page control metadata does not exist either
(`src/gen/PageWriter.cpp` consumes `SourceTable` alone). So both halves are missing.

## The choice

The mask lands on the control as `constexpr` data, parsed by the generator the same way
board:0376's is -- and the two share that parser, because the value syntax is the same minus the
case distinction. The renderer omits the control when the check fails.

**The virtual-table exception is a `static_assert`-adjacent decision**: whether a field's source is a
virtual table is known at translation time, so the property can be dropped THERE rather than checked
at render time on 10 211 controls.

## Ordering

Behind board:0062 for the check and board:0030 for the control metadata. The `static_assert`s are
available before either.

## Gate, and its negative control

A control declaring `AccessByPermission = tabledata T = I` is absent from the fragment for a user
without insert on `T` and present for one with it.

**The negative control is the absence** -- checking that the control renders for a permitted user
proves nothing, since it renders for everyone today. The gate has to assert it is NOT in the
fragment.
