# Tests

| Directory | Purpose |
| --- | --- |
| `gate/` | Focused C++ regression tests for compiler and runtime primitives. |
| `transpiler/` | AL input fixtures, source binding, generated-code compilation and native declaration contracts. |
| `runtime/` | Runtime integration fixtures: isolation, test contexts, catalogue/reflection and number sequences. |
| `reporting/` | Report declarations, layout asset integrity and registry/linking fixtures. Not full rendering coverage. |
| `tooling/` | Build, lint, snapshot and test-runner checks; their Python helpers and existing diagnostic baselines. |
| `ui/` | Shared semantic HTML, external CMD/MCP and actual browser/SQL operation parity. |

`transpiler/golden/` contains authored C++ output specifications. Do not regenerate
them from observed compiler output. Keep authored AL fixtures with their owning
integration tests; original platform declaration fixtures belong under `transpiler/`.
Retain original notices.

`run.sh` runs the local regression population. `slice` is the ordered generated C++
integration slice, not the AL test denominator.

`make session-identity JOBS=2` qualifies host-supplied user GUIDs against the system
User table, committed audit ownership and persistent AL state across exclusive
per-command SQL leases. Thirteen compiled defects test identity, rollback, durable
completion, cursor ownership, credential expiry/revocation/ownership and crypto failure.
Disposable databases also check idle contexts, worker reuse, revocation, deferred failures
and broken-connection cleanup. Native agent bearer credentials store only verifiers;
random/digest provider failures explicitly refuse. The provider fixture has a successful
compile receipt before lint. This is not browser/password sign-in, a connection pool or page/table authorization;
container qualifiers need `AGIRU_TEST_DSN` pointing at container-local PostgreSQL.

`make browser-sessions JOBS=2` qualifies PostgreSQL browser identities separate from
agent bearers: source-capped idle/absolute expiry, revocation, atomic rotation/rollback,
CSRF verifiers and serialized per-user admission. Nine compiled defects must fail named
checks; RFC 4231 MAC vectors, provider failure and secret-free SQL statement/row tracing
are checked independently. Disposable databases and private secrets are removed.
This is a storage gate, not live HTTPS cookies, browser login or SaaS acceptance (0720).

`make browser-auth JOBS=2` checks the native cookie/CSRF adapter and production page
front door; nine compiled transport/retention/configuration defects must reject.
`make browser-https-test` exercises real Caddy TLS, private libmicrohttpd and PostgreSQL
in one disposable container with an external protocol client and independent SQL probes.
Actual Chromium exercises the htmx cookie client; external CMD/MCP retain the same
exact generated-list values. Independent SQL checks Validate/Save/replay, passive
bootstrap, tab isolation, logout and expiry during an unanswered AL question.
Malformed grants must refuse without bearer fallback or AL execution. The private CA
is explicitly trusted in an isolated profile after an actual browser rejection, never
bypassed. Protocol probes retain metadata/origin denials, rotation and source expiry.
Neither target proves stolen-token device binding, full business workflows or SaaS security.

`make session-identity JOBS=2` also checks session-owned ApplicationArea and random sequences through
nested, reused, migrated and concurrent workers. Six compiled ownership/seed/bound/clock
defects must fail named checks. Explicit zero seeds and a compiled fixed-clock fixture
distinguish both Randomize overloads; generated callers preserve omitted arguments.
Integer maximum is covered; Random's minimum Integer bound explicitly refuses and
remains unqualified against BC. This is not suspended AL-stack migration or full lint/UT.

`make ui-host JOBS=2` qualifies the session-owned native dialog bridge: 154 C++ checks,
205 native-configuration checks and eleven compiled capability/answer/test-fallback/
implicit-commit/live-binding/callback-policy defects.
Background GuiAllowed is false; actual host callbacks and explicit AL handlers are
separate. Questions never implicitly commit, prior Commit survives unwind, and live
progress values remain exact. Detached authenticated sessions retain their own endpoint
across workers. This is not HTTP dialog admission/suspension, automatic progress teardown,
modal-page or browser/CMD/MCP acceptance; those remain in WI 0720.
The trusted `transactions.allow_session_call_suspend_when_write_transaction_started`
default is true. False refuses Confirm/StrMenu before native or AL test callbacks in a
write transaction; read-only callbacks, messages and progress remain allowed. Both policies,
Commit/unwind, independent SQL visibility and worker migration are covered. Modal and
report/request-page callback enforcement still requires their actual UI integration.

