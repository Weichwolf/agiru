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
an explicit verified `AGIRU_SYSTEM_SYMBOLS` also compiles and calls all nine original
Base64 overloads, retaining named refusals. Neither proves implemented native
behaviour or authenticates authored fixtures as System packages.

`runtime/xml-reader.sh` proves shared cursor/close and consuming DOM-load contracts
in `XmlReaderGate`; separate-state and raw-input reload mutants must fail. It covers
positioned/ended readers, node ownership, namespaces, DTD retention and whitespace.
DTD/resolver security, encoding and streaming bounds remain open (0035).

`runtime/codeunit-record.sh` executes generated typed/static/dynamic `Codeunit.Run`
forms. Table-global saves survive SQL rollback through scoped var-Record identity;
ordinary assignment remains independent. Nested runs, callee restoration, handles,
temporary cursors and compiled no-borrow/no-restore/assignment-alias controls are covered.

`runtime/page-navigation.sh` executes generated list/card system Edit routing,
selected-record identity, opening triggers, explicit-action precedence and card
ModifyAllowed policy. View navigation and general command permissions remain open (0030).

`runtime/reflection-metadata.sh` verifies declaration projection and shared compiled
record filters, including group intersections, cross-column OR, FlowFilters and
owned expression snapshots. Sixteen controls must reject. Temporary operations
use the same predicate; this does not activate live metadata providers (0044).

The ERP milestones are separate: `make ut` executes the source-counted AL UT
population through `agiru run-tests`; the full AL suite follows. Their original
source is in BCApps, not in these authored fixtures. A green local regression
run does not establish either ERP milestone.
