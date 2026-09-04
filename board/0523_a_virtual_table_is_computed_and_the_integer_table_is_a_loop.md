Type:     task
Status:   open
Parent:   0032
Area:     rt, gen
Source:   developer/devenv-virtual-tables.md, developer/devenv-integer-virtual-table.md, developer/devenv-date-virtual-table.md, developer/devenv-extend-pages-based-on-date-virtual-table.md
Verdict:  teilweise
Class:    activation

# A virtual table is computed, and the `Integer` table is how AL writes a loop

**Four pages, one item**: the overview and the three that describe or extend the two virtual tables an
ERP actually uses.

> "A virtual table contains system information. **You CAN'T CHANGE the data. You can only read.**
> Virtual tables **aren't stored in the database, but are COMPUTED at runtime.**"
>
> **"You can use the SAME METHODS to access information in virtual tables as with ordinary tables."**
> Filters, ranges, `Find`, `Next`.

**Read-only and computed, with the full `Record` surface over them.** So a virtual table is a
generated class whose read path is a generator rather than a cursor -- the same instance-level
dispatch board:0522's temporary record needs, with a third backend.

## `Integer` (2000000026) is the AL `for` loop

> "includes integers in the range **-1 000 000 000 to 1 000 000 000**. Contains **only one field**."
>
> "By applying a filter, you can easily get a subset or range of numbers that can be used to **CONTROL
> LOOPING IN REPORTS**."

**This is how a report iterates N times without a table**, and CLAUDE.md counts 668 reports in scope.
A `dataitem` over `Integer` with `SetRange(Number, 1, 10)` is the AL idiom for "do this ten times", so
board:0063's data-item loop meets it immediately.

**And the field-name contradiction is the one CLAUDE.md already names**, so this item confirms rather
than discovers it: the page's table lists the field under the heading `Field` as **`Integer`**;
**the field is called `Number`**, which the AL source says 33 times and contradicts 0 times. CLAUDE.md's
rule -- "the page is describing the CONTENTS of the column, not naming it; where the documentation
DESCRIBES and the source DECLARES, the source declares" -- resolves it, and the resolution is
`Number`.

**That makes this page the canonical example of the rule, and the item records that** so the next
reader meets it as a settled case rather than a fresh contradiction.

## `Date` (2000000007) is five fields and it returns closing dates

> | field | |
> |---|---|
> | `Period Type` | **Days, weeks, months, quarters, or years** |
> | `Period Start` | the first day in the period |
> | `Period End` | the last day |
> | `Period No.` | |
> | `Period Name` | |
>
> **"The `Period End` field returns THE CLOSING DATE at the end of the period."**

**So the `Date` virtual table produces closing dates as ordinary values**, which is board:0016's subject
arriving from a source nobody would look for. A `Period End` compared against an ordinary date must
sort after it and before the next day -- and any code that filters on `Period End` inherits that.

**And `Period Type` makes one table five tables**: the same rows exist five times over, at five
granularities, selected by a filter. So the generator behind it is parameterised by a filter value,
which is unlike any other read in this tree.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

CLAUDE.md: the field name `Number` appears **33 times** and is contradicted **0 times**. board:0032
counts the platform tables: **87 tables named by 381 declarations, and no AL file declares any of
them.**

## The IST-state, and it is why this is `teilweise`

`include/platform/Integer.h:67` -- `KeyDef{.name = "PK", .fields = Integer::kKey1, .clustered = true}`.
`include/platform/Date.h:122` -- the same. **So both virtual tables EXIST as platform table
definitions with keys** (board:0348 cites them), which is more than most of board:0032's 87.

**What is not known here is whether their READ produces rows** -- whether `Integer.FindSet` yields
integers or queries a table that does not exist. That is this item's first check, at
`src/rt/Cursor.cpp` and `src/rt/Storage.cpp`.

## The choice

A third read backend on the record instance: cursor (board:0045), in-memory (board:0522), **computed**.
For `Integer` the generator is a counter bounded by the filter; for `Date` it is a period walker
parameterised by `Period Type`.

**The filter must be interrogated, not just applied** -- board:0508 and board:0474 already need that,
and here it is unavoidable: an unbounded `Integer` read is two billion rows, so the range must be read
OUT of the filter before any row is produced.

**An unfiltered `Integer` read is a refusal**, not a two-billion-row stream. That is a deviation from
"the same methods as ordinary tables" and it is the only safe reading of a table whose full extent is
2 000 000 001 rows.

## Ordering

Behind board:0018's filter structure. Ahead of board:0063's data items, which use `Integer` to loop.

## Gate, and its negative control

`Integer.SetRange(Number, 1, 10)` yields ten rows numbered 1 to 10; `Date` filtered to
`Period Type = Month` over a year yields twelve rows whose `Period End` values are closing dates.

**The negative control is the unfiltered read** -- it must REFUSE rather than begin streaming, and an
implementation that lazily generates rows passes every filtered gate and hangs the first time AL
forgets a filter.
