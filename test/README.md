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

The ERP milestones are separate: `make ut` executes the source-counted AL UT
population through `agiru run-tests`; the full AL suite follows. Their original
source is in BCApps, not in these authored fixtures. A green local regression
run does not establish either ERP milestone.
