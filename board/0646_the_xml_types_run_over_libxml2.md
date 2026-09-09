# The XML types run over libxml2, AL's and .NET's alike

**Finding (2026-09-09).** The AL XML door is declared and refuses: `XmlDocument` 39 of 39
methods, `XmlNode` 32 of 33, `XmlElement` 47 of 49, `XmlAttribute` 24 of 26, `XmlText`, `XmlCData`,
`XmlComment`, `XmlDeclaration` 18-21 each, `XmlNodeList`, `XmlNamespaceManager`,
`XmlAttributeCollection` -- some 240 documented signatures under `methods-auto/xml*/` with no engine
behind them. The .NET XML classes the BaseApp names beside them are absent stubs: `DotNet XmlNode`
1 743 declarations, `XmlDocument` 972, `XmlNodeList` 347, `XmlNamespaceManager` 68, `XmlAttribute`
62, `XmlElement` 44 (counted over BCApps). Over the UT milestone the .NET side alone stops 19 cases
on `XmlDocument.XmlDocument`, 14 on `XMLDOMManagement.LoadXMLNodeFromInStream` and 11 on
`XMLDOMManagement.XMLEscape` (ut_seeded43), across Inc Doc Attachment Overview, XML DOM Management
UT, ERM VAT Tool, WF Buffer Table/Page UT and Incoming Doc. To Data.

**Reference.** `methods-auto/xmldocument/` .. `xmlnamespacemanager/`: the AL types are an immutable
handle over a DOM -- `ReadFrom(Text | InStream [, XmlReadOptions], var Doc)`, `WriteTo(OutStream |
var Text [, XmlWriteOptions])`, `SelectSingleNode(XPath [, XmlNamespaceManager], var Node)`,
`GetChildElements`, `GetDescendantElements`, `AsXmlElement`/`AsXmlText` conversions that refuse
on the wrong kind, `Add`/`AddFirst`/`AddAfterSelf` taking a Variant of node or text. The .NET
classes are `System.Xml`: `XmlDocument.Load/LoadXml/Save`, `SelectSingleNode(xpath, nsmgr)`,
`CreateElement`, `AppendChild`, `InnerText`/`InnerXml`/`OuterXml`, `Attributes`, `ChildNodes`.
`XMLDOMManagement` (codeunit 6224) is the BaseApp's own wrapper over the .NET side and names 30
members; rebuilding those covers most call sites.

**Predecessor.** openerp mapped both onto Python's `xml.dom`/`lxml` and bled on the semantic
difference (CLAUDE.md, "the .NET types are classes, not bridges").

**Choice.** One engine, libxml2 (`libxml2-dev` is on the box and on every Debian architecture;
XPath and namespaces come with it, which a hand-written parser would not), wrapped in `src/net`
as a reference-counted document with node handles, and BOTH surfaces over it: the AL types keep
their door signatures, the .NET classes get `include/dotnet/Xml*.h` with the members the tree
names. CLAUDE.md allows the library and names XML as one of the four things not written from
scratch. Gate cases per documented method under `test/gate/XmlGate.cpp`. Ranks after
board:0645's measurement lands.
