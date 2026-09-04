Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/triggers-auto/pagefield/devenv-onassistedit-pagefield-trigger.md, developer/triggers-auto/pagefieldextension/devenv-onassistedit-pagefieldextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page field's `OnAssistEdit` replaces the AssistEdit button's own behaviour

```al
trigger OnAssistEdit()
```

"Runs **in place of** the AssistEdit property features that are provided in the application."
AssistEdit is the ellipsis button beside a field that opens a custom picker -- `ui-enter-data.md`
lists it among the four pickers: "Some fields provide custom pickers that are suited to looking up
and choosing the best value for that field, such as a pop-up window."

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnAssistEdit()` on a page field or extension: **352 declarations** -- and the commonest
body in the BaseApp is the number-series picker, which is why the trigger matters to a document
page.

## The IST-state

No page runtime. `TestField::Activate` exists (`src/rt/TestPage.cpp`) and reaches `Unopened()`;
there is no `AssistEdit` on `TestField` at all.

## The choice

The trigger is consulted from the page's assist-edit gesture, and **`TestField` gains an
`AssistEdit()`** -- `testpage-data-type.md`'s surface includes it, and board:0030's ledger of what
`TestPage` owes does not list it yet.

## Ordering

Blocked on board:0030.

## Gate, and its negative control

A field whose `OnAssistEdit` writes a value: invoking assist-edit through `TestPage` leaves the
field holding it.

**The negative control is a field WITHOUT the trigger** -- invoking assist-edit on it must do
nothing rather than fall through to a lookup, because the two gestures are different buttons.
