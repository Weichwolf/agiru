Type:     task
Status:   open
Parent:   0065
Area:     gen, rt
Source:   developer/properties/devenv-minoccurs-property.md, developer/properties/devenv-maxoccurs-property.md, developer/properties/devenv-occurrence-property.md
Verdict:  fehlt
Class:    activation

# An XMLport element declares how often it may occur

**Three pages, one item**: `MinOccurs` and `MaxOccurs` are the occurrence constraint on an ELEMENT and
`Occurrence` is the same constraint on an ATTRIBUTE. One concept, two node classes, and the pages
reference each other.

> **MinOccurs**: `Zero` or `Once`. **MaxOccurs**: `Once` or `Unbounded`. **"The `MinOccurs` and
> `MaxOccurs` properties conform to the standard occurrence constraints that are used when defining
> XML schemas."**
>
> **`MaxOccurs` defaults depend on the source type:** Table -- `Unbounded`; Text -- `Unbounded`;
> **Field -- `Once`.**
>
> **Occurrence** (attributes): `Required` (**the default**) or `Optional`. "Primarily used to ensure
> that the XML data that you are importing conforms to the data structure."

**The per-source-type default is the trap.** One property, three defaults, decided by what the element
is bound to -- so a single `MaxOccurs = Unbounded` initialiser is wrong for every field element, and a
single `Once` is wrong for every table and text element.

**And these are XSD occurrence constraints by the documentation's own words**, which means an
XMLport's structure maps onto a schema -- relevant to `InlineSchema` (board:0446) and to whether an
import validates before or while it reads.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`MinOccurs =` **887** · `MaxOccurs =` **227** · `Occurrence =` **502**.

`MinOccurs` at 887 against `MaxOccurs`'s 227: elements far more often relax the minimum than the
maximum, because the maximum's default is already right for the common table element.

## The IST-state

XMLports have no generator (board:0065, board:0034).

## The choice

Two enumerators on the element and one on the attribute, with the three defaults resolved by the
GENERATOR from the source type -- so the descriptor carries an answer and never a "it depends".

On import the constraint is checked and a violation raises with BC's wording (board:0055); on export
it is an assertion about what the writer produced.

## Ordering

Inside board:0065, with the element kinds. Behind board:0442's format -- occurrence is an XML concept
and a text XMLport has none.

## Gate, and its negative control

An import missing a `MinOccurs = Once` element raises; an import missing a `MinOccurs = Zero` element
does not; an import with two of a `MaxOccurs = Once` element raises.

**The negative control is a FIELD element with no `MaxOccurs`** -- its default is `Once`, so two of
them must raise, and an implementation with one shared `Unbounded` default accepts them silently.
