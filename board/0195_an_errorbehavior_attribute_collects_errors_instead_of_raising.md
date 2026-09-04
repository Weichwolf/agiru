Type:     task
Status:   open
Parent:   0055
Area:     gen, rt
Source:   developer/attributes/devenv-errorbehavior-attribute.md
Verdict:  fehlt
Class:    activation

# An `[ErrorBehavior]` attribute collects errors inside the scope instead of raising at the first

`[ErrorBehavior(Behavior: ErrorBehavior)]` on a method. `Collect` postpones error handling to the
end of the call: "AL code execution doesn't stop on errors. But instead, it continues until the end
and gathers errors as they occur" (`devenv-error-collection.md`).

**And an uncollected collection RAISES at the boundary**: "if any errors are present in the
collected list when a procedure ends, the user receives an error dialog that concatenates all the
error messages". So the attribute does not remove the error, it moves it -- from the first failing
statement to the end of the method, and from one message to all of them joined.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**25 `[ErrorBehavior` declarations** -- small, and every one of them is a bulk-validation routine
whose whole point is to show the user every problem at once.

## The IST-state

`include/type/ErrorBehavior.h` exists as a door header. `HasCollectedErrors`,
`GetCollectedErrors`, `ClearCollectedErrors` and `IsCollectingErrors` are door refusals
(`coverage`-verified). The attribute parses and is dropped, so a collecting method raises at its
first error like any other.

## The choice

The same scoped guard as board:0193, on a different session field: the annotated method pushes
"collecting", and `Error` consults it -- appending to the session's collected list and RETURNING
instead of throwing. The guard's destructor, on the way out, raises once with the concatenation if
the list is non-empty and nobody drained it.

**The subtlety that decides the shape**: `ClearCollectedErrors` does NOT roll back what was written
(board:0028), so the documented pattern wraps a collecting call in `if Codeunit.Run then`
(board:0077). The guard therefore must not touch the transaction -- collecting is about error
FLOW, and the write set stays whatever the enclosing boundary decides.

## Ordering

After board:0055's error type carries its text, and after board:0028 declares the four collection
builtins. 25 sites, none of them on the milestone's critical path.

## Gate, and its negative control

A collecting method with three failing validations: all three appear in `GetCollectedErrors`,
execution reached the end, and the concatenated raise did not happen because the test drained the
list. The same method with the list left undrained must raise once, with all three texts.

**The negative control is the undrained case.** A guard that collects and never raises turns 25
bulk validations into silent successes.
