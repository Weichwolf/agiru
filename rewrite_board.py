from pathlib import Path
import json,re
root=Path('board')
original={p.name:p.read_text() for p in root.glob('*.md')}
Path('build/review-board-original.json').write_text(json.dumps(original))
items={}
def wi(id,title,priority,evidence,steps,proof,refs):
 items[id]=dict(title=title,priority=priority,evidence=evidence,steps=steps,proof=proof,refs=refs)
wi('0012','Production commits will be durable and concurrent writes will be checked','P0',
'`src/rt/Transaction.cpp::Boundaries::Commit` releases and recreates savepoints but never issues SQL COMMIT. `TransactionGate` reads through the writing session, so it cannot prove durability. `Session` owns a connection for its whole lifetime. `Table.h::LockTable` and `ReadIsolation` store state without enforcing SQL locks; `Storage.cpp::Updated` checks the primary key without an observed rowversion.',
['First add a two-connection test: create the fixture outside the tested transaction, write and Commit through A, observe through B, close A, then observe again. Add an uncommitted-write control and a write-after-Commit rollback case.',
 'Separate production transaction completion from nested error boundaries and test isolation. Production Commit must flush pending work, COMMIT synchronously, invalidate transaction-bound cursors, then recreate logical boundaries only when needed. Do not emulate durability by releasing savepoints.',
 'Represent isolation per session/table, with record overrides. Implement UpdLock with PostgreSQL row locks and bounded wait policy; name PostgreSQL READ UNCOMMITTED divergence. Check update/delete against the version originally read and report conflicts.',
 'Introduce a transaction-owned connection lease after correctness is gated. Handle rollback/destructor failures explicitly; a throwing Scope destructor during unwinding must not terminate the process.'],
'Two independent connections prove visibility, rollback, conflicts and lock release. Exercise Commit at depth zero and inside nested boundaries. Production durability gates must stay green when 0039 adds isolation. Compare the full UT population before/after activation.',
'Platform: methods-auto/database/database-commit-method.md, devenv-tri-state-locking.md, devenv-read-isolation.md. AL: posting codeunits and their Commit calls. Predecessor: search transaction/commit/rollback findings; 0718 records why globally ignoring Commit lost error logs.')
wi('0718','Record images and temporary handles will survive copies without dangling references','P0',
'`include/runtime/Table.h::CaptureImage` replaces the owned image; `RecordState.h::CopyStateFrom` swaps out the state that owns it. `xRec` is handed out as a reference. The previous WI recorded ASan use-after-free and a candidate stable-image fix, but that fix is absent from HEAD. Its historical -7 regression was traced to committed serial rows leaking between tests, not evidence that freed memory was correct.',
['Create a small fixture that retains xRec through Copy, Get, Modify and nested triggers. Run it under ASan/UBSan before changing ownership.',
 'Preserve the destination image owner when copying filter/cursor state. Update an existing image in place when live references can still name it; avoid recursively copying image state. Specify separately when temporary table storage is shared and when record fields are copied.',
 'Coordinate with 0039: test-isolation rollback must include explicit commits while preserving a commit across an inner Codeunit.Run rollback. Do not install CommitScope(Ignore) around every test; the historical full run lost 15 cases with that shortcut.',
 'Recheck Variant, RecordRef and Instance lifetime bridges against the same fixture. Variant already owns record snapshots; the old claim that it always stores a bare address is obsolete.'],
'ASan fixture fails before the ownership repair and passes after it. Compare the same complete SCM Available to Pick UT codeunit, Payment Export Validation UT and Price Worksheet Line UT before/after, then the whole milestone. Report crashes and method identities, not only aggregate wins.',
'Platform: devenv-system-defined-variables.md, methods-auto/record/record-copy-method.md. AL: TrackingSpecification.Table.al and SCMAvailabletoPickUT.Codeunit.al. Predecessor: WI-781, WI-1078, WI-1137, WI-1156; Variant findings WI-1095 and WI-1241.')
wi('0039','The AL runner will control test lifecycle and isolation explicitly','P0',
'`src/rt/TestRunner.cpp::RunOne` invokes methods directly. It keeps successful writes before checking unused handlers, so a failed handler check can retain writes. RunRegisteredTests always wraps a codeunit; no generated TestRunner drives before/after hooks. The CLI runner exists, so the old claim that there is no runner is obsolete.',
['Carry the selected TestRunner, TestIsolation and TransactionModel as explicit runtime metadata. Register OnBeforeTestRun/OnAfterTestRun hooks and drive the translated runner through the CLI, including the empty-function codeunit callbacks and skip result.',
 'Implement an isolation floor below nested error scopes, coordinated with 0012. None, Codeunit and Function are distinct policies. Successful methods may share state under Codeunit isolation; do not unconditionally roll back each successful method.',
 'Validate handler usage before choosing Keep/Discard. Ensure exceptions during installation, invocation, result collection and hook execution always uninstall handlers and release traps.',
 'Implement the PermissionTestHelper bookkeeping needed by the translated reset chain before activating it. Replace native reset duplicates only once the corresponding AL hooks are exercised.'],
'Gates cover explicit Commit followed by method failure, Commit surviving an inner rollback, successful cross-method state under Codeunit isolation, Function isolation, AutoRollback, unused handlers, false before-hook and throwing after-hook. Run the same full codeunits for A/B; per-method sampling is not an isolation proof.',
'Platform: properties/devenv-testisolation-property.md, attributes/devenv-transactionmodel-attribute.md, devenv-testrunner-codeunits.md and both TestRunner triggers. AL: Tools/Test Framework/Test Runner/src. Predecessor: WI-963, WI-1088 (164/190 versus 179/190 under different isolation), WI-1316.')
wi('0058','Every UT run will reconcile results with an independent source manifest','P0',
'`scripts/ut-milestone.sh` discovers codeunits from AL text but takes successful denominators from the runner output. A runner can omit methods and still shrink the total. The script accepts any printed total regardless of process status and ends with a successful printf even when tests fail. Historical denominators in the board disagree.',
['Create one source-text manifest of codeunit ID, name, source path and every Test method; exclude comments and strings without depending on the transpiler AST. Make builtin ranking and milestone aggregation consume it.',
 'Match every reported method to this manifest. Missing, duplicate, unexpected and crashed cases are explicit failures. Compare the reported total even for a process that printed a summary.',
 'Use collision-free scratch database/log identifiers per codeunit and run, trap cleanup, validate workers, retain timeout/crash logs, and return nonzero for any red/lost/incomplete case.',
 'Record BCApps revision, agiru revision, seed identity, work date and command alongside sorted per-method results. Add a make entry point for this script.'],
'Fixtures cover a lost parse, a missing linked method, duplicate method results, exit failure after a printed total, timeout, zero population and two names that sanitize to the same filename. Each must make the aggregator red without lowering the expected denominator.',
'Platform: devenv-test-codeunits-and-test-methods.md. AL: Layers/W1/Tests, Subtype=Test with names ending UT/-UT/.UT. Predecessor: WI-1088 warns that measurement shape itself changes results.')
wi('0589','The toolchain will be reproducible and every gate will fail reliably','P1',
'The review repairs test discovery, redundant builds, generator-check mutation and swallowed analyzer exits. See README for measured make/test/lint outcomes. Remaining architecture concerns: default builds compile the generated slice and all gates; CMake includes all tiers in every gate, and generated flags are reused for handwritten runtime targets.',
['Keep one serialized build per directory. Use the selected compiler consistently across Make, syntax sweeps and formatter helpers; keep Clang and GCC build directories separate.',
 'Add a GCC build gate through make, plus a standalone generated-source compile without PCH/unity. PCH and unity must not conceal missing includes or app dependencies.',
 'Finish the full clang-tidy/Doxygen audit without increasing baselines. Count exactly the selected handwritten translation units and preserve tool failures. Treat changed headers as their compiled consumers.',
 'Measure no-op build, one runtime source edit, one generator edit and one public-header edit. Optimize only against these measurements; batch header edits and preserve mtimes for byte-identical output.'],
'`make`, `make test`, `make lint`, a full lint run and the GCC gate must report actual outcomes. Negative controls: missing gate executable, failed analyzer without diagnostics, stale generated file and a changed header without a compiled consumer. Never hide failing C++ gates in a baseline.',
'Repository: Makefile, CMakeLists.txt, test/run.sh, test/lint.sh, test/lint-analysis.py, test/toolchain.py, scripts/gen_builtins.py. No AL semantic is changed by build orchestration.')
wi('0004','Provisioning will record source and seed provenance and produce usable test databases','P1',
'`BC_VERSION` is 28.4.53241.0. `scripts/provision.sh` claims it checks the source version but only runs three scripts and prints that transfer is not implemented. A seeded runner and import scripts already exist. Imported CRONUS schemas and the runner public schema are different layouts.',
['Record artefact version/checksum, BCApps commit and scope hash in database metadata. Compare schemas and publish unmapped columns explicitly; do not claim the artefact and current main are version-matched.',
 'Make provision orchestrate the actual download, restore, conversion, schema and seed steps with checked exit statuses. Keep imported data immutable and create disposable test clones.',
 'Remove early-return coupling between platform catalogue tables in ProvisionInstalled: an existing AllObj row must not prevent Field/Page Metadata from being repaired. Make each catalogue population idempotent.',
 'Derive the work date from documented seed policy and record it in run results. Keep test gate DSNs distinct from masters and refuse destructive operations on templates.'],
'Provision twice, compare catalogue/data counts and identity, then clone and run a representative AL codeunit. Interrupt a transfer and prove restart does not advertise a complete seed. Check empty and partially populated catalogue tables.',
'Repository: scripts/provision.sh, seed_demo.py, cronus_to_pg.py, pg_master.sh, src/rt/Storage.cpp::ProvisionInstalled, RunnerDatabase.cpp. AL: platform metadata consumers. Predecessor: WI-832 (master held open), WI-847 (demo working date).')
wi('0006','Session ownership and performance will have measured portable bounds','P2',
'`Session` uses a thread-local current pointer, while Events.cpp holds thread-local instance maps. Nested sessions restore the pointer but release thread-owned instances, so thread-local storage is not by itself session ownership. Catalogue registration uses startup vectors despite the intended immutable metadata design. No current aarch64 result was established by this review.',
['Move mutable subscriber instances, manual bindings, random state and page traps under explicit Session ownership. Test nested sessions and sequential sessions reusing one worker thread.',
 'Measure sizeof/session arena usage, marginal RSS/PSS per active session and shared-library relocation pages. Distinguish constexpr declarations from actual .rodata versus .data.rel.ro placement.',
 'Benchmark indexed Get, bounded FindSet and posting against equivalent SQL with AGIRU_FAST_LOOP=OFF. Record compiler, architecture, row count, concurrency and database settings.',
 'Only then optimize decimal division, allocation and code layout. Do not adopt a prescribed divider/layout merely because an old WI proposed it.'],
'Two concurrent and two nested sessions never see each other\'s mutable state. Publish x86_64 and aarch64 measurements with comparable workloads; no guessed portability claim. Metadata initialization and transaction-pool costs are counted separately.',
'Repository: src/rt/Session.cpp, Events.cpp, SingleInstance.cpp, Catalogue.cpp; include/runtime/Session.h; src/net/Decimal.cpp. Platform: session and SingleInstance contracts. Original WIs 0006, 0008, 0009, 0596 describe hypotheses, not measured completion.')
wi('0013','System fields and schema keys will obey their declared contracts','P1',
'System-field declarations and storage support exist; the old claim that none are generated is obsolete. `Storage.cpp` builds indexes, but its write predicates only identify keys. Audit schema guarantees separately from fields being present.',
['Audit SystemId uniqueness, supplied-ID Insert overloads, created/modified stamps and database-wide monotonic SystemRowVersion. Assign versions in PostgreSQL so independent tiers cannot collide.',
 'Verify key properties: Unique, MaintainSqlIndex, IncludedFields, Enabled, AutoIncrement and SqlTimestamp. Encode constraints in DDL; preserve explicit non-applicability of SQL Server clustering.',
 'Return platform-owned fields from writes and preserve identity on Rename. Coordinate version checks with 0012 and company-qualified sequences with its company work.',
 'Use typed generated metadata for constraints and migration comparisons; never infer a field number from display order.'],
'Two connections writing different tables receive distinct increasing versions. Duplicate supplied SystemId fails, Rename retains it, audit stamps follow documented trigger order, and disabled/nonmaintained keys generate the intended DDL.',
'Platform: devenv-table-system-fields.md, methods-auto/record/record-insert-boolean-boolean-method.md and key property pages. AL: table/key declarations. Predecessor: inspect SystemId/rowversion findings before changing write order.')
wi('0018','SQL and temporary records will evaluate the same filter grammar','P1',
'`src/rt/Filter.cpp` is an existing parser; `RecordState.cpp` separately parses views using parentheses counts. Its splitter does not track quoted field/filter text. Temporary and SQL paths apply filters through different evaluators, so a passing SQL case does not prove the temporary case.',
['Define one typed filter AST and quote-aware view grammar. Cover escaped quotes, commas/parentheses inside literals, OR/AND precedence, wildcard modifiers, blank values, open ranges and field types.',
 'Run the same fixtures through SQL and Temporary.cpp, including filter groups, SetRange replacement, CopyFilter, marks, GetRangeMin/Max and RecordId values.',
 'Separate UI date/token expansion from SetFilter parsing; resolve metadata filter tokens through declared platform hooks rather than guessing.',
 'Reject malformed input with a named diagnostic. Avoid repeatedly parsing immutable CalcFormula/TableRelation text on hot reads.'],
'Differential fixtures return the same ordered keys for SQL and temporary tables. Include quoted delimiter characters and closing dates; remove quoting or a filter-group term as negative controls. Preserve benchmark bounds for large filtered reads.',
'Platform: devenv-entering-criteria-in-filters.md and methods-auto/record filter overloads. AL: No. Series temporary lookups and declared CalcFormula filters. Predecessor: WI-870/889/1063/1136 and the filter findings referenced in the old 0044.')
wi('0019','FlowFields will preserve semantics and use bounded aggregate queries','P1',
'FieldClass and CalcFields support are already present. FlowField filtering and aggregation still need coherent SQL/temporary proofs; old claims that every FlowField is a stored column are stale. Stored properties alone do not prove SIFT behaviour.',
['Gate all seven CalcFormula operations, typed results, empty-set defaults, FlowFilter substitution and negated formulas. Compile declaration structure once into typed metadata.',
 'For filters on FlowFields emit a correlated subquery over the correct table/company and combine it with ordinary filters. Exercise nested parentheses and NULL aggregates.',
 'Verify CalcSums and selected key/filter behaviour from the current overload documentation; do not inherit the old title\'s blanket current-key restriction without checking it.',
 'Implement maintained aggregates only after measuring plain SQL. Honor MaintainSiftIndex and consistency under insert/modify/delete/rollback from two writers.'],
'SQL and temporary fixtures agree; FlowFields never become stored columns. A second connection and rollback test prove aggregate consistency. Explain EXPLAIN plans and lock duration before claiming SIFT performance.',
'Platform: devenv-flowfields.md, properties/devenv-calcformula-property.md, methods-auto/record/record-calcsums-method.md. AL: aggregate/filter users. Predecessor: WI-890 records activation regressions from FlowField filters.')
wi('0030','One page lifecycle will drive TestPage and the HTTP UI','P1',
'Page generation, navigation and TestPage exist in src/gen/PageWriter.cpp and src/rt/{TestPage,PageRecord,Navigate}. No test/ui directory or HTTP server is currently present. Metadata carriage does not implement visibility, editing or save behaviour.',
['Finish one explicit page state machine for open/new/edit/validate/row-leave/close. Respect source views, temporary sources, default Boolean trigger results, page events and table-before-control validation.',
 'Implement header save before part entry, SetRecords sharing, delayed insert, Update(SaveRecord) and subpage links against the same record state. Keep OnAfterGetRecord separate from OnAfterGetCurrRecord.',
 'Build the htmx HTTP renderer over ControlDef/PageDef with server-held session state. Cover cards, lists, documents, role centres/cues, lookup/drilldown, Tell Me, dialogs and page actions. Enforce permissions and editability on the server.',
 'Evaluate dynamic properties and inherited container restrictions once per relevant lifecycle event. Include captions/translations, views, actionrefs, customizations, FactBoxes, keyboard navigation and supported page kinds. Add API/control-add-in adapters after the shared lifecycle works.'],
'Every UT TestPage case runs a second time through actual HTTP and yields the same messages, rows and values. Add lifecycle-order traces, denied-edit requests, two independent sessions and header/part save controls. Activation requires a full UT A/B.',
'Platform: devenv-testing-pages.md, page/field/action triggers and page property pages. AL: page declarations plus UT TestPage users. Predecessor: WI-1169/1170, WI-1235 (part SetRecords), WI-1401 (Update), WI-1411 (discount recalculation).')
wi('0033','App boundaries and extension merges will be explicit and enforced','P1',
'Extensions are merged in src/tc/Main.cpp and apps.json describes dependencies. The slice target includes all app directories and links one library, so it does not prove full-app dependency direction. CMake uses private includes but does not by itself prevent public headers from exposing higher-tier types or shared libraries retaining unresolved references.',
['Validate the reaches/app dependency graphs, rejecting cycles and unknown edges. Configure dependencies in topological order and track reaches/apps.json as CMake inputs.',
 'Preserve declaring app identity on merged fields, procedures and metadata. Complete each extension/customization kind and deterministic anchor ordering; unresolved anchors must be diagnostic failures or counted refusals.',
 'Enforce namespace, Access/local, Extensible, obsolete declarations and preprocessor symbols at generation time. Refuse malformed or unsupported directives rather than silently selecting a branch.',
 'Prove normal app linking with undefined-symbol checks appropriate to declared dependencies. Keep slice fallback stubs explicitly out of the full-app correctness claim.'],
'Cross-app fixtures cover legal dependency, illegal reverse reference, late extension anchor, missing anchor and duplicate names. Compile without the all-app slice include path. Same inputs produce byte-identical merge order.',
'Platform: devenv-json-files.md, namespace/access/extension and obsolete/preprocessor documentation. AL: apps.json inputs and extension declarations. Predecessor: WI-990 defines scope boundaries; do not widen them accidentally.')
wi('0034','Every object kind will have a truthful translation and runtime census','P1',
'Tables, codeunits, enums, pages, interfaces, queries, reports, XMLports and profiles have implementation paths. `src/tc/Main.cpp::UntranslatedKinds` is a hardcoded filename-suffix list; it still lists reportext and can miss actual declaration spellings. Permissions, entitlements and control add-ins remain material gaps.',
['Count top-level object declarations independently from successful parsing; include every extension and namespace-less source selected by scope.json. Report parsed, emitted, compiled, linked and runnable as different stages.',
 'Give each kind a typed AST and explicit writer/runtime registration. Remove page-shaped encoding of unrelated kinds incrementally behind generator fixtures rather than spreading more Boolean mode flags.',
 'Wire permission kinds to 0062, reports to 0063, queries to 0064, XMLports to 0065 and control add-ins/profiles to 0030. Keep a refused kind visible until a representative runtime case passes.',
 'Replace filename-derived missing-kind guesses with parser dispatch accounting and validate the census against source declarations.'],
'One fixture per object/extension kind, including mixed-case unconventional filenames and a deliberately unsupported declaration. A lost parse cannot lower totals; a header-only stub cannot count as a runnable kind.',
'Platform: object declaration documentation. AL: apps.json and src/gen/scope.json populations. Predecessor: object writers are useful findings, not authority for the C++ representation.')
wi('0035','Rebuilt .NET types will preserve reference and culture semantics','P1',
'XML, JSON, regex, culture, stream and date types already have src/net implementations; the board\'s claims that these whole families are absent are stale. Absent-type generation and variadic refusals still allow compilation without behaviour.',
['Measure actual refusing calls by type and signature across the current UT run. Use that ranking to choose a family, then read its callers and predecessor findings before implementing it.',
 'Keep shared engine code behind AL and .NET-specific public contracts. Gate object identity/copying, disposal, out parameters, null, encoding and exception differences rather than aliasing similarly named types.',
 'Review std::regex compatibility and CultureInfo/TextInfo formatting/casing against the source usages. Add Unicode and culture fixtures that distinguish invariant, session and explicit-provider behaviour.',
 'Rebuild PermissionTestHelper bookkeeping for 0039 and event-capable DotNet variables with explicit subscription lifetimes. Report remaining unsupported signatures by name.'],
'Family-specific contract gates plus full codeunit A/B; include reference-alias mutation and disposed-object controls. Removing a real implementation must increase the refusal counter and fail the associated gate.',
'Repository: src/net, include/dotnet, generated absent types, src/gen/Names.cpp. AL: DotNet declarations and call sites. Predecessor: XML/JSON/culture and out-parameter findings; do not copy Python bridge semantics.')
wi('0038','The complete generated tree will compile and link without slice fallbacks','P1',
'The current build uses test/slice, unity/PCH and scripts/unlinked.py fallbacks. A green slice proves only the listed sources and can translate missing procedures into runtime refusals. Historical fixed blocker counts in the board are obsolete.',
['Run a fresh source/header census through make tree and rank current generic root diagnostics. Fix the highest-impact root in src/gen or public/runtime code, then recompile that root before changing the census.',
 'Add compiling sources to the slice without removing existing entries. Compare standalone and unity/PCH compilation to expose accidental include dependencies.',
 'Measure unresolved procedures by app and forbid fallback symbols in the complete-app build. Link every declared app dependency and verify load before running tests.',
 'Record source revision, scope hash, compiler and source counts with the census. Handle duplicate generated class names deterministically from namespace/app identity.'],
'`make apps` completes under both compilers; a missing implementation makes the full build red. Slice fallbacks remain loud and counted during transition. A source deliberately removed from compilation must be detected by coverage checks.',
'Repository: CMakeLists.txt, scripts/tree_syntax.sh, first_gap.sh, unlinked.py, test/slice, src/gen. AL: complete selected source population. Historical 36-root/compile-count claims are intentionally removed.')
wi('0043','Validation and relations will run in the documented order','P1',
'`src/rt/Relation.cpp` now parses conditional relation text and runtime validation exists. The old no-op description is obsolete. Missing related tables and trigger sequencing still need end-to-end fixtures; parsing metadata is not proof that Validate enforces it.',
['Gate default validation, table field OnValidate, page control OnValidate and platform/extension events in documented order. Include failure restoration and xRec visibility.',
 'Represent relation targets, branches and filters as typed generated metadata; unknown installed targets must refuse rather than silently accept a value.',
 'Implement property-specific enforcement at its documented boundary. UI-only NotBlank/MinValue rules must not be copied blindly onto all assignments or Record.Validate calls.',
 'Verify ValidateTableRelation=false, lookup overrides, fieldgroups and Rename propagation with real related tables. Batch metadata/header changes after the schema is agreed.'],
'A two-table fixture proves missing-parent refusal, conditional branches, filtered relation, opted-out validation, extension order and cascade rollback. A trace proves the field trigger never runs after rejected default validation.',
'Platform: properties/devenv-tablerelation-property.md, devenv-validatetablerelation-property.md, each validation property, table/page OnValidate triggers. AL: TableRelation declarations. Predecessor: validation-order findings; WI-781/1078/1137/1156 for xRec.')
wi('0044','Record operations will share one correct SQL and temporary contract','P1',
'Find/Get/Init/Copy/DeleteAll and temporary storage are implemented; old blanket refusal inventories are obsolete. `Table.h` and RecordRef expose parallel paths that can diverge in discarded-return semantics, filter state and ownership.',
['Write a small operation matrix over typed Record, RecordRef and temporary records: Init versus Clear, assignment versus Copy, Copy(ShareTable), Get versus filters, Find directions, marks and ModifyAll/DeleteAll triggers.',
 'Centralize primitives below the typed wrappers while retaining typed field access. Preserve table variables and temporary ownership according to operation, not a general C++ copy rule.',
 'Implement documented Boolean result versus statement-raises behaviour consistently; only recognized not-found/duplicate conditions may become false. Preserve database faults.',
 'Complete computed platform tables from system symbols and requested ranges, avoiding fixed-date population as the authoritative implementation.'],
'The same fixture yields identical keys, field values, filters and events through all three paths. Include no-match, duplicate, negative Next, absent key and malformed typed key controls. Handle lifetime tests belong to 0718.',
'Platform: methods-auto/record and recordref overloads, devenv-temporary-tables.md and virtual-table pages. AL: No. Series temporary filters and platform table users. Predecessor: WI-1063/1136/1173/1206/1229; retain source usage as a fixture, never a hardcoded runtime branch.')
wi('0045','Reads will remain bounded and partial records will be real','P2',
'`Cursor.cpp` already FETCHes blocks, so the old whole-result-set claim is obsolete. `Navigate.cpp` still selects Columns(table), and Table.h accepts SetLoadFields/LoadFields without implementing projection. Cursor validity is approximated by rollback count and boundary depth.',
['Preserve bounded cursor reads while making transaction generations and cursor invalidation explicit. Reopen from a stable key when AL requires continued navigation after writes/commit.',
 'Implement selected-field state, initial projection and JIT loading with observed-rowversion checks. Public typed member reads need a deliberate generated access mechanism; do not pretend recording field numbers alone implements JIT.',
 'Load BLOB payloads only through the documented CalcFields/load path. Keep media identifiers cheap; storage work is 0074.',
 'Verify each maintained key index and ORDER BY prefix/tiebreak against actual plans. Benchmark with realistic cardinality and concurrent sessions, not the small gate fixture.'],
'Peak memory remains proportional to fetch block and selected field widths as row count grows. Unloaded BLOBs are not transferred. JIT detects modified/deleted/renamed rows, and post-rollback cursor cleanup never aborts a later transaction.',
'Platform: devenv-partial-records.md, its FAQ, record-findset-boolean-method.md, key properties and BLOB contract. Repository: Cursor.cpp, Navigate.cpp, Selection.cpp. Predecessor: consult cursor/partial-record findings before selecting resume semantics.')
wi('0055','Errors and labels will keep BC text, codes and navigation context','P1',
'Error codes and RecordErrorGate exist; old claims that GetLastErrorCode is missing are stale. Several public contracts retain refusal descriptions alongside implemented bodies. ErrorInfo collection, translated labels and UI actions need distinct behaviour proofs.',
['Map PostgreSQL failure classes to typed runtime errors without exposing a different error for the same AL operation. Preserve inner codes when table/page validation adds context.',
 'Parse label attributes and carry translation metadata; format diagnostic parameters using AL Format rather than C++ approximations.',
 'Implement collected-error continuation, clearing and actionable navigation without adding an implicit transaction rollback. Store record/control/action context as typed metadata.',
 'Audit handler kinds and dialog wording through both TestPage and HTTP. Diagnose the specific error-log/drilldown path under 0719 after 0718 and 0061.'],
'Exact-message/code fixtures cover FieldError/TestField forms, duplicate/not-found, nested validation, collected errors and ClearLastError. UI error actions address the intended record after a failed transaction.',
'Platform: devenv-error-collection.md, devenv-actionable-errors.md, label properties and methods-auto/errorinfo. AL: Assert.ExpectedError/ExpectedErrorCode consumers. Predecessor: WI-1141 for last-error replacement.')
wi('0057','Events will preserve lifetime, permissions and isolated transaction semantics','P1',
'Publisher/subscriber registration and dispatch already exist in src/rt/Events.cpp. The old assertion that events all fire into empty bodies is false. Manual/automatic subscriber state is thread-local, and isolated execution depends on the currently incomplete transaction layer.',
['Build a dispatch matrix for integration/business/internal events and platform table/page events. Preserve var writeback, IncludeSender, GlobalVarAccess, subscriber instance mode and declared subscription filters.',
 'Make bindings session-owned and remove them on every instance destruction/error path. Keep deterministic dispatch order as an explicit agiru choice without making AL business logic depend on it.',
 'Enforce skip-on-missing-permission/license and internal app visibility with 0062/0033. An event with no subscribers is legal and must not raise simply for being unobserved.',
 'Implement isolated subscriber transactions on top of 0012, respecting an already active write transaction and documented commit/error rules.'],
'Fixtures distinguish manual from automatic instances, object lifetime, nested sessions, var outputs, permission skip versus error, empty publisher, subscriber failure and isolated commit. A two-connection test proves isolation durability.',
'Platform: devenv-eventsubscriber-attribute.md, devenv-integrationevent-attribute.md, devenv-eventsubscriberinstance-property.md, devenv-events-isolated.md. AL: event declarations and system triggers. Predecessor: WI-1036/1127/1145/1169/1216.')
wi('0059','Surface coverage will distinguish declarations, refusals and tested behaviour','P1',
'Existing surface and trigger counters mostly detect names/signatures. A variadic method that always refuses or ignores arguments can count as present; metadata strings can satisfy text searches without any dispatcher calling them. Baseline files are not all zero, contrary to the old instructions.',
['Discover all documented types and overload files, including reportinstance/queryinstance/xmlportinstance forms, and compare per-type signatures with the public declarations.',
 'Track declared, refusing, implemented and contract-tested as separate states. Map each runtime refusal to its owning consolidated WI; do not call a name search a semantic coverage test.',
 'For properties, verify a metadata consumer or explicit diagnostic as well as emission. For triggers, exercise registration and actual lifecycle dispatch.',
 'Remove stale Doxygen refusal blocks when implementing a method. Count the actual analyzed source population and never lower findings by excluding newly problematic code.'],
'Negative fixtures remove a real overload, replace a body with RefuseDoor, disconnect a trigger and drop a property consumer. Each changes the corresponding counter or gate. Missing docs or analyzer outputs fail the audit.',
'Repository: scripts/al_surface.py, dropped_properties.py, test/triggers.py, doc/al-surface.json, public headers and baselines. Platform: methods-auto, properties and triggers-auto inventories. The former ledger is historical reading activity, not coverage proof.')
wi('0061','Consumed and discarded calls will keep AL error semantics','P1',
'BodyWriter.cpp has IsTriedCall/Tried support, restricted to bare or simple member shapes. The old statement that TryFunction never appears in the generator is obsolete. Chained receivers and table-local resolution remain paths to verify.',
['Represent value consumption explicitly through assignments, conditions, arguments, exit, case selectors and nested expressions; do not infer it from an isolated token spelling.',
 'Resolve TryFunction attributes from the callee symbol/type, including table-local and chained calls. Consumed calls catch and set last error; discarded calls propagate normally.',
 'Do not insert a savepoint around TryFunction: documented write behaviour differs from Codeunit.Run and asserterror. Keep the three mechanisms separate.',
 'Extend the same consumption model to Boolean Record/File/XML operations, preserving only their documented failure-to-false cases.'],
'Generate and execute the same throwing function in every consumption context. Include a write before error that survives a consumed TryFunction, a discarded call that raises and last-error replacement. Run Incoming Doc. To Data Exch.UT as an activation A/B.',
'Platform: devenv-handling-errors-using-try-methods.md and attributes/devenv-tryfunction-attribute.md. AL: incoming-document conversion and table-local TryFunctions. Predecessor: WI-1141 and prior value-context findings; obsolete rollback advice from 0226 is rejected.')
wi('0062','Authorization will be enforced at data and object boundaries','P1',
'Table.h currently describes every session as SUPER until a permission system exists. PermissionSet/Entitlement kinds are not fully generated. IsolatedStorage.cpp omits extension identity from its primary key and SetEncrypted stores the supplied value as plaintext with a Boolean flag.',
['Generate permission sets/extensions, composition/exclusion and entitlement metadata with declaring app identity. Build effective permissions per user/company/session, including indirect and scoped inherent grants.',
 'Check reads, writes, executable objects and related-table/FlowField access at runtime boundaries; UI hiding is additional presentation, never authorization. Implement SecurityFiltering modes explicitly.',
 'Key isolated storage by extension plus documented DataScope dimensions. Replace plaintext SetEncrypted with a justified encryption/key-management mechanism, or refuse it explicitly until supported.',
 'Carry TestPermissions and temporary-record exceptions according to documentation, coordinated with 0039. Implement HTTP/API identity before multi-user exposure.'],
'Two users, two companies and two extensions prove denial and isolation. Include forged HTTP requests, denied indirect writes, scoped elevation unwinding and encrypted-value-at-rest inspection. No test may pass merely because every user is SUPER.',
'Platform: security documentation, permissions/inherent permissions/TestPermissions properties, isolatedstorage method overloads. AL: LibraryLowerPermissions and permission set declarations. Predecessor: security findings identify traps; its subset success is not authorization proof.')
wi('0063','Reports will execute datasets and render declared layouts','P2',
'Report datasets and request pages have generator/runtime paths in PageWriter.cpp and Report.h/.cpp. SaveAsPdf/Print/Preview still require a renderer. Two old files used ID 0063; they are consolidated here, retaining the implemented dataset work.',
['Separate report dataset/lifecycle metadata from page request controls without duplicating the page engine. Verify nested dataitem ordering, link/view filters, column expressions, temporary items, Skip/Break/Quit and all report/extension triggers.',
 'Resolve request filters per dataitem/table identity; preserve page-owned members over control-name collisions. Wire report substitution and declared platform events through normal event dispatch.',
 'Read rendering and legacy layout declarations with explicit precedence, language/format region, limits and timeouts. Translate supported RDL layouts to XSL-FO and render through Apache FOP.',
 'Implement request-page handlers, preview lifecycle, streams/files and scheduling using the same dataset pipeline. Unsupported layout features must refuse with names/counts.'],
'Golden dataset and lifecycle fixtures precede PDF comparison. Render a representative invoice and compare rows/totals/captions; preview must follow its documented second-run behaviour. Prove output failure rolls back only the intended boundary.',
'Platform: devenv-report-object.md, report/dataitem triggers, rendering/layout properties and reportinstance overloads. AL: report declarations and extensions. Predecessor: WI-1082 (request-page name precedence) and report view/layout findings.')
wi('0064','Queries will stream correct joins, filters and typed aggregates','P2',
'QueryWriter.cpp and src/rt/Query.cpp exist and QueryOpen uses Cursor. The old no-parser/no-writer claim is obsolete. Build constructs SELECT/JOIN/GROUP/HAVING and deserves semantic fixtures independent of generated metadata.',
['Gate all join kinds and nesting order. Child DataItemTableFilter belongs in ON for an outer join; root filters and column/HAVING filters have different placement.',
 'Verify filter overwrite versus conjunction rules, grouped nonaggregate columns, integer Average, NULL defaults, ReverseSign, ordering and TopNumberOfRows.',
 'Honor read isolation through 0012 and security/company context through 0062. Preserve bounded streaming for exports and propagate stream errors.',
 'Add query API publication only after runtime Open/Read/Close semantics are complete; metadata reachability is not an endpoint.'],
'Fixture includes unmatched parent rows, filtered child rows, NULL aggregate inputs and runtime filter replacement. Compare generated query output to hand-written SQL and assert fetch memory stays bounded.',
'Platform: queryinstance overloads; SqlJoinType, Method, ColumnFilter, DataItemTableFilter and ReadState properties. AL: query declarations. Existing QueryGate outer-join regression is the starting point, not a replacement target.')
wi('0065','XMLports will preserve schema, encoding and import transaction semantics','P2',
'XMLport generation and src/rt/XmlPort.cpp exist. XmlPortInput materializes parsed input; ReadWhole accumulates every stream block. The old claim that import/export is entirely absent is false, but large imports are not bounded.',
['Gate XML and text formats independently: namespaces, attributes, min/max occurrences, unbound loops, separators, fixed widths, encodings and direction-specific triggers.',
 'Use a pull parser/streaming writer for large datasets; retain only current nesting state and bounded text fields. Preserve namespace identity rather than stripping prefixes and comparing local names indiscriminately.',
 'Implement AutoSave/AutoUpdate/AutoReplace and FieldValidate/default validation through existing Record primitives. Failure must respect the import boundary and explicit Commit policy.',
 'Share request-page lifecycle with 0030/0063 while preserving XMLport-specific controls and handler behaviour.'],
'Round trips include namespace collisions, quoted separators, empty fields, UTF-8/UTF-16 and malformed input after earlier writes. A large generated stream demonstrates bounded memory; a failure proves correct rollback.',
'Platform: XMLport object/schema documentation, XMLport/element triggers and format/import properties. AL: data-exchange XMLports. Predecessor: inspect encoding/namespace and import-validation findings before changing the parser.')
wi('0066','Formatting and text operations will follow AL culture and character rules','P1',
'Format, Text, DateFormula and CultureInfo implementations exist; old all-ASCII/all-refusing descriptions are stale. Text positions, code-unit length, locale selection, option captions and diagnostic formatting require distinct fixtures.',
['Make Format/Evaluate consume a typed format grammar with standard formats and explicit-provider/session precedence. Keep invariant storage formatting separate from user output.',
 'Define AL character/length/index semantics at the UTF boundary. Audit Code uppercase, Text slicing/search, Char formatting and case-insensitive comparison with non-ASCII examples.',
 'Use declared option/enum captions and label translations; never replace member ordinals with translated names. Apply page/report format overrides over table defaults.',
 'Gate DateFormula grammar rejection, calendar/closing-date arithmetic and UTC DateTime versus session display timezone. Optimize Decimal division only after correctness and measured cost.'],
'Documentation-derived tables cover decimal scale/rounding, at least two regions, format 9 round trips, non-ASCII case/positions, invalid formulas and date boundaries. Exact BC error-message comparisons use this same formatting path.',
'Platform: Format/Evaluate overloads, devenv-format-property.md, Text/Code/Char methods, DateFormula and DateTime contracts. AL: TypeHelper and expected-error tests. Predecessor: culture and format findings, not Python locale behaviour.')
wi('0070','Install and upgrade will execute declared lifecycle transactions','P2',
'Install/Upgrade subtypes can parse as codeunits, but the CLI and runtime lack a complete application lifecycle driver. ProvisionInstalled writes platform catalogue rows directly and is not an AL install/upgrade implementation.',
['Persist installed app/version/data-version state and company identity. Drive install and upgrade hooks in documented database/company phases, with deterministic ordering where BC leaves order unspecified.',
 'Run preconditions before writes and validation after migration; rollback the correct phase on failure. Expose NavApp module metadata with declaring-app identity.',
 'Implement DataTransfer set operations only in allowed upgrade context. Preserve obsolete/moved-field storage and schema migrations instead of dropping columns from current source visibility.',
 'Support app-owned packaged files under an installation root with path validation and app access checks.'],
'Install, reinstall and upgrade fixtures cover multiple companies, failing preconditions, validation rollback, schema/data version changes and cross-app file denial. Restart after interruption must not report a completed upgrade.',
'Platform: install/upgrade codeunits, DataTransfer and NavApp methods, moved/obsolete properties. AL: Install/Upgrade subtypes. Predecessor: lifecycle findings guide failure cases; no Python startup apparatus is needed.')
wi('0073','Generated expressions will preserve AL types and evaluation effects','P1',
'BodyWriter.cpp\'s operator table maps and/or to &&/|| and / and div to /. Verify the complete Binary emission before changing it: operator spelling alone does not establish which typed conversion/helper is used. Names and array/interface bridges have grown across multiple scope implementations.',
['Add execution fixtures for integer division yielding Decimal, decimal DIV/MOD, eager Boolean operands with var effects, overflow and operand order. Derive expectations from AL documentation and source usage.',
 'Centralize typed expression lowering before printing C++. Avoid global spelling sets deciding whether an unrelated field is a method. Resolve by receiver type, namespace and declaring scope.',
 'Gate arrays with all dimensions, one-based bounds and var views; enum compatibility only when declared; interface inheritance, implementation selection and assignment ownership.',
 'Use generated static_asserts for known limits and type relations. Unsupported conversions must fail at generation/compile time rather than fall into permissive Variant conversions.'],
'Include 7/2=3.5, a right Boolean operand that changes a var despite a decisive left operand, multidimensional ArrayLen, incompatible enums and a returned interface call. Inspect emitted code and execute it; header compilation alone is insufficient.',
'Platform: devenv-al-operators.md, type conversion tables, array methods, interface/enum properties. AL: Round(1/4*100,1) and Evaluate in compound conditions. Predecessor: WI-1057 proves eager effects; array and var-parameter findings identify copying traps.')
wi('0074','Streams and media will have explicit ownership and persistent storage','P2',
'Stream engines exist, but Media.cpp still refuses imports/exports and MediaSet operations. Media identifiers therefore do not imply a working media store. Stream materialization and shared ownership need bounded-resource proofs.',
['Specify InStream/OutStream attachment, shared backing data, position, encoding and disposal. Keep BLOB lazy-load state distinct from stream ownership.',
 'Implement tenant media/media-set storage with ordered membership, transactional references, import/export and orphan cleanup. Resolve identifiers through the database across service tiers.',
 'Enforce app/company/user scope where documented; reject invalid/missing media IDs loudly. Keep external files and HTTP streams behind explicit size/timeout/error handling.',
 'Expose actual HTTP request/response errors at both transport and status-code layers; integrate HttpClientHandler and per-test registration before enabling external fallthrough.'],
'Binary and encoded round trips, unattached/disposed streams, alias positions, ordered media sets and rollback/orphan cases. Another session/process retrieves imported bytes; large streams remain bounded.',
'Platform: stream/BLOB/Media/MediaSet and HTTP methods. AL: media table fields and data-exchange consumers. Predecessor: stream position, encoding and two-level HTTP failure findings.')
wi('0090','Background work will have database-backed ownership and session isolation','P2',
'The current Session opens a connection and thread-local state; there is no complete task/session service. Scheduled reports and page background tasks cannot be implemented by spawning a thread over the caller\'s mutable Session.',
['Implement child sessions with explicit immutable launch context and separate transactions/connection leases. Page background tasks are read-only and must reject locks/writes.',
 'Persist scheduled task state, claims, retries and failure-codeunit transitions in PostgreSQL. Use leases/transactional claims so two service tiers cannot execute the same claimed work concurrently.',
 'Respect cancellation, page lifetime, timeout and completion/error callbacks through the page lifecycle. Marshal only supported parameter/result types.',
 'Declare at-least-once retry behaviour and deterministic result ordering; do not claim exactly-once execution across process failure.'],
'Two workers/tiers race for a task, one crashes, another recovers the lease. Read-only task writes fail, cancellation cannot update a closed page, and failed work rolls back before retry.',
'Platform: devenv-page-background-tasks.md, Session/TaskScheduler methods and scheduled-task lifecycle. AL: task consumers. Predecessor: consult retry findings, not Python thread/context infrastructure.')
wi('0719','Error-message drilldown will retrieve the logged record context','P1',
'The previous trace plan identifies Incoming Doc. To Data Exch.UT: the conversion failure should be caught, logged under a RecordId context, then exposed by a trapped Error Messages page. It remains a hypothesis until reproduced on the repaired image/runner; no current result was measured in this review.',
['After 0718 and 0061, run the complete codeunit on a fresh seed and isolate TestProcessWithDataExchSucceeds while preserving its initialization.',
 'Trace the error insert and the drilldown select, including RecordId serialization and context filters. Determine whether logging never occurred, the filter differs, or page trapping failed.',
 'Fix the generic primitive identified by that trace. Do not special-case the table, test, page name or expected message.',
 'Add a reduced error-context round trip and trapped DrillDown fixture, then include it in the HTTP parity suite.'],
'Logged RecordId context survives write/read/filter and the intended trapped page opens with the same message. An intentionally different context returns no rows; SQL tracing is removed or remains a generic opt-in facility.',
'AL: IncomingDocToDataExchUT.Codeunit.al, ErrorMessage.Table.al SetContext/SetContextFilter/ShowErrorMessages. Platform: RecordId and TestPage DrillDown methods. Predecessor: search logged-error/context/filter findings after the trace identifies the primitive.')
# Existing IDs survive; detailed historical transcripts remain in Git.
alias={
'0005':'0589','0007':'0066','0008':'0006','0009':'0006','0010':'0066','0015':'0033','0016':'0066','0017':'0045',
'0025':'0044','0027':'0073','0028':'0059','0029':'0043','0031':'0074','0032':'0044','0037':'0718','0040':'0059','0041':'0066','0042':'0718',
'0046':'0589','0047':'0019','0048':'0045','0049':'0073','0050':'0589','0051':'0073','0053':'0066','0056':'0044','0060':'0012','0068':'0043','0069':'0033','0071':'0059','0075':'0066','0076':'0073','0077':'0012','0078':'0035','0079':'0012','0080':'0013','0081':'0073','0082':'0066','0083':'0030','0084':'0073','0085':'0073','0086':'0073','0087':'0012','0088':'0073','0089':'0073',
'0580':'0038','0581':'0073','0584':'0073','0585':'0035','0587':'0073','0588':'0059','0590':'0073','0591':'0038','0593':'0059','0594':'0059','0595':'0038','0596':'0006','0597':'0073','0598':'0038','0599':'0038',
'0604':'0034','0607':'0044','0608':'0073','0610':'0059','0612':'0038','0613':'0004','0614':'0073','0616':'0589','0618':'0035','0620':'0044','0621':'0012','0623':'0034','0624':'0718','0628':'0063','0629':'0073','0630':'0039','0632':'0066','0633':'0073','0640':'0718','0646':'0035','0657':'0030','0659':'0044','0660':'0045','0665':'0030','0671':'0062','0675':'0035','0677':'0043','0678':'0019','0695':'0073','0696':'0030','0697':'0044','0700':'0030','0702':'0038','0703':'0030','0706':'0057','0714':'0035','0715':'0039','0716':'0066',
'0014':'0718','0026':'0073','0054':'0030','0067':'0059','0190':'0033','0334':'0030','0331':'0043','0343':'0019','0381':'0062','0452':'0063','0482':'0030','0553':'0030','0539':'0030','0200':'0074','0364':'0044'}
by_id={}
for name,t in original.items():
 if name[:4].isdigit(): by_id.setdefault(name[:4],t)
