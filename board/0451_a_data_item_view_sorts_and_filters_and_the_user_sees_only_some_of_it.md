Type:     task
Status:   open
Parent:   0063
Area:     al, gen, rt
Source:   developer/properties/devenv-dataitemtableview-property.md
Verdict:  fehlt
Class:    activation

# A data item's view sorts and filters, and the user sees only some of it

> Sets the key on which to sort, the sort order, and the filters for the data item. Applies to:
> **Report Data Item.**
>
> ```AL
> DataItemTableView = SORTING("Entry No.");
> DataItemTableView = WHERE("Document Type" = FILTER(Payment|Invoice|"Credit Memo"), Open = CONST(true));
> ```
>
> - **If you set a KEY, then the data item does not have a FastTab on the request page and the end
>   users cannot select a key** for sorting, sort order, or filters for the data item.
> - **If you set a SORT ORDER, this order is used regardless of the sort order the user selects.**
> - **If you set a FILTER, this filter is NOT DISPLAYED on the request page but it IS USED** along
>   with any filters the user specifies.
> - Setting a sort order or filter **does not prevent users from selecting a sort field** on the
>   request page. The default sort field displayed is the primary key, and the list includes all keys
>   for the data item. **To add fields to the list, you must add keys to the table.**

**Four rules and three of them are about what the USER can still do**, which is the same
visible/hidden distinction board:0433's `RunPageLink` and `RunPageView` have -- and here all three
outcomes differ: a key REMOVES the request-page FastTab, a sort order OVERRIDES the user, a filter is
INVISIBLE and ANDed with the user's.

The value is an AL view -- board:0432's representation, `SORTING`/`ORDER`/`WHERE` -- so no new parser.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`DataItemTableView =`: **7 710 declarations.**

**Against CLAUDE.md's 668 reports in scope, that is eleven per report** -- and it is the largest
report property in this theme by a factor of four. Every data item in the BaseApp declares its view.

## The IST-state

Reports have no generator (board:0063, board:0034); board:0018's view parser does not exist.

## The choice

One `constexpr` view on the data item, parsed by board:0432's parser, plus **three bits derived from
which parts were declared** -- key present, order present, filter present -- because those three
decide what the request page shows, and deriving them at translation time means the request-page
generator asks one question per data item.

## Ordering

Inside board:0063. Behind board:0018's view parser, with board:0432.

## Gate, and its negative control

A data item declaring `SORTING("Entry No.")` has no sort FastTab on the request page; one declaring
only a filter does, and the user's filter is ANDed with the declared one.

**The negative control is the user's filter** -- an implementation that replaces the declared filter
with the user's produces a larger result set that looks plausible, and only asserting the
intersection catches it.
