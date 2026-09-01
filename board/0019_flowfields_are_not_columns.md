Type: root
State: open
Area: rt, db, gen
Tags: navision, semantics, blocker

# FlowFields are not columns, and SIFT is why NAV was fast

A FlowField has no column. It carries a `CalcFormula` -- `Sum("Amount" WHERE (...))`, `Count`,
`Lookup`, `Exist`, `Min`, `Max`, `Average` -- and it holds nothing until `CALCFIELDS` runs the
query behind it. `Customer."Balance (LCY)"` is a Sum over `Detailed Cust. Ledg. Entry`; nothing in
the Customer row holds a balance, and BC has never stored one.

Three mechanisms hang together here and none of them exists in this tree yet.

**The field class.** `FieldClass = FlowField` and `FieldClass = FlowFilter`. `FieldDef` has a type
and no class, so a FlowField is currently emitted as an ordinary column -- which means the
generated CREATE TABLE has a column BC does not have, and an INSERT writes a value BC never wrote.
That is a defect in the schema and not only in the calculation. The same missing field class blocks
BLOBs (board:0017).

**The FlowFilter.** A field of class FlowFilter is not data either: it is a filter the USER sets,
which the CalcFormula of another field reads. `Customer.SETFILTER("Date Filter", '..%1', Today)`
followed by `CALCFIELDS("Balance (LCY)")` gives the balance as of a date. So a FlowFilter needs the
filter language (board:0018) and a place on the record that is neither a column nor a value.

**SIFT.** A key with `SumIndexFields` was, in NAV, a maintained aggregate -- a separate table
updated on every insert, which is why a customer balance was instant over millions of entries. BC
on SQL Server keeps indexed views for it. Whether agiru needs the same, or whether PostgreSQL's
aggregate over a covering index is fast enough on the target hardware, is EXACTLY the kind of
question that gets a measurement rather than an opinion (board:0006). It is written down here so
that the answer is chosen rather than defaulted into.

## The benchmark

The number of FlowFields and FlowFilters in the BaseApp, counted rather than guessed, and the wall
time of one `CALCFIELDS("Balance (LCY)")` on a CRONUS customer -- with an aggregate query and, if
it comes to it, with a maintained total -- on x86_64 and on the Pi.

## Closed when

A FlowField is absent from the generated schema, empty until CALCFIELDS, and correct afterwards for
all seven formula kinds; and the SIFT question has a measured answer either way.
