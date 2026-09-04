Type:     task
Status:   open
Parent:   0063
Area:     gen
Source:   developer/properties/devenv-optionmembers-report-property.md, developer/properties/devenv-optionmembers-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# A report column declares its own option members

> **OptionMembers property on report columns**: Specifies the values that are available in an
> `Option` **report column**.
>
> The overview page `devenv-optionmembers-property.md` says the property applies to two object types:
> table fields and report columns.

board:0053 owns the TABLE FIELD half -- `OptionMembers` beside `OptionCaption`, the member names a
body writes against the caption a user reads. This item is the other half, and it is separate because
the consumer is different: a report column's members are a `constexpr` list on a dataset column and
never a field on a record.

**A report column is a value in a dataset, not a member of a table**, so its option list cannot come
from a `FieldDef`. Where the column is bound to a table field the field's list would do; where it is
bound to an expression, the column's own declaration is the only source. That is why AL lets a column
declare it at all, and it is the case an implementation reusing `TableWriter`'s option path would
miss.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`OptionMembers =`: **3 790 declarations**, table fields and report columns together. The two cannot be
separated by `grep` alone -- the property is spelled identically -- so the split is measured by
counting the declarations inside `.Report.al` files when this item is pulled.

**That is a limitation of the measurement and it is said rather than rounded**: this item's own
population is not yet known, only its upper bound.

## The IST-state

Reports have no generator (board:0063, board:0034). `src/gen/TableWriter.cpp` consumes `OptionMembers`
on a FIELD (one of the nine, board:0067) and `src/gen/BodyWriter.cpp` reads it to resolve a member
name in a body -- so the field half works and the report half has nowhere to land.

## The choice

A `constexpr` span of `EnumValueDef` on the report column descriptor -- the same type the field uses,
so an option value renders and compares identically wherever it came from. Member names are resolved
to ordinals by the generator, as they are for a field.

**The identical TYPE matters more than the shared code path.** Two different representations of an
option would give two different `Format` results for the same value.

## Ordering

Inside board:0063's report generator. Behind board:0053, which settles the caption-versus-member rule
once for both halves.

## Gate, and its negative control

A report column declaring its own members renders a value by its member name and not its ordinal.

**The negative control is a column bound to a table field with a DIFFERENT list** -- the column's own
declaration must win, and an implementation that prefers the field's passes every gate where the two
agree.
