Type:     task
Status:   open
Parent:   0065
Area:     gen, rt
Source:   developer/properties/devenv-format-property.md
Verdict:  fehlt
Class:    activation

# An XMLport declares its format, and that decides which of its properties apply

> Sets the formats of the source expression for various data types. Applies to: **Xml Port.**
>
> `Xml` -- XML documents. **This is the default.** `VariableText` -- variable text files.
> `FixedText` -- fixed-width text fields.
>
> This property supports **CSV (comma separated values) export files and XML files.**

**One property that switches an object between two entirely different serialisations**, and half the
XMLport property list is conditional on it. The other pages say so themselves:

- `TextEncoding` -- "**only available when `Format` is `Fixed Text` or `Variable Text`**"
- `FieldDelimiter` -- "**only used if `Format` is `Variable Text`. Otherwise the setting is
  ignored**"
- `RecordSeparator` -- "**only used if `Format` is `Variable Text` or `Fixed Text`**"
- `Encoding`, `Namespaces`, `NamespacePrefix`, `XmlName` -- meaningful only for `Xml`

So the format is a discriminator over the whole property set, and **every one of those conditions is
decidable at translation time** -- a `FieldDelimiter` on an `Xml` XMLport is a declaration that does
nothing, and the generator can say so instead of dropping it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Format =`: **272 declarations**, all necessarily non-`Xml` since `Xml` is the default.

Against `XmlName`'s 3 587 -- so the great majority of XMLports in the BaseApp are XML, and 272 are
text files.

## The IST-state

XMLports have no generator (board:0065, board:0034).

## The choice

A three-valued enumerator on the XMLport, read FIRST by the writer, which then emits either the XML
serialiser or the text one. **Two writers behind one discriminator**, not one writer with branches:
the two produce different documents from different property sets, and the branchy version carries
every text property into the XML path.

The conditional properties become `static_assert`s -- eight of them, one per property whose page
states its condition.

## Ordering

**First inside board:0065.** Nothing else about an XMLport is meaningful until the format is known.

## Gate, and its negative control

A `VariableText` XMLport writes a delimited text file; an `Xml` one writes an XML document; a
`FieldDelimiter` on an `Xml` XMLport fails to transpile.

**The negative control is the ignored property** -- BC ignores it silently, and this tree's rule is
that a declaration accepted and ignored is worse than one refused, so the assertion is a deliberate
deviation from BC's leniency and is recorded as one.
