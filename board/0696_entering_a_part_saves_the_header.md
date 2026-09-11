# 0696 Entering a part saves the header

**The finding.** 24 UT cases fail with
`Document No. must have a value in Sales Line: Document Type='Quote', Document No.='', Line No.='0'`
on a CREDIT MEMO -- the type is the enum's first member and the number is blank, which is what a
line gets when the header it belongs to has never been written.

**The AL is the BaseApp's own pattern:**

```al
SalesCreditMemo.OpenNew();
SalesCreditMemo."Sell-to Customer Name".SetValue(Customer."No.");
SalesCreditMemo.SalesLines.Last();
SalesCreditMemo.SalesLines."No.".SetValue(ItemNo);
```

The line needs the header's `Document No.`, and that number comes from the No. Series in the
header's `OnInsert`. BC writes the new record when focus LEAVES the header -- entering the lines
part is such a move -- so by the time the part is read the document has a number and a type.

**The choice.** `LinkPart` saves a pending new record first: the page's `OnInsertRecord` when it
declares one (133 pages do), then `Insert(true)` so `OnInsert` runs and the series assigns. A page
with nothing pending is untouched, and a page whose `OnInsertRecord` answers false is left alone,
which is what a trigger refusing the insert means.

**Classification: activation.** A record that was never written now is, inside every test that
opens a document page and reaches its lines, so the A/B is over the whole suite.

**Measured.** Chain 121, A/B against chain 120's 1 741.

**Chain 121 measured the save at 1 741 -- NO CHANGE -- and that is the useful part of the
measurement.** Saving the header was necessary and not sufficient: the test does
`SalesLines.Last(); SalesLines.Next();` and the line it then writes to is BC's BLANK LAST ROW --
the new-row placeholder an editable list carries after the last record. Our `Next()` answered false
at the end and left the part where it was, so nothing was seeded and the fields stayed at their
defaults: `Document Type='Quote'` is the enum's first member, not the header's type.

So `Next()` past the last row on an EDITABLE list lands on a new row -- `New()`, which seeds the
key from the filters (board:0681) -- and answers true, because in BC there IS a row there. The
property is read as AL wrote it: `Editable = false` or `InsertAllowed = false` means no such row.

**Both halves are needed and neither shows without the other**, which is why the first measurement
looked like a no-op rather than a mistake.

**Chain 122 measured the blank-row rule at 1 739 -- MINUS TWO, and the shape it was aimed at did
not move.** It is TAKEN BACK: `Next()` answers what the data says again. The two it cost were tests
asserting there is no next row (`Assert.IsFalse failed. Next Variant is found`), which is the
control this needed and did not have.

**What the type actually comes from.** `Sales Credit Memo` declares
`SourceTableView = WHERE("Document Type" = FILTER("Credit Memo"))`, and nothing applied it: the
page opened with no view, `Init` left the enum's first member, and every line inherited
`Document Type='Quote'`. A page now opens ON ITS OWN VIEW, and a record the page opens NEW takes
what that view fixes -- the same single-value-filter seeding board:0681 gave a new row, now given
to the page's own record.

**Three tries on one shape, and the board carries all three**: the header save (neutral, kept
because BC saves on leaving the header), the blank last row (-2, taken back), the page's view (this
one). The lesson is that `Document Type='Quote'` named the cause all along -- the enum's first
member is what a record that never met a filter looks like.

**Chain 123 measured the page's view at 1 729 -- MINUS TWELVE -- and it is TAKEN BACK too.** One
case fixed, thirteen lost: eleven document TOTALS moved, and two number series changed prefix
(`Expected:<SOIN>. Actual:<SCIN>`), which says applying the view seeded a Document Type on pages
whose tests expect the record to start empty. Whatever BC does with `SourceTableView` on a new
record, it is not "narrow the record and seed from the narrowing" -- the filter belongs to what the
page SHOWS, and the type of a new document comes from somewhere else.

**Three hypotheses, three measurements, one shape still open:**

| tried | measured | kept |
|---|---|---|
| the header is saved when a part is entered | 1 741, no change | yes -- BC saves on leaving the header |
| `Next()` past the last row lands on the blank row | 1 739, -2 | no |
| the page opens on its `SourceTableView` and seeds from it | 1 729, -12 | no |

What is left to test is where `Document Type` comes from on a NEW document page -- the candidates
are `OnOpenPage` (which the generated page runs and which sets it in AL for some documents), the
page's `SourceTableView` read as a FILTER GROUP rather than as the record's own view, and the
caller's `SetRange` before `OpenNew`. The next round takes one of those with a control that can
tell them apart, rather than a fourth guess.
