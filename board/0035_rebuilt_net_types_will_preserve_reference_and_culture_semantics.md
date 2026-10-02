# 0035 — Rebuilt .NET types will preserve reference and culture semantics

Status: open | Priority: P0 | Stage: UT XML safety first; All bridge closure | Reviewed: 2026-09-30
Depends on: 0073 typed calls; 0066 culture; 0722 JSON engine safety.

## Evidence

- XML/JSON/regex/streams have implementations. Confirmed JSON use-after-free and Decimal loss are isolated in 0722; XML stream analyzer findings still need focused reproduction.
- WorkbookReader/Writer refusals are real .NET bridge work. Base64Convert, EntityText and WebService errors also use '.NET member' wording but refer to absent AL/platform objects: route those to 0034/0038/0044.
- XmlDeclaration accessors reject an element handle; FirstChild still does not represent the declaration node.
- `XmlReader::Create` ignores DtdProcessing/XmlResolver; `Over` always enables NOENT. BoundaryProbe confirms Prohibit accepts the DTD and reads its local review-owned entity file. NONET does not prevent this disclosure.
- Installed libxml2 is 2.9.14. `XML_PARSE_NO_XXE` requires 2.13; context-local resource loaders require 2.14. Do not propose those APIs without a portable dependency/version plan or use a process-global loader as a session policy. Current BCApps `XMLDOMManagement.Codeunit.al::CreateXMLReaderFromInStream` explicitly requests Ignore; its DTD must not be reported or expanded.

## Implementation

1. Close the reproduced XML DTD/resolver safety hole first (step 6); then WorkbookReader/Writer stream/ZIP/XML signatures and declaration-node navigation. Do not invent AL object wrappers in the .NET layer; preserve exact refusal provenance.
2. Measure actual refusing calls by type and signature across the current UT run. Use that ranking to choose a family, then read its callers and predecessor findings before implementing it.
3. Reproduce or refute XML stream lifetime paths with focused ASan/UBSan cases. Keep shared engines behind AL and .NET-specific public contracts; gate identity/copying, disposal, out parameters, null, encoding and exception differences. Delegate JSON node/number representation to 0722.
4. Review std::regex compatibility and CultureInfo/TextInfo formatting/casing against the source usages. Add Unicode and culture fixtures that distinguish invariant, session and explicit-provider behaviour.
5. Rebuild PermissionTestHelper bookkeeping for 0039 and event-capable DotNet variables with explicit subscription lifetimes. Report remaining unsupported signatures by name.
6. Enforce Prohibit/Ignore/Parse and resolver policy per reader; no process-global parser setting. Default Prohibit rejects DTDs; Ignore neither reports nor processes them. Choose a supported per-context backend/version before allowing Parse; an unavailable safe policy must refuse explicitly. Limit entity expansion and source bytes. Preserve explicitly authorized internal DTD parsing without enabling arbitrary external file/network reads. Keep AL XmlDocument and .NET XmlReader contracts distinct.

## Acceptance

- Family-specific contract gates plus full codeunit A/B; include reference-alias mutation and disposed-object controls. Removing a real implementation must increase the refusal counter and fail the associated gate.
- Prohibit rejects a DTD; Ignore does not expand its entities; Parse honors explicit resolver policy. A fixture-owned local file and a local HTTP trap prove prohibited resources are never fetched. No private host file is needed as a test fixture.

## References

Code: `src/net/{XmlReader,DotNetXml,Regex}.cpp`, `include/dotnet/XmlReader.h`, `src/gen/Names.cpp`; gates: `XmlReaderGate`, `XmlGate`. Platform: `methods-auto/xmldocument/xmldocument-readfrom-string-xmldocument-method.md` does not specify .NET DTD/resolver settings. AL: `Layers/W1/BaseApp/Modules/System/Xml/XMLDOMManagement.Codeunit.al`, `Layers/W1/Tests/Misc/XMLDOMManagementUT.Codeunit.al`, DotNet declarations, current BCApps revision in README. Contracts: [DtdProcessing](https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmlreadersettings.dtdprocessing), [XmlResolver](https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmlreadersettings.xmlresolver), [libxml2 parser options and per-context loader versions](https://gnome.pages.gitlab.gnome.org/libxml2/html/parser_8h.html). User/predecessor searches add no resolver-policy guarantee; do not copy Python bridge semantics.
