Type: arc
State: open
Area: rt, gen, cli

# A table number finds its declaration

`RecordRef.Open(TableNo)` and `RecordId.GetRecord()` both start from a NUMBER and have to arrive at
a record. Today both refuse, and they refuse honestly -- there is no way from `27` to the
declaration of `Item`, because nothing in this tree holds the set of tables.

## What the references say

`recordref-open-integer-method.md` gives `RecRef.Open(TableNo)` with no further ceremony: the
platform has the whole catalogue and hands back a reference over it. `recordid-getrecord-method.md`
returns a RecordRef the same way.

**The UT suite asks for it 24 times**, measured over the 114 `*UT*.Codeunit.al` under
`src/Layers/W1/Tests`: `RecRef.Open` 20, `RecordRef.Open` 4. What follows an `Open` there is not
metadata but WORK -- `RecRef.Insert` (3), `RecRef.Modify` (7), `RecRef.FindFirst` (3),
`RecRef.Count` (2), `RecRef.SetTable` (5), `RecRef.Rename` (1). So this is not a lookup that returns
a `TableDef`; it is a record with storage behind it.

`openerp` built the equivalent as a dict filled at import time, one entry per table, and that dict is
part of the gigabyte per process this tree left Python over. It is a hint about the SHAPE -- number
to declaration -- and a verdict about nothing.

## The choice

**The catalogue is `constexpr` and the transpiler emits it**, sorted by table number, with a
`static_assert` on the sort and on the absence of a duplicate number. It is `.rodata` like every
other piece of metadata: paged on demand, shared between processes, nothing at startup. A registry
assembled by 1 767 static initialisers is the same data at a cost, and the cost is paid in every
process before `main`.

**The runtime holds a span, not the array.** `rt` may not reach `apps`, and a catalogue defined in
`apps/` and read from `src/rt` would invert the tier. So the runtime declares the span and the DOOR
installs it: one call, one pointer, at the point the process decides which apps it is. The gate
installs a catalogue of two tables and needs no `apps/` at all, which is also how the negative
control is written -- a number no catalogue holds must refuse, not return an empty record.

**An entry is wider than a `TableDef *`.** A generated record has no virtual functions -- it cannot,
because `offsetof` over the field table needs standard layout -- so a record made from a number
alone cannot dispatch through a vtable. The entry therefore carries the two function pointers a
`TableDef` cannot: one that places a record in the session's arena and one that runs its triggers.
Where exactly the trigger half lands is open until event dispatch exists; the storage half is not,
and it comes first.

## What is true when this closes

- A `RecordRef` opened by number answers `Number`, `Name`, `FieldCount` and `Field` as one opened by
  `GetTable` does, over the same declaration.
- A number the catalogue does not hold REFUSES and says so with the number in the text.
- `RecordId.GetRecord()` exists.
- The catalogue costs nothing before `main`: no dynamic initialiser is emitted for it, and that is
  checked rather than believed.
