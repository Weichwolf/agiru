Type:     task
Status:   open
Parent:   0055
Area:     rt, gen
Source:   developer/devenv-error-collection.md, developer/devenv-error-collection-api.md
Verdict:  deklariert
Class:    activation

# A collected error does not stop execution, and clearing the list does not roll back

**Two pages, one item**: the mechanism and its API surface. board:0195 filed `[ErrorBehavior]` from the
attribute page; this is what the attribute switches on.

> "Using collectable errors essentially **POSTPONES ERROR HANDLING TO THE END OF THE PROCEDURE CALL.
> AL code execution DOESN'T STOP on errors.** It continues until the end and gathers errors as they
> occur."
>
> Adding `[ErrorBehavior(ErrorBehavior::Collect)]` to a procedure **"makes it possible to collect and
> handle errors that are raised IN THE SCOPE of the procedure."**
>
> **"If any errors are present in the collected list WHEN A PROCEDURE ENDS, the user receives an error
> dialog that CONCATENATES ALL THE ERROR MESSAGES."**

**So an `Error()` inside a collect scope RETURNS instead of unwinding** -- and that is not a C++ shape
either. A `throw` that does not unwind is a call that records and continues, so within a collect scope
every `Error()` is a record-and-return and the raising procedure keeps running to its end.

**The unhandled case is a documented fallback, not an omission**: leftover errors become one
concatenated dialog. So the scope's exit always does something with the list.

## The clearing rule, and it is the dangerous one

> **"IMPORTANT: If you CLEAR the list of collected errors, ANY CHANGES PERFORMED IN THE DATABASE WON'T
> BE ROLLED BACK.** So in most cases it makes sense to combine the clear operation with an
> `if Codeunit.Run then ...` statement."

**Collecting errors decouples the error from the rollback.** A procedure can collect five errors,
clear them, and keep every write the failing paths performed. Under CLAUDE.md's first invariant that is
the exact shape to be careful about -- and BC's own advice is to wrap the whole thing in
`Codeunit.Run`, which board:0077 owns and which IS the rollback boundary.

**So the item's rule is: collecting changes error PROPAGATION and never transaction scope.** The
transaction boundary stays `Codeunit.Run`'s, and `ClearCollectedErrors` clears a list and nothing else.

## The API is nine methods on `ErrorInfo` and three on `System`

| on `ErrorInfo` | |
|---|---|
| `Create(String [, Boolean] [, var Record] [, Integer] [, Integer] [, String] [, Verbosity] [, DataClassification] [, Dictionary of [Text, Text]])` | nine parameters |
| `Callstack()` | **"a callstack where the ErrorInfo was collected"** |
| `Collectible([Boolean])` | whether this error is collectible at all |
| `CustomDimensions([Dictionary of [Text, Text]])` | |
| `FieldNo`, `PageNo`, `RecordId`, `SystemId`, `TableId` | **what the error RELATES TO** |

| on `System` | |
|---|---|
| `HasCollectedErrors()` | |
| `GetCollectedErrors([Boolean])` | the Boolean clears while getting |
| `ClearCollectedErrors()` | |

**`ErrorInfo` carries a `RecordId`, a `TableId`, a `FieldNo`, a `PageNo` and a `SystemId`** -- so an
error is addressable to a record and a field, which is what board:0506's `TestField` navigation and
board:0519's Fix-it actions consume. **That makes `ErrorInfo` the structured error type this whole
family is built on**, and board:0055's "an error carries its code and its BC wording" needs it.

**"These methods can be invoked using PROPERTY ACCESS SYNTAX"** -- `FixitErrorInfo.Message(...)` and
`.Message` both work. That is an AL calling convention the generator must accept, and it is the second
sighting after the trigger form in board:0502.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0195 owns `[ErrorBehavior]`'s count. **Stated rather than guessed.**

## The IST-state, and it is why this is `deklariert`

`include/Builtins.h:121` -- `ClearCollectedErrors()` is declared and refuses the door.
`include/Builtins.h:326` -- `GetCollectedErrors(Boolean Clear = {})` is declared and refuses. So **two
of the three `System` methods exist as refusing declarations** (board:0035's counted surface), which is
what `deklariert` means here. `ErrorInfo` is an AL type and its door file is board:0051's question.

## The choice

A per-scope error list on the session, pushed by `[ErrorBehavior(Collect)]` and popped at the
procedure's end, with `Error()` recording into the innermost collect scope instead of throwing when one
is active.

**`Error()` checks the scope stack, it does not have two implementations.** One `Error`, one check.

**The scope's exit concatenates and raises** when the list is non-empty and nothing handled it -- the
documented fallback, which also means the "error was swallowed" failure cannot happen silently.

## Ordering

Behind board:0055's `ErrorInfo` type and board:0077's `Codeunit.Run` boundary. With board:0195.

## Gate, and its negative control

A procedure with `[ErrorBehavior(Collect)]` that raises three errors runs to its end and
`GetCollectedErrors` returns three; the same procedure without the attribute stops at the first.

**The negative control is a write performed before a collected error, followed by
`ClearCollectedErrors`** -- the write must SURVIVE, which is what the documentation says and what an
implementation that ties clearing to a rollback would undo.
