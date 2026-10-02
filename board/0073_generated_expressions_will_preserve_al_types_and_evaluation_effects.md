# 0073 — Generated expressions will preserve AL types and evaluation effects

Status: open | Priority: P1 | Stage: UT lowering | Reviewed: 2026-09-28
Depends on: 0033 symbol identity.

## Evidence

- Integrated control-name allocation reserves page procedure spellings before allocating suffixes. Frozen tree reproduced `DataSyncStatus.h`: AL part "Posting Errors" collided with procedure PostingErrors. After full regeneration, its header compiles with Clang 19/GCC 14 and body with Clang 19. A generated two-part/two-procedure fixture executes six checks under both compilers, preserving original AL lookup names and separate instances. GenPageGate: 12 checks green; frozen pre-fix generator: 7 red. Logs and fixture inputs: `build/development-generator/`.
- The frozen tree also exposes five header failures from undeclared absent interfaces: PowerBIServiceProvider and GraphAuthorization. Their actual AL declarations are excluded by namespace while in-scope callers name them. This remains an identity/dependency-closure gap in 0033/0034, not a working interface implementation or permission to broaden fallback conversions.
- `BodyWriter::Binary` promotes division to Decimal but Boolean Link emits short-circuit operators. AL eager-operand behavior needs execution proof.
- `TableWriter` caches names by AST address/partial identity; shared option generation writes sanitized C++ spelling as AL names.
- `BodyWriter::ControlTrigger` checks only the base method name against AL procedures, then returns one unchecked `_Control` suffix. Reserve the complete generated member namespace: both that suffix and another control's synthesized method can collide. Cover multiple occupied suffixes and two controls with intersecting trigger/getter spellings; do not rename either AL declaration.

## Implementation

1. Resolve typed callee/receiver and value-consumption in a lowering pass before printing. Sequence AL operands explicitly where C++ evaluation differs; reuse the same semantic representation for 0061.
2. Add execution fixtures for integer division yielding Decimal, decimal DIV/MOD, eager Boolean operands with var effects, overflow and operand order. Derive expectations from AL documentation and source usage.
3. Centralize typed expression lowering before printing C++. Avoid global spelling sets deciding whether an unrelated field is a method. Resolve by receiver type, namespace and declaring scope.
4. Gate arrays with all dimensions, one-based bounds and var views; enum compatibility only when declared; interface inheritance, implementation selection and assignment ownership.
5. Remove the process-static field-name cache or bind it to an explicit generation context with complete ownership. Emit original AL option names separately from sanitized identifiers; gate `Group(Resource)`, `% Extra` and `LCY Extra` through the real shared option writer. Use generated static_asserts for known limits and type relations. Unsupported conversions must fail at generation/compile time rather than fall into permissive Variant conversions.

## Acceptance

- Include 7/2=3.5, a right Boolean operand that changes a var despite a decisive left operand, multidimensional ArrayLen, incompatible enums and a returned interface call. Inspect emitted code and execute it; header compilation alone is insufficient.

## References

Control/procedure allocation: `src/gen/PageWriter.cpp::ControlIdentifiers`, `test/gate/GenPageGate.cpp`; platform `devenv-page-object.md` separates layout and AL procedures; BCApps main `src/Layers/W1/BaseApp/DataSyncStatus.Page.al`. No user-facing rule specifies C++ spelling; preserve AL identity. Predecessor `openerp/board/1318_gleichnamige-definitionen-ueberschreiben-sich-still-im-genera.md` concerns overwritten definitions/overloads; retain distinct original AL identities instead of replacing either declaration.

Platform: devenv-al-operators.md, type conversion tables, array methods, interface/enum properties. AL: Round(1/4*100,1) and Evaluate in compound conditions. Predecessor: WI-1057 proves eager effects; array and var-parameter findings identify copying traps.

Property scope: `assignmentcompatibility`, `assignmentcompatibilityreason`, `defaultimplementation`, `implementation`, `singleinstance`, `unknownvalueimplementation`.
