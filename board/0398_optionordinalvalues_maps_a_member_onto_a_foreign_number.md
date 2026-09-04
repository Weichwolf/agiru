Type:     task
Status:   open
Parent:   0364
Area:     gen
Source:   developer/properties/devenv-optionordinalvalues-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `OptionOrdinalValues` maps a member onto a foreign number

> Specifies the list of option values. **Can be set if the property `ExternalType` is set to
> `Picklist`.** Applies to: **Table field.**
>
> **The position of the option members value in the external database.** You can set `-1`, `1`, `2`,
> **but you cannot set the value to `0`.**
>
> `OptionOrdinalValues = 1,2,3,4,5;`
>
> This property is used when you specify **CDS** in the `TableType` property.

**An AL option's ordinal is its position and this property breaks that**: the third member may be
number 7 in Dataverse, and `-1` is legal while `0` is not. So an option field carrying this property
has two numbers -- the AL ordinal a body compares against, and the external ordinal the row holds.

**And that is precisely the pair a translation must not collapse.** board:0076 asserts an option
type's own ordinals; this property says the STORED number is a different one. An implementation that
used the external value as the ordinal would break every `case` in AL that compares against a member.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`OptionOrdinalValues =`: **310 declarations.**

**Against zero tables of `TableType = CDS`** (board:0364 measures `Temporary` 298, `CRM` 83,
`Exchange` 4, `MicrosoftGraph` 1, and no `CDS` at all). So this is the same contradiction
board:0365's `ExternalName` shows at 3 900: the property is declared on tables of a type the page
says it requires, and that type is never declared.

**The two are almost certainly the same finding** -- the `CRM` proxy tables carry both -- and
board:0365 resolves it from the AL source. This item waits on that answer rather than guessing a
second time.

## The IST-state

Not among the nine properties the generator consumes (board:0067). `include/meta/TableDef.h:81` --
`FieldDef::values` is a span of `EnumValueDef` carrying `{ordinal, name, caption}`, where the ordinal
is the AL one.

## The choice

Follows board:0364's decision about `CRM` tables. If those are refused, this property is refused with
them, because it has no meaning on a table whose rows are local.

If they are ever translated, the external ordinal is a SECOND number on `EnumValueDef` and never a
replacement, and the `0` prohibition is a `static_assert`.

## Ordering

Behind board:0364 and board:0365. Nothing to decide before the `CRM`/`CDS` question is answered.

## Gate, and its negative control

A field declaring `OptionOrdinalValues` on a `CRM` table fails to transpile, with the same message
board:0364's table type produces.

**The negative control is an ordinary Option field** -- it must be unaffected, and an implementation
that added a second ordinal to every `EnumValueDef` would widen 1 609 tables' metadata for 310
declarations.
