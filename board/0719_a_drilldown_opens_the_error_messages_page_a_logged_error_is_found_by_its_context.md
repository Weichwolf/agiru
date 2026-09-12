Type:     task
Parent:   0030
Area:     rt
Source:   ~/Git/BCApps/src/Layers/W1/Tests/Data Exchange/IncomingDocToDataExchUT.Codeunit.al; ErrorMessage.Table.al:657
Class:    silent-wrong-data

# A DrillDown opens the Error Messages page, and a logged error is found by its context

With board:0718's TryFunction wrap in place (a table's own `[TryFunction]` called `if not X()` now
catches), `Incoming Document.CreateWithDataExchange` reaches `SetProcessFailed('')`, which reads
`GetLastErrorText` and `ErrorMessage.LogSimpleMessage(Error, ...)` under `SetContext(Rec.RecordId)`.
The test then asserts the error through a drilldown:

```al
ErrorMessages.Trap();
IncomingDocuments.StatusField.DrillDown();   // OnDrillDown -> ErrorMessage.ShowErrorMessages(false)
Assert.IsTrue(ErrorMessages.FindFirstField(Description, CannotCreateErr));
```

This tree answers `The TestPage is not open` -- `ShowErrorMessages` did `SetContextFilter();
SetRange(Context, false); if IsEmpty then exit;` and found NO rows, so it never ran the trapped
`Error Messages` page. So either the error was not logged to the `Error Message` table under the
incoming document's `RecordId`, or the context filter that `ShowErrorMessages` rebuilds does not
match the `RecordId` that `SetProcessFailed` logged under. The two are `Context Record ID` (a
`RecordId` field) filtered by `SetContext`/`SetContextFilter`; a `RecordId` filter that renders
differently on write and on read would miss (\see the RecordId storage-form rule, board:0018).

**Next:** trace `TestProcessWithDataExchSucceeds` with `AGIRU_TRACE_SQL=2` after board:0718 lands --
does an `INSERT INTO "Error Message"` happen, and does the `SELECT` the drilldown issues carry the
same `Context Record ID`? That decides between "the log never happened" and "the filter misses".
Then a TestPage `DrillDown` on a field must run a trapped page the trigger opens, which is the
other half (board:0030's TestPage surface).

This is the real blocker of the Incoming Doc. To Data Exch.UT family (21 cases) once the throw is
gone.

## comment (2026-09-12)

This gap sits BEHIND board:0718: reaching the drilldown needs batch218's TryFunction wrap (so the
`Cannot create` error is caught and logged rather than thrown), and 218 is reverted this round
because its codegen heap-shift tips board:0718's item-tracking use-after-free (-7 on SCM). So the
throw is back for now; this item unblocks only once 218 can ship (after board:0718), and is recorded
so the second half is not rediscovered. The trace it asks for is still the right first step once 218
is in.
