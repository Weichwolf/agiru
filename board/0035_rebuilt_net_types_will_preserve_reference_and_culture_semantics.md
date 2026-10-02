# 0035 — Rebuilt .NET types will preserve reference and culture semantics

Status: open | Priority: P0 | Stage: UT XML safety first; All bridge closure | Reviewed: 2026-09-28
Depends on: 0073 typed calls; 0066 culture; 0722 JSON engine safety.

## Evidence

- XML/JSON/regex/streams have implementations. Confirmed JSON use-after-free and Decimal loss are isolated in 0722; XML stream analyzer findings still need focused reproduction.
- WorkbookReader/Writer refusals are real .NET bridge work. Base64Convert, EntityText and WebService errors also use '.NET member' wording but refer to absent AL/platform objects: route those to 0034/0038/0044.
- XmlDeclaration accessors reject an element handle; FirstChild still does not represent the declaration node.
- `XmlReader::Create` ignores DtdProcessing/XmlResolver; `Over` always enables NOENT. BoundaryProbe confirms Prohibit accepts the DTD and reads its local review-owned entity file. NONET does not prevent this disclosure.

## Implementation

1. Close the reproduced XML DTD/resolver safety hole first (step 6); then WorkbookReader/Writer stream/ZIP/XML signatures and declaration-node navigation. Do not invent AL object wrappers in the .NET layer; preserve exact refusal provenance.
2. Measure actual refusing calls by type and signature across the current UT run. Use that ranking to choose a family, then read its callers and predecessor findings before implementing it.
3. Reproduce or refute XML stream lifetime paths with focused ASan/UBSan cases. Keep shared engines behind AL and .NET-specific public contracts; gate identity/copying, disposal, out parameters, null, encoding and exception differences. Delegate JSON node/number representation to 0722.
4. Review std::regex compatibility and CultureInfo/TextInfo formatting/casing against the source usages. Add Unicode and culture fixtures that distinguish invariant, session and explicit-provider behaviour.
5. Rebuild PermissionTestHelper bookkeeping for 0039 and event-capable DotNet variables with explicit subscription lifetimes. Report remaining unsupported signatures by name.
6. Enforce Prohibit/Ignore/Parse and resolver policy per reader; no process-global parser setting. Limit entity expansion and source bytes. Preserve explicitly authorized internal DTD parsing without enabling arbitrary external file/network reads.

## Acceptance

- Family-specific contract gates plus full codeunit A/B; include reference-alias mutation and disposed-object controls. Removing a real implementation must increase the refusal counter and fail the associated gate.
- Prohibit rejects a DTD; Ignore does not expand its entities; Parse honors explicit resolver policy. A fixture-owned local file and a local HTTP trap prove prohibited resources are never fetched. No private host file is needed as a test fixture.

## References

Code: `src/net/{XmlReader,DotNetXml,Regex}.cpp`, `include/dotnet/XmlReader.h`, `src/gen/Names.cpp`; gates: `XmlReaderGate`, `XmlGate`. AL: `Layers/W1/BaseApp/Modules/System/Xml/XMLDOMManagement.Codeunit.al`, `Layers/W1/Tests/Misc/XMLDOMManagementUT.Codeunit.al`, DotNet declarations. Contracts: [DtdProcessing](https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmlreadersettings.dtdprocessing), [libxml2 parser options](https://gnome.pages.gitlab.gnome.org/libxml2/html/parser_8h.html). Predecessor: XML/JSON/culture and out-parameter findings; do not copy Python bridge semantics.
