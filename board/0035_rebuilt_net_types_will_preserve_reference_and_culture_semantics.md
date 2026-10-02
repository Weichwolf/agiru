# 0035 — Rebuilt .NET types will preserve reference and culture semantics

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

XML, JSON, regex, culture, stream and date types already have src/net implementations; the board's claims that these whole families are absent are stale. Absent-type generation and variadic refusals still allow compilation without behaviour. The full review analysis also reports potential JSON lifetime defects in `JsonHandle.h`/`Json.cpp`, a moved-value path in `DotNetJson.cpp::JProperty::Replace`, and stream-state handling in `XmlReader.cpp`. These are analyzer findings, not yet reproduced runtime defects; retain the traces from `build/lint/tidy.log`.

The complete frozen UT result has repeated explicit refusals: `WorkbookWriter.Create` (4 methods), `WorkbookReader.Open` (2), `EntityText.ReadPermission` (3), `EntityText.SetRange` (2), and `Base64Convert.ToBase64` (2). Generated `apps/absent/absent/Types.h` declares these but the runtime throws. Start with caller signatures and stream ownership for WorkbookReader/Writer; a dummy success would merely move the failure deeper. Trace whether EntityText is an AL object misclassified as absent before implementing a .NET wrapper. Use the source manifest methods as the A/B set, then rerun their full codeunits.

`XmlDocument.Load(String)` now parses a path or URL and throws on load failure; the focused gate covers a local file and failure preservation. `XmlDeclaration` now exposes version, encoding and standalone from the owning document. This compiles the previously missing LibraryXMLRead source, but `XmlDocument.FirstChild()` currently returns libxml's first element rather than the declaration node for a document with a declaration. A declaration accessor on that node therefore has misleading success. Implement correct declaration presence, document child ordering, and node typing before treating `VerifyXMLDeclaration` as covered; include XML with and without declarations and a DTD between declaration and root. The file/URL path also needs an HTTP fixture before network parity is claimed.

## Implementation for Sol

1. Measure actual refusing calls by type and signature across the current UT run. Use that ranking to choose a family, then read its callers and predecessor findings before implementing it.
2. First reproduce or refute the reported JSON handle and XML stream paths with focused ASan/UBSan cases, including aliases retained after parent mutation/removal. Do not silence them based on a green aggregate. Keep shared engine code behind AL and .NET-specific public contracts. Gate object identity/copying, disposal, out parameters, null, encoding and exception differences rather than aliasing similarly named types.
3. Review std::regex compatibility and CultureInfo/TextInfo formatting/casing against the source usages. Add Unicode and culture fixtures that distinguish invariant, session and explicit-provider behaviour.
4. Rebuild PermissionTestHelper bookkeeping for 0039 and event-capable DotNet variables with explicit subscription lifetimes. Report remaining unsupported signatures by name.

## Acceptance

Family-specific contract gates plus full codeunit A/B; include reference-alias mutation and disposed-object controls. Removing a real implementation must increase the refusal counter and fail the associated gate.

## References

Repository: src/net, include/dotnet, generated absent types, src/gen/Names.cpp. AL: DotNet declarations and call sites. Predecessor: XML/JSON/culture and out-parameter findings; do not copy Python bridge semantics.
