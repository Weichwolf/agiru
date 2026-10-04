# 0035 — Rebuilt .NET types will preserve reference and culture semantics

Status: open | Priority: P0 | Stage: UT XML safety first; All bridge closure | Reviewed: 2026-10-04
Depends on: 0073 typed calls; 0066 culture; 0722 JSON engine safety.

## Evidence

- Unicode foundation: `src/net/Encoding.cpp` validates UTF-8 maximal subparts,
  UTF-16 pairs/tails and UTF-32 scalars/tails; encoders replace isolated surrogate
  units and char arrays preserve UTF-16 units. No byte-conversion BOM or point vector.
  `make encoding`: 48 checks; five compiled controls reject passthrough, unchecked
  pairs, lost tails, wrong scalar range and scalar-valued char arrays. Original
  BC29 native text cores on temporary CLR 10.0.12: 193,398 raw cases, 192,332 within
  the Unicode profile (30,598 encode/161,734 decode), 1,066 outside that profile.
  All selected cases agree; previous actual library fails 81,373 reference cases.
  Reference SHA256 `1ad63a57512ecd536baf9b623b7db811d1c4f6d2bbcbde498a3dfa6adbbf33a9`;
  `/tmp/agiru-base64-text-reference.pGQWvN`, `/tmp/agiru-encoding.t0a7W7`.
  Codepage provider is registered; codepage zero on this Linux CLR is UTF-8, not
  proof of Windows ANSI/OEM locale behaviour. Measured 1252 best-fit maps combining
  acute to B4 and a supplementary character to two question marks; Latin-1 is wrong.
  Encoding/runtime gate analysis has no own findings; existing header findings
  remain unsuppressed. Complete local replay passes 136 cases/232 tooling tests,
  exit 0; `all-local.log`, final source/image hashes in `final-inputs.sha256`.
  Full AL replay remains pending; outside frozen 164436.
  Remaining: codepage tables/best-fit/default selection, array bounds/types and
  all nine Native Base64 bindings/streams (0034); no BC workflow/WASM/performance claim.
  References: developer `ff5939a46e`, `devenv-file-handling-and-text-encoding.md`,
  `methods-auto/textencoding/textencoding-option.md`; BCApps `bb7111877f`, original
  Base64ConvertImpl/Base64ConvertTest; user `0ff62b2266`, data-exchange definitions;
  predecessor 1164. Unicode/RFC 3629 scalar/prefix boundaries supply codec constants.
- Original Windows-1252 authority: all 65,536 UTF-16 units and 256 byte values
  measured through BC29 native text cores; all 1,048,576 supplementary scalars
  produce two question marks. 256 direct/441 non-question-mark best-fit encodes;
  Latin-1 differs at 495 encode/27 decode identities. Complete byte roundtrip.
  `CodePageProbe.cs`, `windows-1252{.tsv,-analysis.json,.log}` under the directory above;
  TSV SHA256 `99e05c67832a638543b851ab05a5538ca3889ecbfe397e86a1e29371de824815`.
  No C++ codepage repair yet. ICU 76.1 `uconv --fallback` substitutes 65 1A for
  e+combining acute (native 65 B4), and 1A for U+1F600 (native 3F 3F); no blind adapter.
  CLR factory probe also proves `GetEncoding(65001)` has a separate three-byte
  preamble; current factory loses it. `encoding-metadata.tsv`.
- `Stream.cpp` no longer copies the entire BLOB on each write. One private append
  preserves raw bytes, zero terminators, existing-stream visibility and borrowed
  self-input; storage grows geometrically. StreamGate: 51 checks green, previous
  actual library four red; 4,096 one-byte writes require at most thirteen capacity
  changes. `/tmp/agiru-stream-append-{gate-final,previous-final}.log`,
  `/tmp/agiru-stream-append-final.sha256`. Runtime/gate have no own lint findings;
  25/47 inherited header findings remain. Three old gate findings removed, no new
  suppressions. Full local replay passes 132 cases/223 tooling tests, exit 0;
  `/tmp/agiru-stream-append-all-local.log`, unchanged final source/image hashes.
  AL replay remains pending; outside frozen 132617.
  Encoding, typed layouts and full BLOB limits remain open, not a streaming/native
  Base64 activation or BC workload performance claim. References: developer
  `ff5939a46e`, `methods-auto/outstream/outstream-{write-text-integer,writetext}-method.md`,
  `methods-auto/blob/blob-data-type.md`; BCApps `bb7111877f`,
  `System Application/App/Base64 Convert/src/Base64ConvertImpl.Codeunit.al` and
  `System Application/Test/Base64 Convert/src/Base64ConvertTest.Codeunit.al`;
  predecessor 1491/1511 require actual stream writes. EncodingGate retains five
  green checks; `/tmp/agiru-stream-append-encoding-gate.log`.
