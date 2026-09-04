Type:     task
Status:   open
Parent:   0054
Area:     rt, gen
Source:   developer/attributes/devenv-modalpagehandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[ModalPageHandler]` answers a page run MODALLY, and returns the user's action

```al
[ModalPageHandler]
procedure ModalPageHandler(var Page: TestPage <id>)
```

and a second documented signature that also takes the answer back:
`(var Page: Page <id>; var Response: Action)`.

**Modal is the distinction.** `Page.RunModal()` blocks and returns an `Action`; `Page.Run()` does
not. The two handler kinds exist because the caller of a modal page reads a return value and the
caller of a non-modal one does not (board:0210).

The handler's PARAMETER carries the object id, so the dispatch key is the pair (kind, page id) --
not the kind alone, as it is for the four text handlers.

**Two declaration rules, checkable at translation time**: the attribute is only legal inside a
`Subtype = Test` codeunit, and the method must be **global**.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**7 274 `[ModalPageHandler` declarations** -- the largest handler population by a factor of four.
Over the milestone's 78 UT codeunits: 147 declarations in 28 codeunits, more than half of all 266
handlers there.

## The IST-state

`include/runtime/test/TestPage.h` exists and is a real design -- a template deriving from the
generated page class, so `SalesOrder."No.".SetValue('X')` resolves. Every `TestField` body reaches
`Unopened()` (`src/rt/TestPage.cpp`). The attribute parses and is dropped; no page opens.

## The choice

A table entry with kind `ModalPage` and the page id taken from the handler's parameter TYPE, which
the generator already knows. When the code under test runs a page modally, the runtime looks up
(ModalPage, id), constructs the `TestPage<Derived>` over the same record, calls the handler, and
returns the `Action` the handler set -- `Action::LookupOK` where it set none, which is what a user
closing an unhandled modal page returns.

**Why the id comes from the parameter type and not from a string.** `TestPage "Customer List"` is a
type, the generator resolves it to the page object, and a typo is a compile error rather than a
handler that never fires.

## Ordering

**Blocked on the page runtime** (board:0030): a modal page handler receives a page that must be
open. The 147 milestone declarations are therefore phase 2, unlike the four text handlers.

## Gate, and its negative control

Code that runs a page modally, a handler that sets `Action::OK`, and the caller branching on it. A
second page id with no handler must FAIL at the run.

**The negative control is the second** -- a runtime that returns a default action for an unhandled
modal page turns every missing handler into a silent pass.
