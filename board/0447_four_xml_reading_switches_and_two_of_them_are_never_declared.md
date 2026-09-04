Type:     task
Status:   open
Parent:   0065
Area:     gen, rt
Source:   developer/properties/devenv-inlineschema-property.md, developer/properties/devenv-uselax-property.md, developer/properties/devenv-preservewhitespace-property.md, developer/properties/devenv-xmlversionno-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# Four XML switches, and two of them are never declared

**Four pages, one item**: all four are XMLport-level switches on how the XML document is read or
written, all conditional on board:0442's `Format = Xml`, and each is a single boolean or two-valued
enum with no interaction. Four separate files would each say one paragraph.

> **InlineSchema** (default false): whether an XML schema definition is **included inside** the XML
> document, so it can be validated without an external source.
>
> **UseLax** (default false): whether the XMLport uses **LAX** processing. "Extra elements and
> attributes are often added to XML documents when they're processed in software systems. If set to
> Yes, the XML document will validate **as long as the document meets the MINIMUM schema
> definition**. When extra elements and attributes are included in a namespace, the document will
> successfully validate."
>
> **PreserveWhiteSpace** (default false): **"By default, the product supports the XML standard by
> NORMALIZING white space in attribute names and values. It converts tabs, carriage returns, and
> spaces to single spaces. It also eliminates leading and trailing white space. When this property
> is set to Yes, no normalizations are performed."**
>
> **XmlVersionNo**: `V10` (default) or `V11`. Inserted into the XML declaration.

**`PreserveWhiteSpace` is the one with a default that does work.** Normalising is the behaviour of
every XMLport that says nothing -- tabs and newlines collapsed to single spaces, ends trimmed -- so
this is another property where the population understates the item (board:0372's `Compressed`,
board:0405's `LinksAllowed`). Eleven declarations opt out; every other XMLport normalises.

**`UseLax` is a validation MODE, not a flag**: strict rejects unknown elements, lax accepts them if
they are namespaced. Two declarations, and both matter to whoever wrote them.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`PreserveWhiteSpace =` **11** · `InlineSchema =` **2** · `UseLax =` **2** · `XmlVersionNo =` **0**.

**`XmlVersionNo` is never declared**, so every XMLport in the BaseApp writes `version="1.0"`.

## The IST-state

XMLports have no generator (board:0065, board:0034).

## The choice

Three bits and one enumerator on the XMLport descriptor. **`XmlVersionNo` is refused on its zero**,
joining the sweep's standing arithmetic; the other three are carried and acted on.

**The normalisation is the reader's default and must be written as such** -- not as "if
`PreserveWhiteSpace` then skip normalising", which is the same thing and reads as though normalising
were the exception.

`InlineSchema` needs a schema to emit, which is board:0444's occurrence constraints and the element
tree: the XMLport's own structure IS the schema, so it is generated rather than stored.

## Ordering

Inside board:0065, behind board:0442. `InlineSchema` behind board:0444.

## Gate, and its negative control

An import of an attribute value `"  a\tb  "` yields `a b` by default and the original with
`PreserveWhiteSpace = true`; an XMLport declaring `XmlVersionNo` fails to transpile.

**The negative control is the default import** -- a reader that preserves whitespace always passes the
`PreserveWhiteSpace = true` half and silently changes every value in the other 11 000 imports.