- Development reader/DOM repair: one shared cursor/declaration/EOF/Close state;
  Load consumes its current nodes, never reparses raw input. Positioned last-child
  Load stops at the parent end; closed/EOF Load creates an empty non-null document.
  Namespaces, attributes, DTD, text/CDATA and xml:space are retained; read errors
  refuse and file ownership is exception-safe. Xml-prefixed PIs are not declarations.
  XmlReaderGate passes 71 checks; independently compiled cursor-local, close-local
  and raw-load mutants fail (`/tmp/agiru-xml-reader-consuming-load-{gate,controls}.log`,
  `/tmp/agiru-xml-reader.keVpCb`). `test/runtime/xml-reader.sh` retains the controls.
  Reader/gate targeted lint have no own findings; 36/62 inherited findings remain,
  unsuppressed. The two earlier file-read analyzer findings are repaired.
  Initial full local Make run ended with signal status 143, no completion count;
  `/tmp/agiru-xml-reader-consuming-load-local-tests.log`. No child remained;
  fresh `make test JOBS=2` replay passes 125 cases and all 223 tooling tests, exit 0
  (`/tmp/agiru-xml-reader-consuming-load-local-replay.log`).
  Completed shared activation 044804 keeps 2,161/2,314 and all identities/pass
  statuses; no XML pass gain/loss. `/tmp/agiru-scoped-xml-ut-comparison.json`.
  DTD/resolver enforcement,
  encoded declarations and streaming bounds remain open; no security claim.
- XML/JSON/regex/streams have implementations. JSON ownership/number migration and remaining contract gaps belong to 0722; XML stream analyzer findings still need focused reproduction.
- Shared XPath handles empty namespace aliases without rewriting literals, axes or longer prefixes; wildcards retain their namespace predicate. XmlGate: 79/zero red, old engine 14 red (`/tmp/agiru-xml-empty-alias-{before,after}.log`). Full UT replay `20261003T172807Z-701995`: 2,160/2,314 passed, one gain (`XMLDOMManagementUT::CheckElementTextWithEmptyNamespace`), zero losses/missing/crashes. `/tmp/agiru-json-xpath-ut-comparison.json`. No DTD/resolver/cursor policy changes.
- WorkbookReader/Writer refusals are real .NET bridge work. Base64Convert, EntityText and WebService errors also use '.NET member' wording but refer to absent AL/platform objects: route those to 0034/0038/0044.
- Original Ncl inspection proves two complete ten-getter Int32 families, not string keys: Caption=4, Description=2, Editable=68. Both families and the tested dictionary primitive are now in main; 45/70 checks green. Main-origin comparison retains all eight consumers: AddPageFields body/definition compile without PCH, DictionaryWrapper.Keys remains one error, no compile loss. All twenty value mutants and both iterator-policy controls remain source-identical to the proved prototype. Contract: build/page-source-binding-20261001/artifacts/designer-contract.json; promotion and frozen verification receipts in README. No live designer or SQL workflow proof.
- Own GenericDictionary2 preserves six boxed scalar key types, duplicate Add errors, shared reference identity, null/out boundaries and owned KeyValuePair snapshots. Its immutable AL constructor binder refuses unsupported CLR boxing. Modern Remove/Clear preserve enumeration without dangling map iterators; Add invalidates. Seventy checks green, old shared-API control six red and both wrong iterator policies fail. Integer-key request-page consumers remain compiled, not executed.
- DictionaryWrapper retains one compiler error: Keys. GenericDictionary2_KeyCollection/CopyTo remains absent; Array.CreateInstance currently ignores element type and clamps negative lengths to zero. Close the complete key-view/array/type contract, not a Keys stub. ProfilingDataProcessor compiles again after restoring KeyValuePair construction, but HashSet construction/boxing remains explicitly unsupported. All six original consumer units and sixteen comparison bodies stay measured; no new compile loss.
- XmlDeclaration accessors reject an element handle; FirstChild still does not represent the declaration node.
- `XmlReader::Create` ignores DtdProcessing/XmlResolver; `Over` always enables NOENT. BoundaryProbe confirms Prohibit accepts the DTD and reads its local review-owned entity file. NONET does not prevent this disclosure.
- Installed libxml2 is 2.9.14. `XML_PARSE_NO_XXE` requires 2.13; context-local resource loaders require 2.14. Do not propose those APIs without a portable dependency/version plan or use a process-global loader as a session policy. Current BCApps `XMLDOMManagement.Codeunit.al::CreateXMLReaderFromInStream` explicitly requests Ignore; its DTD must not be reported or expanded.
- `XmlDocument.Load(XmlReader)` now consumes the production cursor; unused Source()
  is removed. Its former raw-input policy bypass is covered by the compiled mutant.
  Read/Create still ignore DTD/resolver settings. Ignore must skip DTD processing
  before entity/attribute expansion; hiding the doctype or clearing NOENT is insufficient.
