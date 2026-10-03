# 0059 — Surface coverage will distinguish declarations, refusals and tested behaviour

Status: open | Priority: P1 | Stage: UT refusal accounting; All semantic coverage | Reviewed: 2026-09-30
Depends on: 0034 object census; 0058 method results.

## Evidence

- Name/signature and metadata-string counters can count refusing/no-op methods as present. Existing suppression baselines are not semantic coverage.
- Integrated census repair counts observed acted-on kinds directly, not catalogue sizes. Six actual-transpiler controls cover empty, acknowledged/acted repetition, mixed and unknown populations: Clang/GCC green; frozen old binary six red. Own/main local suites 85 cases, final toolchain 65 tests green. `make test` builds the transpiler and carries B into its harness; missing binaries cannot skip census checks. Page fixture: zero changed files, truthful 0/0 count. No policy/suppression change. Changed-code lint: 174/185 units, 167 failed; existing diagnostics remain. Main `build/attribute-census-main-tests.log`, own `build/attribute-census-*`; next frozen proof pending native Field closure (0034).

## Implementation

1. Attach each overload/property/trigger to its consumer, focused proof and owning WI. A generic forwarding/variadic body cannot certify distinct contracts.
2. Discover all documented types and overload files, including reportinstance/queryinstance/xmlportinstance forms, and compare per-type signatures with the public declarations.
3. Track declared, refusing, implemented and contract-tested as separate states. Map each runtime refusal to its owning consolidated WI; do not call a name search a semantic coverage test.
4. For properties, verify a metadata consumer or explicit diagnostic as well as emission. For triggers, exercise registration and actual lifecycle dispatch.
5. Remove stale Doxygen refusal blocks when implementing a method. Count the actual analyzed source population and never lower findings by excluding newly problematic code.

## Acceptance

- Negative fixtures remove a real overload, replace a body with RefuseUnimplemented, disconnect a trigger and drop a property consumer. Each changes the corresponding counter or gate. Missing docs or analyzer outputs fail the audit.

## References

Repository: `src/tc/Main.cpp`, `test/tooling/toolchain.py::TranspilerAttributeCensusGate`, `Makefile`, `test/run.sh`, scripts/al_surface.py, test/tooling/triggers.py, doc/al-surface.json, public headers and baselines. Platform: methods-auto, properties and triggers-auto inventories. The unused property/string-count heuristic is removed; production transpiler diagnostics remain authoritative for parsed declarations. The former ledger is historical reading activity, not coverage proof.

Property scope: `assignmentcompatibility`, `replicatedata`.
