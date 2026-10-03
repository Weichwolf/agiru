# Tests

| Directory | Purpose |
| --- | --- |
| `gate/` | Focused C++ regression tests for compiler and runtime primitives. |
| `transpiler/` | AL input fixtures, source binding, generated-code compilation and native declaration contracts. |
| `runtime/` | Runtime integration fixtures: isolation, test contexts, reflection and number sequences. |
| `reporting/` | Report declarations, layout asset integrity and registry/linking fixtures. Not full rendering coverage. |
| `tooling/` | Build, lint, snapshot and test-runner checks; their Python helpers and existing diagnostic baselines. |

`transpiler/golden/` contains authored C++ output specifications. Do not regenerate
them from observed compiler output. AL fixtures and original platform declarations
belong under `transpiler/`; retain their notices.

`run.sh` runs the local regression population. `slice` is the ordered generated C++
integration slice, not the AL test denominator.

Use `make gate GATE=RecordRefGate JOBS=2` for a focused C++ regression,
`make test JOBS=2` for all local checks, and `make tc JOBS=2` after generator changes.
`make verify-check` checks build tooling without rebuilding C++.

The ERP milestones are separate: `make ut` executes the source-counted AL UT
population through `agiru run-tests`; the full AL suite follows. Their original
source is in BCApps, not in these authored fixtures. A green local regression
run does not establish either ERP milestone.
