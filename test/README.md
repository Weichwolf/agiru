# Tests

| Directory | Purpose |
| --- | --- |
| `gate/` | Focused C++ regression tests for compiler and runtime primitives. |
| `transpiler/` | AL input fixtures, source binding, generated-code compilation and native declaration contracts. |
| `runtime/` | Runtime integration fixtures: isolation, test contexts, catalogue/reflection and number sequences. |
| `reporting/` | Report declarations, layout asset integrity and registry/linking fixtures. Not full rendering coverage. |
| `tooling/` | Build, lint, snapshot and test-runner checks; their Python helpers and existing diagnostic baselines. |
| `ui/` | Shared semantic HTML, external agent CMD/MCP transports and future actual browser/SQL parity. |

`transpiler/golden/` contains authored C++ output specifications. Do not regenerate
them from observed compiler output. Keep authored AL fixtures with their owning
integration tests; original platform declaration fixtures belong under `transpiler/`.
Retain original notices.

`run.sh` runs the local regression population. `slice` is the ordered generated C++
integration slice, not the AL test denominator.

Use `make gate GATE=RecordRefGate JOBS=2` for a focused C++ regression,
`make test JOBS=2` for all local checks, and `make tc JOBS=2` after generator changes.
`make verify-check` checks build tooling without rebuilding C++.

`make dev-check` is a host-only Podman packaging check, separate from `make test`:
it uses owned disposable containers/volumes to verify committed SQL persistence,
supervisor exits, container ownership refusals and incompatible storage preservation.
It does not qualify an HTTP server, client parity or the BC seed. Run container C++
gates through `make dev-exec` with `B=/workspace/build/podman`.

`runtime/test-contexts.sh` executes generated AL call/selection semantics and negative
controls. Its shared sorting controls require the compiled `gate_CurrentKeyGate`;
`make test` builds both the transpiler and all gates before running the script.

`runtime/boolean-expressions.sh` executes eager, ordered Boolean operands, owned
values, error/TryFunction boundaries and lazy ternary branches; an operand-order
source mutant must fail. `make boolean-expressions JOBS=2` is the focused entry point.

`runtime/for-loops.sh` executes owned, once-evaluated bounds for ascending/descending
Integer and Boolean loops, empty ranges, bound errors, global/var/nested counters,
Option and exact BigInteger values, hygienic temporaries, break and continue. Four
compiled controls reintroduce repeated Integer/Boolean/typed-queue evaluation and
borrowed mutable bounds; all must fail. `make for-loops JOBS=2` is the focused entry point. Bound conversion,
terminal overflow and Decimal/Date/Time stepping are not qualified by this profile.

`make variant-text JOBS=2` proves shared Variant/Format/report display text, typed
FieldRef assignment, sparse ordinals, captions/name fallbacks, blank members and
exact Unicode. Five compiled defects must fail their consumers, including numeric
report ordinals and display-formatted XML Boolean scalars; Decimal scale 28 stays
exact. Strict typed Get and numeric Format 2/9 remain distinct. Session-language
caption selection and broader native conversion/culture guarantees remain open.

`make xmlport-import JOBS=2` executes generated AL field/attribute assignment before
validation, Yes/No/Undefined/default inheritance, temporary XMLport source nodes
and propagated validation errors. Explicit Record.Validate still runs on temporary
records. Five compiled defects must fail; fixtures use AutoSave=false and do not
write SQL rows. This profile does not qualify every XMLport import/trigger policy.

`make rowversions JOBS=2` qualifies allocator fences and SQL record/SystemId paths.
The disconnect gate observes the specific backend's termination before asserting
the unchanged active minimum; a live backend must time out without hiding its fence.
A compiled delayed-close adapter must pass, while removing the observation must fail.
This is a test synchronization contract, not a blocking production destructor.

`transpiler/control-extensions.sh` executes page/report-request-page control order,
forward anchors and property overrides across separate declaring/extension apps.
Application identity, raw AL namespace and MultipleNewLines survive composition;
defaults and independent names/captions execute. Three compiled metadata controls reject.
Missing/cyclic anchors must refuse in both analysis and generation, retaining previous
output. `make control-extensions JOBS=2` is the focused entry point; complete live Page Metadata,
move operations and modified triggers remain separate gaps.

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
`XmlGate` also checks call-local DOM failure classification, first-error retention
and qualified UTF-16 positions. Eleven compiled controls reject; the resource/context
trap checks external-resource requests and process-global error-handler writes.
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
ModifyAllowed policy. `PageDispatcherGate` drives the shared command primitive over
existing page bindings: per-command authorization, exact identities, current inherited
and computed control state, refusal before effects and unchanged AL errors/text.
The production `PageSession` and AL `TestPage` adapter share validation/save/trigger
execution. Generated delayed insertion and edit cases reconcile direct SQL results;
production save errors propagate while AL test error collection remains explicit.
Generated TestPage copy/move rebinding and request-page fields/filter accept/cancel
paths retain adapter behaviour. The installed catalogue creates closed production
instances, not headless page runs; mode/cursor/RecordId paths use the same kernel.
AL controls Open/Move/Declaration retain their names. Missing/null/mismatched factories
refuse. Eleven execution mutants and a control-shadowing compile refusal must reject;
the narrow PageInstance interface must not pull in typed page/control/record headers.
`make page-profile JOBS=2` additionally checks exact bound scalar values and a bounded
current-row semantic HTML fragment over the same production handles. Display text is
separate from canonical Decimal/Int64, enum domains/member names and temporal flags.
Unqualified variable/expression bindings, parts and non-scalar/filter values remain
counted gaps. Nine scalar/HTML execution mutants must reject, including execution of
dynamic visibility before authorization. UTF-8 validation reuses
the codec through a narrow header; malformed text never silently changes values.
This is not HTTP/CMD/MCP/browser parity, production permission storage, a complete
list window or complete page lifecycle. View navigation remains open (0720).

