Type: arc
State: open
Area: rt, gen, cli

# A table number finds its declaration

`RecordRef.Open(TableNo)` and `RecordId.GetRecord()` both start from a NUMBER and have to arrive at
a record. ~~Today both refuse~~ -- **the first one landed**, and this item is re-measured rather
than left reading as though nothing had happened.

**Done** (verified 2026-09-04 by the sweep, board:0071): `src/rt/RecordRef.cpp:110` opens against the
catalogue and refuses an unknown number with the number in the text -- "this installation carries no
table 4711". `Number`, `Name`, `Field` by number and by name, `FieldCount`, `FieldExist` and
`FieldIndex` all answer over the declaration it found, and `RecordRefGate.cpp` covers them.

**What is left is the two BRIDGES and the row half:**

- `GetTable(Record)` and `SetTable(Record)` -- the conversion between a typed record and a
  `RecordRef`, which is 5 of the 24 UT call sites this item counted.
- `RecordId.GetRecord()`, still absent -- and the reason `include/type/RecordId.h:26` gives has
  expired: "there is no RecordRef in this runtime" was true when it was written.
- `Get(RecordId)`, `GetBySystemId`, `RecordId()`, `Caption()`, `FullyQualifiedName()`.
- `Open(Integer [, Temporary] [, CompanyName])` takes three arguments in the documentation and one
  in the door, so a `RecordRef` cannot be opened temporary or against another company
  (board:0047, board:0060, board:0072).
- The five `System*No()` methods, which return the FIELD NUMBER of each system field -- metadata
  `agiru::kSystemFields` already holds (board:0013).

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
