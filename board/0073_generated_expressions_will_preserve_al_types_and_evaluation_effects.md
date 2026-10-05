# 0073 — Complete declaration-owned call and expression lowering

Status: queued | Priority: P1
Depends on: existing app/namespace symbol identity; native declarations qualified in 0058.
Next: bind one callee declaration by name, receiver, ordered types and argument modes;
compile and execute the four accepted overload counterexamples.

## Implementation

1. Resolve calls once to ProcedureDecl using complete receiver/argument identity,
   conversion ranking and var modes. Consume that binding for C++ spelling, borrowing,
   literal coercion, publisher arguments and returned handles. Unknown/ambiguous
   matches refuse; never first/last-declaration or arity-only selection.
2. Reuse Objects/TableRef/common declaration indices, not another method catalogue.
   Cover own/member Codeunit/Record/Page/Report/Query/XMLport/Interface calls, both
   declaration orders, shadowing, array/generic/enum identity and var writeback.
3. Lower sequencing only when C++ differs: single evaluation, eager AL Boolean
   effects/errors, numeric division/DIV/MOD/overflow, aliases/chained/indexed receivers.
   Keep consumed/discarded failure rules and TryFunction/Codeunit.Run boundaries
   in one value-context model; no implicit TryFunction savepoint.
4. Declared controls/fields/methods precede helper/CLR getter fallbacks. Preserve
   original AL names separately from allocated C++ identifiers. Reject AL-invalid
   Option.AsInteger despite C++ wrapper availability; Enum.AsInteger is distinct.
5. Complete enum/interface defaults, arrays with one-based bounds/var views and
   typed Text/Code/Guid/SecretText conversion at declared boundaries. Source/name caches
   belong to a generation context, not mutable process globals.

## Useful implementation details and acceptance

- `~/Git/openerp/scripts/transpiler/generator/codeunit_gen/_metadata.py`:
  declaration/parameter-mode propagation; page/report/XMLport generator context.
  Use existing `src/gen/{BodyWriter,Names,CodeunitWriter}.cpp`, not Python dispatch.
- Predecessor 1020/1023/1057/1150: var propagation, sibling call paths, Evaluate
  effects inside Boolean expressions and name shadowing; read the actual fixtures.
- Retain four Microsoft-compiler-accepted wrong-binding fixtures and three existing
  first-letter controls. Source-binding fixture's custom Codeunit.Run is invalid AL;
  repair fixture legality independently, not runtime permissiveness.
- Compile/execute `test/transpiler/{source-binding,interface-defaults}/`, generator
  gates and `make tc`; previous/wrong-type/var/order mutants must fail.
  All original SCMProductionOrdersII and full UT identities remain under 0058.
- Developer overload files and BCApps declarations establish signatures; helper names,
  parameter lengths and return types cannot invent an overload identity.

Previous absorbed IDs and detailed matrices remain at
Git `356dadda4a4aa435899bc8aa9e9c4f24a8c0fa21:board/`.
