# 0013 — Complete implicit fields and schema/key metadata

Status: queued | Priority: P0
Depends on: existing source-owned app/native declarations and minimum-runtime metadata.
Activation: 0044's live Field catalogue and 0058's unchanged full UT replay.
Next: replay qualified Integer projection and live Field navigation/counts on the
unchanged UT population; complete the four remaining Field attributes under 0044.
Retain the full 2314-case UT population through replay.

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
- Developer `f928288ee840`: `devenv-table-system-fields.md`, record-fieldname,
  fieldref-name/value/validate, recordref-fieldcount/fieldindex and key properties.
  BCApps `bb7111877f`: ApplicationAreaMgmt, SLPopulateHistTables and
  ExpenseActivityLogTest; `src/gen/{TableKeys,TableWriter}.cpp`,
  `src/rt/{FieldMetadata,RecordRef,Storage}.cpp`.
- `make reflection-metadata` passes the source, reflection, RecordRef, Field and
  system-profile and both catalogue gates plus 53 mutation controls. The canonical profile selects
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
- `make system-profiles`: 954 generator checks, 28 compiled host/kind/LinkedObject
  consumers and two production-wrapper consumers pass, with eleven generated-AL/SQL
  checks for each direct host and each wrapper path. Host 17/18 is
  explicit (`agirutc --host-runtime`, Make `AGIRU_HOST_RUNTIME`), independent of
  fixture app version 99 and minimum runtime 12. Binding/output mismatches,
  wrong audit/lookup presence/count and unsupported/duplicate host options refuse.
  Independent SQL checks the physical version and absence of alias/lookup columns.
  Fixtures: `test/gate/GenSystemProfileGate.cpp`,
  `test/transpiler/system-profile.sh` and `test/transpiler/system-profile/`.
  External/LinkedObject declarations compile but translation remains nonzero and
  their business paths are unexecuted. `make transpile` now selects Runtime 18 by
  default, with explicit `AGIRU_HOST_RUNTIME=17.0` override; an explicitly empty
  host refuses before output. Nineteen SymbolsPackageGate tooling tests pass.
  Direct generator fixture APIs still allow the unselected migration view.
- Production regeneration from BCApps `d99152ee35f0` and the verified System
  package emits ordinary/native Runtime-18 declarations. Translation remains
  exit 1: 5683 refused properties and 215 selected unbound native tables; no full
  compilation, provider or G1 claim follows from generated files.
- `make native-consumers` now uses the production wrapper with the verified System
  package and Runtime 18, plus independently generated source-bound declarations.
  All eight original page units compile in each of three variants (24/24); missing/
  duplicate consumer and wrong original field-number controls reject. Input/source
  hashes remain unchanged. Both full translations still exit 1 on unsupported
  declarations, so the qualifier remains red; syntax success is not live-provider,
  business execution or G1 proof. Reproduce with `test/transpiler/native-consumers.sh`
  and `test/transpiler/native-binding/consumers.json`, after `make native-bindings`.
- Integer's Runtime-18 projection now recognizes the canonical timestamp and retains
  nonstored user lookups, rather than rejecting every computed read. Timestamp uses
  the physical SQL alias; unknown/mistyped stored fields still refuse. The value 1
  comes from original BC29 `IntegerDataProvider` iterator RVA `2e953c` →
  `VirtualDataProvider.CreateVirtualRecordValues` RVA `adc40` →
  `AddSystemFieldValues` RVA `adcb0`, with constant initialization RVA `addf3`.
  The same original path supplies blank SystemId/audit values; it does not allocate
  PostgreSQL rowversions. Native assembly SHA-256:
  `277e35cbdfb87f17b979813e46fb73c2e84f5b04b85ed806c40367acf72b48b7`,
  original member `ServiceTier/PFiles64/Microsoft Dynamics NAV/290/Service/Microsoft.Dynamics.Nav.Ncl.dll`
  in `https://bcartifacts-exdbf9fwegejdqak.b02.azurefd.net/onprem/29.0.54011.55407/platform`.
  This is original static call-chain authority, not fresh sandbox/BC28 equivalence.
  CursorGate passes 255 checks and FilterGate 129; SQL/typed/RecordRef agree on
  bounded reads/counts/navigation and all six stored implicit values. Existing
  one-million-row truncation remains a functional/scale gap: replace it with complete
  interval/cardinality and bounded navigation contracts, not a larger cap.
  Completed `00c187c` replay: seventy-two Integer-related failures, including
  38 previously passing methods; one additional loss is Sales Invoice Aggregate's
  normal-field count (expected 77, actual 78). Preserve every identity. Qualify the
  provider and reflected field population together, rather than masking field 0.
  Original BC29 FieldDataProvider iterator RVA `2e888c` clamps catalogue field keys
  to 1..2147483647 and starts after the internal timestamp. This differs from
  RecordRef.Field(0), which remains required. Native Field.Get and Find/Next/Count
  now use the immutable installed declarations, excluding nonpositive catalogue keys;
  temporary zero keys remain valid. Provisioning no longer inserts SQL Field snapshots;
  legacy copies are ignored, not deleted. PlatformField/FieldCatalogue retain 418/61
  checks; the shared qualifier detects population, filter, bookmark, frozen-identity/
  version and empty-write mutants. The full aggregate UT replay remains required.
  Native Table Metadata now shares the same filter/order/bookmark kernel, borrowing
  InstalledTables directly; 48 local checks retain raw identities, exact projected
  values and temporary isolation. GetBySystemId, authorization, full AL execution
  and secondary-order performance remain gaps (0044); no SQL catalogue is installed.
  Original FieldDataProvider.GetFieldRecordBuffer RVA `a4330` constructs
  MetadataSystemId from `{2000000041, TableNo, No, 0}` and invokes
  VirtualDataProvider.GetSystemPopulatedVirtualRecordValues RVA `adc1f` →
  AddSystemFieldValues RVA `adcb0`. Frozen timestamp 1 and blank audits use
  the original virtual-field initialization above, not PostgreSQL
  allocation. Keep timestamp in declarations/buffers/reflection; do not hide the failure.
