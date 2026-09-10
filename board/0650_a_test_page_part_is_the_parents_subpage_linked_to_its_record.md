# A test page part is the parent's subpage, linked to its record

**Finding (2026-09-10).** `PurchaseCreditMemo.PurchLines.Last()` refused with "a TestPage needs
a running page" (48 UT cases), `SalesInvoice.SalesLines."Invoice Discount Amount"` with "the
control is not on a running page" (15), and `GoToRecord(Variant)` / `Page.SetRecord` refused
outright (33 + 23): a `part` control is a nested `TestPage` in the door, but nothing opened it
when the parent opened, and nothing kept it on the parent's lines.

**Reference.** `devenv-subpagelink-property.md`: a part's records are filtered by `SubPageLink`
(`Field = field(Parent Field)`, `= const(Value)`, `= filter(...)`) from the parent's current
record, and the link follows the parent as it moves. The generated page already carries the
part's `ControlDef.subPageLink` and a `PartRef<SubPage>` the page's own code opens lazily.

**Choice.** The generated page answers `PartInstance(name)` with its `PartRef`'s instance; the
nested test page attaches to it on first use (`BindPart`, emitted in `BindControls`), applies
the link through `detail::ApplySubPageLink` (`src/rt/SubPageLink.cpp`, over the field tables and
the catalogue for `Database::X`), opens the subpage, and relinks before every navigation and
value set so the part follows the parent. `GoToRecord(Variant)` positions by the Variant's
record id; `Page.SetRecord` assigns the page's `Rec`. Activation; measured by the milestone.
