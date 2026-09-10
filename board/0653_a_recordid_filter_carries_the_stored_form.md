# A `RecordID` filter carries the stored form on both sides

**Finding (2026-09-10).** `Record Set UT` was red end to end (43 cases): `RecordSetTree.SetRange(
Value, RecRef.RecordId)` bound `Customer: GL00000030` while the column held
`18\x1fCustomer\x1fGL00000030` (`RecordId::ToStorageText`), so no node was ever found again and
every set was created twice. A temporary row compared its `Format` text against the same filter
and matched, so the temporary side hid the SQL side.

**Reference.** `record-setrange-method.md` says nothing about a RecordID's text; `RecordId.h`
records that `Format(RecordId)` is a caption and a key that nothing can read back into a table
number. So the filter has to carry the form the column holds.

**Choice.** `FilterText(RecordId)` and `FieldRef.SetRange(Variant holding a RecordId)` produce
the stored form; the temporary matcher compares a `RecordID` field on its `StorageText`
(`Temporary.cpp Passes`). `GetFilter(Value)` on such a field therefore shows the stored form, a
visible deviation recorded here rather than hidden. `SetFilter(Value, '%1', RecId)` goes through
`StrSubstNo` and would carry the display form; the generated BaseApp has no such call site
(counted 2026-09-10), so it is not built for.
