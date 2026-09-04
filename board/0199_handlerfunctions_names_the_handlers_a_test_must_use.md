Type:     task
Status:   open
Parent:   0054
Area:     gen, rt
Source:   developer/attributes/devenv-handlerfunctions-attribute.md
Verdict:  fehlt
Class:    activation

# `[HandlerFunctions]` names the handlers a test must use, and one that never ran fails it

`[HandlerFunctions('Handler1,Handler2')]` on a `[Test]` procedure names the handler methods that
may answer UI during that test. Four rules decide the shape, and the last is the one an
implementation forgets:

- A handler must live in the SAME test codeunit as the test method, so the table is per codeunit and
  never global.
- The parameters of the handled call are the handler's parameters.
- A page or report handler binds to ONE object, because its parameter is typed to it -- the dispatch
  key is the pair (kind, object id).
- **Every non-optional handler named must be CALLED AT LEAST ONCE, or the test FAILS.**

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**47 994 `[HandlerFunctions` declarations** tree-wide. Over the milestone's 78 UT codeunits it is
501 of 2 291 `[Test]` procedures (21.9 %), naming 201 distinct handlers against 266 declared.

## The IST-state

`grep -rn "HandlerFunctions" src/ include/` returns nothing (2026-09-04). The attribute parses into
the raw list and is dropped; no handler is registered and no counter exists.

## The choice

- `TestMethod` gains `std::span<const std::string_view> handlers`, emitted by the generator from
  the attribute -- the same `constexpr` array `kTestMethods` already is.
- A codeunit with any handler procedure emits a second `constexpr` array beside it: kind, object id
  (0 where the kind has none), name, and a thunk address.
- `TestRunner` installs that table for the duration of ONE procedure and counts every call. The
  session holds one table, because a handler may only be reached from its own codeunit.
- After the procedure returns and BEFORE the transaction is decided, a named handler with a count of
  zero fails the test with the platform's own wording.

## Ordering

Needs 0196 (`[EventSubscriber]`) only for the page handlers; the four TEXT handlers -- Message,
Confirm, StrMenu, Hyperlink -- need nothing but this table and are the part that can be finished
before phase 2.

## Gate, and its negative control

Three procedures in a test codeunit of our own: a named handler that runs (green), a named handler
that does NOT run (must go red), and a dialog with no handler at all (must go red at the dialog).

**The negative control is the middle one.** A runner that installs the table and forgets the counter
passes it, and that is exactly the check that would otherwise be blind.
