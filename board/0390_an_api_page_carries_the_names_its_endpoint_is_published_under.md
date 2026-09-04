Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-entityname-property.md, developer/properties/devenv-entitysetname-property.md, developer/properties/devenv-entitycaption-property.md, developer/properties/devenv-entitysetcaption-property.md
Verdict:  fehlt
Class:    activation

# An API page carries the names its endpoint is published under

**Four pages, one item**: singular name, plural name, singular caption, plural caption. They are one
declaration of how an object appears in the API surface, they apply to the same object kinds, and
none of them means anything without the others.

> **EntityName** (runtime 1.0): the **singular** entity name with which the page is exposed in the
> API endpoint. **The value `EntityMetadata` is reserved and using it will result in a compiler
> error.**
>
> **EntitySetName** (runtime 1.0): the **plural** entity name.
>
> **EntityCaption** / **EntitySetCaption** (runtime 6.0): the caption of the entity and of a set of
> entities.
>
> Applies to: Page, Page Part, Page System Part, Page Chart Part, **Query**.

The reserved-word rule is a `static_assert`: `EntityName = 'EntityMetadata'` is a translation error,
and it is decidable from the declaration.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`EntityName =` **854** · `EntitySetName =` **854** · `EntityCaption =` **287** ·
`EntitySetCaption =` **272**. The `ML` twins are 0 (board:0386).

**The two names match exactly at 854**, which confirms they are one declaration made twice: an API
page names both or neither. The captions are declared on a third of them.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone, and no OData or REST surface exists
(board:0030 is the UI, not the API).

## The choice

Four `string_view`s on the page and query descriptors, `constexpr`, with the `static_assert` for the
reserved name and one for the pairing -- a page declaring one name and not the other is a declaration
BC would publish half of.

**The API surface itself is not this item.** agiru has no OData endpoint and this item does not
propose one; it makes the declarations available so that whoever builds one is not re-reading 854
pages of AL.

## Ordering

With board:0030's page metadata. Ahead of any API work, which has no board item yet.

## Gate, and its negative control

A page declaring the four carries all four in its descriptor; `EntityName = 'EntityMetadata'` fails
to transpile.

**The negative control is a page declaring `EntityName` alone** -- it must fail, and an
implementation that treats the plural as optional accepts 0 such pages today, so the gate needs a
constructed one.
