# 0065 — XMLports will preserve schema, encoding and import transaction semantics

Status: open | Priority: P2 | Reviewed: 2026-09-22

## Current evidence

XMLport generation and src/rt/XmlPort.cpp exist. XmlPortInput materializes parsed input; ReadWhole accumulates every stream block. The old claim that import/export is entirely absent is false, but large imports are not bounded.

The frozen full UT result includes an `Incoming Doc. To Data Exch.UT` import cluster (20 failures in that codeunit) and a Latin data-exchange export failure where a raw `0x85` reaches a UTF-8 PostgreSQL text parameter. Reproduce that export with its declared encoding and inspect the typed value at stream, generator and database boundaries; preserve bytes as bytes until encoding conversion is explicitly required. Trace the first nested conversion error in one PEPPOL import before changing XMLport parsing, because the visible document assertion alone does not identify the faulty layer.

## Implementation for Sol

1. Gate XML and text formats independently: namespaces, attributes, min/max occurrences, unbound loops, separators, fixed widths, encodings and direction-specific triggers.
2. Use a pull parser/streaming writer for large datasets; retain only current nesting state and bounded text fields. Preserve namespace identity rather than stripping prefixes and comparing local names indiscriminately.
3. Implement AutoSave/AutoUpdate/AutoReplace and FieldValidate/default validation through existing Record primitives. Failure must respect the import boundary and explicit Commit policy.
4. Share request-page lifecycle with 0030/0063 while preserving XMLport-specific controls and handler behaviour.

## Acceptance

Round trips include namespace collisions, quoted separators, empty fields, UTF-8/UTF-16 and malformed input after earlier writes. A large generated stream demonstrates bounded memory; a failure proves correct rollback.

## References

Platform: XMLport object/schema documentation, XMLport/element triggers and format/import properties. AL: data-exchange XMLports. Predecessor: inspect encoding/namespace and import-validation findings before changing the parser.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `autoreplace`, `autosave`, `autoupdate`, `defaultfieldsvalidation`, `defaultnamespace`, `direction`, `encoding`, `fielddelimiter`, `fieldseparator`, `fieldvalidate`, `filename`, `format`, `inlineschema`, `linkfields`, `linktable`, `linktableforceinsert`, `maxoccurs`, `minoccurs`, `namespaceprefix`, `namespaces`, `occurrence`, `preservewhitespace`, `recordseparator`, `tableseparator`, `textencoding`, `texttype`, `unbound`, `usedefaultnamespace`, `uselax`, `width-xmlport`, `xmlname`, `xmlversionno`.
