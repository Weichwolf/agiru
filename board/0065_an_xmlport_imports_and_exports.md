Type: arc
State: open
Area: al, gen, rt, net
Tags: target

# An XmlPort imports and exports over a stream, and its schema is its declaration

`XmlPort` has no parser, no writer and no door header. `methods-auto/xmlport/` holds 21 signature
pages -- `Import`, `Export`, `Run`, `SetSource`, `SetDestination`, `SetTableView`, `Filename`,
`ImportFile`, `TextEncoding`, `FieldSeparator`, `FieldDelimiter`, `RecordSeparator`,
`TableSeparator`, `CurrentPath`, `Skip`, `Break`, `BreakUnbound`, `Quit` -- and the door carries
none of them. **18 of the 21 are `xmlportinstance-*` pages the completeness counter never reads**
(board:0059), so the surface baseline records this type as three methods.

**51 xmlport files in the read roots** -- 49 `.XmlPort.al` (base 36, tests 8, system 3,
test_runner 2) plus two BCApps spells `.xmlport.al` and `.Xmlport.al`, which the suffix match reads
like any other -- and 10
`[EventSubscriber(ObjectType::XmlPort, ...)]` subscribe to events an xmlport publishes
(board:0057).

## Where the milestone touches it, measured 2026-09-04

No `[Test]` procedure of the 2 305 declares an `XmlPort` VARIABLE, but `Payment Export XMLPort UT`
(24 tests), `Payment Export FX Tables UT` (12) and `Payment Export Validation UT` (41) pass
`XMLPORT::"Export Generic CSV"` and `XMLPORT::"Data Exch. Import - CSV"` as an OBJECT ID into the
Data Exchange framework. **An object id that names nothing is not a missing method, it is a
translation that cannot resolve the name** -- so those three codeunits reach the same wall from a
direction a method count does not show. The number to take from this is not "0 variables" but "the
object has to EXIST before its id can be written down".

## What the platform documents

`devenv-xmlport-object.md`, `devenv-xmlport-schema.md`, `devenv-xmlport-overview.md`,
`devenv-using-namespaces-with-xmlports.md`.

- The schema is `schema { textelement | tableelement | fieldelement | fieldattribute |
  textattribute }` -- **the same `<kind>(<name>[; <source>]) { ... }` grammar** as the page layout,
  the report dataset and the query elements (board:0034). `triggers-auto/` carries 3 xmlport
  triggers plus 15 element triggers across the five element kinds -- 18 of the 152.
- `tableelement` iterates: "the code nested inside the table element is iterated for all records in
  the underlying table". `fieldelement` maps to a field via `SourceField`. `textelement` maps to
  nothing.
- **There is exactly one `<root>` node, and with `Format = Xml` it must be a `textelement`.**
- **Attribute nodes are ORDERED against element nodes**: "attribute nodes must be specified inside
  the element nodes they refer to and **before other element nodes**. They can't have nested element
  nodes." That is a parse rule, so the reader refuses a misplaced attribute rather than accepting a
  schema that would export in the wrong shape.
- **An XmlPort has a REQUEST PAGE**, like a report -- `UseRequestPage` and the twelve request-page
  triggers (board:0063 owns the same surface, so the two items share it rather than each building
  one).
- **`Format` is not only XML.** `Format = VariableText` with `FieldSeparator`,
  `FieldDelimiter`, `RecordSeparator` and `TableSeparator` is a FLAT FILE reader and writer -- which
  is what the Data Exchange framework uses it for, and what the three UT codeunits above exercise.
- `Direction` decides whether the object may import, export or both; `SetSource`/`SetDestination`
  take an `InStream`/`OutStream`, `ImportFile` and `Filename` a path.

## What the predecessor paid for

| item | finding |
|---|---|
| **WI-1048** | a CSV xmlport read correctly and **wrote no fields at all** into the Data Exch. field table -- an import that reports success and stores nothing |
| **WI-1058** | the xmlport's `[IntegrationEvent]` publishers were dead on both sides |
| **WI-1059** | the `var`-parameter index did not know xmlport procedures, so out parameters were not written back |
| **WI-987** | the event wiring around xmlports needed its own round |

Three of the four are the same shape as CLAUDE.md's first measured failure mode -- **an out parameter
never written** -- and in C++ that one is closed by the language, provided the generator emits `var`
as a reference.

## The choice

- **The schema is `constexpr` metadata and the element triggers are code**, which is the same split
  every other generated object already makes: what AL puts in a `schema` block goes in the `.h`,
  what it puts in a `trigger` goes in the `.cpp`.
- **Import and export are two walks over one declaration**, not two objects. `Direction` refuses the
  wrong one at translation time where it is declared.
- **XML goes through `agiru::XmlDocument`**, which is a door type with a real implementation ahead of
  it (board:0035 names the XML family as the one that goes first), and `Format = VariableText` goes
  through the stream types directly. Neither needs a new dependency.
- **The stream is the boundary and it is defensive there** (CLAUDE.md): an encoding fallback, a
  refusal that names the element and the line, never a silent skip. WI-1048's import "succeeded"
  while writing nothing, and that is the failure this item exists to make impossible.

## Gate

An xmlport with a `tableelement` and two `fieldelements` exports the rows a `Record` with the same
filters would give, and importing what it wrote reproduces them field for field. A
`Format = VariableText` port round-trips through the declared separators, including a value that
CONTAINS the field separator. A port declared `Direction = Export` refuses to import, at translation
time. An element trigger's `var` parameter reaches the caller.

**Negative control**: drop one `fieldelement` from the write side and require the round trip to go
red -- WI-1048 is exactly the case where the read side passed and the write side stored nothing.
