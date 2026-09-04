Type:     task
Status:   open
Parent:   0034
Area:     gen, rt
Source:   developer/properties/devenv-apipublisher-property.md, developer/properties/devenv-apipublisher-page-property.md, developer/properties/devenv-apipublisher-query-property.md, developer/properties/devenv-apigroup-property.md, developer/properties/devenv-apigroup-page-property.md, developer/properties/devenv-apigroup-query-property.md, developer/properties/devenv-apiversion-property.md, developer/properties/devenv-apiversion-page-property.md, developer/properties/devenv-apiversion-query-property.md, developer/properties/devenv-odatakeyfields-property.md, developer/properties/devenv-odataedmtype-property.md
Verdict:  fehlt
Class:    activation

# An API object is addressed by publisher, group and version

**Eleven pages, one item**: three properties each with an overview page and one page per object kind
(page, query), plus the two OData properties. They are the URL an API object is published at, and no
part of it is separable -- an endpoint with a group and no version has no address.

> **APIPublisher**, **APIGroup**, **APIVersion**: applied to **API pages** and **API queries**. The
> three compose into the endpoint path.
>
> **ODataKeyFields** (Page): the fields to select when using OData. `ODataKeyFields = Id, Number;`
> **Dependent property: `SourceTable`.**
>
> **ODataEDMType** (Page Field): **deprecated in runtime 6.0.** The Entity Data Model type for the
> node in the OData metadata.

**agiru has no OData surface**, so what this item delivers is that the declarations survive translation
and are checkable -- exactly board:0390's position for `EntityName` and `EntitySetName`, which are the
same endpoint's other half.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`APIVersion =` **481** · `APIPublisher =` **295** · `APIGroup =` **295** · `ODataKeyFields =` **349**
· `ODataEDMType =` **34**.

**`APIPublisher` and `APIGroup` match exactly at 295** -- the two are declared together, as
board:0390's `EntityName` and `EntitySetName` are at 854. And `APIVersion` at 481 exceeds both, which
means 186 API objects declare a version with Microsoft's own default publisher and group.

Against board:0429's 374 `PageType = API` and board:0464's 346 `QueryType = API`: **720 API objects
and 481 version declarations**, so a version default exists and is not on these pages.

## The IST-state

No OData surface; `src/gen/PageWriter.cpp` consumes `SourceTable` alone; queries have no generator.

## The choice

Three `string_view`s and a span of `FieldNo` on the page and query descriptors, `constexpr`, composed
by the generator into the endpoint path so nothing builds a URL at run time.

**`ODataEDMType` is refused**: deprecated in runtime 6.0, 34 declarations, and it describes an OData
metadata document that does not exist. That is the sweep's one refusal of a NON-zero property, and the
reason is the deprecation rather than the count.

## Ordering

Behind board:0034's query generator. Ahead of any API work, which has no board item.

## Gate, and its negative control

An API page declaring publisher, group and version carries a composed endpoint path in its descriptor;
one declaring `ODataEDMType` fails to transpile.

**The negative control is an API page declaring only a version** -- the 186 that do must still get a
path, from whatever default applies, and an implementation that requires all three drops them.
