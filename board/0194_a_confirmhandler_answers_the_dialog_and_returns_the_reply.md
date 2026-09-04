Type:     task
Status:   open
Parent:   0054
Area:     rt
Source:   developer/attributes/devenv-confirmhandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[ConfirmHandler]` answers the dialog and hands the reply back through `var`

`[ConfirmHandler]` marks a procedure that stands in for the user at a `Confirm` call. Its signature
is fixed by what `Confirm` passes and takes back:

```al
[ConfirmHandler]
procedure H(Question: Text[1024]; var Reply: Boolean)
```

`Confirm(Question, Default)` hands the QUESTION and reads the reply back out of `var Reply`. The
handler is synchronous -- `Confirm` returns the value the handler wrote (board:0054's asynchrony
note applies to `Message` and not here).

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**2 734 `[ConfirmHandler` declarations.** Over the milestone's 78 UT codeunits: 32 declarations in
23 codeunits.

## The IST-state

`Confirm` is a door refusal (`include/Builtins.h`); the attribute parses into the raw list and is
dropped. No dispatch exists.

## The choice

One entry in the codeunit's `constexpr` handler table with kind `Confirm` and no object id, because
a confirm dialog belongs to no object. `Confirm` consults the session's installed table by kind:

- a handler registered -> call it, return what it wrote, increment its counter (0199);
- **no handler and a test runner active -> FAIL at the call**, with BC's own wording;
- no handler and no runner -> the platform's ordinary refusal, because a codeunit run outside a
  runner behaves like an application.

**`GuiAllowed()` must answer TRUE inside a test**, or the BaseApp's `if GuiAllowed then Confirm(...)`
guards skip the branch and the test reports "expected an error, got none" -- a failure pointing
nowhere near its cause. The predecessor paid for that (openerp `_system.py`, `_al_gui_allowed`).

## Ordering

Needs 0199's table. Needs no page runtime -- this is one of the four text handlers that can be
finished before phase 2.

## Gate, and its negative control

A test whose code calls `Confirm` and whose handler writes `true`, and a second writing `false`:
the code under test must take different branches. **The negative control is a `Confirm` with the
handler removed -- it must fail AT the dialog**, not later.
