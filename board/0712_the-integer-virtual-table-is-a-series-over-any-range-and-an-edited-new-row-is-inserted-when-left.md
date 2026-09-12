Type:     bug
Status:   active
Area:     rt
Source:   developer/devenv-integer-virtual-table.md
Verdict:  teilweise
Class:    silent-wrong-data

# 0712 The Integer virtual table is a series over any range, and an edited new row is inserted when left

**Two findings of run 121 (2026-09-12).**

1. **`Integer` over an open range read the empty physical table.** `src/rt/Selection.cpp`
   `Series()` turns a filter on `Number` into `generate_series(low, high)` -- but only for a
   bounded range under a million rows; an open end (`Number = filter(1..)`, the shape of the
   PEPPOL xmlports' header loop and 176 BaseApp loops, all breaking out through their own
   iterator) fell back to the physical `"Integer"` table, which holds 0 rows. So `Sales Invoice -
   PEPPOL30` exported its XML declaration and nothing else, and every import of it failed with
   `XmlDocument.LoadXml: the data at the root level is invalid` (19 cases, Incoming Doc. To Data
   Exch.UT). `devenv-integer-virtual-table.md`: the table holds -1 000 000 000 through
   1 000 000 000. The series is now capped at a million rows from the low end of what the filters
   admit, which no AL loop reaches; the cap is the number to argue with, not the physical table.
2. **A new row a test edited was never inserted unless a part relinked.** `TestPage` inserted the
   parent's new record when a part attached (`LinkPart`) and dropped a part's own new row on the
   next navigation (`Landed_` set `newRecord_ = false`). `devenv-delayedinsert-property.md`: the
   record is inserted "when the user leaves the row". So `SalesLines.Quantity.SetValue(q)`
   followed by `SalesLines."Invoice Discount Amount".SetValue(x)` -- a control that is no field
   of the row -- found no line ("Cannot apply an invoice discount because the document does not
   include lines where the Allow Invoice Disc. field is selected", 12 cases of the three
   aggregate codeunits). Now a field edit marks the row (`edited_`), and leaving it -- another
   control, another row, an action, `New`, `Close`, or the PARENT being left, which reaches the
   parts through `PageCore::RowLeft` -- inserts an edited new row and drops an unedited one, the
   predecessor's `_row_edited` rule.

3. **A control with nothing behind it refused, where BC's headless session does nothing.** A
   `usercontrol` runs in the client and the test framework runs its methods as no-ops; a part
   whose page is carved out of the scope (`Power BI Embedded Report Part`, in the excluded
   `System.Integration.PowerBI`) stopped `Job List.OnOpenPage` at `SetPageContext` (9 cases).
   `::agiru::AbsentControl` answers nothing and the caller's default, the predecessor's
   `_NilValue`; `RefusedControl` stays for the loud form.
4. **A table event's `xRec` was `Rec` itself.** `TableEvent` handed the record twice, so `Price
   Helper V16.AfterRenameItem` renamed price lines from the new number to itself (14 cases of
   Price Worksheet Line UT and Price List Line UT). The stored image for a modify, the row under
   the old key for a rename; EventGate carries the case.

**Class: silent-wrong-data** for the series (an empty answer where rows were owed);
**activation** for the insert on leave, measured with run 123 against run 122.
