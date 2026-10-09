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

- Caller-module gap: `BodyWriter.cpp` passes the current object's module to both
  NavApp getters; `src/net/NavApp.cpp` describes that argument. Developer revision
  `f928288ee840334be73142e5fc0202c0e19b246d`,
  `methods-auto/navapp/navapp-getcallermoduleinfo-method.md`, requires A when an
  A method calls B, which asks for its caller. Preserve GetCurrentModuleInfo's own
  module; implement session-owned method frames and qualify cross-app, same-app,
  nested/reentrant and exception-unwind cases without process globals. 0720's
  Copilot wrapper/readback stays in the System app and is not cross-app proof.
- Refreshed predecessor 1916/1917/1971/1972/1988: allocate collision-free names
  per declaration owner and retain original AL identity everywhere: fields/keys/
  CalcFormula/TableRelation/RecordRef, page/request controls, arrays and table globals.
  Do not globally rename seed columns or choose a record field over a page variable.
  Item Journal Line's UnitCost versus "Unit Cost" changes posting costs; percentage
  versus amount collisions change costing. Qualify original callers and SQL values.
- Resolve namespace-qualified TableRelation through declared symbols and owner/type,
  not dotted-path guesses. The same binding supplies validation and client lookups.
- Value-context cases from 1992/1994/1995: TestPage GoToKey/GoToRecord and record
  Find/Get failures raise when discarded, return false when consumed; cover unqualified
  own-table calls, implicit report dataitems and TestPart receivers. DotNet Char Unicode
  category methods must validate original Customer/Vendor/Contact phone fields, not
  produce a nil value. Unsupported signatures refuse; no unknown-member catch-all.
- Sources: `~/Git/openerp/test/openerp/runtime/test_{colliding_field_names,
  table_global_beside_field,request_page_control_keys,namespaced_table_relation,
  bare_find_in_own_code,testpage_goto_raises,dotnet_char}.py`. Port contracts into
  existing C++ generator gates and AL fixtures; verify local developer guarantees first.

- `src/gen/BodyWriter.cpp` captures each `for` end expression once as an owned value,
  after start evaluation, for ascending/descending and Boolean loops. Fresh lower-case
  C++ temporaries cannot collide with AL's capitalized identifiers; actual declared
  global/var counters, nested scopes, break and continue remain intact.
  Developer `f928288ee840` `devenv-al-control-statements.md` owns conversion and
  undefined post-loop/body-counter behavior. BCApps main `d99152ee35f0`
  `Layers/W1/Tests/Physical Inventory/PhysInvtOrderSubformUT.Codeunit.al`
  enqueues a count followed by typed lot/quantity values and consumes the count in
  the loop bound. Once-only capture is inferred from this native usage; repeated
  C++ evaluation consumed a lot Text as Integer in CU 137462's posting UT.
  User docs `bf5ffffa9b026` `inventory-how-count-inventory-with-documents.md`;
  predecessor 1192 warns that later Codeunit-isolated siblings inherit earlier
  failures, and 876 owns the global-counter lesson. Neither is a conversion workaround.
  `make for-loops JOBS=2`: 49 existing codeunit-generator checks and 32 generated AL checks;
  table/page generator gates retain 105/40 passes; slice-check retains all 14225 sources.
  four compiled repeated-bound/borrowed-bound/typed-queue controls must fail.
  The runner is tidy-clean; BodyWriter findings fall from 21 to 20 after extraction.
  Bound conversion, terminal overflow, Decimal/Date/Time stepping and full native
  evaluation-order proof remain gaps. Full UT replay is still required (0058).
  Verified System package `f59a4e4200af` regeneration emits the repaired native
  handler; translation still exits 1 with 5683 refused properties, not G1 proof.
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
