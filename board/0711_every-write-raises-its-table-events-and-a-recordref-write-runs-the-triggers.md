Type:     task
Status:   active
Area:     rt
Source:   developer/triggers-auto/events/table/devenv-onaftermodifyevent-table-trigger.md
Verdict:  teilweise
Class:    activation

# 0711 Every write raises its table events, and a RecordRef write runs the triggers

**The finding (2026-09-12, run 120).** `Record.Modify()` and `Record.Delete()` -- the statement
forms without `RunTrigger` -- wrote the row and raised nothing, while `Insert()` is `Insert(false)`
and raises its two events. The event pages carry `RunTrigger` as a PARAMETER: "Executed after a
record is modified in the table ... RunTrigger: Specifies whether to execute the code in the event
trigger" -- the event fires either way and the subscriber reads the flag. `API Update Referenced
Fields` assigns a customer's `Payment Terms Id` from `OnBeforeModifyEvent`, and `Customer.Modify()`
left it blank (`API Setup UT`, `TestModifyingWithoutTriggerAssingsRelatedRecordIDs*`, 4 cases);
`Graph Mgt - Purch Cr Memo` drops the aggregate from `OnAfterDeleteEvent`, and
`PurchCrMemoHdr.Delete()` left it standing (`TestDeletePostedCrMemo`, 3 cases across the three
aggregate codeunits). The BaseApp subscribes 65 times to `OnAfterModifyEvent`, 21 to
`OnBeforeModifyEvent`, 138 to `OnAfterDeleteEvent`, 26 to `OnBeforeDeleteEvent`.

**The predecessor** raises both pairs from `modify()` and `delete()` whatever the flag
(`_fire_dml_event`, `_table.py`), and keeps `DeleteAll()` without a trigger a set-based DELETE that
raises nothing; `record-deleteall-method.md` says only that `RunTrigger` "only affects the
OnDelete trigger". This tree takes the same line: `Modify()` is `Modify(false)`, `Delete()` is
`Delete(false)`, `DeleteAll()` and `ModifyAll()` stay set-based.

**Still open here: a `RecordRef` write runs nothing.** `RecordRef::Insert/Modify/Delete(RunTrigger)`
discard the flag and write the row (`src/rt/RecordRef.cpp`), so a table's `OnInsert`, its
extension triggers and every table event are skipped through that door. The catalogue's
`TableEntry` knows the type; the shape is a type-erased `run` per trigger beside `make`/`free`.

**Class: activation** -- subscribers start running on every `Modify()` and `Delete()` in the
BaseApp; measured with the round (run 122 against run 121), taken back on a loss with the list.
