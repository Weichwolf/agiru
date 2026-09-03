# 0047 -- A FlowField is computed, filtered and summed, and is never a column

`meta/TableDef.h` carries no `FieldClass`, so the runtime cannot tell a FlowField from a stored
field. Three consequences, and the first is already in the schema:

- `CreateTable` gives every declared field a column, so 2 153 FlowFields under `Layers/W1` are
  columns that will always hold the default. A `Customer."Balance (LCY)"` reads 0 and does not
  throw.
- `Select` puts a filter on any field straight into the `WHERE`, so a filter on a FlowField
  becomes a predicate on that empty column -- a WRONG ANSWER rather than a refusal, which is the
  worst shape a defect takes here.
- `CalcFields`, `CalcSums` and `SetAutoCalcFields` have no path at all.

Measured in `~/Git/BCApps/src/Layers/W1`, 2026-09-03:

| construct | count |
|---|---|
| `FieldClass = FlowField` | 2 153 |
| `CalcFormula =` | 2 150 |
| `FieldClass = FlowFilter` | 490 |
| `.CalcFields(` | 3 604 |
| `.CalcSums(` | 1 319 |
| `SetAutoCalcFields(` | 251 |
| `Temporary = true` | 407 |
| a record variable declared `temporary` | 12 032 |

**The reference.** `properties/devenv-fieldclass-property.md` and
`devenv-calcformula-property.md` give the six kinds of `CalcFormula` -- Sum, Average, Exist, Count,
Min, Max -- each with a table filter and an optional `WHERE` over it.
`methods-auto/record/record-calcfields-method.md` and `record-calcsums-method.md` say what runs
when. A FlowFilter is not a value at all: it is a filter a FlowField's formula reads.

**What the AL source does.** A FlowField appears in a filter and in a key like any other field, so
the shape cannot be refused at the call site -- it has to be understood.

**What the predecessor made of it.** `~/Git/openerp/` implements all six kinds and a filter over
them; `ls board | grep -i flow` names the rounds it cost. Read the finding, not the fix: it built
the descriptors at run time and paid a gigabyte for it, and here they are `constexpr`.

**The choice.**

1. `FieldDef` gains a `FieldClass` and a FlowField gains its `CalcFormula` as `constexpr` data --
   the method, the table, the source field, the filter list. It is knowable at translation time and
   therefore belongs in `.rodata` beside the rest.
2. `CreateTable` gives a FlowField NO column. That is a schema change and it is cheaper now than
   after the CRONUS load is mapped against it (board:0004).
3. A filter on a FlowField becomes a SUBQUERY in the `WHERE`, which is what BC's own SQL does, and
   until it does the filter REFUSES with the field's name rather than reading an empty column.
4. `CalcFields` and `CalcSums` are one aggregate per field over the formula's filters, and
   `SetAutoCalcFields` makes them part of the `FindSet` statement rather than a query per row.

**And a temporary record is the other half.** 12 032 record variables are declared `temporary` and
`Temporary<T>` already carries its own `Count`, `IsEmpty`, `FindSet`, `Next` and `DeleteAll` over an
in-memory store -- so the navigation that landed for stored records (board:0044) has a twin that
must give the SAME answers. A gate that runs the same walk over both is what proves it, and the
places they are allowed to differ (no locking, no rowversion, no SQL) are the interesting part.
