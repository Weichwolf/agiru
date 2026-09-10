# `XmlReader` walks a document over libxml2's reader

**Finding (2026-09-10).** `XML Buffer Writer` fills the XML Buffer from `DotNet XmlReader` and
`XML DOM Management` reads a document from a stream through it; both stopped at
`XmlReaderSettings.XmlReaderSettings` as absent (11 UT cases). The members named are `Create`
over a path, a stream or a `StringReader`, `Read`, `Close`, `Depth`, `Name`, `Value`, `NodeType`
and the two attribute moves, with `XmlNodeType.Equals` against `Element`, `Text`,
`ProcessingInstruction`, `XmlDeclaration`, `Comment`.

**Reference.** .NET `System.Xml.XmlReader`; libxml2's `xmlTextReader`, whose node-type numbers
are .NET's own and whose walk reports the same sequence with whitespace nodes, which is the
.NET default the BaseApp keeps.

**Choice.** `include/dotnet/XmlReader.h` over `xmlTextReader` (board:0646's engine, PRIVATE to
the value layer), with `XmlNodeType`, `DtdProcessing`, `XmlReaderSettings`, `XmlUrlResolver`,
`NetCredentialCache` and `StringReader` as the value stubs the calls need; DTDs are never
resolved and no URL is fetched. `EOF` is spelled `Eof`, the one door name the C library's macro
forbids, recorded in the header. Gate `XmlReaderGate`.
