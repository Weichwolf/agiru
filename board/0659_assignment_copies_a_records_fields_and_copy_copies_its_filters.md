# `Rec := Other` copies a record's fields, and `Copy` copies its filters too

**Finding (2026-09-10).** `Record Set Management.SaveSet` does `TempFoundRecordSetTree :=
RecordSetTree; TempFoundRecordSetTree.Insert()` for every node it finds through a filtered
`RecordSetTree`. The assignment carried the database record's `FindNode` filters onto the
temporary buffer, so `CreateNewSet`'s `FindFirst` saw one node of ten and every multi-record set
came out one line long (27 cases of `Record Set UT`, traced 2026-09-10: ten inserts into
`Record Set Tree`, one into `Record Set Definition`).

**Reference.** `record-copy-method.md`: `Copy` "copies a specified record's filters, views,
automatically calculated FlowFields, marks, fields, and keys"; assignment is the record buffer.
The predecessor's `_al_assign` is documented as "full field-value copy (fields only)" and stood
at 2 260 green. This tree had `StateHandle::operator=` copy the filters and the key.

**Choice.** Assignment leaves the target's state alone; `Copy` calls `CopyStateFrom`, which is
the former assignment. `RecordRef.GetTable` / `SetTable` and the catalogue's `copy` use `Copy`,
because a RecordRef over a record sees its filters (`recordref-gettable-method.md`). 839
`Temp := Rec` sites in the generated BaseApp change behaviour, so this is an activation and the
milestone is the measurement; a loss names the deeper root before it is taken back.