- Every bound native record now materializes the original Runtime-18 Normal,
  unlinked profile: ten implicit fields, typed offsets/capacities and nonstored
  User lookups. `PlatformSystemFieldsGate` passes 3117 checks across all eighteen;
  `PlatformSourceGate` passes 3060 source/reflection checks. The existing
  `make reflection-metadata` qualifier retains all prior controls; 53 mutants reject.
- Native fixture assertions now select Runtime 18 explicitly, rather than counting
  the five legacy declarations. PageTableField/ObjectCatalogue/FeatureKey/
  UserPersonalization gates pass 312/345/93/320 checks; all four pass focused tidy.
  User Personalization independently expects six stored implicit columns, excluding
  four user lookups, as documented in `devenv-table-system-fields.md` at `f928288ee840`.
  All original source fields, reflection, refusals and SQL checks remain; no provider
  downgrade or test-population reduction. Full integration/replay remains required.
- `make native-bindings` uses the verified System 29.0.55365.0 / Runtime 18.0
  package: 234 raw tables, one licensing exclusion, 233 selected; eighteen original
  contracts and separate-library consumers compile without PCH, 215 unbound remain
  red. Nine implicit-field mutants plus source-number/page-expression/library-drop/
  namespace controls reject. Original page binding has six passing checks;
  original library ownership/reflection has 221. These are declarations, not live
  providers, complete app compilation or G1. Preserve `4dadfec`/`5a14741`.
- `make include-cost HEADERS='platform/Integer.h platform/Field.h'` qualifies both
  headers standalone. Their compiler dependency sets remain 759/760 respectively,
  unchanged against `9dca232`; no transitive dependency was added. Concurrent-run
  frontend timings are not a performance-improvement claim.
- Shared Char decoding and Text iteration now use constexpr UTF-8 framing constants
  and one width primitive. `TextGate` passes 91 checks, `TextBuiltinGate` 43 and
  `make text-positions` ten generated AL checks. Source-index and two payload-mask
  mutants fail execution; malformed framing still refuses. `make lint-one
  UNIT=test/gate/TextGate.cpp` passes. PlatformSource retains 3060 passing checks;
  its focused tidy now passes after clearing all 33 shared-header diagnostics.
  Record snapshots keep private ownership/table/identity state with explicit borrows;
  RecordRef passes 147 checks, Variant 58, including exact Duration-to-Decimal
  conversion across signed 64-bit bounds. Public-owner and assignment-state mutants
  reject, bringing reflection-metadata to 34 controls. New/changed gate code has no
  tidy findings; RecordRefGate remains red on two unchanged Table.h diagnostics,
  VariantGate on nineteen existing diagnostics outside the changed code. No
  suppression or baseline rises. Full tidy and the native-migration AL replay remain open.
- The shared qualifier emitter has a passing focused tidy receipt. Both its
  original-table and original-codeunit compilation paths include public headers;
  `make native-codeunits` retains source-owned refusal controls and all 61
  original Base64 checks.
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
