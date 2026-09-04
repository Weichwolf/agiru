Type:     epic
Status:   open
Area:     al, gen
Source:   developer/attributes/devenv-method-attributes.md
Verdict:  fehlt
Tags:     surface, counted

# Every declared attribute is acted on or refused, and none is dropped in silence

`src/al/Parser.cpp:545` reads EVERY attribute into `ProcedureDecl::attributes` as a raw string, and
`al::HasAttribute` (`Parser.cpp:926`) matches one by name, case-insensitively, ignoring anything
from the first `(`. So all 41 documented attributes PARSE.

**The generator acts on four.** `src/gen/CodeunitWriter.cpp`:

| | |
|---|---|
| `IntegrationEvent`, `BusinessEvent`, `InternalEvent` | `IsPublisher` (`:29`) -- the procedure gets an empty body |
| `Test` | `IsTest` (`:77`) -- the procedure enters `kTestMethods` |
| `TransactionModel` | `TransactionModelOf` (`:65`) -- mapped to `AutoCommit` / `None` / `AutoRollback` |

**The other 37 are parsed and dropped.** CLAUDE.md: "accepting a declaration and doing nothing with
it is worse than refusing it, and `catch (...) {}` is a finding with a counter." An attribute is a
declaration; dropping it is the same shape.

## Why this is an epic and not a task

Each attribute is its own behaviour with its own signature, its own population and its own place in
the runtime -- `[TryFunction]` is a transaction boundary, `[ConfirmHandler]` is a dispatch table,
`[NonDebuggable]` is a compile-time marker. They share only the mechanism that reads them. The
children carry the work; this item carries the RULE and the counter.

## The counter this epic owns

**Attributes the generator acts on: 4 of 41.** It may only rise. Nothing the generator emits today
would let a reader see the other 37 being ignored, which is why the count lives here rather than in
the tree.

## What is true when this closes

- Every one of the 41 attributes is either acted on, or REFUSED with a diagnostic naming it and the
  procedure it sat on.
- The refusal is the DEFAULT: an attribute the generator does not know stops the translation instead
  of vanishing, so a new BC release adding one is a build error rather than silence.
- The count of acted-on attributes is a baseline beside the others in `test/`.
