# 0059 — Surface coverage will distinguish declarations, refusals and tested behaviour

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

Existing surface and trigger counters mostly detect names/signatures. A variadic method that always refuses or ignores arguments can count as present; metadata strings can satisfy text searches without any dispatcher calling them. Baseline files are not all zero, contrary to the old instructions.

## Implementation for Sol

1. Discover all documented types and overload files, including reportinstance/queryinstance/xmlportinstance forms, and compare per-type signatures with the public declarations.
2. Track declared, refusing, implemented and contract-tested as separate states. Map each runtime refusal to its owning consolidated WI; do not call a name search a semantic coverage test.
3. For properties, verify a metadata consumer or explicit diagnostic as well as emission. For triggers, exercise registration and actual lifecycle dispatch.
4. Remove stale Doxygen refusal blocks when implementing a method. Count the actual analyzed source population and never lower findings by excluding newly problematic code.

## Acceptance

Negative fixtures remove a real overload, replace a body with RefuseDoor, disconnect a trigger and drop a property consumer. Each changes the corresponding counter or gate. Missing docs or analyzer outputs fail the audit.

## References

Repository: scripts/al_surface.py, dropped_properties.py, test/triggers.py, doc/al-surface.json, public headers and baselines. Platform: methods-auto, properties and triggers-auto inventories. The former ledger is historical reading activity, not coverage proof.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `assignmentcompatibility`, `replicatedata`.
