# A page variable runs its own object, and a modal page hands its record back

**Finding (2026-09-10).** `ItemList.SetTableView(Item); ItemList.LookupMode(true); if
ItemList.RunModal() = Action::LookupOK then ItemList.GetRecord(Item)` is the BaseApp's lookup
idiom: 1 216 `RunModal()` calls on a page variable, 499 `SetTableView`, 438 `LookupMode`, 225
`GetRecord`. Every one refused with "needs a running UI (board:0030)" -- `RunModal()` was the
static form and ran a FRESH page, so the variable's record and filters never reached it. And
`if Page.RunModal(0, Item) = Action::LookupOK then ... Item."No."` read the record the caller
passed in, never the one the handler picked, because the runner took it by const reference.

**Reference.** `page-runmodal-method.md`, `page-gettrecord-method.md`, `page-settableview-method.md`,
`page-lookupmode-method.md`: the variable's page runs; `RunModal(Page, var Record)` passes the
record by reference. The predecessor kept a page variable as an object with its own `rec` and
`run_modal` on that object (openerp, `runtime/page`), which is the shape taken here.

**Choice.** `Page<Derived>` gets non-static `Run()` / `RunModal()` running THIS object through
the handler (`detail::RunInstance`), `SetTableView` copies filters onto `Rec`, `GetRecord` /
`SetRecord` copy the record, `LookupMode` is a flag the page carries, `SaveRecord` inserts or
modifies `Rec`, `SetSelectionFilter` puts the current record's key as a filter on the caller's
record. The static `Run(args...)` / `RunModal(args...)` keep the fresh-page form and, on close,
give the page's record back to a `var` argument (`GiveBackRecord`); the catalogue's `run` says
whether the caller's record is writable. A `Trap` on a page variable's `Run()` drives the object
without owning it. `TestField.Lookup()` on a control with no `OnLookup` opens the related table's
lookup page (`FindLookupPage`: `LookupPageId`, else the first List page on the table) and puts
the picked key back -- for a SIMPLE `TableRelation` only.

**Open.** A conditional or filtered `TableRelation` (`if (...) T.F where(...) else ...`) carries no
`relationTable` yet, so its lookup still refuses; and a TABLE field's own `OnLookup` trigger (549
in the BaseApp) is not emitted by the generator, so a control without one falls straight to the
relation. Both are the next items on this shape. Activation, measured by the milestone.

**2026-09-10, chain 86.** The round measured GAINED 74 / LOST 1 -- and one crash: a page variable
LOCAL to an action ran non-modally under a `Trap`, the harness kept the pointer, and the object
died with the action (`WF Buffer Table/Page UT`, SIGSEGV). A pending trap now takes a COPY the
harness owns (`RunInstance`, `TrapPending`); a page that cannot be copied is still handed over
unowned. The one LOST case, `GenJnlLineDocNoGapWithNoSeries`, sees `Gen. Journal Batch.TestField
("Bal. Account Type")` refuse where a No. Series error was expected -- traced to
`Export Payment File (Yes/No).OnRun`, root still open.