def owner(id,seen=()):
 if id in items: return id
 if id in seen: return '0059'
 if id in alias: return owner(alias[id],(*seen,id))
 t=by_id.get(id,'')
 parent=re.search(r'^Parent:\s*(\d+)',t,re.M)
 if parent: return owner(parent[1].zfill(4),(*seen,id))
 title=next((l.lower() for l in t.splitlines() if l.startswith('# ')),t[:200].lower())
 for words,target in [(['permission','entitlement','security'],'0062'),(['xmlport'],'0065'),(['report'],'0063'),(['query'],'0064'),(['event','subscriber'],'0057'),(['page','control','action','profile'],'0030'),(['filter'],'0018'),(['key','index','system field'],'0013'),(['trigger','validate','relation'],'0043'),(['format','caption','label','text','date'],'0066'),(['dotnet'],'0035')]:
  if any(w in title for w in words): return target
 return '0059'
groups={id:[] for id in items}
for id in by_id: groups[owner(id)].append(id)
Path('build/review-board-map.json').write_text(json.dumps(groups,indent=2))
for p in root.glob('*.md'): p.unlink()
for id,item in items.items():
 slug=re.sub('[^a-z0-9]+','_',item['title'].lower()).strip('_')
 p=root/f'{id}_{slug}.md'; item['path']=p.name
 text=f"# {id} — {item['title']}\n\nStatus: open | Priority: {item['priority']} | Reviewed: 2026-09-22\n\n## Current evidence\n\n{item['evidence']}\n\n## Implementation for Sol\n\n"
 text+='\n'.join(f'{i}. {s}' for i,s in enumerate(item['steps'],1))
 text+=f"\n\n## Acceptance\n\n{item['proof']}\n\n## References\n\n{item['refs']}\n"
 properties=set()
 for old in groups[id]:
  properties.update(re.findall(r'devenv-([a-z0-9-]+)-property\.md',by_id[old]))
 if properties:
  text+='\nConsolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): '+', '.join(f'`{prop}`' for prop in sorted(properties))+'.\n'
 p.write_text(text)
