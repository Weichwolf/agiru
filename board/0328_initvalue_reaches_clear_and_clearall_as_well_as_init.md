Type:     task
Status:   open
Parent:   0068
Area:     rt
Source:   developer/properties/devenv-initvalue-property.md
Verdict:  teilweise
Class:    activation

# `InitValue` reaches `Clear` and `ClearAll`, not only `Init`

> Sets the initial value of this field when a user creates a new record.
>
> This attribute is only important if you create the record **in a window** or by using the AL
> methods **Clear** or **ClearAll**.

The page names three entry points and the tree has one.

> For a new field added in the table, values are initialized by default according to the data type
> or by the value given in `InitValue` **for all new records**. If you want to change the `InitValue`
> for existing records, you have to specify it in the code and run an upgrade -- the value doesn't
> set the values for already existing records.

That second note is a non-requirement worth recording: `InitValue` is never a backfill, so board:0070
has nothing to do for it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`InitValue =`: **2 546 declarations** (815 of them under `Layers/W1`, as `TableDef.h:86` records),
most of them `true` on a Boolean.

## The IST-state, and it is the reason this is `teilweise`

- `include/meta/TableDef.h:95` -- `FieldDef::initValue` exists, `std::optional<std::string_view>`,
  with the member name already resolved to its ordinal by the generator and "empty is not absent"
  stated in the door.
- `src/rt/Table.cpp:296` -- `RuntimeInit` walks the fields, skips the primary key, writes
  `*def.initValue` through `SetFieldText` and otherwise clears. **`Record.Init()` is done.**
- `src/rt/Builtins.cpp:74` -- `Clear(SecretText)` refuses the door.
- `src/rt/Builtins.cpp:79` -- `ClearAll()` refuses the door.
- `include/Builtins.h:102` -- `Clear(Any1&)` is a template that refuses for every type, so
  `Clear(Rec)` refuses too.

So two of the page's three entry points do not exist, and the third -- "in a window" -- is
board:0030's.

## The choice

`Clear(Record)` is `Init()` plus the record's OWN reset, and the door already says which: "clears all
the filters that were set if the variable is a record and resets the key to the primary key and the
company" (`include/Builtins.h:96`). So it is not a memset; it is a documented four-part reset and
`RuntimeInit` is one of the four.

`Clear(Any1&)` cannot stay one template. A record and an Integer clear differently, and the
distinction is a `requires` on `TableTraits<T>` -- the same shape `Table.h` already uses for the
trigger detection at `include/runtime/Table.h:353`.

`ClearAll()` is the harder one and stays refused until it has an owner: it clears the variables of
the CALLING OBJECT, which means the runtime needs a handle on a codeunit's or a page's locals. That
is board:0037's territory and this item does not pre-empt it.

## Ordering

`Clear(Record)` now -- `RuntimeInit` exists and the reset is documented. `ClearAll()` behind
board:0037.

## Gate, and its negative control

`Clear(Rec)` on a record with a filter, a non-primary current key and a Boolean field declaring
`InitValue = true` leaves: no filters, the primary key current, and the field `true`.

**The negative control is each of the four parts separately** -- an implementation that only calls
`RuntimeInit` passes the field half and leaves the filter set, and a single combined assertion cannot
tell which half failed.