`make record-windows JOBS=2` runs the row-bounded SQL window gate and five compiled
bound/seek/reverse/permission/continuation defects through the existing record-order
script. Its owned database explicitly uses UTF-8, with C and case/accent-insensitive
ICU column profiles. Independent SQL ordering/filtering checks cover 0/1/39/40/41
rows, limits 7/40/80, ties, mixed directions, Unicode, numeric Codes, filter groups,
deleted anchors, exact stored values and pending-write rollback. The full record-order
target retains its 37 existing controls. This is not page trigger/HTML/client parity,
BC collation equivalence, byte/scan bounds or qualification of native SQL_ASCII seeds.

`make table-permissions JOBS=2` checks the session-owned TableData boundary for typed
and reflected records, buffered record/query reads, individual write kinds, temporary
buffers and company isolation. Independent SQL verifies denied writes; two compiled
permission-bypass/partial-write defects must fail. The SQL grant table is an authored
fixture, not native BC permission-set composition, indirect rights or security filters.
Authenticated sessions without an authority refuse instead of becoming SUPER.

`make http-test JOBS=2` runs transport and native credential tests through actual Caddy,
C++ and PostgreSQL in `agiru-dev`, with CMD/MCP outside. Authentication uses disposable
users/databases/private auth files and independent SQL checks; these authored static HTML
fixtures do not qualify production ERP pages, permission sets or posting parity.

`make page-host-test JOBS=2` runs generated List → Card → Validate/Save over Caddy,
C++ and PostgreSQL with external CMD/MCP. Independent SQL checks typed values,
modifier identity, write counts, command replay/revisions, rollback and durable AL Commit.
Generated actions also attempt denied reads/inserts on a second table; HTTP must refuse
without disclosing its values or changing either table.
AL calls use a separate bounded executor and shared profile-3 `working` status; clients
poll the opaque call instead of repeating admission. Delayed field writes run with one
HTTP worker and independent SQL observers; CMD/MCP/htmx must retain exact final values,
one write effect, rollback/durable Commit and original failure identities. Questions and
modals retain the original AL stack, explicit answers and timeout/cancellation rollback.
This does not qualify queue saturation, disconnect cancellation or multi-user scale.
Compiled ownership/revision/replay/policy/duplicate/list-bound/failed-receipt and
synchronous-execution defects must fail named cases. The fixture
has real generated pages and SQL grants, not a full BC permission provider or browser
acceptance. It reuses the page-navigation compilation pipeline and private auth-file helper.
Separate credentials for the same user cannot read/write/replay each other's contexts,
poll calls, answer questions, operate modals or fetch modal receipts. Independent SQL
checks verifier ownership and unchanged effects; two compiled client-binding defects
must fail. Legacy unbound contexts are invalidated without deleting their receipt evidence.
This is bearer-client isolation, not browser cookie login or stolen-token device binding.
Fresh modal inputs renew the idle answer wait within the absolute context lifetime;
reads and identical command replay do not. Independent SQL verifies no premature commit,
and a compiled fixed-deadline defect must fail the active-input case.

`make erp-fixture JOBS=2` qualifies a disposable clone of the verified native shared
CRONUS transfer. It copies all eleven original Company columns exactly, provisions the
four generated System permission tables, and assigns one explicit fixture-only tenant
administrator role. A second authenticated user has no role; native Customer/Page rights,
company isolation and a denied AL write with independent SQL effects are checked.
The original source and seed remain unchanged; owned database, binaries and private tokens
are removed even on failure. A non-fixture DSN must refuse before connecting. This is
client preparation, not actual Customer execution, system-role installation or a full seed.

