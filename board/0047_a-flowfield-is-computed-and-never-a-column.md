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

**THE SCOPE IS `Layers/W1` AND NOT THE TREE**, which matters because board:0339 and board:0510 print
`FlowField` **8 772** and `FlowFilter` **1 510** for the whole of `~/Git/BCApps/src`. 490 against
1 510 is not a disagreement; it is one subtree against all of them. **Re-measured 2026-09-04 with the
settled pattern over the same subtree, the numbers above reproduce**: `FlowField` 2 153,
`FlowFilter` 490, `CalcFields` 3 604, `CalcSums` 1 319 -- with two drifts of a few counts,
`CalcFormula` 2 149 against 2 150 and `SetAutoCalcFields` 241 against 251, which is the shell's
wrapped `grep` against `/usr/bin/grep` and the leading dot. The drifts are left visible rather than
smoothed.

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

**Three rules from `devenv-calcfields-calcsums-...-methods.md`** (read 2026-09-04, board:0071),
none of them in this item:

- **`CalcSums` requires the key carrying the `SumIndexFields` to be the CURRENT KEY.** "For
  `CalcSums`, a key that contains the SumIndexFields must be selected as the current key." So
  `SetCurrentKey` is a precondition and not a hint, and a `CalcSums` without it is an error rather
  than a slow path.
- **Both use the CURRENT FILTERS**, which is what makes a `FlowFilter` reach a `CalcFormula` at all.
- **A FlowField that is the DIRECT source expression of a page control is calculated
  automatically**; `CalcFields` is only needed "when they're part of a more complex expression". So
  a page runtime (board:0030) calculates them without being asked, and AL code must ask.
- **And it calculates them even when they are INVISIBLE.**
  `calculate-only-visible-flowfields-feature-key.md`: "**By default, FlowFields are calculated even
  if their `Visible` property is set to `false`**, either explicitly or dynamically. This behavior
  can lead to unnecessary computations and performance issues." A feature key changes it. So the
  faithful default is the expensive one, and a page runtime that calculated only what it draws
  would be faster than BC and produce different `OnAfterGetRecord` timings -- worth knowing before
  it is chosen by accident.

`FieldError` carries its own reason for existing: the message "reflects the CURRENT name of the
field", so it is the translated caption at the moment it is raised rather than a name baked into a
label (board:0055).

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

## WHEN A SIFT INDEX IS USED, AND WHEN THE SUM FALLS BACK TO THE BASE TABLE

`administration/optimize-sql-al-Database-methods-and-performance-on-server.md`, read 2026-09-04
(board:0071), states the selection rule as two conditions rather than a heuristic:

> Each **CalcFields** or **CalcSums** request should be confined to use only one SIFT index. The SIFT
> index can only be used if:
>
> - **All requested sum-fields are contained in the same SIFT index.**
> - **The filtered fields are part of the key fields specified in the SIFT index containing all the
>   sum fields.**
>
> If neither of these requirements is fulfilled, then the sum is calculated **directly from the base
> table**.

and one more, which is why `Count` belongs here too:

> SIFT indexes **count records in a filter** if a SIFT index contains all filtered fields in the key
> fields defined for the SIFT index.

**Both conditions are decidable against the metadata this tree already emits**: the SIFT keys are
`constexpr`, and the filtered fields are known where the filter is (board:0018). So choosing the
index -- or deciding there is none and summing the base table -- is a lookup over `.rodata` rather
than a planner, which is exactly board:0019's argument for why NAV was fast.

**And the result is CACHED**: "Each call ... requires a separate SQL statement **unless the client
calculates the same sum or another sum using the same SumIndexFields or filters in a recent
operation. In that case, the result is cached.**" That is a cache with a scope this item must fix,
and CLAUDE.md draws the line: nothing cached across a transaction without the rowversion is anything
but stale. A FlowField cache is therefore per TRANSACTION at the widest, and invalidated by any
write to the summed table -- which the same page's rule about SIFT maintenance on every `Insert`,
`Modify` and `Delete` makes cheap to detect.

