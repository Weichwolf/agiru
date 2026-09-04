Type:     task
Status:   open
Parent:   0065
Area:     gen, rt
Source:   developer/properties/devenv-xmlname-property.md, developer/properties/devenv-texttype-property.md, developer/properties/devenv-encoding-property.md, developer/properties/devenv-textencoding-property.md, developer/properties/devenv-width-xmlport-property.md, developer/properties/devenv-filename-property.md
Verdict:  fehlt
Class:    activation

# An XMLport node names itself, and a fixed-text field declares its width

**Six pages, one item**: the remaining per-node and per-port serialisation declarations. They are
grouped because each is a single value with no interaction, they all belong to the same writer, and
each alone is a paragraph.

> **XmlName** (every node kind): the name of the node. **"If `XmlName` is not defined, it will use the
> string specified in the `NodeName` property as the default."**
>
> **TextType** (text attribute, text element): `Text` (**default**) or `BigText`.
>
> **Encoding** (XmlPort): `UTF8` **with BOM** (default), `UTF16`, `ISO88592`. **"Information about
> the encoding system used is inserted into the header of the XML document."**
>
> **TextEncoding** (XmlPort): `MSDOS` (**the default**), `UTF8` with BOM, `UTF16`, `WINDOWS`. **"Only
> available when the `Format` property is `Fixed Text` or `Variable Text`."** And: **"Internally,
> Business Central uses Unicode encoding. The `Text` data type in AL uses UTF-16 encoding, the same
> encoding as .NET strings."**
>
> **Width** (XMLport field): the field's width. **"Used only when the `Format` property is set to
> `FixedText`."**
>
> **FileName** (XmlPort): the file the port reads or writes.

**Two encodings, two defaults, and one of them is not Unicode.** `Encoding` is for XML and defaults to
UTF-8; `TextEncoding` is for text files and defaults to **MSDOS** -- code page 437 or 850 depending on
the system, which is a lossy single-byte encoding. So a text XMLport that says nothing writes a
non-Unicode file, and board:0074 ("a stream carries its encoding") is where that has to be honest
rather than defaulting to UTF-8 because it is nicer.

**The UTF-8 BOM is stated twice and is not optional**: BC writes it, so a reader comparing bytes
against a BC-produced file sees it.

`Width` on a fixed-text field is what makes the format fixed at all -- without it there are no column
boundaries -- so a `FixedText` XMLport with an undeclared field width is a `static_assert`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`XmlName =` **3 587** · `TextType =` **47** · `Encoding =` **80** · `TextEncoding =` **39** ·
`Width =` **248** (XMLport fields and page fields together, board:0423) · `FileName =` **6**.

## The IST-state

XMLports have no generator (board:0065, board:0034). board:0074 records the stream-encoding state.

## The choice

`string_view`s and small enumerators on the node and port descriptors, `constexpr`. `XmlName`'s
fallback to `NodeName` is folded by the generator, so the descriptor always carries a name.

**The encodings reach board:0074's stream layer and are not re-implemented here.** `MSDOS` needs a
code page table; that is one dependency, named, and it is the reason this item cannot be closed by
carrying four enumerators.

## Ordering

Inside board:0065, behind board:0442's format and board:0074's encoding.

## Gate, and its negative control

A `FixedText` export writes fields at their declared widths in `MSDOS` encoding by default; an XML
export writes UTF-8 with a BOM and the encoding in the declaration.

**The negative control is the default text encoding** -- writing UTF-8 where BC writes MSDOS produces
a file that looks correct in a modern editor and is wrong for every non-ASCII character, and only a
byte comparison against a BC-produced file shows it.
