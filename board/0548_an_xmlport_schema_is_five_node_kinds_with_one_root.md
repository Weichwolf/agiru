Type:     task
Status:   open
Parent:   0065
Area:     gen, rt
Source:   developer/devenv-xmlport-schema.md, developer/devenv-xmlport-overview.md, developer/devenv-xmlport-object.md, developer/devenv-using-namespaces-with-xmlports.md
Verdict:  fehlt
Class:    activation

# An XMLport schema is five node kinds with exactly one root

**Four pages, one item**: the schema grammar, the object overview, the object page and the namespace
guide. board:0065 owns XMLports and the property sweep filed seven items against their properties
(board:0442-0448); **this is the OBJECT those properties sit on.**

## Five node kinds

| keyword | maps to | |
|---|---|---|
| `textelement` | nothing | "for XML elements that DON'T MAP to a database item" |
| `textattribute` | nothing | the attribute form |
| `tableelement` | a **table** (`SourceTable`) | **"the code nested inside is ITERATED FOR ALL RECORDS"** |
| `fieldelement` | a **field** (`SourceField`) | **"must be specified INSIDE THE PARENT TABLE ELEMENT of the field"** |
| `fieldattribute` | a field | the attribute form |

```AL
schema {
  textelement(Customers) {
    tableelement(Customer; Customer) {
      fieldelement(Address; Customer.Address) {
        fieldattribute(County; Customer.County){}
        fieldattribute(City; Customer.City){}
```

**A `tableelement` is a LOOP**, so the schema tree is also the control flow -- nesting a table element
inside another is board:0367's `LinkTable` join, and the iteration order is the tree's.

## Four structural rules, all decidable

> **"There can only be ONE `<root>` node, which MUST BE AN ELEMENT. If `Format` is `Xml`, it must be a
> `textelement` node."**
>
> "There can be several attributes for a single element and **their ORDER DOESN'T MATTER.**"
>
> **"Attribute nodes MUST BE SPECIFIED INSIDE the element nodes they refer to AND BEFORE other element
> nodes."**
>
> **"They CAN'T HAVE NESTED ELEMENT NODES."**

**Four `static_assert`s**: one root, root is an element (and a `textelement` when the format is XML --
board:0442's discriminator reaching into the schema), attributes before elements within a parent, and
attributes are leaves.

**The attributes-before-elements rule is a SOURCE-ORDER constraint**, which is the fifth time this
sweep has met one -- and unlike board:0538's and board:0542's it is a restriction rather than a
meaning, so the generator checks it rather than preserving it.

## What this settles for the seven property items

board:0442 found `Format` discriminates half the property list; **this page shows it also discriminates
the SCHEMA** -- the root's kind depends on it. board:0444's occurrence constraints apply to elements,
board:0445's namespaces to element nodes only ("ignored if set in `textattribute` and `fieldattribute`
nodes"), board:0448's `XmlName` to every node kind.

**So the node kind is the discriminator for four other properties**, and the descriptor is a tagged
union of five shapes rather than one node type with optional fields.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0448: `XmlName` **3 587**. board:0444: `MinOccurs` **887**, `MaxOccurs` **227**, `Occurrence`
**502**. board:0445: `NamespacePrefix` **2 473**. board:0442: `Format` **272**.

**The per-keyword node counts are declaration blocks, not `Name = Value` properties**, so this sweep's
pattern does not reach them -- stated rather than guessed, and they are this item's first task.

## The IST-state

XMLports have no generator (board:0065, board:0034). `src/al/Parser.cpp` parses the AL language;
**whether it parses the five schema keywords is this item's first check** and is not measured here.

## The choice

A `constexpr` schema tree of five node kinds, tagged, with the four structural rules as
`static_assert`s and the iteration emitted from the tree's shape -- a `tableelement` becomes a
`FindSet`/`repeat` over its source (board:0504), nested one level per nesting.

**Tagged union, not one node with optional fields**, because four other properties dispatch on the
kind and a nullable member would push that dispatch to run time.

## Ordering

board:0065's core, before board:0442's format and the six other XMLport property items -- they all
describe nodes of this tree.

## Gate, and its negative control

An XMLport with the documentation's example exports one element per customer with `Address` as an
element and `County` and `City` as its attributes; a schema with two roots fails to transpile; an
attribute declared after an element fails.

**The negative control is the attribute AFTER an element** -- BC rejects it, and an implementation that
accepts it emits a document whose attribute order differs from BC's, which no round-trip through the
same XMLport can detect.