- Fresh LLVM production probe: 32 UTF-8/UTF-16LE stream cases reproduce four Prohibit/Ignore external-file leaks, eight reported ignored DTDs and two positioned-reader reloads of consumed siblings. Six closed readers reload their original root; the source contract instead leaves an empty document, not a presumed exception. build/xml-policy-20261002/runtime-probe.json; reader/Load must share one cursor/policy state.
- libxml2 2.9.14 context experiment: 24 observations retain internal entity/default-attribute parsing while blocking external general/parameter/subset resources. Owned FIFO control is opened by the unsafe parser, never by guarded/Prohibit parsers; no global loader changed. Critical: xmlStopParser returns a non-null document with wellFormed=true and error 111; check policy/error state, not pointer/form alone. Ignore grammar, expansion bounds, HTTP trap and production reader integration remain unproved. build/xml-policy-20261002/{context-probe,resource-trap,next-policy}.json; this is a measured backend option, not a security fix.
- Full-app blocker `TryGetStringTenantSetting` is a native Boolean(Text, out Text), not an AL object. Pinned IL clears out first; empty name returns false. GetStringTenantSetting reads current session/tenant, uppercases with invariant culture, and exposes only DISPLAYNAME/AADTENANTID/TENANTID. Try catches ArgumentException only; missing session/provider must not become false or blank success. Current TenantSettings contains none of these identities. IsWSKeyAllowed also reads tenant policy, not a constant.
- Chart numeric primitive now preserves all fifteen declared sparse values (Line=3, StackedColumn=11), named members and AL Integer read/write boundaries. DataMeasure gate 49/49; DataTable retains 38 checks with its incorrect Line expectation corrected from source. Old numeric primitive compiles but fails five of seven targeted controls. Receipts: build/compiler-llvm-20261001/artifacts/numeric-controls.json; final image measurements belong in README.
- Original chart page/codeunit bodies now clear the DataMeasureType compilation refusal without PCH. Actual-page driver still fails dependency linking: adding GenericChartCustomization exposes further table, preview and Language bodies. All twelve intended page checks remain missing, not a green subset or SQL/rendering proof. BusinessChartBuilder refusals and CLR enum Format behaviour remain open.

## Implementation

1. Close the reproduced XML DTD/resolver safety hole first (step 6); then WorkbookReader/Writer stream/ZIP/XML signatures and declaration-node navigation. Do not invent AL object wrappers in the .NET layer; preserve exact refusal provenance.
2. Measure actual refusing calls by type and signature across the current UT run. Use that ranking to choose a family, then read its callers and predecessor findings before implementing it.
   Collections/designer: implement live Keys/KeyCollection.CopyTo, CLR array type/bounds/null rules and remaining boxing/default-value signatures from their actual callers. Retain original AddPageFields, request-page and DictionaryWrapper compilation/execution controls and all main provenance tests. NavDesigner mutations remain explicit refusals until their real extension/schema lifecycle is implemented; static constants are not that lifecycle.
   Tenant settings: 0006 owns session selection; 0004 supplies PostgreSQL-backed deployment facts; bridge bodies belong in `src/rt/dotnet/`. Implement the proven signature/casing/out/error contract over that authority, with explicit refusal while its provider is absent. No duplicate mutable map or per-business-object key fix.
   Chart: close the original consumer's declared link dependencies under the LLVM stack; execute its twelve retained checks and the original AL numeric conversions. Keep BusinessChartBuilder refusals and all missing checks counted; establish CLR enum Format behaviour separately. Primitive gates alone are not chart workflow proof.
   Feed the resulting exact typed series/dimensions into one shared chart model: C++ vector scenes for PDF/SVG (0063), interactive selection/drilldown through the common CLI/web command contract (0720). Preserve all declared chart kinds and events; neither a static image nor an empty builder closes the chart requirement.
