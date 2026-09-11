# 0700 A page opens on its own view, and a trigger defaults as the documentation says

**The shape: 24 UT cases,** `Document No. must have a value in Sales Line: Document Type='Quote',
Document No.='', Line No.='0'`. Three hypotheses were tried and taken back before this one
(board:0696, deleted with this item: the header save -- neutral, kept; `Next()` onto a blank last
row -- minus two; the page's view seeded on EVERY open -- minus twelve).

**Two defects, and the case needed both.**

1. **A SUBFORM carries its own `SourceTableView` and nothing applied it.** `Sales Credit Memo`
   links its lines part with `SubPageLink = "Document No." = field("No.")` -- the link names the
   NUMBER and not the TYPE, because page 96's own view fixes the type:
   `where("Document Type" = filter("Credit Memo"))`. So the view is applied on every page open and
   on every relink, in FILTER GROUP 2 (`record-filtergroup-method.md` tabulates `SourceTableView`
   and `DataItemTableView` there, under `Form`), and a record the page opens NEW is seeded from it.
   Group 2 is what makes this safe where chain 123 was not: the filters a caller set in group 0
   survive, and `GetFilters` answers what the caller asked.
   A view term's value is also TRIMMED now -- the property arrives spelled
   `where ( Document Type = filter ( Credit Memo ) )`, and ` Credit Memo ` matched no option
   member, so the filter was there and selected nothing.

2. **`OnInsertRecord` returned FALSE by default, so no page ever inserted.**
   `devenv-oninsertrecord-page-trigger.md`: "The return value is checked after each call. **The
   default value is true**." The generator knew that rule for `OnQueryClosePage` alone. It is
   documented for five triggers -- `OnInsertRecord`, `OnModifyRecord`, `OnDeleteRecord`,
   `OnQueryClosePage` and `OnBeforeTestRun` -- and the BaseApp's pages rely on it: `Sales Credit
   Memo`'s `OnInsertRecord` has no `exit` at all, so every save through a test page was refused and
   the header never got its number from the series.

**Measured, whole suite: 1 775 -> 1 777 (+2, 0 new red).** The number is small and the CHAIN is
what moved: the 24 cases now reach `The Standard Text does not exist` -- a line whose `Type` is the
enum's first member (`" "`, Comment) instead of `Item`, which is the next link and a different gap.

**The lesson, again:** a trigger's return value is a PLATFORM guarantee and the documentation
states it per trigger. Reading one page and generalising from it is how four of the five were
missed.

**Third half, and the one that paid: a linked part positioned on NO ROW is on a NEW row.**
`SalesLines.Last(); SalesLines.Next(); SalesLines."No.".SetValue(ItemNo)` -- the test never sets
`Type`, because page 96's `OnNewRecord` calls `Rec.InitType()`. So when a relink finds no row, the
part seeds from its filters AND runs `OnNewRecord`, which is what BC does for the blank row an
editable list carries.

This is the same ground chain 122 lost two cases on, approached from the other side: `Next()` still
answers what the data says, so the tests asserting "there is no next row" stay green -- what changed
is what a part's record IS when nothing was found, not what `Next()` returns.

**Measured: 1 777 -> 1 788 (+11, 0 new red).** `ERM Sales Cr. Memo Aggr. UT` went 21 -> 26 of 44.

**A fourth try, measured and TAKEN BACK: `Init()` on the blank row.** `Sales Line."Allow Invoice
Disc."` carries `InitValue = true`, and 12 cases said `Cannot apply an invoice discount because the
document does not include lines where the Allow Invoice Disc. field is selected` -- so the blank row
a part lands on looked like a record that never met `Init`. Calling it there measured **1 788 ->
1 786, minus two and nothing fixed**, and it is out.

Why it is wrong, on the evidence: BC's blank row is not a freshly initialised record. The trigger is
`OnNewRecord(BelowxRec)` and it takes `xRec` -- the row the user was standing on -- so a new sales
line inherits from the PREVIOUS line rather than from the field defaults, and `Init` threw that
inheritance away. `InitValue` still has to reach that row; it reaches it through whatever BC does
with `xRec`, which is the next thing to find out and not a guess to ship.