readme='''# Open work after the 2026-09-22 review

This board contains implementation work, not session transcripts. The review started at
commit `136a5a7`. Source paths in WIs are repository-relative; platform references are under
`~/Git/dynamics365smb-devitpro-pb/dev-itpro/developer/`. Historical measurements are identified
as historical and must not be used as the current pass count.

## Execution order for Sol

1. Establish reliable measurements (0058, 0589), then reproduce the memory and transaction
   defects (0718, 0012). Keep production durability and test isolation distinct.
2. Repair the runner's isolation/lifecycle (0039); activate fixes only with a per-method A/B
   over the same AL source manifest and seed. Then investigate the remaining UT failures
   through their generic primitive, starting with 0061/0719 and record/page paths.
3. Continue complete-tree compilation (0038), close semantic gaps and deliver HTTP/TestPage
   parity (0030). Permissions and company/session isolation precede multi-user exposure.
4. Complete reports, XMLports, queries, installation, background work and the whole AL suite.
   Later priority means ordering, not exclusion from the ERP target.

## Findings that change implementation decisions

- Production `Commit` currently renews savepoints without a database COMMIT. Same-session
  tests are insufficient evidence of durability (0012).
- The recorded xRec use-after-free is still present; a prior apparent regression exposed
  test contamination. Keeping undefined behaviour is not a correctness strategy (0718).
- TestRunner callbacks are bypassed, and an unused handler is detected after keeping writes
  (0039). Historic green totals do not establish the intended isolation policy.
- Lock/isolation/load-field declarations can accept arguments without enforcing their
  behaviour. Separate signature coverage from implementation and contract tests (0059).
- Session lifetime is not equivalent to thread lifetime; thread-local caches must belong to
  the current session. A shared slice also does not prove app dependency isolation (0006/0033).
- IsolatedStorage omits extension identity, and SetEncrypted stores plaintext (0062).
- Report/query/XMLport implementations now exist. Their original “no generator” descriptions
  and the duplicate 0063 file were obsolete. Rendering and complete semantics remain open.

## Verification

Results are filled from the review runs below; no historical gate failure is accepted as a new baseline.

REVIEW_RESULTS_PENDING

## Open items and consolidation

The old board had 481 numbered files (480 distinct IDs) plus a 3,908-line reading ledger,
44,094 lines in total. The 598-line project instructions mixed goals, historical measurements
and contradictory environment claims. They are replaced by concise standing rules.

The rows below are a migration map, not a claim that all absorbed requirements were already
implemented. Duplicate property/trigger descriptions now live under their runtime outcome;
completed mechanisms are described as existing evidence, and unproved remainder stays open.
The reading ledger and speculative C++ reflection task are removed; Git retains the old text.
Use `git show 136a5a7:board/<old-name>` or `git log --all -- board/` for the recorded experiments.
No IDs have been reissued and no stale active ownership marks survive.

| WI | Priority | Outcome | Absorbed IDs |
|---|---|---|---|
'''
for id,item in sorted(items.items()):
 absorbed=', '.join(x for x in sorted(groups[id]) if x!=id) or '—'
 readme+=f"| [{id}]({item['path']}) | {item['priority']} | {item['title']} | {absorbed} |\n"
(root/'README.md').write_text(readme)
print(len(items),'open items;',sum(len(p.read_text().splitlines()) for p in root.glob('*.md')),'board lines')