**`SetAutoCalcFields` is the documented fast path** and the page's own worked example is the gate:
three versions of one loop -- `CalcFields` per row, then a `SetFilter` on the FlowField, then
`SetAutoCalcFields` -- each producing the same answer with fewer statements. **A FlowField in a
FILTER is the middle step**, and the page states it plainly: "setting a filter on a record is
translated into a single SQL statement", including when the filtered field is a FlowField. That is
this item's hardest case and the page confirms it is not optional.

`SetAutoCalcFields` is also on `RecordRef` from 2025 release wave 1, which board:0048's partial-load
design has to carry.

## THE SEVEN TYPES PAIR WITH FIELD TYPES, AND THE PAIRING IS A `static_assert`

`devenv-flowfields.md` (read 2026-09-04, board:0071) tabulates them, and the second column is the
part no other page states:

| FlowField type | the field's own type must be |
|---|---|
| `Sum` | Decimal, Integer, BigInteger or Duration |
| `Average` | Decimal, Integer, BigInteger or Duration |
| `Exist` | **Boolean** |
| `Count` | Integer or BigInteger |
| `Min` | Any |
| `Max` | Any |
| `Lookup` | Any |

**Both halves are declarations**, so a `Sum` on a `Text` field or an `Exist` on an `Integer` is a
TRANSLATION error and not a run-time surprise -- exactly the shape CLAUDE.md asks for: "anything
decidable at translation time is a `static_assert`, never a test case". The generator emits one
beside each FlowField and a mis-declared `CalcFormula` stops the build naming the field.

Two more sentences from the same page settle behaviour this item carries loosely:

- **"Values in FlowFields are automatically initialized to 0 (zero)"** -- so an uncalculated
  FlowField is 0 and never null or undefined, which matches the `ISNULL(..., @3)` board:0019 found in
  the platform's own SQL. One rule, seen from both ends.
- **Calculation happens in exactly three situations**: the FlowField is "the direct source
  expression of a control on a page"; `CalcFields` is called; or **a FlowFilter is applied**. The
  third is the one an implementation forgets -- applying a FlowFilter RECALCULATES, it does not
  merely narrow a cached value.

And the visibility rule now has its version: calculation happens even when `Visible = false` **by
default**, and version 26.0 adds a **Feature Management** switch, "Calculate only visible
FlowFields", that changes it (`calculate-only-visible-flowfields-feature-key.md`). So the expensive
behaviour is the faithful one and the cheap one is opt-in -- which is the right way round for a
runtime that has to match BC, and worth knowing before anyone optimises it away.

## A TEMPORARY TABLE IS OUTSIDE THE TRANSACTION, AND THAT IS THE CITATION board:0055 NEEDED

`devenv-temporary-tables.md` (read 2026-09-04, board:0071) states it in one line:

> - A temporary table data isn't stored in the database. It's only held in memory until the table is
>   closed.
> - **The write transaction principle that applies to a database table doesn't apply to a temporary
>   table.**

So a temporary table's rows survive a rollback because they were never in the transaction -- which is
exactly what makes Preview Posting work (board:0055): the posting runs, the entries are captured into
temporary tables, and `Error('')` rolls the DATABASE back while the capture stands. `Temporary<T>`
must therefore hold its rows in memory and outside `detail::Scope`, and a design that backed it with
a real table -- even a session-scoped or `ON COMMIT DROP` one -- would break that feature.

**And a temporary record still carries the system fields**: "Temporary tables retain system fields,
like SystemID and data audit fields" (board:0013). So `SystemId` is assigned in memory and
`SystemCreatedAt` is set, which a design that treated a temporary record as "just the declared
fields" would miss.

Three implementations, and the first is the one this item does not yet carry:

| how | effect |
|---|---|
| `TableType = Temporary` on the TABLE OBJECT | no physical table at all, and **"the table schema isn't synchronized with the database"** -- so no breaking-change restrictions on it |
| a `temporary` record VARIABLE | the shape `Temporary<T>` models |
| `SourceTableTemporary = true` on a PAGE | the page's `Rec` is temporary (board:0030) |

`TableType = Temporary` is a translation-time fact: such a table gets no `CREATE TABLE` and no
columns, which `src/rt/Storage.cpp` currently has no way to know. It is one `constexpr bool` on the
`TableDef` and it removes the table from the schema entirely.
