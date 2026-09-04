Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/properties/devenv-datacaptionfields-property.md
Verdict:  fehlt
Class:    activation

# `DataCaptionFields` names what stands beside a page caption

> Sets the fields that appear **to the left of the caption** on pages that display the contents of
> this table. Applies to: **Table, Page, Request Page.**

It is what turns the browser tab from "Customer Card" into "10000 · Adatum Corporation". The rules
are per page type and they are not symmetrical:

**Card pages** -- one record at a time:

> The value is taken from the **underlying table**. **Any value set for this property on the page
> itself is ignored.** If the property isn't defined on the table, **the primary key** is used as a
> fallback.

**Tabular pages** -- many records:

> A data caption is shown **only if a filter applied to the fields defined in the property evaluates
> to a single value**. **With a table relation**: the `DataCaptionFields` from the underlying table
> is used; if the related table doesn't define it, the primary key. **Without a table relation**: the
> single value resulting from the filter is used directly.

Three things follow, and each is a place an implementation goes wrong:

1. **On a card page the page's own declaration is DEAD.** A renderer that preferred the more specific
   declaration -- the obvious choice, and the rule everywhere else, including `ExtendedDataType`
   (board:0329) -- would be wrong here and only here.
2. **The fallback is the primary key**, not nothing. A table declaring no caption fields still has a
   caption.
3. **On a tabular page the caption depends on the FILTER**, not on the record. It appears and
   disappears as the user filters, which makes it a render-time computation over the filter state and
   not a property read.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DataCaptionFields =`: **1 326 declarations.**

## The IST-state

Pages carry no metadata beyond `SourceTable` (`src/gen/PageWriter.cpp`); `TableDef` has no caption
field list.

## The choice

A span of `FieldNo` on `TableDef` and on the page, resolved by the generator. The **card rule is
where the two meet**: the page's list is emitted and then never read, or -- better -- the generator
refuses to emit it on a card page at all, so the dead declaration is visible at translation time
rather than silently ignored at render time.

The tabular rule needs the filter, so it belongs to board:0030's renderer and reads board:0331's
relation for the related table's list.

## Ordering

Behind board:0030. The tabular half additionally behind board:0331 and board:0018's filter state.

## Gate, and its negative control

A card page shows the TABLE's caption fields even when the page declares different ones; a table
declaring none shows its primary key.

**The negative control is the page's own declaration on a card page** -- it must have NO effect, and
an implementation that honours the more specific declaration passes every gate that does not set the
two differently.
