# 0059 — Surface coverage will distinguish declarations, refusals and tested behaviour

Status: open | Priority: P1 | Stage: UT refusal accounting; All semantic coverage | Reviewed: 2026-09-30
Depends on: 0034 object census; 0058 method results.

## Evidence

- Name/signature and metadata-string counters can count refusing/no-op methods as present. Existing suppression baselines are not semantic coverage.
- A zero-attribute AL page fixture reports `attributes acted on 18446744073709551610 of 0 kind(s) declared`: `src/tc/Main.cpp` subtracts the entire acknowledged-kind inventory from an unsigned observed count. Exit 0 is not a truthful census. Reproducer: own worktree `build/development-page-source/fixture/`, `build/page-source-al-range-transpile.log`.

## Implementation

1. Attach each overload/property/trigger to its consumer, focused proof and owning WI. A generic forwarding/variadic body cannot certify distinct contracts.
2. Discover all documented types and overload files, including reportinstance/queryinstance/xmlportinstance forms, and compare per-type signatures with the public declarations.
3. Track declared, refusing, implemented and contract-tested as separate states. Map each runtime refusal to its owning consolidated WI; do not call a name search a semantic coverage test.
   Attribute counts must partition observed declarations/kinds; intersect acknowledged/refused kinds with the observed set instead of subtracting catalogue sizes. Gate empty, acknowledged-only and mixed populations; no unsigned wraparound.
4. For properties, verify a metadata consumer or explicit diagnostic as well as emission. For triggers, exercise registration and actual lifecycle dispatch.
5. Remove stale Doxygen refusal blocks when implementing a method. Count the actual analyzed source population and never lower findings by excluding newly problematic code.

## Acceptance

- Negative fixtures remove a real overload, replace a body with RefuseDoor, disconnect a trigger and drop a property consumer. Each changes the corresponding counter or gate. Missing docs or analyzer outputs fail the audit.

## References

Repository: scripts/al_surface.py, dropped_properties.py, test/triggers.py, doc/al-surface.json, public headers and baselines. Platform: methods-auto, properties and triggers-auto inventories. The former ledger is historical reading activity, not coverage proof.

Property scope: `assignmentcompatibility`, `replicatedata`.
