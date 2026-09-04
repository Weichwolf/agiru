Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-multiplicity-property.md
Verdict:  fehlt
Class:    activation

# A page part declares how many entities it holds

> **Version**: runtime 6.3. Applies to: **Page Part.**
>
> `ZeroOrOne` -- zero or one entity. `Many` -- any number of entities.
>
> ```al
> part(carModels; "API Car Model")
> {
>     Multiplicity = Many;
>     EntityName = 'carModel';
>     EntitySetName = 'carModels';
>     SubPageLink = "Brand Id" = Field(SystemId);
> }
> ```

**The example places it**: this is an API page's part, and the property is what makes the nested entity
a collection or a single reference in the OData document. It sits with board:0390's `EntityName` and
`EntitySetName` -- the example declares all three -- and with board:0430's `SubPageLink`.

So it is not a UI property despite applying to a page part: `ZeroOrOne` versus `Many` is the
difference between `"vendor": {...}` and `"carModels": [...]`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Multiplicity =`: **83 declarations.**

Against board:0429's 374 API pages -- so a fifth of them expose a nested entity.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; no OData surface exists.

## The choice

A two-valued enumerator on the part descriptor, carried with board:0390's names. It has no consumer
until an API surface exists, and it is carried rather than refused because 83 declarations are the
shape of an API document somebody will need.

## Ordering

With board:0390. Behind any API work, which has no board item.

## Gate, and its negative control

A part declaring `Many` carries that value beside its `EntitySetName`; one declaring `ZeroOrOne`
carries the singular.

**The negative control is the pairing** -- a `Many` part with only an `EntityName` and no
`EntitySetName` is a declaration whose collection has no name, and the assertion belongs here because
board:0390 checks the names and this checks the multiplicity that needs them.
