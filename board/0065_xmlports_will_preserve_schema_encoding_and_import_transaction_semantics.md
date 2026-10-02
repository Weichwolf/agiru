# 0065 — XMLports will preserve schema, encoding and import transaction semantics

Status: open | Priority: P1 | Stage: UT data exchange; All streaming | Reviewed: 2026-09-28
Depends on: 0043 validation; 0035 XML; 0074 streams.

## Evidence

- `XmlPortInput` materializes parsed input and ReadWhole accumulates all stream blocks.
- UT data exchange includes raw 0x85 in a UTF-8 SQL parameter and hidden PEPPOL conversion errors; the responsible layer must be traced.

## Implementation

1. Capture original conversion error and encoding at each byte/text boundary before changing parsing. Use libxml2 pull parsing and a bounded writer; share 0030 request controls without importing page save rules.
2. Gate XML and text formats independently: namespaces, attributes, min/max occurrences, unbound loops, separators, fixed widths, encodings and direction-specific triggers.
3. Use a pull parser/streaming writer for large datasets; retain only current nesting state and bounded text fields. Preserve namespace identity rather than stripping prefixes and comparing local names indiscriminately.
4. Implement AutoSave/AutoUpdate/AutoReplace and FieldValidate/default validation through existing Record primitives. Failure must respect the import boundary and explicit Commit policy.
5. Share request-page lifecycle with 0030/0063 while preserving XMLport-specific controls and handler behaviour.

## Acceptance

- Round trips include namespace collisions, quoted separators, empty fields, UTF-8/UTF-16 and malformed input after earlier writes. A large generated stream demonstrates bounded memory; a failure proves correct rollback.

## References

Code: `src/rt/XmlPort.cpp`, `src/gen/PageWriter.cpp`, `test/gate/GenXmlPortGate.cpp`, `test/gate/XmlPortGate.cpp`.

Platform: XMLport object/schema documentation, XMLport/element triggers and format/import properties. AL: data-exchange XMLports. Predecessor: inspect encoding/namespace and import-validation findings before changing the parser.

Property scope: `autoreplace`, `autosave`, `autoupdate`, `defaultfieldsvalidation`, `defaultnamespace`, `direction`, `encoding`, `fielddelimiter`, `fieldseparator`, `fieldvalidate`, `filename`, `format`, `inlineschema`, `linkfields`, `linktable`, `linktableforceinsert`, `maxoccurs`, `minoccurs`, `namespaceprefix`, `namespaces`, `occurrence`, `preservewhitespace`, `recordseparator`, `tableseparator`, `textencoding`, `texttype`, `unbound`, `usedefaultnamespace`, `uselax`, `width-xmlport`, `xmlname`, `xmlversionno`.
