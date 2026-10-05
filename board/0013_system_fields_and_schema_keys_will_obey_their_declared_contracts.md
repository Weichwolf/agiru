# 0013 — Complete implicit fields and schema/key metadata

Status: in progress | Priority: P0
Depends on: existing source-owned app/native declarations and minimum-runtime metadata.
Activation: 0044's live Field catalogue and 0058's unchanged full UT replay.
Next: materialize the selected implicit profile in generated and handwritten records;
qualify computed user lookups without renaming SQL columns.

## Implementation

1. Keep one narrow immutable profile in `include/meta/SystemFields.h`; derive
   `src/gen/BodyWriter.cpp` recognition and `TableWriter.cpp` materialization from it.
   App version, source minimum runtime and host capability are separate.
   An omitted source runtime is unspecified, not a host selection.
2. Timestamp 0 / SystemId 2000000000 apply to every kind. Audit families require
   unlinked Normal/Temporary. Runtime-18 adds fields 2000000005–2000000008:
   created/modified User Name Text[50] and Full Name Text[80], lookup User ID
   field 1 → fields 2/3. Runtime-17 lacks these four. Keep standard-layout offsets
   and nonstored lookup calculation; don't reinterpret native Query as source CDS.
3. Keep source member/SQL names distinct from reflected timestamp / $systemId.
   `FieldRef.Name`, `Record.FieldName` and Field catalogue projection share a primitive.
   Typed source assignment restrictions differ from reflected-buffer writes:
   no blanket Editable-based write prohibition or internal Store/CalcField ban.
4. Complete shared default-key projection and explicit SqlIndex/Unique/Enabled/
   MaintainSqlIndex/IncludedFields/AutoIncrement/SqlTimestamp contracts.
   Preserve obsolete Normal data and declaring-app migration ownership.
5. Native effective-profile assertions, emitted ordinary/native records and
   source callers must agree. Don't promote metadata-only presence to SQL audit,
   computed provider or business execution proof. 0044 owns database-wide
   rowversion, durable identity/audit writes and observed-version concurrency.

## Useful implementation details

- `~/Git/openerp/openerp/runtime/base/system_tables.py` and
  `runtime/builtins/_recordref.py`: inspect Field catalogue projection and
  FieldName/FieldRef access; reuse the caller contract, not its catalogue shortcuts.
- Predecessor board 1114: exercise Record.FieldName → Field.SetRange/FindFirst →
  FieldRef.Value with a real caller, not expected-name literals.
  1150: local/global/var bindings precede system-name recognition.
  1233: a copied SystemId snapshot must not discard a valid zero-option primary key.
- Qualified original System 29.0.55365.0 / Runtime 18.0 has Table Metadata
  23 source fields / 33 effective fields. Original getter chains do not normalize
  timestamp / $systemId. Preserve source identities separately.

## Acceptance and references

- Existing `PlatformSystemFieldsGate`, `PlatformFieldGate`, `RecordRefGate`,
  `GenTableGate`, `GenKeyGate`, `GenNativeBindingGate` and negative controls survive.
  Profile/kind/LinkedObject/alias/ordinal mutants fail. Run `make tc`, affected gates,
  `make native-bindings` and native package qualifiers with original inputs.
- `test/transpiler/table-keys.sh`, `test/runtime/reflection-metadata.sh` and
  `test/transpiler/native-bindings.sh` provide reproducible source-owned tests.
  Every implicit field is addressable by number; declared index/count excludes them.
- Developer `ff5939a46e`: `devenv-table-system-fields.md`, record-fieldname,
  fieldref-name/value/validate, recordref-fieldcount/fieldindex and key properties.
  BCApps `bb7111877f`: ApplicationAreaMgmt, SLPopulateHistTables and
  ExpenseActivityLogTest; `src/gen/{TableKeys,TableWriter}.cpp`,
  `src/rt/{FieldMetadata,RecordRef,Storage}.cpp`.
- `make reflection-metadata` passes the source, reflection, RecordRef, Field and
  system-profile gates plus 26 mutation controls. The canonical profile selects
  kind/LinkedObject/host presence; all three reflection callers share original names.
  A real Record.FieldName → Field lookup → FieldRef.Value caller detects source-name
  substitution. The narrow identity header's typed-dependency control also passes.
- Actual record materialization still uses the five-field compatibility view.
  Timestamp/user lookups, host wiring and live providers are not activated by these
  declaration tests. Full AL replay is pending; preserve `4dadfec`/`5a14741`.

Absorbs prior 0080/0353/0371/0511. Previous detail and mappings:
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
