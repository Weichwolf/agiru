# Tests

| Directory | Purpose |
| --- | --- |
| `gate/` | Focused C++ regression tests for compiler and runtime primitives. |
| `transpiler/` | AL input fixtures, source binding, generated-code compilation and native declaration contracts. |
| `runtime/` | Runtime integration fixtures: isolation, test contexts, catalogue/reflection and number sequences. |
| `reporting/` | Report declarations, layout asset integrity and registry/linking fixtures. Not full rendering coverage. |
| `tooling/` | Build, lint, snapshot and test-runner checks; their Python helpers and existing diagnostic baselines. |

`transpiler/golden/` contains authored C++ output specifications. Do not regenerate
them from observed compiler output. Keep authored AL fixtures with their owning
integration tests; original platform declaration fixtures belong under `transpiler/`.
Retain original notices.

`run.sh` runs the local regression population. `slice` is the ordered generated C++
integration slice, not the AL test denominator.

Use `make gate GATE=RecordRefGate JOBS=2` for a focused C++ regression,
`make test JOBS=2` for all local checks, and `make tc JOBS=2` after generator changes.
`make verify-check` checks build tooling without rebuilding C++.

`runtime/test-contexts.sh` executes generated AL call/selection semantics and negative
controls. Its shared sorting controls require the compiled `gate_CurrentKeyGate`;
`make test` builds both the transpiler and all gates before running the script.

`runtime/boolean-expressions.sh` executes eager, ordered Boolean operands, owned
values, error/TryFunction boundaries and lazy ternary branches; an operand-order
source mutant must fail. `make boolean-expressions JOBS=2` is the focused entry point.

`transpiler/control-extensions.sh` executes page/report-request-page control order,
forward anchors and property overrides. Missing/cyclic anchors must refuse in both
analysis and generation, retaining previous output. `make control-extensions JOBS=2`
is the focused entry point; move operations and modified triggers remain separate gaps.

`transpiler/native-table-ids.sh` executes source-owned `Database::` constants through
codeunit/table/page/report procedures without promoting unbound native declarations to
record providers. ID mutation, missing/excluded sources and identity collisions must
fail. Qualified names and local Option shadowing remain distinct. The fixture's one
unbound table keeps translation nonzero; `make native-table-ids JOBS=2` qualifies the
constant path, not native provider/business execution or the UT milestone.

`transpiler/native-codeunits.sh` executes void/value/named-return/overloaded/local
Native refusals before var/stream/event effects. Removing Native must fail the
compiled runner. `make native-codeunits JOBS=2` is the focused entry point;
the source-bound fixture uses production `--system-symbols` loading, indexes bare,
qualified and numeric identities, links cross-codeunit AL calls and checks registry
metadata and original module ownership. Wrong source IDs and missing definitions
must fail compilation/linking. Unbound Native calls remain named refusals.
An explicit verified `AGIRU_SYSTEM_SYMBOLS` compiles all nine original Base64
overloads: five text-input bindings execute; four InStream overloads retain named
refusals. Wrong-encoding and terminated-output controls must fail. The text-output
profile includes the original 10 MiB character boundary; larger transform branches
and locale-default codepages refuse explicitly. This is not all thirteen original
AL tests, complete native behaviour or authentication of authored System fixtures.

`runtime/xml-reader.sh` proves shared cursor/close and consuming DOM-load contracts
in `XmlReaderGate`; separate-state and raw-input reload mutants must fail. It covers
positioned/ended readers, node ownership, namespaces, DTD retention and whitespace.
DTD/resolver security, encoding and streaming bounds remain open (0035).

`make hashing JOBS=2` proves named MD5/SHA1/SHA256/SHA384/SHA512 byte-array and
region hashing, shared disposal and generated AL calls. Three compiled mutants
must fail both consumers. The primitive and consumers also run under ASan/UBSan;
OpenSSL and the whole runtime are not instrumented. `AGIRU_HASH_REFERENCE=<TSV>`
replays the separately measured CLR corpus. Header controls keep OpenSSL/Array
implementation dependencies private. Keyed/stream/transform hashing, complete
System.Array identity/type rules and WASM qualification remain open (0035).

`make conversion JOBS=2` proves CLR Convert byte-array Base64 overloads, typed
formatting options, regions, byte validation and block-seam line breaks through
C++ and generated AL. Four compiled mutants must fail both consumers; converter/
byte-array helpers and consumers run under ASan/UBSan, not the whole codec/runtime.
Use `AGIRU_BASE64_REFERENCE=<original-core-TSV>` and
`AGIRU_CONVERT_REFERENCE=<CLR-region-TSV>` for the measured external populations.
`make hashing` additionally executes the generated HashAlgorithm→Convert chain.
The four numeric Convert methods remain named refusals; no complete Convert,
System.Array or ERP milestone is claimed (0035).

`make streams JOBS=2` proves shared cursors, fresh/wrapper-local bindings, escaped
local BLOB providers and independent BLOB values through C++ and generated AL.
FileGate and that AL consumer also prove file binary/text modes, declared text
capacity, zero terminators and zero-based positions. The C++ empty-doctype path
loads a reader, replaces the doctype, saves and reads the file back.
Both consumers also run with their stream/provider primitives under ASan/UBSan;
the whole runtime/File implementation is not instrumented. Nine compiled stream/
provider/File mutants must fail their C++ and AL consumers. File/record/codeunit
lifetimes, AL assignment/Clear/disposal,
encoding, bounds, nonseekable/BigInteger positions and Native activation remain open
(0035/0034).

