Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-multiline-property.md
Verdict:  fehlt
Class:    activation

# `MultiLine` makes a control a text area

> **Version**: runtime 3.2. Applies to: **Page Label, Page Field.**
>
> Sets the value that indicates whether a field can display multiple lines of text. **The default is
> false.**
>
> With 2023 release wave 2, you can use the `RichContent` option on the `ExtendedDataType` property
> to enable a rich text field. **To enable a rich text field, the field must have the `MultiLine`
> property set to true and it must reside alone within a FastTab group.**

The second paragraph is board:0329's precondition seen from this side, and it is the reason this bit
cannot simply be a rendering detail: `ExtendedDataType = RichContent` is a translation error unless
this property is `true` on the same control, and both are declarations, so the check is a
`static_assert` and it needs both properties in the metadata.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`MultiLine =`: **717 declarations**, all necessarily `true` since `false` is the default.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

One bit on the control descriptor; the renderer emits a `textarea` rather than an `input`.

The `RichContent` `static_assert` lives with board:0329 and reads this bit -- so the two land in the
same round or the assertion is written against a member that does not exist.

## Ordering

With board:0030's control metadata and board:0329.

## Gate, and its negative control

A control declaring `MultiLine = true` renders a multi-line editor and preserves a newline typed into
it; `ExtendedDataType = RichContent` without it fails to transpile.

**The negative control is the newline** -- a single-line input renders differently and also silently
drops the line break, so a gate on the rendered element alone misses half the property.

## The metadata half landed 2026-09-07 (board:0553)

The property this item is about now reaches `constexpr` metadata -- `ControlDef`/`PageDef` in
`include/meta/PageDef.h`, `FieldDef`/`TableDef` in `meta/TableDef.h`, `CodeunitDef` in
`meta/CodeunitDef.h` -- emitted per object into the `.cpp` and reached through the object's traits.
What is open here is what READS it, not what carries it: the property census over the whole BaseApp
went from 46 203 declarations dropped in silence to 813 in the same round.
