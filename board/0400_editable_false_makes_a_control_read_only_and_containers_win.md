Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-editable-property.md
Verdict:  fehlt
Class:    activation

# `Editable = false` makes a control read-only, and the container wins

> Sets a value that indicates whether a field, page, or control can be edited **through the UI**. The
> default is **true**. Applies to: **Table field**, Page, Request Page, and every page container and
> control.
>
> **For controls, if the `Editable` property for the CONTAINER that contains this control is set to
> false, then that setting OVERRIDES what you enter here.** If a page has `Editable` set to false,
> then the controls on the page aren't editable, **even if the individual `Editable` properties are
> set to true**.
>
> The property setting is checked during validation. **Validation occurs only if the field or control
> value is updated through the UI ... If a field is updated through application code, then the
> `Editable` property isn't validated.**
>
> **NOTE:** When using `CurrPage.Editable`, the property also reflects **the page MODE the page was
> opened in**. This applies to Edit, Create and Delete modes, **but not to View mode**.

Three rules and each is a separate way to get it wrong:

1. **The container wins, and it wins DOWNWARD only.** A `false` container makes every descendant
   read-only regardless of their own declaration; a `true` container does not make a `false` child
   editable. So it is an AND down the tree and not an override.
2. **It is the same UI-only enforcement as board:0317's constraint family** -- an AL assignment to a
   non-editable field is legal, which is why this is `activation` and its negative control is the
   assignment.
3. **`CurrPage.Editable` mixes the declaration with the page MODE**, and View mode is excluded from
   that mixing by name. So the value AL reads is not the declared property.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Editable =`: **51 886 declarations.**

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; `include/meta/TableDef.h:67` has no editable
bit on a field either, so both halves are missing.

## The choice

One bit per control and per field, and **the AND down the container tree is computed at RENDER time
and not folded by the generator** -- because a container's `Editable` may itself be an expression
(board:0030's `CurrPage` assignments) and folding a constant tree would freeze what AL changes.

`CurrPage.Editable` returns the declared bit combined with the mode, with View excluded, as the note
says.

## Ordering

With board:0030's control metadata and its page-mode handling.

## Gate, and its negative control

A control inside a group declaring `Editable = false` is read-only even though it declares
`Editable = true`; an AL assignment to the same field succeeds.

**The negative control is the AL assignment**, and the second is the `true` child under a `false`
parent -- an implementation using the most specific declaration renders it editable and lets a user
write to a page BC locks.