`runtime/codeunit-record.sh` executes generated typed/static/dynamic `Codeunit.Run`
forms. Table-global saves survive SQL rollback through scoped var-Record identity;
ordinary assignment remains independent. Nested runs, callee restoration, handles,
temporary cursors and compiled no-borrow/no-restore/assignment-alias controls are covered.

`runtime/page-navigation.sh` executes generated list/card system Edit routing,
selected-record identity, opening triggers, explicit-action precedence and card
ModifyAllowed policy. View navigation and general command permissions remain open (0030).

`runtime/reflection-metadata.sh` verifies declaration projection, timestamp-free AL
field indices and shared compiled record filters, including group intersections,
cross-column OR, FlowFilters and owned expression snapshots. Fifty-one compiled
controls and the narrow system-field header dependency control must reject.
Installed Table Metadata.Get and positive-key Field.Get share typed/RecordRef readers;
timestamp zero remains addressable through FieldRef, and temporary zero keys remain valid.
The Field reader checks the native ABI before accessing its buffer. Typed missing reads
retain optional-result semantics; ordinary filters stay unchanged. Native Field
Find/FindSet/Next/Count/IsEmpty use a shared immutable positive-key index, never a SQL
copy or per-session row catalogue. `FieldCatalogueGate` checks independent bookmarks,
filters/marks/signed navigation, the real name-derived FieldRef caller and exact metadata
identity/version. Native writes, including empty ModifyAll/DeleteAll(true), refuse; temporary rows
remain independently writable. Four unprojected attributes refuse filtering/ordering.
Complete metadata providers, authorization and secondary-order performance remain gaps (0044).

`make record-order JOBS=2` verifies mixed field directions, global reversal,
primary-key ties, filters, relative searches and cursor/keyset direction changes.
Cursor/Filter gates also qualify native Integer SQL/typed/RecordRef projections:
virtual timestamp 1, blank identity/audit values, physical timestamp alias and
explicit refusal of unknown or mistyped stored fields. Compiled zero-version and
source-alias controls reject; wide/unfiltered series cardinality remains a gap.
The same exact-value matrix runs through typed Record and RecordRef on SQL and
temporary rows. SelectionChangeGate adds changed filters/copies, keys/directions/views,
active marks, unchanged setters and shared temporary Modify visibility. CursorLifecycleGate
adds Commit/rollback buffer recovery, released/surviving/absent portal cleanup, partial
and reversed walks. DynamicRecordGate adds same-session Modify/Insert/Delete/Rename,
ModifyAll/DeleteAll and filter admission/exclusion through typed Record/RecordRef,
inside/across fetch blocks, with uniform/mixed keys, global reversal and already-open
backward cursors. Own-variable Modify/Delete/Rename preserve the frame/system identity
and resume from its current key. Own ModifyAll retains the caller's buffer, system identity
and position on SQL/shared temporary rows in all eight orders, including block boundaries.
Revision storage follows active readers, not historical
table visits; session/table/connection isolation and failed/temporary writes are checked.
RenameGate retains the cascade reader's old key while a separate record writes the
new key. Typed/reflected parent renames in both directions retain every keyed/non-key
child and its exact aggregate at 1/64/130 rows, including unrelated-parent controls.
Twenty-three compiled controls must reject, including caller-traversing ModifyAll
and a cascade that overwrites its read anchor;
traced SQL counts reject a functional-green
one-row fetch. Full AL dynamic-write replay and Query transaction contracts remain open;
this is not client-page presentation or live metadata-provider activation (0044).

`make base64 JOBS=2` qualifies the shared raw byte codec, 76-column CRLF profile,
CLR Convert decoder rules, bounded stream writes and borrowed-input safety. Five
compiled mutants must fail. Optional `AGIRU_BASE64_REFERENCE=<TSV>` replays an external
byte-level reference population through both output forms; the receipt retains its
hash. It does not bind Native methods or prove text encoding, stream input/cursors,
CLR transform-block decoding or execution of the original BC Base64 tests (0034).

`make encoding JOBS=2` qualifies Unicode replacement, UTF-16 char-array units,
Windows-1252 best-fit, distinct ASCII/Latin-1, factory aliases/preambles and explicit
unsupported-page refusals. Twelve compiled mutants must fail. Optional
`AGIRU_ENCODING_REFERENCE=<TSV>` compares the declared encoding profile against
original native text cores, retaining total/selected/outside counts and hashes.
`AGIRU_CODEPAGE_REFERENCE=<TSV>` requires every BMP encode/byte decode identity,
rejects duplicates and checks every supplementary scalar against the original
single-byte fallback. `AGIRU_ASCII_REFERENCE` and `AGIRU_LATIN1_REFERENCE` apply
the same complete-population checks to their respective original native tables.
Other codepages, locale defaults, array bounds/types, Native
binding and original AL execution remain gaps (0034/0035). Data notices: `licenses/`.

The ERP milestones are separate: `make ut` executes the source-counted AL UT
population through `agiru run-tests`; the full AL suite follows. Their original
source is in BCApps, not in these authored fixtures. A green local regression
run does not establish either ERP milestone.
