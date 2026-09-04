Type:     task
Status:   open
Parent:   0065
Area:     rt, gen
Source:   developer/properties/devenv-autosave-property.md, developer/properties/devenv-autoreplace-property.md, developer/properties/devenv-autoupdate-property.md
Verdict:  fehlt
Class:    activation

# An XMLport import replaces, updates, or refuses an existing record

**Three pages, one item**: `AutoSave` decides whether the runtime writes at all, and `AutoReplace` and
`AutoUpdate` decide what it does when the row already exists. The pages define each other and each
warns about the other two.

> **AutoSave** (default **true**): whether imported records are automatically written to the table.
> If false, the record is not written -- **you insert or modify it yourself from `OnBeforeInsertRecord`
> and `OnBeforeModifyRecord`.**
>
> **AutoReplace** (default false): if a record with the same primary key is found, **the record is
> initialized with the initial value for each field and then populated with the values in the
> imported record. Any field not present in the imported record retains its INITIAL value.**
>
> **AutoUpdate** (default false): the record in the database **is updated** with the values from the
> XMLport. **Fields that are not defined in the record from the XMLport remain the same.**
>
> **If `AutoReplace` is true, then `AutoUpdate` has no effect.** And conversely: "If `AutoUpdate` is
> set to true, then `AutoReplace` has no effect."
>
> **If the record already exists and both are false, then an ERROR occurs.**

**The difference between replace and update is what happens to the fields the file does not mention**:
replace resets them to their `InitValue` (board:0328), update leaves them. That is a data-loss
difference on every import of a partial file, and the two properties are one letter apart in a
property list.

**And both pages claim precedence over the other**, which cannot both be true. That contradiction is
recorded rather than resolved by preference; the AL source and a BC import settle it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AutoSave =` **209** (all `false`) · `AutoReplace =` **2** · `AutoUpdate =` **68**.

**209 table elements switch the automatic write off entirely** and do their own inserting -- ten times
as many as use either overwrite mode. So the common case in the BaseApp is manual, and the three-way
default matters least where it is most subtle.

## The IST-state

XMLports have no generator (board:0065, board:0034). `RuntimeInit` at `src/rt/Table.cpp:296` already
implements the `InitValue` reset `AutoReplace` needs.

## The choice

Three bits on the table element, resolved by the generator into ONE three-valued decision -- write
nothing, replace, or update -- so the import path has no precedence question left to get wrong, and
the contradiction above is settled once at translation time rather than per row.

The "both false and the record exists" error is board:0055's, with BC's own wording.

## Ordering

Inside board:0065's XMLport writer. Behind board:0328, whose `InitValue` reset is what `AutoReplace`
means.

## Gate, and its negative control

Importing a partial row over an existing record: with `AutoUpdate` the unmentioned fields keep their
values, with `AutoReplace` they are reset to their `InitValue`, with neither the import raises.

**The negative control is the unmentioned field** -- the two modes differ only there, and an
implementation that maps both onto a `Modify` passes every gate whose file mentions every field.
