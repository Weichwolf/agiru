Type:     task
Status:   open
Parent:   0061
Area:     gen, rt
Source:   developer/attributes/devenv-tryfunction-attribute.md
Verdict:  fehlt
Class:    activation

# A `[TryFunction]` catches the error, returns `false`, and rolls back what it wrote

`[TryFunction]` makes a procedure return a Boolean it did not declare: it yields `true` when the
body completed and `false` when it raised, and the error's text is left where `GetLastErrorText`
reads it.

**One restriction the page states and nothing else does**: "In test and upgrade codeunits, this
property only applies to NORMAL methods" -- so `[TryFunction]` on a `[Test]` procedure or on an
upgrade method is not a try method at all. That is a translation-time check (board:0208).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**919 `[TryFunction` declarations.**

## The IST-state

`grep -rn "TryFunction" src/ include/` finds nothing that acts on it (2026-09-04). The attribute
parses into the raw list and is dropped, so a try procedure is emitted with its declared return type
-- usually none -- and its errors propagate. Every one of the 919 call sites that tests the result
is therefore translated against a procedure that has no result.

## The choice

The generator emits the procedure with `Boolean` as its return type and wraps the translated body:

```cpp
Boolean TryX() {
  detail::Scope scope;
  try { <body>; } catch (const Error &e) { scope.Discard(e.what()); return false; }
  scope.Keep();
  return true;
}
```

`detail::Scope` is the boundary `AssertError` already uses (`include/runtime/Error.h:50`), so the
write set is discarded on failure and `GetLastErrorText` finds the message -- the two halves AL
promises together.

**Why not `std::expected`.** The tree prefers it "where a refusal carries its reason", and here the
reason is fetched separately by `GetLastErrorText`, which is AL's own shape. An `expected` would
carry the text twice and change 919 call sites from `if TryX() then` into something AL does not
write.

**What must NOT be caught**: only `agiru::Error`. A `std::bad_alloc` is not an AL error and a
`[TryFunction]` that swallowed one would be `catch (...) {}` with extra steps.

## Ordering

After board:0061 defines the boundary. Before board:0028's `GetLastErrorText`, which has nothing to
read until a try function writes it.

## Gate, and its negative control

A try procedure that inserts a row and then raises: the call returns `false`, `GetLastErrorText`
holds the message, and **the row is gone**. A try procedure that completes returns `true` and keeps
its row.

**The negative control is the row.** A wrapper that catches and returns `false` without the scope
passes the return-value assertion and leaves the write behind -- which is the defect that matters,
because it is invisible until something counts rows.
