# System declaration regression inputs

Fourteen original declarations, normalized to LF with a final newline. Twelve
come from System.app 28.0.53152.0 / runtime 17.0, package SHA256
`5b72ba127cb2221722f02544bfb3f3bd5a50402bf92bfab6d7e584e049b9681d`.
Privacy Notice and Privacy Notice Approval come from System.app 29.0.55365.0 /
runtime 18.0, package SHA256
`f59a4e4200af2b819670655302ce4ba4bfdd51e133cae6d4faf7896e5bba6b44`.
Original Microsoft notices and MIT permission text are retained in `License.txt`.

Page Table Field supplies the production native-source compiler tests, including
original bare/numeric page bindings, arbitrary filenames, refused source/identity
controls and native-extension ABI checks. It is separate from the thirteen-family
PlatformSourceGate population below.

`make gate GATE=PlatformSourceGate JOBS=2` compares every declared field, option
position, key and represented property with the handwritten/runtime declarations
and compiler aliases. Negative controls mutate identity, field count/number/type/
length, key order and option population. `Field` checks all 24 fields, compact
native Type codes, classification order and external-name identity/length; wrong
codes and missing package fields fail. Reflection includes all implicit system
fields currently represented by the runtime and maximum-length text roundtrips.
Company/User/Record Link/Date/Integer/All Profile retain original global scope,
field types, lengths, keys and represented properties. Removed URL and pending
profile-note fields retain their obsoletion metadata; retaining metadata does not
authorize AL references to removed fields. Authentication email and record-link
user identifiers preserve Text case. Each family's dropped-field control fails.

Page Metadata retains all 32 source fields; Table Metadata retains all 23. Their
Caption is Text[80], independent of Name. Reflection option codes are source-owned:
HeadlinePart is 12; TableType.Query is 5, not CDS. Table Metadata has the completed
implicit `ID` key; Page Metadata has its explicit `pk`. Both retain five represented
system fields. Caption-length, invented-key and option-vocabulary mutants must fail.
The compiler's common key completion runs on a copy of the independently parsed source.

`make reflection-metadata` also verifies named property-to-reflection mappings,
temporary rows and refusal of unqualified live reads/writes. Ordinal casts, CDS-to-Query,
unknown-type fallback and a removed provider guard must fail. The old partial physical
Page/Table Metadata seeding is removed; existing database rows remain untouched.
These virtual tables need a live read-only provider and qualified schema identity,
not a corrected declaration over stale persisted snapshots.

`test/transpiler/page-record-binding/` exercises the metadata records through generated AL:
named and numeric aliases, all seven option vocabularies, quoted newly represented
fields, independent Name/Caption, temporary Insert/Get/Count and per-call isolation.
The exact option-code sum is 30; a successful C++ declaration audit alone is insufficient.

Set `AGIRU_SYSTEM_SYMBOLS` when running `make test` to verify the package and audit
its actual original sources through `test/run.sh`. The C++ gate runs once, with the
verified package when supplied. Missing or invalid
input fails, never falls back. Frozen integration supplies its immutable package
automatically. The C++ gate accepts an optional package-root argument, reads it
before testing, and does not access mutable process environment state.

`Field.Table.al`, `PageMetadata.Table.al` and `TableMetadata.Table.al` are also
byte-identical in the verified System.app 29.0.55365.0
package from platform 29.0.54011.55407 (SHA256
`f59a4e4200af2b819670655302ce4ba4bfdd51e133cae6d4faf7896e5bba6b44`).
Other families may differ between releases: 29 adds inherent permissions to the
privacy tables; their current fixtures preserve those properties. Report such
mismatches; never replace the demo pin or weaken checks.

These are declaration checks, not System-loader activation, SQL migration,
Scope/fieldgroup dispatch, privacy workflows or live provider acceptance. A
`Cloud` declaration is extension availability, not a product integration mandate.
Runtime-18 audit FlowFields, timestamp metadata, default-key SQL enforcement and full
property ownership remain separate gaps. Populated schemas must not silently
change company ownership or reinterpret columns after a declaration correction.
