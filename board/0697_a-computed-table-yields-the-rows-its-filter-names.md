# 0697 A computed table yields the rows its filter names

`Integer` (2000000026) is a virtual table: AL writes `Integer.SetRange(Number, 1, N); Integer.FindSet`
where C would write `for`, and a `dataitem("Integer"; "Integer")` is the same loop in a report. Here
it was a PHYSICAL, EMPTY table, so every such loop ran zero times in silence.

**The references.** `devenv-integer-virtual-table.md` gives the range -1,000,000,000 to
1,000,000,000 and the single column; the source calls it `Number` (33 times, contradicted 0 times).
The predecessor built exactly this (`_IntegerTable._seq_bounds`, openerp WI-1040 / WI-936) and
measured the two halves apart: the BOUNDED half is a read that returns the rows a filter admits, the
UNBOUNDED half (`SetFilter(Number, '1..')`, or a dataitem with no filter at all) is an infinite loop
AL ends with `CurrReport.Break`, and activating it there was NET NEGATIVE (WI-936: it uncovered a
`Next`-exhaustion in the caller and iterated to a 1 000 000 cap).

**The choice.** `TableDef::sequenceField` names the field whose values the rows ARE, and a read on
such a table selects `generate_series` over the intervals the field's filters admit
(`src/rt/Selection.cpp`, `Selection::from`). `IntegerIntervals` was already there and does the
parsing. A filter that admits an edge of the domain, or more than 1 000 000 rows, yields the
physical table -- that is, nothing -- because this runtime has no `CurrReport.Break` brake on a
dataitem walk yet.

**What is still open:** the unbounded half. It needs `MaxIteration` carried from AL and
`CurrReport.Break` ending the walk; without both it is the predecessor's measured loss. Filed here
rather than done, because the bounded half is a silent-wrong-data fix and the other is an activation.

**A second defect the same round uncovered.** With the report walking, `Service Timesheet Posting
UT` reached `ServShptHeader.TransferFields(ServHeader)` and died with `std::bad_alloc`.
`TableTraits<Instance<T>>` inherits the table's traits, so a record held by HANDLE (board:0018)
compiles wherever a record is expected -- and `&From` was then the handle's own address, read
against the table's field offsets. `detail::RecordAddress` unwraps it, and it is what every runtime
entry taking a source record's address now goes through.