3. Reproduce or refute XML stream lifetime paths with focused ASan/UBSan cases. Keep shared engines behind AL and .NET-specific public contracts; gate identity/copying, disposal, out parameters, null, encoding and exception differences. Delegate JSON node/number representation to 0722.
4. Review std::regex compatibility and CultureInfo/TextInfo formatting/casing against the source usages. Add Unicode and culture fixtures that distinguish invariant, session and explicit-provider behaviour.
   Encoding: use one generic codepage/Unicode converter with immutable sorted
   source-qualified mappings; preserve best-fit and two-unit supplementary fallback.
   Qualify every supported page, aliases, factory preambles and unknown-page refusals;
   1252 is the first measured family, not a complete Native API. Retain original
   dependency notices if adopting mapping data; do not replace the remaining pages
   with Latin-1 or infer server locale from the Linux CLR probe.
5. Rebuild PermissionTestHelper bookkeeping for 0039 and event-capable DotNet variables with explicit subscription lifetimes. Report remaining unsupported signatures by name.
6. Add settings snapshots and parser-local policy to the shared reader with original error timing; no process-global parser setting. Preserve cursor-consuming DOM Load. Default XmlReader Prohibit rejects DTDs; Ignore skips their processing before entity/attribute expansion. The 2.9.14 SAX callback option above can reject external resources without a version upgrade, but is not an Ignore implementation or forward-only adapter. Reject stopped/denied partial documents even when non-null/wellFormed; collect context-local diagnostics. Limit source/entity output and stream allocations; no unbounded DOM validation pre-pass. Preserve authorized internal Parse and explicit resolver policy; unavailable capabilities refuse. Keep AL/.NET contracts distinct.

## Acceptance

- Family-specific contract gates plus full codeunit A/B; include reference-alias mutation and disposed-object controls. Removing a real implementation must increase the refusal counter and fail the associated gate.
- Prohibit rejects a DTD; Ignore does not expand its entities; Parse honors explicit resolver policy. A fixture-owned local file and a local HTTP trap prove prohibited resources are never fetched. No private host file is needed as a test fixture.
- Repeat these controls through XmlDocument.Load(reader), including an unread reader and DTD-backed attributes. No consumer may reparse raw input outside the reader's policy/cursor contract.
- Positioned-last loads that subtree, not consumed siblings; closed/EOF readers do not reload the old root. Copy aliases share cursor/close state. Cover settings mutation after Create, encodings/BOMs, partial-document parser stops and independent concurrent policies; retain all existing gates and raw UT identities.

## References

DataMeasureType: platform `methods-auto/dotnet/dotnet-data-type.md`, `devenv-{al-type-conversion-expressions,get-started-call-dotnet-from-al}.md`; BCApps main `a9ea4d84534cebba852c44bf0f841c2ea149de4e`, `src/System Application/App/{Business Chart/src/{BusinessChartType.Enum,BusinessChartImpl.Codeunit},DotNet Aliases/src/dotnet}.al`, `src/Layers/W1/{BaseApp/System/GenericChart/GenericChartMgt.Codeunit,Tests/Misc/ChartConfigurationTool.Codeunit}.al`. Alias assembly is Microsoft.Dynamics.Nav.Client.BusinessChart.Model. User-document search adds no legacy-chart/ordinal guarantee; earlier 1016 requires both DataTable and BusinessChartData and rejects silent empty values, but specifies no CLR enum ordinals. Receipts above; source declarations/usage, not a BC execution claim.

Designer/dictionary: same BCApps main, DotNet Aliases/src/dotnet.al (Ncl DesignerFieldProperty/DesignerFieldType; mscorlib GenericDictionary2); W1 BaseApp Modules/System/{PageDesigner/AddPageFields.Page,RequestPage/RequestPageParametersHelper.Codeunit}.al; DictionaryWrapper.Codeunit.al. Platform: devenv-{get-started-call-dotnet-from-al,inclient-designer}.md; user intent: business-central/admin-sandbox-environments.md#designer. Predecessor search adds no Designer getter/dictionary-key authority. Verified Ncl 28.4.53241.0 hash d37240e842d6e259407f27fc100cccc6fd4d885e9dd059503253b516491b4213, 11,269,984 bytes; identity and original analysis-only packages remain in /home/cosmo/Git/agiru-worktrees/goal-20260928/build/field-native-proof/. CLR collection guarantees: https://learn.microsoft.com/en-us/dotnet/api/system.collections.generic.dictionary-2?view=net-8.0. No proprietary assembly/runtime added to the product.

