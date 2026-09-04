Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/properties/devenv-requestfilterfields-property.md, developer/properties/devenv-requestfilterheading-property.md
Verdict:  fehlt
Class:    activation

# A request page offers the filter fields the data item names

**Two pages, one item**: one names the fields on a request-page tab and the other captions that tab.
Both apply to Report Data Item and XMLport Table Element, and the caption is meaningless without the
tab.

> **RequestFilterFields**: which fields are automatically included on the tab of the request page
> related to this data item. **A comma-separated list of field names.**
>
> **"If you DO NOT specify the property, then the request page will display with only the actions
> Send to, Print, Preview and Cancel"** -- and you should instead specify a sort key in
> `DataItemTableView` (board:0451) or `SourceTableView` (board:0432).
>
> **RequestFilterHeading**: a caption for that tab. **"If not set, the default is the NAME OF THE
> TABLE"** specified in `DataItemTable` for a report or `SourceTable` for an XMLport.

**The absent property is a documented state and not an omission**: no `RequestFilterFields` means a
request page with actions and no filter tab, which is different from no request page at all
(board:0436's `UseRequestPage`). Three states from two properties.

The heading carries the same `Locked` / `Comment` / `MaxLength` named arguments board:0389 records for
the whole caption family.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`RequestFilterFields =` **1 944** · `RequestFilterHeading =` **425**.

Against CLAUDE.md's 668 reports, roughly three filter-field declarations per report -- and only a
fifth of them caption the tab, so the table-name default carries the rest.

## The IST-state

Reports and XMLports have no generator (board:0063, board:0065, board:0034).

## The choice

A `constexpr` span of `FieldNo` on the data item and a `string_view` heading, with the table-name
fallback folded by the generator. The request-page generator emits one tab per data item that declares
fields.

**A named field the data item's table does not have is a `static_assert`.**

## Ordering

Inside board:0063 and board:0065, with the request page. Behind board:0431 and board:0451, which
supply the table and its view.

## Gate, and its negative control

A report whose data item names three filter fields shows a tab with those three; one naming none shows
a request page with actions and no tab.

**The negative control is the data item naming none** -- an implementation that falls back to the
table's primary key offers a filter tab BC does not show, and every gate on a declaring data item
passes.
