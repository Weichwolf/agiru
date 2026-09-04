Type:     task
Status:   open
Parent:   0030
Area:     al, gen, rt
Source:   developer/properties/devenv-subpagelink-property.md, developer/properties/devenv-subpageview-property.md
Verdict:  fehlt
Class:    activation

# A page part is linked to its parent and filtered by two grammars

**Two pages, one item**: `SubPageLink` ties a part's rows to the parent record and `SubPageView` sorts
and filters them. They apply to the same three kinds, they compose into one read, and the second page
names the first as its relative.

```
SubPageLink = <TableFilters>
<TableFilter> ::= <PagePartTableFieldName> =
  CONST(<FieldConst>) | FILTER(<Filter>) | FIELD(<SourceFieldName>) |
  FIELD(UPPERLIMIT(<SourceFieldName>)) | FIELD(FILTER(<SourceFieldName>)) |
  FIELD(UPPERLIMIT(FILTER(<SourceFieldName>)))

SubPageView =
  [SORTING(<KeyList>)] [ORDER(Ascending|Descending)] [WHERE(<TableFilters>)]
```

**`SubPageLink`'s term grammar is `CalcFormula`'s, exactly** -- the same six shapes including
`UPPERLIMIT` and `FIELD(FILTER(...))` (board:0340). **`SubPageView`'s is a VIEW**, which is what
`SetView` accepts and `GetView` returns (board:0018), with `SORTING` naming a key (board:0045).

So neither is a new language: this item is two consumers of two parsers that already have to exist,
and building a third parser here would be the mistake.

> **The link is updated when the current record changes.**

That sentence is the runtime half: the part re-reads whenever the parent's record moves, which on a
document page is every arrow key.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SubPageLink =` **3 487** · `SubPageView =` **96**.

Every FactBox and every document-line subform declares a link; almost none declares a view.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone. board:0018's filter parser and board:0340's
`CalcFormula` parser do not exist either, so all three are the same missing work.

## The choice

Both are parsed by the GENERATOR into `constexpr` data on the part descriptor -- link terms as
`{ part FieldNo, kind, parent FieldNo or literal }`, view as `{ key, direction, filter terms }`. The
runtime applies them to the part's record when the parent's record changes and never re-parses a
string.

**One term parser for `TableRelation`, `CalcFormula` and `SubPageLink`.** Three properties, one
grammar, and board:0331 and board:0340 already say so from their side.

## Ordering

Behind board:0018's filter parser. With board:0340, which shares the term grammar. Behind board:0429,
which is what makes a page a part.

## Gate, and its negative control

A document page's line subform shows only the lines of the current header, and moving to another
header changes them.

**The negative control is moving the parent record** -- a part that filters correctly on open and
never re-reads shows the first document's lines under every header, and a single-record gate passes.
