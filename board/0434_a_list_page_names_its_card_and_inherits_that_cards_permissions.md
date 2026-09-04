Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-cardpageid-property.md
Verdict:  fehlt
Class:    activation

# A list page names its card, and takes that card's New, Delete and Edit

> Sets the card page that is associated with items in the current list page. Applies to: **Page,
> Request Page.** Use it on a list or listpart page to specify the card shown when a user
> double-clicks an item.
>
> **When you set `CardPageId` on a list page, the runtime uses the LINKED CARD PAGE to determine
> which record-level actions are available on the list. Specifically, the New, Delete and Edit
> actions on the list page are controlled by the `InsertAllowed`, `DeleteAllowed` and `ModifyAllowed`
> properties ON THE CARD PAGE, not the list page itself.**
>
> For example, if your list page is non-editable (`Editable = false`) but the card page has
> `DeleteAllowed = true`, **the Delete action still appears on the list page**.

**That is the item, and it is not a navigation detail.** board:0403 files `InsertAllowed`,
`ModifyAllowed` and `DeleteAllowed` as page properties; this page says that on a list with a
`CardPageId` they are read from a DIFFERENT page. An implementation that reads the list's own three
bits gets 599 list pages wrong, and the failure is an action that appears or disappears -- not an
error.

And a merge rule from 2025 wave 1:

> You can modify `CardPageID` through a page extension. **If the property is already specified on the
> base page, the value in the page extension OVERRIDES it. If multiple page extensions modify the
> property, THE LAST EXTENSION TO BE APPLIED takes effect.**

**Last-writer-wins across extensions is order-dependent**, and board:0033 merges extensions at
translation time -- so the apply order has to be declared and deterministic, which is CLAUDE.md's
determinism invariant applied to a merge.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`CardPageId =`: **599 declarations.**

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; board:0403's three bits do not exist either.

## The choice

A `PageId` on the list page descriptor, resolved by the generator, **and the three action bits
resolved FROM THE CARD at translation time** -- both pages are declarations, so the list's effective
New/Delete/Edit is a constant and the renderer never follows a link.

**A `CardPageId` naming a page that is not a `Card` is a `static_assert`** once board:0429 puts the
type in the metadata.

## Ordering

With board:0403 -- it decides where those three bits are read from -- and board:0429 for the
assertion. Behind board:0033 for the extension override order.

## Gate, and its negative control

A non-editable list page whose card declares `DeleteAllowed = true` shows the Delete action.

**The negative control is the list's own `DeleteAllowed = false`** -- it must have NO effect when a
`CardPageId` is set, and an implementation reading the list's bits passes every gate where the two
pages agree.
