# 0013 — Complete implicit fields and schema/key metadata

Status: in progress | Priority: P0
Depends on: existing source-owned app/native declarations and minimum-runtime metadata.
Activation: 0044's live Field catalogue and 0058's unchanged full UT replay.
Next: wire explicit host selection into generated/native records and migrate remaining
five-field declarations; qualify original source consumers and SQL profiles before
0044's catalogue activation.

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
  system-profile gates plus 32 mutation controls. The canonical profile selects
  kind/LinkedObject/host presence; all three reflection callers share original names.
  A real Record.FieldName → Field lookup → FieldRef.Value caller detects source-name
  substitution. The narrow identity header's typed-dependency control also passes.
- `WithImplicitFields` materializes every selected profile with sorted IDs, actual
  offsets, checked storage types/capacities and nonstored lookup formulas. Native
  Table Metadata now has 23 + 10 fields; timestamp/lookup reflection is qualified.
  Typed CalcFields and FieldRef.CalcField read current creator/modifier names from
  PostgreSQL User rows, including renamed and absent users. The fixture owns a
  connection-private PostgreSQL schema and rolls back; it never drops a shared User.
  Typed GetTable rejects an absent buffer instead of dereferencing null.
- `BodyWriter.cpp` refuses direct/compound AL assignment to timestamp and user
  lookups, including indexed records and no-op self-assignment. Native/ordinary
  page/table/codeunit contexts preserve declared local/global/parameter/return
  shadowing, readable fields, supplied identity/audit and reflected FieldRef writes.
  `GenSourceBindingGate` and the compiled writable-role mutant qualify this boundary;
  borrowed-var writes and source Validate/Clear paths remain separate gaps.
- Original-package audit: eighteen bound candidates compile without PCH; original
  page/source-library and offset/type/number/drop/namespace controls pass. All 215
  selected unbound tables remain red. Other native/generated records still use the
  five-field view; host wiring and live providers remain gaps.
  No AL replay yet covers this materialization. Preserve `4dadfec`/`5a14741`.
- `runtime/RowVersionStorage.h`, `src/rt/RowVersionStorage.cpp` and
  `test/gate/RowVersionGate.cpp` qualify the PostgreSQL allocation/fence foundation.
  The SQL record integration in `test/gate/SqlRowVersionGate.cpp` exercises an
  authored Runtime-17 profile and source SqlTimestamp alias through production
  storage, Record/RecordRef, query, FlowField and navigation primitives. Generated
  app/native host selection is not activated by this fixture. Existing AL database
  methods still refuse; 0044 owns remaining DML/profile coverage, observed-version
  checks and server-restart proof. Reproduce both gates/controls with `make rowversions`.

Absorbs prior 0080/0353/0371/0511. Previous detail and mappings:
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