`make erp-client-test JOBS=2` reuses that isolated fixture and freezes existing native
binaries under `/tmp` while the external CMD/MCP and actual Chromium/htmx use Caddy.
The shared browser helper compares rendered controls, typed values and command identities
with the agent model; native SQL remains the independent effect oracle. Eleven explicit cases
cover unassigned-user refusal, original Customer List state, Card selection, Unicode Name
Validate/Save and original template-based creation/edit/reopen via CMD/MCP/HTML forms,
with independent SQL/audit/rowversion/receipt effects and unchanged ledger fingerprints.
Dependent cases fail explicitly when their prerequisite did not execute; they never vanish
or become skipped acceptance. The accepted baseline passes eleven cases; a copied existing
binary is not proof of the current integration build, full ERP parity or posting acceptance.

Use `make gate GATE=RecordRefGate JOBS=2` for a focused C++ regression,
`make test JOBS=2` for all local checks, and `make tc JOBS=2` after generator changes.
`make verify-check` checks build tooling without rebuilding C++.
`BuildSourcesGate` compiles the shared CMake source projection and tests stale/missing
inputs, exact product exclusions and separately counted selection omissions. Raw slice
identities and unity assignments stay unchanged. `make slice-check` requires the current
generator source list, policy/app-root snapshots and original-source map; file presence
alone is insufficient. The transpile wrapper preserves removed source identities, including
after a successful sweep. Generated-library builds recheck the same contract; missing inputs
do not prevent handwritten gates or the transpiler from building.
Native platform declarations use bounded product exclusions; BCApps namespace/area
reachability does not remove required system declarations. Native inventory retains both
the namespace diagnostic and the independently selected identity.
Frozen runs pin the selected build's database, source, app/slice and build-type settings,
with explicit database/source overrides. Private configuration is hash-checked; `make
configure CMAKE_ARGS=<declared-settings>` runs before targets, including reused builds.
Missing/changed configuration or configure failure refuses execution; inherited `B`
cannot select live outputs. SnapshotGate covers quoted values, private storage and refusal.

`make dev-check` is a host-only Podman packaging check, separate from `make test`:
it uses owned disposable containers/volumes to verify committed SQL persistence,
supervisor exits, container ownership refusals, incompatible storage preservation,
local TLS/redirects and persistent certificate storage with the admin API disabled.
It does not qualify public ACME, ERP/client parity or the BC seed. Run container C++
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
records through an explicit AL procedure, not a generated C++ member name. Mutation
anchors resolve the emitted record binding and refuse missing/ambiguous matches.
Five compiled defects must fail; fixtures use AutoSave=false and do not
write SQL rows. This profile does not qualify every XMLport import/trigger policy.

`make rowversions JOBS=2` qualifies allocator fences and SQL record/SystemId paths.
The PostgreSQL write-transaction UUID stays stable through released savepoints and
changes at Commit/rollback. Its cache is transaction-local; fixed, regenerated and
session-persistent tokens fail compiled controls. It does not allocate rowversions.
Typed Record and RecordRef.Init retain loaded timestamp aliases without writing SQL;
Clear still resets them. Removing timestamp retention must fail both record paths.
These checks do not yet qualify atomic stale-write rejection.
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

`make native-storage JOBS=2 AGIRU_SYSTEM_SYMBOLS=<verified-package>` translates four
original stored System permission tables through the ordinary table writer, preserving
the original module, namespace, fields, keys, options and InitValue. Its C++ runner creates
a disposable PostgreSQL database, writes/reads typed generated records and checks independent
SQL and native permission resolution. Changed source defaults/widths must fail. TableFilter
storage and Init are tested; enforcing a nonempty security filter remains an explicit refusal.
This pinned four-source qualification is not the full System-table inventory, virtual metadata
provider, system permission-set installation, Customer workflow or UT acceptance. The separate
intrinsic ABI audit reports generated stored sources as `storage-unqualified`, never an empty
successful assertion unit.

