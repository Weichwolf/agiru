Type:     task
Status:   open
Parent:   0055
Area:     gen, rt
Source:   developer/properties/devenv-caption-property.md
Verdict:  teilweise
Class:    silent-wrong-data

# A caption reaches every object that declares one, not only a table field

> Sets the string that is used to identify a control or other object in the user interface.
>
> Applies to: **Table, Table field, Page Field, Field Group, Page, Request Page, Page Label, Page
> Group, Page Part, Page System Part, Page Action, Page Action Separator, Page Action Group, Xml
> Port** -- and, through their own pages, enum types and enum values.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Caption =`: **288 491 declarations.**

**That is the largest population in this entire sweep, by a factor of seven over `TableRelation`'s
40 221.** It is the most-declared thing in the BaseApp after code itself, and it is the reason the
caption path may not be an afterthought: a caption lookup that costs anything is paid 288 491 times.

## The IST-state

Two of the fourteen applicable kinds work:

- `src/gen/TableWriter.cpp:37` -- `Find(field.properties, "Caption")`, falling back to the field name.
  It reaches `FieldDef::caption` (`include/meta/TableDef.h:70`), which is `constexpr` `.rodata`.
- `src/gen/EnumWriter.cpp` -- `Find(value.properties, "Caption")` on an enum value.

Everything else is dropped: **a table's own caption, and every page control, group, part and action.**
`src/gen/PageWriter.cpp` consumes `SourceTable` alone. `TableDef` at `include/meta/TableDef.h` has a
`caption` member and `src/gen/TableWriter.cpp:551` writes `.caption = <tableIdentifier>::kName` --
**the table's caption is hardcoded to its name**, so a table declaring a different one loses it.

## The choice

The mechanism is right and the coverage is not: a `string_view` into `.rodata`, resolved at
translation time, no lookup and no allocation. Extend it to the table itself (one line at
`src/gen/TableWriter.cpp:551`) and to every page element as board:0030's control metadata arrives.

**SIZED by board:0566: 3 765 tables declare a `Caption`, across 4 564 `.Table.al` files.** So roughly
four table captions in five are declared and discarded, and the fall-back to the name is right only
for the fifth. `devenv-work-with-translation-files.md` states the rule the field path already keeps
and the table path does not: *"If the object already has a `Caption` property set, THAT VALUE IS
USED."*

**Nothing here is computed at run time**, which at 288 491 declarations is the whole design.

## Ordering

The table half is one line and goes now. The page half arrives with board:0030's control metadata.

## Gate, and its negative control

A table declaring `Caption = 'Sales Header'` reports that caption and not its name; a field without
one reports its field name.

**The negative control is a table whose caption differs from its name** -- today `.caption` IS the
name, so a gate on a table where the two agree passes against the hardcoded value.
