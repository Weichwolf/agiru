# 0073 — Generated expressions will preserve AL types and evaluation effects

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

`BodyWriter.cpp::Binary` already promotes `/` to Decimal, so the old integer-division finding is obsolete. Boolean chains still pass `&&`/`||` through Link and short-circuit; decimal DIV/MOD and side-effect order need execution fixtures. Names and array/interface bridges have grown across multiple scope implementations. `TableWriter.cpp` caches field identifiers by AST address plus partial identity; a reused address with changed fields can reuse stale names. `src/tc/Main.cpp::WriteOptions` writes sanitized C++ enumerator spellings into OptionTraits names instead of original AL member text.

## Implementation for Sol

1. Add execution fixtures for integer division yielding Decimal, decimal DIV/MOD, eager Boolean operands with var effects, overflow and operand order. Derive expectations from AL documentation and source usage.
2. Centralize typed expression lowering before printing C++. Avoid global spelling sets deciding whether an unrelated field is a method. Resolve by receiver type, namespace and declaring scope.
3. Gate arrays with all dimensions, one-based bounds and var views; enum compatibility only when declared; interface inheritance, implementation selection and assignment ownership.
4. Remove the process-static field-name cache or bind it to an explicit generation context with complete ownership. Emit original AL option names separately from sanitized identifiers; gate `Group(Resource)`, `% Extra` and `LCY Extra` through the real shared option writer. Use generated static_asserts for known limits and type relations. Unsupported conversions must fail at generation/compile time rather than fall into permissive Variant conversions.

## Acceptance

Include 7/2=3.5, a right Boolean operand that changes a var despite a decisive left operand, multidimensional ArrayLen, incompatible enums and a returned interface call. Inspect emitted code and execute it; header compilation alone is insufficient.

## References

Platform: devenv-al-operators.md, type conversion tables, array methods, interface/enum properties. AL: Round(1/4*100,1) and Evaluate in compound conditions. Predecessor: WI-1057 proves eager effects; array and var-parameter findings identify copying traps.

Consolidated property scope (look up each under `developer/properties/`; carriage alone does not close behaviour): `assignmentcompatibility`, `assignmentcompatibilityreason`, `defaultimplementation`, `implementation`, `singleinstance`, `unknownvalueimplementation`.