`transpiler/native-codeunits.sh` executes void/value/named-return/overloaded/local
Native refusals before var/stream/event effects. Removing Native must fail the
compiled runner. `make native-codeunits JOBS=2` is the focused entry point;
the source-bound fixture uses production `--system-symbols` loading, indexes bare,
qualified and numeric identities, links cross-codeunit AL calls and checks registry
metadata and original module ownership. Wrong source IDs and missing definitions
must fail compilation/linking. Unbound Native calls remain named refusals.
Ordinary AL calls on an unavailable Record compile implicit/quoted field arguments
for ten methods and refuse with the original member identity before subsequent effects.
Removing the implicit field declaration must fail compilation; no table schema or
successful fallback is invented. Emitter and dependency collection share one signature table.
Unavailable array members also compile for direct, nested and multidimensional indices,
including a bracket string literal in an index call and an implicit Validate field.
The compiled runner proves a side-effecting index runs once before the named refusal;
removing the matrix-only declaration must fail compilation. Case-normalized unavailable
type identity remains a separate generator gap, not covered by these member checks.
Unavailable scalar fields compile compound assignments and both Clear forms, then
refuse with their original member identity before subsequent AL effects. Removing all
six operations must fail the compiled runner. RefusedGate retains Decimal operands,
unknown-option Clear and an ordinary Integer Clear control. Whole absent-record Clear
and same-type refusal assignment are separate gaps, not qualified by these checks.
Record FieldNo calls retain the platform Integer result shape before C++ overload
resolution. Four unavailable-record/array cases refuse before later AL effects; a
selected record reaches the Integer overload and a Codeunit FieldNo retains Text.
Removing the result typing must fail compilation with an ambiguous Integer/JsonArray
overload. Side-effecting indexed method receivers remain a separate qualification gap.
Unselected named Page.Run/RunModal calls compile through the numbered runtime API,
including optional records, field numbers, Action results and case-insensitive names.
They throw with the original AL identity before subsequent effects, not through an
invented absent class or object zero. Removing these calls must fail the compiled runner.
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
temporary cursors, durable production Run, pending-write refusal and both configured
TryFunction write policies are covered. Three global-state and four transaction/scope
defects must fail. `TransactionContractGate` checks rollback/commit, collectible errors,
isolated-event fallback and report/XMLport Quit. `SessionParallelGate` checks concurrent
native HTTP sessions with independent committed/rolled-back SQL effects. These do not
certify BC isolation, complete AL transaction coverage or financial posting integrity.