Collection continuation: same main, System Application/App/Performance Profiler/src/ProfilingDataProcessor.Codeunit.al::ComputeFullTimeAggregate; W1 BaseApp/DictionaryWrapper.Codeunit.al::InitializeKeysArray. CLR constructor: https://learn.microsoft.com/en-us/dotnet/api/system.collections.generic.keyvaluepair-2.-ctor; modern enumeration: https://learn.microsoft.com/en-us/dotnet/api/system.collections.generic.dictionary-2.getenumerator?view=net-9.0. Earlier 1025 requires actual KeyValuePair iteration; it supplies no key/boxing or iterator-policy authority. Main code: include/dotnet/{Generic,DesignerFieldProperty,DesignerFieldType}.h, src/net/Generic.cpp, test/gate/{Dictionary,DesignerConstants}Gate.cpp. Promotion: build/dictionary-integration-20261002/artifacts/proof.json; prototype mutation controls: build/dictionary-20261001/artifacts/proof.json.

Tenant: platform `devenv-get-started-call-dotnet-from-al.md`; current BCApps main `System Application/App/{Azure AD Tenant/src/AzureADTenantImpl,Environment Information/src/TenantInformationImpl}.Codeunit.al`; user `business-central/admin-troubleshoot-connectivity.md`; predecessor board search has no method guarantee. Official 28.4.53241.0 `ServiceTier/PFiles64/Microsoft Dynamics NAV/280/Service/Microsoft.Dynamics.Nav.NavUserAccount.dll`, 33,080 bytes, SHA256 `464ed2d63f40bacc7dc405c28f95e1aa8ca2df96df4213aef7b161bc538bffbd`: Try RVA 217c, Get 249c, IsWSKeyAllowed 22ac. Own `build/field-native-proof/{fetch_runtime.py,inspect_runtime.py,tenant-settings-contract.txt}`; catch token 01000015 is System.ArgumentException. Static inspection, no BC workload claim.

Code: `src/net/{XmlReader,DotNetXml,Regex}.cpp`, `include/dotnet/XmlReader.h`, `src/gen/Names.cpp`; gates: `XmlReaderGate`, `XmlGate`. Platform: `methods-auto/xmldocument/xmldocument-readfrom-string-xmldocument-method.md` does not specify .NET DTD/resolver settings. AL: `Layers/W1/BaseApp/Modules/System/Xml/XMLDOMManagement.Codeunit.al`, `Layers/W1/Tests/Misc/XMLDOMManagementUT.Codeunit.al`, DotNet declarations, current BCApps revision in README. Contracts: [DtdProcessing](https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmlreadersettings.dtdprocessing), [XmlResolver](https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmlreadersettings.xmlresolver), [libxml2 parser options and per-context loader versions](https://gnome.pages.gitlab.gnome.org/libxml2/html/parser_8h.html). User/predecessor searches add no resolver-policy guarantee; do not copy Python bridge semantics.

Reader alias state: local `methods-auto/dotnet/dotnet-data-type.md` and
`devenv-get-started-call-dotnet-from-al.md` describe the bridge, not cursor/Close
guarantees; missing detail checked against [XmlReader.Close](https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmlreader.close?view=netframework-4.8.1)
and [ReadState](https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmlreader.readstate?view=net-5.0).
Original `XMLDOMManagement::LoadXmlDocFromText` loads then closes a reader;
user `business-central/across-income-documents.md` supplies workflow intent, not
reader-state authority. Earlier 1185 concerns Load error propagation, not alias state.

DOM cursor: original BCApps `XMLDOMManagement::LoadXmlDocFromText` and
`XMLDOMManagementUT::CheckDoctypeElementWithEmptyInternalSubset`; .NET-specific
positioned/end semantics are absent from the local AL method docs and predecessor.
Primary [Framework XmlLoader](https://raw.githubusercontent.com/microsoft/referencesource/main/System.Xml/System/Xml/Dom/XmlLoader.cs)
confirms current-position sequencing, empty ended readers and parent-end boundaries.
Own adapter copies current nodes through Read, not a transplanted implementation.
Whitespace inheritance uses the original
[libxml2 2.9.14 tree contract](https://raw.githubusercontent.com/GNOME/libxml2/v2.9.14/tree.c).

Empty-namespace XPath: developer docs `ff5939a46e`, `methods-auto/{xmlnamespacemanager/xmlnamespacemanager-addnamespace,xmlnode/xmlnode-selectsinglenode-string-xmlnamespacemanager-xmlnode}-method.md`; BCApps `bb7111877f`, original XMLDOMManagementUT and XMLDOMManagement overloads. Earlier 1214 reproduces the same prefix failure; [XPath 1.0](https://www.w3.org/TR/1999/REC-xpath-19991116/) defines QName, wildcard and quoted-token boundaries. User import documentation supplies no XPath token grammar.
