Type:     task
Status:   open
Parent:   0064
Area:     gen, rt
Source:   developer/properties/devenv-querytype-property.md, developer/properties/devenv-querycategory-property.md, developer/properties/devenv-columnfilter-property.md, developer/properties/devenv-orderby-property.md
Verdict:  fehlt
Class:    activation

# A query is normal or an API, and a category puts it on a list page

**Four pages, one item**: the query's own kind, the categories that surface it, its per-column filter
and its ordering. They are the remaining query-object declarations, each one value, and each alone a
paragraph. `OrderBy` already has board:0352 for its page-view half; the query half is here.

> **QueryType**: `Normal` or `API`. An API query is a web-service endpoint (board:0368's
> `DataAccessIntent` applies to exactly these).
>
> **QueryCategory** (Query and **Page**): "indicates a given query can be made available as **views
> displayed on certain main entity lists**." `QueryCategory = 'Customer', 'Items';` -- on a query it
> names one or more categories, **on a page it names the category the page supports.**
>
> **ColumnFilter** (Query Column): a filter -- combination rules in board:0453.
>
> **OrderBy** (Query and Page View): board:0352.

**`QueryCategory` is a two-ended binding**: a query names categories, a page names its category, and
the client offers the matching queries as views on that page. So it is not a label -- it is a lookup
from page to query resolved somewhere, and at translation time both ends are declarations.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`QueryType =` **346** · `QueryCategory =` **136** · `ColumnFilter =` **214** · `OrderBy =` **113**.

346 `QueryType` declarations, all necessarily `API` since `Normal` is the default -- so **most queries
in the BaseApp are API endpoints**, not internal datasets, which reorders board:0064: the endpoint is
the common case.

## The IST-state

Queries have no generator (board:0064, board:0034); no OData surface exists.

## The choice

Four fields on the query descriptor, and **the category binding resolved by the generator into a
per-page list** -- both ends are declarations, so the client never searches.

`ColumnFilter` keeps its own span, separate from `DataItemTableFilter`'s, for board:0453's overwrite
rule.

## Ordering

Inside board:0064. The API half is behind whatever OData surface exists, which is nothing.

## Gate, and its negative control

A page declaring `QueryCategory = 'Customer'` offers the queries that declare that category as views.

**The negative control is a query with a category no page declares** -- it must appear nowhere, and an
implementation that lists every query on every page passes the positive gate.