`make client` builds the external Node 20+ TypeScript client from its locked
dependencies. `make client-test` runs on the host: C++ `PageHtmlGate --html` produces
the actual fragment in `agiru-dev`; a clearly labelled Node HTTP fixture checks
lossless values, shell CMD and real MCP stdio calls over one agent library. Four
executable mutants must expose rounded scalars, disabled-command execution,
stale revisions and duplicate POSTs. This is not a Node ERP server or proof of
production authentication, SQL effects, actual htmx browser behaviour or complete
page/ERP parity. `AGIRU_PAGE_HTML_GATE` can select an explicitly built host producer.
Fixtures and disposable mutant modules use `/tmp`; Node stays outside the ERP container.

`make http-test` qualifies the native transport in `agiru-dev`: unmodified nginx
on the sole published loopback port, libmicrohttpd on private container loopback and
PostgreSQL together; actual external CMD and MCP calls preserve the C++ HTML profile.
Eleven HTTP cases cover raw encoded URLs, exact POST/binary bytes, independent SQL
transport records, static-file separation, forged forwarding headers, ambiguous
framing refusals, request/response/header bounds, aggregate upload ownership,
bounded worker admission and shutdown. SQL records are transport receipts, not AL
Validate/Save/posting effects. The fixture's short-lived connections do not qualify
production session leases. TLS, authentication/session authority, modal continuation,
actual htmx browser execution and complete ERP parity remain pending.

`runtime/reflection-metadata.sh` verifies declaration projection, timestamp-free AL
field indices and shared compiled record filters, including group intersections,
cross-column OR, FlowFilters and owned expression snapshots. Seventy-seven compiled
controls and the narrow system-field header dependency control must reject.
Installed Table Metadata.Get and positive-key Field.Get share typed/RecordRef readers;
timestamp zero remains addressable through FieldRef, and temporary zero keys remain valid.
Both readers check the native ABI before accessing their buffers. Typed missing reads
retain optional-result semantics; ordinary filters stay unchanged. Native Field
Find/FindSet/Next/Count/IsEmpty borrow a shared immutable positive-key index;
Table Metadata borrows InstalledTables directly; qualified Page Metadata rows borrow
InstalledPages. One catalogue filter/order/navigation
kernel uses call-scoped scratch rows, never SQL copies or per-session populations.
`FieldCatalogueGate` checks independent bookmarks,
filters/marks/signed navigation, the real name-derived FieldRef caller and exact metadata
identity/version. `TableMetadataCatalogueGate` adds raw-identity counts, sparse filters,
native option/mixed ordering, Get/Next anchoring, RecordRef parity and ownership/binding
refusals. `PageMetadataCatalogueGate` covers native source/card IDs, Name/Caption,
policy flags, original app identity, UTF-16 widths, bookmark/filter/mark/order and
typed/RecordRef parity. Page caption fallback recognizes the 25 .NET whitespace
characters without trimming nonblank caption data; ASCII-only and zero-width-as-blank
mutants reject. Shared bookmark/filter/key-hole mutants fail all applicable
catalogue gates; page-specific controls detect wrong projections and fabricated
defaults. `PageRecordBindingGate` executes generated AL list-to-card Get and
navigation in named/numeric forms without copying native source declarations.
`CatalogueFlowFieldGate` exercises scalar CalcFields/FieldRef calculations over all
three native catalogues: exact typed lookups, seven aggregate kinds, six operand modes,
duplicate predicates, empty values, refusal boundaries and unchanged source views.
Resolved expressions are not reparsed as filters; SQL/native literal semantics stay shared.
Catalogue scans borrow provider scratch rows, bound the leading-key window and stop
Lookup/Exist early, never copying the installed population. Native calculations do not
remove storage write guards or create Field SQL copies. The AL page fixture executes
the native caption CalcFormulas through the generator as well as FieldRef.CalcField.
Native calculated predicates and SQL-correlated native columns remain counted gaps;
scalar execution is not proof of the full provider or a complete UT milestone.
Canonical views, caption expressions/field lists, masks, dynamic properties, compiled
API formats/defaults, absent-source contracts and non-English captions still explicitly
refuse; key-only counts retain their identities. Native writes, including
empty ModifyAll/DeleteAll(true), refuse; temporary rows
remain independently writable. Four unprojected attributes refuse filtering/ordering.
Complete metadata providers, authorization and secondary-order performance remain gaps (0044).

RecordRef.Get shares typed Get consumption and captioned primary-key diagnostics:
consumed misses return false, discarded misses raise, malformed/provider failures still
throw. RecordRef/native metadata gates exercise temporary/live reads; SqlRowVersionGate
adds owned SQL exact-value/filter/read-only/error checks. Two compiled controls must fail
both temporary and native paths. `make verify-check VERIFY_CHECKS=TableSourceBindingGate`
executes both AL Get forms in named/numeric and table/codeunit fixture contexts.

`make record-order JOBS=2` verifies mixed field directions, global reversal,
primary-key ties, filters, relative searches and cursor/keyset direction changes.
`make record-position JOBS=2` selects its database-free position profile: one
shared Record/RecordRef CONST codec, caption defaults, exact primary-key values,
borrowed-input safety and cursor invalidation. Nine compiled defects must fail.
The full ordering profile also executes assigned existing/missing/out-of-filter
anchors through typed/RecordRef readers on SQL and temporary tables. Regional
scalar formatting and full native position conformance remain open (0044).
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
Thirty-seven compiled controls must reject, including caller-traversing ModifyAll
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
