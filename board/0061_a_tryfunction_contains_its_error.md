Type: root
State: open
Area: al, gen, rt
Tags: navision, semantics

# A `[TryFunction]` contains its error, and the value context decides whether it does

The parser reads a procedure's attributes into `ProcedureDecl::attributes` and the generator asks
for exactly two of them: `IsPublisher` (board:0057) and `Test`. **`TryFunction` occurs nowhere in
`src/`** -- measured 2026-09-04, the only mention in the tree is a sentence in
`include/runtime/Error.h`.

So a try method is emitted as an ordinary procedure returning `void`, and the AL that calls it --
`if TrySomething() then` -- becomes a C++ conditional over `void`. That is a compile error rather
than a wrong answer, which is the better half of the news, and it is one of the classes board:0038's
census walks into.

## The population

| | |
|---|---:|
| `[TryFunction]` in BCApps | **916** |
| of them under `Layers/W1` | **340** |

## What the platform documents, and it is a VALUE CONTEXT rule

`attributes/devenv-tryfunction-attribute.md` and `devenv-handling-errors-using-try-methods.md`:

- **"A method that is designated as a try method has a Boolean return value ... A try method can't
  have a user-defined return value."** So the signature the generator emits is not the signature the
  AL declares: AL writes `procedure X()` and the caller writes `OK := X()`.
- **"The return value isn't accessible within the try method itself."** `exit;` inside it is not
  `exit(false)` -- it ends the method successfully, and the caller gets `true`.
- **AND WHETHER IT CATCHES AT ALL DEPENDS ON THE CALL SITE.** "If a try method call doesn't use the
  return value, the try method operates like an ordinary method, and errors are exposed as usual ...
  the call isn't considered a try function call." The same procedure catches in
  `if DoWork() then` and does not catch in `DoWork();`. That is CLAUDE.md's `value context` trap
  stated by the platform itself, and it is the same rule board:0056 follows for `Find` and
  board:0055 for `Get` -- the generator already knows which context it emits into.
- **A try method does NOT roll back what it wrote.** "changes to the database that are made with a
  try method aren't rolled back", and on-premises the server refuses a write inside one by default
  (`DisableWriteInsideTryFunctions`). So it is NOT a savepoint and must not be built as one: a
  `Codeunit.Run` boundary rolls back, a try method does not, and confusing the two would break the
  posting invariant in the quiet direction.
- `GetLastErrorText` and `GetLastErrorObject` read what it caught -- board:0055 owns the text and the
  code.

## What the predecessor paid for

| item | finding | measured |
|---|---|---|
| **WI-1056** | `exit;` inside a try method returned nothing, so the caller read a false where AL reads true | 40/53 -> 42/53, **GAINED 2**, controls 40/40 |
| **WI-1141** | a caught try method did not store its error, so `GetLastErrorText` after it read the previous error | neutral and correct -- the kind of defect that shows only when something asks |

Both are about the EDGES rather than the mechanism, which says the mechanism itself is small and the
rules around it are where the cost is.

## The choice

- **The attribute changes the SIGNATURE.** A `[TryFunction]` procedure becomes `Boolean X(...)`,
  emitted from the attribute and not from the AL return clause, which AL forbids it to have.
- **The body is wrapped where it is DECLARED, not where it is called** -- one `try`/`catch (Error &)`
  around the emitted body, storing the error where `GetLastErrorText` reads it (board:0055) and
  returning `false`. One place, 916 times.
- **The discard context calls a second entry point that does not catch**, the same split board:0056
  makes for `Find`: `bool X()` answers, `void XOrRaise()` does not catch. What must not happen is one
  function guessing from `[[nodiscard]]`.
- **It is not a transaction boundary.** No savepoint, no rollback -- the platform says so and the
  posting invariant depends on the difference.

## Gate

A try method that raises returns `false` in `if X() then` and propagates in a bare `X();`. `exit;`
inside one yields `true`. `GetLastErrorText` after a caught call reads THAT error and not the
previous one (WI-1141). A row inserted inside a try method that then raises is still there after the
catch -- the platform's own rule, and the negative control is wrapping it in a savepoint, which must
make that case go red.
