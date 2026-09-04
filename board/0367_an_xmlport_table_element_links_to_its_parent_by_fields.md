Type:     task
Status:   open
Parent:   0065
Area:     rt, gen
Source:   developer/properties/devenv-linktable-property.md, developer/properties/devenv-linkfields-property.md, developer/properties/devenv-linktableforceinsert-property.md
Verdict:  fehlt
Class:    activation

# An XMLport table element links to its parent by fields

**Three pages, one item.** Each says so of the other two: "This property works in combination with
the `LinkFields` property and the `LinkTableForceInsert` property." They describe one mechanism --
how a nested XML element's table is joined to its parent's -- and none of them is implementable alone.

> **LinkTable** (Xml Port Table Element): Sets the table that this XML item should be linked to.
>
> **LinkFields**: which field of this table equals which field of the linked one.
>
> **LinkTableForceInsert**: **True** if you want to forcibly insert or modify data; **the default is
> true**. Setting this to true will forcibly insert or modify data from the linked table **and run
> the OnAfterInitRecord trigger on the main table**.
>
> This is useful if you have a **header to line relationship** in your XML document. The table and
> the header information must be inserted before you can insert the line information.

The page's own export example is the shape: a `Customer` element with a nested `Sales Header`
element, linked on `Sell-to Customer No. = No.`, so only that customer's orders are exported.

**On IMPORT the same three properties become an ordering rule**, and that is the half with teeth: the
header row must be in the database before the line rows reference it, so `LinkTableForceInsert`
decides whether the header is inserted as a side effect of reading the lines.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`LinkTable =` **115** · `LinkFields =` **113** · `LinkTableForceInsert =` **1**.

`LinkTable` and `LinkFields` track each other, which confirms the pairing. The single
`LinkTableForceInsert` is a deliberate `false`, since `true` is the default.

## The IST-state

XMLports are not generated (board:0065, board:0034).

## The choice

The link is a FILTER on the child element's read, resolved by the generator into a span of
`{ child FieldNo, parent FieldNo }` -- the same shape board:0331's `TableRelation` terms take, and
for the same reason: nothing about it needs deciding while an import is running.

`LinkTableForceInsert` is one bit on the element and the ordering it implies is the XMLport writer's,
not the runtime's.

**A `LinkFields` naming a field neither table has is a `static_assert`.**

## Ordering

Inside board:0065, with the table element itself.

## Gate, and its negative control

An export of a linked pair emits only the child rows whose link field matches the parent; an import
of a header/line document inserts the header before the lines.

**The negative control is the import order** -- an implementation that reads the elements in document
order without the forced insert fails on the first line with a relation error, and only an import
gate sees it; an export gate passes either way.
