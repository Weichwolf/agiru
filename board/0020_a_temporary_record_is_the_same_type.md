Type: root
State: open
Area: rt, gen
Tags: navision, semantics, architecture

# A temporary record is the SAME type with a different store, and the tests live on them

`Rec.SetTemporary(true)`, or a variable declared `Temporary`, and the record no longer touches the
database: inserts go to an in-memory table, keys and filters still work, `FINDSET` still walks in
key order, and the AL code cannot tell -- deliberately, because that is the point.

BC test code uses this constantly. Buffer tables, `TempSalesLine`, `Assert` helpers that build a
result set without a transaction. A UT run that cannot do it is a UT run that fails on setup rather
than on anything under test.

## Why it is architecture and not a feature

`runtime/Table.h` reaches the database directly today: `RuntimeInsert` calls `InsertRow(Session::
Current().Database(), ...)`. There is no seam. A temporary record needs the same `Table<T>` methods
to land somewhere else, which means a STORE behind the table -- one implementation over libpq and
one over memory -- chosen per record instance rather than per type.

The seam is worth more than temporary records alone. It is also where a test harness can stand, and
it is what makes the runtime testable without a database at all. Getting it in late means changing
every call site; getting it in now costs one indirection on a path that is already doing SQL.

**But the indirection is not free and the target says so.** A virtual call per record operation on
a Pi (board:0006) is measurable in a posting run. The likely answer is that the store is chosen
once per record and held, not resolved per call -- and that the memory store is the one that gets
the fast path, since it is the one used in a loop.

## The benchmark

The count of `SetTemporary` and `Temporary` in the BaseApp and its test codeunits, counted; and the
wall time of ten thousand inserts into a temporary record against the same into a real one, both
measured, before and after the seam goes in.

## Predecessor

`openerp/runtime/base/table/_temp_table.py` is the whole store in 70 lines, and it carries one
measured lesson that is not obvious and would be paid for again:

> `_TempStore` carries a monotonic mutation `version` so the `_next_temp` fast-path can tell when a
> cached filtered+sorted snapshot is still valid without re-scanning the whole store on every
> `Next()` (the O(n^2) hot-path: a `repeat ... until Next()=0` loop over a large temp buffer
> re-filtered+re-sorted the entire store on each step).

And a second, about sharing: the version rides on the STORE and not on the record instance, because
AL `Copy(src, true)` makes two record variables share one store and each must see the other's
mutations.

Both are about the SAME thing -- that a temporary record is walked in a loop far more often than a
real one -- which is the argument for giving the memory store the fast path here too.

## Closed when

A temporary record inserts, gets, modifies, filters and walks in key order with no database
connection open at all, through exactly the same generated `Table<T>` methods a real one uses.
