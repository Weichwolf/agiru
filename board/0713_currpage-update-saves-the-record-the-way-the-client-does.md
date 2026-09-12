Type:     task
Status:   open
Parent:   0030
Area:     rt
Source:   developer/methods-auto/page/page-update-method.md, developer/methods-auto/page/page-saverecord-method.md, developer/properties/devenv-delayedinsert-property.md
Verdict:  deklariert
Class:    activation

# 0713 `CurrPage.Update(SaveRecord)` saves the record the way the client does

**The finding (2026-09-12, read while tracing Price List Line UT).** `Page::Update(SaveRecord)`
in `include/runtime/Page.h` discards its argument: "until the row-leave exists, `SaveRecord` is
carried and acted on by nothing". The row-leave exists now (`TestPage::SaveEditedNewRecord_`,
board:0711's round), so the half that is missing is the platform's: `page-update-method.md` --
"Saves the current record and then updates the controls on the page" -- and its remark that the
default of `SaveRecord` is TRUE when the page has a `SourceTable` and false otherwise.

**Population, measured 2026-09-12 over the transpiled scope (openerp WI-1330 counted the same):**
`CurrPage.Update()` without a parameter **904** sites, `CurrPage.Update(true)` **318**. A page
materialises a record the user has only started typing into this way (`Dimension Correction
Draft.OnValidate`: `if Rec.Description <> '' then CurrPage.Update(true)`, which assigns the
AutoIncrement key), and a page total is refreshed through it (`UpdatePropagation = Both`, 145
part controls).

**What the predecessor paid for, read before building.**
- WI-1395 (done, +1): on a `DelayedInsert = true` page `SaveRecord` and `Update(true)` do NOT
  insert a row the user has begun -- the client inserts on leaving the row, and the BaseApp's own
  insert path (`OnInsertRecord -> ValidateAndInsert`, which starts with `if Rec."Entry No." <> 0
  then exit(false)`) is destroyed if the row is written earlier.
- WI-1379 (done): an untouched blank row is not written by `SaveRecord`.
- WI-1401 (rejected, -3 +1): the SECOND half of the documentation, "and then updates the
  controls", was measured net negative -- the Document Totals refresh cleared its totals between
  the clearing and the recalculation. The save half stands; the refresh half is taken back.
- WI-1330 (open): `UpdatePropagation` is unknown to the runtime.

**The shape.** `Page::Update(SaveRecord)`: when `SaveRecord` (or when it is defaulted and the
page has a `Rec`), `SaveRecord()`. `Page::SaveRecord()` modifies an existing row through
`OnModifyRecord` and its event, inserts a NEW row through `OnInsertRecord` and its event -- the
sequence `TestPage::SaveNewRecord_` runs today, which moves into the page so both doors share one
-- and does NOT insert on a page with `PageTraits<P>::kPage.delayedInsert`. A row the harness
marked `edited_` and the page then saved is not saved twice (`StandsOnNewRecord`).

**The gate, and its negative control.** A card page whose field trigger calls
`CurrPage.Update(true)`: after `SetValue` the row is in the database with the platform's key. The
control: the same on a `DelayedInsert` list part, where the row is NOT there until the row is
left.

**Class: activation.** 1 222 sites go from a no-op to a write; a full A/B, and on a loss the list
names the deeper roots first.