`NativeServiceConfigGate` qualifies the complete `deploy/dev/agiru.json` profile:
exact numeric bounds, types, duplicate/unknown/missing keys and bounded regular files.
`ui/page-host.sh` starts the real CLI using only `serve --config`, proves both TryFunction
write policies against independent SQL and rejects compiled policy/duplicate defects.

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
Generated list/card instances also retain selection across detached SessionCommand
leases; validation/save is verified by independent SQL value and modifier-GUID reads.
Its authored permission allowlist does not establish production authorization or HTTP parity.
AL controls Open/Move/Declaration retain their names. Missing/null/mismatched factories
refuse. The production-only generated SQL window adapter additionally proves row/probe
bounds at 0/1/39/40/41/80 and limits 7/40/80, block trigger order, retained selection,
post-trigger record values/original images and rollback after loaded-row errors. It does not clone whole
pages or use TestPage.Next to enumerate rows. New/custom/temporary providers, refresh,
selected globals/xRec and full ERP providers remain unqualified.
Sixteen execution mutants and a control-shadowing compile refusal must reject;
the narrow PageInstance interface must not pull in typed page/control/record headers.
`make page-host-test JOBS=2` additionally exercises shared native profile-2 list rows
through Caddy, external CMD/MCP and actual Chromium. Trusted `pages.list_rows` defaults
to 40; actual-entry runs qualify 7/40/80, empty/one/39/40/41/81 populations, forward/
backward/last blocks, retained row handles and row selection. Exact Decimal/Int64 and
Unicode values remain unchanged; independent UTF-8 owned fixture SQL checks effects.
URL limits and malformed row profiles refuse; an ignored-config compiled mutant must
fail the list-bound test. Cards keep profile 1. This is not full ERP/BC collation parity.
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
lossless values, shell CMD and real MCP stdio calls over one agent library. Eight
executable mutants must expose rounded scalars, disabled-command execution,
stale revisions, duplicate POSTs, blocking authentication-file opens and unsafe MCP
read-only/idempotent hints, accidental page reopening during command preflight and
acceptance of a failure belonging to another command.
Execute must use the matching retained `/?handle=<page>` path and refuse an opening,
foreign, duplicate or mismatched path before any HTTP request.
Discovery must disclose that page opening can write from
AL triggers; neither operation promises safe automatic retries. FIFOs must
refuse without a writer; the defective child is killed after a bounded timeout.
This is not a Node ERP server or proof of
production authentication, SQL effects, actual htmx browser behaviour or complete
page/ERP parity. `AGIRU_PAGE_HTML_GATE` can select an explicitly built host producer.
Fixtures and disposable mutant modules use `/tmp`; Node stays outside the ERP container.
`make web-test` builds local htmx/static browser assets and executes fourteen real Chromium
cases against the same native `PageHtmlGate` fragment and agent profile/envelope.
Three compiled bundles must fail named response-effect, hidden-envelope and concurrent
POST cases. One additional actual Caddy case qualifies static assets/licenses, document
versus HX routing and security headers, with private certificate/config state and clean
shutdown. Screenshots, source/binary hashes and logs remain current receipts under
`/tmp`; mutant bundles are removed even on failure. This is not a Node ERP server,
native HTTP/SQL/browser workflow acceptance, complete lists, dialogs or production login.

Native page errors use the bounded `data-agiru-error="1"` HTML envelope: unchanged
error text/code, explicit command identity and `refused`, `failed` or `unknown` outcome.
`failed` requires durable page invalidation and a failed SQL command receipt; it never
promises to undo an earlier explicit Commit. `refused` describes this submission, not
historical effects of the same command. Empty AL classification becomes transport-only
`AlError`; AL error state is unchanged. CMD/MCP share strict decoding; htmx displays
untrusted text without replacing the retained page. Malformed, mismatched, disconnected
or unqualified write responses remain uncertain; no outcome authorizes automatic retries.
`ui/page-host.mjs` independently checks rollback, durable Commit, failed receipts and
actual external CMD/MCP/Chromium error parity. A compiled lost-receipt defect must fail.
`AGIRU_PAGE_GATE_DATABASE` selects a separate existing disposable gate database for its
generated navigation regressions; HTTP fixtures still create/drop their own databases.
Full ErrorInfo/actionable-error dialogs and live Confirm/StrMenu suspension remain gaps.
`tooling/header-dependencies.sh` checks the native server's narrow options-only headers;
forcing HTTP or retained page execution into `NativeService.h` must fail. Configuration
and execution share the same option types, not copied defaults or validation rules.

`make http-test` qualifies the native transport in `agiru-dev`: unmodified Caddy
on the sole published loopback port, libmicrohttpd on private container loopback and
PostgreSQL together; actual external CMD and MCP calls preserve the C++ HTML profile.
Twelve HTTP cases cover raw encoded URLs, exact POST/binary bytes, independent SQL
transport records, static-file separation, forged forwarding headers, ambiguous
framing refusals and CL/TE normalization (exact decoded body, no conflicting upstream
length or hidden SQL request), request/response/header bounds, aggregate upload ownership,
bounded worker admission and shutdown. SQL records are transport receipts, not AL
Validate/Save/posting effects. The fixture's short-lived connections do not qualify
production session leases. Public ACME/TLS, authentication/session authority, modal continuation,
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
The executable fixtures use an owned database/session for their consumed TryFunction;
temporary records do not replace session-owned error/transaction context. Native catalogue
write checks require the provider diagnostic, not merely any exception; a compiled
session-before-provider defect must fail both empty bulk-write checks.

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
