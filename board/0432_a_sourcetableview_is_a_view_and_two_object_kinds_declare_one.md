Type:     task
Status:   open
Parent:   0018
Area:     al, gen, rt
Source:   developer/properties/devenv-sourcetableview-property.md, developer/properties/devenv-sourcetableview-pages-property.md, developer/properties/devenv-sourcetableview-xmlports-property.md
Verdict:  fehlt
Class:    activation

# A `SourceTableView` is a view, and two object kinds declare one

**Three pages, one item**: an overview page and one page per object kind, exactly as `Scope`
(board:0361) and `OptionMembers` (board:0397) are shaped. The syntax is identical for both kinds and
only the object differs, so unlike `Scope` -- whose two uses mean unrelated things -- this really is
one property.

> **On pages**: Specifies the key, sort order, and filters that define the view of the source table.
>
> ```al
> SourceTableView = sorting(Name) order(descending)
>     where("Balance (LCY)" = filter(>= 50000), "Sales (LCY)" = filter(<> 0));
> ```
>
> **On XMLport table elements**: the same. `SourceTableView = sorting(Code);`

**The value is a VIEW in AL's own sense** -- what `SetView` accepts and `GetView` returns (board:0018)
-- so this property is a declared `SetView` applied before the page or the XMLport reads anything.
Nothing new is parsed: `sorting` names a key (board:0045), `where` carries filter expressions
(board:0018's language), `order` is a direction.

It is the page-level twin of board:0430's `SubPageView`, which has the same grammar on a part.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SourceTableView =`: **1 275 declarations**, pages and XMLport table elements together. They are not
separable by `grep`; the split is counted by file extension when the item is pulled. **That is a limit
of the measurement, stated rather than rounded.**

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` and not its view; board:0018's filter parser does not
exist; XMLports have no generator (board:0065).

## The choice

Parsed by the generator into the same `constexpr` view representation board:0430 uses -- one type, one
parser, three consumers (`SourceTableView`, `SubPageView`, and whatever board:0018's `SetView`
becomes).

**A `sorting` naming a key the table does not declare is a `static_assert`**, and it is the cheapest
guard against board:0045's "a `SetCurrentKey` onto a key with no index is a sort of the table".

## Ordering

Behind board:0018's filter and view parser. With board:0430.

## Gate, and its negative control

A page declaring the documentation's example opens showing only customers with a balance of at least
50 000, sorted by name descending.

**The negative control is the `order(descending)`** -- an implementation that applies the filter and
ignores the direction returns the same ROWS in the wrong order, and a set-equality gate passes.
